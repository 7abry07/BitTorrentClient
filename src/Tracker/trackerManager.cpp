#include "Bencode/bencodeValue.h"
#include "error_codes.h"
#include <Bencode/bencodeDecoder.h>
#include <Net/httpConnection.h>
#include <Torrent/peer.h>
#include <Tracker/trackerManager.h>
#include <array>
#include <boost/beast/core/buffers_to_string.hpp>
#include <cstdint>
#include <errors.h>
#include <expected>
#include <optional>
#include <string_view>
#include <sys/types.h>
#include <vector>

namespace btc {

TrackerManager::await_exp_tracker_resp
TrackerManager::send(TrackerRequest req) {
  if (req.url.scheme() == "http") {
    auto resp = co_await httpSend(req);
    co_return !resp ? std::unexpected(resp.error()) : resp;
  } else if (req.url.scheme() == "udp") {
    // TODO UDP
    co_return TrackerResponse();
  }
  co_return std::unexpected(error_code::invalidUrlSchemeErr);
}

TrackerManager::await_exp_tracker_resp
TrackerManager::httpSend(TrackerRequest req) {
  auto conn = co_await HttpConnection::connect(ctx, req.url.host_name(),
                                               req.url.port_number());
  if (!conn)
    co_return std::unexpected(conn.error());

  if (httpUrls.contains(req.url.buffer()))
    req.trackerID = httpUrls.at(req.url.buffer());

  urls::url url = req.url;
  std::string q = "";

  if (req.kind == requestKind::Announce) {
    std::array<std::string, 4> eventStr{"none", "completed", "started",
                                        "stopped"};
    appendQuery(q, "info_hash", req.infoHash, true);
    appendQuery(q, "peer_id", req.pID);
    appendQuery(q, "port", req.port);
    appendQuery(q, "uploaded", req.uploaded);
    appendQuery(q, "downloaded", req.downloaded);
    appendQuery(q, "left", req.left);
    appendQuery(q, "compact", req.compact);
    appendQuery(q, "no_peer_id", req.no_pID);
    appendQuery(q, "event", eventStr[static_cast<int>(req.event)]);
    appendQuery(q, "numwant", req.numwant);
    appendQuery(q, "ip", req.ip);
    appendQuery(q, "key", req.key);
    appendQuery(q, "trackerid", req.trackerID);
    url.set_encoded_query(std::string_view(q));
  } else {
    std::string path = url.path();
    if (int announceEnd = path.find_last_of("/announce") != std::string::npos) {
      path.replace(announceEnd, 8, "scrape", 6);
      url.set_encoded_path(path);
      appendQuery(q, "info_hash", req.infoHash, true);
      url.set_encoded_query(q);
    } else {
      co_return std::unexpected(error_code::scrapeNotSupported);
    }
  }
  auto httpResp = co_await conn->get(url.buffer());
  if (!httpResp)
    co_return std::unexpected(httpResp.error());

  if (req.kind == requestKind::Announce) {
    auto resp = TrackerResponse::parseAnnounceHttp(
        beast::buffers_to_string(httpResp->body().cdata()));
    if (!resp)
      co_return std::unexpected(resp.error());

    if (resp->trackerID != "")
      httpUrls.insert_or_assign(req.url.buffer(), resp->trackerID);
    co_return resp;

  } else if (req.kind == requestKind::Scrape) {
    auto resp = TrackerResponse::parseScrapeHttp(
        beast::buffers_to_string(httpResp->body().cdata()), req.infoHash);

    if (!resp)
      co_return std::unexpected(resp.error());
    co_return resp;
  }
  co_return std::unexpected(error_code::invalidTrackerResponseErr);
}

void TrackerManager::appendQuery(std::string &fullq, std::string k,
                                 std::string v, bool first) {
  fullq.append((first ? "" : "&") + k + "=" +
               urls::encode(v, urls::unreserved_chars));
}

void TrackerManager::appendQuery(std::string &fullq, std::string k,
                                 std::int64_t v, bool first) {
  appendQuery(fullq, k, std::to_string(v), first);
}

// ----------------------------------------------
// TRACKER RESPONSE
// ----------------------------------------------

TrackerResponse::exp_tracker_resp
TrackerResponse::parseAnnounceHttp(const std::span<char const> &resp) {
  BencodeDecoder decoder;
  TrackerResponse trackerResp;

  auto nodeRes = decoder.decode(beast::buffers_to_string(resp));

  if (!(nodeRes && nodeRes->isDict()))
    return std::unexpected(error_code::invalidTrackerResponseErr);

  b_dict root = nodeRes.value().getDict();

  if (!(root.contains("interval") && root.at("interval").isInt()))
    return std::unexpected(error_code::invalidTrackerResponseErr);
  if (!(root.contains("complete") && root.at("complete").isInt()))
    return std::unexpected(error_code::invalidTrackerResponseErr);
  if (!(root.contains("incomplete") && root.at("incomplete").isInt()))
    return std::unexpected(error_code::invalidTrackerResponseErr);
  if (!(root.contains("peers") && root.at("peers").isStr()) &&
      !(root.contains("peers") && root.at("peers").isDict()))
    return std::unexpected(error_code::invalidTrackerResponseErr);

  trackerResp.interval = root.at("interval").getInt();
  trackerResp.complete = root.at("complete").getInt();
  trackerResp.incomplete = root.at("incomplete").getInt();
  trackerResp.minInterval =
      (root.contains("min interval") && root.at("min interval").isInt())
          ? root.at("min interval").getInt()
          : 0;
  trackerResp.trackerID =
      (root.contains("tracker id") && root.at("tracker id").isStr())
          ? root.at("tracker id").getStr()
          : "";
  trackerResp.warning =
      (root.contains("warning reason") && root.at("warning reason").isStr())
          ? root.at("warning reason").getStr()
          : "";

  if (root.contains("peers") && root.at("peers").isStr()) {
    auto peerRes = TrackerResponse::parseCompactPeersHttp(root);
    if (!peerRes)
      return std::unexpected(error_code::invalidTrackerResponseErr);
    trackerResp.peerList = std::move(peerRes.value());

  } else if (root.contains("peers") && root.at("peers").isList()) {
    auto peerRes = TrackerResponse::parsePeersHttp(root);
    if (!peerRes)
      return std::unexpected(error_code::invalidTrackerResponseErr);
    trackerResp.peerList = std::move(peerRes.value());
  } else {
    return std::unexpected(error_code::invalidTrackerResponseErr);
  }
  return trackerResp;
}

TrackerResponse::exp_tracker_resp
TrackerResponse::parseScrapeHttp(const std::span<char const> &resp,
                                 std::string infohash) {
  BencodeDecoder decoder;
  TrackerResponse trackerResp;

  auto nodeRes = decoder.decode(beast::buffers_to_string(resp));

  if (!(nodeRes && nodeRes->isDict()))
    return std::unexpected(error_code::invalidTrackerResponseErr);
  b_dict root = nodeRes.value().getDict();
  if (!(root.contains("files") && root.at("files").isDict()))
    return std::unexpected(error_code::invalidTrackerResponseErr);
  b_dict files = root.at("files").getDict();
  if (!(files.contains(infohash) && files.at(infohash).isDict()))
    return std::unexpected(error_code::invalidTrackerResponseErr);
  b_dict file = files.at(infohash).getDict();

  if (!(file.contains("complete") && file.at("complete").isInt()))
    return std::unexpected(error_code::invalidTrackerResponseErr);
  if (!(file.contains("incomplete") && file.at("incomplete").isInt()))
    return std::unexpected(error_code::invalidTrackerResponseErr);
  if (!(file.contains("downloaded") && file.at("downloaded").isInt()))
    return std::unexpected(error_code::invalidTrackerResponseErr);

  trackerResp.complete = file.at("complete").getInt();
  trackerResp.incomplete = file.at("incomplete").getInt();
  trackerResp.downloaded = file.at("downloaded").getInt();

  return trackerResp;
}

TrackerResponse::opt_peers TrackerResponse::parseCompactPeersHttp(b_dict root) {
  std::vector<Peer> peerList;
  std::string_view peersView = root.at("peers").getStr();

  while (peersView.size() != 0) {
    std::string_view peerView = peersView.substr(0, 6);
    unsigned char ip[4];
    unsigned char port[2];

    ip[0] = static_cast<unsigned char>(peerView.at(0));
    ip[1] = static_cast<unsigned char>(peerView.at(1));
    ip[2] = static_cast<unsigned char>(peerView.at(2));
    ip[3] = static_cast<unsigned char>(peerView.at(3));

    port[0] = static_cast<unsigned char>(peerView.at(4));
    port[1] = static_cast<unsigned char>(peerView.at(5));

    Peer peer{};
    peer.ip = std::format("{}.{}.{}.{}", ip[3], ip[2], ip[1], ip[0]);
    peer.port = port_t(port[0]) | port_t(port[1]) << 8;

    peersView.remove_prefix(6);
    peerList.push_back(peer);
  }
  return peerList;
}

TrackerResponse::opt_peers TrackerResponse::parsePeersHttp(b_dict root) {
  std::vector<Peer> peerList;
  b_list peers = root.at("peers").getList();
  for (auto node : peers) {
    Peer peer{};

    if (!node.isDict())
      return std::nullopt;
    b_dict peerNode = node.getDict();
    if (!(peerNode.contains("peer id") && peerNode.at("peer id").isStr()))
      return std::nullopt;
    if (!(peerNode.contains("ip") && peerNode.at("ip").isStr()))
      return std::nullopt;
    if (!(peerNode.contains("port") && peerNode.at("port").isInt()))
      return std::nullopt;

    peer.pID = peerNode.at("peer id").getStr();
    peer.ip = peerNode.at("ip").getStr();
    peer.port = peerNode.at("port").getInt();

    peerList.push_back(peer);
  }
  return peerList;
}

} // namespace btc
