#pragma once

#include <cstdint>
#include <expected>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace BitTorrentClient::Bencode {

enum Error {
  parse_err,
  emptyInputErr,
  invalidIntegerErr,
  invalidTypeEncounterErr,
  terminatorNotFoundErr,
  lengthMismatchErr,
  nonDigitCharacterErr,
  nonStringKeyErr,
  invalidInput,
  maximumNestingLimitExcedeed
};

class Value {
public:
  using Integer = std::int64_t;
  using String = std::string;
  using List = std::vector<Value>;
  using Dict = std::map<String, Value>;
  using ValueType = std::variant<Integer, String, List, Dict>;

  Value(ValueType val);

  bool isInt();
  bool isStr();
  bool isList();
  bool isDict();

  std::optional<Integer> getSafeInt();
  std::optional<String> getSafeStr();
  List *getSafeList();
  Dict *getSafeDict();

  Integer &getInt();
  String &getStr();
  List &getList();
  Dict &getDict();

private:
  ValueType val;
};

using expected_int = std::expected<Value::Integer, Error>;
using expected_str = std::expected<Value::String, Error>;
using expected_list = std::expected<Value::List, Error>;
using expected_dict = std::expected<Value::Dict, Error>;
using expected_val = std::expected<Value, Error>;

class Parser {
public:
  static expected_val parse(std::string_view input);

private:
  static expected_val internal_parse(std::string_view *input);
  static expected_int parse_int(std::string_view *input);
  static expected_str parse_str(std::string_view *input);
  static expected_list parse_list(std::string_view *input);
  static expected_dict parse_dict(std::string_view *input);

  static bool hasLeadingZeroes(std::string_view input);
  static bool isNegativeZero(std::string_view input);

  inline static std::size_t depth = 0;
  inline static bool leftover = false;
  inline static const std::uint16_t maxDepth = 256;
};

} // namespace BitTorrentClient::Bencode
