#pragma once

#include <chrono>
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

} // namespace btc::Torrent
