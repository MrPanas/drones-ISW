#include "Server.hpp"
#include "Session.cpp"
#include <exception>
#include <memory>

Server::Server(std::string host, std::string port, std::string password) {
  try {
    net::io_context ioc{1};
    unsigned short port_num = std::stoi(port);

    tcp::endpoint endpoint(net::ip::make_address(host), port_num);

    do_listen(ioc, endpoint);
  } catch (std::exception const &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}

void do_listen(net::io_context &ioc, tcp::endpoint endpoint) {

  beast::error_code ec;

  // Open the acceptor
  tcp::acceptor acceptor{ioc};
  acceptor.open(endpoint.protocol(), ec);
  if (ec) {
    std::cerr << "Error: " << ec.message() << std::endl;
    return;
  }

  // Bind to the server address
  acceptor.bind(endpoint, ec);
  if (ec) {
    std::cerr << "Error: " << ec.message() << std::endl;
    return;
  }

  // Start listening for connections
  acceptor.listen(net::socket_base::max_listen_connections, ec);
  if (ec) {
    std::cerr << "Error: " << ec.message() << std::endl;
    return;
  }

  // Accept a connection
  acceptor.async_accept(
      [&ioc, &acceptor](beast::error_code ec, tcp::socket socket) {
        if (!ec) {
          // Assuming http::session is defined and takes a tcp::socket in its
          // constructor
          std::make_shared<http_session>(std::move(socket))->run();
        }
        // Continue accepting connections
        do_listen(ioc, acceptor.local_endpoint());
      });
}
