#include "bencode.h"
#include "torrent.h"
#include <gtest/gtest.h>
#include <print>

using torrentParser = btc::Torrent::TorrentParser;
using bencodeDecoder = btc::Bencode::Decoder;

#define TEST_PATH "../testFiles/naruto.torrent"

#define ASSERT_OK(expr) ASSERT_TRUE((expr).has_value())
#define EXPECT_ERR(expr, err)                                                  \
  ASSERT_FALSE((expr).has_value());                                            \
  EXPECT_EQ((expr).error().code, err);

TEST(TorrentFile, parseValidTorrentFile) {
  bencodeDecoder decoder;
  torrentParser parser;
  auto fileRes = parser.parseFile(TEST_PATH, decoder);
  if (!fileRes)
    std::println("{}", fileRes.error().get());
  ASSERT_OK(fileRes);
}
