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
    appendQuery(q, "info_hash", req.infoHash);
    appendQuery(q, "&peer_id", req.pID);
    appendQuery(q, "&port", req.port);
    appendQuery(q, "&uploaded", req.uploaded);
    appendQuery(q, "&downloaded", req.downloaded);
    appendQuery(q, "&left", req.left);
    appendQuery(q, "&compact", req.compact);
    appendQuery(q, "&no_peer_id", req.no_pID);
    appendQuery(q, "&event", eventStr[static_cast<int>(req.event)]);
    appendQuery(q, "&numwant", req.numwant);
    appendQuery(q, "&ip", req.ip);
    appendQuery(q, "&key", req.key);
    appendQuery(q, "&trackerid", req.trackerID);
    url.set_encoded_query(std::string_view(q));
  } else {
    std::string path = url.path();

    if (int announceEnd = path.find_last_of("/announce") != std::string::npos)
      path.replace(announceEnd, 8, "scrape", 6);
    else
      co_return std::unexpected(error_code::scrapeNotSupported);

    url.set_encoded_path(path);
    appendQuery(q, "info_hash", req.infoHash);
    url.set_encoded_query(q);
  }
  auto httpResp = co_await conn->get(url.buffer());
  if (!httpResp)
    co_return std::unexpected(httpResp.error());

  auto resp =
      req.kind == requestKind::Announce
          ? parseAnnounceHttp(
                beast::buffers_to_string(httpResp->body().cdata()))
          : parseScrapeHttp(beast::buffers_to_string(httpResp->body().cdata()),
                            req.infoHash);
  if (!resp)
    co_return std::unexpected(resp.error());
  if (resp->trackerID != "")
    httpUrls.insert_or_assign(req.url.buffer(), resp->trackerID);
  co_return resp;
}

void TrackerManager::appendQuery(std::string &q, std::string k, std::string v) {
  q.append(k + "=" + urls::encode(v, urls::unreserved_chars));
}
void TrackerManager::appendQuery(std::string &q, std::string k,
                                 std::int64_t v) {
  appendQuery(q, k, std::to_string(v));
}

TrackerResponse::exp_tracker_resp
TrackerManager::parseAnnounceHttp(const std::span<char const> &resp) {
  BencodeDecoder decoder;
  TrackerResponse trackerResp;

  auto nodeRes = decoder.decode(beast::buffers_to_string(resp));

  if (!(nodeRes && nodeRes->isDict()))
    return std::unexpected(error_code::invalidTrackerResponseErr);

  BNode interval = nodeRes->dictFindInt("interval", 1800);
  BNode minInterval = nodeRes->dictFindInt("min interval", 30);

  BNode failure = nodeRes->dictFindString("failure reason", "");
  if (!failure.getStr().empty()) {
    trackerResp.failure = failure.getStr();
    return trackerResp;
  }

  BNode trackerID = nodeRes->dictFindString("tracker id", "");
  BNode complete = nodeRes->dictFindInt("complete", -1);
  BNode incomplete = nodeRes->dictFindInt("incomplete", -1);
  BNode warning = nodeRes->dictFindString("warning reason", "");

  BNode peerList;
  BNode peerCompactList = nodeRes->dictFindString("peers", "");
  auto peerListRes = nodeRes->dictFindDict("peers");
  if (peerListRes)
    peerList = peerListRes.value();

  trackerResp.interval = interval.getInt();
  trackerResp.minInterval = minInterval.getInt();
  trackerResp.trackerID = trackerID.getStr();
  trackerResp.complete = complete.getInt();
  trackerResp.incomplete = incomplete.getInt();
  trackerResp.warning = warning.getStr();

  auto peerRes = !peerCompactList.getStr().empty()
                     ? parseCompactPeersHttp(nodeRes.value())
                     : parsePeersHttp(nodeRes.value());
  if (!peerRes)
    return std::unexpected(error_code::invalidTrackerResponseErr);
  trackerResp.peerList = std::move(peerRes.value());

  return trackerResp;
}

TrackerManager::exp_tracker_resp
TrackerManager::parseScrapeHttp(const std::span<char const> &resp,
                                std::string infohash) {
  BencodeDecoder decoder;
  TrackerResponse trackerResp;

  auto nodeRes = decoder.decode(beast::buffers_to_string(resp));

  if (!(nodeRes && nodeRes->isDict()))
    return std::unexpected(error_code::invalidTrackerResponseErr);

  auto filesRes = nodeRes->dictFindDict("files");
  if (!filesRes)
    return std::unexpected(error_code::invalidTrackerResponseErr);
  auto fileRes = filesRes->dictFindDict(infohash);
  if (!fileRes)
    return std::unexpected(error_code::invalidTrackerResponseErr);

  auto completeRes = fileRes->dictFindInt("complete");
  auto incompleteRes = fileRes->dictFindInt("incomplete");
  auto downloadedRes = fileRes->dictFindInt("downloaded");

  if (!completeRes || !incompleteRes || !downloadedRes)
    return std::unexpected(error_code::invalidTrackerResponseErr);

  trackerResp.complete = completeRes->getInt();
  trackerResp.incomplete = incompleteRes->getInt();
  trackerResp.downloaded = downloadedRes->getInt();

  return trackerResp;
}

TrackerManager::opt_peers TrackerManager::parseCompactPeersHttp(BNode root) {
  std::vector<Peer> peerList;
  std::string_view peersView = root.dictFindString("peers", "").getStr();

  while (!peersView.empty()) {
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
    peer.port = port_t(port[1]) | port_t(port[0]) << 8;

    peersView.remove_prefix(6);
    peerList.push_back(peer);
  }
  return peerList;
}

TrackerManager::opt_peers TrackerManager::parsePeersHttp(BNode root) {
  std::vector<Peer> peerList;
  auto peersRes = root.dictFindList("peers");
  for (auto node : peersRes->getList()) {
    Peer peer{};

    if (!node.isDict())
      return std::nullopt;

    auto pIDRes = node.dictFindString("peer id");
    auto ipRes = node.dictFindString("ip");
    auto portRes = node.dictFindInt("port");
    if (!pIDRes || ipRes || portRes)
      return std::nullopt;

    peer.pID = pIDRes->getStr();
    peer.ip = ipRes->getStr();
    peer.port = portRes->getInt();

    peerList.push_back(peer);
  }
  return peerList;
}

} // namespace btc
