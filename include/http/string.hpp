#pragma once

#include <string_view>

namespace tyga::http {
int hex_value(char c);
bool iequals(std::string_view a, std::string_view b);
std::string url_decode(std::string_view value);
std::string_view trim_left(std::string_view value);
} // namespace tyga::http