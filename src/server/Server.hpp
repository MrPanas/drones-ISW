#ifndef DRONE8_SERVER_HPP
#define DRONE8_SERVER_HPP

#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CONNECTIONS 200

class Server {
public:
  Server(std::string host, std::string port, std::string password);
  ~Server();
  void startServer();
  void stopServer();

private:
  int createSocket();
  int socket_fd;
  bool isRunning;
  std::string host;
  std::string port;
  std::string password;
};

#endif // DRONE8_SERVER_HPP
