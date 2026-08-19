#include "http/router.hpp"
#include "http/method.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include "http/string.hpp"
#include <iostream>
#include <vector>

namespace tyga::http {
void Router::get(const std::string &path, Handler handler) {
  routes_[{HttpMethod::GET, path}] = std::move(handler);
}

void Router::post(const std::string &path, Handler handler) {
  routes_[{HttpMethod::POST, path}] = std::move(handler);
}

bool Router::match_path(std::string_view pattern, std::string_view path,
                        std::vector<PathParam> &params) {
  size_t pattern_pos = 0;
  size_t path_pos = 0;

  while (pattern_pos < pattern.size() || path_pos < path.size()) {
    if (pattern_pos < pattern.size() && pattern[pattern_pos] == '/') {
      ++pattern_pos;
    }

    if (path_pos < path.size() && path[path_pos] == '/') {
      ++path_pos;
    }

    size_t pattern_end = pattern.find('/', pattern_pos);
    size_t path_end = path.find('/', path_pos);

    if (pattern_end == std::string_view::npos) {
      pattern_end = pattern.size();
    }

    if (path_end == std::string_view::npos) {
      path_end = path.size();
    }

    std::string_view pattern_part =
        pattern.substr(pattern_pos, pattern_end - pattern_pos);
    std::string_view path_part = path.substr(path_pos, path_end - path_pos);

    if (!pattern_part.empty() && pattern_part.front() == ':') {
      params.push_back({
          std::string(pattern_part.substr(1)),
          std::string(path_part),
      });
    } else if (pattern_part != path_part) {
      return false;
    }

    pattern_pos = pattern_end;
    path_pos = path_end;
  }

  return pattern_pos == pattern.size() && path_pos == path.size();
}

Response Router::handle(Request request) {
  bool path_matched = false;
  std::vector<HttpMethod> allowed_methods;

  for (const auto &[key, handler] : routes_) {
    std::vector<PathParam> params;
    if (!match_path(key.path, request.path, params)) {
      continue;
    }
    path_matched = true;

    allowed_methods.push_back(key.method);

    if (key.method != request.method) {
      continue;
    }

    request.params = std::move(params);
    return handler(request);
  }

  if (!path_matched) {
    return Response::not_found();
  }

  Response response = Response::method_not_allowed();
  std::string allow;
  for (size_t i = 0; i < allowed_methods.size(); ++i) {
    if (i > 0) {
      allow += ", ";
    }
    allow += method_to_string(allowed_methods[i]);
  }

  std::cout << allow << std::endl;
  response.header("Allow", allow);
  return response;
}
} // namespace tyga::http
