#pragma once

#include <string>
#include <vector>

namespace tyga::http {
struct ResponseHeader {
  std::string name;
  std::string value;
};

struct Response {
  int status_code;
  std::string status_text;
  std::vector<ResponseHeader> headers;
  std::string body;
};

std::string serialize_response(const Response &response);
} // namespace tyga::http
