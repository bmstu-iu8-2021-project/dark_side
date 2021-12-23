// Copyright 2021 Demon1747 Specdok117@mail.ru

#include <iostream>

#include "database_redactor.h"
#include "file_sharing.h"

int main() {
  std::string db_path("users.db");

  std::cout << "Available commands:" << std::endl
            << "  f [ file-messenger ]  : starts file sharing client"
            << std::endl
            << "  d [ database-redactor ] : starts database redactor"
            << std::endl
            << "  e [ exit ] " << std::endl
            << std::endl
            << "  This application needs special database (users.db) for "
               "current work!"
            << std::endl
            << std::endl
            << "Print your option here: ";

  std::string command;
  std::cin >> command;
  std::string line(80, '-');
  std::cout << std::endl << line << std::endl << std::endl;
  if (command == "f" || command == "file-messenger") {
    file_sharing(db_path);
  } else if (command == "d" || command == "database-redactor") {
    database_redactor(db_path);
  } else if (command == "e" || command == "exit") {
    exit(EXIT_SUCCESS);
  } else {
    std::cout << "Invalid command, terminating..." << std::endl;
    exit(EXIT_FAILURE);
  }

  return 0;
}

//  Secure_channel a, b;
//  auto str = a.send_handshake(user1, user2);
//  b.receive_handshake(str, user2, db);
//  str = b.send_handshake(user2, user1);
//  a.receive_handshake(str, user1, db);
//  str = a.send_init_data();
//  auto str1 = b.send_init_data();
//  a.finalise_protocol(str1, "sender");
//  b.finalise_protocol(str, "receiver");
//  str = a.encipher_buff(str_to_bytes("Yes, -3, yahoooooooo"), 1);
//  str = b.decrypt_buff(str, 1);

//  std::unique_ptr<Botan::PKCS8_PrivateKey> keeeeey1(Botan::PKCS8::load_key(
//      "private_" + std::to_string(user1.id()), *rng, "1234"));
//
//  std::unique_ptr<Botan::X509_PublicKey> keeeeey2(
//      Botan::X509::load_key(user1.key_storage()));
//
//  Botan::PK_Encryptor_EME encoder(*keeeeey2, *rng, "EME1(SHA-256)");
//
//  std::vector<uint8_t> ciphertext =
//      encoder.encrypt(str_to_bytes("Pizdec nahooy blyat"), *rng);
//
//  std::cout << ciphertext.size() << std::endl;
//
//  Botan::PK_Decryptor_EME decoder(*keeeeey1, *rng, "EME1(SHA-256)");
//
//  Botan::secure_vector<uint8_t> paintext = decoder.decrypt(ciphertext);
//
//  std::string output(paintext.begin(), paintext.end());
//
//  Botan::PK_Signer signer(*keeeeey1, *rng, "EMSA4(SHA-256)");
//
//  std::vector<uint8_t> signature =
//      signer.sign_message(str_to_bytes(output), *rng);
//
//  std::cout << signature.size() << std::endl;
//
//  Botan::PK_Verifier verifier(*keeeeey2, "EMSA4(SHA-256)");
//
//  bool aaaaa = verifier.verify_message(str_to_bytes(output), signature);
