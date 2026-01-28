#pragma once

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/detail/error_code.hpp>

namespace btc {

using IOContext = boost::asio::io_context;
using TCPSocket = boost::asio::ip::tcp::socket;
using TCPResolver = boost::asio::ip::tcp::resolver;
using Endpoint = boost::asio::ip::tcp::endpoint;
using BoostErrorCode = boost::system::error_code;

} // namespace btc
