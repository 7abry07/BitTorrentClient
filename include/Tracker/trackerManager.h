#pragma once

#include "Torrent/infoHash.h"
#include <boost/asio/io_context.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <cstdint>
#include <expected>
#include <helpers.h>
#include <string>
#include <system_error>
#include <vector>

namespace btc {

class Peer {
  std::string peerid;
};

enum class eventType { None = 0, Completed, Started, Stopped };
enum class requestKind { Announce = 0, Scrape };

struct TrackerRequest {
  urls::url url;
  std::string trackerID;
  std::string infoHash;
  std::string pID;
  std::int64_t downloaded = 0;
  std::int64_t left = 0;
  std::int64_t uploaded = 0;
  std::uint32_t numwant = 50;
  std::string ip;
  std::uint32_t key = 0;
  std::uint16_t port = 0;
  bool no_pID = 0;
  bool compact = true;

  eventType event = eventType::None;
  requestKind kind;
};

struct TrackerResponse {
  std::string failure;
  std::string warning;
  std::string trackerID;
  std::uint32_t interval = 0;
  std::uint32_t minInterval = 0;
  std::uint64_t complete = 0;
  std::uint64_t incomplete = 0;
  std::uint64_t downloaded = 0;
  std::vector<Peer> peerList;
};

class TrackerManager {

private:
  using exp_tracker_resp = std::expected<TrackerResponse, std::error_code>;
  using await_exp_tracker_resp = net::awaitable<exp_tracker_resp>;

public:
  TrackerManager(net::io_context &ctx) : ctx(ctx) {}
  await_exp_tracker_resp send(TrackerRequest req);

private:
  net::io_context &ctx;

  void appendQuery(std::string &fullq, std::string k, std::string v,
                   bool first = false);
  void appendQuery(std::string &fullq, std::string k, std::int64_t v,
                   bool first = false);
  await_exp_tracker_resp httpSend(TrackerRequest req);
  exp_tracker_resp
  parseHttpResponse(const http::response<http::dynamic_body> &resp);
};

} // namespace btc
