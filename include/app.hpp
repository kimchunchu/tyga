#pragma once

#include "http/response.hpp"
#include "http/router.hpp"
#include "net/server.hpp"

namespace tyga {
class App {
public:
  App();
  void run();
  void route(const std::string &path, tyga::http::HttpMethod method,
             http::Handler handler);
  http::Response handle(http::Request request);

private:
  http::Router router_;
  net::Server server_;
};
} // namespace tyga