#include "../bencode.h"
#include <gtest/gtest.h>
#include <limits>

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
// GENERAL
// --------------------------------------------------------------------

TEST(BencodeGeneral, RejectInvalidTrailingInput) {
  auto res = bencode_parser::parse("i43ee");
  EXPECT_ERR(res, bencode_err::trailingInputErr);

  auto res1 = bencode_parser::parse("4:spamfdf");
  EXPECT_ERR(res1, bencode_err::trailingInputErr);

  auto res2 = bencode_parser::parse("lee");
  EXPECT_ERR(res2, bencode_err::trailingInputErr);

  auto res3 = bencode_parser::parse("d4:spami43eegfs");
  EXPECT_ERR(res3, bencode_err::trailingInputErr);
}

TEST(BencodeGeneral, RejectEmptyInput) {
  auto res = bencode_parser::parse("");
  EXPECT_ERR(res, bencode_err::emptyInputErr);
}

TEST(BencodeGeneral, RejectMaxNestingExceeded) {
  std::string list;
  for (int i = 0; i < 1000; i++) {
    if (i <= 499)
      list.push_back('l');
    else
      list.push_back('e');
  }
  auto res = bencode_parser::parse(list);
  EXPECT_ERR(res, bencode_err::maximumNestingLimitExcedeedErr);

  std::string dict;
  for (int i = 0; i < 1000; i++) {
    if (i <= 499)
      dict.push_back('d');
    else
      dict.push_back('e');
  }
  auto res1 = bencode_parser::parse(dict);
  EXPECT_ERR(res1, bencode_err::maximumNestingLimitExcedeedErr);
}

// --------------------------------------------------------------------
// INTEGER
// --------------------------------------------------------------------

TEST(BencodeInteger, ParsePositiveInteger) {
  auto res = bencode_parser::parse("i42e");
  ASSERT_OK(res);

  ASSERT_TRUE(res->isInt());
  EXPECT_EQ(res->getInt(), 42);
}

TEST(BencodeInteger, ParseNegativeInteger) {
  auto res = bencode_parser::parse("i-17e");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isInt());
  EXPECT_EQ(res->getInt(), -17);
}

TEST(BencodeInteger, RejectEmptyInteger) {
  auto res = bencode_parser::parse("ie");
  EXPECT_ERR(res, bencode_err::invalidIntegerErr);
}

TEST(BencodeInteger, ParseMaxPositiveInteger) {
  auto res = bencode_parser::parse("i9223372036854775807e");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isInt());
  ASSERT_EQ(res->getInt(), std::numeric_limits<bencode_val::Integer>::max());
}

TEST(BencodeInteger, ParseMaxNegativeInteger) {
  auto res = bencode_parser::parse("i-9223372036854775808e");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isInt());
  ASSERT_EQ(res->getInt(), std::numeric_limits<bencode_val::Integer>::min());
}

TEST(BencodeInteger, RejectPositiveOverflowingInteger) {
  auto res = bencode_parser::parse("i9223372036854775808e");
  EXPECT_ERR(res, bencode_err::outOfRangeIntegerErr);
}

TEST(BencodeInteger, RejectNegativeoverflowingInteger) {
  auto res = bencode_parser::parse("i-9223372036854775809e");
  EXPECT_ERR(res, bencode_err::outOfRangeIntegerErr);
}

TEST(BencodeInteger, RejectLeadingZero) {
  auto res = bencode_parser::parse("i042e");
  EXPECT_ERR(res, bencode_err::invalidIntegerErr);
}

TEST(BencodeInteger, RejectNegativeZero) {
  auto res = bencode_parser::parse("i-0e");
  EXPECT_ERR(res, bencode_err::invalidIntegerErr);
}

TEST(BencodeInteger, RejectWeirdMix) {
  auto res1 = bencode_parser::parse("i-01e");
  EXPECT_ERR(res1, bencode_err::invalidIntegerErr);

  auto res2 = bencode_parser::parse("i--1e");
  EXPECT_ERR(res2, bencode_err::invalidIntegerErr);

  auto res3 = bencode_parser::parse("i1.0e");
  EXPECT_ERR(res3, bencode_err::invalidIntegerErr);

  auto res4 = bencode_parser::parse("i1");
  EXPECT_ERR(res4, bencode_err::missingIntegerTerminatorErr);

  auto res5 = bencode_parser::parse("i+1e");
  EXPECT_ERR(res5, bencode_err::invalidIntegerErr);

  auto res6 = bencode_parser::parse("i e");
  EXPECT_ERR(res6, bencode_err::invalidIntegerErr);
}

