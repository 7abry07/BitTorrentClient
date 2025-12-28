#include "bencode.h"
#include <cstddef>
#include <expected>
#include <format>
#include <stdexcept>
#include <string>
#include <variant>

namespace BitTorrentClient::Bencode {

// ---------------------------------------------------
// ERROR
// ---------------------------------------------------

Error::Error(err_code code) : code(code) {}
std::string Error::get() { return err_mess.at(code); }

// ---------------------------------------------------
// VALUE
// ---------------------------------------------------

Value::Value(ValueType val) : val(std::move(val)) {}

bool Value::isInt() { return std::holds_alternative<Integer>(this->val); }
bool Value::isStr() { return std::holds_alternative<String>(this->val); }
bool Value::isList() { return std::holds_alternative<List>(this->val); }
bool Value::isDict() { return std::holds_alternative<Dict>(this->val); }

Value::Integer &Value::getInt() { return std::get<Integer>(this->val); }
Value::String &Value::getStr() { return std::get<String>(this->val); }
Value::List &Value::getList() { return std::get<List>(this->val); }
Value::Dict &Value::getDict() { return std::get<Dict>(this->val); }

// ---------------------------------------------------
// PARSER
// ---------------------------------------------------
Decoder::expected_val Decoder::decode(std::string_view input) {
  depth = 0;
  auto result = internal_decode(&input);
  if (result && !input.empty())
    return std::unexpected(trailingInputErr);
  return result;
}

Decoder::expected_val Decoder::internal_decode(std::string_view *input) {
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
    auto result = decode_str(input);
    depth--;
    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected<Error>(result.error());
  }

  // INTEGER
  case 'i': {
    auto result = decode_int(input);
    depth--;
    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected<Error>(result.error());
  }

  // LIST
  case 'l': {
    auto result = decode_list(input);
    depth--;
    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected<Error>(result.error());
  }

  // DICT
  case 'd': {
    auto result = decode_dict(input);
    depth--;
    return result.has_value()
               ? std::expected<Value, Error>(Value(result.value()))
               : std::unexpected<Error>(result.error());
  }
  }
  depth--;
  return std::unexpected(invalidTypeEncounterErr);
}

