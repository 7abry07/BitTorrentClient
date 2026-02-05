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

  if (req.kind == requestKind::announce) {
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
      parseHttp(beast::buffers_to_string(httpResp->body().cdata()), req);
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

TrackerManager::exp_tracker_resp
TrackerManager::parseHttp(const std::span<char const> &resp,
                          TrackerRequest &req) {
  BencodeDecoder decoder;
  TrackerResponse trackerResp;

  auto nodeRes = decoder.decode(beast::buffers_to_string(resp));

  if (!(nodeRes && nodeRes->isDict()))
    return std::unexpected(error_code::invalidTrackerResponseErr);

  trackerResp.interval = nodeRes->dictFindInt("interval", 1800).getInt();
  trackerResp.minInterval = nodeRes->dictFindInt("min interval", 30).getInt();
  trackerResp.trackerID = nodeRes->dictFindString("tracker id", "").getStr();
  trackerResp.warning = nodeRes->dictFindString("warning reason", "").getStr();
  trackerResp.failure = nodeRes->dictFindString("failure reason", "").getStr();
  if (!trackerResp.failure.empty())
    return trackerResp;

  if (req.kind == requestKind::scrape) {
    auto filesRes = nodeRes->dictFindDict("files");
    if (!filesRes)
      return std::unexpected(error_code::invalidTrackerResponseErr);
    auto fileRes = filesRes->dictFindDict(req.infoHash);
    if (!fileRes)
      return std::unexpected(error_code::invalidTrackerResponseErr);

    trackerResp.complete = fileRes->dictFindInt("complete", -1).getInt();
    trackerResp.incomplete = fileRes->dictFindInt("incomplete", -1).getInt();
    trackerResp.downloaded = fileRes->dictFindInt("downloaded", -1).getInt();

    return trackerResp;
  }

  trackerResp.complete = nodeRes->dictFindInt("complete", -1).getInt();
  trackerResp.incomplete = nodeRes->dictFindInt("incomplete", -1).getInt();
  trackerResp.downloaded = nodeRes->dictFindInt("downloaded", -1).getInt();

  auto peerNodeRes = nodeRes->dictFind("peers");
  if (peerNodeRes && peerNodeRes->isStr()) {
    auto peerRes = parseCompactPeersHttp(nodeRes.value());
    if (peerRes)
      trackerResp.peerList = peerRes.value();
  } else if (peerNodeRes && peerNodeRes->isDict()) {
    auto peerRes = parsePeersHttp(nodeRes.value());
    if (peerRes)
      trackerResp.peerList = peerRes.value();
  } else
    return std::unexpected(error_code::invalidTrackerResponseErr);
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
