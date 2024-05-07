#include "Server.hpp"
#include <iostream>
#include <ostream>

Server::Server(std::string host, std::string port, std::string password)
    : host(host), port(port), password(password) {
  socket_fd = createSocket();
  isRunning = true;
}

Server::~Server() { close(socket_fd); }

int Server::createSocket() {
  int sockfd;
  struct sockaddr_in addr = {};
  int int_port;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    throw std::runtime_error("Error while opening socket");

  memset(&addr, 0, sizeof(addr));
  std::istringstream(this->port) >> int_port;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(int_port);

  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    throw std::runtime_error("Error while binding socket");

  if (listen(sockfd, MAX_CONNECTIONS) < 0)
    throw std::runtime_error("Error while listening on socket");

  return sockfd;
}

void Server::startServer() {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int new_socket;

  new_socket = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);
  if (new_socket < 0)
    throw std::runtime_error("Failed to accept connection");

  while (isRunning) {
    char buffer[1024] = {0};

    int bytesReceived = read(new_socket, buffer, 1024);

    if (bytesReceived > 0) {
      std::cout << buffer << std::endl;
      std::string response = "Hello from server";

      send(new_socket, response.c_str(), response.length(), 0);
    } else {
      std::cout << "client disconnected" << std::endl;
      break;
    }
  }
}

void Server::stopServer() {
  std::cout << "Stoptting Server" << std::endl;
  this->isRunning = false;
}
