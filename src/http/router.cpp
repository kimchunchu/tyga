#include "http/router.hpp"
#include "http/method.hpp"
#include "http/request.hpp"

namespace tyga::http {
void Router::get(const std::string &path, Handler handler) {
  routes_[{HttpMethod::GET, path}] = std::move(handler);
}

void Router::post(const std::string &path, Handler handler) {
  routes_[{HttpMethod::POST, path}] = std::move(handler);
}

// path => /users/123
// pattern => /users/:id
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
  for (const auto &[key, handler] : routes_) {
    if (key.method != request.method) {
      continue;
    }

    std::vector<PathParam> params;

    if (!match_path(key.path, request.path, params)) {
      continue;
    }

    request.params = std::move(params);
    return handler(request);
  }

  return Response{
      404,
      "Not Found",
      {},
      "Not Found",
  };
}
} // namespace tyga::http
