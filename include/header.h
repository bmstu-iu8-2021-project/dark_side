// Copyright 2021

#ifndef DARK_SIDE_HEADER_H
#define DARK_SIDE_HEADER_H

//#include <boost/asio.hpp>
//#include <boost/bind.hpp>
//#include <boost/thread.hpp>
//#include <string>
//
//#include "user_interface.h"
//
//using namespace boost::asio;
//
//typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;
//
//class Server {};
//
//class Client {};
//
//struct message_buf {
//  int msg_num; // 2^32 - 1 max
//  int data_length; // in bytes
//  std::string data;
//
//};
//
//namespace olc {
//namespace net {
//template <typename T>
//struct message_header {
//  T id{};
//  uint32_t size = 0;
//};
//
//template <typename T>
//struct message {
//  message_header<T> header{};
//  std::vector<uint8_t> body;
//
//  [[nodiscard]] inline size_t size() const {
//    return sizeof(message_header<T>) + body.size();
//  }
//
//  // Override for std::cout compatibility - friendly description of message
//  friend std::ostream& operator<<(std::ostream& os, const message<T>& msg) {
//    os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
//    return os;
//  }
//
//  template <typename DataT>
//  friend message<T>& operator<<(message<T>& msg, const DataT& data) {
//    static_assert(std::is_standard_layout<DataT>::value,
//                  "Data is too complex to be pushed");
//
//    size_t size = msg.body.size();
//
//    msg.body.resize(msg.body.size() + sizeof(DataT));
//
//    std::memcpy(msg.body.data() + size, &data, sizeof(DataT));
//
//    msg.header.size = msg.size();
//
//    return msg;
//  }
//
//  template <typename DataT>
//  friend message<T>& operator>>(message<T>& msg, DataT& data) {
//    static_assert(std::is_standard_layout<DataT>::value,
//                  "Data is too complex to be pushed");
//
//    size_t size = msg.body.size() - sizeof(DataT);
//
//    std::memcpy(&data, msg.body.data() + size, sizeof(DataT));
//
//    msg.body.resize(msg.body.size() - sizeof(DataT));
//
//    msg.header.size = msg.size();
//
//    return msg;
//  }
//};
//
//template <typename T>
//class tsqueue {
// public:
//  tsqueue() = default;
//  tsqueue(const tsqueue<T>&) = delete;
////  virtual ~tsqueue() { clear(); }
//
////  const T& front() {
////    std::scoped_lock lock(mux_queue);
////    return deq_queue.front();
////  }
////
////  const T& back() {
////    std::scoped_lock lock(mux_queue);
////    return deq_queue.back();
////  }
////
////  void push_back(const T& item) {
////    std::scoped_lock lock(mux_queue);
////    deq_queue.template emplace_back(std::move(item));
////  }
////
////  void push_front(const T& item) {
////    std::scoped_lock lock(mux_queue);
////    deq_queue.template emplace_front(std::move(item));
////  }
////
////  bool empty() {
////    std::scoped_lock lock(mux_queue);
////    return deq_queue.empty();
////  }
////
////  T pop_front() {
////    std::scoped_lock lock(mux_queue);
////    auto t = std::move(deq_queue.front());
////    deq_queue.pop_front();
////    return t;
////  }
////
////  T pop_back() {
////    std::scoped_lock lock(mux_queue);
////    auto t = std::move(deq_queue.back());
////    deq_queue.pop_back();
////    return t;
////  }
////
////  size_t count() {
////    std::scoped_lock lock(mux_queue);
////    return deq_queue.size();
////  }
////
////  void clear() {
////    std::scoped_lock lock(mux_queue);
////    deq_queue.clear();
////  }
//
// protected:
//  std::mutex mux_queue;
//  std::deque<T> deq_queue;
//};
//
//template <typename T>
//class connection;
//
//template <typename T>
//struct owned_message {
//  std::shared_ptr<connection<T>> remote = nullptr;
//  message<T> msg;
//
//  friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg) {
//    os << msg.msg;
//    return os;
//  }
//};
//
//
//}  // namespace net
//}  // namespace olc

#endif  // DARK_SIDE_HEADER_H
