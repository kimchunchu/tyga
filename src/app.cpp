#include "app.hpp"
#include "http/router.hpp"

namespace tyga {
App::App() : router_(), server_(8080, router_) {}

void App::run() { server_.run(); }

void App::route(const std::string &path, tyga::http::HttpMethod method,
                http::Handler handler) {
  router_.add(path, method, handler);
}

http::Response App::handle(http::Request request) {
  return router_.handle(request);
}
} // namespace tyga