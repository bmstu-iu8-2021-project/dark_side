// Copyright 2021 Demon1747 Specdok117@mail.ru

#include "database.h"

#include <fstream>
#include <iostream>
#include <odb/connection.hxx>
#include <odb/exception.hxx>
#include <odb/sqlite/database.hxx>
#include <odb/transaction.hxx>

#include "user-odb.hxx"

Database::Database()
    : db(new odb::sqlite::database(
          "users.db", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)) {
  odb::connection_ptr c(db->connection());

  c->execute("PRAGMA foreign_keys=OFF");

  odb::transaction t(c->begin());
  db->execute(
      "CREATE TABLE IF NOT EXISTS Users ("
      "id BIGINT UNSIGNED NOT NULL PRIMARY KEY,"
      "username TEXT NOT NULL,"
      "address TEXT NOT NULL,"
      "port BIGINT UNSIGNED NOT NULL,"
      "key_storage TEXT NOT NULL)");
  t.commit();

  c->execute("PRAGMA foreign_keys=ON");
}

Database::Database(const std::string& db_path)
    : db(new odb::sqlite::database(
          db_path, SQLITE_OPEN_READWRITE)) {  // SQLITE_OPEN_READONLY)) {
  std::fstream db_file(db_path);
  if (!db_file) {
    std::cout << "Unable to open database file! Check for users.db"
              << std::endl;
    exit(EXIT_FAILURE);
  }
}

void Database::insert_user(User& input) {
  try {
    odb::transaction t(db->begin());
    db->persist(input);
    t.commit();
  } catch (const odb::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

User Database::extract_user_by_un(const std::string& in_u_name) const {
  try {
    odb::transaction t(db->begin());
    std::unique_ptr<User> out_user(
        db->query_one<User>(odb::query<User>::username == in_u_name));
    t.commit();
    if (out_user != nullptr) {
      return *out_user;
    } else {
      throw(std::invalid_argument(
          "There is no such user in database, try again"));
    }
  } catch (const odb::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

User Database::extract_user_by_id(size_t in_u_id) const {
  try {
    odb::transaction t(db->begin());
    std::unique_ptr<User> out_user(
        db->query_one<User>(odb::query<User>::id == in_u_id));
    t.commit();
    if (out_user != nullptr) {
      return *out_user;
    } else {
      throw(std::invalid_argument(
          "There is no such user in database, try again"));
    }
  } catch (const odb::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

void Database::delete_user(size_t del_id) {
  try {
    odb::transaction t(db->begin());
    std::unique_ptr<User> del_user(
        db->query_one<User>(odb::query<User>::id == del_id));
    if (del_user != nullptr) {
      db->erase(*del_user);
    }
    t.commit();
  } catch (const odb::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

void Database::erase() {
  try {
    odb::transaction t(db->begin());
    db->erase_query<User>();
    t.commit();
  } catch (const odb::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

//  void Database::update_user(const User& upd_data) {
//    odb::transaction t(db->begin());
//    std::unique_ptr<User> upd_ptr(db->load<User>(upd_data.ID));
//    if (upd_ptr) {
//      *upd_ptr = upd_data;
//      db->update(*upd_ptr);
//    }
//    t.commit();
//  }

void Database::print_users() const {
  try {
    odb::transaction t(db->begin());
    odb::result<User> list(db->query<User>());
    std::string line(80, '-');

    std::cout << line << std::endl << "Known users: " << std::endl;
    if (list.empty()) {
      std::cout << "nobody" << std::endl << std::endl;
    }
    for (auto i(list.begin()); i != list.end(); ++i) {
      std::cout << line << std::endl;
      std::cout << "User ID: " << i->id() << std::endl
                << "Username: " << i->username() << std::endl;
    }
    std::cout << line << std::endl;
    t.commit();
  } catch (const odb::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

bool Database::empty() const {
  try {
    odb::transaction t(db->begin());
    odb::result<User> list(db->query<User>());
    bool empty = list.empty();
    t.commit();
    return empty;
  } catch (const odb::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
