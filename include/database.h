// Copyright 2021 Demon1747 Specdok117@mail.ru

#ifndef DARK_SIDE_DATABASE_H
#define DARK_SIDE_DATABASE_H

#include <memory>

#include "odb/connection.hxx"
#include "odb/database.hxx"
#include "odb/schema-catalog.hxx"
#include "odb/sqlite/database.hxx"
#include "odb/transaction.hxx"
#include "user-odb.hxx"
//#include "user.h"

class Database {
 public:
  explicit Database(const std::string& db_path)
      : db(new odb::sqlite::database(
            db_path,  //"/home/demon/CLionProjects/dark_side/data/users.db",
            SQLITE_OPEN_READWRITE |
                SQLITE_OPEN_CREATE)) {  // SQLITE_OPEN_READONLY

    odb::connection_ptr c(db->connection());

    c->execute("PRAGMA foreign_keys=OFF");

    odb::transaction t(c->begin());
    db->execute(
        "CREATE TABLE IF NOT EXISTS Users ("
        "id BIGINT UNSIGNED NOT NULL PRIMARY KEY,"
        "status TEXT NOT NULL,"
        "username TEXT NOT NULL,"
        "password TEXT NOT NULL,"
        "address TEXT NOT NULL,"
        "port BIGINT UNSIGNED NOT NULL)");
    t.commit();

    c->execute("PRAGMA foreign_keys=ON");
  }

  void insert_user(User& input) {
    try {
      odb::transaction t(db->begin());
      db->persist(input);
      t.commit();
    } catch (const odb::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }

  User extract_user(const std::string& in_u_name) {
    try {
      odb::transaction t(db->begin());
      std::unique_ptr<User> out_user(
          db->query_one<User>(odb::query<User>::username == in_u_name));
      t.commit();
      if (out_user != nullptr) {
        return *out_user;
      } else {
        throw(std::invalid_argument("Incorrect username, see you next time!"));
      }
    } catch (const odb::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }

  void delete_user(const std::string& in_u_name) {
    try {
      odb::transaction t(db->begin());
      std::unique_ptr<User> del_user(
          db->query_one<User>(odb::query<User>::username == in_u_name));
      if (del_user != nullptr) {
        db->erase(*del_user);
      }
      t.commit();
    } catch (const odb::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }

  //  void update_user(const User& upd_data) {
  //    odb::transaction t(db->begin());
  //    std::unique_ptr<User> upd_ptr(db->load<User>(upd_data.ID));
  //    if (upd_ptr) {
  //      *upd_ptr = upd_data;
  //      db->update(*upd_ptr);
  //    }
  //    t.commit();
  //  }

  void print_users() {
    try {
      odb::transaction t(db->begin());
      odb::result<User> list(db->query<User>());
      std::string line(80, '-');
      std::cout << "Known users: " << std::endl;
      for (auto i(list.begin()); i != list.end(); ++i) {
        std::cout << line << std::endl;
        std::cout << "User ID: " << i->id() << std::endl
                  << "Username: " << i->username() << std::endl;
        std::cout << line << std::endl;
      }
      t.commit();
    } catch (const odb::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }

 private:
  std::unique_ptr<odb::database> db;
};

#endif  // DARK_SIDE_DATABASE_H
