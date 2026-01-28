#pragma once

#include <Bencode/bencodeValue.h>
#include <Torrent/torrentFile.h>
#include <chrono>
#include <errors.h>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

namespace btc {

class BencodeDecoder;

class TorrentParser {

public:
  static std::expected<TorrentFile, std::error_code>
  parseContent(std::string content, BencodeDecoder decoder);
  static std::expected<TorrentFile, std::error_code>
  parseFile(std::filesystem::path path, BencodeDecoder decoder);

private:
  using Date = std::chrono::year_month_day;

  static std::expected<std::string, std::error_code>
  parseAnnounce(BencodeDict root);

  static std::expected<std::size_t, std::error_code>
  parsePieceLength(BencodeDict info);
  static std::expected<std::string, std::error_code>
  parseName(BencodeDict info);
  static std::expected<std::string, std::error_code>
  parsePieces(BencodeDict info);
  static std::expected<FileMode, std::error_code>
  validateFileMode(BencodeDict info);
  static std::expected<std::size_t, std::error_code>
  parseSingle(BencodeDict info);

  static std::expected<FileInfo, std::error_code> parseFile(BencodeDict file);
  static std::expected<std::size_t, std::error_code>
  parseFileLength(BencodeDict file);
  static std::expected<std::string, std::error_code>
  parseFilePath(BencodeDict file);
  static std::expected<std::vector<FileInfo>, std::error_code>
  parseMultiple(BencodeDict info);

  static std::optional<std::string> parseComment(BencodeDict root);
  static std::optional<std::string> parseCreatedBy(BencodeDict root);
  static std::optional<std::string> parseEncoding(BencodeDict root);
  static std::optional<Date> parseCreationDate(BencodeDict root);
  static std::optional<std::vector<std::string>>
  parseAnnounceList(BencodeDict root);
};

} // namespace btc
