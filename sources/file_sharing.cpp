// Copyright 2021 Demon1747 Specdok117@mail.ru

#include "file_sharing.h"

#include <atomic>
#include <boost/filesystem.hpp>
#include <boost/system.hpp>
#include <iostream>
#include <list>
#include <thread>

#include "client.h"
#include "log.h"
#include "server.h"

std::atomic<bool> stop(false);
std::mutex stop_mutex;
std::mutex queue_lock;
std::condition_variable time_to_stop;
std::condition_variable time_to_work;

/// Logging in interface ///////////////////////////////////////////////////////

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
        BOOST_LOG_TRIVIAL(fatal) << "Program terminated with invalid login";
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

      std::cout << std::endl
                << "Welcome to the club, " << incoming_user.username() << "!"
                << std::endl;
      std::string line(80, '-');
      std::cout << line << std::endl << std::endl;
      return priv_k;
    } catch (const Botan::Decoding_Error &err) {
      if (i == 2) {
        std::cout << "Access denied" << std::endl;
        BOOST_LOG_TRIVIAL(fatal) << "Program terminated with invalid password";
        exit(EXIT_FAILURE);
      }

      std::cout << "Incorrect password, try again!" << std::endl;
    } catch (const Botan::Stream_IO_Error &err) {
      std::cout << "You have no key" << std::endl;
      BOOST_LOG_TRIVIAL(fatal) << "Program terminated with invalid key";
      exit(EXIT_FAILURE);
    }
  }
  return nullptr;
}

/// File messenger interface ///////////////////////////////////////////////////

void file_sharing(const std::string &db_path) {
  std::cout << "Welcome to the file messenger!" << std::endl << std::endl;

  Database db(db_path);
  User me;
  std::shared_ptr<Botan::PKCS8_PrivateKey> private_key = log_in(db, me);

  boost::system::error_code err;

  std::string file_dir = "accepted_files";
  if (!boost::filesystem::exists(file_dir)) {
    boost::filesystem::create_directory(file_dir, err);
  }
  if (err) {
    std::cout << "Failed to create a directory" << std::endl;
  }

  std::queue<client_ptr> connections_queue;

  std::thread acceptor_thr(accept_thread, std::ref(me), std::ref(db),
                           std::ref(file_dir), private_key,
                           std::ref(connections_queue));

  std::thread handle_connections_thr(handle_clients_thread,
                                     std::ref(connections_queue));

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
      {
        std::unique_lock<std::mutex> stop_locker(stop_mutex);
        std::unique_lock<std::mutex> queue_locker(queue_lock);
        stop.store(true);
        time_to_stop.notify_one();
        time_to_work.notify_one();
      }
      acceptor_thr.join();
      handle_connections_thr.join();

      BOOST_LOG_TRIVIAL(info) << "Program terminated successfully";
      exit(EXIT_SUCCESS);
    } else {
      std::cout << "Invalid command, terminating..." << std::endl;

      {
        std::unique_lock<std::mutex> stop_locker(stop_mutex);
        std::unique_lock<std::mutex> queue_locker(queue_lock);
        stop.store(true);
        time_to_stop.notify_one();
        time_to_work.notify_one();
      }
      acceptor_thr.join();
      handle_connections_thr.join();

      BOOST_LOG_TRIVIAL(fatal) << "Program terminated with invalid command ";
      exit(EXIT_FAILURE);
    }

    std::string line(80, '-');
    std::cout << line << std::endl << std::endl;
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
  std::cout
      << std::endl
      << "Choose a user you want to transmit a file to and enter username: ";
  std::cin >> in_username;

  User receiver;
  try {
    receiver = db.extract_user_by_un((in_username));
  } catch (const std::invalid_argument &err) {
    std::cout << std::endl << err.what() << std::endl;
    return;
  }

  if (sender.id() == receiver.id()) {
    std::cout << std::endl
              << "Don't choose yourself..." << std::endl
              << std::endl;
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
    BOOST_LOG_TRIVIAL(error) << "Client terminated with error";
  }

  std::string line(80, '-');
  std::cout << std::endl << line << std::endl;
}

/// Connections acceptor interface /////////////////////////////////////////////

void accept_thread(const User &receiver, const Database &db,
                   const std::string &file_dir,
                   const std::shared_ptr<Botan::PKCS8_PrivateKey> &p_key,
                   std::queue<client_ptr> &connections_queue) {
  io_context context;
  ip::tcp::acceptor acceptor(context,
                             ip::tcp::endpoint(ip::tcp::v4(), receiver.port()));
  acceptor.listen();
  BOOST_LOG_TRIVIAL(info)
      << "Now you are available to accept files from other users ";

  std::thread stop_acceptor(stop_it, std::ref(acceptor));

  handle_accept(receiver, db, file_dir, p_key, connections_queue, acceptor,
                time_to_work);
  context.run();

  stop_acceptor.join();
}

void handle_accept(const User &receiver, const Database &db,
                   const std::string &file_dir,
                   const std::shared_ptr<Botan::PKCS8_PrivateKey> &p_key,
                   std::queue<client_ptr> &connections_queue,
                   ip::tcp::acceptor &acceptor,
                   std::condition_variable &time_to_work) {
  if (!stop) {
    acceptor.async_accept([&](std::error_code err_c, ip::tcp::socket socket) {
      if (!err_c) {
        client_ptr new_client = std::make_shared<Server>(
            std::move(socket), receiver, db, file_dir, p_key);
        {
          std::unique_lock<std::mutex> locker(queue_lock);
          connections_queue.push(std::move(new_client));
          time_to_work.notify_one();
        }
      }
      handle_accept(receiver, db, file_dir, p_key, connections_queue, acceptor,
                    time_to_work);
    });
  }
}

/// Handling connections interface /////////////////////////////////////////////

void handle_clients_thread(std::queue<client_ptr> &connect_queue) {
  std::queue<std::thread> in_connections;

  while (!stop) {
    std::unique_lock<std::mutex> queue_locker(queue_lock);
    time_to_work.wait(queue_locker,
                      [&]() { return !connect_queue.empty() || stop.load(); });
    for (; !connect_queue.empty(); connect_queue.pop()) {
      in_connections.emplace(
          std::thread(handle_connection, connect_queue.front()));
    }
  }

  for (; !in_connections.empty(); in_connections.pop()) {
    in_connections.front().join();
  }
}

void handle_connection(client_ptr client) { client->connect(); }

/// Stopping program interface /////////////////////////////////////////////////

void stop_it(ip::tcp::acceptor &acceptor) {
  std::unique_lock<std::mutex> stop_locker(stop_mutex);

  time_to_stop.wait(stop_locker, [&]() { return stop.load(); });

  acceptor.cancel();
  acceptor.close();
}
