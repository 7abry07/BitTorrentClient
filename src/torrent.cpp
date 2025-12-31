#include "torrent.h"
#include "bencode.h"
#include <expected>
#include <fstream>
#include <iterator>

namespace btc::Torrent {

// ---------------------------------------------------
// ERROR
// ---------------------------------------------------

Error::Error(err_code code) : code(code) {}
std::string Error::get() { return err_mess.at(code); }

// ---------------------------------------------------
// TORRENT FILE
// ---------------------------------------------------
void TorrentFile::setAnnounce(std::string) {}
void TorrentFile::setAnnounceList(std::vector<std::string>) {}
void TorrentFile::setInfo(TorrentInfo) {}
void TorrentFile::setCreationDate(date) {}
void TorrentFile::setComment(std::string) {}
void TorrentFile::setCreatedBy(std::string) {}
void TorrentFile::setEncoding(std::string) {}

// ---------------------------------------------------
// TORRENT PARSER
// ---------------------------------------------------
std::expected<TorrentFile, TorrentParser::torrentError>
TorrentParser::parseFile(std::filesystem::path path, Bencode::Decoder decoder) {
}

std::vector<FileInfo> TorrentParser::parseFiles(Bencode::Value files) {}
std::vector<std::string>
TorrentParser::parseAnnounceList(Bencode::Value announceList) {}

} // namespace btc::Torrent
  // namespace btc::Torrent
