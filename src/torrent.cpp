#include "torrent.h"
#include "bencode.h"
#include "errors.h"
#include <expected>
#include <fstream>
#include <iterator>
#include <openssl/sha.h>
#include <optional>
#include <print>
#include <vector>

namespace btc::Torrent {

// ---------------------------------------------------
// TORRENT PARSER
// ---------------------------------------------------
std::expected<TorrentFile, Error>
TorrentParser::parseFile(std::filesystem::path path, Bencode::Decoder decoder) {
  std::fstream file(path);
  if (!file)
    return std::unexpected(Error::errorOpeningFileErr);

  std::string content{(std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>()};

  if (!content.empty() && content.back() == '\n')
    content.pop_back();

  return parseContent(content, decoder);
}

std::expected<TorrentFile, Error>
TorrentParser::parseContent(std::string content, Bencode::Decoder decoder) {
  auto bencodeRes = decoder.decode(content);
  if (!bencodeRes)
    return std::unexpected(bencodeRes.error());

  if (!bencodeRes->isDict())
    return std::unexpected(Error::rootStructureNotDictErr);

  BencodeDict root = bencodeRes->getDict();

  if (!root.contains("info"))
    return std::unexpected(Error::missingInfoKeyErr);
  if (!root.at("info").isDict())
    return std::unexpected(Error::infoKeyNotDictErr);
  BencodeDict info = root.at("info").getDict();

  auto announceRes = parseAnnounce(root);
  if (!announceRes)
    return std::unexpected(announceRes.error());

  auto nameRes = parseName(info);
  if (!nameRes)
    return std::unexpected(nameRes.error());

  auto piecesRes = parsePieces(info);
  if (!piecesRes)
    return std::unexpected(piecesRes.error());

  auto pieceLengthRes = parsePieceLength(info);
  if (!pieceLengthRes)
    return std::unexpected(pieceLengthRes.error());

  auto fileModeRes = validateFileMode(info);
  if (!fileModeRes)
    return std::unexpected(fileModeRes.error());

  std::size_t length{};
  std::vector<FileInfo> files{};

  switch (*fileModeRes) {
  case FileMode::single: {
    auto lengthRes = parseSingle(info);
    if (!lengthRes)
      return std::unexpected(lengthRes.error());
    length = *lengthRes;
    break;
  }

  case FileMode::multiple: {
    auto filesRes = parseMultiple(info);
    if (!filesRes)
      return std::unexpected(filesRes.error());
    files = std::move(*filesRes);
    break;
  }
  }

  auto announceListRes = parseAnnounceList(root);
  auto commentRes = parseComment(root);
  auto createdByRes = parseCreatedBy(root);
  auto encodingRes = parseEncoding(root);
  auto creationDateRes = parseCreationDate(root);

  TorrentFile file;
  file.announce = *announceRes;
  file.announceList = announceListRes;
  file.comment = commentRes;
  file.createdBy = createdByRes;
  file.encoding = encodingRes;
  file.creationDate = creationDateRes;
  file.comment = file.name = *nameRes;
  file.pieces = *piecesRes;
  file.pieceLength = *pieceLengthRes;
  file.announceList = announceListRes;
  file.length =
      (*fileModeRes == FileMode::single) ? std::optional(length) : std::nullopt;
  file.files = (*fileModeRes == FileMode::multiple) ? std::optional(files)
                                                    : std::nullopt;

  unsigned char hash[20];
  Bencode::Encoder encoder;
  std::string infoBencode = encoder.encode(static_cast<Bencode::Value>(info));

  SHA1(reinterpret_cast<const unsigned char *>(infoBencode.data()),
       infoBencode.size(), hash);

  file.infoHash = std::string(reinterpret_cast<char *>(hash), 20);

  return file;
}

std::expected<std::string, Error>
TorrentParser::parseAnnounce(BencodeDict root) {
  if (!root.contains("announce"))
    return std::unexpected(Error::missingAnnounceKeyErr);
  if (!root.at("announce").isStr())
    return std::unexpected(Error::announceKeyNotStrErr);
  return root.at("announce").getStr();
}

std::expected<std::size_t, Error>
TorrentParser::parsePieceLength(BencodeDict info) {
  if (!info.contains("piece length"))
    return std::unexpected(Error::missingPieceLengthFieldErr);
  if (!info.at("piece length").isInt())
    return std::unexpected(Error::pieceLengthFieldNotIntErr);
  if (info.at("piece length").getInt() < 0)
    return std::unexpected(Error::pieceLengthNegativeErr);
  if (info.at("piece length").getInt() == 0)
    return std::unexpected(Error::pieceLengthZeroErr);
  return info.at("piece length").getInt();
}

std::expected<std::string, Error> TorrentParser::parseName(BencodeDict info) {
  if (!info.contains("name"))
    return std::unexpected(Error::missingNameFieldErr);
  if (!info.at("name").isStr())
    return std::unexpected(Error::nameFieldNotStrErr);
  return info.at("name").getStr();
}

std::expected<std::string, Error> TorrentParser::parsePieces(BencodeDict info) {
  if (!info.contains("pieces"))
    return std::unexpected(Error::missingPiecesFieldErr);
  if (!info.at("pieces").isStr())
    return std::unexpected(Error::piecesFieldNotStrErr);
  return info.at("pieces").getStr();
}

std::expected<FileMode, Error>
TorrentParser::validateFileMode(BencodeDict info) {
  if (!info.contains("length") && !info.contains("files"))
    return std::unexpected(Error::bothLengthAndFilesFieldsMissingErr);
  if (info.contains("length") && info.contains("files"))
    return std::unexpected(Error::bothLengthAndFilesFieldsPresentErr);
  return (info.contains("length")) ? FileMode::single : FileMode::multiple;
}

std::expected<std::size_t, Error> TorrentParser::parseSingle(BencodeDict info) {
  if (!info.at("length").isInt())
    return std::unexpected(Error::lengthFieldNotIntErr);
  if (info.at("length").getInt() < 0)
    return std::unexpected(Error::singleLengthNegativeErr);
  if (info.at("length").getInt() == 0)
    return std::unexpected(Error::singleLengthZeroErr);
  return info.at("length").getInt();
}

std::expected<std::vector<FileInfo>, Error>
TorrentParser::parseMultiple(BencodeDict info) {
  if (!info.at("files").isList())
    return std::unexpected(Error::filesFieldNotListErr);

  std::vector<FileInfo> files;
  auto fileList = info.at("files").getList();

  for (auto file : fileList) {
    if (!file.isDict())
      return std::unexpected(Error::filesFieldItemNotDictErr);

    if (!file.getDict().contains("length"))
      return std::unexpected(Error::missingFileLengthErr);
    if (!file.getDict().at("length").isInt())
      return std::unexpected(Error::fileLengthNotIntErr);
    if (file.getDict().at("length").getInt() < 0)
      return std::unexpected(Error::multiLengthNegativeErr);
    if (file.getDict().at("length").getInt() == 0)
      return std::unexpected(Error::multiLengthZeroErr);
    if (!file.getDict().contains("path"))
      return std::unexpected(Error::missingFilePathErr);
    if (!file.getDict().at("path").isList())
      return std::unexpected(Error::missingFilePathErr);

    auto path = file.getDict().at("path").getList();

    std::size_t fileLength = file.getDict().at("length").getInt();
    std::string strPath = "";

    for (auto item : path) {
      if (!item.isStr())
        return std::unexpected(Error::filePathFragmentNotStrErr);
      strPath.append(item.getStr());
    }

    files.push_back({fileLength, strPath});
  }
  return files;
}

std::optional<std::string> TorrentParser::parseComment(BencodeDict root) {
  if (!root.contains("comment"))
    return std::nullopt;
  if (!root.at("comment").isStr())
    return std::nullopt;
  return root.at("comment").getStr();
}
std::optional<std::string> TorrentParser::parseCreatedBy(BencodeDict root) {
  if (!root.contains("created by"))
    return std::nullopt;
  if (!root.at("created by").isStr())
    return std::nullopt;
  return root.at("created by").getStr();
}
std::optional<std::string> TorrentParser::parseEncoding(BencodeDict root) {
  if (!root.contains("encoding"))
    return std::nullopt;
  if (!root.at("encoding").isStr())
    return std::nullopt;
  return root.at("encoding").getStr();
}
std::optional<TorrentParser::Date>
TorrentParser::parseCreationDate(BencodeDict root) {
  if (!root.contains("creation date"))
    return std::nullopt;
  if (!root.at("creation date").isInt())
    return std::nullopt;

  auto tp = std::chrono::system_clock::time_point{
      std::chrono::seconds{root.at("creation date").getInt()}};
  auto days = std::chrono::floor<std::chrono::days>(tp);
  return std::chrono::year_month_day{days};
}

std::optional<std::vector<std::string>>
TorrentParser::parseAnnounceList(BencodeDict root) {
  if (!root.contains("announce-list"))
    return std::nullopt;
  if (!root.at("announce-list").isList())
    return std::nullopt;

  std::vector<std::string> trackers;

  auto topLevelList = root.at("announce-list").getList();

  for (auto &list : topLevelList) {
    if (!list.isList())
      return std::nullopt;
    for (auto &tracker : list.getList()) {
      if (!tracker.isStr())
        return std::nullopt;
      trackers.push_back(tracker.getStr());
    }
  }
  return trackers;
}
} // namespace btc::Torrent
