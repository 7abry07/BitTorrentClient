#include "Bencode/bencodeValue.h"
#include <Bencode/bencodeDecoder.h>
#include <cstddef>
#include <errors.h>
#include <expected>
#include <stdexcept>
#include <string>

namespace btc {

BencodeDecoder::exp_node BencodeDecoder::decode(std::string_view input) {
  depth = 0;
  auto result = internal_decode(&input);
  if (result && !input.empty())
    return std::unexpected(error_code::trailingInputErr);
  return result;
}

BencodeDecoder::exp_node
BencodeDecoder::internal_decode(std::string_view *input) {
  if (++depth == maxDepth)
    return std::unexpected(error_code::maximumNestingLimitExcedeedErr);
  if (input->empty())
    return std::unexpected(error_code::emptyInputErr);

  switch (input->at(0)) {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case '+':
  case '-': {
    auto result = decode_str(input);
    depth--;
    return result.has_value()
               ? std::expected<BNode, std::error_code>(BNode(result.value()))
               : std::unexpected(result.error());
  }

  case 'i': {
    auto result = decode_int(input);
    depth--;
    return result.has_value()
               ? std::expected<BNode, std::error_code>(BNode(result.value()))
               : std::unexpected(result.error());
  }

  case 'l': {
    auto result = decode_list(input);
    depth--;
    return result.has_value()
               ? std::expected<BNode, std::error_code>(BNode(result.value()))
               : std::unexpected(result.error());
  }

  case 'd': {
    auto result = decode_dict(input);
    depth--;
    return result.has_value()
               ? std::expected<BNode, std::error_code>(BNode(result.value()))
               : std::unexpected(result.error());
  }
  }

  depth--;
  return std::unexpected(error_code::invalidTypeEncounterErr);
}

BencodeDecoder::exp_int BencodeDecoder::decode_int(std::string_view *input) {
  BNode::int_t int_;
  input->remove_prefix(1);
  auto int_end = isIntegerValid(*input);
  if (!int_end)
    return std::unexpected(int_end.error());

  try {
    std::size_t read_bytes = 0;
    int_ = std::stoll(std::string(*input), &read_bytes);
    if (read_bytes < *int_end)
      return std::unexpected(error_code::invalidIntegerErr);
  } catch (std::invalid_argument &) {
    return std::unexpected(error_code::invalidIntegerErr);
  } catch (std::out_of_range &) {
    return std::unexpected(error_code::outOfRangeIntegerErr);
  }

  input->remove_prefix(*int_end + 1);
  return int_;
}

BencodeDecoder::exp_str BencodeDecoder::decode_str(std::string_view *input) {
  BNode::string_t str;
  std::size_t str_len;
  auto len_end = isStringValid(*input);
  if (!len_end)
    return std::unexpected(len_end.error());

  try {
    std::size_t read_bytes = 0;
    str_len = std::stoll(std::string(*input), &read_bytes);
    if (read_bytes < *len_end)
      return std::unexpected(error_code::invalidStringLengthErr);
  } catch (std::invalid_argument &) {
    return std::unexpected(error_code::invalidStringLengthErr);
  } catch (std::out_of_range &) {
    return std::unexpected(error_code::stringTooLargeErr);
  }

  std::string_view read_str = input->substr(*len_end + 1, str_len);
  if (read_str.length() < str_len)
    return std::unexpected(error_code::lengthMismatchErr);

  str.append(input->substr(*len_end + 1, str_len));
  input->remove_prefix(*len_end + 1 + str_len);
  return str;
}

BencodeDecoder::exp_list BencodeDecoder::decode_list(std::string_view *input) {
  BNode::list_t list;
  input->remove_prefix(1);

  for (;;) {
    if (input->length() == 0)
      return std::unexpected(error_code::missingListTerminatorErr);
    if (input->at(0) == 'e') {
      input->remove_prefix(1);
      return list;
    }

    auto result = internal_decode(input);
    if (!result)
      return (result.error() == error_code::maximumNestingLimitExcedeedErr)
                 ? std::unexpected(error_code::maximumNestingLimitExcedeedErr)
                 : std::unexpected(error_code::invalidListElementErr);
    list.push_back(result.value());
  }
}

BencodeDecoder::exp_dict BencodeDecoder::decode_dict(std::string_view *input) {
  BNode::dict_t dict;
  input->remove_prefix(1);

  for (;;) {
    if (input->length() == 0)
      return std::unexpected(error_code::missingDictTerminatorErr);
    if (input->at(0) == 'e') {
      input->remove_prefix(1);
      return dict;
    }

    auto key_result = internal_decode(input);
    if (!key_result)
      return std::unexpected(key_result.error());
    if (!key_result->isStr())
      return std::unexpected(error_code::nonStringKeyErr);

    auto val_result = internal_decode(input);
    if (!val_result)
      return std::unexpected(val_result.error());

    auto insert_result = dict.emplace(key_result->getStr(), *val_result);
    if (!insert_result.second)
      return std::unexpected(error_code::duplicateKeyErr);
  }
}

BencodeDecoder::exp_sizet
BencodeDecoder::isIntegerValid(std::string_view input) {
  std::size_t int_end = input.find('e');

  if (int_end == std::string::npos)
    return std::unexpected(error_code::missingIntegerTerminatorErr);
  if (hasLeadingZeroes(input.substr(0, int_end)) ||
      isNegativeZero(input.substr(0, int_end)))
    return std::unexpected(error_code::invalidIntegerErr);
  if (input.at(0) == '+')
    return std::unexpected(error_code::invalidIntegerErr);

  return int_end;
}

BencodeDecoder::exp_sizet
BencodeDecoder::isStringValid(std::string_view input) {
  std::size_t len_end = input.find(':');

  if (len_end == std::string::npos)
    return std::unexpected(error_code::missingColonErr);
  if (hasLeadingZeroes(input.substr(0, len_end)))
    return std::unexpected(error_code::invalidStringLengthErr);
  if (input.at(0) == '-')
    return std::unexpected(error_code::negativeStringLengthErr);
  if (input.at(0) == '+')
    return std::unexpected(error_code::signedStringLengthErr);

  return len_end;
}

bool BencodeDecoder::hasLeadingZeroes(std::string_view input) {
  return (input.size() > 1 && input.at(0) == '0');
}

bool BencodeDecoder::isNegativeZero(std::string_view input) {
  return input.size() >= 2 && input.substr(0, 2) == "-0";
}

} // namespace btc
  // namespace btc
