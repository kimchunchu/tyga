#include "net/server.hpp"
#include "core/threadpool.hpp"
#include "http/connection.hpp"
#include "http/router.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <poll.h>
#include <stdexcept>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

using namespace tyga::core;

namespace tyga::net {
Server::Server(int port, http::Router &router) : port_(port), router_(router) {
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

void Server::remove_connection(std::vector<pollfd> &fds, std::size_t index) {
  int fd = fds[index].fd;

  close(fd);
  connections_.erase(fd);
  fds.erase(fds.begin() + index);
}

void Server::run() {
  running_ = true;
  std::vector<pollfd> fds;
  fds.push_back({server_fd_, POLLIN});
  while (running_) {
    int result = poll(fds.data(), fds.size(), 1000);

    if (result < 0) {
      if (errno == EINTR) {
        continue;
      }
      break;
    }

    if (result == 0) {
      check_timeouts(fds);
      continue;
    }

    const std::size_t count = fds.size();
    for (size_t i = 0; i < count; ++i) {
      pollfd &pfd = fds[i];

      if (pfd.fd == server_fd_) {
        if (!(pfd.revents & POLLIN)) {
          continue;
        }

        int client_fd = accept(server_fd_, nullptr, nullptr);

        if (client_fd < 0) {
          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            continue;
          }

          if (!running_) {
            break;
          }

          continue;
        }

        int flags = fcntl(client_fd, F_GETFL, 0);
        if (flags < 0 || fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
          close(client_fd);
          continue;
        }

        fds.push_back({client_fd, POLLIN, 0});
        connections_.emplace(client_fd, std::make_unique<http::HttpConnection>(
                                            client_fd, router_));
        continue;
      }

      auto it = connections_.find(pfd.fd);
      if (it == connections_.end()) {
        continue;
      }

      auto &connection = *it->second;

      if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
        remove_connection(fds, i);
        --i;
        continue;
      }

      if (pfd.revents & POLLIN) {

        if (!connection.read()) {
          remove_connection(fds, i);
          --i;
          continue;
        }

        if (!connection.process()) {
          remove_connection(fds, i);
          --i;
          continue;
        }

        if (connection.wants_write()) {
          pfd.events |= POLLOUT;
        }
      }

      if (pfd.revents & POLLOUT) {
        if (!connection.write()) {
          remove_connection(fds, i);
          --i;
          continue;
        }

        if (!connection.wants_write()) {
          pfd.events &= ~POLLOUT;
        }
      }
    }
  }
}

void Server::stop() { running_ = false; }

void Server::check_timeouts(std::vector<pollfd> &fds) {
  for (std::size_t i = 1; i < fds.size();) {
    int fd = fds[i].fd;
    auto it = connections_.find(fd);
    if (it == connections_.end()) {
      ++i;
      continue;
    }

    if (it->second->is_idle_timeout()) {
      std::cout << "[Timeout] fd=" << fd << std::endl;
      close(fd);
      connections_.erase(it);
      fds.erase(fds.begin() + 1);
      continue;
    }

    ++i;
  }
};
} // namespace tyga::net