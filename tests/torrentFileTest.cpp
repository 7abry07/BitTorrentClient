#include "torrent.h"
#include <chrono>
#include <gtest/gtest.h>
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
  ASSERT_OK(fileRes);

  btc::Torrent::TorrentFile file = *fileRes;

  std::string infoHash = "\xDE\x2F\xEE\x7C\xD8\xF3\x25\x14\xDC\x13"
                         "\x8B\x4C\xDD\x53\xC9\x3D\x7D\x7A\x1E\xB6";
  ASSERT_EQ(file.getInfoHash(), infoHash);
  ASSERT_EQ(file.getAnnounce(), "http://nyaa.tracker.wf:7777/announce");
  ASSERT_EQ(file.getName(),
            "[Sotark] Naruto Shippuden [480p][720p][HEVC][x265][Dual-Audio]");

  ASSERT_TRUE(file.getComment());
  ASSERT_TRUE(file.getCreatedBy());
  ASSERT_TRUE(file.getCreationDate());
  ASSERT_TRUE(file.getAnnounceList());
  ASSERT_TRUE(file.getFiles());

  ASSERT_EQ(*(file.getComment()), "https://nyaa.si/view/1189228");
  ASSERT_EQ(*(file.getCreatedBy()), "NyaaV2");

  const std::chrono::year y{2019};
  const std::chrono::month m{10};
  const std::chrono::day d{30};
  const std::chrono::year_month_day ymd{y, m, d};
  ASSERT_EQ(*(fileRes->getCreationDate()), ymd);

  ASSERT_EQ(*(fileRes->getEncoding()), "utf-8");
  ASSERT_EQ(file.getAnnounceList()->front(),
            "http://nyaa.tracker.wf:7777/announce");
  ASSERT_EQ(file.getAnnounceList()->back(),
            "udp://tracker.torrent.eu.org:451/announce");

  ASSERT_EQ(file.getFiles()->front().length, 288245151);
  ASSERT_EQ(
      file.getFiles()->front().path.string(),
      "[Sotark] Naruto Shippuden - 175 [720p][HEVC][x265][Dual-Audio].mkv");
  ASSERT_EQ(file.getFiles()->back().length, 103279091);
  ASSERT_EQ(
      file.getFiles()->back().path.string(),
      "[Sotark] Naruto Shippuden - 500 [720p][HEVC][x265][Dual-Audio].mkv");
}
