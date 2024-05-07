#include "Server.hpp"
#include <iostream>
#include <ostream>
#include <sys/socket.h>

int createAndConnectClient() {

  const char *server_ip = "127.0.0.1";
  int server_port = 3000;

  std::cout << "Client Started" << std::endl;

  // Create socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    std::cerr << "Error creating socket." << std::endl;
    return 1;
  }

  // Set server address
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);
  if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
    std::cerr << "Invalid address / Address not supported." << std::endl;
    return 1;
  }

  // Connect to server
  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    std::cerr << "Connection Failed." << std::endl;
    return 1;
  }

  const char *arrayOfMessage[] = {"Galileo", "Newton", "Einstein", "Feynmann"};
  for (int i = 0; i < 4; i++) {

    std::string message = arrayOfMessage[i];
    send(sock, message.c_str(), message.size(), 0);

    // Receive response from server
    char buffer[1024] = {0};
    int bytesReceived = read(sock, buffer, sizeof(buffer));
    if (bytesReceived > 0) {
      std::cout << "Server: " << buffer << std::endl;
    }
  }

  // Send message

  // Close the socket
  close(sock);
  return 0;
}

int main() { int result = createAndConnectClient(); }
