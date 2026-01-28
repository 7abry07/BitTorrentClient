#include <Bencode/bencodeDecoder.h>
#include <Bencode/bencodeEncoder.h>
#include <Bencode/bencodeValue.h>
#include <gtest/gtest.h>
#include <limits>
#include <string>

using bencode_decoder = btc::BencodeDecoder;
using bencode_encoder = btc::BencodeEncoder;

#define ASSERT_OK(expr) ASSERT_TRUE((expr).has_value())
#define EXPECT_ERR(expr, err)                                                  \
  ASSERT_FALSE((expr).has_value());                                            \
  EXPECT_EQ((expr).error(), err);

// --------------------------------------------------------------------
// GENERAL
// --------------------------------------------------------------------

TEST(BencodeGeneral, RejectInvalidTrailingInput) {
  auto res = bencode_decoder::decode("i43ee");
  EXPECT_ERR(res, btc::error_code::trailingInputErr);

  auto res1 = bencode_decoder::decode("4:spamfdf");
  EXPECT_ERR(res1, btc::error_code::trailingInputErr);

  auto res2 = bencode_decoder::decode("lee");
  EXPECT_ERR(res2, btc::error_code::trailingInputErr);

  auto res3 = bencode_decoder::decode("d4:spami43eegfs");
  EXPECT_ERR(res3, btc::error_code::trailingInputErr);
}

TEST(BencodeGeneral, RejectEmptyInput) {
  auto res = bencode_decoder::decode("");
  EXPECT_ERR(res, btc::error_code::emptyInputErr);
}

TEST(BencodeGeneral, RejectMaxNestingExceeded) {
  std::string list;
  for (int i = 0; i < 1000; i++) {
    if (i <= 499)
      list.push_back('l');
    else
      list.push_back('e');
  }
  auto res = bencode_decoder::decode(list);
  EXPECT_ERR(res, btc::error_code::maximumNestingLimitExcedeedErr);

  std::string dict;
  for (int i = 0; i < 1000; i++) {
    if (i <= 499)
      dict.push_back('d');
    else
      dict.push_back('e');
  }
  auto res1 = bencode_decoder::decode(dict);
  EXPECT_ERR(res1, btc::error_code::maximumNestingLimitExcedeedErr);
}

TEST(BencodeGeneral, DecodeEncode) {
  std::string val = "d4:dictd3:key5:value6:nestedli42e4:spamd3:subi-7eeee9:"
                    "emptydictde9:emptylistle7:integeri123456789e4:listli0e3:"
                    "fooli1ei2ei3eed5:inner6:foobaree6:neginti-98765ee";

  auto res = bencode_decoder::decode(val);
  ASSERT_OK(res);

  std::string encoded_val = bencode_encoder::encode(*res);
  ASSERT_EQ(encoded_val, val);
}

TEST(BencodeGeneral, decodeRawBytes) {
  std::string input("3:\xFF\x00\x61", 5);
  auto res = bencode_decoder::decode(input);
  ASSERT_OK(res);
  ASSERT_TRUE(res->isStr());
  ASSERT_EQ(res->getStr().size(), 3);
  ASSERT_EQ(res->getStr().at(0), '\xFF');
  ASSERT_EQ(res->getStr().at(1), '\x00');
  ASSERT_EQ(res->getStr().at(2), 'a');
}

// --------------------------------------------------------------------
// INTEGER
// --------------------------------------------------------------------

TEST(BencodeInteger, ParsePositiveInteger) {
  auto res = bencode_decoder::decode("i42e");
  ASSERT_OK(res);

  ASSERT_TRUE(res->isInt());
  EXPECT_EQ(res->getInt(), 42);
}

TEST(BencodeInteger, ParseNegativeInteger) {
  auto res = bencode_decoder::decode("i-17e");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isInt());
  EXPECT_EQ(res->getInt(), -17);
}

TEST(BencodeInteger, RejectEmptyInteger) {
  auto res = bencode_decoder::decode("ie");
  EXPECT_ERR(res, btc::error_code::invalidIntegerErr);
}

TEST(BencodeInteger, ParseMaxPositiveInteger) {
  auto res = bencode_decoder::decode("i9223372036854775807e");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isInt());
  ASSERT_EQ(res->getInt(), std::numeric_limits<btc::BencodeInteger>::max());
}

TEST(BencodeInteger, ParseMaxNegativeInteger) {
  auto res = bencode_decoder::decode("i-9223372036854775808e");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isInt());
  ASSERT_EQ(res->getInt(), std::numeric_limits<btc::BencodeInteger>::min());
}

TEST(BencodeInteger, RejectPositiveOverflowingInteger) {
  auto res = bencode_decoder::decode("i9223372036854775808e");
  EXPECT_ERR(res, btc::error_code::outOfRangeIntegerErr);
}

TEST(BencodeInteger, RejectNegativeoverflowingInteger) {
  auto res = bencode_decoder::decode("i-9223372036854775809e");
  EXPECT_ERR(res, btc::error_code::outOfRangeIntegerErr);
}

TEST(BencodeInteger, RejectLeadingZero) {
  auto res = bencode_decoder::decode("i042e");
  EXPECT_ERR(res, btc::error_code::invalidIntegerErr);
}

TEST(BencodeInteger, RejectNegativeZero) {
  auto res = bencode_decoder::decode("i-0e");
  EXPECT_ERR(res, btc::error_code::invalidIntegerErr);
}

