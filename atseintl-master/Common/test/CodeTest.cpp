#include "test/include/CppUnitHelperMacros.h"

#include "Common/Code.h"

using namespace std;

namespace tse
{
class CodeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CodeTest);
  CPPUNIT_TEST(testCodeWithOneChar);
  CPPUNIT_TEST(testCodeWithString);
  CPPUNIT_TEST(testCompare);
  CPPUNIT_TEST(testIntCompare);
  CPPUNIT_TEST(testLessThan);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCodeWithOneChar()
  {
    Code<1, unsigned char> codeStr;
    Code<1, unsigned char> codeStr2("c");

    codeStr = 'c';

    CPPUNIT_ASSERT(codeStr == "c");
    CPPUNIT_ASSERT(codeStr == codeStr2);
  }

  void testCodeWithString()
  {
    Code<20, unsigned char> codeStr1("sample string");
    Code<20, unsigned char> codeStr2 = codeStr1;

    CPPUNIT_ASSERT(codeStr1 == codeStr2);

    codeStr2[codeStr2.size() - 1] = 'q';

    CPPUNIT_ASSERT(codeStr1 < codeStr2);
    CPPUNIT_ASSERT(codeStr1.find("str") == 7);
    CPPUNIT_ASSERT(codeStr2.find("string") == string::npos); //-1 );
  }

  void testCompare()
  {
    Code<8> loc1 = "US";
    Code<3> loc2 = "US";
    Code<3> loc3 = "CA";

    CPPUNIT_ASSERT(loc1 == loc2);
    CPPUNIT_ASSERT(loc1 != loc3);
  }
  void testIntCompare()
  {
    typedef Code<3> m3Code;
    m3Code c1 = "ABC";
    m3Code c2 = "AB";

    CPPUNIT_ASSERT(c1 != m3Code::toInt("AB"));
    CPPUNIT_ASSERT(!c1.equalToConst("AB"));

    CPPUNIT_ASSERT(!(c1 == m3Code::toInt("AB")));
    CPPUNIT_ASSERT(!(c1.equalToConst("AB")));

    CPPUNIT_ASSERT(c1 == m3Code::toInt("ABC"));
    CPPUNIT_ASSERT(c1.equalToConst("ABC"));

    CPPUNIT_ASSERT(c2 == m3Code::toInt("AB"));
    CPPUNIT_ASSERT(c2.equalToConst("AB"));


    typedef Code<4> m4Code;
    m4Code c3 = "A";
    CPPUNIT_ASSERT(c3 == m3Code::toInt("A"));
    CPPUNIT_ASSERT(c3.equalToConst("A"));
    m4Code c4 = "ABCD";
    CPPUNIT_ASSERT(c4 != m4Code::toInt("ABC"));
    CPPUNIT_ASSERT(!c4.equalToConst("ABC"));
    CPPUNIT_ASSERT(c4 == m4Code::toInt("ABCD"));
    CPPUNIT_ASSERT(c4.equalToConst("ABCD"));
  }
  void testLessThan()
  {
    typedef Code<2> c2;
    c2 a("AB");
    c2 b("BA");
    CPPUNIT_ASSERT(a < b);

    typedef Code<3> c3;
    c3 x("AB");
    c3 y("ABC");
    CPPUNIT_ASSERT(x < y);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CodeTest);
}
