#pragma once

#include "bencodeValue.h"
#include "errors.h"
#include <cstddef>
#include <expected>

namespace btc::Bencode {

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
} // namespace btc::Bencode
