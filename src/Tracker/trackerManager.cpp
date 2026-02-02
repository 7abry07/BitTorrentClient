#include "Bencode/bencodeDecoder.h"
#include "Bencode/bencodeValue.h"
#include "error_codes.h"
#include <Net/httpConnection.h>
#include <Tracker/trackerManager.h>
#include <array>
#include <cstdint>
#include <errors.h>
#include <expected>
#include <sys/types.h>

namespace btc {

TrackerManager::await_exp_tracker_resp
TrackerManager::send(TrackerRequest req) {
  if (req.url.scheme() == "http") {
    auto resp = co_await httpSend(req);
    if (!resp)
      co_return std::unexpected(resp.error());
    co_return resp;
  } else if (req.url.scheme() == "udp") {
    // TODO
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

    if (req.ip != "")
      appendQuery(q, "ip", req.ip);
    if (req.key != 0)
      appendQuery(q, "key", req.key);
    if (req.trackerID != "")
      appendQuery(q, "trackerid", req.trackerID);

    url.set_encoded_query(std::string_view(q));
  } else {
    // TODO
  }

  auto httpResp = co_await conn->get(url.buffer());
  if (!httpResp)
    co_return std::unexpected(httpResp.error());

  auto resp = parseHttpResponse(httpResp.value());
  if (!resp)
    co_return std::unexpected(resp.error());

  co_return resp;
}

TrackerManager::exp_tracker_resp TrackerManager::parseHttpResponse(
    const http::response<http::dynamic_body> &resp) {
  TrackerResponse trackerResp;
  BencodeDecoder decoder;
  auto nodeRes = decoder.decode(beast::buffers_to_string(resp.body().cdata()));

  if (!nodeRes)
    return std::unexpected(nodeRes.error());
  if (!nodeRes->isDict())
    return std::unexpected(error_code::invalidTrackerResponseErr);

  b_dict root = nodeRes.value().getDict();

  if (root.contains("failure reason") && root.at("failure reason").isStr()) {
    trackerResp.failure = root.at("failure reason").getStr();
    return trackerResp;
  }

  if (!(root.contains("interval") && root.at("interval").isInt()))
    return std::unexpected(error_code::invalidTrackerResponseErr);
  if (!(root.contains("complete") && root.at("complete").isInt()))
    return std::unexpected(error_code::invalidTrackerResponseErr);
  if (!(root.contains("incomplete") && root.at("incomplete").isInt()))
    return std::unexpected(error_code::invalidTrackerResponseErr);
  if (!(root.contains("peers") && root.at("peers").isStr()))
    return std::unexpected(error_code::invalidTrackerResponseErr);

  trackerResp.interval = root.at("interval").getInt();
  trackerResp.complete = root.at("complete").getInt();
  trackerResp.incomplete = root.at("incomplete").getInt();
  trackerResp.minInterval =
      (root.contains("min interval") && root.at("min interval").isInt())
          ? root.at("min interval").getInt()
          : 0;
  trackerResp.trackerID =
      (root.contains("tracker id") && root.at("tracker id").isInt())
          ? root.at("tracker id").getStr()
          : "";
  trackerResp.warning =
      (root.contains("warning reason") && root.at("warning reason").isStr())
          ? root.at("warning reason").getStr()
          : "";

  if (root.contains("peers") && root.at("peers").isStr()) {
    std::string_view peersView = root.at("peers").getStr();

    while (peersView.size() != 0) {
      std::string_view peerView = peersView.substr(0, 6);
      unsigned char ip[4];
      unsigned char port[2];

      ip[0] = peerView.at(0) & 0xff;
      ip[1] = (peerView.at(1) >> 8) & 0xff;
      ip[2] = (peerView.at(2) >> 16) & 0xff;
      ip[3] = (peerView.at(3) >> 24) & 0xff;

      port[0] = peerView.at(0) & 0xff;
      port[1] = (peerView.at(0) >> 8) & 0xff;

      Peer peer{};
      peer.ip = std::format("{}.{}.{}.{}", ip[3], ip[2], ip[1], ip[0]);
      peer.port = port_t(port[0]) | port_t(port[1]) << 8;

      peersView.remove_prefix(6);
      trackerResp.peerList.push_back(peer);
    }
  } else if (root.contains("peers") && root.at("peers").isList()) {
    b_list peers = root.at("peers").getList();
    for (auto node : peers) {
      Peer peer{};

      if (!node.isDict())
        return std::unexpected(error_code::invalidTrackerResponseErr);
      b_dict peerNode = node.getDict();
      if (!(peerNode.contains("peer id") && peerNode.at("peer id").isStr()))
        return std::unexpected(error_code::invalidTrackerResponseErr);
      if (!(peerNode.contains("ip") && peerNode.at("ip").isStr()))
        return std::unexpected(error_code::invalidTrackerResponseErr);
      if (!(peerNode.contains("port") && peerNode.at("port").isInt()))
        return std::unexpected(error_code::invalidTrackerResponseErr);

      peer.pID = peerNode.at("peer id").getStr();
      peer.ip = peerNode.at("ip").getStr();
      peer.port = peerNode.at("port").getInt();

      trackerResp.peerList.push_back(peer);
    }
  }
  return trackerResp;
}

void TrackerManager::appendQuery(std::string &fullq, std::string k,
                                 std::string v, bool first) {
  if (!first)
    fullq.append("&");
  fullq.append(k + "=" + urls::encode(v, urls::unreserved_chars));
}

void TrackerManager::appendQuery(std::string &fullq, std::string k,
                                 std::int64_t v, bool first) {
  appendQuery(fullq, k, std::to_string(v), first);
}

} // namespace btc
