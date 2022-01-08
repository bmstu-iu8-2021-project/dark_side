// Copyright 2021 Demon1747 Specdok117@mail.ru

#include "database_redactor.h"

#include <boost/filesystem/operations.hpp>
#include <fstream>
#include <iostream>

#include "convert.h"
#include "database.h"
#include "encrypt.h"
#include "log.h"

void database_redactor(const std::string& db_path) {
  std::cout << "Welcome to the database redactor!" << std::endl << std::endl;

  //  std::string in_uname;
  //  std::string in_pw;
  //
  //  std::cout << "Enter username: ";
  //  std::cin >> in_uname;
  //  std::cout << "Enter password: ";
  //  std::cin >> in_pw;
  //
  //  std::string really_strong_password(
  //      "D82494F05D6917BA02F7AAA29689CCB444BB73F20380876CB05D1F37537B7892");
  //
  //  if (in_uname != "admin" ||
  //      Botan::hex_encode(hash(str_to_bytes(in_pw))) !=
  //      really_strong_password) {
  //    std::cout << "Access denied!" << std::endl;
  //    exit(EXIT_FAILURE);
  //  }
  //
  //  std::cout << std::endl << "Access granted" << std::endl << std::endl;

  std::fstream db_file("users.db");
  if (!db_file.is_open()) {
    Database db;
  }

  Database db(db_path);

  boost::system::error_code err;

  if (!boost::filesystem::exists("public_keys")) {
    boost::filesystem::create_directory("public_keys", err);
  }
  if (err) {
    std::cout << "Failed to create a directory" << std::endl;
  }

  while (true) {
    std::cout << "Available commands:" << std::endl
              << "  p [ print-users ] " << std::endl
              << "  a [ add-user ] " << std::endl
              << "  d [ delete-user ] " << std::endl
              << "  e [ exit ] " << std::endl
              << std::endl
              << "Print your option here: ";

    std::string command;

    std::cin >> command;
    std::cout << std::endl;
    if (command == "p" || command == "print-users") {
      db.print_users();
    } else if (command == "a" || command == "add-user") {
      add_user(db);
    } else if (command == "d" || command == "delete-user") {
      delete_user(db);
    } else if (command == "e" || command == "exit") {
      BOOST_LOG_TRIVIAL(trace) << "Program terminated successfully";
      exit(EXIT_SUCCESS);
    } else {
      std::cout << "Invalid command, terminating..." << std::endl;
      BOOST_LOG_TRIVIAL(fatal) << "Program terminated with invalid command";
      exit(EXIT_FAILURE);
    }

    std::string line(80, '-');
    std::cout << line << std::endl << std::endl;
  }
}

void add_user(Database& db) {
  // Checking redactor status
  //  if (status != "admin") {
  //    std::cerr << "Access denied" << std::endl;
  //    return;
  //  }

  std::string str_id;
  std::string in_username;
  std::string in_address;
  std::string in_password;
  std::string str_port;

  try {
    /// Entering data
    std::cout << "Enter user ID (number): ";
    std::cin >> str_id;
    size_t in_id = std::stoul(str_id);
    std::cout << "Enter user name: ";
    std::cin >> in_username;
    std::cout << "Enter user password: ";
    std::cin >> in_password;
    std::cout << "Enter user address: ";
    std::cin >> in_address;
    std::cout << "Enter user port (this number must be > 1024): ";
    std::cin >> str_port;
    size_t in_port = std::stoul(str_port);

    if (in_port <= 1024) {
      throw std::invalid_argument("You entered wrong port number!");
    }

    /// Hashing user password
    std::vector<uint8_t> hash_in_bytes =
        hash(str_to_bytes(in_password + std::to_string(in_id)));

    std::string pub_key_storage("public_keys/public_k_" +
                                std::to_string(in_id) + ".txt");
    std::string priv_key_storage("private_" + std::to_string(in_id));

    std::unique_ptr<Botan::RandomNumberGenerator> rng(new Botan::System_RNG);

    /// Generating key pair
    size_t pub_key_size = 4096;

    Botan::RSA_PrivateKey key_pair(*rng, pub_key_size);
    std::string public_key = Botan::X509::PEM_encode(key_pair);
    std::string priv_key(Botan::PKCS8::PEM_encode(
        key_pair, *rng, Botan::hex_encode(hash_in_bytes)));

    std::ofstream pub_key_file(pub_key_storage, std::ofstream::binary);
    pub_key_file.write(public_key.data(),
                       static_cast<std::streamsize>(public_key.size()));
    pub_key_file.close();

    std::ofstream priv_key_file(priv_key_storage, std::ofstream::binary);
    priv_key_file.write(priv_key.data(),
                        static_cast<std::streamsize>(priv_key.size()));
    priv_key_file.close();

    User input(in_id, in_username, in_address, in_port, pub_key_storage);
    db.insert_user(input);

    std::cout << std::endl
              << "A new user was added successfully!" << std::endl
              << std::endl;
  } catch (const std::exception& err) {
    std::cout << std::endl
              << err.what() << std::endl
              << std::endl
              << "User insertion denied" << std::endl
              << std::endl;
  }
}

void delete_user(Database& db) {
  if (db.empty()) {
    std::cout << "There is no users in database" << std::endl << std::endl;
    return;
  }

  db.print_users();

  size_t del_id;
  std::cout << "Enter id of user you want to delete: ";
  std::cin >> del_id;
  std::cout << std::endl;

  try {
    std::string pub_key_path(db.extract_user_by_id(del_id).key_storage());
    std::string priv_key_path("private_" + std::to_string(del_id));

    std::remove(pub_key_path.data());
    std::remove(priv_key_path.data());

    db.delete_user(del_id);
  } catch (const std::invalid_argument& err) {
    std::cout << err.what() << std::endl;
    return;
  }
  std::cout << "User was deleted successfully!" << std::endl << std::endl;
}
