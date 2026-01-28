#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <cstdint>

namespace btc {

using port_t = std::uint16_t;

namespace beast = boost::beast;
namespace net = boost::asio;
namespace http = beast::http;
namespace sys = boost::system;

using tcp = net::ip::tcp;

} // namespace btc
