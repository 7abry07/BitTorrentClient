#pragma once

#include <string>
#include <unordered_map>

namespace btc {
enum error_code {
  // ---------------------------------
  // BENCODE PARSER
  // ---------------------------------

  // General Errors
  emptyInputErr = 100,
  invalidInputErr,
  invalidTypeEncounterErr,
  maximumNestingLimitExcedeedErr,
  trailingInputErr,

  // Integer Errors
  invalidIntegerErr = 200,
  missingIntegerTerminatorErr,
  nonDigitCharacterInIntegerErr,
  outOfRangeIntegerErr,

  // String Errors
  invalidStringLengthErr = 300,
  negativeStringLengthErr,
  signedStringLengthErr,
  stringTooLargeErr,
  lengthMismatchErr,
  missingColonErr,

  // List Errors
  invalidListElementErr = 400,
  missingListTerminatorErr,

  // Dictionary Errors
  missingDictTerminatorErr = 500,
  nonStringKeyErr,
  duplicateKeyErr,

  // ---------------------------------
  // TORRENT PARSER
  // ---------------------------------

  errorOpeningFileErr,

  missingAnnounceKeyErr,
  missingInfoKeyErr,
  missingNameFieldErr,
  missingPieceLengthFieldErr,
  missingPiecesFieldErr,
  missingFileLengthErr,
  missingFilePathErr,

  rootStructureNotDictErr,
  announceKeyNotStrErr,
  infoKeyNotDictErr,
  nameFieldNotStrErr,
  pieceLengthFieldNotIntErr,
  piecesFieldNotStrErr,
  filesFieldNotListErr,
  fileLengthNotIntErr,
  lengthFieldNotIntErr,
  filePathNotListErr,
  filePathFragmentNotStrErr,
  filesFieldItemNotDictErr,

  pieceLengthNegativeErr,
  pieceLengthZeroErr,
  singleLengthNegativeErr,
  singleLengthZeroErr,
  multiLengthNegativeErr,
  multiLengthZeroErr,

  piecesFieldLengthNonDivisibleBy20Err,

  bothLengthAndFilesFieldsMissingErr,
  bothLengthAndFilesFieldsPresentErr,

  // ---------------------------------
  // TRACKER
  // ---------------------------------

  invalidUrlSchemeErr,
  invalidTrackerResponseErr
};

static const std::unordered_map<error_code, std::string> err_mess = {

    // ---------------------------------
    // BENCODE PARSER
    // ---------------------------------
    {emptyInputErr, "Input is empty"},
    {invalidInputErr, "Input is invalid"},
    {invalidTypeEncounterErr, "Encountered invalid type"},
    {maximumNestingLimitExcedeedErr, "Maximum nesting limit exceeded"},
    {invalidIntegerErr, "Invalid integer encountered"},
    {missingIntegerTerminatorErr, "Missing integer terminator"},
    {missingListTerminatorErr, "Missing list terminator"},
    {missingDictTerminatorErr, "Missing dictionary terminator"},
    {lengthMismatchErr, "Length does not match expected string value"},
    {nonDigitCharacterInIntegerErr, "Non-digit character found in integer"},
    {nonStringKeyErr, "Dictionary key is not a string"},
    {outOfRangeIntegerErr, "Integer value out of range"},
    {invalidStringLengthErr, "Invalid string length"},
    {negativeStringLengthErr, "String length is negative"},
    {signedStringLengthErr, "Signed string length encountered"},
    {stringTooLargeErr, "String size exceeds maximum allowed"},
    {invalidListElementErr, "Invalid element in list"},
    {trailingInputErr, "Unexpected trailing input"},
    {duplicateKeyErr, "Duplicate key in dictionary"},
    {missingColonErr, "Missing colon after string length"},

    // ---------------------------------
    // TORRENT PARSER
    // ---------------------------------

    {errorOpeningFileErr, "Error opening file"},
    {rootStructureNotDictErr,
     "the root structure is not mapped to a dictionary"},
    {missingAnnounceKeyErr, "Announce key is missing."},
    {missingInfoKeyErr, "Info key is missing."},
    {missingNameFieldErr, "Name field is missing in the info dictionary."},
    {missingPieceLengthFieldErr, "Piece length field is missing."},
    {missingPiecesFieldErr, "Pieces field is missing."},
    {missingFileLengthErr, "Length field missing in files item."},
    {missingFilePathErr, "Path field missing in files item."},

    {announceKeyNotStrErr, "Announce key is not a string."},
    {infoKeyNotDictErr, "Info key is not mapped to a dictionary."},
    {nameFieldNotStrErr, "Name field is not a string."},
    {pieceLengthFieldNotIntErr, "Piece length field is not an integer."},
    {piecesFieldNotStrErr, "Pieces field is not a string."},
    {filesFieldNotListErr, "Files field is not a list."},
    {fileLengthNotIntErr, "Length field in file item is not an integer."},
    {lengthFieldNotIntErr, "Length field is not an integer."},
    {filePathNotListErr, "Path field in file item is not a list."},
    {filePathFragmentNotStrErr, "Path fragment in file item is not a string."},
    {filesFieldItemNotDictErr, "An item in files field is not a dictionary."},

    {pieceLengthNegativeErr, "Piece length is negative."},
    {pieceLengthZeroErr, "Piece length is zero."},
    {singleLengthNegativeErr, "Length field in single-file mode is negative."},
    {singleLengthZeroErr, "Length field in single-file mode is zero."},
    {multiLengthNegativeErr, "Length field in multi-file mode is negative."},
    {multiLengthZeroErr, "Length field in multi-file mode is zero."},

    {piecesFieldLengthNonDivisibleBy20Err,
     "Pieces field length is not divisible by 20."},

    {bothLengthAndFilesFieldsMissingErr,
     "Both length and files fields are missing."},
    {bothLengthAndFilesFieldsPresentErr,
     "Both length and files fields are present; only one allowed."},

    // ---------------------------------
    // TRACKER
    // ---------------------------------

    {invalidUrlSchemeErr, "the announce url scheme was neither http or udp"},
    {invalidTrackerResponseErr, "the tracker response is invalid"}};
} // namespace btc
