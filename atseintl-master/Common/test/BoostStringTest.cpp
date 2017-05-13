#include "test/include/CppUnitHelperMacros.h"

#include "Common/BoostString.h"

namespace tse
{
class BoostStringTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BoostStringTest);
  CPPUNIT_TEST(testDefaultCtor);
  CPPUNIT_TEST(testInitCtors);
  CPPUNIT_TEST(testCopyCtors);
  CPPUNIT_TEST(testMoveCtors);
  CPPUNIT_TEST(testAssign);
  CPPUNIT_TEST(testCast);
  CPPUNIT_TEST(testSwap);
  CPPUNIT_TEST_SUITE_END();

public:
  void testDefaultCtor()
  {
    BoostString str1;
    BoostString str2{};
    CPPUNIT_ASSERT(str1.empty());
    CPPUNIT_ASSERT(str2.empty());
  }

  void testInitCtors()
  {
    BoostString str1 = "abc";
    BoostString str2("abc");
    BoostString str3("abcdef", 3);
    BoostString str4(std::string("abc"));
    BoostString str5(Code<3>("abc"));
    // BoostString str6 = std::string("abc"); // implicit conversion from std::string not allowed
    // BoostString str7 = Code("abc"); // implicit conversion from tse::Code not allowed

    CPPUNIT_ASSERT(str1 == "abc");
    CPPUNIT_ASSERT(str2 == "abc");
    CPPUNIT_ASSERT(str3 == "abc");
    CPPUNIT_ASSERT(str4 == "abc");
    CPPUNIT_ASSERT(str5 == "abc");
  }

  void testCopyCtors()
  {
    BoostString str1("abc");
    BoostString str2(str1);
    BoostString str3(str2);

    CPPUNIT_ASSERT(str1 == "abc");
    CPPUNIT_ASSERT(str2 == "abc");
    CPPUNIT_ASSERT(str3 == "abc");
  }

  void testMoveCtors()
  {
    boost::container::string str1("abc");
    BoostString str2(std::move(str1));
    BoostString str3(std::move(str2));

    CPPUNIT_ASSERT(str1.empty());
    CPPUNIT_ASSERT(str2.empty());
    CPPUNIT_ASSERT(str3 == "abc");
  }

  void testCast()
  {
    BoostString str1 = "abc";
    std::string str2 = static_cast<std::string>(str1);
    std::string str3 = (std::string)(str1);
    Code<3> str4 = static_cast<Code<3>>(str1);
    Code<3> str5 = (Code<3>)(str1);
    // std::string str6 = str1; // implicit conversion from BoostString not allowed
    // Code<3> str5 = str1; // implicit conversion from BoostString not allowed

    CPPUNIT_ASSERT(str2 == str1);
    CPPUNIT_ASSERT(str3 == str1);
    CPPUNIT_ASSERT(str4 == str1);
    CPPUNIT_ASSERT(str5 == str1);
  }

  void testAssign()
  {
    BoostString str;

    str = "abc";
    CPPUNIT_ASSERT(str == "abc");

    str = std::string("def");
    CPPUNIT_ASSERT(str == "def");

    str = 'g';
    CPPUNIT_ASSERT(str == "g");

    str = Code<3>("hij");
    CPPUNIT_ASSERT(str == "hij");
  }

  void testSwap()
  {
    BoostString str1("abc");
    BoostString str2("def");

    std::swap(str1, str2);

    CPPUNIT_ASSERT(str1 == "def");
    CPPUNIT_ASSERT(str2 == "abc");
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(BoostStringTest);
}
