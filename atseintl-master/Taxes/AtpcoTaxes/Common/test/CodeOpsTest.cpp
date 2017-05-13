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
#include "DataModel/Common/CodeOps.h"
#include "DataModel/Common/CodeIO.h"

namespace tax
{

struct TagForOps{};

class CodeOpsTest : public CppUnit::TestFixture
{
  typedef TagForOps Tag;

  CPPUNIT_TEST_SUITE(CodeOpsTest);
  CPPUNIT_TEST(equal_);
  CPPUNIT_TEST(match_string);
  CPPUNIT_TEST(match_code);
  CPPUNIT_TEST(to_int);
  CPPUNIT_TEST_SUITE_END();

  Code<Tag, 2> AA, AB, BB, empty;

public:

  CodeOpsTest() : AA("AA"), AB("AB"), BB("BB"), empty(UninitializedCode) {}

  void equal_()
  {
    CPPUNIT_ASSERT(equal(AA, "AA"));
    CPPUNIT_ASSERT(equal(BB, "BB"));
    CPPUNIT_ASSERT(equal(AA, std::string("AA")));
    CPPUNIT_ASSERT(equal(BB, std::string("BB")));

    CPPUNIT_ASSERT(!equal(AA, "AB"));
    CPPUNIT_ASSERT(!equal(BB, "AA"));
    CPPUNIT_ASSERT(!equal(AA, std::string("AB")));
    CPPUNIT_ASSERT(!equal(BB, std::string("AA")));

    CPPUNIT_ASSERT(!equal(BB, ""));
    CPPUNIT_ASSERT(!equal(BB, "B"));
    CPPUNIT_ASSERT(!equal(BB, "BBB"));
  }

  void match_string()
  {
    CPPUNIT_ASSERT( matches("AA", AA));
    CPPUNIT_ASSERT(!matches("AA", BB));
    CPPUNIT_ASSERT(!matches("AA", empty));

    CPPUNIT_ASSERT(!matches("A", AA));
    CPPUNIT_ASSERT(!matches("A", AB));
    CPPUNIT_ASSERT(!matches("A", empty));

    CPPUNIT_ASSERT( matches("", AA));
    CPPUNIT_ASSERT( matches("", BB));
    CPPUNIT_ASSERT( matches("", empty));
  }

  void match_code()
  {
    CPPUNIT_ASSERT( matches(AA, AA));
    CPPUNIT_ASSERT(!matches(AA, BB));
    CPPUNIT_ASSERT(!matches(AA, empty));

    CPPUNIT_ASSERT( matches(empty, AA));
    CPPUNIT_ASSERT( matches(empty, BB));
    CPPUNIT_ASSERT( matches(empty, empty));
  }

  void to_int()
  {
    typedef Code<Tag, 1, 3> NumCode;
    CPPUNIT_ASSERT_EQUAL(1, toInt(NumCode("1")));
    CPPUNIT_ASSERT_EQUAL(12, toInt(NumCode("12")));
    CPPUNIT_ASSERT_EQUAL(550, toInt(NumCode("550")));
    CPPUNIT_ASSERT_EQUAL(0, toInt(NumCode("0")));
    CPPUNIT_ASSERT_EQUAL(0, toInt(NumCode(UninitializedCode)));
    CPPUNIT_ASSERT_EQUAL(1, toInt(NumCode("1A"))); // silly, but preserves backwards compatibility
    CPPUNIT_ASSERT_EQUAL(0, toInt(NumCode("A")));
    CPPUNIT_ASSERT_EQUAL(0, toInt(NumCode("AAA")));
    CPPUNIT_ASSERT_EQUAL(0, toInt(NumCode("AA1")));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CodeOpsTest);

} // namespace tax

