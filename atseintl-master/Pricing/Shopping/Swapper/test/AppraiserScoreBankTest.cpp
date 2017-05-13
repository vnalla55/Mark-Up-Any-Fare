//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "Common/ErrorResponseException.h"
#include "Pricing/Shopping/Swapper/AppraiserScoreBank.h"

#include <string>
#include <vector>
#include <map>

using namespace std;

namespace tse
{

using namespace swp;

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

struct DummyAppraiser
{
  typedef std::string Item;
  typedef int Score;
  std::string name;
};

class AppraiserScoreBankTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AppraiserScoreBankTest);
  CPPUNIT_TEST(emptyBankTest);
  CPPUNIT_TEST(exceptionsTest);
  CPPUNIT_TEST(usageTest);
  CPPUNIT_TEST(iterationTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void emptyBankTest()
  {
    Bank b;
    CPPUNIT_ASSERT_EQUAL(0u, b.getItemsCount());
    CPPUNIT_ASSERT_EQUAL(0u, b.getScoresCount());
  }

  void exceptionsTest()
  {
    Bank b;
    // No such key
    CPPUNIT_ASSERT_THROW(b.getScoresForItem("mouse"), ErrorResponseException);

    // zero appraiser pointer
    CPPUNIT_ASSERT_THROW(b.setScore("x", 0, 57), ErrorResponseException);

    DummyAppraiser a1;
    a1.name = "a1";
    b.setScore("rainbow", &a1, 10);

    // No such key
    CPPUNIT_ASSERT_THROW(b.getScoresForItem("flower"), ErrorResponseException);
  }

  void usageTest()
  {
    Bank b;
    DummyAppraiser a1, a2;
    a1.name = "a1";
    a2.name = "a2";

    CPPUNIT_ASSERT_EQUAL(0u, b.getItemsCount());
    CPPUNIT_ASSERT_EQUAL(0u, b.getScoresCount());

    b.setScore("flower", &a1, 5);
    CPPUNIT_ASSERT_EQUAL(1u, b.getItemsCount());
    CPPUNIT_ASSERT_EQUAL(1u, b.getScoresCount());

    b.setScore("flower", &a2, 8);
    CPPUNIT_ASSERT_EQUAL(1u, b.getItemsCount());
    CPPUNIT_ASSERT_EQUAL(2u, b.getScoresCount());

    // score overwrite
    b.setScore("flower", &a1, 51);
    CPPUNIT_ASSERT_EQUAL(1u, b.getItemsCount());
    CPPUNIT_ASSERT_EQUAL(2u, b.getScoresCount());

    b.setScore("rainbow", &a2, -8);
    CPPUNIT_ASSERT_EQUAL(2u, b.getItemsCount());
    CPPUNIT_ASSERT_EQUAL(3u, b.getScoresCount());

    b.removeItem("flower");
    CPPUNIT_ASSERT_EQUAL(1u, b.getItemsCount());
    CPPUNIT_ASSERT_EQUAL(1u, b.getScoresCount());

    // nothing happens
    b.removeItem("meteorite");

    // clear
    b.removeItem("rainbow");
    CPPUNIT_ASSERT_EQUAL(0u, b.getItemsCount());
    CPPUNIT_ASSERT_EQUAL(0u, b.getScoresCount());

    b.setScore("cascade", &a2, 6);
    CPPUNIT_ASSERT_EQUAL(1u, b.getItemsCount());
    CPPUNIT_ASSERT_EQUAL(1u, b.getScoresCount());
  }

  void iterationTest()
  {
    Bank b;
    DummyAppraiser a1, a2, a3;
    a1.name = "a1";
    a2.name = "a2";
    a3.name = "a3";
    b.setScore("Bach", &a2, 10);
    b.setScore("Beethoven", &a2, -5);
    b.setScore("Bach", &a3, 1557);
    b.setScore("Rachmaninov", &a1, 200);
    b.setScore("Bach", &a1, 97);

    CPPUNIT_ASSERT_EQUAL(3u, b.getItemsCount());
    CPPUNIT_ASSERT_EQUAL(5u, b.getScoresCount());

    std::map<std::string, int> bachScores;
    bachScores["a1"] = 97;
    bachScores["a2"] = 10;
    bachScores["a3"] = 1557;

    Bank::Map m = b.getScoresForItem("Bach");
    std::set<std::string> names;
    for (Bank::Map::const_iterator iter = m.begin(); iter != m.end(); ++iter)
    {
      CPPUNIT_ASSERT_EQUAL(bachScores[iter->first->name], iter->second);
      names.insert(iter->first->name);
    }
    std::set<std::string> EXPECTED_NAMES;
    EXPECTED_NAMES.insert("a1");
    EXPECTED_NAMES.insert("a2");
    EXPECTED_NAMES.insert("a3");
    CPPUNIT_ASSERT(EXPECTED_NAMES == names);

    m = b.getScoresForItem("Beethoven");
    CPPUNIT_ASSERT_EQUAL(std::string("a2"), m.begin()->first->name);
    CPPUNIT_ASSERT_EQUAL(-5, m.begin()->second);

    m = b.getScoresForItem("Rachmaninov");
    CPPUNIT_ASSERT_EQUAL(std::string("a1"), m.begin()->first->name);
    CPPUNIT_ASSERT_EQUAL(200, m.begin()->second);
  }

private:
  typedef AppraiserScoreBank<DummyAppraiser> Bank;
};

CPPUNIT_TEST_SUITE_REGISTRATION(AppraiserScoreBankTest);

} // namespace tse
