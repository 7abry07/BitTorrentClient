#pragma once

#include <Bencode/bencodeValue.h>
#include <cstddef>
#include <errors.h>
#include <expected>
#include <system_error>

namespace btc {

class Error;

class BencodeDecoder {

public:
  static std::expected<BencodeValue, std::error_code>
  decode(std::string_view input);

private:
  using expected_int = std::expected<BencodeInteger, std::error_code>;
  using expected_str = std::expected<BencodeString, std::error_code>;
  using expected_list = std::expected<BencodeList, std::error_code>;
  using expected_dict = std::expected<BencodeDict, std::error_code>;
  using expected_val = std::expected<BencodeValue, std::error_code>;
  using expected_sizet = std::expected<std::size_t, std::error_code>;

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
} // namespace btc
