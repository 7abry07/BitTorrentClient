#pragma once

#include <Bencode/bencodeValue.h>
#include <Torrent/peer.h>
#include <cstdint>
#include <expected>
#include <helpers.h>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

namespace btc {

class TrackerManager;
class TrackerResponse;
class TrackerRequest;

enum class eventType { None = 0, Completed, Started, Stopped };
enum class requestKind { Announce = 0, Scrape };

class TrackerRequest {
  friend TrackerManager;

private:
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
  bool no_pID = false;
  bool compact = true;
  eventType event = eventType::None;
  requestKind kind;

public:
  void setUrl(const urls::url &v) { url = v; }
  void setInfoHash(const std::string &v) { infoHash = v; }
  void setPID(const std::string &v) { pID = v; }
  void setDownloaded(std::int64_t v) { downloaded = v; }
  void setLeft(std::int64_t v) { left = v; }
  void setUploaded(std::int64_t v) { uploaded = v; }
  void setNumwant(std::uint32_t v) { numwant = v; }
  void setIP(const std::string &v) { ip = v; }
  void setPort(std::uint16_t v) { port = v; }
  void setNoPID(bool v) { no_pID = v; }
  void setCompact(bool v) { compact = v; }
  void setEvent(eventType v) { event = v; }
  void setKind(requestKind v) { kind = v; }
};

class TrackerResponse {
  friend TrackerManager;

private:
  using http_resp = http::response<http::dynamic_body>;
  using exp_tracker_resp = std::expected<TrackerResponse, std::error_code>;
  using opt_peers = std::optional<std::vector<Peer>>;
  using opt_bdict = std::optional<b_dict>;

public:
  const std::string &getFailure() const { return failure; }
  bool isFailure() const { return failure != ""; }
  const std::string &getWarning() const { return warning; }
  bool isWarning() const { return warning != ""; }
  const std::string &getTrackerID() const { return trackerID; }
  std::uint32_t getInterval() const { return interval; }
  std::uint32_t getMinInterval() const { return minInterval; }
  std::uint64_t getComplete() const { return complete; }
  std::uint64_t getIncomplete() const { return incomplete; }
  std::uint64_t getDownloaded() const { return downloaded; }
  const std::vector<Peer> &getPeerList() const { return peerList; }

private:
  std::string failure;
  std::string warning;
  std::string trackerID;
  std::uint32_t interval = 0;
  std::uint32_t minInterval = 0;
  std::uint64_t complete = 0;
  std::uint64_t incomplete = 0;
  std::uint64_t downloaded = 0;
  std::vector<Peer> peerList;

  static opt_peers parseCompactPeersHttp(b_dict root);
  static opt_peers parsePeersHttp(b_dict root);

  static exp_tracker_resp parseAnnounceHttp(const http_resp &resp);
  static exp_tracker_resp parseScrapeHttp(const http_resp &resp,
                                          std::string infohash);
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
  std::unordered_map<std::string, std::string> httpUrls;

  await_exp_tracker_resp httpSend(TrackerRequest req);
  void appendQuery(std::string &fullq, std::string k, std::string v,
                   bool first = false);
  void appendQuery(std::string &fullq, std::string k, std::int64_t v,
                   bool first = false);
};

} // namespace btc
