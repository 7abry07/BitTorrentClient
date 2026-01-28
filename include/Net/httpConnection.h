#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system/detail/error_code.hpp>
#include <cstdint>
#include <errors.h>
#include <expected>
#include <helpers.h>

namespace btc {

class HttpConnection {

public:
  static net::awaitable<std::expected<HttpConnection, sys::error_code>>
  connect(net::io_context &ctx, std::string hostname, std::uint16_t port);

  net::awaitable<http::response<http::dynamic_body>> get(std::string url);

private:
  HttpConnection(net::io_context &ctx, std::string hostname, std::uint16_t port)
      : ctx(ctx), resolver(ctx), stream(ctx), hostname(hostname), port(port) {}

  net::io_context &ctx;
  tcp::resolver resolver;
  beast::tcp_stream stream;

  std::uint16_t port;
  std::string hostname;
};

} // namespace btc
