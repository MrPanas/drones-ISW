#include "Server.hpp"
#include "Session.hpp"
#include <exception>
#include <iostream>
#include <memory>
#include <ostream>

Server::Server(std::string host, std::string port, std::string password) : host_(host), port_(port), password_(password) {
    std::cout << "Server created with host_: " << host << ", port_: " << port << ", password_: " << password << std::endl;
}

void Server::start() {
    try {
        net::io_context ioc{1};
        net::executor_work_guard<net::io_context::executor_type> work =
                net::make_work_guard(ioc);

        unsigned short port_num =
                std::stoi(port_); // Ensure this conversion is valid.

        std::cout << "Port: " << port_num << std::endl;

        boost::system::error_code ec; // Define the error_code object
        net::ip::address address = net::ip::make_address(host_, ec);
        if (ec) {
            std::cerr << "Address resolution error: " << ec.message() << std::endl;
            throw std::runtime_error("Failed to resolve host_ address.");
        }

        tcp::endpoint endpoint(address, port_num);
        std::cout << "Endpoint created: " << endpoint << std::endl;

        do_listen(ioc, endpoint);

        ioc.run();

    } catch (std::exception const &e) {
        std::cerr << "Server initialization error: " << e.what() << std::endl;
        throw; // Re-throwing to catch it in main if needed
    }
}

void do_listen(net::io_context &ioc, tcp::endpoint endpoint) {
  static tcp::acceptor acceptor(
      ioc); // Make acceptor static or manage its lifecycle differently

  beast::error_code ec;

  if (!acceptor.is_open()) {
    acceptor.open(endpoint.protocol(), ec);

    if (ec) {
      std::cerr << "Acceptor open error: " << ec.message() << std::endl;
      return;
    }
    acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
      std::cerr << "Set option error: " << ec.message() << std::endl;
      return;
    }

    acceptor.bind(endpoint, ec);
    if (ec) {
      std::cerr << "Bind error: " << ec.message() << " (" << ec.value() << ")"
                << std::endl;
      acceptor.close();
      return;
    }

    acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
      std::cerr << "Listen error: " << ec.message() << std::endl;
      acceptor.close();
      return;
    }
  }

  // Accept a connection
  acceptor.async_accept(
      [&ioc, &acceptor](beast::error_code ec, tcp::socket socket) {
        if (!ec) {
          std::make_shared<http_session>(std::move(socket))->run();
        } else {
          std::cerr << "Async accept error: " << ec.message() << std::endl;
        }
        // Recur only if there are no critical errors
        if (!ec || ec != net::error::operation_aborted) {
          do_listen(ioc, acceptor.local_endpoint());
        } else {
          std::cerr << "Stopping accept due to error: " << ec.message()
                    << std::endl;
        }
      });
}
