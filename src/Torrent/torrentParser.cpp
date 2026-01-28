#include "error_codes.h"
#include <Bencode/bencodeDecoder.h>
#include <Bencode/bencodeEncoder.h>
#include <Torrent/torrentParser.h>
#include <expected>
#include <fstream>
#include <iterator>
#include <openssl/sha.h>
#include <optional>
#include <system_error>
#include <vector>

namespace btc {

std::expected<TorrentFile, std::error_code>
TorrentParser::parseFile(std::filesystem::path path, BencodeDecoder decoder) {
  std::fstream file(path);
  if (!file)
    return std::unexpected(error_code::errorOpeningFileErr);

  std::string content{(std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>()};

  if (!content.empty() && content.back() == '\n')
    content.pop_back();

  return parseContent(content, decoder);
}

std::expected<TorrentFile, std::error_code>
TorrentParser::parseContent(std::string content, BencodeDecoder decoder) {
  auto bencodeRes = decoder.decode(content);
  if (!bencodeRes)
    return std::unexpected(bencodeRes.error());

  if (!bencodeRes->isDict())
    return std::unexpected(error_code::rootStructureNotDictErr);

  BencodeDict root = bencodeRes->getDict();
  if (!root.contains("info"))
    return std::unexpected(error_code::missingInfoKeyErr);
  if (!root.at("info").isDict())
    return std::unexpected(error_code::infoKeyNotDictErr);
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
  file.name = *nameRes;
  file.pieces = *piecesRes;
  file.pieceLength = *pieceLengthRes;
  file.announceList = announceListRes;

  if (*fileModeRes == FileMode::single)
    file.length = length;
  else
    file.files = files;

  unsigned char hash[20];
  BencodeEncoder encoder;
  std::string infoBencode = encoder.encode(static_cast<BencodeValue>(info));

  SHA1(reinterpret_cast<const unsigned char *>(infoBencode.data()),
       infoBencode.size(), hash);

  file.infoHash = std::string(reinterpret_cast<char *>(hash), 20);

  return file;
}

std::expected<std::string, std::error_code>
TorrentParser::parseAnnounce(BencodeDict root) {
  if (!root.contains("announce"))
    return std::unexpected(error_code::missingAnnounceKeyErr);
  if (!root.at("announce").isStr())
    return std::unexpected(error_code::announceKeyNotStrErr);
  return root.at("announce").getStr();
}

std::expected<std::size_t, std::error_code>
TorrentParser::parsePieceLength(BencodeDict info) {
  if (!info.contains("piece length"))
    return std::unexpected(error_code::missingPieceLengthFieldErr);
  if (!info.at("piece length").isInt())
    return std::unexpected(error_code::pieceLengthFieldNotIntErr);
  if (info.at("piece length").getInt() < 0)
    return std::unexpected(error_code::pieceLengthNegativeErr);
  if (info.at("piece length").getInt() == 0)
    return std::unexpected(error_code::pieceLengthZeroErr);
  return info.at("piece length").getInt();
}

std::expected<std::string, std::error_code>
TorrentParser::parseName(BencodeDict info) {
  if (!info.contains("name"))
    return std::unexpected(error_code::missingNameFieldErr);
  if (!info.at("name").isStr())
    return std::unexpected(error_code::nameFieldNotStrErr);
  return info.at("name").getStr();
}

std::expected<std::string, std::error_code>
TorrentParser::parsePieces(BencodeDict info) {
  if (!info.contains("pieces"))
    return std::unexpected(error_code::missingPiecesFieldErr);
  if (!info.at("pieces").isStr())
    return std::unexpected(error_code::piecesFieldNotStrErr);
  return info.at("pieces").getStr();
}

std::expected<FileMode, std::error_code>
TorrentParser::validateFileMode(BencodeDict info) {
  if (!info.contains("length") && !info.contains("files"))
    return std::unexpected(error_code::bothLengthAndFilesFieldsMissingErr);
  if (info.contains("length") && info.contains("files"))
    return std::unexpected(error_code::bothLengthAndFilesFieldsPresentErr);
  return (info.contains("length")) ? FileMode::single : FileMode::multiple;
}

std::expected<std::size_t, std::error_code>
TorrentParser::parseSingle(BencodeDict info) {
  if (!info.at("length").isInt())
    return std::unexpected(error_code::lengthFieldNotIntErr);
  if (info.at("length").getInt() < 0)
    return std::unexpected(error_code::singleLengthNegativeErr);
  if (info.at("length").getInt() == 0)
    return std::unexpected(error_code::singleLengthZeroErr);
  return info.at("length").getInt();
}

std::expected<std::vector<FileInfo>, std::error_code>
TorrentParser::parseMultiple(BencodeDict info) {
  if (!info.at("files").isList())
    return std::unexpected(error_code::filesFieldNotListErr);

  std::vector<FileInfo> files;
  auto fileList = info.at("files").getList();

  for (auto file : fileList) {
    if (!file.isDict())
      return std::unexpected(error_code::filesFieldItemNotDictErr);
    auto fileInfoRes = parseFile(file.getDict());
    if (!fileInfoRes)
      return std::unexpected(fileInfoRes.error());
    files.push_back(*fileInfoRes);
  }
  return files;
}

std::expected<FileInfo, std::error_code>
TorrentParser::parseFile(BencodeDict file) {
  auto fileLenghtRes = parseFileLength(file);
  auto filePathRes = parseFilePath(file);

  if (!fileLenghtRes)
    return std::unexpected(fileLenghtRes.error());
  if (!filePathRes)
    return std::unexpected(filePathRes.error());
  return FileInfo{*fileLenghtRes, *filePathRes};
}

std::expected<std::size_t, std::error_code>
TorrentParser::parseFileLength(BencodeDict file) {
  if (!file.contains("length"))
    return std::unexpected(error_code::missingFileLengthErr);
  if (!file.at("length").isInt())
    return std::unexpected(error_code::fileLengthNotIntErr);
  if (file.at("length").getInt() < 0)
    return std::unexpected(error_code::multiLengthNegativeErr);
  if (file.at("length").getInt() == 0)
    return std::unexpected(error_code::multiLengthZeroErr);
  return file.at("length").getInt();
}

std::expected<std::string, std::error_code>
TorrentParser::parseFilePath(BencodeDict file) {
  if (!file.contains("path"))
    return std::unexpected(error_code::missingFilePathErr);
  if (!file.at("path").isList())
    return std::unexpected(error_code::missingFilePathErr);

  auto path = file.at("path").getList();
  std::string strPath = "";
  for (auto item : path) {
    if (!item.isStr())
      return std::unexpected(error_code::filePathFragmentNotStrErr);
    strPath.append(item.getStr());
  }
  return strPath;
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
} // namespace btc
