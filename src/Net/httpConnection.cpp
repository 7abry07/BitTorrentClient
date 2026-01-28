#include <Net/httpConnection.h>
#include <expected>
#include <helpers.h>
#include <string>

namespace btc {

HttpConnection::await_exp_connection
HttpConnection::connect(net::io_context &ctx, std::string hostname,
                        port_t port) {
  HttpConnection conn(ctx, hostname, port);

  sys::error_code ec;
  auto endpoints = co_await conn.resolver.async_resolve(
      hostname, std::to_string(port),
      boost::asio::redirect_error(boost::asio::use_awaitable, ec));

  if (ec)
    co_return std::unexpected(ec);

  co_await conn.stream.async_connect(
      endpoints, boost::asio::redirect_error(boost::asio::use_awaitable, ec));

  if (ec)
    co_return std::unexpected(ec);

  co_return conn;
}

HttpConnection::await_response HttpConnection::get(std::string url) {
  auto hostnamePos = url.find("://");
  if (hostnamePos == std::string::npos)
    co_return http::response<http::dynamic_body>();

  auto rest = url.substr(hostnamePos + 3);
  auto targetPos = rest.find("/");
  if (hostnamePos == std::string::npos)
    co_return http::response<http::dynamic_body>();

  std::string target = rest.substr(targetPos);

  http::request<http::string_body> req{http::verb::get, target, 11};
  req.set(http::field::host, hostname);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  sys::error_code ec;
  co_await http::async_write(stream, req,
                             net::redirect_error(net::use_awaitable, ec));
  if (ec)
    co_return http::response<http::dynamic_body>();

  beast::flat_buffer buf;
  http::response<http::dynamic_body> resp;

  co_await http::async_read(stream, buf, resp,
                            net::redirect_error(net::use_awaitable, ec));

  if (ec)
    co_return http::response<http::dynamic_body>();

  co_return resp;
}

} // namespace btc
