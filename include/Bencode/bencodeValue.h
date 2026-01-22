#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace btc {
class BencodeValue;

using BencodeInteger = std::int64_t;
using BencodeString = std::string;
using BencodeList = std::vector<BencodeValue>;
using BencodeDict = std::map<BencodeString, BencodeValue>;

class BencodeValue {

public:
  // using Integer = std::int64_t;
  // using String = std::string;
  // using List = std::vector<BencodeValue>;
  // using Dict = std::map<String, BencodeValue>;
  using ValueType =
      std::variant<BencodeInteger, BencodeString, BencodeList, BencodeDict>;

  BencodeValue(ValueType val);

  bool isInt();
  bool isStr();
  bool isList();
  bool isDict();

  BencodeInteger &getInt();
  BencodeString &getStr();
  BencodeList &getList();
  BencodeDict &getDict();

private:
  ValueType val;
};

} // namespace btc
