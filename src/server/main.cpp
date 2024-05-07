#include "Server.hpp"

int main() {
  Server myServer("localhost", "3000", "password123");
  try {
    myServer.startServer();
  } catch (const std::exception &e) {
    std::cerr << "Error occurred: " << e.what() << std::endl;
  }

  return 0;
}
