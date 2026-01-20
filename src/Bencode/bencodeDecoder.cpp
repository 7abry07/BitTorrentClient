#include "Bencode/bencodeDecoder.h"
#include <cstddef>
#include <expected>
#include <stdexcept>
#include <string>

namespace btc::Bencode {

Decoder::expected_val Decoder::decode(std::string_view input) {
  depth = 0;
  auto result = internal_decode(&input);
  if (result && !input.empty())
    return std::unexpected(Error::trailingInputErr);
  return result;
}

Decoder::expected_val Decoder::internal_decode(std::string_view *input) {
  if (++depth == maxDepth)
    return std::unexpected(Error::maximumNestingLimitExcedeedErr);
  if (input->empty())
    return std::unexpected(Error::emptyInputErr);

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
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected(Error(result.error().code));
  }

  case 'i': {
    auto result = decode_int(input);
    depth--;
    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected(Error(result.error().code));
  }

  case 'l': {
    auto result = decode_list(input);
    depth--;
    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected(Error(result.error().code));
  }

  case 'd': {
    auto result = decode_dict(input);
    depth--;
    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected(Error(result.error().code));
  }
  }
  depth--;
  return std::unexpected(Error::invalidTypeEncounterErr);
}

Decoder::expected_int Decoder::decode_int(std::string_view *input) {
  Value::Integer int_;
  input->remove_prefix(1);
  auto int_end = isIntegerValid(*input);
  if (!int_end)
    return std::unexpected(Error(int_end.error().code));

  try {
    std::size_t read_bytes = 0;
    int_ = std::stoll(std::string(*input), &read_bytes);
    if (read_bytes < *int_end)
      return std::unexpected(Error::invalidIntegerErr);

  } catch (std::invalid_argument &e) {
    return std::unexpected(Error::invalidIntegerErr);
  } catch (std::out_of_range &e) {
    return std::unexpected(Error::outOfRangeIntegerErr);
  }

  input->remove_prefix(*int_end + 1);
  return int_;
}

Decoder::expected_str Decoder::decode_str(std::string_view *input) {
  Value::String str_;
  std::size_t str_len;
  auto len_end = isStringValid(*input);
  if (!len_end)
    return std::unexpected(Error(len_end.error().code));

  try {
    std::size_t read_bytes = 0;
    str_len = std::stoll(std::string(*input), &read_bytes);
    if (read_bytes < *len_end)
      return std::unexpected(Error::invalidStringLengthErr);
  } catch (std::invalid_argument &e) {
    return std::unexpected(Error::invalidStringLengthErr);
  } catch (std::out_of_range &e) {
    return std::unexpected(Error::stringTooLargeErr);
  }

  std::string_view str = input->substr(*len_end + 1, str_len);
  if (str.length() < str_len)
    return std::unexpected(Error::lengthMismatchErr);

  str_.append(input->substr(*len_end + 1, str_len));
  input->remove_prefix(*len_end + 1 + str_len);
  return str_;
}

Decoder::expected_list Decoder::decode_list(std::string_view *input) {
  Value::List list_;
  input->remove_prefix(1);

  for (;;) {
    if (input->length() == 0)
      return std::unexpected(Error::missingListTerminatorErr);
    if (input->at(0) == 'e') {
      input->remove_prefix(1);
      return list_;
    }

    auto result = internal_decode(input);
    if (!result)
      return (result.error().code == Error::maximumNestingLimitExcedeedErr)
                 ? std::unexpected(Error::maximumNestingLimitExcedeedErr)
                 : std::unexpected(Error::invalidListElementErr);
    list_.push_back(result.value());
  }
}

Decoder::expected_dict Decoder::decode_dict(std::string_view *input) {
  Value::Dict dict_;
  input->remove_prefix(1);

  for (;;) {
    if (input->length() == 0)
      return std::unexpected(Error::missingDictTerminatorErr);
    if (input->at(0) == 'e') {
      input->remove_prefix(1);
      return dict_;
    }

    auto key_result = internal_decode(input);
    if (!key_result)
      return std::unexpected(Error(key_result.error().code));
    if (!key_result->isStr())
      return std::unexpected(Error::nonStringKeyErr);

    auto val_result = internal_decode(input);
    if (!val_result)
      return std::unexpected(Error(val_result.error().code));

    auto insert_result = dict_.emplace(key_result->getStr(), *val_result);
    if (!insert_result.second)
      return std::unexpected(Error::duplicateKeyErr);
  }
}

Decoder::expected_sizet Decoder::isIntegerValid(std::string_view input) {
  std::size_t int_end = input.find('e');

  if (int_end == std::string::npos)
    return std::unexpected(Error::missingIntegerTerminatorErr);
  if (hasLeadingZeroes(input.substr(0, int_end)) ||
      isNegativeZero(input.substr(0, int_end)))
    return std::unexpected(Error::invalidIntegerErr);
  if (input.at(0) == '+')
    return std::unexpected(Error::invalidIntegerErr);

  return int_end;
}

Decoder::expected_sizet Decoder::isStringValid(std::string_view input) {
  std::size_t len_end = input.find(':');

  if (len_end == std::string::npos)
    return std::unexpected(Error::missingColonErr);
  if (hasLeadingZeroes(input.substr(0, len_end)))
    return std::unexpected(Error::invalidStringLengthErr);
  if (input.at(0) == '-')
    return std::unexpected(Error::negativeStringLengthErr);
  if (input.at(0) == '+')
    return std::unexpected(Error::signedStringLengthErr);

  return len_end;
}

bool Decoder::hasLeadingZeroes(std::string_view input) {
  return (input.size() > 1 && input.at(0) == '0');
}
bool Decoder::isNegativeZero(std::string_view input) {
  return input.size() >= 2 && input.substr(0, 2) == "-0";
}

} // namespace btc::Bencode
