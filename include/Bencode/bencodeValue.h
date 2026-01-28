#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace btc {

class BenNode;

using b_int = std::int64_t;
using b_string = std::string;
using b_list = std::vector<BenNode>;
using b_dict = std::map<b_string, BenNode>;

class BenNode {

public:
  using b_val = std::variant<b_int, b_string, b_list, b_dict>;

  BenNode(b_val val);

  bool isInt();
  bool isStr();
  bool isList();
  bool isDict();

  b_int &getInt();
  b_string &getStr();
  b_list &getList();
  b_dict &getDict();

private:
  b_val val;
};

} // namespace btc
