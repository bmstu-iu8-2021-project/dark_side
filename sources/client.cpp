// Copyright 2021 Demon1747 Specdok117@mail.ru

#include "client.h"

#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "convert.h"
#include "database.h"
#include "log.h"

boost::system::error_code err_c;
using namespace std::chrono;

// std::atomic<bool> program_must_terminate;
//
// void signal_handler(const boost::system::error_code &err, int signum) {
//   if (signum == 2) {
//     program_must_terminate.store(true);
//   }
//   exit(signum);
// }

Client::Client(const User &sender, const User &receiver, const Database &db,
               const std::shared_ptr<Botan::PKCS8_PrivateKey> &p_key,
               std::string &&file_path)
    : sock_(context_),
      ep_(ip::address::from_string(receiver.address()), receiver.port()),
      dc_timer_(context_),
      sender_(sender),
      receiver_(receiver),
      db_(db),
      file_path_(file_path),
      sec_ch_(p_key),
      time_dl_(5),
      msg_count_(1),
      already_read_(0) {
  //  sig_.async_wait(signal_handler);
}

void Client::connect() { sock_.connect(ep_); }

bool Client::sec_channel_init() {
  try {
    write_request(sec_ch_.send_handshake(sender_, receiver_));
    read_to_buff();
    sec_ch_.receive_handshake(buff_, receiver_, sender_, db_);
    write_request(sec_ch_.send_init_data());
    read_to_buff();
    return sec_ch_.finalise_protocol(buff_, "sender");
  } catch (std::exception &err) {
    BOOST_LOG_TRIVIAL(error) << err.what();
    return false;
  }
}

void Client::disconnect() { sock_.close(err_c_); }

void Client::send_file() {
  std::ifstream in_f(file_path_, std::ios::in | std::ios::binary);
  if (!in_f) {
    std::cerr << "there is no such file" << std::endl;
    return;
  }

  in_f.seekg(0, std::ios::end);
  size_t file_size = in_f.tellg();
  in_f.seekg(0, std::ios::beg);

  size_t num_of_blocks(file_size / block_size_ + 1);

  /// Transmitting file information
  transmit_safe_data(
      str_to_bytes(std::to_string(num_of_blocks) + ' ' + filename_parse()));

  std::this_thread::sleep_for(1s);

  /// transmitting file
  std::string file_buff;
  file_buff.resize(block_size_);
  while (!in_f.eof()) {
    if (file_size < block_size_) {
      file_buff.resize(file_size);
    }
    in_f.read(&file_buff[0], block_size_);

    transmit_safe_data(str_to_bytes(file_buff));
    file_size -= block_size_;
  }

  BOOST_LOG_TRIVIAL(info) << "File was transmitted to " << receiver_.username();
}

std::string Client::filename_parse() {
  std::string result(file_path_);
  while (result.find('/') != std::string::npos) {
    result.erase(0, result.find('/') + 1);
  }
  return result;
}

void Client::write_request(const std::vector<uint8_t> &msg) {
  sock_.write_some(buffer(msg));
}

void Client::transmit_safe_data(const std::vector<uint8_t> &buff) {
  std::vector<uint8_t> safe_data = sec_ch_.encipher_buff(buff, msg_count_);
  sock_.write_some(buffer(safe_data));
  ++msg_count_;
}

void Client::read_to_buff() {
  dc_timer_.expires_from_now(boost::posix_time::seconds(time_dl_));
  while (dc_timer_.expires_at() > deadline_timer::traits_type::now()) {
    if (sock_.available()) {
      buff_.resize(buff_size_);
      already_read_ = sock_.read_some(buffer(buff_));
      buff_.resize(already_read_);
      return;
    }
  }

  throw boost::system::system_error(err_c_, "Reading buffer error");
}
