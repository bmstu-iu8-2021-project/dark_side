// Copyright 2021 Demon1747 Specdok117@mail.ru

#include "encrypt.h"

#include <botan/data_src.h>
#include <botan/hash.h>
#include <botan/mac.h>
#include <botan/pk_keys.h>
#include <botan/pubkey.h>
#include <botan/stream_cipher.h>

#include <chrono>
#include <iostream>

#include "convert.h"
#include "user.h"

using namespace std::chrono;

/// SHAd-256
std::vector<uint8_t> hash(const std::vector<uint8_t>& input) {
  std::unique_ptr<Botan::HashFunction> hash_f(
      Botan::HashFunction::create("SHA-256"));
  hash_f->update(input);
  hash_f->update(input);
  Botan::secure_vector<uint8_t> res(hash_f->final());
  std::vector<uint8_t> result(res.begin(), res.end());
  return result;
}

Secure_channel::Secure_channel(std::shared_ptr<Botan::PKCS8_PrivateKey> my_k)
    :  // iv_(16),
      my_private_key_(std::move(my_k)),
      partner_public_key_(nullptr),
      PK_size_(512),
      mac_size_(32),
      cipher_(Botan::StreamCipher::create("CTR(AES-256,8)")) {}

/// Initialisation of Secure Channel part ////////////////////////////////////

///
std::vector<uint8_t> Secure_channel::send_handshake(const User& sender,
                                                    const User& receiver) {
  if (!partner_public_key_) {
    extract_pub_key(receiver.key_storage());
  }

  size_t my_system_time =
      duration_cast<milliseconds>(system_clock::now().time_since_epoch())
          .count();

  std::vector<uint8_t> out_msg(num_to_bytes(sender.id()) +
                               num_to_bytes(my_system_time));

  std::vector<uint8_t> signature = PK_sign_msg(out_msg);

  out_msg.insert(out_msg.end(), signature.begin(), signature.end());
  return out_msg;
}

///
bool Secure_channel::receive_handshake(const std::vector<uint8_t>& in_msg,
                                       User& sender, const User& receiver,
                                       const Database& db) {
  size_t my_system_time =
      duration_cast<milliseconds>(system_clock::now().time_since_epoch())
          .count();

  size_t sender_time = bytes_to_num(std::vector<uint8_t>(
      in_msg.begin() + sizeof(size_t), in_msg.begin() + 2 * sizeof(size_t)));

  if (my_system_time - sender_time > 600000) {
    std::cerr << "Sender time is too in past" << std::endl;
    return false;
  }

  size_t sender_id = bytes_to_num(
      std::vector<uint8_t>(in_msg.begin(), in_msg.end() + sizeof(size_t)));

  /// There invalid argument exception can be thrown
  sender = db.extract_user_by_id(sender_id);

  if (partner_public_key_ == nullptr) {
    extract_pub_key(sender.key_storage());
  }

  std::vector<uint8_t> msg(in_msg.begin(), in_msg.begin() + 16);
  std::vector<uint8_t> signature(in_msg.begin() + 16, in_msg.end());

  if (!PK_verify_msg(msg, signature)) {
    std::cerr << "Msg is not verified" << std::endl;
    return false;
  }

  return true;
}

/// Sender to receiver step 2
std::vector<uint8_t> Secure_channel::send_init_data() {
  std::unique_ptr<Botan::RandomNumberGenerator> rng(new Botan::System_RNG);

  std::vector<uint8_t> init_part(16);
  rng->randomize(init_part.data(), init_part.size());

  parent_key_.assign(init_part.begin(), init_part.end());

  std::vector<uint8_t> init_sign = PK_sign_msg(init_part);
  init_part.reserve(1024);
  init_part = PK_encrypt_msg(init_part);

  init_part.insert(init_part.end(), init_sign.begin(), init_sign.end());
  return init_part;
}

/// Receiver to sender part 2
bool Secure_channel::finalise_protocol(const std::vector<uint8_t>& in_msg,
                                       const std::string& role) {
  std::vector<uint8_t> in_data(in_msg.begin(), in_msg.begin() + PK_size_);
  std::vector<uint8_t> in_sign(in_msg.begin() + PK_size_, in_msg.end());

  in_data = PK_decrypt_msg(in_data);
  if (!PK_verify_msg(in_data, in_sign)) {
    std::cerr << "Msg is not verified" << std::endl;
    return false;
  }

  if (role == "sender") {
    parent_key_.insert(parent_key_.end(), in_data.begin(), in_data.end());
  } else if (role == "receiver") {
    parent_key_.insert(parent_key_.begin(), in_data.begin(), in_data.end());
  }

  session_key_ = keys_hash(parent_key_, "encryption" + destiny_);
  hmac_key_ = keys_hash(parent_key_, "authentication" + destiny_);

  iv_.assign(parent_key_.begin(), parent_key_.end());
  iv_ = hash(iv_);  /// iv max length = 16;

  cipher_->set_key(session_key_);
  cipher_->set_iv(iv_.data(), cipher_->default_iv_length());

  return true;
}

