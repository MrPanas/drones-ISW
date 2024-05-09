#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class http_session : public std::enable_shared_from_this<http_session> {
public:
  // Take ownership of the socket
  explicit http_session(tcp::socket socket) : socket_(std::move(socket)) {}

  // Start the asynchronous operation
  void run() { do_read(); }

private:
  tcp::socket socket_;
  beast::flat_buffer
      buffer_; // This buffer is used for reading and must be persisted

  void do_read() {
    // Read a request
    auto self = shared_from_this();
    http::async_read(
        socket_, buffer_, request_,
        [self](beast::error_code ec, std::size_t bytes_transferred) {
          if (!ec) {
            self->do_write();
          }
        });
  }

  http::request<http::string_body> request_;
  http::response<http::string_body> response_;

  void do_write() {
    // Construct a response
    response_.version(request_.version());
    response_.keep_alive(request_.keep_alive());
    response_.result(http::status::ok);
    response_.set(http::field::server, "Beast/Boost");
    response_.set(http::field::content_type, "text/html");
    response_.body() = "<html><body><h1>Hello, world!</h1></body></html>";
    response_.prepare_payload(); // Make sure the response is consistent

    // Write the response
    auto self = shared_from_this();
    http::async_write(socket_, response_,
                      [self](beast::error_code ec, std::size_t) {
                        self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                      });
  }
};
