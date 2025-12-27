#include "bencode.h"
#include <expected>
#include <stdexcept>
#include <string>
#include <variant>

namespace BitTorrentClient::Bencode {

Value::Value(ValueType val) : val(std::move(val)) {}

bool Value::isInt() { return std::holds_alternative<Integer>(this->val); }
bool Value::isStr() { return std::holds_alternative<String>(this->val); }
bool Value::isList() { return std::holds_alternative<List>(this->val); }
bool Value::isDict() { return std::holds_alternative<Dict>(this->val); }

Value::Integer &Value::getInt() { return std::get<Integer>(this->val); }
Value::String &Value::getStr() { return std::get<String>(this->val); }
Value::List &Value::getList() { return std::get<List>(this->val); }
Value::Dict &Value::getDict() { return std::get<Dict>(this->val); }

Parser::expected_val Parser::parse(std::string_view input) {
  depth = 0;
  auto result = internal_parse(&input);
  if (result && !input.empty())
    return std::unexpected(trailingInputErr);
  return result;
}

Parser::expected_val Parser::internal_parse(std::string_view *input) {
  if (++depth == maxDepth)
    return std::unexpected(maximumNestingLimitExcedeedErr);
  if (input->empty())
    return std::unexpected(emptyInputErr);

  switch (input->at(0)) {

  // STRING
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
    auto result = parse_str(input);
    depth--;
    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected<Error>(result.error());
  }

  // INTEGER
  case 'i': {
    auto result = parse_int(input);
    depth--;
    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected<Error>(result.error());
  }

  // LIST
  case 'l': {
    auto result = parse_list(input);
    depth--;
    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected<Error>(result.error());
  }

  // DICT
  case 'd': {
    auto result = parse_dict(input);
    depth--;
    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected<Error>(result.error());
  }
  }
  depth--;
  return std::unexpected(Error::invalidTypeEncounterErr);
}

Parser::expected_int Parser::parse_int(std::string_view *input) {
  Value::Integer int_;
  input->remove_prefix(1);
  auto int_end = isIntegerValid(*input);
  if (!int_end)
    return std::unexpected(int_end.error());

  try {

    std::size_t read_bytes = 0;
    int_ = std::stoll(std::string(*input), &read_bytes);
    if (read_bytes < *int_end)
      return std::unexpected(invalidIntegerErr);

  } catch (std::invalid_argument &e) {
    return std::unexpected(invalidIntegerErr);
  } catch (std::out_of_range &e) {
    return std::unexpected(outOfRangeIntegerErr);
  }

  input->remove_prefix(*int_end + 1);
  return int_;
}

Parser::expected_str Parser::parse_str(std::string_view *input) {
  Value::String str_;
  std::size_t str_len;
  auto len_end = isStringValid(*input);
  if (!len_end)
    return std::unexpected(len_end.error());

  try {

    std::size_t read_bytes = 0;
    str_len = std::stoll(std::string(*input), &read_bytes);
    if (read_bytes < *len_end)
      return std::unexpected(invalidStringLengthErr);

  } catch (std::invalid_argument &e) {
    return std::unexpected(invalidStringLengthErr);
  } catch (std::out_of_range &e) {
    return std::unexpected(stringTooLargeErr);
  }

  std::string_view str = input->substr(*len_end + 1, str_len);
  if (str.length() < str_len)
    return std::unexpected(lengthMismatchErr);

  str_.append(input->substr(*len_end + 1, str_len));
  input->remove_prefix(*len_end + 1 + str_len);
  return str_;
}

Parser::expected_list Parser::parse_list(std::string_view *input) {
  Value::List list_;
  input->remove_prefix(1);

  for (;;) {
    if (input->length() == 0)
      return std::unexpected(missingListTerminatorErr);
    if (input->at(0) == 'e') {
      input->remove_prefix(1);
      return list_;
    }

    auto result = internal_parse(input);
    if (!result)
      return (result.error() == maximumNestingLimitExcedeedErr)
                 ? std::unexpected(maximumNestingLimitExcedeedErr)
                 : std::unexpected(invalidListElementErr);
    list_.push_back(result.value());
  }
}

Parser::expected_dict Parser::parse_dict(std::string_view *input) {
  Value::Dict dict_;
  input->remove_prefix(1);

  Value::String prev_key = "";
  bool first = true;

  for (;;) {
    if (input->length() == 0)
      return std::unexpected(missingDictTerminatorErr);
    if (input->at(0) == 'e') {
      input->remove_prefix(1);
      return dict_;
    }

    auto key_result = internal_parse(input);

    if (!key_result)
      return std::unexpected(key_result.error());
    if (!key_result->isStr())
      return std::unexpected(nonStringKeyErr);
    if (!first && key_result->getStr() < prev_key)
      return std::unexpected(unorderedKeysErr);

    auto val_result = internal_parse(input);
    if (!val_result)
      return std::unexpected(val_result.error());

    auto insert_result = dict_.emplace(key_result->getStr(), *val_result);
    if (!insert_result.second)
      return std::unexpected(duplicateKeyErr);
    prev_key = key_result->getStr();
    first = false;
  }
}

Parser::expected_sizet Parser::isIntegerValid(std::string_view input) {
  std::size_t int_end = input.find('e');

  if (int_end == std::string::npos)
    return std::unexpected(missingIntegerTerminatorErr);
  if (hasLeadingZeroes(input.substr(0, int_end)) ||
      isNegativeZero(input.substr(0, int_end)))
    return std::unexpected(invalidIntegerErr);
  if (input.at(0) == '+')
    return std::unexpected(invalidIntegerErr);

  return int_end;
}

Parser::expected_sizet Parser::isStringValid(std::string_view input) {
  std::size_t len_end = input.find(':');

  if (len_end == std::string::npos)
    return std::unexpected(missingColonErr);
  if (hasLeadingZeroes(input.substr(0, len_end)))
    return std::unexpected(invalidStringLengthErr);
  if (input.at(0) == '-')
    return std::unexpected(negativeStringLengthErr);
  if (input.at(0) == '+')
    return std::unexpected(signedStringLengthErr);

  return len_end;
}

bool Parser::hasLeadingZeroes(std::string_view input) {
  return (input.size() > 1 && input.at(0) == '0');
}
bool Parser::isNegativeZero(std::string_view input) {
  return input.size() >= 2 && input.substr(0, 2) == "-0";
}

} // namespace BitTorrentClient::Bencode
