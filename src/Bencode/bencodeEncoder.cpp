#include "Bencode/bencodeEncoder.h"
#include "Bencode/bencodeValue.h"

#include <format>
#include <string>

namespace btc {

std::string BencodeEncoder::encode(BNode val) {
  std::string result = "";

  if (val.isInt())
    result.append(encode_int(val.getInt()));
  else if (val.isStr())
    result.append(encode_str(val.getStr()));
  else if (val.isList())
    result.append(encode_list(val.getList()));
  else if (val.isDict())
    result.append(encode_dict(val.getDict()));

  return result;
}

std::string BencodeEncoder::encode_int(BNode::int_t val) {
  return std::format("i{}e", val);
}

std::string BencodeEncoder::encode_str(BNode::string_t val) {
  return std::format("{}:{}", val.length(), val);
}

std::string BencodeEncoder::encode_list(BNode::list_t val) {
  std::string result = "l";
  for (auto items : val)
    result.append(encode(items));
  result.append("e");
  return result;
}

std::string BencodeEncoder::encode_dict(BNode::dict_t val) {
  std::string result = "d";
  for (auto [first, second] : val) {
    result.append(encode_str(first));
    result.append(encode(second));
  }
  result.append("e");
  return result;
}
} // namespace btc
