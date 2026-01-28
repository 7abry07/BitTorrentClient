#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/system/detail/error_code.hpp>
#include <cstdint>
#include <errors.h>
#include <expected>

namespace btc {

namespace http = boost::beast::http;
namespace beast = boost::beast;
namespace net = boost::asio;
namespace sys = boost::system;

using net::awaitable;
using net::io_context;
using net::ip::tcp;

class HttpConnection {

public:
  static awaitable<std::expected<HttpConnection, sys::error_code>>
  connect(io_context &ctx, std::string hostname, std::uint16_t port);

  awaitable<http::response<http::dynamic_body>> get(std::string url);

private:
  HttpConnection(io_context &ctx, std::string hostname, std::uint16_t port)
      : ctx(ctx), resolver(ctx), stream(ctx), hostname(hostname), port(port) {}

  io_context &ctx;
  tcp::resolver resolver;
  beast::tcp_stream stream;

  std::uint16_t port;
  std::string hostname;
};

} // namespace btc
