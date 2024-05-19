#include "Server.hpp"

int main() {
  try {
    std::string host = "0.0.0.0";
    std::string port = "3000";
    std::string password = "secure"; // Example, not used directly here.

    Server server(host, port, password); // This constructs and runs the server
    server.start();

  } catch (const std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
