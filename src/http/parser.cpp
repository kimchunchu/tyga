#include "http/parser.hpp"
#include "http/method.hpp"
#include "http/string.hpp"
#include <cctype>
#include <charconv>
#include <optional>
#include <string_view>
#include <system_error>

namespace tyga::http {
std::optional<size_t> parse_content_length(std::string_view value) {
  size_t result = 0;
  auto [ptr, ec] =
      std::from_chars(value.data(), value.data() + value.size(), result);
  if (ec != std::errc{}) {
    return std::nullopt;
  }

  if (ptr != value.data() + value.size()) {
    return std::nullopt;
  }
  return result;
};

std::optional<RequestHeader> parse_header_line(std::string_view line) {
  const size_t colon = line.find(':');
  if (colon == std::string_view::npos) {
    return std::nullopt;
  }

  if (colon == 0 || line[colon - 1] == ' ') {
    return std::nullopt;
  }

  RequestHeader header;
  header.name = line.substr(0, colon);
  header.value = trim_left(line.substr(colon + 1));
  return header;
}

std::optional<HttpMethod> parse_method(std::string_view method) {
  if (method == "GET") {
    return HttpMethod::GET;
  }
  if (method == "POST") {
    return HttpMethod::POST;
  }
  if (method == "PUT") {
    return HttpMethod::PUT;
  }
  if (method == "PATCH") {
    return HttpMethod::PATCH;
  }
  if (method == "DELETE") {
    return HttpMethod::DELETE;
  }
  if (method == "OPTIONS") {
    return HttpMethod::OPTIONS;
  }
  if (method == "HEAD") {
    return HttpMethod::HEAD;
  }
  return std::nullopt;
}

ParseResult parse_request(std::string_view buffer) {
  const size_t first_space = buffer.find(' ');
  if (first_space == std::string_view::npos) {
    return {ParseStatus::Incomplete, std::nullopt, 0};
  }

  const size_t second_space = buffer.find(' ', first_space + 1);
  if (second_space == std::string_view::npos) {
    return {ParseStatus::Incomplete, std::nullopt, 0};
  }

  const size_t request_line_end = buffer.find("\r\n", second_space + 1);
  if (request_line_end == std::string_view::npos) {
    return {ParseStatus::Incomplete, std::nullopt, 0};
  }

  auto parsed_method = parse_method(buffer.substr(0, first_space));
  if (!parsed_method) {
    return {ParseStatus::Error, std::nullopt, 0};
  }

  Request request;
  request.method = *parsed_method;
  request.version =
      buffer.substr(second_space + 1, request_line_end - second_space - 1);

  std::string_view target =
      buffer.substr(first_space + 1, second_space - first_space - 1);

  const size_t query_pos = target.find('?');
  if (query_pos == std::string_view::npos) {
    request.path = target;
    request.query = {};
  } else {
    request.path = target.substr(0, query_pos);
    request.query = target.substr(query_pos + 1);
  }

  size_t offset = request_line_end + 2;
  while (true) {
    const size_t line_end = buffer.find("\r\n", offset);
    if (line_end == std::string_view::npos) {
      return {ParseStatus::Incomplete, std::nullopt, 0};
    }

    if (line_end == offset) {
      offset += 2;
      break;
    }

    std::string_view line = buffer.substr(offset, line_end - offset);
    std::optional<RequestHeader> header = parse_header_line(line);
    if (!header) {
      return {ParseStatus::Error, std::nullopt, 0};
    }

    request.headers.emplace_back(header->name, header->value);

    offset = line_end + 2;
  }

  std::optional<size_t> content_length;
  for (const auto &header : request.headers) {
    if (!iequals(header.name, "Content-Length")) {
      continue;
    }

    if (content_length.has_value()) {
      return {ParseStatus::Error, std::nullopt, 0};
    }

    std::optional<size_t> parsed = parse_content_length(header.value);
    if (!parsed) {
      return {ParseStatus::Error, std::nullopt, 0};
    }

    content_length = *parsed;
  }

  const size_t body_start = offset;

  if (!content_length.has_value()) {
    request.body = {};
    return {ParseStatus::Complete, request, body_start};
  }

  size_t available = buffer.size() - body_start;
  if (available < *content_length) {
    return {ParseStatus::Incomplete, std::nullopt, 0};
  }

  request.body = buffer.substr(body_start, *content_length);
  size_t consumed = body_start + *content_length;
  return {ParseStatus::Complete, request, consumed};
}
} // namespace tyga::http
