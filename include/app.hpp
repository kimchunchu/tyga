#pragma once

#include "core/threadpool.hpp"
#include "http/response.hpp"
#include "http/router.hpp"
#include "net/server.hpp"
#include <string>

namespace tyga {
class App {
public:
  App();
  void run();
  void get(const std::string &path, http::Handler handler);
  void post(const std::string &path, http::Handler handler);
  http::Response handle(http::Request request);

private:
  http::Router router_;
  net::Server server_;
  core::ThreadPool thread_pool_;
};
} // namespace tyga