#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/detail/error_code.hpp>
#include <cstdint>
#include <errors.h>
#include <expected>

namespace btc {

using boost::asio::awaitable;
using boost::asio::io_context;
using boost::asio::ip::tcp;
using boost::system::error_code;

class HttpConnection {

public:
  static awaitable<std::expected<HttpConnection, error_code>>
  connect(io_context &ctx, std::string hostname, std::uint16_t port);

private:
  HttpConnection(io_context &ctx, std::string hostname, std::uint16_t port)
      : ctx(ctx), resolver(ctx), socket(ctx), hostname(hostname), port(port) {}

  io_context &ctx;
  tcp::resolver resolver;
  tcp::socket socket;

  std::uint16_t port;
  std::string hostname;
};

} // namespace btc
