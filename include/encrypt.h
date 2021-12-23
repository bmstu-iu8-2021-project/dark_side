// Copyright 2021 Demon1747 Specdok117@mail.ru

#ifndef DARK_SIDE_ENCRYPTION_H
#define DARK_SIDE_ENCRYPTION_H

#include <botan/hex.h>
#include <botan/pkcs8.h>
#include <botan/rsa.h>
#include <botan/stream_cipher.h>
#include <botan/system_rng.h>
#include <botan/x509_key.h>

#include "database.h"

/// SHAd-256
std::vector<uint8_t> hash(const std::vector<uint8_t>& input);

class Secure_channel {
 public:
  Secure_channel(std::shared_ptr<Botan::PKCS8_PrivateKey> my_k);

  /// Initialisation of Secure Channel part ////////////////////////////////////
  /// id + indextime + fingerprint
  std::vector<uint8_t> send_handshake(const User& sender, const User& receiver);
  bool receive_handshake(const std::vector<uint8_t>& in_msg, User& sender,
                         const User& receiver, const Database& db);

  /// key parts
  std::vector<uint8_t> send_init_data();
  bool finalise_protocol(const std::vector<uint8_t>& in_msg,
                         const std::string& role);

 private:
  void extract_pub_key(const std::string& pub_key_path);

  std::vector<uint8_t> PK_encrypt_msg(const std::vector<uint8_t>& msg);

  std::vector<uint8_t> PK_decrypt_msg(const std::vector<uint8_t>& ciphertext);

  std::vector<uint8_t> PK_sign_msg(const std::vector<uint8_t>& msg);

  bool PK_verify_msg(const std::vector<uint8_t>& msg,
                     const std::vector<uint8_t>& signature);

  Botan::secure_vector<uint8_t> keys_hash(
      const Botan::secure_vector<uint8_t>& in_key,
      const std::string& key_purpose);

  /// Stream part //////////////////////////////////////////////////////////////
 public:
  std::vector<uint8_t> decrypt_buff(const std::vector<uint8_t>& input,
                                    size_t msg_num);

  std::vector<uint8_t> encipher_buff(const std::vector<uint8_t>& input,
                                     size_t msg_num);

 private:
  std::vector<uint8_t> compute_mac(const std::vector<uint8_t>& msg);

 private:
  std::shared_ptr<Botan::PKCS8_PrivateKey> my_private_key_;
  std::unique_ptr<Botan::X509_PublicKey> partner_public_key_;

  Botan::secure_vector<uint8_t> parent_key_;
  std::vector<uint8_t> iv_;
  Botan::secure_vector<uint8_t> session_key_;
  Botan::secure_vector<uint8_t> hmac_key_;

  std::unique_ptr<Botan::StreamCipher> cipher_;

  std::string destiny_;

  size_t mac_size_;
  size_t PK_size_;
};

#endif  // DARK_SIDE_ENCRYPTION_H
