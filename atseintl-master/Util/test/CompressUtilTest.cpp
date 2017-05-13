#include <iostream>
#include <fstream>
#include <string>
#include "test/include/CppUnitHelperMacros.h"
#include "Util/Base64.h"
#include "Util/CompressUtil.h"

using namespace std;

namespace tse
{
class CompressUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CompressUtilTest);
  CPPUNIT_TEST(testCompressDecompressSimpleString);
  CPPUNIT_TEST(testCompressDecompressInputStringToOutputString);
  CPPUNIT_TEST(testCompressBz2);
  CPPUNIT_TEST(testDecompressBz2);
  CPPUNIT_TEST_SUITE_END();

private:
  std::string uncmpStr;
  std::string cmpStr;

public:
  CompressUtilTest() {}

  void setUp() {}

  void tearDown() {}

  void loadFile(const char* fileName, std::string& bufStr)
  {
    ifstream inFile(fileName);
    inFile.seekg(0, ios::end);
    int length = inFile.tellg();
    inFile.seekg(0, ios::beg);
    char* buf = new char[length + 1];
    inFile.read(buf, length);
    buf[length] = '\0';
    inFile.close();
    bufStr.assign(buf);
    delete[] buf;
  }

  void testCompressDecompressSimpleString()
  {
    std::string initialString =
        "The quick brown fox jumped over the lazy dog's back 0123456789 times.";
    std::string buff = initialString;

    CPPUNIT_ASSERT(CompressUtil::compress(buff));
    CPPUNIT_ASSERT(!(initialString == buff));

    CPPUNIT_ASSERT(CompressUtil::decompress(buff));
    CPPUNIT_ASSERT_EQUAL(initialString, buff);
  }

  void testCompressDecompressInputStringToOutputString()
  {
    std::string initialString =
        "The quick brown fox jumped over the lazy dog's back 0123456789 times.";
    std::string input = initialString;
    std::string output;
    std::string returnString;

    CPPUNIT_ASSERT(CompressUtil::compress(input, output));
    // Should not change input string
    CPPUNIT_ASSERT_EQUAL(input, initialString);
    CPPUNIT_ASSERT(!(output == input));

    CPPUNIT_ASSERT(CompressUtil::decompress(output, returnString));
    CPPUNIT_ASSERT(!(output == input));
    CPPUNIT_ASSERT_EQUAL(initialString, returnString);
  }

  void testCompressBz2()
  {
    loadFile("bz2TestMsg.xml", uncmpStr);
    loadFile("bz2TestMsg.b64", cmpStr);

    CPPUNIT_ASSERT(CompressUtil::compressBz2(uncmpStr));
    string encodedStr = Base64::encode(uncmpStr);
    CPPUNIT_ASSERT_EQUAL(cmpStr, encodedStr);
  }

  void testDecompressBz2()
  {
    loadFile("bz2TestMsg.xml", uncmpStr);
    loadFile("bz2TestMsg.b64", cmpStr);

    string decodedStr = Base64::decode(cmpStr);
    CPPUNIT_ASSERT(CompressUtil::decompressBz2(decodedStr));
    CPPUNIT_ASSERT_EQUAL(uncmpStr, decodedStr);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CompressUtilTest);
}
