// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/Common/CompactOptional.h"
#include "DataModel/Common/CompactOptionalIO.h"
#include "DataModel/Common/OptionalCode.h"
#include "DataModel/Common/CodeIO.h"
#include <string>

namespace tax
{

template<>
struct CompactOptionalTraits<std::string>
{
  static std::string singularValue() { return std::string(); }
  static bool isSingularValue(const std::string& v) { return v.empty(); }
};

struct XTag{};
typedef Code<XTag, 2> XCode;


class CompactOptionalTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CompactOptionalTest);
  CPPUNIT_TEST(basic);
  CPPUNIT_TEST(string);
  CPPUNIT_TEST(code);
  CPPUNIT_TEST_SUITE_END();

public:
  void basic()
  {
    CompactOptional<int> i{0}, j{-1}, nv;

    CPPUNIT_ASSERT(i.has_value());
    CPPUNIT_ASSERT_EQUAL(0, i.value());

    CPPUNIT_ASSERT(j.has_value());
    CPPUNIT_ASSERT_EQUAL(-1, j.value());

    CPPUNIT_ASSERT(!nv.has_value());
  }

  void string()
  {
    CompactOptional<std::string> nv, a{"a"}, aa{"aa"};

    CPPUNIT_ASSERT(a.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string{"a"}, a.value());

    CPPUNIT_ASSERT(aa.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string{"aa"}, aa.value());

    CPPUNIT_ASSERT(!nv.has_value());
  }

  void code()
  {
    CompactOptional<XCode> noVal;
    CompactOptional<XCode> uninitCode{XCode{UninitializedCode}};
    CompactOptional<XCode> AA{XCode{"AA"}};

    CPPUNIT_ASSERT(AA.has_value());
    CPPUNIT_ASSERT_EQUAL(XCode{"AA"}, AA.value());

    CPPUNIT_ASSERT(uninitCode.has_value());
    CPPUNIT_ASSERT_EQUAL(uninitCode.value(), XCode{UninitializedCode});

    CPPUNIT_ASSERT(!noVal.has_value());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CompactOptionalTest);

} // namespace tax
