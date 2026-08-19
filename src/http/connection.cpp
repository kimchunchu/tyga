#include "http/connection.hpp"
#include "http/parser.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include <sys/socket.h>
#include <unistd.h>

namespace tyga::http {
void HttpConnection::process() {
  while (true) {
    ParseResult result = parse_request(buffer_);
    if (result.status == ParseStatus::Incomplete) {
      return;
    }

    if (result.status == ParseStatus::Error) {
      return;
    }

    Request &request = *result.request;
    Response response = router_.handle(request);
    bool should_close = request.should_close();
    if (should_close) {
      response.headers.push_back({"Connection", "close"});
    }

    send_response(response);

    buffer_.erase(0, result.consumed);
  }
}

void HttpConnection::run() {
  while (true) {
    char temp[4096];
    ssize_t bytes_read = read(fd_, temp, sizeof(temp));
    if (bytes_read == 0) {
      break;
    }

    if (bytes_read < 0) {
      break;
    }

    buffer_.append(temp, bytes_read);

    process();
  }
}

void HttpConnection::send_response(const Response &response) {
  std::string data = serialize_response(response);
  size_t total_sent = 0;
  while (total_sent < data.size()) {
    ssize_t sent =
        send(fd_, data.data() + total_sent, data.size() - total_sent, 0);
    if (sent <= 0) {
      return;
    }

    total_sent += static_cast<size_t>(sent);
  }
}
} // namespace tyga::http