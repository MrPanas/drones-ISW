#ifndef HTTP_SESSION_HPP
#define HTTP_SESSION_HPP

#include "../con2db/pgsql.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/verb.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <ostream>
#include <postgresql/libpq-fe.h>
#include <string>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
using json = nlohmann::json;    // Use nlohmann::json directly
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class http_session : public std::enable_shared_from_this<http_session> {
public:
  explicit http_session(tcp::socket socket);
  void run();

private:
  tcp::socket socket_;
  beast::flat_buffer buffer_; // Buffer for reading, must be persisted
  http::request<http::string_body> request_;
  http::response<http::string_body> response_;
  Con2DB db_;        // Database connection
  char sqlcmd_[512]; // SQL command buffer

  void do_read();
  void process_request();
  void handle_post_report();
  void handle_get_report();
  void send_not_found();
  void do_write();

  void prepare_response(http::status status, const std::string &body,
                        const std::string &content_type);
  std::string get_image_url(int cc_id, int offset);
  void write_csv_file(const std::string &file_path,
                      const std::vector<std::vector<long>> &data);
  std::string
  create_csv_file(int cc_id,
                  const std::vector<std::vector<long>> &grid_last_visited);
  void delete_image_file(const std::string &file_path);
};

#endif // HTTP_SESSION_HPP
