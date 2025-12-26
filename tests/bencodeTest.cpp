#include "../bencode.h"
#include <gtest/gtest.h>

using bencode_parser = BitTorrentClient::Bencode::Parser;
using bencode_err = BitTorrentClient::Bencode::Error;
using bencode_val = BitTorrentClient::Bencode::Value;

#define EXPECT_OK(expr) EXPECT_TRUE((expr).has_value())
#define ASSERT_OK(expr) ASSERT_TRUE((expr).has_value())
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
  ASSERT_OK(res);

  ASSERT_TRUE(res->isInt());
  EXPECT_EQ(res->getInt(), 42);
}

TEST(BencodeInteger, ParsesNegativeInteger) {
  auto res = bencode_parser::parse("i-17e");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isInt());
  EXPECT_EQ(res->getInt(), -17);
}

TEST(BencodeInteger, RejectsEmptyInteger) {
  auto res = bencode_parser::parse("ie");
  EXPECT_ERR(res, bencode_err::invalidIntegerErr);
}

TEST(BencodeInteger, RejectsNegativeoverflowingInteger) {
  auto res = bencode_parser::parse("i-9223372036854775809e");
  EXPECT_ERR(res, bencode_err::invalidIntegerErr);
}

TEST(BencodeInteger, RejectsPositiveOverflowingInteger) {
  auto res = bencode_parser::parse("i9223372036854775808e");
  EXPECT_ERR(res, bencode_err::invalidIntegerErr);
}

TEST(BencodeInteger, RejectsLeadingZero) {
  auto res = bencode_parser::parse("i042e");
  EXPECT_ERR(res, bencode_err::invalidIntegerErr);
}

TEST(BencodeInteger, RejectsNegativeZero) {
  auto res = bencode_parser::parse("i-0e");
  EXPECT_ERR(res, bencode_err::invalidIntegerErr);
}

TEST(BencodeInteger, RejectsWeirdMix) {
  auto res1 = bencode_parser::parse("i-01e");
  EXPECT_ERR(res1, bencode_err::invalidIntegerErr);

  auto res2 = bencode_parser::parse("i--1e");
  EXPECT_ERR(res2, bencode_err::invalidIntegerErr);

  auto res3 = bencode_parser::parse("i1.0e");
  EXPECT_ERR(res3, bencode_err::invalidIntegerErr);

  auto res4 = bencode_parser::parse("i1");
  EXPECT_ERR(res4, bencode_err::terminatorNotFoundErr);

  auto res5 = bencode_parser::parse("i+1e");
  EXPECT_ERR(res5, bencode_err::invalidIntegerErr);

  auto res6 = bencode_parser::parse("i e");
  EXPECT_ERR(res6, bencode_err::invalidIntegerErr);
}

// --------------------------------------------------------------------
// STRING
// --------------------------------------------------------------------

TEST(BencodeString, ParsesSimpleString) {
  auto res = bencode_parser::parse("4:spam");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isStr());
  EXPECT_EQ(res->getStr(), "spam");
}

TEST(BencodeString, ParsesEmptyString) {
  auto res = bencode_parser::parse("0:");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isStr());
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
  ASSERT_OK(res);
  ASSERT_TRUE(res->isList());
  ASSERT_EQ(res->getList().size(), 2);
  ASSERT_EQ(res->getList()[0].getStr(), "abc");
  ASSERT_EQ(res->getList()[1].getStr(), "spam");
}

TEST(BencodeList, ParsesListOfIntegers) {
  auto res = bencode_parser::parse("li32ei45ee");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isList());
  ASSERT_EQ(res->getList().size(), 2);
  ASSERT_EQ(res->getList()[0].getInt(), 32);
  ASSERT_EQ(res->getList()[1].getInt(), 45);
}

TEST(BencodeList, ParsesMixedList) {
  auto res = bencode_parser::parse("li32e4:spame");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isList());
  ASSERT_EQ(res->getList().size(), 2);
  ASSERT_EQ(res->getList()[0].getInt(), 32);
  ASSERT_EQ(res->getList()[1].getStr(), "spam");
}

TEST(BencodeList, ParsesEmptyList) {
  auto res = bencode_parser::parse("le");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isList());
  ASSERT_EQ(res->getList().size(), 0);
}

TEST(BencodeList, RejectsMaxNestingExceeded) {
  std::string str;
  for (int i = 0; i < 512; i++) {
    if (i <= 255)
      str.push_back('l');
    else
      str.push_back('e');
  }
  auto res = bencode_parser::parse(str);
  EXPECT_ERR(res, bencode_err::maximumNestingLimitExcedeed);
}

TEST(BencodeList, RejectsMissingTerminator) {
  auto res = bencode_parser::parse("lle");
  EXPECT_ERR(res, bencode_err::terminatorNotFoundErr);
}

// --------------------------------------------------------------------
// DICT
// --------------------------------------------------------------------

TEST(BencodeDict, parsesSimpleDict) {
  auto res = bencode_parser::parse("d4:spam3:abc5:seveni43ee");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isDict());

  ASSERT_TRUE(res->getDict().contains(bencode_val::String("spam")));
  ASSERT_EQ(res->getDict().at("spam").getStr(), "abc");

  ASSERT_TRUE(res->getDict().contains(bencode_val::String("seven")));
  ASSERT_EQ(res->getDict().at("seven").getInt(), 43);
}

TEST(BencodeDict, parsesEmptyDict) {
  auto res = bencode_parser::parse("de");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isDict());
  ASSERT_TRUE(res->getDict().empty());
}
