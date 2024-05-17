#include "Session.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <list>
#include <ostream>
#include <postgresql/libpq-fe.h>
#include <string>

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

    // TODO:
    // retrieve    @var cc_id   ;  @var timestamp
    //
    handle_request_report();
  } else {
    send_not_found();
  }
}

class PathException : public std::exception {
private:
  std::string message;

public:
  PathException(const std::string &msg) : message(msg) {}

  virtual const std::string what() throw() { return message.c_str(); }
};

struct path_drone_scann_init {
  int path_id;
  int drone_id;
  std::string scan_init_time;

  path_drone_scann_init(int p_id, int d_id, const std::string &time)
      : path_id(p_id), drone_id(d_id), scan_init_time(time) {}
};

void get_most_recent_paths(
    std::list<path_drone_scann_init> &paths_with_init_times) {

  Con2DB db("localhost", "5432", "postgres", "postgres", "postgres");
  char sqlcmd[512];

  snprintf(sqlcmd, sizeof(sqlcmd),
           "SELECT path.path_id, dl.drone_id, MAX(dl.time) AS max_time "
           "FROM path "
           "LEFT JOIN drone_log AS dl ON path.path_id = dl.path_id AND "
           "dl.status = 'SCANNING' "
           "GROUP BY path.path_id, dl.drone_id ORDER BY path.path_id");

  PGresult *res = db.ExecSQLtuples(sqlcmd);

  for (int i = 0; i < PQntuples(res); i++) {
    int path_id =
        strtol(PQgetvalue(res, i, PQfnumber(res, "path_id")), NULL, 10);
    int drone_id =
        strtol(PQgetvalue(res, i, PQfnumber(res, "drone_id")), NULL, 10);
    const char *time_scan_str = PQgetvalue(res, i, PQfnumber(res, "max_time"));

    if (time_scan_str == NULL || strlen(time_scan_str) == 0) {
      throw PathException("Path with id : " + std::to_string(path_id) +
                          " has not drone");
    } else {
      paths_with_init_times.push_back(
          path_drone_scann_init(path_id, drone_id, std::string(time_scan_str)));
    }
  }
}
/**
 * @var int cc_id
 * @var timestamp map_timestamp
 */
void http_session::handle_request_report() {

  std::list<path_drone_scann_init> paths;

  try {
    get_most_recent_paths(paths);
  } catch (PathException &pe) {
    std::cerr << pe.what() << std::endl;
  }
  for (const auto &path : paths) {
    std::cout << "Path ID: " << path.path_id << ", Drone ID: " << path.drone_id
              << ", Latest Scan Time: " << path.scan_init_time << std::endl;
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
