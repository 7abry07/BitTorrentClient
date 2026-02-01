#include "Bencode/bencodeDecoder.h"
#include <Net/httpConnection.h>
#include <Tracker/trackerManager.h>
#include <array>
#include <cstdint>
#include <errors.h>
#include <expected>
#include <print>
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

  std::array<std::string, 3> eventStr{"completed", "started", "stopped"};

  appendQuery(q, "info_hash", req.infoHash, true);
  appendQuery(q, "peer_id", req.pID);
  appendQuery(q, "port", req.port);
  appendQuery(q, "uploaded", req.uploaded);
  appendQuery(q, "downloaded", req.downloaded);
  appendQuery(q, "left", req.left);
  appendQuery(q, "compact", req.compact);
  appendQuery(q, "no_peer_id", req.no_pID);
  if (req.event != eventType::None)
    appendQuery(q, "event", eventStr[static_cast<int>(req.event) - 1]);
  if (req.ip != "")
    appendQuery(q, "ip", req.ip);
  appendQuery(q, "numwant", req.numwant);
  if (req.key != 0)
    appendQuery(q, "key", req.key);
  if (req.trackerID != "")
    appendQuery(q, "trackerid", req.trackerID);

  url.set_encoded_query(std::string_view(q));

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
    return std::unexpected(error_code::trackerResponseNotDictErr);

  b_dict root = nodeRes.value().getDict();
  if (root.contains("failure reason") && root.at("failure reason").isStr()) {
    trackerResp.failure = root.at("failure reason").getStr();
    return trackerResp;
  }
  if (root.contains("warning reason") && root.at("warning reason").isStr())
    trackerResp.failure = root.at("warning reason").getStr();

  trackerResp.interval = root.at("interval").getInt();
  if (root.contains("min interval") && root.at("min interval").isInt())
    trackerResp.minInterval = root.at("min interval").getInt();
  if (root.contains("tracker id") && root.at("tracker id").isInt())
    trackerResp.trackerID = root.at("tracker id").getStr();
  trackerResp.complete = root.at("complete").getInt();
  trackerResp.incomplete = root.at("incomplete").getInt();

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
