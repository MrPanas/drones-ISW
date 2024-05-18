#include "Session.hpp"
#include <iostream>
#include <ostream>
#include <string>
#include <utility>

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

enum Direction { EAST, WEST };
const int GRID_SIZE = 300;
const int TOTAL_STEPS = 750;
const int CENTER_X = 150;
const int CENTER_Y = 150;
Direction curr_direction = EAST;

bool isManhattanDistanceConditionMet(int x, int y, int stepsLeft) {
  return abs(x - CENTER_X) + abs(y - CENTER_Y) == stepsLeft;
}

void addStartingPoint(std::vector<std::pair<int, int>> &startingPoints, int &x,
                      int &y) {
  std::pair<int, int> next_starting_point;
  bool point_set = false;

  if (curr_direction == EAST && x < GRID_SIZE - 1) {
    next_starting_point = std::make_pair(x + 1, y);
    x++;
    point_set = true;
  } else if (curr_direction == WEST && x > 0) {
    next_starting_point = std::make_pair(x - 1, y);
    x--;
    point_set = true;
  } else if (curr_direction == EAST && x == GRID_SIZE - 1 &&
             y < GRID_SIZE - 1) {
    next_starting_point = std::make_pair(x, y + 1);
    curr_direction = WEST;
    y++;
    point_set = true;
  } else if (curr_direction == WEST && x == 0 && y < GRID_SIZE - 1) {
    next_starting_point = std::make_pair(x, y + 1);
    curr_direction = EAST;
    y++;
    point_set = true;
  }

  if (point_set) {
    startingPoints.push_back(next_starting_point);
    std::cout << "Starting points added:" << std::endl;
    std::cout << "(" << next_starting_point.first << ", "
              << next_starting_point.second << ")" << std::endl;
  }
}

int calculate_path_coordinates() {
  int x = -1, y = 0;
  int steps;

  std::vector<std::pair<int, int>> startingPoints;

  while (y < GRID_SIZE) {
    addStartingPoint(startingPoints, x, y);
    steps = TOTAL_STEPS;

    while (steps > 0) {

      while (x < GRID_SIZE - 1 && curr_direction == EAST &&
             !isManhattanDistanceConditionMet(x, y, steps)) {
        x++;
        steps--;
        if (steps <= 0)
          break;
      }
      if (steps <= 0 || isManhattanDistanceConditionMet(x, y, steps))
        break;

      if (y < GRID_SIZE - 1 && curr_direction == EAST) {
        y++;
        steps--;
        curr_direction = WEST;
        if (steps <= 0 || isManhattanDistanceConditionMet(x, y, steps))
          break;
      }

      // Move left
      while (x > 0 && curr_direction == WEST &&
             !isManhattanDistanceConditionMet(x, y, steps)) {
        x--;
        steps--;
        if (steps <= 0)
          break;
      }
      if (steps <= 0 || isManhattanDistanceConditionMet(x, y, steps))
        break;

      // Move down one step
      if (y < GRID_SIZE - 1 && curr_direction == WEST) {
        y++;
        steps--;
        curr_direction = EAST;
        if (steps <= 0 || isManhattanDistanceConditionMet(x, y, steps))
          break;
      }

      if (y == GRID_SIZE - 1 && (curr_direction == EAST && x == GRID_SIZE - 1 ||
                                 curr_direction == WEST && x == 0)) {
        y++;
        break;
      }
    }
  }

  // Output the starting points of each path
  std::cout << "Starting points of each path:" << std::endl;
  for (const auto &point : startingPoints) {
    std::cout << "(" << point.first << ", " << point.second << ")" << std::endl;
  }

  return 0;
}

/**
 * TODO: This datastrucutre will carry for each path a set of drones with their
 * scan init times for that path
 *
 *
 */
struct drone_scan_init {
  int drone_id;
  std::string scan_init_time;

  drone_scan_init(int d_id, const std::string &time)
      : drone_id(d_id), scan_init_time(time) {}
};

struct path_drone_scann_init {
  int path_id;
  std::vector<drone_scan_init> drones;

  path_drone_scann_init(int p_id) : path_id(p_id) {}

  void add_drone(int drone_id, const std::string &time) {
    drones.emplace_back(drone_id, time);
  }
};

void calculate_currrent_coordinates(int path_start_x, int path_start_y,
                                    std::string init_time,
                                    std::string current_time, int &current_x,
                                    int &current_y) {}

