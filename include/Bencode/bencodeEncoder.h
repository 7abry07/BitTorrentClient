#pragma once

#include <Bencode/bencodeValue.h>
#include <string>

namespace btc {

class BencodeEncoder {

public:
  static std::string encode(BNode val);

private:
  static std::string encode_int(BNode::int_t val);
  static std::string encode_str(BNode::string_t val);
  static std::string encode_list(BNode::list_t val);
  static std::string encode_dict(BNode::dict_t val);
};

} // namespace btc
