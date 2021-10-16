// Copyright 2021

//#define BOOST_ASIO_SEPARATE_COMPILATION

#include <header.h>
#include <iostream>

typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;

void client_session(socket_ptr sock) {
  while (true) {
    char data[512];
    size_t len = sock->read_some(buffer(data));
    if (len > 0) {
      write(*sock, buffer("ok", 2));
    }
  }
}

int main() {

  std::cout << "Connection: close\r\n\r\n";

  //  io_service service;
  //  ip::tcp::endpoint ep(ip::tcp::v4(), 2001);
  //  ip::tcp::acceptor acc(service, ep);
  //  while (true) {
  //    socket_ptr sock(new ip::tcp::socket(service));
  //    acc.accept(*sock);
  //    boost::thread(bind(client_session, sock));
  //  }

  boost::asio::io_service service;
  boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.53"), 2001);
  boost::asio::ip::tcp::socket sock(service);
  sock.connect(ep);

  return 0;
}
