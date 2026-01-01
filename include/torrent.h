#pragma once

#include "bencode.h"
#include "errors.h"
#include <chrono>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace btc::Torrent {

enum class FileMode { single, multiple };

typedef struct {
  std::size_t length;
  std::filesystem::path path;
} FileInfo;

class TorrentFile {

private:
  using Date = std::chrono::year_month_day;

  std::string announce{};
  std::string name{};
  std::size_t pieceLength{};
  std::string pieces{};
  std::string infoHash{};

  bool private_{};

  std::optional<std::size_t> length{};
  std::optional<std::vector<FileInfo>> files{};
  std::optional<std::vector<std::string>> announceList{};
  std::optional<Date> creationDate{};
  std::optional<std::string> comment{};
  std::optional<std::string> createdBy{};
  std::optional<std::string> encoding{};

  TorrentFile() {}

  friend class TorrentParser;

public:
  const std::string &getAnnounce() const { return announce; }
  const std::string &getName() const { return name; }
  const std::string &getPieces() const { return pieces; }
  const std::string &getInfoHash() const { return infoHash; }

  std::size_t getPieceLength() const { return pieceLength; }
  bool isPrivate() const { return private_; }

  const std::optional<std::size_t> &getLength() const { return length; }
  const std::optional<std::vector<FileInfo>> &getFiles() const { return files; }
  const std::optional<Date> &getCreationDate() const { return creationDate; }
  const std::optional<std::string> &getComment() const { return comment; }
  const std::optional<std::string> &getCreatedBy() const { return createdBy; }
  const std::optional<std::string> &getEncoding() const { return encoding; }
  const std::optional<std::vector<std::string>> &getAnnounceList() const {
    return announceList;
  }
};

class TorrentParser {
public:
  static std::expected<TorrentFile, Error> parseFile(std::filesystem::path path,
                                                     Bencode::Decoder decoder);
  static std::expected<TorrentFile, Error>
  parseContent(std::string content, Bencode::Decoder decoder);

private:
  using Date = std::chrono::year_month_day;
  using BencodeDict = Bencode::Value::Dict;

  static std::expected<std::string, Error> parseAnnounce(BencodeDict root);

  static std::expected<std::size_t, Error> parsePieceLength(BencodeDict info);
  static std::expected<std::string, Error> parseName(BencodeDict info);
  static std::expected<std::string, Error> parsePieces(BencodeDict info);
  static std::expected<FileMode, Error> validateFileMode(BencodeDict info);

  static std::expected<std::size_t, Error> parseSingle(BencodeDict info);
  static std::expected<std::vector<FileInfo>, Error>
  parseMultiple(BencodeDict info);

  static std::expected<FileInfo, Error> parseFile(BencodeDict info);

  static std::optional<std::string> parseComment(BencodeDict root);
  static std::optional<std::string> parseCreatedBy(BencodeDict root);
  static std::optional<std::string> parseEncoding(BencodeDict root);
  static std::optional<Date> parseCreationDate(BencodeDict root);
  static std::optional<std::vector<std::string>>
  parseAnnounceList(BencodeDict root);
};

} // namespace btc::Torrent
