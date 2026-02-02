#pragma once

#include <Bencode/bencodeValue.h>
#include <Torrent/peer.h>
#include <cstdint>
#include <expected>
#include <helpers.h>
#include <string>
#include <system_error>
#include <vector>

namespace btc {

class TrackerManager;
struct TrackerResponse;
struct TrackerRequest;

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
  friend TrackerManager;

private:
  using http_resp = http::response<http::dynamic_body>;
  using exp_tracker_resp = std::expected<TrackerResponse, std::error_code>;
  using opt_peers = std::optional<std::vector<Peer>>;
  using opt_bdict = std::optional<b_dict>;

public:
  std::string failure;
  std::string warning;
  std::string trackerID;
  std::uint32_t interval = 0;
  std::uint32_t minInterval = 0;
  std::uint64_t complete = 0;
  std::uint64_t incomplete = 0;
  std::uint64_t downloaded = 0;
  std::vector<Peer> peerList;

private:
  static exp_tracker_resp parseAnnounceHttp(const http_resp &resp);
  static opt_bdict validateTopLevAnnounceHttp(const http_resp &resp);

  static opt_peers parsePeersHttp(b_dict root, bool compact = true);
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
};

} // namespace btc
