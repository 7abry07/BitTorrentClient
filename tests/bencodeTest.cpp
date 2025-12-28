#include "../bencode.h"
#include <gtest/gtest.h>
#include <limits>
#include <string>

using bencode_decoder = BitTorrentClient::Bencode::Decoder;
using bencode_encoder = BitTorrentClient::Bencode::Encoder;
using bencode_printer = BitTorrentClient::Bencode::Printer;
using bencode_err = BitTorrentClient::Bencode::Error::err_code;
using bencode_val = BitTorrentClient::Bencode::Value;

#define EXPECT_OK(expr) EXPECT_TRUE((expr).has_value())
#define ASSERT_OK(expr) ASSERT_TRUE((expr).has_value())
#define EXPECT_ERR(expr, err)                                                  \
  do {                                                                         \
    ASSERT_FALSE((expr).has_value());                                          \
    EXPECT_EQ((expr).error().code, err);                                       \
  } while (0)

// --------------------------------------------------------------------
// GENERAL
// --------------------------------------------------------------------

TEST(BencodeGeneral, RejectInvalidTrailingInput) {
  auto res = bencode_decoder::decode("i43ee");
  EXPECT_ERR(res, bencode_err::trailingInputErr);

  auto res1 = bencode_decoder::decode("4:spamfdf");
  EXPECT_ERR(res1, bencode_err::trailingInputErr);

  auto res2 = bencode_decoder::decode("lee");
  EXPECT_ERR(res2, bencode_err::trailingInputErr);

  auto res3 = bencode_decoder::decode("d4:spami43eegfs");
  EXPECT_ERR(res3, bencode_err::trailingInputErr);
}

TEST(BencodeGeneral, RejectEmptyInput) {
  auto res = bencode_decoder::decode("");
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
  auto res = bencode_decoder::decode(list);
  EXPECT_ERR(res, bencode_err::maximumNestingLimitExcedeedErr);

  std::string dict;
  for (int i = 0; i < 1000; i++) {
    if (i <= 499)
      dict.push_back('d');
    else
      dict.push_back('e');
  }
  auto res1 = bencode_decoder::decode(dict);
  EXPECT_ERR(res1, bencode_err::maximumNestingLimitExcedeedErr);
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

TEST(BencodeGeneral, CorrectPrinterFormattedValue) {
  std::string val = "d4:dictd3:key5:value6:nestedli42e4:spamd3:subi-7eeee9:"
                    "emptydictde9:emptylistle7:integeri123456789e4:listli0e3:"
                    "fooli1ei2ei3eed5:inner6:foobaree6:neginti-98765ee";
  std::string correctFormattedValue = R"(DICT
{
  dict: DICT
  {
    key: value
    nested: LIST
    [
      42
      spam
      DICT
      {
        sub: -7
      }
    ]
  }
  emptydict: DICT
  {
  }
  emptylist: LIST
  [
  ]
  integer: 123456789
  list: LIST
  [
    0
    foo
    LIST
    [
      1
      2
      3
    ]
    DICT
    {
      inner: foobar
    }
  ]
  negint: -98765
})";
  auto res = bencode_decoder::decode(val);

  ASSERT_OK(res);
  ASSERT_EQ(bencode_printer::getFormattedValue(*res, 2), correctFormattedValue);
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
  EXPECT_ERR(res, bencode_err::invalidIntegerErr);
}

TEST(BencodeInteger, ParseMaxPositiveInteger) {
  auto res = bencode_decoder::decode("i9223372036854775807e");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isInt());
  ASSERT_EQ(res->getInt(), std::numeric_limits<bencode_val::Integer>::max());
}

TEST(BencodeInteger, ParseMaxNegativeInteger) {
  auto res = bencode_decoder::decode("i-9223372036854775808e");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isInt());
  ASSERT_EQ(res->getInt(), std::numeric_limits<bencode_val::Integer>::min());
}

TEST(BencodeInteger, RejectPositiveOverflowingInteger) {
  auto res = bencode_decoder::decode("i9223372036854775808e");
  EXPECT_ERR(res, bencode_err::outOfRangeIntegerErr);
}

