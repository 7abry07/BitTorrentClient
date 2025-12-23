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

std::expected<Value, Error> Parser::parse(std::string_view input) {
  switch (input.at(0)) {
  case 'i': {
    auto result = parse_int(&input);
    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected<Error>(result.error());

    break;
  }

  case 'l': {
    auto result = parse_list(&input);
    break;
  }

  case 'd': {
    auto result = parse_dict(&input);
    break;
  }

  default: {
    if (!std::isdigit(input.at(0)))
      break;

    auto result = parse_str(&input);
    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected<Error>(result.error());
  }
  }
  return std::unexpected(Error::invalidTypeEncounterErr);
}

std::expected<Value::Integer, Error>
Parser::parse_int(std::string_view *input) {
  Value::Integer val;
  input->remove_prefix(1);

  if (input->size() > 1 && input->at(0) == '0')
    return std::unexpected(Error::invalidIntegerErr);

  std::size_t int_end = input->find('e');

  if (int_end == std::string::npos)
    return std::unexpected(Error::terminatorNotFoundErr);

  try {
    std::size_t read_bytes = 0;
    val = std::stoll(std::string(*input), &read_bytes);
    if (read_bytes < int_end)
      return std::unexpected(Error::invalidIntegerErr);

  } catch (std::exception e) {
    return std::unexpected(Error::invalidIntegerErr);
  }

  input->remove_prefix(int_end + 1);
  return val;
}

std::expected<Value::String, Error> Parser::parse_str(std::string_view *input) {
  Value::String val;
  std::size_t str_len;
  std::size_t len_end = input->find(':');

  if (len_end == std::string::npos)
    return std::unexpected(Error::terminatorNotFoundErr);

  try {
    std::size_t read_bytes = 0;
    str_len = std::stoll(std::string(*input), &read_bytes);
    if (read_bytes < len_end)
      return std::unexpected(Error::parse_err);
  } catch (std::exception e) {
    return std::unexpected(Error::parse_err);
  }

  std::string_view str = input->substr(len_end + 1, str_len);
  if (str.length() < str_len)
    return std::unexpected(Error::lengthMismatchErr);

  val.append(input->substr(len_end + 1, str_len));
  input->remove_prefix(len_end + 1 + str_len);
  return val;
}
std::expected<Value::List, Error> Parser::parse_list(std::string_view *input) {}
std::expected<Value::Dict, Error> Parser::parse_dict(std::string_view *input) {}

} // namespace BitTorrentClient::Bencode
