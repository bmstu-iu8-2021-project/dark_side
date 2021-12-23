// Copyright 2021 Demon1747 Specdok117@mail.ru

#include "server.h"

#include <fstream>
#include <iostream>
#include <utility>

#include "convert.h"

Server::Server(const User &receiver, const Database &db,
               const std::string &file_dir,
               std::shared_ptr<Botan::PKCS8_PrivateKey> p_key)
    : sock_(context_),
      dc_timer_(context_),
      receiver_(receiver),
      db_(db),
      file_dir_(file_dir),
      sec_ch_(std::move(p_key)),
      msg_count_(1),
      already_read_(0),
      disconnect_(false) {}

void Server::connect() {
  try {
    if (sec_channel_init()) {
      accept_file();
    }

    disconnect_client();
  } catch (boost::system::system_error &err_c) {
    disconnect_client();
    std::cerr << "haha loh" << std::endl;
  }
}

bool Server::sec_channel_init() {
  try {
    read_to_buff();
    sec_ch_.receive_handshake(buff_, sender_, receiver_, db_);
    write_request(sec_ch_.send_handshake(receiver_, sender_));
    write_request(sec_ch_.send_init_data());
    read_to_buff();
    return sec_ch_.finalise_protocol(buff_, "receiver");
  } catch (std::exception &err) {
    std::cout << err.what() << std::endl;
    return false;
  }
}

void Server::write_request(const std::vector<uint8_t> &msg) {
  sock_.write_some(buffer(msg), err_c_);
  // throw boost::system::system_error(err_c_, __func__);
}

void Server::accept_file() {
  read_to_buff();

  std::istringstream in(bytes_to_str(sec_ch_.encipher_buff(buff_, msg_count_)));
  ++msg_count_;

  size_t blocks_num = 0;
  std::string filename;
  in >> blocks_num >> filename;

  std::ofstream out_f(file_dir_ + filename, std::ofstream::binary);

  buff_.resize(buff_size_);

  for (size_t i = 0; i < blocks_num; ++i, ++msg_count_) {
    read_to_buff();
    std::string block_data(
        bytes_to_str(sec_ch_.encipher_buff(buff_, msg_count_)));
    out_f.write(&block_data[0], block_data.size());
  }

  std::cout << "File from " << sender_.username() << " was accepted"
            << std::endl;
}

void Server::read_to_block() {
  dc_timer_.expires_from_now(boost::posix_time::seconds(time_dl_));
  while (dc_timer_.expires_at() > deadline_timer::traits_type::now()) {
    if (sock_.available()) {
      buff_.resize(block_size_);
      already_read_ = sock_.read_some(buffer(buff_, block_size_));
      buff_.resize(already_read_);
      return;
    }
  }
  throw boost::system::system_error(err_c_, "Timed out!");
}

void Server::read_to_buff() {
  dc_timer_.expires_from_now(boost::posix_time::seconds(time_dl_));
  while (dc_timer_.expires_at() > deadline_timer::traits_type::now()) {
    if (sock_.available()) {
      buff_.resize(buff_size_);
      already_read_ = sock_.read_some(buffer(buff_));
      buff_.resize(already_read_);
      return;
    }
  }
  throw boost::system::system_error(err_c_, __func__);
}

//  void set_clients_changed() { clients_changed_ = true; }

bool Server::is_disconnected() { return disconnect_; }

void Server::disconnect_client() {
  disconnect_ = true;
  sock_.close(err_c_);
}

ip::tcp::socket &Server::sock() { return sock_; }
