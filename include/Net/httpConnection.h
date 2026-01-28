#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system.hpp>

#include <errors.h>
#include <expected>
#include <helpers.h>
#include <system_error>

namespace btc {

class HttpConnection {

private:
  using exp_connection = std::expected<HttpConnection, std::error_code>;
  using response = http::response<http::dynamic_body>;

  using await_exp_connection = net::awaitable<exp_connection>;
  using await_response = net::awaitable<response>;

public:
  await_response get(std::string url);
  static await_exp_connection connect(net::io_context &ctx,
                                      std::string hostname, port_t port);

private:
  HttpConnection(net::io_context &ctx, std::string hostname, port_t port)
      : ctx(ctx), resolver(ctx), stream(ctx), hostname(hostname), port(port) {}

  net::io_context &ctx;
  tcp::resolver resolver;
  beast::tcp_stream stream;

  std::string hostname;
  port_t port;
};

} // namespace btc
