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
  req.infoHash = file.getInfoHash();
  req.kind = btc::requestKind::Announce;
  req.pID = "dsghbfevwevnunwp9gnw";
  req.compact = true;
  auto urlres = urls::parse_uri(file.getAnnounce());
  if (!urlres) {
    std::println("error -> {}", urlres.error().message());
    co_return;
  }
  req.url = urlres.value();

  auto respRes = co_await manager.send(req);
  if (!respRes)
    std::println("error -> {}", respRes.error().message());
  if (respRes->failure != "")
    std::println("failure -> {}", respRes->failure);
  if (respRes->warning != "")
    std::println("warning -> {}", respRes->warning);
  else {
    for (auto peer : respRes->peerList)
      std::println("{} : {}", peer.ip, peer.port);
  }
}

int main() {
  ioctx io;
  btc::net::co_spawn(io, session(io), btc::net::detached);
  io.run();
}
