#pragma once

#include "Bencode/bencodeValue.h"
#include <string>

namespace btc {

class BencodeEncoder {

public:
  static std::string encode(BencodeValue val);

private:
  static std::string encode_int(BencodeValue::Integer val);
  static std::string encode_str(BencodeValue::String val);
  static std::string encode_list(BencodeValue::List val);
  static std::string encode_dict(BencodeValue::Dict val);
};

} // namespace btc
