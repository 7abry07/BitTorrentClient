#pragma once

#include <expected>
#include <helpers.h>
#include <system_error>

namespace btc {

class HttpConnection {

private:
  using exp_connection = std::expected<HttpConnection, std::error_code>;
  using exp_response =
      std::expected<http::response<http::dynamic_body>, std::error_code>;
  using await_exp_connection = net::awaitable<exp_connection>;
  using await_exp_response = net::awaitable<exp_response>;

public:
  await_exp_response get(std::string url);
  static await_exp_connection connect(net::io_context &ctx,
                                      std::string hostname, port_t port);
  void close();

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
