#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace btc {

class BencodeValue {

public:
  using Integer = std::int64_t;
  using String = std::string;
  using List = std::vector<BencodeValue>;
  using Dict = std::map<String, BencodeValue>;
  using ValueType = std::variant<Integer, String, List, Dict>;

  BencodeValue(ValueType val);

  bool isInt();
  bool isStr();
  bool isList();
  bool isDict();

  Integer &getInt();
  String &getStr();
  List &getList();
  Dict &getDict();

private:
  ValueType val;
};

} // namespace btc
