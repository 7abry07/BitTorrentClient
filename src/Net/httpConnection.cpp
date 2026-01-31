#include <Net/httpConnection.h>

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

HttpConnection::await_exp_response HttpConnection::get(std::string url) {
  sys::result<url_view> r = urls::parse_uri(std::string_view(url));
  if (r.has_error())
    co_return std::unexpected(r.error());

  http::request<http::string_body> req{http::verb::get, r.value().path(), 11};
  req.set(http::field::host, hostname);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  sys::error_code ec;
  co_await http::async_write(stream, req,
                             net::redirect_error(net::use_awaitable, ec));
  if (ec)
    co_return std::unexpected(ec);

  beast::flat_buffer buf;
  http::response<http::dynamic_body> resp;
  co_await http::async_read(stream, buf, resp,
                            net::redirect_error(net::use_awaitable, ec));
  if (ec)
    co_return std::unexpected(ec);

  co_return resp;
}

} // namespace btc
