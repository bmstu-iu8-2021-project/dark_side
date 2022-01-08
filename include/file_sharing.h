// Copyright 2021 Demon1747 Specdok117@mail.ru

#ifndef DARK_SIDE_FILE_SHARING_H
#define DARK_SIDE_FILE_SHARING_H

#include <condition_variable>
#include <queue>

#include "convert.h"
#include "database.h"
#include "encrypt.h"
#include "server.h"

std::shared_ptr<Botan::PKCS8_PrivateKey> log_in(const Database &db,
                                                User &incoming_user);

void file_sharing(const std::string &db_path);
void send_file(const User &sender, const Database &db,
               const std::shared_ptr<Botan::PKCS8_PrivateKey> &p_key);

void accept_thread(const User &receiver, const Database &db,
                   const std::string &file_dir,
                   const std::shared_ptr<Botan::PKCS8_PrivateKey> &p_key,
                   std::queue<client_ptr> &connections_queue);
void handle_accept(const User &receiver, const Database &db,
                   const std::string &file_dir,
                   const std::shared_ptr<Botan::PKCS8_PrivateKey> &p_key,
                   std::queue<client_ptr> &connections_queue,
                   ip::tcp::acceptor &acceptor);

void handle_clients_thread(std::queue<client_ptr> &connect_queue);
void handle_connection(client_ptr client);

void stop_it(ip::tcp::acceptor &acceptor);

#endif  // DARK_SIDE_FILE_SHARING_H
