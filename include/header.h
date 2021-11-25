// Copyright 2021

#ifndef DARK_SIDE_HEADER_H
#define DARK_SIDE_HEADER_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <string>

using namespace boost::asio;

typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;

class Server {};

class Client {};

namespace olc {
namespace net {
template <typename T>
struct message_header {
  T id{};
  uint32_t size = 0;
};

template <typename T>
struct message {
  message_header<T> header{};
  std::vector<uint8_t> body;

  [[nodiscard]] inline size_t size() const {
    return sizeof(message_header<T>) + body.size();
  }
};
}  // namespace net
}  // namespace olc

#endif  // DARK_SIDE_HEADER_H
