#include "net/server.hpp"
#include "core/threadpool.hpp"
#include "http/connection.hpp"
#include "http/router.hpp"
#include <arpa/inet.h>
#include <cstddef>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

using namespace tyga::core;

namespace tyga::net {
Server::Server(int port, http::Router &router, ThreadPool &thread_pool)
    : port_(port), router_(router), thread_pool_(thread_pool) {
  server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd_ < 0) {
    throw std::runtime_error("failed to receive socket file discriptor");
  }

  int opt = 1;

  if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    close(server_fd_);
    throw std::runtime_error("failed to set SO_REUSEADDR");
  }

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = htons(port_);
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (bind(server_fd_, reinterpret_cast<sockaddr *>(&address),
           sizeof(address)) < 0) {
    close(server_fd_);
    throw std::runtime_error("failed to bind socket");
  }

  std::cout << "[127.0.0.1:8080] Server is running...\n";

  if (listen(server_fd_, 16) == -1) {
    close(server_fd_);
    throw std::runtime_error("failed to listen");
  }
};

void Server::run() {
  ThreadPool thread_pool(4);
  while (true) {
    int client_fd = accept(server_fd_, nullptr, nullptr);
    if (client_fd < 0) {
      continue;
    }

    thread_pool.submit([this, client_fd] {
      http::HttpConnection connection{client_fd, router_};
      connection.run();
    });
  }
}
} // namespace tyga::net