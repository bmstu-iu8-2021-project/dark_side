// Copyright 2021 Demon1747 Specdok117@mail.ru

#ifndef DARK_SIDE_DATABASE_H
#define DARK_SIDE_DATABASE_H

#include <odb/database.hxx>

#include "user.h"

class Database {
 public:
  Database();
  explicit Database(const std::string& db_path);

  void insert_user(User& input);

  User extract_user_by_un(const std::string& in_u_name) const;
  User extract_user_by_id(size_t in_u_id) const;

  void delete_user(size_t del_id);

  void erase();

  //  void update_user(const User& upd_data);

  void print_users() const;

  bool empty() const;

 private:
  std::shared_ptr<odb::database> db;
};

#endif  // DARK_SIDE_DATABASE_H
