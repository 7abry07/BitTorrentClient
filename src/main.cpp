#include <cstdlib>
#include <fstream>
#include <iterator>
#include <print>

#include "bencode.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::println("missing path");
    return EXIT_FAILURE;
  }

  std::fstream file(argv[1], std::ios::in | std::ios::binary);
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

  auto res = btc::Bencode::Decoder::decode(content);
  if (!res)
    std::println("error -> {}", res.error().get());
  else
    std::println("no errors");
}
