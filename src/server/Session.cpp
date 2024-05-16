#include "Session.hpp"
#include <cstdio>
#include <iostream>
#include <ostream>
#include <postgresql/libpq-fe.h>

http_session::http_session(tcp::socket socket) : socket_(std::move(socket)) {}

void http_session::run() { do_read(); }

void http_session::do_read() {
  auto self = shared_from_this();
  http::async_read(socket_, buffer_, request_,
                   [self](beast::error_code ec, std::size_t bytes_transferred) {
                     if (!ec) {
                       self->process_request();
                     } else {
                       std::cerr << "Read error: " << ec.message() << std::endl;
                     }
                   });
}

// Process the HTTP request
void http_session::process_request() {
  if (request_.method() == http::verb::post && request_.target() == "/report") {
    handle_request_report();
  } else {
    send_not_found();
  }
}

// Handle specific request to generate a report
void http_session::handle_request_report() {

  try {
    Con2DB db("localhost", "5432", "postgres", "postgres", "postgres");

    char sqlcmd[256];
    sprintf(sqlcmd, "SELECT * FROM drone");

    PGresult *res = db.ExecSQLtuples(sqlcmd);

    if (res != nullptr) {
      int rows = PQntuples(res);
      std::cout << "We have retrieved " << rows << " rows" << std::endl;
      PQclear(res); // Clear the result object
    } else {
      std::cerr << "Failed to execute the SQL command." << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  response_.version(request_.version());
  response_.keep_alive(request_.keep_alive());
  response_.result(http::status::created);
  response_.set(http::field::server, "Beast/Boost");
  response_.set(http::field::content_type, "text/html");
  response_.body() =
      "<html><body><h1>Report generated successfully!</h1></body></html>";
  response_.prepare_payload();
  do_write();
}

// Send a 404 Not Found response
void http_session::send_not_found() {
  response_.version(request_.version());
  response_.keep_alive(request_.keep_alive());
  response_.result(http::status::not_found);
  response_.set(http::field::server, "Beast/Boost");
  response_.set(http::field::content_type, "text/html");
  response_.body() = "<html><body><h1>Error 404: Not Found</h1></body></html>";
  response_.prepare_payload();
  do_write();
}

// Write the response back to the client
void http_session::do_write() {
  auto self = shared_from_this();
  http::async_write(
      socket_, response_, [self](beast::error_code ec, std::size_t) {
        if (!ec) {
          self->socket_.shutdown(tcp::socket::shutdown_send, ec);
          if (ec) {
            std::cerr << "Shutdown error: " << ec.message() << std::endl;
          }
        } else {
          std::cerr << "Write error: " << ec.message() << std::endl;
        }
      });
}
