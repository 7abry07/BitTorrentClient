#include "Bencode/bencodeEncoder.h"

#include <format>
#include <string>

namespace btc::Bencode {

std::string Encoder::encode(Value val) {
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

std::string Encoder::encode_int(Value::Integer val) {
  return std::format("i{}e", val);
}

std::string Encoder::encode_str(Value::String val) {
  return std::format("{}:{}", val.length(), val);
}

std::string Encoder::encode_list(Value::List val) {
  std::string result = "l";
  for (auto items : val)
    result.append(encode(items));
  result.append("e");
  return result;
}

std::string Encoder::encode_dict(Value::Dict val) {
  std::string result = "d";
  for (auto [first, second] : val) {
    result.append(encode_str(first));
    result.append(encode(second));
  }
  result.append("e");
  return result;
}

} // namespace btc::Bencode
