#include "Bencode/bencodeValue.h"
#include <optional>

namespace btc {

BNode::BNode(b_val val) : val(std::move(val)) {}

bool BNode::isInt() { return std::holds_alternative<int_t>(this->val); }
bool BNode::isStr() { return std::holds_alternative<string_t>(this->val); }
bool BNode::isList() { return std::holds_alternative<list_t>(this->val); }
bool BNode::isDict() { return std::holds_alternative<dict_t>(this->val); }

BNode::int_t &BNode::getInt() { return std::get<int_t>(this->val); }
BNode::string_t &BNode::getStr() { return std::get<string_t>(this->val); }
BNode::list_t &BNode::getList() { return std::get<list_t>(this->val); }
BNode::dict_t &BNode::getDict() { return std::get<dict_t>(this->val); }

BNode BNode::dictFindInt(std::string k, BNode::int_t def) {
  BNode::dict_t node = getDict();
  return node.contains(k) && node.at(k).isInt() ? BNode(node.at(k).getInt())
                                                : BNode(def);
}
BNode BNode::dictFindString(std::string k, BNode::string_t def) {
  BNode::dict_t node = getDict();
  return node.contains(k) && node.at(k).isStr() ? BNode(node.at(k).getStr())
                                                : BNode(def);
}

std::optional<BNode> BNode::dictFindInt(std::string k) {
  BNode::dict_t node = getDict();
  if (node.contains(k) && node.at(k).isInt())
    return BNode(node.at(k).getInt());
  return std::nullopt;
}
std::optional<BNode> BNode::dictFindString(std::string k) {
  BNode::dict_t node = getDict();
  if (node.contains(k) && node.at(k).isStr())
    return BNode(node.at(k).getStr());
  return std::nullopt;
}
std::optional<BNode> BNode::dictFindList(std::string k) {
  BNode::dict_t node = getDict();
  if (node.contains(k) && node.at(k).isList())
    return BNode(node.at(k).getList());
  return std::nullopt;
}
std::optional<BNode> BNode::dictFindDict(std::string k) {
  BNode::dict_t node = getDict();
  if (node.contains(k) && node.at(k).isDict())
    return BNode(node.at(k).getDict());
  return std::nullopt;
}

} // namespace btc
