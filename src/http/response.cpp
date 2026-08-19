#include "http/response.hpp"
#include "http/string.hpp"
#include <string>

namespace tyga::http {
std::string serialize_response(const Response &response) {
  std::string result;
  result += "HTTP/1.1";
  result += " ";
  result += std::to_string(response.status_code);
  result += " ";
  result += response.status_text;
  result += "\r\n";

  bool has_content_length = false;
  for (const auto &header : response.headers) {
    if (iequals(header.name, "Content-Length")) {
      has_content_length = true;
    }

    result += header.name;
    result += ": ";
    result += header.value;
    result += "\r\n";
  }

  if (!has_content_length) {
    result += "Content-Length: ";
    result += std::to_string(response.body.size());
    result += "\r\n";
  }

  result += "\r\n";
  result += response.body;

  return result;
}
} // namespace tyga::http