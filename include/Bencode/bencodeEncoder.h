#pragma once

#include "Bencode/bencodeValue.h"
#include <string>

namespace btc::Bencode {

class Encoder {

public:
  static std::string encode(Value val);

private:
  static std::string encode_int(Value::Integer val);
  static std::string encode_str(Value::String val);
  static std::string encode_list(Value::List val);
  static std::string encode_dict(Value::Dict val);
};

} // namespace btc::Bencode
