#pragma once

#include "bencode.h"
#include <chrono>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace btc::Torrent {

class Error {

public:
  enum err_code {
    errorOpeningFileErr,
    missingAnnounceKeyErr,
    missingInfoKeyErr,
    missingNameFieldErr,
    missingPieceLengthFieldErr,
    missingPiecesFieldErr,

    rootStructureNotDictErr,
    announceKeyNotMappedToStringErr,
    infoKeyNotMappedToDictErr,
    nameFieldNotMappedToStrErr,
    pieceLengthFieldNotMappedToIntErr,
    piecesFieldNotMapppedToStrErr,
    lengthFieldNotMappedToIntErr,
    filesFieldNotMappedToListErr,
    lengthFieldInFilesItemNotMappedToIntErr,
    pathFieldInFilesItemNotMappedToListErr,
    pathFragmentInFilesItemNotMappedToStrErr,

    pieceLengthNegativeErr,
    pieceLengthZeroErr,

    piecesFieldLengthNonDivisibleBy20Err,

    filesFieldItemIsNotDictErr,
    missingLengthFieldInFilesItemErr,
    missingPathFieldInFilesItemErr,

    bothLengthAndFilesFieldsMissingErr,
    bothLengthAndFilesFieldsPresentErr,
  };

  Error(err_code code);
  std::string get();

  err_code code;

private:
  const std::unordered_map<err_code, std::string> err_mess = {
      {rootStructureNotDictErr,
       "the root structure is not mapped to a dictionary"},
      {missingAnnounceKeyErr, "Announce key is missing."},
      {missingInfoKeyErr, "Info key is missing."},
      {missingNameFieldErr, "Name field is missing in the info dictionary."},
      {missingPieceLengthFieldErr, "Piece length field is missing."},
      {missingPiecesFieldErr, "Pieces field is missing."},

      {announceKeyNotMappedToStringErr, "Announce key is not a string."},
      {infoKeyNotMappedToDictErr, "Info key is not mapped to a dictionary."},
      {nameFieldNotMappedToStrErr, "Name field is not a string."},
      {pieceLengthFieldNotMappedToIntErr,
       "Piece length field is not an integer."},
      {piecesFieldNotMapppedToStrErr, "Pieces field is not a string."},
      {lengthFieldNotMappedToIntErr, "Length field is not an integer."},
      {filesFieldNotMappedToListErr, "Files field is not a list."},
      {lengthFieldInFilesItemNotMappedToIntErr,
       "Length field in files item is not an integer."},
      {pathFieldInFilesItemNotMappedToListErr,
       "Path field in files item is not a list."},
      {pathFragmentInFilesItemNotMappedToStrErr,
       "path fragment inside file dict in file list is not a string"},

      {pieceLengthNegativeErr, "Piece length is negative."},
      {pieceLengthZeroErr, "Piece length is zero."},

      {piecesFieldLengthNonDivisibleBy20Err,
       "Pieces field length is not divisible by 20."},

      {filesFieldItemIsNotDictErr,
       "An item in files field is not a dictionary."},
      {missingLengthFieldInFilesItemErr, "Length field missing in files item."},
      {missingPathFieldInFilesItemErr, "Path field missing in files item."},

      {bothLengthAndFilesFieldsMissingErr,
       "Both length and files fields are missing."},
      {bothLengthAndFilesFieldsPresentErr,
       "Both length and files fields are present; only one allowed."},
  };
};

typedef struct {
  std::size_t length;
  std::filesystem::path path;
} FileInfo;

typedef struct TorrentInfo {
  std::string name{};
  std::size_t pieceLength{};
  std::string pieces{};
  bool private_{};

  std::optional<std::size_t> length{};
  std::optional<std::vector<FileInfo>> files{};
} TorrentInfo;

class TorrentFile {

private:
  using date = std::chrono::year_month_day;

  std::string announce{};
  std::optional<std::vector<std::string>> announceList{};
  TorrentInfo info{};
  std::optional<date> creationDate{};
  std::optional<std::string> comment{};
  std::optional<std::string> createdBy{};
  std::optional<std::string> encoding{};
  std::string infoHash{};

public:
  TorrentFile() {}

  void setAnnounce(std::string);
  void setAnnounceList(std::vector<std::string>);
  void setInfo(TorrentInfo);
  void setCreationDate(date);
  void setComment(std::string);
  void setCreatedBy(std::string);
  void setEncoding(std::string);
};

class TorrentParser {
private:
  using torrentError = std::variant<Error, Bencode::Error>;

public:
  static std::expected<TorrentFile, torrentError>
  parseFile(std::filesystem::path path, Bencode::Decoder decoder);

private:
  static std::vector<FileInfo> parseFiles(Bencode::Value files);
  static std::vector<std::string>
  parseAnnounceList(Bencode::Value announceList);
};

} // namespace btc::Torrent
