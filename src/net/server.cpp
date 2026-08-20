#include "net/server.hpp"
#include "core/threadpool.hpp"
#include "http/connection.hpp"
#include "http/router.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <fcntl.h>
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

  int flags = fcntl(server_fd_, F_GETFL, 0);
  if (flags < 0) {
    throw std::runtime_error("failed to get socket flags");
  }

  if (fcntl(server_fd_, F_SETFL, flags | O_NONBLOCK) < 0) {
    throw std::runtime_error("failed to set non-blocking socket");
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
  running_ = true;
  ThreadPool thread_pool(4);

  while (running_) {
    int client_fd = accept(server_fd_, nullptr, nullptr);
    if (client_fd < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        std::cout << "accept(): no connection\n";
        continue;
      }

      if (!running_) {
        break;
      }

      continue;
    }

    timeval timeout{};
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    thread_pool.submit([this, client_fd] {
      http::HttpConnection connection{client_fd, router_};
      connection.run();
    });
  }
}

void Server::stop() { running_ = false; }
} // namespace tyga::net