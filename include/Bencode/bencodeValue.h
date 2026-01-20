#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace btc::Bencode {

class Value {

public:
  using Integer = std::int64_t;
  using String = std::string;
  using List = std::vector<Value>;
  using Dict = std::map<String, Value>;
  using ValueType = std::variant<Integer, String, List, Dict>;

  Value(ValueType val);

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

} // namespace btc::Bencode