TEST(BencodeInteger, RejectNegativeoverflowingInteger) {
  auto res = bencode_decoder::decode("i-9223372036854775809e");
  EXPECT_ERR(res, bencode_err::outOfRangeIntegerErr);
}

TEST(BencodeInteger, RejectLeadingZero) {
  auto res = bencode_decoder::decode("i042e");
  EXPECT_ERR(res, bencode_err::invalidIntegerErr);
}

TEST(BencodeInteger, RejectNegativeZero) {
  auto res = bencode_decoder::decode("i-0e");
  EXPECT_ERR(res, bencode_err::invalidIntegerErr);
}

TEST(BencodeInteger, RejectWeirdMix) {
  auto res1 = bencode_decoder::decode("i-01e");
  EXPECT_ERR(res1, bencode_err::invalidIntegerErr);

  auto res2 = bencode_decoder::decode("i--1e");
  EXPECT_ERR(res2, bencode_err::invalidIntegerErr);

  auto res3 = bencode_decoder::decode("i1.0e");
  EXPECT_ERR(res3, bencode_err::invalidIntegerErr);

  auto res4 = bencode_decoder::decode("i1");
  EXPECT_ERR(res4, bencode_err::missingIntegerTerminatorErr);

  auto res5 = bencode_decoder::decode("i+1e");
  EXPECT_ERR(res5, bencode_err::invalidIntegerErr);

  auto res6 = bencode_decoder::decode("i e");
  EXPECT_ERR(res6, bencode_err::invalidIntegerErr);
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
  EXPECT_ERR(res, bencode_err::lengthMismatchErr);
}

TEST(BencodeString, RejectNegativeLengthError) {
  auto res = bencode_decoder::decode("-1:a");
  EXPECT_ERR(res, bencode_err::negativeStringLengthErr);
}

TEST(BencodeString, RejectOverflowingLength) {
  auto res = bencode_decoder::decode("9999999999999999999999999:");
  EXPECT_ERR(res, bencode_err::stringTooLargeErr);
}

TEST(BencodeString, RejectSignedLengthError) {
  auto res = bencode_decoder::decode("+1:a");
  EXPECT_ERR(res, bencode_err::signedStringLengthErr);
}

TEST(BencodeString, RejectMissingColon) {
  auto res = bencode_decoder::decode("1a");
  EXPECT_ERR(res, bencode_err::missingColonErr);
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
  EXPECT_ERR(res, bencode_err::missingListTerminatorErr);
}

TEST(BencodeList, RejectInvalidElement) {
  auto res = bencode_decoder::decode("lxe");
  EXPECT_ERR(res, bencode_err::invalidListElementErr);
}

// --------------------------------------------------------------------
// DICT
// --------------------------------------------------------------------

TEST(BencodeDict, ParseSimpleDict) {
  auto res = bencode_decoder::decode("d4:spam3:abc5:zebrai43ee");
  ASSERT_OK(res);
  ASSERT_TRUE(res->isDict());

  ASSERT_TRUE(res->getDict().contains(bencode_val::String("spam")));
  ASSERT_EQ(res->getDict().at("spam").getStr(), "abc");

  ASSERT_TRUE(res->getDict().contains(bencode_val::String("zebra")));
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
  EXPECT_ERR(res, bencode_err::missingDictTerminatorErr);
}

TEST(BencodeDict, RejectNonStringKey) {
  auto res = bencode_decoder::decode("di34e4:spame");
  EXPECT_ERR(res, bencode_err::nonStringKeyErr);
}

TEST(BencodeDict, RejectDuplicateKey) {
  auto res = bencode_decoder::decode("d4:spami43e4:spami56ee");
  EXPECT_ERR(res, bencode_err::duplicateKeyErr);
}

TEST(BencodeDict, RejectUnorderedKeys) {
  auto res = bencode_decoder::decode("d4:cccci34e4:aaaai56ee");
  EXPECT_ERR(res, bencode_err::unorderedKeysErr);
}
