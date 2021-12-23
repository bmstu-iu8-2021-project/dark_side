// Copyright 2021 Demon1747 Specdok117@mail.ru

#ifndef DARK_SIDE_FILE_SHARING_H
#define DARK_SIDE_FILE_SHARING_H

#include "convert.h"
#include "database.h"
#include "encrypt.h"
#include "server.h"

void file_sharing(const std::string &db_path);

std::shared_ptr<Botan::PKCS8_PrivateKey> log_in(const Database &db, User &incoming_user);

void accept_thread(const User &receiver, const Database &db,
                   const std::string &file_dir,
                   const std::shared_ptr<Botan::PKCS8_PrivateKey> &p_key);
void handle_connection(client_ptr client);

void send_file(const User &sender, const Database &db,
               const std::shared_ptr<Botan::PKCS8_PrivateKey> &p_key);

#endif  // DARK_SIDE_FILE_SHARING_H
