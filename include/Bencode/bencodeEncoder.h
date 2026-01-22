#pragma once

#include <Bencode/bencodeValue.h>
#include <string>

namespace btc {

class BencodeEncoder {

public:
  static std::string encode(BencodeValue val);

private:
  static std::string encode_int(BencodeInteger val);
  static std::string encode_str(BencodeString val);
  static std::string encode_list(BencodeList val);
  static std::string encode_dict(BencodeDict val);
};

} // namespace btc
