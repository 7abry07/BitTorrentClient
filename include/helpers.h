#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace btc {

namespace http = boost::beast::http;
namespace beast = boost::beast;
namespace net = boost::asio;
namespace sys = boost::system;

using tcp = net::ip::tcp;

} // namespace btc
