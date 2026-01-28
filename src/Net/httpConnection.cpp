#include <Net/httpConnection.h>
#include <boost/asio.hpp>
#include <expected>
#include <string>

namespace btc {

awaitable<std::expected<HttpConnection, error_code>>
HttpConnection::connect(io_context &ctx, std::string hostname,
                        std::uint16_t port) {
  HttpConnection conn(ctx, hostname, port);

  error_code ec;
  auto endpoints = co_await conn.resolver.async_resolve(
      hostname, std::to_string(port),
      boost::asio::redirect_error(boost::asio::use_awaitable, ec));

  if (ec)
    co_return std::unexpected(ec);

  co_await boost::asio::async_connect(
      conn.socket, endpoints,
      boost::asio::redirect_error(boost::asio::use_awaitable, ec));

  if (ec)
    co_return std::unexpected(ec);

  co_return conn;
}

} // namespace btc
