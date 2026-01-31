#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system/result.hpp>
#include <boost/url.hpp>
#include <boost/url/parse.hpp>
#include <boost/url/urls.hpp>
#include <cstdint>

namespace btc {

using port_t = std::uint16_t;

namespace beast = boost::beast;
namespace net = boost::asio;
namespace urls = boost::urls;
namespace http = beast::http;
namespace sys = boost::system;

using tcp = net::ip::tcp;
using url_view = boost::url_view;

} // namespace btc
