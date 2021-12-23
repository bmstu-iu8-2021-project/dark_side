// Copyright 2021 Demon1747 Specdok117@mail.ru

#ifndef DARK_SIDE_SERVER_H
#define DARK_SIDE_SERVER_H

#include <boost/asio.hpp>
#include <vector>

#include "user.h"
#include "database.h"
#include "encrypt.h"

using namespace boost::asio;

class Server;
typedef boost::shared_ptr<Server> client_ptr;

class Server : public std::enable_shared_from_this<Server> {
 public:
  Server(const User &receiver, const Database &db, const std::string &file_dir,
         std::shared_ptr<Botan::PKCS8_PrivateKey> p_key);

  void connect();

  bool sec_channel_init();

  void write_request(const std::vector<uint8_t> &msg);

  void accept_file();

  void read_safe_data();

  void read_to_buff();

  //  void set_clients_changed() { clients_changed_ = true; }

  bool is_disconnected();

  void disconnect_client();

  ip::tcp::socket &sock();

 private:
  io_context context_;
  ip::tcp::socket sock_;
  std::vector<uint8_t> buff_;

  User sender_;
  User receiver_;  // server
  Database db_;
  Secure_channel sec_ch_;
  std::string file_dir_;

  deadline_timer dc_timer_;
  boost::system::error_code err_c_;

  size_t already_read_;
  size_t msg_count_;
  size_t buff_size_ = 81920;
  std::streamsize block_size_ = 40960 + 32 + 8;
  size_t time_dl_;
  bool disconnect_;
  //  bool clients_changed_;
};

// detach

#endif  // DARK_SIDE_SERVER_H
