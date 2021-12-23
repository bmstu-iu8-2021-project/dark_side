// Copyright 2021 Demon1747 Specdok117@mail.ru

#ifndef DARK_SIDE_USER_H
#define DARK_SIDE_USER_H

#include <odb/core.hxx>
#include <string>

#pragma db object table("Users")
class User {
 public:
  User(){};

  User(size_t id, const std::string& username,
       const std::string& address, size_t port, const std::string& key_storage)
      : id_(id),
        username_(username),
        address_(address),
        port_(port),
        key_storage_(key_storage) {}

  inline size_t id() const { return id_; }
  inline std::string username() const { return username_; }
  inline std::string address() const { return address_; }
  inline size_t port() const { return port_; }
  inline std::string key_storage() const { return key_storage_; }

 private:
  friend class odb::access;

  size_t id_;
  std::string username_;
  std::string address_;  // = "127.0.0.1";
  size_t port_;
  std::string key_storage_;
};

//#pragma db object table(Users)
#pragma db member(User::id_) id

#endif  // DARK_SIDE_USER_H
