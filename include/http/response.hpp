#pragma once

#include <string>
#include <vector>

namespace tyga::http {
struct ResponseHeader {
  std::string name;
  std::string value;
};

class Response {
public:
  Response(int status_code, std::string status_text,
           std::vector<ResponseHeader> headers, std::string body);

  static Response ok(std::string body);
  static Response not_found();
  static Response method_not_allowed();
  static Response bad_request();
  static Response payload_too_large();
  static Response request_timeout();

  Response &header(std::string name, std::string value);
  std::string serialize();

private:
  std::vector<ResponseHeader> headers_;
  std::string body_;
  std::string status_text_;
  int status_code_;
};

} // namespace tyga::http
