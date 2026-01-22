#include "Bencode/bencodeEncoder.h"

#include <format>
#include <string>

namespace btc {

std::string BencodeEncoder::encode(BencodeValue val) {
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

std::string BencodeEncoder::encode_int(BencodeInteger val) {
  return std::format("i{}e", val);
}

std::string BencodeEncoder::encode_str(BencodeString val) {
  return std::format("{}:{}", val.length(), val);
}

std::string BencodeEncoder::encode_list(BencodeList val) {
  std::string result = "l";
  for (auto items : val)
    result.append(encode(items));
  result.append("e");
  return result;
}

std::string BencodeEncoder::encode_dict(BencodeDict val) {
  std::string result = "d";
  for (auto [first, second] : val) {
    result.append(encode_str(first));
    result.append(encode(second));
  }
  result.append("e");
  return result;
}
} // namespace btc
