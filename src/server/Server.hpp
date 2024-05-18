#ifndef DRONE8_SERVER_HPP
#define DRONE8_SERVER_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <iostream>
#include <postgresql/libpq-fe.h>
#include <string>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

template <class Body, class Allocator, class Send>
void handle_request(http::request<Body, http::basic_fields<Allocator>> &&req,
                    Send &&send);

void do_listen(net::io_context &ioc, tcp::endpoint endpoint);

class Server {
public:
  // Il costruttore crea crea il server, crea un sockert sul host_
  Server(std::string host, std::string port, std::string password);
  void start();

private:
  PGconn *conn{};
  std::string host_;
  std::string port_;
  std::string password_;
  int socket{};
};

#endif // DRONE8_SERVER_HPP
