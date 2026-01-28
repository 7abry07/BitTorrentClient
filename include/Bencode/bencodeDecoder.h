#pragma once

#include <Bencode/bencodeValue.h>
#include <cstddef>
#include <errors.h>
#include <expected>
#include <system_error>

namespace btc {

class Error;

class BencodeDecoder {

private:
  using exp_int = std::expected<b_int, std::error_code>;
  using exp_str = std::expected<b_string, std::error_code>;
  using exp_list = std::expected<b_list, std::error_code>;
  using exp_dict = std::expected<b_dict, std::error_code>;
  using exp_node = std::expected<BenNode, std::error_code>;
  using exp_sizet = std::expected<std::size_t, std::error_code>;

public:
  static exp_node decode(std::string_view input);

private:
  static exp_node internal_decode(std::string_view *input);
  static exp_int decode_int(std::string_view *input);
  static exp_str decode_str(std::string_view *input);
  static exp_list decode_list(std::string_view *input);
  static exp_dict decode_dict(std::string_view *input);

  static bool hasLeadingZeroes(std::string_view input);
  static bool isNegativeZero(std::string_view input);

  inline static exp_sizet isIntegerValid(std::string_view input);
  inline static exp_sizet isStringValid(std::string_view input);

  inline static std::size_t depth = 0;
  inline static const std::uint16_t maxDepth = 256;
};
} // namespace btc
