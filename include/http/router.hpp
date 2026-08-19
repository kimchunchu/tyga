#pragma once

#include "http/request.hpp"
#include "http/response.hpp"
#include <functional>
#include <unordered_map>

namespace tyga::http {
using Handler = std::function<Response(const Request &)>;

struct RouteKey {
  HttpMethod method;
  std::string path;

  bool operator==(const RouteKey &other) const {
    return method == other.method && path == other.path;
  }
};

struct RouteKeyHash {
  std::size_t operator()(const RouteKey &key) const {
    std::size_t h1 = std::hash<int>{}(static_cast<int>(key.method));
    std::size_t h2 =
        std::hash<std::string>{}(static_cast<std::string>(key.path));
    return h1 ^ (h2 << 1);
  }
};

class Router {
public:
  bool match_path(std::string_view pattern, std::string_view path,
                  std::vector<PathParam> &params);
  void get(const std::string &path, Handler handler);
  void post(const std::string &path, Handler handler);
  Response handle(Request request);

private:
  std::unordered_map<RouteKey, Handler, RouteKeyHash> routes_;
};
} // namespace tyga::http