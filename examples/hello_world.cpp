#include "app.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  tyga::App app;
  app.get("/", [](const tyga::http::Request request) {
    return tyga::http::Response{200, "OK", {}, "Hello World!"};
  });

  app.run();
  return 0;
}