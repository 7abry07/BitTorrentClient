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
  EXPECT_EQ(res->getInt(), 42);
}

TEST(BencodeInteger, ParsesNegativeInteger) {
  auto res = bencode_parser::parse("i-17e");
  EXPECT_OK(res);
  ASSERT_TRUE(res.value().isInt());
  EXPECT_EQ(res->getInt(), -17);
}

TEST(BencodeInteger, RejectsLeadingZero) {
  auto res = bencode_parser::parse("i042e");
  EXPECT_ERR(res, bencode_err::invalidIntegerErr);
}

TEST(BencodeInteger, RejectsNegativeZero) {
  auto res = bencode_parser::parse("i-0e");
  EXPECT_ERR(res, bencode_err::invalidIntegerErr);
}

// --------------------------------------------------------------------
// STRING
// --------------------------------------------------------------------

TEST(BencodeString, ParsesSimpleString) {
  auto res = bencode_parser::parse("4:spam");
  EXPECT_OK(res);
  ASSERT_TRUE(res.value().isStr());
  EXPECT_EQ(res->getStr(), "spam");
}

TEST(BencodeString, ParsesEmptyString) {
  auto res = bencode_parser::parse("0:");
  EXPECT_OK(res);
  ASSERT_TRUE(res.value().isStr());
  EXPECT_EQ(res->getStr(), "");
}

TEST(BencodeString, LengthMismatchError) {
  auto res = bencode_parser::parse("5:spam");
  EXPECT_ERR(res, bencode_err::lengthMismatchErr);
}

// --------------------------------------------------------------------
// LIST
// --------------------------------------------------------------------

TEST(BencodeList, ParsesListOfStrings) {
  auto res = bencode_parser::parse("l3:abc4:spame");
  EXPECT_OK(res);
  ASSERT_TRUE(res.value().isList());
  ASSERT_EQ(res.value().getList().size(), 2);
  ASSERT_EQ(res->getList()[0].getStr(), "abc");
  ASSERT_EQ(res->getList()[1].getStr(), "spam");
}

TEST(BencodeList, ParsesListOfIntegers) {
  auto res = bencode_parser::parse("li32ei45ee");
  EXPECT_OK(res);
  ASSERT_TRUE(res.value().isList());
  ASSERT_EQ(res->getList().size(), 2);
  ASSERT_EQ(res->getList()[0].getInt(), 32);
  ASSERT_EQ(res->getList()[1].getInt(), 45);
}

TEST(BencodeList, ParsesMixedList) {
  auto res = bencode_parser::parse("li32e4:spame");
  EXPECT_OK(res);
  ASSERT_TRUE(res.value().isList());
  ASSERT_EQ(res->getList().size(), 2);
  ASSERT_EQ(res->getList()[0].getInt(), 32);
  ASSERT_EQ(res->getList()[1].getStr(), "spam");
}

TEST(BencodeList, ParsesEmptyList) {
  auto res = bencode_parser::parse("le");
  EXPECT_OK(res);
  ASSERT_TRUE(res.value().isList());
  ASSERT_EQ(res->getList().size(), 0);
}
