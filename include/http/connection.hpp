#pragma once

#include "http/response.hpp"
#include "http/router.hpp"
#include <string>

namespace tyga::http {
class HttpConnection {
public:
  HttpConnection(int fd, Router &router) : fd_(fd), router_(router) {};
  void run();

private:
  void process();
  void send_response(const Response &response);
  int fd_;
  std::string buffer_;
  tyga::http::Router &router_;
};
} // namespace tyga::http