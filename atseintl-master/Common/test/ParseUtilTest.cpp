#include "Common/ParseUtil.h"
#include "test/include/CppUnitHelperMacros.h"

#include <iostream>
#include <string>

using namespace tse;
using namespace std;

namespace tse
{
class ParseUtilTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(ParseUtilTest);
  CPPUNIT_TEST(testParseIndexesString);
  CPPUNIT_TEST(testParseIndexesStringNegativeTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void testParseIndexesString()
  {
    std::vector<uint16_t> result;
    std::string inputStr = "2/4/7-9";
    ParseUtil::parseIndexes(result, inputStr, "/");

    std::string outputStr;
    intVecToString(result, outputStr);
    CPPUNIT_ASSERT_EQUAL(std::string("2 4 7 8 9 "), outputStr);
  }

  void testParseIndexesStringNegativeTest()
  {
    // negative tests
    std::vector<uint16_t> result;
    std::string inputStr = "2/4/";
    ParseUtil::parseIndexes(result, inputStr, "/");

    std::string outputStr;
    intVecToString(result, outputStr);
    CPPUNIT_ASSERT_EQUAL(std::string("2 4 "), outputStr);
  }

  void intVecToString(const std::vector<uint16_t>& result, std::string& strResult)
  {
    std::ostringstream outputStr;
    std::vector<uint16_t>::const_iterator indexIter = result.begin();
    const std::vector<uint16_t>::const_iterator indexIterEnd = result.end();
    for (; indexIter != indexIterEnd; indexIter++)
    {
      outputStr << *indexIter << " ";
    }
    strResult = outputStr.str();
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(ParseUtilTest);
}
