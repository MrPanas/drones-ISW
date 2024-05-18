#include "Session.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using tcp = boost::asio::ip::tcp;    // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>
namespace beast = boost::beast;      // from <boost/beast/core.hpp>
using json = nlohmann::json;

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

void http_session::process_request() {
  if (request_.method() == http::verb::post && request_.target() == "/report") {
    handle_request_report();
  } else {
    send_not_found();
  }
}

// TODO: pass cc_id
void http_session::handle_request_report() {
  try {
    auto request_json = json::parse(request_.body());

    auto grid_last_visited =
        request_json.at("area").get<std::vector<std::vector<long>>>();

    auto cc_id = request_json.at("cc-id").get<int>();

    Con2DB db("localhost", "5432", "postgres", "postgres", "postgres");
    char sqlcmd[512];
    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "SELECT cc_id, max(id) AS image_id FROM report_image GROUP BY cc_id");
    PGresult *res = db.ExecSQLtuples(sqlcmd);

    int last_inserted_image_id = 0;

    if (PQntuples(res) > 0) {
      int last_inserted_image_id =
          strtol(PQgetvalue(res, 0, PQfnumber(res, "image_id")), NULL, 10);
    }
    PQclear(res);

    std::ostringstream csv_file_path;
    csv_file_path << "/images/report_" << cc_id << "image_"
                  << last_inserted_image_id << ".csv";

    // Open CSV file for writing
    std::ofstream csv_file(csv_file_path.str());
    if (!csv_file.is_open()) {
      throw std::runtime_error("Failed to open CSV file for writing");
    }

    // Write the array to the CSV file
    for (const auto &row : grid_last_visited) {
      for (size_t i = 0; i < row.size(); ++i) {
        csv_file << row[i];
        if (i != row.size() - 1) {
          csv_file << ",";
        }
      }
      csv_file << "\n";
    }
    csv_file.close();

    // Set up the HTTP response
    response_.version(request_.version());
    response_.keep_alive(request_.keep_alive());
    response_.result(http::status::created);
    response_.set(http::field::server, "Beast/Boost");
    response_.set(http::field::content_type, "application/json");
    response_.body() = R"({"message": "CSV file created successfully"})";
    response_.prepare_payload();
    do_write();
  } catch (const std::exception &e) {
    // Handle JSON parsing or file writing errors
    response_.version(request_.version());
    response_.keep_alive(request_.keep_alive());
    response_.result(http::status::internal_server_error);
    response_.set(http::field::server, "Beast/Boost");
    response_.set(http::field::content_type, "application/json");
    response_.body() = R"({"error": ")" + std::string(e.what()) + R"("})";
    response_.prepare_payload();
    do_write();
  }
}

void http_session::send_not_found() {
  response_.version(request_.version());
  response_.keep_alive(request_.keep_alive());
  response_.result(http::status::not_found);
  response_.set(http::field::server, "Beast/Boost");
  response_.set(http::field::content_type, "application/json");
  response_.body() = R"({"error": "Not Found"})";
  response_.prepare_payload();
  do_write();
}

void http_session::do_write() {
  auto self = shared_from_this();
  http::async_write(
      socket_, response_,
      [self](beast::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
          // Gracefully close the socket
          beast::error_code ec;
          self->socket_.shutdown(tcp::socket::shutdown_send, ec);
        } else {
          std::cerr << "Write error: " << ec.message() << std::endl;
        }
      });
}
