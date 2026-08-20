#pragma once

#include "http/response.hpp"
#include "http/router.hpp"
#include <string>

namespace tyga::http {
class HttpConnection {
public:
  HttpConnection(int fd, Router &router);
  bool read();
  bool process();
  bool write();
  bool wants_write() const;

private:
  void send_response(Response &response);
  int fd_;
  std::string read_buffer_;
  std::string write_buffer_;
  std::size_t write_offset_ = 0;
  bool close_after_write_ = false;
  tyga::http::Router &router_;
};
} // namespace tyga::http