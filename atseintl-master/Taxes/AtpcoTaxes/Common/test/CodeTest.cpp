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
#include "DataModel/Common/Code.h"
#include "DataModel/Common/CodeIO.h"

namespace tax
{

struct Tag{};

class CodeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CodeTest);
  CPPUNIT_TEST(equality);
  CPPUNIT_TEST(equality_with_literal);
  CPPUNIT_TEST(order);
  CPPUNIT_TEST(empty_);
  CPPUNIT_TEST(asString);
  CPPUNIT_TEST(codeFromString_);
  CPPUNIT_TEST(output);
  CPPUNIT_TEST(rawArray);
  CPPUNIT_TEST(assign_literal);
  CPPUNIT_TEST(sizes);
  CPPUNIT_TEST_SUITE_END();

  Code<Tag, 2> AA, AB, BB, empty;

public:

  CodeTest() : AA("AA"), AB("AB"), BB("BB"), empty(UninitializedCode) {}

  void equality()
  {
    CPPUNIT_ASSERT(AA == AA);
    CPPUNIT_ASSERT(AB == AB);
    CPPUNIT_ASSERT(empty  == empty);
    CPPUNIT_ASSERT(!(AA == AB));
    CPPUNIT_ASSERT(!(AA == BB));
    CPPUNIT_ASSERT(!(AB == AA));
    CPPUNIT_ASSERT(!(empty  == AA));

    CPPUNIT_ASSERT((AA != AB));
    CPPUNIT_ASSERT((AA != BB));
    CPPUNIT_ASSERT((AB != AA));
    CPPUNIT_ASSERT((empty != AA));
    CPPUNIT_ASSERT(!(AA != AA));
    CPPUNIT_ASSERT(!(AB != AB));
    CPPUNIT_ASSERT(!(empty != empty));
  }

  void equality_with_literal()
  {
    CPPUNIT_ASSERT(AA == "AA");
    CPPUNIT_ASSERT(AB == "AB");
    CPPUNIT_ASSERT(!(AA == "AB"));
    CPPUNIT_ASSERT(!(AA == "BB"));
    CPPUNIT_ASSERT(!(AB == "AA"));
    CPPUNIT_ASSERT(!(empty  == "AA"));

    CPPUNIT_ASSERT((AA != "AB"));
    CPPUNIT_ASSERT((AA != "BB"));
    CPPUNIT_ASSERT((AB != "AA"));
    CPPUNIT_ASSERT((empty != "AA"));
    CPPUNIT_ASSERT(!(AA != "AA"));
    CPPUNIT_ASSERT(!(AB != "AB"));
  }

  void order()
  {
    CPPUNIT_ASSERT(empty < AA);
    CPPUNIT_ASSERT(empty < AB);
    CPPUNIT_ASSERT(empty < BB);
    CPPUNIT_ASSERT(AA < AB);
    CPPUNIT_ASSERT(AA < BB);
    CPPUNIT_ASSERT(AB < BB);

    CPPUNIT_ASSERT(!(empty < empty));
    CPPUNIT_ASSERT(!(AA < AA));
    CPPUNIT_ASSERT(!(AB < AB));
    CPPUNIT_ASSERT(!(BB < BB));

    CPPUNIT_ASSERT(!(AA < empty));
    CPPUNIT_ASSERT(!(AB < empty));
    CPPUNIT_ASSERT(!(BB < empty));
    CPPUNIT_ASSERT(!(AB < AA));
    CPPUNIT_ASSERT(!(BB < AA));
    CPPUNIT_ASSERT(!(BB < AB));
  }

  void empty_()
  {
    CPPUNIT_ASSERT(empty.empty());
    CPPUNIT_ASSERT(!AA.empty());
    CPPUNIT_ASSERT(!AB.empty());
    CPPUNIT_ASSERT(!BB.empty());
  }

  void asString()
  {
    CPPUNIT_ASSERT_EQUAL(empty.asString(), std::string(""));
    CPPUNIT_ASSERT_EQUAL(AA.asString(), std::string("AA"));
    CPPUNIT_ASSERT_EQUAL(AB.asString(), std::string("AB"));
    CPPUNIT_ASSERT_EQUAL(BB.asString(), std::string("BB"));
  }

  void codeFromString_()
  {
    typedef Code<Tag, 2> Code2;
    Code<Tag, 2> c1(UninitializedCode);
    std::string XX("XX"), XXX("XXX"), X("X");

    CPPUNIT_ASSERT(codeFromString(XX, c1));
    CPPUNIT_ASSERT_EQUAL(c1, Code2("XX"));

    CPPUNIT_ASSERT(!codeFromString(XXX, c1));
    CPPUNIT_ASSERT(!codeFromString(X, c1));
    CPPUNIT_ASSERT_EQUAL(c1, Code2("XX")); //unchanged

    const char *YY = "YY", *YYY = "YYY", *Y = "Y";
    CPPUNIT_ASSERT(codeFromString(YY, c1));
    CPPUNIT_ASSERT_EQUAL(c1, Code2("YY"));

    CPPUNIT_ASSERT(!codeFromString(YYY, c1));
    CPPUNIT_ASSERT(!codeFromString(Y, c1));
    CPPUNIT_ASSERT_EQUAL(c1, Code2("YY")); //unchanged
  }

  void output()
  {
    std::ostringstream o;
    o << AA << empty << AB << empty << BB;
    CPPUNIT_ASSERT_EQUAL(o.str(), std::string("AA""AB""BB"));
  }

  void rawArray()
  {
    CPPUNIT_ASSERT_EQUAL(AA.rawArray()[0], 'A');
    CPPUNIT_ASSERT_EQUAL(AA.rawArray()[1], 'A');
    CPPUNIT_ASSERT_EQUAL(AB.rawArray()[0], 'A');
    CPPUNIT_ASSERT_EQUAL(AB.rawArray()[1], 'B');
    CPPUNIT_ASSERT_EQUAL(BB.rawArray()[0], 'B');
    CPPUNIT_ASSERT_EQUAL(BB.rawArray()[1], 'B');
    CPPUNIT_ASSERT_EQUAL(empty.rawArray()[0], '\0');
  }

  void assign_literal()
  {
    Code<Tag, 2> US("US");
    CPPUNIT_ASSERT(US == "US");
    US = "UK";
    CPPUNIT_ASSERT(US == "UK");
  }

  void sizes()
  {
    CPPUNIT_ASSERT_EQUAL((sizeof(Code<Tag, 1>)), 1UL);
    CPPUNIT_ASSERT_EQUAL((sizeof(Code<Tag, 2>)), 2UL);
    CPPUNIT_ASSERT_EQUAL((sizeof(Code<Tag, 3>)), 4UL);
    CPPUNIT_ASSERT_EQUAL((sizeof(Code<Tag, 4>)), 4UL);
    CPPUNIT_ASSERT_EQUAL((sizeof(Code<Tag, 5>)), 8ul);
    CPPUNIT_ASSERT_EQUAL((sizeof(Code<Tag, 6>)), 8ul);
    CPPUNIT_ASSERT_EQUAL((sizeof(Code<Tag, 7>)), 8ul);
    CPPUNIT_ASSERT_EQUAL((sizeof(Code<Tag, 8>)), 8ul);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CodeTest);

} // namespace tax

