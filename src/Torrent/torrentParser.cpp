#include "Bencode/bencodeValue.h"
#include <Bencode/bencodeDecoder.h>
#include <Bencode/bencodeEncoder.h>
#include <Torrent/torrentParser.h>
#include <expected>
#include <fstream>
#include <iterator>
#include <openssl/sha.h>
#include <optional>
#include <vector>

namespace btc {

TorrentParser::exp_torrentfile
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

TorrentParser::exp_torrentfile
TorrentParser::parseContent(std::string content, BencodeDecoder decoder) {
  auto bencodeRes = decoder.decode(content);
  if (!bencodeRes)
    return std::unexpected(bencodeRes.error());

  if (!bencodeRes->isDict())
    return std::unexpected(error_code::rootStructureNotDictErr);

  auto infoRes = bencodeRes->dictFindDict("info");
  if (!infoRes)
    return std::unexpected(error_code::infoKeyNotDictErr);

  auto announceRes = parseAnnounce(bencodeRes.value());
  if (!announceRes)
    return std::unexpected(announceRes.error());

  auto nameRes = parseName(infoRes.value());
  if (!nameRes)
    return std::unexpected(nameRes.error());

  auto piecesRes = parsePieces(infoRes.value());
  if (!piecesRes)
    return std::unexpected(piecesRes.error());

  auto pieceLengthRes = parsePieceLength(infoRes.value());
  if (!pieceLengthRes)
    return std::unexpected(pieceLengthRes.error());

  auto fileModeRes = validateFileMode(infoRes.value());
  if (!fileModeRes)
    return std::unexpected(fileModeRes.error());

  std::size_t length{};
  std::vector<FileInfo> files{};

  switch (*fileModeRes) {
  case FileMode::single: {
    auto lengthRes = parseSingle(infoRes.value());
    if (!lengthRes)
      return std::unexpected(lengthRes.error());
    length = *lengthRes;
    break;
  }

  case FileMode::multiple: {
    auto filesRes = parseMultiple(infoRes.value());
    if (!filesRes)
      return std::unexpected(filesRes.error());
    files = std::move(*filesRes);
    break;
  }
  }

  auto announceListRes = parseAnnounceList(bencodeRes.value());
  auto commentRes = parseComment(bencodeRes.value());
  auto createdByRes = parseCreatedBy(bencodeRes.value());
  auto encodingRes = parseEncoding(bencodeRes.value());
  auto creationDateRes = parseCreationDate(bencodeRes.value());

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
  std::string infoBencode = encoder.encode(static_cast<BNode>(infoRes.value()));

  SHA1(reinterpret_cast<const unsigned char *>(infoBencode.data()),
       infoBencode.size(), hash);

  file.infoHash = std::string(reinterpret_cast<char *>(hash), 20);

  return file;
}

TorrentParser::exp_string TorrentParser::parseAnnounce(BNode root) {
  auto announceRes = root.dictFindString("announce");
  if (!announceRes)
    return std::unexpected(error_code::missingAnnounceKeyErr);
  return announceRes->getStr();
}

TorrentParser::exp_sizet TorrentParser::parsePieceLength(BNode info) {
  auto pieceLenRes = info.dictFindInt("piece length");
  if (!pieceLenRes)
    return std::unexpected(error_code::missingPieceLengthFieldErr);

  if (pieceLenRes->getInt() < 0)
    return std::unexpected(error_code::pieceLengthNegativeErr);
  if (pieceLenRes->getInt() == 0)
    return std::unexpected(error_code::pieceLengthZeroErr);
  return pieceLenRes->getInt();
}

TorrentParser::exp_string TorrentParser::parseName(BNode info) {
  auto nameRes = info.dictFindString("name");
  if (!nameRes)
    return std::unexpected(error_code::missingNameFieldErr);
  return nameRes->getStr();
}

TorrentParser::exp_string TorrentParser::parsePieces(BNode info) {
  auto piecesRes = info.dictFindString("pieces");
  if (!piecesRes)
    return std::unexpected(error_code::missingPiecesFieldErr);
  return piecesRes->getStr();
}

TorrentParser::exp_filemode TorrentParser::validateFileMode(BNode info) {
  auto filesRes = info.dictFindList("files");
  auto lengthRes = info.dictFindInt("length");

  if (!lengthRes && !filesRes)
    return std::unexpected(error_code::bothLengthAndFilesFieldsMissingErr);
  if (lengthRes && filesRes)
    return std::unexpected(error_code::bothLengthAndFilesFieldsPresentErr);
  return (lengthRes) ? FileMode::single : FileMode::multiple;
}

TorrentParser::exp_sizet TorrentParser::parseSingle(BNode info) {
  auto lengthRes = info.dictFindInt("length");
  if (!lengthRes)
    return std::unexpected(error_code::lengthFieldNotIntErr);
  if (lengthRes->getInt() < 0)
    return std::unexpected(error_code::singleLengthNegativeErr);
  if (lengthRes->getInt() == 0)
    return std::unexpected(error_code::singleLengthZeroErr);
  return lengthRes->getInt();
}

TorrentParser::exp_files TorrentParser::parseMultiple(BNode info) {
  auto filesRes = info.dictFindList("files");
  if (!filesRes)
    return std::unexpected(error_code::filesFieldNotListErr);

  std::vector<FileInfo> files;

  for (auto file : filesRes->getList()) {
    if (!file.isDict())
      return std::unexpected(error_code::filesFieldItemNotDictErr);
    auto fileInfoRes = parseFile(file);
    if (!fileInfoRes)
      return std::unexpected(fileInfoRes.error());
    files.push_back(*fileInfoRes);
  }
  return files;
}

TorrentParser::exp_fileinfo TorrentParser::parseFile(BNode file) {
  auto fileLenghtRes = parseFileLength(file);
  auto filePathRes = parseFilePath(file);

  if (!fileLenghtRes)
    return std::unexpected(fileLenghtRes.error());
  if (!filePathRes)
    return std::unexpected(filePathRes.error());
  return FileInfo{*fileLenghtRes, *filePathRes};
}

TorrentParser::exp_sizet TorrentParser::parseFileLength(BNode file) {
  auto lengthRes = file.dictFindInt("length");
  if (!lengthRes)
    return std::unexpected(error_code::missingFileLengthErr);
  if (lengthRes->getInt() < 0)
    return std::unexpected(error_code::multiLengthNegativeErr);
  if (lengthRes->getInt() == 0)
    return std::unexpected(error_code::multiLengthZeroErr);
  return lengthRes->getInt();
}

TorrentParser::exp_string TorrentParser::parseFilePath(BNode file) {
  auto pathRes = file.dictFindList("path");
  if (!pathRes)
    return std::unexpected(error_code::missingFilePathErr);

  std::string strPath = "";
  for (auto item : pathRes->getList()) {
    if (!item.isStr())
      return std::unexpected(error_code::filePathFragmentNotStrErr);
    strPath.append(item.getStr());
  }
  return strPath;
}

TorrentParser::opt_string TorrentParser::parseComment(BNode root) {
  auto commentRes = root.dictFindString("comment");
  if (!commentRes)
    return std::nullopt;
  return commentRes->getStr();
}

TorrentParser::opt_string TorrentParser::parseCreatedBy(BNode root) {
  auto createdBy = root.dictFindString("created by");
  if (!createdBy)
    return std::nullopt;
  return createdBy->getStr();
}

TorrentParser::opt_string TorrentParser::parseEncoding(BNode root) {
  auto encoding = root.dictFindString("encoding");
  if (!encoding)
    return std::nullopt;
  return encoding->getStr();
}

TorrentParser::opt_date TorrentParser::parseCreationDate(BNode root) {
  auto creationDate = root.dictFindInt("creation date");
  if (!creationDate)
    return std::nullopt;

  auto tp = std::chrono::system_clock::time_point{
      std::chrono::seconds{creationDate->getInt()}};
  auto days = std::chrono::floor<std::chrono::days>(tp);
  return std::chrono::year_month_day{days};
}

TorrentParser::opt_stringlist TorrentParser::parseAnnounceList(BNode root) {
  auto announceListRes = root.dictFindList("announce-list");
  if (!announceListRes)
    return std::nullopt;

  std::vector<std::string> trackers;

  for (auto &list : announceListRes->getList()) {
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
