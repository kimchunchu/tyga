#include "http/request.hpp"
#include "http/string.hpp"
#include <optional>
#include <string_view>

namespace tyga::http {
std::optional<std::string> Request::query_param(std::string_view name) const {
  size_t offset = 0;
  while (offset < query.size()) {
    size_t end = query.find('&');
    if (end == std::string_view::npos) {
      end = query.size();
    }

    std::string_view pair = query.substr(offset, end - offset);
    size_t equal = pair.find('=');
    if (equal != std::string_view::npos) {
      std::string_view key = pair.substr(0, equal);
      std::string_view value = pair.substr(equal + 1);

      if (key == name) {
        return url_decode(value);
      }
    }

    offset = end + 1;
  }

  return std::nullopt;
}

std::optional<std::string_view> Request::header(std::string_view name) const {
  for (const auto &header : headers) {
    if (iequals(header.name, name)) {
      return header.value;
    }
  }
  return std::nullopt;
}

bool Request::should_close() const {
  auto connection = header("Connection");
  if (!connection) {
    return version == "HTTP/1.0";
  }
  return iequals(*connection, "close");
}
} // namespace tyga::http