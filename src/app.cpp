#include "app.hpp"

namespace tyga {
App::App() : router_(), server_(8080, router_) {}

void App::run() { server_.run(); }

void App::get(const std::string &path, http::Handler handler) {
  router_.get(path, handler);
}

void App::post(const std::string &path, http::Handler handler) {
  router_.post(path, handler);
}

http::Response App::handle(http::Request request) {
  return router_.handle(request);
}
} // namespace tyga