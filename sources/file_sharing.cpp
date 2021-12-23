// Copyright 2021 Demon1747 Specdok117@mail.ru

#include "file_sharing.h"

#include <iostream>
#include <list>
#include <thread>

#include "client.h"
#include "server.h"

void file_sharing(const std::string &db_path) {
  Database db(db_path);
  User me;
  std::shared_ptr<Botan::PKCS8_PrivateKey> private_key = log_in(db, me);

  std::string file_dir;
  std::cout << "Enter full path to directory where you want to store accepted "
               "files: ";
  std::cin >> file_dir;
  std::cout << std::endl;

  std::thread acceptor_thread(accept_thread, std::ref(me), std::ref(db),
                              std::ref(file_dir), private_key);

  while (true) {
    std::cout << "Available commands:" << std::endl
              << "  s [ send-file ] " << std::endl
              << "  e [ exit ] " << std::endl
              << std::endl
              << "Print your option here: ";

    std::string command;

    std::cin >> command;
    std::cout << std::endl;
    if (command == "s" || command == "send-file") {
      send_file(me, db, private_key);
    } else if (command == "e" || command == "exit") {
      exit(EXIT_SUCCESS);
    } else {
      std::cout << "Invalid command, terminating..." << std::endl;
      exit(EXIT_FAILURE);
    }

    std::string line(80, '-');
    std::cout << line << std::endl << std::endl;
  }
}

std::shared_ptr<Botan::PKCS8_PrivateKey> log_in(const Database &db,
                                                User &incoming_user) {
  std::string in_uname;
  for (size_t i = 0; i < 3; ++i) {
    std::cout << "Enter username: ";
    std::cin >> in_uname;
    try {
      incoming_user = db.extract_user_by_un(in_uname);
    } catch (const std::invalid_argument &e) {
      std::cout << e.what() << std::endl;
      if (i == 2) {
        std::cout << "Something got wrong, see you next time!" << std::endl;
        exit(EXIT_FAILURE);
      }
      continue;
    }
    break;
  }

  std::string input_password;
  for (size_t i = 0; i < 3; ++i) {
    std::cout << "Enter your password: ";
    std::cin >> input_password;
    std::vector<uint8_t> pw_in_bytes =
        hash(str_to_bytes(input_password + std::to_string(incoming_user.id())));

    input_password = Botan::hex_encode(pw_in_bytes);

    try {
      std::unique_ptr<Botan::RandomNumberGenerator> rng(new Botan::System_RNG);

      /// Botan::Decoding_Error
      std::shared_ptr<Botan::PKCS8_PrivateKey> priv_k(Botan::PKCS8::load_key(
          "private_" + std::to_string(incoming_user.id()), *rng,
          Botan::hex_encode(pw_in_bytes)));

      std::cout << std::endl << "Welcome to the club, buddy!" << std::endl;
      std::string line(80, '-');
      std::cout << line << std::endl << std::endl;
      return priv_k;
    } catch (const Botan::Decoding_Error &err) {
      if (i == 2) {
        std::cout << "Access denied" << std::endl;
        exit(EXIT_FAILURE);
      }

      std::cout << "Incorrect password, try again!" << std::endl;
    }
  }
}

void handle_connection(client_ptr client) { client->connect(); }

void accept_thread(const User &receiver, const Database &db,
                   const std::string &file_dir,
                   const std::shared_ptr<Botan::PKCS8_PrivateKey> &p_key) {
  io_context context;
  ip::tcp::acceptor acceptor(context, ip::tcp::endpoint(ip::tcp::v4(), 1747));
  acceptor.listen();
  std::cout << "Now you can accept files from other users " << std::endl
            << std::endl;
  std::list<std::thread> thr_list;
  boost::system::error_code err_c;
  while (true) {
    client_ptr new_client(new Server(receiver, db, file_dir, p_key));
    acceptor.accept(new_client->sock(), err_c);
    if (err_c != boost::system::errc::success) {
      std::cout << "Acceptor error: " << err_c << std::endl;
      break;
    }

    thr_list.emplace_back(handle_connection, new_client);
  }
  for (auto &thread : thr_list) {
    thread.join();
  }
}

void send_file(const User &sender, const Database &db,
               const std::shared_ptr<Botan::PKCS8_PrivateKey> &p_key) {
  std::string in_file_path;
  std::cout << std::endl
            << "Write the full path to file you want to transmit: ";
  std::cin >> in_file_path;

  db.print_users();
  std::string in_username;
  std::cout << std::endl
            << "Enter a name of user you want to transmit a file to: ";
  std::cin >> in_username;

  User receiver;
  try {
    receiver = db.extract_user_by_un((in_username));
  } catch (const std::invalid_argument &err) {
    std::cout << err.what() << std::endl;
    return;
  }

  Client client(sender, receiver, db, p_key, std::move(in_file_path));
  try {
    client.connect();
    if (client.sec_channel_init()) {
      client.send_file();
    }
    client.disconnect();
  } catch (boost::system::system_error &err) {
    std::cout << "Client terminated" << std::endl;
  }

  std::string line(80, '-');
  std::cout << std::endl << line << std::endl;
}
