#import "Server.cpp"

int main() {
  try {
    Server srv = Server("localhost", "3000", "password");
  } catch (std::exception const &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
