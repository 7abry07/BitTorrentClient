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
private:
  using exp_torrentfile = std::expected<TorrentFile, std::error_code>;
  using exp_string = std::expected<std::string, std::error_code>;
  using exp_sizet = std::expected<std::size_t, std::error_code>;
  using exp_filemode = std::expected<FileMode, std::error_code>;
  using exp_fileinfo = std::expected<FileInfo, std::error_code>;
  using exp_files = std::expected<std::vector<FileInfo>, std::error_code>;

  using opt_string = std::optional<std::string>;
  using opt_stringlist = std::optional<std::vector<std::string>>;
  using opt_date = std::optional<std::chrono::year_month_day>;

public:
  static exp_torrentfile parseContent(std::string content,
                                      BencodeDecoder decoder);
  static exp_torrentfile parseFile(std::filesystem::path path,
                                   BencodeDecoder decoder);

private:
  static exp_string parseAnnounce(BNode root);

  static exp_sizet parsePieceLength(BNode info);
  static exp_string parseName(BNode info);
  static exp_string parsePieces(BNode info);
  static exp_filemode validateFileMode(BNode info);
  static exp_sizet parseSingle(BNode info);

  static exp_fileinfo parseFile(BNode file);
  static exp_sizet parseFileLength(BNode file);
  static exp_string parseFilePath(BNode file);
  static exp_files parseMultiple(BNode info);

  static opt_string parseComment(BNode root);
  static opt_string parseCreatedBy(BNode root);
  static opt_string parseEncoding(BNode root);
  static opt_date parseCreationDate(BNode root);
  static opt_stringlist parseAnnounceList(BNode root);
};

} // namespace btc
