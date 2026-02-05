#include <Bencode/bencodeDecoder.h>
#include <Torrent/torrentFile.h>
#include <Torrent/torrentParser.h>
#include <Tracker/trackerManager.h>
#include <helpers.h>
#include <print>

#define TEST_PATH "testFiles/naruto.torrent"

namespace urls = btc::urls;
using torrentParser = btc::TorrentParser;
using torrentFile = btc::TorrentFile;
using bencodeDecoder = btc::BencodeDecoder;
using trackerManager = btc::TrackerManager;
using trackerRequest = btc::TrackerRequest;
using ioctx = btc::net::io_context;

btc::net::awaitable<void> session(btc::net::io_context &io) {

  bencodeDecoder decoder;
  torrentParser parser;
  trackerManager manager(io);

  auto res = parser.parseFile(TEST_PATH, decoder);
  if (!res) {
    std::println("error -> {}", res.error().message());
    co_return;
  }

  torrentFile file = res.value();

  trackerRequest req{};
  req.setInfoHash(file.getInfoHash());
  req.setKind(btc::requestKind::announce);
  req.setPID("dsghbfevwevnunwp9gnw");
  req.setCompact(true);

  auto urlres = urls::parse_uri(file.getAnnounce());
  if (!urlres) {
    std::println("error -> {}", urlres.error().message());
    co_return;
  }
  req.setUrl(urlres.value());

  auto respRes = co_await manager.send(req);
  if (!respRes)
    std::println("error -> {}", respRes.error().message());
  else if (respRes->isFailure())
    std::println("failure -> {}", respRes->getFailure());
  else if (respRes->isWarning())
    std::println("warning -> {}", respRes->getWarning());
  else {
    for (auto peer : respRes->getPeerList())
      std::println("<{}> : [{}]", peer.ip, peer.port);
    std::println("{}", respRes->getComplete());
    std::println("{}", respRes->getIncomplete());
    std::println("{}", respRes->getDownloaded());
  }
}

int main() {
  ioctx io;
  btc::net::co_spawn(io, session(io), btc::net::detached);
  io.run();
}
