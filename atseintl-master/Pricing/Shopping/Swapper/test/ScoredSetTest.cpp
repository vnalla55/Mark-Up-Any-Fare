
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

#include "Pricing/Shopping/Swapper/ScoredSet.h"

#include <string>
#include <iostream>
#include <sstream>

using namespace std;

namespace tse
{

using namespace swp;

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class ScoredSetTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ScoredSetTest);
  CPPUNIT_TEST(basicTest);
  CPPUNIT_TEST(basicMaxTest);
  CPPUNIT_TEST(advancedTest);
  CPPUNIT_TEST(addDuplicate);
  CPPUNIT_TEST(getScoreForNonexistentElem);
  CPPUNIT_TEST(updateScoreForNonexistentElem);
  CPPUNIT_TEST(topOnEmptySet);
  CPPUNIT_TEST(popOnEmptySet);
  CPPUNIT_TEST(displayTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void basicTest()
  {
    ScoredSet<std::string, int, MinComparePolicy<int> > s;
    CPPUNIT_ASSERT_EQUAL(true, s.isEmpty());
    CPPUNIT_ASSERT_EQUAL(0u, s.getSize());
    s.add("Eat soup", 10);
    CPPUNIT_ASSERT_EQUAL(false, s.isEmpty());
    CPPUNIT_ASSERT_EQUAL(string("Eat soup"), s.top());
    s.add("Say hello", 0);
    CPPUNIT_ASSERT_EQUAL(2u, s.getSize());
    CPPUNIT_ASSERT_EQUAL(string("Say hello"), s.top());
    s.add("Play violin", 8);
    s.add("Wash teeth", 20);
    s.add("Feed fishes", -1);
    s.add("Buy potatoes", 23);
    CPPUNIT_ASSERT_EQUAL(6u, s.getSize());

    s.updateScore("Play violin", 100);
    s.updateScore("Say hello", 180);
    s.updateScore("Say hello", -15);

    CPPUNIT_ASSERT_EQUAL(6u, s.getSize());
    CPPUNIT_ASSERT_EQUAL(false, s.isEmpty());
    CPPUNIT_ASSERT_EQUAL(string("Say hello"), s.top());

    vector<pair<string, int> > order;
    order.push_back(make_pair("Say hello", -15));
    order.push_back(make_pair("Feed fishes", -1));
    order.push_back(make_pair("Eat soup", 10));
    order.push_back(make_pair("Wash teeth", 20));
    order.push_back(make_pair("Buy potatoes", 23));
    order.push_back(make_pair("Play violin", 100));

    int i = 0;
    while (!s.isEmpty())
    {
      CPPUNIT_ASSERT_EQUAL(order[i].first, s.top());
      CPPUNIT_ASSERT_EQUAL(order[i].second, s.getScore(s.top()));
      s.pop();
      i++;
    }
  }

  void basicMaxTest()
  {
    ScoredSet<std::string, int> s;
    CPPUNIT_ASSERT_EQUAL(true, s.isEmpty());
    CPPUNIT_ASSERT_EQUAL(0u, s.getSize());
    s.add("Eat soup", 10);
    CPPUNIT_ASSERT_EQUAL(false, s.isEmpty());
    CPPUNIT_ASSERT_EQUAL(string("Eat soup"), s.top());
    s.add("Say hello", 0);
    CPPUNIT_ASSERT_EQUAL(2u, s.getSize());
    CPPUNIT_ASSERT_EQUAL(string("Eat soup"), s.top());
    s.add("Play violin", 8);
    s.add("Wash teeth", 20);
    s.add("Feed fishes", -1);
    s.add("Buy potatoes", 23);
    CPPUNIT_ASSERT_EQUAL(6u, s.getSize());

    s.updateScore("Play violin", 100);
    s.updateScore("Say hello", 180);
    s.updateScore("Say hello", -15);

    CPPUNIT_ASSERT_EQUAL(6u, s.getSize());
    CPPUNIT_ASSERT_EQUAL(false, s.isEmpty());
    CPPUNIT_ASSERT_EQUAL(string("Play violin"), s.top());

    vector<pair<string, int> > order;
    order.push_back(make_pair("Play violin", 100));
    order.push_back(make_pair("Buy potatoes", 23));
    order.push_back(make_pair("Wash teeth", 20));
    order.push_back(make_pair("Eat soup", 10));
    order.push_back(make_pair("Feed fishes", -1));
    order.push_back(make_pair("Say hello", -15));

    int i = 0;
    while (!s.isEmpty())
    {
      CPPUNIT_ASSERT_EQUAL(order[i].first, s.top());
      CPPUNIT_ASSERT_EQUAL(order[i].second, s.getScore(s.top()));
      s.pop();
      i++;
    }
  }

  void advancedTest()
  {
    ScoredSet<std::string, int, MinComparePolicy<int> > s;
    s.add("Eat soup", 10);
    CPPUNIT_ASSERT_EQUAL(string("Eat soup"), s.top());
    s.add("Say hello", 0);
    s.add("Play violin", 8);
    s.add("Wash teeth", 20);
    s.pop();
    s.pop();
    CPPUNIT_ASSERT_EQUAL(string("Eat soup"), s.top());
    s.add("Feed fishes", -1);
    CPPUNIT_ASSERT_EQUAL(string("Feed fishes"), s.top());

    s.add("Buy potatoes", 23);
    s.add("A", 56);
    s.add("B", -44);
    s.add("C", 88);
    s.add("D", 57);

    CPPUNIT_ASSERT_EQUAL(string("B"), s.top());
    s.pop();
    CPPUNIT_ASSERT_EQUAL(string("Feed fishes"), s.top());
    s.updateScore("Feed fishes", 99);
    CPPUNIT_ASSERT_EQUAL(string("Eat soup"), s.top());
    s.updateScore("Eat soup", 1444);
    s.updateScore("Wash teeth", 111);
    CPPUNIT_ASSERT_EQUAL(string("Buy potatoes"), s.top());
    s.add("Say hello", 0);
    s.add("Play violin", 8);
    s.pop();
    CPPUNIT_ASSERT_EQUAL(string("Play violin"), s.top());
  }

  void addDuplicate()
  {
    ScoredSet<std::string, int> s;
    s.add("Alice", 10);
    CPPUNIT_ASSERT_THROW(s.add("Alice", 15), ErrorResponseException);
  }

  void getScoreForNonexistentElem()
  {
    ScoredSet<std::string, int> s;
    s.add("Alice", 10);
    CPPUNIT_ASSERT_THROW(s.getScore("Alison"), ErrorResponseException);
  }

  void updateScoreForNonexistentElem()
  {
    ScoredSet<std::string, int> s;
    s.add("Alice", 10);
    CPPUNIT_ASSERT_THROW(s.updateScore("Alison", 30), ErrorResponseException);
  }

  void topOnEmptySet()
  {
    ScoredSet<std::string, int> s;
    CPPUNIT_ASSERT_THROW(s.top(), // stop!
                         ErrorResponseException);
  }

  void popOnEmptySet()
  {
    ScoredSet<std::string, int> s;
    CPPUNIT_ASSERT_THROW(s.pop(), ErrorResponseException);
  }

  void displayTest()
  {
    ScoredSet<std::string, int, MinComparePolicy<int> > s;
    s.add("Eat soup", 5);
    s.add("Say hello", -7);
    s.add("Play violin", 92);
    s.add("Wash teeth", 358);
    s.add("Feed fishes", 0);
    s.add("Buy potatoes", -113);

    ostringstream out;
    out << s;

    string expected = "Scored Set with 6 elements in order:\n"
                      "Buy potatoes: -113\n"
                      "Say hello: -7\n"
                      "Feed fishes: 0\n"
                      "Eat soup: 5\n"
                      "Play violin: 92\n"
                      "Wash teeth: 358\n";
    CPPUNIT_ASSERT_EQUAL(expected, out.str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ScoredSetTest);

} // namespace tse
