// Copyright 2021 Demon1747 Specdok117@mail.ru

#ifndef DARK_SIDE_CLIENT_H
#define DARK_SIDE_CLIENT_H

#include <boost/asio.hpp>

#include "encrypt.h"
#include "user.h"

using namespace boost::asio;

void signal_handler(const boost::system::error_code &err, int signum);

class Client {
 public:
  Client(const User &sender, const User &receiver, const Database &db,
         const std::shared_ptr<Botan::PKCS8_PrivateKey> &p_key,
         std::string &&file_path);

  void connect();

  bool sec_channel_init();

  void disconnect();

  void send_file();

  std::string filename_parse();

  void write_request(const std::vector<uint8_t> &msg);

  void transmit_data(const std::vector<uint8_t> &buff);

  void read_to_buff();

 private:
  io_context context_;
  ip::tcp::socket sock_;
  ip::tcp::endpoint ep_;
  std::vector<uint8_t> buff_;

  User sender_;
  User receiver_;
  Database db_;
  Secure_channel sec_ch_;

  deadline_timer dc_timer_;
  boost::system::error_code err_c_;
  boost::asio::signal_set sig_;

  std::string file_path_;

  size_t msg_count_;
  size_t already_read_;
  size_t buff_size_ = 81920;
  std::streamsize block_size_ = 40960;
  size_t time_dl_ = 55;
};

#endif  // DARK_SIDE_CLIENT_H
