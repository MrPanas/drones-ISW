#ifndef HTTP_SESSION_HPP
#define HTTP_SESSION_HPP

#include "../con2db/pgsql.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/verb.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <postgresql/libpq-fe.h>
#include <string>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class http_session : public std::enable_shared_from_this<http_session> {
public:
  explicit http_session(tcp::socket socket);
  void run();

private:
  tcp::socket socket_;
  beast::flat_buffer buffer_; // Buffer for reading, must be persisted
  http::request<http::string_body> request_;
  http::response<http::string_body> response_;

  void do_read();
  void process_request();
  void handle_request_report();
  void send_not_found();
  void do_write();
};

#endif // HTTP_SESSION_HPP
