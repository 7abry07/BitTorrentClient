#include "../bencode.h"
#include <gtest/gtest.h>

using bencode_parser = BitTorrentClient::Bencode::Parser;
using bencode_err = BitTorrentClient::Bencode::Error;
using bencode_val = BitTorrentClient::Bencode::Value;

#define EXPECT_OK(expr) EXPECT_TRUE((expr).has_value())
#define EXPECT_ERR(expr, err)                                                  \
  do {                                                                         \
    ASSERT_FALSE((expr).has_value());                                          \
    EXPECT_EQ((expr).error(), err);                                            \
  } while (0)

// --------------------------------------------------------------------
// INTEGER
// --------------------------------------------------------------------

TEST(BencodeInteger, ParsesPositiveInteger) {
  auto res = bencode_parser::parse("i42e");
  EXPECT_OK(res);

  ASSERT_TRUE(res.value().isInt());
  EXPECT_EQ(res.value().getInt(), 42);
}

TEST(BencodeInteger, ParsesNegativeInteger) {
  auto res = bencode_parser::parse("i-17e");
  EXPECT_OK(res);
  ASSERT_TRUE(res.value().isInt());
  EXPECT_EQ(res.value().getInt(), -17);
}

TEST(BencodeInteger, RejectsLeadingZero) {
  auto res = bencode_parser::parse("i042e");
  EXPECT_ERR(res, bencode_err::invalidIntegerErr);
}

// --------------------------------------------------------------------
// STRING
// --------------------------------------------------------------------

TEST(BencodeString, ParsesSimpleString) {
  auto res = bencode_parser::parse("4:spam");
  ASSERT_TRUE(res.value().isStr());
  EXPECT_EQ(res->getStr(), "spam");
}

TEST(BencodeString, ParsesEmptyString) {
  auto res = bencode_parser::parse("0:");
  ASSERT_TRUE(res.value().isStr());
  EXPECT_EQ(res->getStr(), "");
}

TEST(BencodeString, LengthMismatchError) {
  auto res = bencode_parser::parse("5:spam");
  EXPECT_ERR(res, bencode_err::lengthMismatchErr);
}
