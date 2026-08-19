#pragma once

#include "core/threadpool.hpp"
#include "http/router.hpp"
namespace tyga::net {
class Server {
public:
  Server(int port, http::Router &router, core::ThreadPool &thread_pool);
  void run();

private:
  int port_;
  int server_fd_;
  tyga::http::Router &router_;
  tyga::core::ThreadPool &thread_pool_;
};
} // namespace tyga::net