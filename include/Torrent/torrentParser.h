#pragma once

#include <Bencode/bencodeDecoder.h>
#include <Torrent/torrentFile.h>
#include <chrono>
#include <errors.h>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace btc {

class TorrentParser {

public:
  static std::expected<TorrentFile, Error> parseContent(std::string content,
                                                        BencodeDecoder decoder);
  static std::expected<TorrentFile, Error> parseFile(std::filesystem::path path,
                                                     BencodeDecoder decoder);

private:
  using Date = std::chrono::year_month_day;
  using BencodeDict = BencodeValue::Dict;

  static std::expected<std::string, Error> parseAnnounce(BencodeDict root);

  static std::expected<std::size_t, Error> parsePieceLength(BencodeDict info);
  static std::expected<std::string, Error> parseName(BencodeDict info);
  static std::expected<std::string, Error> parsePieces(BencodeDict info);
  static std::expected<FileMode, Error> validateFileMode(BencodeDict info);
  static std::expected<std::size_t, Error> parseSingle(BencodeDict info);

  static std::expected<FileInfo, Error> parseFile(BencodeDict file);
  static std::expected<std::size_t, Error> parseFileLength(BencodeDict file);
  static std::expected<std::string, Error> parseFilePath(BencodeDict file);
  static std::expected<std::vector<FileInfo>, Error>
  parseMultiple(BencodeDict info);

  static std::optional<std::string> parseComment(BencodeDict root);
  static std::optional<std::string> parseCreatedBy(BencodeDict root);
  static std::optional<std::string> parseEncoding(BencodeDict root);
  static std::optional<Date> parseCreationDate(BencodeDict root);
  static std::optional<std::vector<std::string>>
  parseAnnounceList(BencodeDict root);
};

} // namespace btc
