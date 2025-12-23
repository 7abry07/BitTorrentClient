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
  invalidIntegerErr,
  invalidTypeEncounterErr,
  terminatorNotFoundErr,
  lengthMismatchErr,
  nonDigitCharacterErr
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

class Parser {
public:
  static std::expected<Value, Error> parse(std::string_view input);

private:
  static std::expected<Value::String, Error> parse_str(std::string_view *input);
  static std::expected<Value::List, Error> parse_list(std::string_view *input);
  static std::expected<Value::Dict, Error> parse_dict(std::string_view *input);
  static std::expected<Value::Integer, Error>
  parse_int(std::string_view *input);
};

} // namespace BitTorrentClient::Bencode