// --------------------------------------------------------------------
// STRING
// --------------------------------------------------------------------

TEST(BencodeString, ParseSimpleString) {
  auto res = bencode_parser::parse("4:spam");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isStr());
  EXPECT_EQ(res->getStr(), "spam");
}

TEST(BencodeString, ParseEmptyString) {
  auto res = bencode_parser::parse("0:");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isStr());
  EXPECT_EQ(res->getStr(), "");
}

TEST(BencodeString, RejectLengthMismatchError) {
  auto res = bencode_parser::parse("5:spam");
  EXPECT_ERR(res, bencode_err::lengthMismatchErr);
}

TEST(BencodeString, RejectNegativeLengthError) {
  auto res = bencode_parser::parse("-1:a");
  EXPECT_ERR(res, bencode_err::negativeStringLengthErr);
}

TEST(BencodeString, RejectOverflowingLength) {
  auto res = bencode_parser::parse("9999999999999999999999999:");
  EXPECT_ERR(res, bencode_err::stringTooLargeErr);
}

TEST(BencodeString, RejectSignedLengthError) {
  auto res = bencode_parser::parse("+1:a");
  EXPECT_ERR(res, bencode_err::signedStringLengthErr);
}

TEST(BencodeString, RejectMissingColon) {
  auto res = bencode_parser::parse("1a");
  EXPECT_ERR(res, bencode_err::missingColonErr);
}

// --------------------------------------------------------------------
// LIST
// --------------------------------------------------------------------

TEST(BencodeList, ParseListOfStrings) {
  auto res = bencode_parser::parse("l3:abc4:spame");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isList());
  ASSERT_EQ(res->getList().size(), 2);
  ASSERT_EQ(res->getList()[0].getStr(), "abc");
  ASSERT_EQ(res->getList()[1].getStr(), "spam");
}

TEST(BencodeList, ParseListOfIntegers) {
  auto res = bencode_parser::parse("li32ei45ee");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isList());
  ASSERT_EQ(res->getList().size(), 2);
  ASSERT_EQ(res->getList()[0].getInt(), 32);
  ASSERT_EQ(res->getList()[1].getInt(), 45);
}

TEST(BencodeList, ParseMixedList) {
  auto res = bencode_parser::parse("li32e4:spame");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isList());
  ASSERT_EQ(res->getList().size(), 2);
  ASSERT_EQ(res->getList()[0].getInt(), 32);
  ASSERT_EQ(res->getList()[1].getStr(), "spam");
}

TEST(BencodeList, ParseEmptyList) {
  auto res = bencode_parser::parse("le");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isList());
  ASSERT_EQ(res->getList().size(), 0);
}

TEST(BencodeList, RejectMissingTerminator) {
  auto res = bencode_parser::parse("lle");
  EXPECT_ERR(res, bencode_err::missingListTerminatorErr);
}

TEST(BencodeList, RejectInvalidElement) {
  auto res = bencode_parser::parse("lxe");
  EXPECT_ERR(res, bencode_err::invalidListElementErr);
}

// --------------------------------------------------------------------
// DICT
// --------------------------------------------------------------------

TEST(BencodeDict, ParseSimpleDict) {
  auto res = bencode_parser::parse("d4:spam3:abc5:zebrai43ee");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isDict());

  ASSERT_TRUE(res->getDict().contains(bencode_val::String("spam")));
  ASSERT_EQ(res->getDict().at("spam").getStr(), "abc");

  ASSERT_TRUE(res->getDict().contains(bencode_val::String("zebra")));
  ASSERT_EQ(res->getDict().at("zebra").getInt(), 43);
}

TEST(BencodeDict, ParseEmptyDict) {
  auto res = bencode_parser::parse("de");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isDict());
  ASSERT_TRUE(res->getDict().empty());
}

TEST(BencodeDict, RejectMissingTerminator) {
  auto res = bencode_parser::parse("d");
  EXPECT_ERR(res, bencode_err::missingDictTerminatorErr);
}

TEST(BencodeDict, RejectNonStringKey) {
  auto res = bencode_parser::parse("di34e4:spame");
  EXPECT_ERR(res, bencode_err::nonStringKeyErr);
}

TEST(BencodeDict, RejectDuplicateKey) {
  auto res = bencode_parser::parse("d4:spami43e4:spami56ee");
  EXPECT_ERR(res, bencode_err::duplicateKeyErr);
}
TEST(BencodeDict, RejectUnorderedKeys) {
  auto res = bencode_parser::parse("d4:cccci34e4:aaaai56ee");
  EXPECT_ERR(res, bencode_err::unorderedKeysErr);
}
