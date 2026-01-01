#include "bencode.h"
#include "torrent.h"
#include <gtest/gtest.h>
#include <print>
#include <string>

using torrentParser = btc::Torrent::TorrentParser;
using torrentFile = btc::Torrent::TorrentFile;
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

  std::string infoHash = "\xDE\x2F\xEE\x7C\xD8\xF3\x25\x14\xDC\x13"
                         "\x8B\x4C\xDD\x53\xC9\x3D\x7D\x7A\x1E\xB6";
  ASSERT_EQ(fileRes->getInfoHash(), infoHash);
}
