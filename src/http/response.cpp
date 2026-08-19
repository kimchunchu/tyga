#include "http/response.hpp"
#include "http/string.hpp"
#include <string>

namespace tyga::http {
Response::Response(int status_code, std::string status_text,
                   std::vector<ResponseHeader> headers, std::string body)
    : status_code_(status_code), status_text_(status_text), headers_(headers),
      body_(body) {};

Response Response::not_found() { return {400, "Not Found", {}, "Not Found"}; }

Response Response::method_not_allowed() {
  return Response{405, "Method Not Allowed", {}, "Method Not Allowed"};
}

Response Response::ok(std::string body) {
  return Response{200, "OK", {}, body};
}

Response Response::bad_request() {
  return Response{400, "Bad Request", {}, "Bad Request"};
}

Response Response::payload_too_large() {
  return Response{413, "Payload Too Large", {}, "Payload Too Large"};
}

Response &Response::header(std::string name, std::string value) {
  headers_.push_back({
      std::move(name),
      std::move(value),
  });

  return *this;
}

std::vector<ResponseHeader> Response::headers() { return headers_; }

std::string Response::serialize() {
  std::string result;
  result += "HTTP/1.1";
  result += " ";
  result += std::to_string(status_code_);
  result += " ";
  result += status_text_;
  result += "\r\n";

  bool has_content_length = false;
  for (const auto &header : headers()) {
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
    result += std::to_string(body_.size());
    result += "\r\n";
  }

  result += "\r\n";
  result += body_;

  return result;
}
} // namespace tyga::http