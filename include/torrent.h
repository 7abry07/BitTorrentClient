#pragma once

#include "bencode.h"
#include <expected>

namespace btc::Torrent {

class Error {

public:
  enum err_code {
    missingAnnounceKeyErr,
    missingInfoKeyErr,
    missingNameFieldErr,
    missingPieceLengthFieldErr,
    missingPiecesFieldErr,

    announceKeyNotMappedToString,
    infoKeyNotMappedToDictErr,
    nameFieldNotMappedToStrErr,
    pieceLengthFieldNotMappedToIntErr,
    piecesFieldNotMapppedToStrErr,
    lengthFieldNotMappedToIntErr,
    filesFieldNotMappedToList,

    pieceLengthNegativeErr,
    pieceLengthZeroErr,

    piecesFieldLengthNonDivisibleBy20Err,

    filesFieldItemIsNotDictErr,
    missingLengthFieldInFilesItemErr,
    missingPathFieldInFilesItemErr,
    lengthFieldInFilesItemNotMappedToIntErr,
    pathFieldInFilesItemNotMappedToStrErr,

    bothLengthAndFilesFieldsMissingErr,
    bothLengthAndFilesFieldsPresentErr,

  };
  Error(err_code code);
  std::string get();

  err_code code;

private:
  const std::unordered_map<err_code, std::string> err_mess = {
      {missingAnnounceKeyErr, "Announce key is missing."},
      {missingInfoKeyErr, "Info key is missing."},
      {missingNameFieldErr, "Name field is missing in the info dictionary."},
      {missingPieceLengthFieldErr, "Piece length field is missing."},
      {missingPiecesFieldErr, "Pieces field is missing."},

      {announceKeyNotMappedToString, "Announce key is not a string."},
      {infoKeyNotMappedToDictErr, "Info key is not mapped to a dictionary."},
      {nameFieldNotMappedToStrErr, "Name field is not a string."},
      {pieceLengthFieldNotMappedToIntErr,
       "Piece length field is not an integer."},
      {piecesFieldNotMapppedToStrErr, "Pieces field is not a string."},
      {lengthFieldNotMappedToIntErr, "Length field is not an integer."},
      {filesFieldNotMappedToList, "Files field is not a list."},

      {pieceLengthNegativeErr, "Piece length is negative."},
      {pieceLengthZeroErr, "Piece length is zero."},

      {piecesFieldLengthNonDivisibleBy20Err,
       "Pieces field length is not divisible by 20."},

      {filesFieldItemIsNotDictErr,
       "An item in files field is not a dictionary."},
      {missingLengthFieldInFilesItemErr, "Length field missing in files item."},
      {missingPathFieldInFilesItemErr, "Path field missing in files item."},
      {lengthFieldInFilesItemNotMappedToIntErr,
       "Length field in files item is not an integer."},
      {pathFieldInFilesItemNotMappedToStrErr,
       "Path field in files item is not a string."},

      {bothLengthAndFilesFieldsMissingErr,
       "Both length and files fields are missing."},
      {bothLengthAndFilesFieldsPresentErr,
       "Both length and files fields are present; only one allowed."},
  };
};

class Torrent {

private:
  using expected_bool = std::expected<bool, Error>;
  using bencode_dict = btc::Bencode::Value::Dict;
  using bencode_list = btc::Bencode::Value::List;
  using expected_val = std::expected<Torrent, Error>;
  bencode_dict root;

  Torrent(bencode_dict root) : root(root) {}

  static expected_bool isAnnounceValid(bencode_dict *root);
  static expected_bool isInfoValid(bencode_dict *root);
  static expected_bool isNameValid(bencode_dict *info);
  static expected_bool isPieceLengthValid(bencode_dict *info);
  static expected_bool isPiecesValid(bencode_dict *info);
  static expected_bool isLengthOrFilesValid(bencode_dict *info);
  static expected_bool isFilesListValid(bencode_list *files);

public:
  static expected_val validate(bencode_dict *root);
};

} // namespace btc::Torrent
