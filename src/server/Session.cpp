#include "Session.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <ostream>
#include <postgresql/libpq-fe.h>

using tcp = boost::asio::ip::tcp;    // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>
namespace beast = boost::beast;      // from <boost/beast/core.hpp>
using json = nlohmann::json;

http_session::http_session(tcp::socket socket) : socket_(std::move(socket)) {}

void http_session::run() { do_read(); }

void http_session::do_read() {
  auto self = shared_from_this();
  auto parser = std::make_shared<http::request_parser<http::string_body>>();
  parser->body_limit(1024 * 1024 * 100); // Set body limit to 100 MB

  http::async_read(
      socket_, buffer_, *parser,
      [self, parser](beast::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
          self->request_ = parser->release();
          self->process_request();
        } else {
          std::cerr << "Read error: " << ec.message() << std::endl;
          if (ec == http::error::body_limit) {
            std::cerr << "Body limit exceeded!" << std::endl;
          }
        }
      });
}

void http_session::process_request() {
  if (request_.method() == http::verb::post && request_.target() == "/report") {
    handle_request_report();
  } else if (request_.method() == http::verb::get &&
             request_.target() == "/report") {

    handle_return_report();

  } else {
    send_not_found();
  }
}

void http_session::handle_return_report() {
  auto request_json = json::parse(request_.body());
  auto cc_id = request_json.at("cc-id").get<int>();

  Con2DB db("localhost", "5432", "postgres", "postgres", "postgres");
  char sqlcmd[512];
  snprintf(sqlcmd, sizeof(sqlcmd),
           "SELECT image_url FROM report_image WHERE cc_id = %d ORDER BY "
           "image_id DESC LIMIT 1",
           cc_id);

  PGresult *res = db.ExecSQLtuples(sqlcmd);
  if (PQntuples(res) > 0) {
    char *image_url = PQgetvalue(res, 0, PQfnumber(res, "image_url"));
    std::string full_path = image_url;

    PQclear(res);

    // Open CSV file for reading
    std::ifstream csv_file(full_path);
    if (!csv_file.is_open()) {
      throw std::runtime_error("Failed to open CSV file for reading");
    }

    // Read the CSV file content
    std::stringstream buffer;
    buffer << csv_file.rdbuf();
    std::string csv_content = buffer.str();
    csv_file.close();

    // Set up the HTTP response
    response_.version(request_.version());
    response_.keep_alive(request_.keep_alive());
    response_.result(http::status::ok);
    response_.set(http::field::server, "Beast/Boost");
    response_.set(http::field::content_type, "text/csv");
    response_.body() = csv_content;
    response_.prepare_payload();
    do_write();
  } else {
    PQclear(res);
    send_not_found();
  }
}

void http_session::handle_request_report() {

  auto request_json = json::parse(request_.body());

  auto grid_last_visited =
      request_json.at("area").get<std::vector<std::vector<long>>>();
  auto cc_id = request_json.at("cc-id").get<int>();

  int last_inserted_image_id = 0;
  Con2DB db("localhost", "5432", "postgres", "postgres", "postgres");
  char sqlcmd[512];
  snprintf(
      sqlcmd, sizeof(sqlcmd),
      "SELECT max(image_id) AS image_id FROM report_image WHERE cc_id = %d",
      cc_id);

  PGresult *res = db.ExecSQLtuples(sqlcmd);

  if (PQntuples(res) > 0) {
    char *image_id_str = PQgetvalue(res, 0, PQfnumber(res, "image_id"));
    if (image_id_str != NULL) {
      last_inserted_image_id = strtol(image_id_str, NULL, 10);
    }
  }
  PQclear(res);
  std::ostringstream csv_file_path;
  csv_file_path << "./area_snapshots/report_" << cc_id << "image_"
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

  snprintf(sqlcmd, sizeof(sqlcmd),
           "INSERT INTO report_image (cc_id, image_url) VALUES (%d, '%s')",
           cc_id, csv_file_path.str().c_str());

  res = db.ExecSQLcmd(sqlcmd);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    PQclear(res);
    throw std::runtime_error("Failed to insert record into report_image table");
  }
  PQclear(res);

  response_.version(request_.version());
  response_.keep_alive(request_.keep_alive());
  response_.result(http::status::created);
  response_.set(http::field::server, "Beast/Boost");
  response_.set(http::field::content_type, "application/json");
  response_.body() = R"({"message": "CSV file created successfully"})";
  response_.prepare_payload();
  do_write();
}

void http_session::send_not_found() {
  std::cout << "Sending not found response" << std::endl;
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
  std::cout << "Writing response" << std::endl;
  http::async_write(
      socket_, response_,
      [self](beast::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
          std::cout << "Response written successfully" << std::endl;
          // Gracefully close the socket
          beast::error_code ec;
          self->socket_.shutdown(tcp::socket::shutdown_send, ec);
          if (ec) {
            std::cerr << "Shutdown error: " << ec.message() << std::endl;
          }
        } else {
          std::cerr << "Write error: " << ec.message() << std::endl;
        }
      });
}
