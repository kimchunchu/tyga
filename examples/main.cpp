#include "app.hpp"
#include "http/method.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include "http/router.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  tyga::App app;
  app.route("/", tyga::http::HttpMethod::GET,
            [](const tyga::http::Request request) {
              return tyga::http::Response{200, "OK", {}, "Hello World!"};
            });

  app.run();
  return 0;
}