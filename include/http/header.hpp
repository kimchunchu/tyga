#pragma once

#include <string_view>

namespace tyga::http {
struct Header {
  std::string_view name;
  std::string_view value;
};
} // namespace tyga::http