void Secure_channel::extract_pub_key(const std::string& pub_key_path) {

  std::unique_ptr<Botan::X509_PublicKey> pub_k(
      Botan::X509::load_key(pub_key_path));
  partner_public_key_ = std::move(pub_k);
}

std::vector<uint8_t> Secure_channel::PK_encrypt_msg(
    const std::vector<uint8_t>& msg) {
  std::unique_ptr<Botan::RandomNumberGenerator> rng(new Botan::System_RNG);

  Botan::PK_Encryptor_EME encoder(*partner_public_key_, *rng, "EME1(SHA-256)");

  std::vector<uint8_t> ciphertext = encoder.encrypt(msg, *rng);
  return encoder.encrypt(msg, *rng);
}

std::vector<uint8_t> Secure_channel::PK_decrypt_msg(
    const std::vector<uint8_t>& ciphertext) {
  std::unique_ptr<Botan::RandomNumberGenerator> rng(new Botan::System_RNG);

  Botan::PK_Decryptor_EME decoder(*my_private_key_, *rng, "EME1(SHA-256)");

  Botan::secure_vector<uint8_t> paintext = decoder.decrypt(ciphertext);

  std::vector<uint8_t> output(paintext.begin(), paintext.end());
  return output;
}

std::vector<uint8_t> Secure_channel::PK_sign_msg(
    const std::vector<uint8_t>& msg) {
  std::unique_ptr<Botan::RandomNumberGenerator> rng(new Botan::System_RNG);

  Botan::PK_Signer signer(*my_private_key_, *rng, "EMSA4(SHA-256)");

  std::vector<uint8_t> signature = signer.sign_message(msg, *rng);
  return signature;
}

bool Secure_channel::PK_verify_msg(const std::vector<uint8_t>& msg,
                                   const std::vector<uint8_t>& signature) {
  Botan::PK_Verifier verifier(*partner_public_key_, "EMSA4(SHA-256)");

  return verifier.verify_message(msg, signature);
}

std::vector<uint8_t> Secure_channel::compute_mac(
    const std::vector<uint8_t>& msg) {
  auto hmac =
      Botan::MessageAuthenticationCode::create_or_throw("HMAC(SHA-256)");
  hmac->set_key(hmac_key_);
  hmac->update(msg);
  Botan::secure_vector<uint8_t> res(hmac->final());
  std::vector<uint8_t> result(res.begin(), res.end());
  return result;
}

Botan::secure_vector<uint8_t> Secure_channel::keys_hash(
    const Botan::secure_vector<uint8_t>& in_key,
    const std::string& key_purpose) {
  std::vector<uint8_t> out(in_key.begin(), in_key.end());
  out.insert(out.end(), key_purpose.begin(), key_purpose.end());

  std::unique_ptr<Botan::HashFunction> hash_f(
      Botan::HashFunction::create("SHA-256"));
  hash_f->update(out);
  hash_f->update(out);
  return hash_f->final();
}

///// Stream part //////////////////////////////////////////////////////////////

