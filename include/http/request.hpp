#pragma once

#include "http/method.hpp"
#include <vector>

namespace tyga::http {
struct PathParam {
  std::string name;
  std::string value;
};

struct RequestHeader {
  std::string_view name;
  std::string_view value;
};

struct Request {
  HttpMethod method;
  std::string_view path;
  std::string_view version;
  std::string_view query;
  std::vector<RequestHeader> headers;
  std::vector<PathParam> params;
  std::string body;
  bool should_close() const;
  std::optional<std::string> query_param(std::string_view name) const;
  std::optional<std::string_view> header(std::string_view name) const;
};
} // namespace tyga::http
