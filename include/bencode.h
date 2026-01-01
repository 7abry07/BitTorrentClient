#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "errors.h"

namespace btc::Bencode {

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

  Integer &getInt();
  String &getStr();
  List &getList();
  Dict &getDict();

private:
  ValueType val;
};

class Decoder {

public:
  static std::expected<Value, Error> decode(std::string_view input);

private:
  using expected_int = std::expected<Value::Integer, Error>;
  using expected_str = std::expected<Value::String, Error>;
  using expected_list = std::expected<Value::List, Error>;
  using expected_dict = std::expected<Value::Dict, Error>;
  using expected_val = std::expected<Value, Error>;
  using expected_sizet = std::expected<std::size_t, Error>;

  static expected_val internal_decode(std::string_view *input);
  static expected_int decode_int(std::string_view *input);
  static expected_str decode_str(std::string_view *input);
  static expected_list decode_list(std::string_view *input);
  static expected_dict decode_dict(std::string_view *input);

  static bool hasLeadingZeroes(std::string_view input);
  static bool isNegativeZero(std::string_view input);

  inline static expected_sizet isIntegerValid(std::string_view input);
  inline static expected_sizet isStringValid(std::string_view input);

  inline static std::size_t depth = 0;
  inline static const std::uint16_t maxDepth = 256;
};

class Encoder {

public:
  static std::string encode(Value val);

private:
  static std::string encode_int(Value::Integer val);
  static std::string encode_str(Value::String val);
  static std::string encode_list(Value::List val);
  static std::string encode_dict(Value::Dict val);
};

} // namespace btc::Bencode
