#include "Bencode/bencodeValue.h"

namespace btc {

BencodeValue::BencodeValue(ValueType val) : val(std::move(val)) {}

bool BencodeValue::isInt() {
  return std::holds_alternative<Integer>(this->val);
}
bool BencodeValue::isStr() { return std::holds_alternative<String>(this->val); }
bool BencodeValue::isList() { return std::holds_alternative<List>(this->val); }
bool BencodeValue::isDict() { return std::holds_alternative<Dict>(this->val); }

BencodeValue::Integer &BencodeValue::getInt() {
  return std::get<Integer>(this->val);
}
BencodeValue::String &BencodeValue::getStr() {
  return std::get<String>(this->val);
}
BencodeValue::List &BencodeValue::getList() {
  return std::get<List>(this->val);
}
BencodeValue::Dict &BencodeValue::getDict() {
  return std::get<Dict>(this->val);
}

} // namespace btc
