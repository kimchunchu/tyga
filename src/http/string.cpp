#include "http/string.hpp"
#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>

namespace tyga::http {
int hex_value(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }

  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }

  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }

  return -1;
}

std::string url_decode(std::string_view value) {
  std::string result;
  for (size_t i = 0; i < value.size(); ++i) {
    if (value[i] == '%') {
      if (i + 2 >= value.size()) {
        return {};
      }

      int high = hex_value(value[i + 1]);
      int low = hex_value(value[i + 2]);

      if (high == -1 || low == -1) {
        return {};
      }

      char decoded = static_cast<char>((high << 4) | low);
      result.push_back(decoded);

      i += 2;
      continue;
    }

    if (value[i] == '+') {
      result.push_back(' ');
      continue;
    }

    result.push_back(value[i]);
  }
  return result;
}

bool iequals(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) {
    return false;
  }

  for (size_t i = 0; i < a.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(a[i])) !=
        std::tolower(static_cast<unsigned char>(b[i]))) {
      return false;
    }
  }
  return true;
}

std::string_view trim_left(std::string_view value) {
  while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
    value.remove_prefix(1);
  }
  return value;
}
} // namespace tyga::http