void fill_array_with_last_visited_times_millis(int x_1, int y_1, int x_2,
                                               int y_2) {}

void fill_array_for_path_drone(int path_id, std::vector<drone_scan_init> drones,
                               std::string current_time) {

  int last_drone_x = path_x;
  int last_drone_y = path_y;

  int current_drone_x;
  int current_drone_y;

  for (const auto &drone : drones) {
    calculate_currrent_coordinates(path_x, path_y, drone.scan_init_time,
                                   current_time, current_drone_x,
                                   current_drone_y);

    fill_array_with_last_visited_times_millis(current_drone_x, current_drone_y,
                                              last_drone_x, last_drone_y);
  }
}

/**
 * TODO:
 * Returns a set of drones with the respective init times for each path ( ~ last
 35 min )
 *
 *
 */
void get_most_recent_paths(
    const std::string &current_time_str,
    std::list<path_drone_scann_init> &paths_with_init_times) {

  Con2DB db("localhost", "5432", "postgres", "postgres", "postgres");
  char sqlcmd[512];

  snprintf(sqlcmd, sizeof(sqlcmd),
           "SELECT path.path_id, dl.drone_id, dl.time AS scan_time "
           "FROM path "
           "LEFT JOIN drone_log AS dl ON path.path_id = dl.path_id AND "
           "dl.status = 'SCANNING' "
           "WHERE dl.time >= TIMESTAMP '%s' - INTERVAL '35 minutes' "
           "ORDER BY path.path_id, dl.drone_id",
           current_time_str.c_str());

  PGresult *res = db.ExecSQLtuples(sqlcmd);

  for (int i = 0; i < PQntuples(res); i++) {
    int path_id =
        strtol(PQgetvalue(res, i, PQfnumber(res, "path_id")), NULL, 10);
    int drone_id =
        strtol(PQgetvalue(res, i, PQfnumber(res, "drone_id")), NULL, 10);
    const char *time_scan_str = PQgetvalue(res, i, PQfnumber(res, "scan_time"));

    auto it =
        std::find_if(paths_with_init_times.begin(), paths_with_init_times.end(),
                     [path_id](const path_drone_scann_init &path) {
                       return path.path_id == path_id;
                     });

    if (it != paths_with_init_times.end()) {
      it->add_drone(drone_id, std::string(time_scan_str));
    } else {
      path_drone_scann_init new_path(path_id);
      new_path.add_drone(drone_id, std::string(time_scan_str));
      paths_with_init_times.push_back(new_path);
    }
  }
}

using json = nlohmann::json;

/**
 * @var int cc_id
 * @var timestamp map_timestamp
 */
void http_session::handle_request_report() {

  calculate_path_coordinates();

  int grid_last_visited[300][300] = {{0}};

  int y_start_curr_path = 0;
  int x_start_curr_path = 0;

  int y_start_next_path;
  int x_start_next_path;
  std::list<path_drone_scann_init> paths;

  std::string current_time = "2024-05-17 12:00:00";

  // get_most_recent_paths(current_time, paths);
  //
  // // Ouput Debug
  // for (const auto &path : paths) {
  //   std::cout << "Path ID: " << path.path_id << std::endl;
  //   for (const auto &drone : path.drones) {
  //     std::cout << "  Drone ID: " << drone.drone_id
  //               << ", Scan Init Time: " << drone.scan_init_time << std::endl;
  //   }
  // }

  // Flatten the 2D array into a 1D array
  std::vector<int> flattened_grid;
  flattened_grid.reserve(90000); // Reserve space for 90,000 entries

  for (int i = 0; i < 300; ++i) {
    for (int j = 0; j < 300; ++j) {
      flattened_grid.push_back(grid_last_visited[i][j]);
    }
  }

  // Create a JSON object
  json response_json;
  response_json["grid_last_visited"] = flattened_grid;

  for (const auto &path : paths) {
    int path_id = path.path_id;
    // calculate_next_path_start();
  }

  // Convert the JSON object to a string
  std::string response_body = response_json.dump();

  // Set up the HTTP response
  response_.version(request_.version());
  response_.keep_alive(request_.keep_alive());
  response_.result(http::status::created);
  response_.set(http::field::server, "Beast/Boost");
  response_.set(http::field::content_type, "application/json");
  response_.body() = response_body;
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
