#pragma once

#include "core/threadpool.hpp"
#include "http/connection.hpp"
#include "http/router.hpp"
#include <atomic>
#include <memory>
#include <sys/poll.h>
#include <unordered_map>
#include <vector>
namespace tyga::net {
class Server {
public:
  Server(int port, http::Router &router);
  void run();
  void stop();
  void check_timeouts(std::vector<pollfd> &fds);

private:
  int port_;
  int server_fd_;
  tyga::http::Router &router_;
  std::atomic<bool> running_{false};
  std::unordered_map<int, std::unique_ptr<tyga::http::HttpConnection>>
      connections_;
};
} // namespace tyga::net