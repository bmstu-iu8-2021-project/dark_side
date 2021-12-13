// Copyright 2021 Demon1747 Specdok117@mail.ru

#ifndef DARK_SIDE_USER_INTERFACE_H
#define DARK_SIDE_USER_INTERFACE_H

#include <iostream>
#include <string>

#include "database.h"
#include "encrypt.h"

bool log_in(Database& db) {
  std::string in_uname;
  std::cout << "Enter username: ";
  std::cin >> in_uname;
  User incoming_user;
  try {
    incoming_user = db.extract_user(in_uname);
  } catch (const std::invalid_argument& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }

  std::string input_password;
  for (size_t i = 0; i < 3; ++i) {
    std::cout << "Enter your password: ";
    std::cin >> input_password;

    input_password = hash(input_password, std::to_string(incoming_user.id()));
    ///Пароль в структуре User хранится в хэшированном виде
    if (incoming_user.check_pw(input_password)) {
      std::cout << "Welcome to the club, buddy!" << std::endl;
      std::string line(80, '-');
      std::cout << line << std::endl;
      return true;
    }
    if (i == 2) {
      std::cerr << "Access denied" << std::endl;
      return false;
    }
    std::cout << "Incorrect password, try again!" << std::endl;
  }
}

void add_user(Database& db, const std::string& status) {
  ///Проверяется статус редактора базы данных
  if (status != "admin") {
    std::cerr << "Access denied" << std::endl;
    return;
  }
  size_t in_id;
  std::string in_status;
  std::string in_username;
  std::string in_address;
  std::string in_password;
  size_t in_port;

  std::cout << "Enter user ID: ";
  std::cin >> in_id;
  std::cout << "Enter user status: ";
  std::cin >> in_status;
  std::cout << "Enter user name: ";
  std::cin >> in_username;
  std::cout << "Enter user password: ";
  std::cin >> in_password;
  std::cout << "Enter user address: ";
  std::cin >> in_address;
  std::cout << "Enter user port: ";
  std::cin >> in_port;

  in_password = hash(in_password, std::to_string(in_id));

  User input(in_id, in_status, in_username, in_password, in_address, in_port);
  db.insert_user(input);
}

#endif  // DARK_SIDE_USER_INTERFACE_H