std::vector<uint8_t> Secure_channel::encipher_buff(
    const std::vector<uint8_t>& input, size_t msg_num) {
  std::vector<uint8_t> enc_buff(input);

  /// Counter
  std::vector<uint8_t> num(num_to_bytes(msg_num));

  /// Authentication: MAC(i || l(x) || x || block_data)
  std::vector<uint8_t> my_auth = compute_mac(
      num + num_to_bytes(destiny_.size()) + str_to_bytes(destiny_) + enc_buff);

  /// Encrypting
  enc_buff.insert(enc_buff.end(), my_auth.begin(), my_auth.end());
  cipher_->encipher(enc_buff);

  /// Adding number
  enc_buff.insert(enc_buff.begin(), num.begin(), num.end());

  return enc_buff;
  //
  //  /// Buff that must be returned
  //  std::vector<uint8_t> enc_buff;
  //  enc_buff.reserve(input.size() * 3 / 2);
  //
  //  /// Cipher initialisation
  //  std::unique_ptr<Botan::StreamCipher> cipher(
  //      Botan::StreamCipher::create("CTR(AES-256,8)"));
  //  cipher->set_key(session_key_);
  //  cipher->set_iv(iv_.data(), iv_.size());
  //
  //  /// Starting encrypting
  //  size_t msg_count = 1;
  //  for (auto it = input.begin(); it < input.end();
  //       it += static_cast<long>(block_size), ++msg_count) {
  //    /// Checking the last block size
  //    if (input.end() - it < block_size) {
  //      block_size = input.end() - it;
  //    }
  //
  //    /// Counter
  //    std::vector<uint8_t> counter_value(num_to_bytes(msg_count));
  //    enc_buff.insert(enc_buff.end(), counter_value.begin(),
  //    counter_value.end());
  //
  //    /// Block that must be encrypted
  //    std::vector<uint8_t> enc_block(it, it + static_cast<long>(block_size));
  //
  //    /// Authentication: MAC(i || l(x) || x || block_data)
  //    std::vector<uint8_t> auth_data(counter_value +
  //                                   num_to_bytes(destiny_.size()) +
  //                                   str_to_bytes(destiny_) + enc_block);
  //    auth_data = compute_mac(auth_data);
  //
  //    /// Block encryption with auth
  //    enc_block.insert(enc_block.end(), auth_data.begin(), auth_data.end());
  //    cipher->encipher(enc_block);
  //
  //    /// Adding block to buffer
  //    enc_buff.insert(enc_buff.end(), enc_block.begin(), enc_block.end());
  //  }
  //
  //  return enc_buff;
}

std::vector<uint8_t> Secure_channel::decrypt_buff(
    const std::vector<uint8_t>& input, size_t msg_num) {
  std::vector<uint8_t> sent_num(input.begin(), input.begin() + sizeof(size_t));

  if (msg_num != bytes_to_num(sent_num)) {
    throw std::runtime_error("Msg is not current");
  }

  std::vector<uint8_t> dec_buff(input.begin() + sizeof(size_t), input.end());
  cipher_->decrypt(dec_buff);

  /// Buff that must be returned
  std::vector<uint8_t> dec_buff_data(
      dec_buff.begin(), dec_buff.end() - static_cast<long>(mac_size_));

  /// Authentication check: MAC(i || l(x) || x || block_data)
  std::vector<uint8_t> received_auth_data(
      dec_buff.end() - static_cast<long>(mac_size_), dec_buff.end());

  std::vector<uint8_t> assumed_auth_data =
      compute_mac(num_to_bytes(msg_num) + num_to_bytes(destiny_.size()) +
                  str_to_bytes(destiny_) + dec_buff_data);

  if (received_auth_data != assumed_auth_data) {
    throw std::runtime_error("MAC is not current");
  }

  return dec_buff_data;

  //  /// Cipher initialisation
  //  std::unique_ptr<Botan::StreamCipher> cipher(
  //      Botan::StreamCipher::create("CTR(AES-256,8)"));
  //  cipher->set_key(session_key_);
  //  cipher->set_iv(iv_.data(), iv_.size());
  //
  //  /// Starting decrypting
  //  size_t msg_count = 1;
  //  for (auto it = input.begin() + sizeof(msg_count); it < input.end();
  //       it += static_cast<long>(enc_block_size), ++msg_count) {
  //    /// Checking the last block size
  //    if (input.end() - it < enc_block_size) {
  //      enc_block_size = input.end() - it;
  //    }
  //
  //    /// Counter (useless)
  //    //      std::vector<uint8_t> counter_value(it, it + sizeof(msg_count));
  //    //      it += sizeof(msg_count);
  //
  //    /// Block decryption
  //    std::vector<uint8_t> enc_block(it, it +
  //    static_cast<long>(enc_block_size)); cipher->decrypt(enc_block);
  //
  //    /// Authentication check: MAC(i || l(x) || x || block_data)
  //    std::vector<uint8_t> received_auth_data(
  //        enc_block.end() - static_cast<long>(mac_size_), enc_block.end());
  //    std::vector<uint8_t> assumed_auth_data(num_to_bytes(msg_count) +
  //                                           num_to_bytes(destiny_.size()) +
  //                                           str_to_bytes(destiny_) +
  //                                           enc_block);
  //    assumed_auth_data = compute_mac(assumed_auth_data);
  //    if (received_auth_data != assumed_auth_data) {
  //      discredited_ = true;
  //    }
  //
  //    /// Adding block to buffer
  //    dec_buff.insert(dec_buff.end(), enc_block.begin(),
  //                    enc_block.end() - static_cast<long>(mac_size_));
}
