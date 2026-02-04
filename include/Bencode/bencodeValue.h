#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace btc {

class BNode;

class BNode {

public:
  using int_t = std::int64_t;
  using string_t = std::string;
  using list_t = std::vector<BNode>;
  using dict_t = std::map<string_t, BNode>;
  using b_val = std::variant<int_t, string_t, list_t, dict_t>;

  BNode() : val(0) {};
  BNode(b_val val);

  bool isInt();
  bool isStr();
  bool isList();
  bool isDict();

  int_t &getInt();
  string_t &getStr();
  list_t &getList();
  dict_t &getDict();

  BNode dictFindInt(std::string k, int_t def);
  BNode dictFindString(std::string k, string_t def);

  std::optional<BNode> dictFindInt(std::string k);
  std::optional<BNode> dictFindString(std::string k);
  std::optional<BNode> dictFindList(std::string k);
  std::optional<BNode> dictFindDict(std::string k);

private:
  b_val val;
};

} // namespace btc