Decoder::expected_int Decoder::decode_int(std::string_view *input) {
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

Decoder::expected_str Decoder::decode_str(std::string_view *input) {
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

Decoder::expected_list Decoder::decode_list(std::string_view *input) {
  Value::List list_;
  input->remove_prefix(1);

  for (;;) {
    if (input->length() == 0)
      return std::unexpected(missingListTerminatorErr);
    if (input->at(0) == 'e') {
      input->remove_prefix(1);
      return list_;
    }

    auto result = internal_decode(input);
    if (!result)
      return (result.error().code == maximumNestingLimitExcedeedErr)
                 ? std::unexpected(maximumNestingLimitExcedeedErr)
                 : std::unexpected(invalidListElementErr);
    list_.push_back(result.value());
  }
}

Decoder::expected_dict Decoder::decode_dict(std::string_view *input) {
  Value::Dict dict_;
  Value::String prev_key = "";
  bool first = true;
  input->remove_prefix(1);

  for (;;) {
    if (input->length() == 0)
      return std::unexpected(missingDictTerminatorErr);
    if (input->at(0) == 'e') {
      input->remove_prefix(1);
      return dict_;
    }

    auto key_result = internal_decode(input);
    if (!key_result)
      return std::unexpected(key_result.error());
    if (!key_result->isStr())
      return std::unexpected(nonStringKeyErr);
    if (!first && key_result->getStr() < prev_key)
      return std::unexpected(unorderedKeysErr);

    auto val_result = internal_decode(input);
    if (!val_result)
      return std::unexpected(val_result.error());

    auto insert_result = dict_.emplace(key_result->getStr(), *val_result);
    if (!insert_result.second)
      return std::unexpected(duplicateKeyErr);

    prev_key = key_result->getStr();
    first = false;
  }
}

Decoder::expected_sizet Decoder::isIntegerValid(std::string_view input) {
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

Decoder::expected_sizet Decoder::isStringValid(std::string_view input) {
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

bool Decoder::hasLeadingZeroes(std::string_view input) {
  return (input.size() > 1 && input.at(0) == '0');
}
bool Decoder::isNegativeZero(std::string_view input) {
  return input.size() >= 2 && input.substr(0, 2) == "-0";
}

// ---------------------------------------------------
// ENCODER
// ---------------------------------------------------

std::string Encoder::encode(Value val) {
  std::string result = "";

  if (val.isInt())
    result.append(encode_int(val.getInt()));
  else if (val.isStr())
    result.append(encode_str(val.getStr()));
  else if (val.isList())
    result.append(encode_list(val.getList()));
  else if (val.isDict())
    result.append(encode_dict(val.getDict()));

  return result;
}

std::string Encoder::encode_int(Value::Integer val) {
  return std::format("i{}e", val);
}

std::string Encoder::encode_str(Value::String val) {
  return std::format("{}:{}", val.length(), val);
}

std::string Encoder::encode_list(Value::List val) {
  std::string result = "l";
  for (auto items : val)
    result.append(encode(items));
  result.append("e");
  return result;
}

std::string Encoder::encode_dict(Value::Dict val) {
  std::string result = "d";
  for (auto [first, second] : val) {
    result.append(encode_str(first));
    result.append(encode(second));
  }
  result.append("e");
  return result;
}

// ---------------------------------------------------
// PRINTER
// ---------------------------------------------------

std::string Printer::getFormattedValue(Value val, std::size_t space_count) {
  space_count_ = space_count;
  std::string formattedVal = getFormattedValue_internal(val);
  if (!formattedVal.empty() && formattedVal.back() == '\n')
    formattedVal.pop_back();
  return formattedVal;
}

std::string Printer::getFormattedValue_internal(Value val) {
  std::string result = "";

  if (val.isInt())
    result.append(formatInt(val.getInt()));
  if (val.isStr())
    result.append(formatStr(val.getStr()));
  if (val.isList())
    result.append(formatList(val.getList(), false));
  if (val.isDict())
    result.append(formatDict(val.getDict(), false));

  return result;
}

std::string Printer::formatInt(Value::Integer val) {
  return std::format("{}{}\n", spaces_, val);
}
std::string Printer::formatStr(Value::String val) {
  return std::format("{}{}\n", spaces_, val);
}

std::string Printer::formatList(Value::List val, bool dict_value) {
  std::string list = "";
  std::string start = (dict_value)
                          ? std::format("\n{}[", spaces_)
                          : std::format("{}LIST\n{}[", spaces_, spaces_);
  list.append(start + '\n');
  for (std::size_t i = 0; i < space_count_; i++)
    spaces_.append(" ");
  for (auto items : val)
    list.append(getFormattedValue_internal(items));
  for (std::size_t i = 0; i < space_count_; i++)
    spaces_.pop_back();
  list.append(std::format("{}]\n", spaces_));

  return list;
}

std::string Printer::formatDict(Value::Dict val, bool dict_value) {
  std::string dict = "";
  std::string start = (dict_value)
                          ? std::format("\n{}{{", spaces_)
                          : std::format("{}DICT\n{}{{", spaces_, spaces_);
  dict.append(start + '\n');
  for (std::size_t i = 0; i < space_count_; i++)
    spaces_.append(" ");
  for (auto [first, second] : val)
    dict.append(formatPair(first, second));
  for (std::size_t i = 0; i < space_count_; i++)
    spaces_.pop_back();
  dict.append(std::format("{}}}\n", spaces_));

  return dict;
}

std::string Printer::formatPair(Value::String val1, Value val2) {
  std::string pair = "";
  pair.append(std::format("{}{}: ", spaces_, val1));

  if (val2.isInt())
    pair.append(std::format("{}\n", val2.getInt()));
  if (val2.isStr())
    pair.append(std::format("{}\n", val2.getStr()));
  if (val2.isList()) {
    pair.append("LIST");
    pair.append(formatList(val2.getList(), true));
  }
  if (val2.isDict()) {
    pair.append("DICT");
    pair.append(formatDict(val2.getDict(), true));
  }
  return pair;
}

} // namespace BitTorrentClient::Bencode
