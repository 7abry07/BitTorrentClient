#include "torrent.h"
#include "bencode.h"
#include <expected>

namespace btc::Torrent {

// ---------------------------------------------------
// TORRENT PARSER
// ---------------------------------------------------
std::expected<TorrentFile, Error>
TorrentParser::parseFile(std::filesystem::path path, Bencode::Decoder decoder) {
}

std::vector<FileInfo> TorrentParser::parseFiles(Bencode::Value files) {}
std::vector<std::string>
TorrentParser::parseAnnounceList(Bencode::Value announceList) {}

} // namespace btc::Torrent
  // namespace btc::Torrent
