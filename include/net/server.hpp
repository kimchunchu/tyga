#pragma once

#include "core/threadpool.hpp"
#include "http/connection.hpp"
#include "http/router.hpp"
#include <atomic>
#include <memory>
#include <unordered_map>
namespace tyga::net {
class Server {
public:
  Server(int port, http::Router &router, core::ThreadPool &thread_pool);
  void run();
  void stop();

private:
  int port_;
  int server_fd_;
  std::atomic<bool> running_{false};
  tyga::http::Router &router_;
  tyga::core::ThreadPool &thread_pool_;
  std::unordered_map<int, std::unique_ptr<tyga::http::HttpConnection>>
      connections_;
};
} // namespace tyga::net