TEST(BencodeInteger, RejectWeirdMix) {
  auto res1 = bencode_decoder::decode("i-01e");
  EXPECT_ERR(res1, btc::error_code::invalidIntegerErr);

  auto res2 = bencode_decoder::decode("i--1e");
  EXPECT_ERR(res2, btc::error_code::invalidIntegerErr);

  auto res3 = bencode_decoder::decode("i1.0e");
  EXPECT_ERR(res3, btc::error_code::invalidIntegerErr);

  auto res4 = bencode_decoder::decode("i1");
  EXPECT_ERR(res4, btc::error_code::missingIntegerTerminatorErr);

  auto res5 = bencode_decoder::decode("i+1e");
  EXPECT_ERR(res5, btc::error_code::invalidIntegerErr);

  auto res6 = bencode_decoder::decode("i e");
  EXPECT_ERR(res6, btc::error_code::invalidIntegerErr);
}

// --------------------------------------------------------------------
// STRING
// --------------------------------------------------------------------

TEST(BencodeString, ParseSimpleString) {
  auto res = bencode_decoder::decode("4:spam");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isStr());
  EXPECT_EQ(res->getStr(), "spam");
}

TEST(BencodeString, ParseEmptyString) {
  auto res = bencode_decoder::decode("0:");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isStr());
  EXPECT_EQ(res->getStr(), "");
}

TEST(BencodeString, RejectLengthMismatchError) {
  auto res = bencode_decoder::decode("5:spam");
  EXPECT_ERR(res, btc::error_code::lengthMismatchErr);
}

TEST(BencodeString, RejectNegativeLengthError) {
  auto res = bencode_decoder::decode("-1:a");
  EXPECT_ERR(res, btc::error_code::negativeStringLengthErr);
}

TEST(BencodeString, RejectOverflowingLength) {
  auto res = bencode_decoder::decode("9999999999999999999999999:");
  EXPECT_ERR(res, btc::error_code::stringTooLargeErr);
}

TEST(BencodeString, RejectSignedLengthError) {
  auto res = bencode_decoder::decode("+1:a");
  EXPECT_ERR(res, btc::error_code::signedStringLengthErr);
}

TEST(BencodeString, RejectMissingColon) {
  auto res = bencode_decoder::decode("1a");
  EXPECT_ERR(res, btc::error_code::missingColonErr);
}

// --------------------------------------------------------------------
// LIST
// --------------------------------------------------------------------

TEST(BencodeList, ParseListOfStrings) {
  auto res = bencode_decoder::decode("l3:abc4:spame");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isList());
  ASSERT_EQ(res->getList().size(), 2);
  ASSERT_EQ(res->getList()[0].getStr(), "abc");
  ASSERT_EQ(res->getList()[1].getStr(), "spam");
}

TEST(BencodeList, ParseListOfIntegers) {
  auto res = bencode_decoder::decode("li32ei45ee");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isList());
  ASSERT_EQ(res->getList().size(), 2);
  ASSERT_EQ(res->getList()[0].getInt(), 32);
  ASSERT_EQ(res->getList()[1].getInt(), 45);
}

TEST(BencodeList, ParseMixedList) {
  auto res = bencode_decoder::decode("li32e4:spame");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isList());
  ASSERT_EQ(res->getList().size(), 2);
  ASSERT_EQ(res->getList()[0].getInt(), 32);
  ASSERT_EQ(res->getList()[1].getStr(), "spam");
}

TEST(BencodeList, ParseEmptyList) {
  auto res = bencode_decoder::decode("le");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isList());
  ASSERT_EQ(res->getList().size(), 0);
}

TEST(BencodeList, RejectMissingTerminator) {
  auto res = bencode_decoder::decode("lle");
  EXPECT_ERR(res, btc::error_code::missingListTerminatorErr);
}

TEST(BencodeList, RejectInvalidElement) {
  auto res = bencode_decoder::decode("lxe");
  EXPECT_ERR(res, btc::error_code::invalidListElementErr);
}

// --------------------------------------------------------------------
// DICT
// --------------------------------------------------------------------

TEST(BencodeDict, ParseSimpleDict) {
  auto res = bencode_decoder::decode("d4:spam3:abc5:zebrai43ee");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isDict());

  ASSERT_TRUE(res->getDict().contains(btc::BencodeString("spam")));
  ASSERT_EQ(res->getDict().at("spam").getStr(), "abc");

  ASSERT_TRUE(res->getDict().contains(btc::BencodeString("zebra")));
  ASSERT_EQ(res->getDict().at("zebra").getInt(), 43);
}

TEST(BencodeDict, ParseEmptyDict) {
  auto res = bencode_decoder::decode("de");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isDict());
  ASSERT_TRUE(res->getDict().empty());
}

TEST(BencodeDict, RejectMissingTerminator) {
  auto res = bencode_decoder::decode("d");
  EXPECT_ERR(res, btc::error_code::missingDictTerminatorErr);
}

TEST(BencodeDict, RejectNonStringKey) {
  auto res = bencode_decoder::decode("di34e4:spame");
  EXPECT_ERR(res, btc::error_code::nonStringKeyErr);
}

TEST(BencodeDict, RejectDuplicateKey) {
  auto res = bencode_decoder::decode("d4:spami43e4:spami56ee");
  EXPECT_ERR(res, btc::error_code::duplicateKeyErr);
}
