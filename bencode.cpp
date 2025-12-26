#include "bencode.h"
#include <cctype>
#include <exception>
#include <expected>
#include <optional>
#include <print>
#include <string>
#include <variant>

namespace BitTorrentClient::Bencode {

Value::Value(ValueType val) : val(std::move(val)) {}

bool Value::isInt() { return std::holds_alternative<Integer>(this->val); }
bool Value::isStr() { return std::holds_alternative<String>(this->val); }
bool Value::isList() { return std::holds_alternative<List>(this->val); }
bool Value::isDict() { return std::holds_alternative<Dict>(this->val); }
std::optional<Value::Integer> Value::getSafeInt() {
  auto *ptr = std::get_if<Integer>(&this->val);
  if (ptr)
    return *ptr;
  else
    return std::nullopt;
}
std::optional<Value::String> Value::getSafeStr() {
  auto *ptr = std::get_if<String>(&this->val);
  if (ptr)
    return *ptr;
  else
    return std::nullopt;
}
Value::List *Value::getSafeList() { return std::get_if<List>(&this->val); }
Value::Dict *Value::getSafeDict() { return std::get_if<Dict>(&this->val); }

Value::Integer &Value::getInt() { return std::get<Integer>(this->val); }
Value::String &Value::getStr() { return std::get<String>(this->val); }
Value::List &Value::getList() { return std::get<List>(this->val); }
Value::Dict &Value::getDict() { return std::get<Dict>(this->val); }

expected_val Parser::parse(std::string_view input) {
  depth = 0;
  leftover = 0;
  auto result = internal_parse(&input);
  depth = 0;
  leftover = 0;
  return result;
}

expected_val Parser::internal_parse(std::string_view *input) {

  if (input->empty())
    return std::unexpected(emptyInputErr);
  switch (input->at(0)) {
  case 'i': {
    depth++;
    if (depth == maxDepth)
      return std::unexpected(maximumNestingLimitExcedeed);

    auto result = parse_int(input);
    if (leftover && depth == 1)
      return std::unexpected(invalidInput);

    depth--;

    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected<Error>(result.error());
  }

  case 'l': {
    depth++;
    if (depth == maxDepth)
      return std::unexpected(maximumNestingLimitExcedeed);

    auto result = parse_list(input);
    if (leftover && depth == 1)
      return std::unexpected(invalidInput);

    depth--;

    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected<Error>(result.error());
  }

  case 'd': {
    depth++;
    if (depth == maxDepth)
      return std::unexpected(maximumNestingLimitExcedeed);

    auto result = parse_dict(input);
    if (leftover && depth == 1)
      return std::unexpected(invalidInput);

    depth--;

    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected<Error>(result.error());
  }

  default: {
    if (!std::isdigit(input->at(0)))
      return std::unexpected(Error::invalidTypeEncounterErr);

    depth++;
    if (depth == maxDepth)
      return std::unexpected(maximumNestingLimitExcedeed);

    auto result = parse_str(input);
    if (leftover && depth == 1)
      return std::unexpected(invalidInput);

    depth--;

    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected<Error>(result.error());
  }
  }
}

expected_int Parser::parse_int(std::string_view *input) {
  Value::Integer int_;
  input->remove_prefix(1);
  std::size_t int_end = input->find('e');

  if (int_end == std::string::npos)
    return std::unexpected(terminatorNotFoundErr);
  if (hasLeadingZeroes(input->substr(0, int_end)) ||
      isNegativeZero(input->substr(0, int_end)))
    return std::unexpected(invalidIntegerErr);
  if (input->at(0) == '+')
    return std::unexpected(invalidIntegerErr);

  try {

    std::size_t read_bytes = 0;
    int_ = std::stoll(std::string(*input), &read_bytes);
    if (read_bytes < int_end)
      return std::unexpected(invalidIntegerErr);

  } catch (std::exception &e) {
    return std::unexpected(invalidIntegerErr);
  }

  input->remove_prefix(int_end + 1);
  leftover = !(input->length() == 0);
  return int_;
}

expected_str Parser::parse_str(std::string_view *input) {
  Value::String str_;
  std::size_t str_len;
  std::size_t len_end = input->find(':');

  if (len_end == std::string::npos)
    return std::unexpected(terminatorNotFoundErr);

  try {
    std::size_t read_bytes = 0;
    str_len = std::stoll(std::string(*input), &read_bytes);
    if (read_bytes < len_end)
      return std::unexpected(parse_err);
  } catch (std::exception &e) {
    return std::unexpected(parse_err);
  }

  std::string_view str = input->substr(len_end + 1, str_len);
  if (str.length() < str_len)
    return std::unexpected(lengthMismatchErr);

  str_.append(input->substr(len_end + 1, str_len));
  input->remove_prefix(len_end + 1 + str_len);
  leftover = !(input->length() == 0);
  return str_;
}

expected_list Parser::parse_list(std::string_view *input) {
  Value::List list_;
  input->remove_prefix(1);

  for (;;) {
    auto result = internal_parse(input);
    if (result) {
      list_.push_back(result.value());
      continue;
    }
    if (input->length() == 0) {
      return std::unexpected(terminatorNotFoundErr);
    }
    if (input->at(0) == 'e') {
      input->remove_prefix(1);
      leftover = !(input->length() == 0);
      return list_;
    }
    return std::unexpected(result.error());
  }
}

expected_dict Parser::parse_dict(std::string_view *input) {
  Value::Dict dict_;
  input->remove_prefix(1);

  for (;;) {
    if (input->at(0) == 'e') {
      input->remove_prefix(1);
      leftover = !(input->length() == 0);
      return dict_;
    }

    auto key_result = internal_parse(input);

    if (!key_result)
      return std::unexpected(key_result.error());
    if (!key_result->isStr())
      return std::unexpected(nonStringKeyErr);

    auto val_result = internal_parse(input);
    if (!val_result)
      return std::unexpected(val_result.error());

    dict_.try_emplace(key_result.value().getStr(), val_result.value());
  }
}

bool Parser::hasLeadingZeroes(std::string_view input) {
  return (input.size() > 1 && input.at(0) == '0');
}
bool Parser::isNegativeZero(std::string_view input) {
  return input.size() >= 2 && input.substr(0, 2) == "-0";
}

} // namespace BitTorrentClient::Bencode
