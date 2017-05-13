// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "Util/Base64.h"

namespace tse
{

class Base64Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Base64Test);
  CPPUNIT_TEST(testEncodeEmpty);
  CPPUNIT_TEST(testEncodeFull);
  CPPUNIT_TEST(testEncodePartial);
  CPPUNIT_TEST(testEncodeShort);
  CPPUNIT_TEST(testEncodeHighBytes);
  CPPUNIT_TEST(testDecodeEmpty);
  CPPUNIT_TEST(testDecodeFull);
  CPPUNIT_TEST(testDecodePartial);
  CPPUNIT_TEST(testDecodeShort);
  CPPUNIT_TEST(testDecodeHighBytes);
  CPPUNIT_TEST(testDecodeInvalidTooShort);
  CPPUNIT_TEST(testDecodeInvalidWrongCharacters);
  CPPUNIT_TEST(testDecodeInvalidWrongCharacters2);
  CPPUNIT_TEST(testDecodeInvalidWrongEnding);
  CPPUNIT_TEST(testDecodeInvalidWrongEnding2);
  CPPUNIT_TEST_SUITE_END();

public:
  void testEncodeEmpty()
  {
    const std::string empty;

    const std::string ret = Base64::encode(empty);

    CPPUNIT_ASSERT_EQUAL(empty, ret);
  }

  void testEncodeFull()
  {
    const std::string input = "The quick brown fox jumps over the lazy dog.\n";
    const std::string expected = "VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZy4K";

    const std::string ret = Base64::encode(input);

    CPPUNIT_ASSERT_EQUAL(expected, ret);
  }

  void testEncodePartial()
  {
    const std::string input = "The quick brown fox jumps over the lazy dog.";
    const std::string expected = "VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZy4=";

    const std::string ret = Base64::encode(input);

    CPPUNIT_ASSERT_EQUAL(expected, ret);
  }

  void testEncodeShort()
  {
    const std::string input = "!";
    const std::string expected = "IQ==";

    const std::string ret = Base64::encode(input);

    CPPUNIT_ASSERT_EQUAL(expected, ret);
  }

  void testEncodeHighBytes()
  {
    const std::string input = "\xDE\xAD\xBE\xEF";
    const std::string expected = "3q2+7w==";

    const std::string ret = Base64::encode(input);

    CPPUNIT_ASSERT_EQUAL(expected, ret);
  }

  void testDecodeEmpty()
  {
    const std::string empty;

    const std::string ret = Base64::decode(empty);

    CPPUNIT_ASSERT_EQUAL(empty, ret);
  }

  void testDecodeFull()
  {
    const std::string input = "VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZy4K";
    const std::string expected = "The quick brown fox jumps over the lazy dog.\n";

    const std::string ret = Base64::decode(input);

    CPPUNIT_ASSERT_EQUAL(expected, ret);
  }

  void testDecodePartial()
  {
    const std::string input = "VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZy4=";
    const std::string expected = "The quick brown fox jumps over the lazy dog.";

    const std::string ret = Base64::decode(input);

    CPPUNIT_ASSERT_EQUAL(expected, ret);
  }

  void testDecodeShort()
  {
    const std::string input = "IQ==";
    const std::string expected = "!";

    const std::string ret = Base64::decode(input);

    CPPUNIT_ASSERT_EQUAL(expected, ret);
  }

  void testDecodeHighBytes()
  {
    const std::string input = "3q2+7w==";
    const std::string expected = "\xDE\xAD\xBE\xEF";

    const std::string ret = Base64::decode(input);

    CPPUNIT_ASSERT_EQUAL(expected, ret);
  }

  void testDecodeInvalidTooShort()
  {
    const std::string input = "VG9vIHNob3J";

    CPPUNIT_ASSERT_THROW(Base64::decode(input), Base64::DecodeError);
  }

  void testDecodeInvalidWrongCharacters()
  {
    const std::string input = "V3JvbmcgY2h?cmFjdGVycw==";

    CPPUNIT_ASSERT_THROW(Base64::decode(input), Base64::DecodeError);
  }

  void testDecodeInvalidWrongCharacters2()
  {
    const std::string input = "V3JvbmcgY2hhcmFj=GVycw==";

    CPPUNIT_ASSERT_THROW(Base64::decode(input), Base64::DecodeError);
  }

  void testDecodeInvalidWrongEnding()
  {
    const std::string input = "V3JvbmcgZW5kaW5nLg=/";

    CPPUNIT_ASSERT_THROW(Base64::decode(input), Base64::DecodeError);
  }

  void testDecodeInvalidWrongEnding2()
  {
    const std::string input = "V3JvbmcgZW5kaW5nLh==";

    CPPUNIT_ASSERT_THROW(Base64::decode(input), Base64::DecodeError);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Base64Test);
}
