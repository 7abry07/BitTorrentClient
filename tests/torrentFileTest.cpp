#include "bencode.h"
#include "torrent.h"
#include <gtest/gtest.h>

using torrent_file = btc::Torrent::Torrent;
using bencode_decoder = btc::Bencode::Decoder;

#define ASSERT_OK(expr) ASSERT_TRUE((expr).has_value())
#define EXPECT_ERR(expr, err)                                                  \
  ASSERT_FALSE((expr).has_value());                                            \
  EXPECT_EQ((expr).error().code, err);

TEST(Torrent, parseValidTorrentFile) {
  std::string input =
      "d8:announce35:http://tracker.example.com/"
      "announce4:infod6:lengthi12345e4:name10:sample.txt12:piece "
      "lengthi262144e6:pieces20:"
      "ABCDEFGHIJKLMNOPQRSTee";
  auto res = bencode_decoder::decode(input);
  ASSERT_OK(res);
  auto fileRes = torrent_file::validate(&res->getDict());
  ASSERT_OK(fileRes);
}
