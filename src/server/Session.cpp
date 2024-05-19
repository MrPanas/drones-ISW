#include "Session.hpp"
#include <ostream>

using tcp = boost::asio::ip::tcp;    // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>
namespace beast =
    boost::beast; // from <boost/beast/core.hpp> using json = nlohmann::json;

/**
 * @brief Construct a new http session::http session object
 *
 * @param socket TCP socket for the session
 */
http_session::http_session(tcp::socket socket)
    : socket_(std::move(socket)),
      db_("postgres", "5432", "postgres", "postgres", "postgres") {}

/**
 * @brief Start the session by reading from the socket
 */
void http_session::run() { do_read(); }

/**
 * @brief Asynchronously read the HTTP request from the socket
 */
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

/**
 * @brief Process the HTTP request and route it to the appropriate handler
 */
void http_session::process_request() {
  std::string target = std::string(request_.target());

  if (request_.method() == http::verb::post && request_.target() == "/report") {
    handle_post_report();
  } else if (request_.method() == http::verb::get) {
    std::regex get_report_regex("^/report/(\\d+)$");
    std::smatch match;
    if (std::regex_match(target, match, get_report_regex)) {
      handle_get_report(std::stoi(match[1]));
    } else {
      send_not_found();
    }
  }
}

/**
 * @brief Handle POST request to create a report
 */
void http_session::handle_post_report() {
  auto request_json = json::parse(request_.body());

  auto grid_last_visited =
      request_json.at("area").get<std::vector<std::vector<long>>>();
  auto cc_id = request_json.at("cc-id").get<int>();

  std::string csv_file_path = create_csv_file(cc_id, grid_last_visited);

  snprintf(sqlcmd_, sizeof(sqlcmd_),
           "INSERT INTO report_image (cc_id, image_url) VALUES (%d, '%s')",
           cc_id, csv_file_path.c_str());

  PGresult *res = db_.ExecSQLcmd(sqlcmd_);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    PQclear(res);
    throw std::runtime_error("Failed to insert record into report_image table");
  }
  PQclear(res);

  prepare_response(http::status::created,
                   R"({"message": "CSV file created successfully"})",
                   "application/json");
  do_write();
}

/**
 * @brief Handle GET request to return the latest report and delete the second
 * most recent image
 */
void http_session::handle_get_report(int cc_id) {

  std::string latest_image_path = get_image_url(cc_id, 0);

  if (!latest_image_path.empty()) {
    // Open CSV file for reading
    std::ifstream csv_file(latest_image_path);
    if (!csv_file.is_open()) {
      throw std::runtime_error("Failed to open CSV file for reading");
    }

    // Read the CSV file content
    std::string line;
    json json_result = json::array();

    while (std::getline(csv_file, line)) {
      std::stringstream line_stream(line);
      std::string cell;
      json json_row = json::array();

      while (std::getline(line_stream, cell, ',')) {
        json_row.push_back(cell);
      }
      json_result.push_back(json_row);
    }
    csv_file.close();

    // Convert JSON object to string
    std::string json_content = json_result.dump();

    // Set up the HTTP response
    prepare_response(http::status::ok, json_content, "application/json");
    do_write();

    // Delete the second most recent image if it exists
    std::string second_image_path = get_image_url(cc_id, 1);
    if (!second_image_path.empty()) {
      delete_image_file(second_image_path);
    }
  } else {
    send_not_found();
  }
}

/**
 * @brief Prepare the HTTP response with the given status, body, and content
 * type
 *
 * @param status HTTP status code
 * @param body Response body
 * @param content_type Content type of the response
 */
void http_session::prepare_response(http::status status,
                                    const std::string &body,
                                    const std::string &content_type) {
  response_.version(request_.version());
  response_.keep_alive(request_.keep_alive());
  response_.result(status);
  response_.set(http::field::server, "Beast/Boost");
  response_.set(http::field::content_type, content_type);
  response_.body() = body;
  response_.prepare_payload();
}

/**
 * @brief Get the image URL from the database
 *
 * @param cc_id Control Center ID
 * @param offset Offset for the image (0 for latest, 1 for second latest)
 * @return std::string Image URL
 */
std::string http_session::get_image_url(int cc_id, int offset) {
  snprintf(sqlcmd_, sizeof(sqlcmd_),
           "SELECT image_url FROM report_image WHERE cc_id = %d ORDER BY "
           "image_id DESC LIMIT 1 OFFSET %d",
           cc_id, offset);

  PGresult *res = db_.ExecSQLtuples(sqlcmd_);
  std::string image_url;
  if (PQntuples(res) > 0) {
    image_url = PQgetvalue(res, 0, PQfnumber(res, "image_url"));
  }
  PQclear(res);
  return image_url;
}

/**
 * @brief Write data to a CSV file
 *
 * @param file_path Path to the CSV file
 * @param data Data to write
 */
void http_session::write_csv_file(const std::string &file_path,
                                  const std::vector<std::vector<long>> &data) {
  std::ofstream csv_file(file_path);
  if (!csv_file.is_open()) {
    throw std::runtime_error("Failed to open CSV file for writing");
  }

  for (const auto &row : data) {
    for (size_t i = 0; i < row.size(); ++i) {
      csv_file << row[i];
      if (i != row.size() - 1) {
        csv_file << ",";
      }
    }
    csv_file << "\n";
  }
  csv_file.close();
}

/**
 * @brief Create a CSV file for the report
 *
 * @param cc_id Control Center ID
 * @param grid_last_visited Data to write to the CSV
 * @return std::string Path to the created CSV file
 */
std::string http_session::create_csv_file(
    int cc_id, const std::vector<std::vector<long>> &grid_last_visited) {
  int last_inserted_image_id = 0;
  snprintf(
      sqlcmd_, sizeof(sqlcmd_),
      "SELECT max(image_id) AS image_id FROM report_image WHERE cc_id = %d",
      cc_id);

  PGresult *res = db_.ExecSQLtuples(sqlcmd_);
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

  write_csv_file(csv_file_path.str(), grid_last_visited);
  return csv_file_path.str();
}

/**
 * @brief Delete an image file
 *
 * @param file_path Path to the image file
 */
void http_session::delete_image_file(const std::string &file_path) {
  if (std::filesystem::exists(file_path)) {
    std::filesystem::remove(file_path);
  }
}

/**
 * @brief Send a 404 Not Found response
 */
void http_session::send_not_found() {
  std::cout << "Sending not found response" << std::endl;
  prepare_response(http::status::not_found, R"({"error": "Not Found"})",
                   "application/json");
  do_write();
}

/**
 * @brief Asynchronously write the HTTP response to the socket
 */
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
