#include "Bencode/bencodeValue.h"

namespace btc {

BencodeValue::BencodeValue(ValueType val) : val(std::move(val)) {}

bool BencodeValue::isInt() {
  return std::holds_alternative<BencodeInteger>(this->val);
}
bool BencodeValue::isStr() {
  return std::holds_alternative<BencodeString>(this->val);
}
bool BencodeValue::isList() {
  return std::holds_alternative<BencodeList>(this->val);
}
bool BencodeValue::isDict() {
  return std::holds_alternative<BencodeDict>(this->val);
}

BencodeInteger &BencodeValue::getInt() {
  return std::get<BencodeInteger>(this->val);
}
BencodeString &BencodeValue::getStr() {
  return std::get<BencodeString>(this->val);
}
BencodeList &BencodeValue::getList() {
  return std::get<BencodeList>(this->val);
}
BencodeDict &BencodeValue::getDict() {
  return std::get<BencodeDict>(this->val);
}

} // namespace btc
