#include "Bencode/bencodeDecoder.h"
#include "Torrent/torrentFile.h"
#include "Torrent/torrentParser.h"
#include <Tracker/trackerManager.h>
#include <boost/url/parse.hpp>
#include <print>
#include <vector>

btc::net::awaitable<void> session(btc::net::io_context &ctx) {
  btc::TorrentParser parser;
  btc::BencodeDecoder decoder;
  auto fileRes = parser.parseFile("../testFiles/naruto.torrent", decoder);
  if (!fileRes) {
    std::println("{}", fileRes.error().message());
    co_return;
  }
  btc::TorrentFile file = fileRes.value();
  btc::TrackerManager tm(ctx);
  btc::TrackerRequest req{};

  std::vector<btc::urls::url> announces;
  for (auto &item : *file.getAnnounceList()) {
    auto res = btc::urls::parse_uri(item);
    if (!res) {
      std::println("{}", res.error().message());
      co_return;
    }
    announces.push_back(res.value());
  }

  int pieces = 0;
  for (auto torrFile : *file.getFiles()) {
    pieces += torrFile.length / file.getPieceLength();
  }

  req.infoHash = file.getInfoHash();
  req.pID = "abcdefghijklmnopqrst";
  req.port = 6881;
  req.left = pieces;

  for (auto ann : announces) {
    req.url = ann;

    std::println("{}", std::string(ann.buffer()));

    auto resp = co_await tm.send(req);
    if (!resp) {
      std::println("{}", resp.error().message());
    } else if (resp->failure != "") {
      std::println("{}", resp->failure);
    } else if (resp->warning != "") {
      std::println("{}", resp->failure);
    }
  }
}

int main() {
  btc::net::io_context ctx;
  btc::net::co_spawn(ctx, session(ctx), btc::net::detached);
  ctx.run();
}
