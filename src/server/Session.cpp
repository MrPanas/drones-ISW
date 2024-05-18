#include "Session.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <ostream>

using tcp = boost::asio::ip::tcp;    // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>
namespace beast = boost::beast;      // from <boost/beast/core.hpp>
using json = nlohmann::json;

http_session::http_session(tcp::socket socket) : socket_(std::move(socket)) {}

void http_session::run() {
  std::cout << "Running session" << std::endl;
  do_read();
}

void http_session::do_read() {
  std::cout << "Do read" << std::endl;
  auto self = shared_from_this();
  std::cout << "After creating self" << std::endl;
  http::request_parser<http::string_body> parser;
  parser.body_limit(1024 * 1024 * 5); // Set body limit to at least 2 MB
  std::cout << "Entering async read" << std::endl;
  http::async_read(socket_, buffer_, request_,
                   [self](beast::error_code ec, std::size_t bytes_transferred) {
                     if (!ec) {
                       std::cout << "Async Read" << std::endl;
                       self->process_request();
                     } else {
                       std::cerr << "Read error: " << ec.message() << std::endl;
                     }
                   });
}

void http_session::process_request() {
  std::cout << "In process request" << std::endl;
  std::cout << "Request method: " << request_.method() << std::endl;
  std::cout << "Request target: " << request_.target() << std::endl;

  if (request_.method() == http::verb::post && request_.target() == "/report") {
    std::cout << "Going inside handler" << std::endl;
    handle_request_report();
  } else {
    send_not_found();
  }
}


void http_session::handle_request_report() {

  // Example of JSON parsing
  auto request_json = json::parse(request_.body());
  std::cout << "Parsed JSON: " << request_json.dump() << std::endl;

  auto grid_last_visited = request_json.at("area").get<std::vector<std::vector<long>>>();
  auto cc_id = request_json.at("cc-id").get<int>();

  int last_inserted_image_id = 0;

  std::ostringstream csv_file_path;
  csv_file_path << "/area_snapshots/report_" << cc_id << "image_" << last_inserted_image_id << ".csv";

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
