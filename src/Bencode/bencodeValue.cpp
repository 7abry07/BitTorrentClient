#include "Bencode/bencodeValue.h"

namespace btc {

BenNode::BenNode(b_val val) : val(std::move(val)) {}

bool BenNode::isInt() { return std::holds_alternative<b_int>(this->val); }
bool BenNode::isStr() { return std::holds_alternative<b_string>(this->val); }
bool BenNode::isList() { return std::holds_alternative<b_list>(this->val); }
bool BenNode::isDict() { return std::holds_alternative<b_dict>(this->val); }

b_int &BenNode::getInt() { return std::get<b_int>(this->val); }
b_string &BenNode::getStr() { return std::get<b_string>(this->val); }
b_list &BenNode::getList() { return std::get<b_list>(this->val); }
b_dict &BenNode::getDict() { return std::get<b_dict>(this->val); }

} // namespace btc
