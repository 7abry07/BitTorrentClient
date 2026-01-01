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

typedef struct {
  std::size_t length;
  std::filesystem::path path;
} FileInfo;

class TorrentFile {

private:
  using date = std::chrono::year_month_day;

  std::string announce{};
  std::optional<std::vector<std::string>> announceList{};

  std::string name{};
  std::size_t pieceLength{};
  std::string pieces{};
  bool private_{};

  std::optional<std::size_t> length{};
  std::optional<std::vector<FileInfo>> files{};

  std::optional<date> creationDate{};
  std::optional<std::string> comment{};
  std::optional<std::string> createdBy{};
  std::optional<std::string> encoding{};
  std::string infoHash{};

  TorrentFile() {}

  friend class TorrentParser;

public:
  const std::string &getAnnounce() const { return announce; }
  const std::string &getName() const { return name; }
  std::size_t getPieceLength() const { return pieceLength; }
  const std::string &getPieces() const { return pieces; }
  bool isPrivate() const { return private_; }
  const std::string &getInfoHash() const { return infoHash; }

  const std::optional<std::vector<std::string>> &getAnnounceList() const {
    return announceList;
  }
  const std::optional<std::size_t> &getLength() const { return length; }
  const std::optional<std::vector<FileInfo>> &getFiles() const { return files; }
  const std::optional<date> &getCreationDate() const { return creationDate; }
  const std::optional<std::string> &getComment() const { return comment; }
  const std::optional<std::string> &getCreatedBy() const { return createdBy; }
  const std::optional<std::string> &getEncoding() const { return encoding; }
};

class TorrentParser {
public:
  static std::expected<TorrentFile, Error> parseFile(std::filesystem::path path,
                                                     Bencode::Decoder decoder);

private:
  static std::vector<FileInfo> parseFiles(Bencode::Value files);
  static std::vector<std::string>
  parseAnnounceList(Bencode::Value announceList);
};

} // namespace btc::Torrent
