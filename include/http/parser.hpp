#pragma once

#include "http/request.hpp"
#include <cstddef>
#include <string_view>

namespace tyga::http {
enum class ParseStatus { Complete, Incomplete, Error };

struct ParseResult {
  ParseStatus status;
  std::optional<Request> request;
  size_t consumed;
};

ParseResult parse_request(std::string_view buffer);
} // namespace tyga::http