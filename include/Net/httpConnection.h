#pragma once

#include <cstdint>
#include <errors.h>
#include <expected>
#include <utils.h>

namespace btc {

class HttpConnection {

public:
  static std::expected<HttpConnection, BoostErrorCode>
  connect(IOContext &ctx, std::string hostname, std::uint16_t port);

private:
  HttpConnection(IOContext &ctx, std::string hostname, std::uint16_t port)
      : ctx(ctx), resolver(ctx), socket(ctx), hostname(hostname), port(port) {}

  IOContext &ctx;
  TCPResolver resolver;
  TCPSocket socket;

  std::uint16_t port;
  std::string hostname;
  std::vector<Endpoint> endpoints;
};

} // namespace btc
