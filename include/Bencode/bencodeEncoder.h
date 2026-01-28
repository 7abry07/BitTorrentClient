#pragma once

#include <Bencode/bencodeValue.h>
#include <string>

namespace btc {

class BencodeEncoder {

public:
  static std::string encode(BenNode val);

private:
  static std::string encode_int(b_int val);
  static std::string encode_str(b_string val);
  static std::string encode_list(b_list val);
  static std::string encode_dict(b_dict val);
};

} // namespace btc
