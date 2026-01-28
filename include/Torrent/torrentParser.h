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
  static exp_string parseAnnounce(b_dict root);

  static exp_sizet parsePieceLength(b_dict info);
  static exp_string parseName(b_dict info);
  static exp_string parsePieces(b_dict info);
  static exp_filemode validateFileMode(b_dict info);
  static exp_sizet parseSingle(b_dict info);

  static exp_fileinfo parseFile(b_dict file);
  static exp_sizet parseFileLength(b_dict file);
  static exp_string parseFilePath(b_dict file);
  static exp_files parseMultiple(b_dict info);

  static opt_string parseComment(b_dict root);
  static opt_string parseCreatedBy(b_dict root);
  static opt_string parseEncoding(b_dict root);
  static opt_date parseCreationDate(b_dict root);
  static opt_stringlist parseAnnounceList(b_dict root);
};

} // namespace btc
