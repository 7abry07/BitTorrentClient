#pragma once

#include <string>
#include <unordered_map>

namespace btc {

class Error {
public:
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
    unorderedKeysErr,

    // ---------------------------------
    // TORRENT PARSER
    // ---------------------------------

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

  Error(error_code code) : code(code) {};
  std::string get() { return err_mess.at(code); }
  error_code code;

private:
  const std::unordered_map<error_code, std::string> err_mess = {

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
      {unorderedKeysErr, "Dictionary keys are not in order"},
      {missingColonErr, "Missing colon after string length"},

      // ---------------------------------
      // TORRENT PARSER
      // ---------------------------------

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
} // namespace btc
