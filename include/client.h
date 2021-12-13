//

#ifndef DARK_SIDE_CLIENT_H
#define DARK_SIDE_CLIENT_H

#include <header.h>
#include <database.h>

namespace olc {
namespace net {
template <typename T>
class connection : public std::enable_shared_from_this<connection<T>> {
 public:
  enum class owner { server, client };

  connection(owner parent, io_context& context, ip::tcp::socket sock,
             tsqueue<owned_message<T>>& q_in)
      : m_asio_context_(context),
        m_socket_(std::move(sock)),
        m_q_msg_in_(q_in) {
    m_owner_type_ = parent;
  }

  virtual ~connection() {}

  uint32_t getID() const { return id; }

  void connect_to_client(uint32_t u_id = 0) {
    if (m_owner_type_ == owner::server) {
      if (m_socket_.is_open()) {
        id = u_id;
        read_header();
      }
    }
  }

  void connect_to_server(const ip::tcp::resolver::results_type& endpoints) {
    if (m_owner_type_ == owner::client) {
      async_connect(m_socket_, endpoints,
                    [this](std::error_code ec, ip::tcp::endpoint endpoint) {
                      if (!ec) {
                        read_header();
                      }
                    });
    }
  }

  void disconnect() {
    if (is_connected()) {
      post(m_asio_context_, [this]() { m_socket_.close(); });
    }
  }

  bool is_connected() const { return m_socket_.is_open(); }

  void send_m(const message<T>& msg) {
    m_socket_.write_some(buffer(msg));

//    post(m_asio_context_, [this, msg]() {
//      bool writing_message = !m_q_msg_out_.empty();
//      m_q_msg_out_.push_back(msg);
//      if (!writing_message) {
//        write_header();
//      }
//    });
  }

 private:
  void read_header() {
    async_read(
        m_socket_,
        buffer(&m_msg_temporary_in_.header, sizeof(message_header<T>)),
        [this](std::error_code ec, size_t length) {
          if (!ec) {
            if (m_msg_temporary_in_.header.size > 0) {
              m_msg_temporary_in_.body.resize(m_msg_temporary_in_.header.size);
              read_body();
            } else {
              add_to_incoming_message_queue();
            }
          } else {
            std::cout << "[" << id << "] Read header fail." << std::endl;
            m_socket_.close();
          }
        });
  }

  void read_body() {
    async_read(m_socket_,
               buffer(m_msg_temporary_in_.body.data(),
                      m_msg_temporary_in_.body.size()),
               [this](std::error_code ec, size_t length) {
                 if (!ec) {
                   add_to_incoming_message_queue();
                 } else {
                   std::cout << "[" << id << "] Read body fail." << std::endl;
                   m_socket_.close();
                 }
               });
  }

  void write_header() {
    async_write(m_socket_,
                buffer(&m_q_msg_out_.front().header, sizeof(message_header<T>)),
                [this](std::error_code ec, size_t length) {
                  if (!ec) {
                    if (m_q_msg_out_.front().body.size() > 0) {
                      write_body();
                    } else {
                      m_q_msg_out_.pop_front();
                      if (!m_q_msg_out_.empty()) {
                        write_header();
                      }
                    }

                  } else {
                    std::cout << "[" << id << "] Write header fail."
                              << std::endl;
                    m_socket_.close();
                  }
                });
  }

  void write_body() {
    async_write(m_socket_,
                buffer(&m_q_msg_out_.front().header, sizeof(message_header<T>)),
                [this](std::error_code ec, size_t length) {
                  if (!ec) {
                    m_q_msg_out_.pop_front();
                    if (!m_q_msg_out_.empty()) {
                      write_header();
                    }
                  } else {
                    std::cout << "[" << id << "] Write body fail." << std::endl;
                    m_socket_.close();
                  }
                });
  }

  void add_to_incoming_message_queue() {
    if (m_owner_type_ == owner::server) {
      m_q_msg_in_.push_back({this->shared_from_this(), m_msg_temporary_in_});
    } else {
      m_q_msg_in_.push_back({nullptr, m_msg_temporary_in_});
    }
  }

 protected:
  ip::tcp::socket m_socket_;

  io_context& m_asio_context_;

  // Holds all messages to be sent to the remote side of this connection
  tsqueue<message<T>> m_q_msg_out_;

  // Holds all messages that have been received from the remote side of this
  // connection/ Note it is a reference as the "owner" of this connection is
  // expected to provide a queue
  tsqueue<owned_message<T>>& m_q_msg_in_;
  message<T> m_msg_temporary_in_;

  owner m_owner_type_ = owner::server;
  uint32_t id = 0;
};

////////////////Client/////////////////////////////////////////////////////////

template <typename T>
class client_interface
{
 public:
  client_interface()
  {}

  virtual ~client_interface()
  {
    // If the client is destroyed, always try and disconnect from server
    Disconnect();
  }

 public:
  // Connect to server with hostname/ip-address and port
  bool Connect(const std::string& host, const uint16_t port)
  {
    try
    {
      // Resolve hostname/ip-address into tangiable physical address
      ip::tcp::resolver resolver(m_context);
      ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

      // Create connection
      m_connection = std::make_unique<connection<T>>(connection<T>::owner::client, m_context, ip::tcp::socket(m_context), m_qMessagesIn);

      // Tell the connection object to connect to server
      m_connection->connect_to_server(endpoints);

      // Start Context Thread
      thrContext = std::thread([this]() { m_context.run(); });
    }
    catch (std::exception& e)
    {
      std::cerr << "Client Exception: " << e.what() << "\n";
      return false;
    }
    return true;
  }

  // Disconnect from server
  void Disconnect()
  {
    // If connection exists, and it's connected then...
    if(IsConnected())
    {
      // ...disconnect from server gracefully
      m_connection->disconnect();
    }

    // Either way, we're also done with the asio context...
    m_context.stop();
    // ...and its thread
    if (thrContext.joinable())
      thrContext.join();

    // Destroy the connection object
    m_connection.release();
  }

  // Check if client is actually connected to a server
  bool IsConnected()
  {
    if (m_connection)
      return m_connection->is_connected();
    else
      return false;
  }

 public:
  // Send message to server
  void Send(const message<T>& msg)
  {
    if (IsConnected())
      m_connection->send_m(msg);
  }

  // Retrieve queue of messages from server
  tsqueue<owned_message<T>>& Incoming()
  {
    return m_qMessagesIn;
  }

 protected:
  // asio context handles the data transfer...
  io_context m_context;
  // ...but needs a thread of its own to execute its work commands
  std::thread thrContext;
  // The client has a single instance of a "connection" object, which handles data transfer
  std::unique_ptr<connection<T>> m_connection;

 private:
  // This is the thread safe queue of incoming messages from server
  tsqueue<owned_message<T>> m_qMessagesIn;
};


}  // namespace net
}  // namespace olc

#endif  // DARK_SIDE_CLIENT_H
