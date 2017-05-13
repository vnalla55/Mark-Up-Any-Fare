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
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "Common/ErrorResponseException.h"

#include "Pricing/Shopping/Swapper/ScoringMediator.h"
#include "Pricing/Shopping/Swapper/PriorityScoreBuilder.h"

#include "Pricing/Shopping/Swapper/test/AllEvenNumbersRepresented.h"
#include "Pricing/Shopping/Swapper/test/NumbersAboveX.h"
#include "Pricing/Shopping/Swapper/test/TakeAllNumbers.h"
#include "Pricing/Shopping/Swapper/test/SmallerNumbersAreBetter.h"

#include <boost/assign/std/vector.hpp>

#include <vector>
#include <iostream>
#include <sstream>

using namespace std;
using boost::assign::operator+=;

namespace tse
{

using namespace swp;

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)
//
class DummyUpdater : public IMapUpdater<int, PositionalScore>
{
public:
  void updateValue(const int& key, const PositionalScore& value)
  {
    // do nothing
  }
};

class SimpleScoreMap : public IMapUpdater<int, PositionalScore>
{
public:
  void addScore(const int& element, const PositionalScore& score)
  {
    CPPUNIT_ASSERT(_scores.find(element) == _scores.end());
    _scores[element] = score;
    _logStream << "add " << element << ": " << score << endl;
  }

  const PositionalScore& getScore(const int& element) const
  {
    CPPUNIT_ASSERT(_scores.find(element) != _scores.end());
    _logStream << "get " << element << ": " << _scores.find(element)->second << endl;
    return _scores.find(element)->second;
  }

  void updateValue(const int& key, const PositionalScore& value)
  {
    CPPUNIT_ASSERT(_scores.find(key) != _scores.end());
    _scores[key] = value;
    _logStream << "upd " << key << ": " << value << endl;
  }

  void discard(const int& element)
  {
    _scores.erase(element);
    _logStream << "del " << element << endl;
  }

  static void formatSimple(std::ostream& out, const SimpleScoreMap& m)
  {
    for (std::map<int, PositionalScore>::const_iterator it = m._scores.begin();
         it != m._scores.end();
         ++it)
    {
      out << it->first << "\trated\t" << it->second << endl;
    }
  }

  static void format(std::ostream& out, const SimpleScoreMap& m)
  {
    typedef map<PositionalScore, vector<int> > ReverseMap;
    ReverseMap reverseMap;
    for (std::map<int, PositionalScore>::const_iterator it = m._scores.begin();
         it != m._scores.end();
         ++it)
    {
      reverseMap[it->second].push_back(it->first);
    }

    for (ReverseMap::const_reverse_iterator it = reverseMap.rbegin(); it != reverseMap.rend(); ++it)
    {
      for (unsigned int i = 0; i < it->second.size(); ++i)
      {
        out << it->second[i] << "\trated\t" << it->first << endl;
      }
    }
  }

  std::string getLog() const { return _logStream.str(); }

  void clearLog() { _logStream.str(""); }

private:
  std::map<int, PositionalScore> _scores;
  mutable ostringstream _logStream;
};

std::ostream& operator<<(std::ostream& out, const SimpleScoreMap& m)
{
  out << utils::format(m, SimpleScoreMap::formatSimple);
  return out;
}

// Appraisers:
// 40, "Three numbers above 100"
// 30, "All even numbers represented"
// 20, "Take all"
// 10, "Smaller numbers are better"
class ScoringMediatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ScoringMediatorTest);
  CPPUNIT_TEST(initialStateTest);
  CPPUNIT_TEST(exceptionsTest);
  CPPUNIT_TEST(oneScoreTest1);
  CPPUNIT_TEST(oneScoreTest2);
  CPPUNIT_TEST(oneScoreTest3);
  CPPUNIT_TEST(oneScoreTest4);
  CPPUNIT_TEST(feedingTest);
  CPPUNIT_TEST(feedAndRemoveTest);
  CPPUNIT_TEST(simultaneousUpdateTest);
  CPPUNIT_TEST(bankCheck);
  CPPUNIT_TEST(setScoreFromAppraiserTest);
  CPPUNIT_TEST_SUITE_END();

public:
  ScoringMediatorTest() {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    int threshold = 100;
    int count = 3;
    _above100 = _memHandle.create<NumbersAboveX>(threshold, count);
    _allEven = _memHandle.create<AllEvenNumbersRepresented>();
    _takeAll = _memHandle.create<TakeAllNumbers>();
    _smallerBetter = _memHandle.create<SmallerNumbersAreBetter>();

    _builder = _memHandle.create<PriorityScoreBuilder>();
    _map = _memHandle.create<SimpleScoreMap>();
    _m = _memHandle.create<IntPositionalMediator>(*_builder, *_map);

    addAppraisers();
  }

  void tearDown() { _memHandle.clear(); }

  void initialStateTest()
  {
    CPPUNIT_ASSERT_EQUAL(false, areAllAppraisersSatisfied());
    const std::vector<const IntPositionalMediator::Appraiser*>& app = _m->getAppraisers();
    CPPUNIT_ASSERT_EQUAL(size_t(4), app.size());

    // Priorities in order of adding (vector)
    CPPUNIT_ASSERT_EQUAL(30, _m->getInfoForAppraiser(app[0]));
    CPPUNIT_ASSERT_EQUAL(40, _m->getInfoForAppraiser(app[1]));
    CPPUNIT_ASSERT_EQUAL(20, _m->getInfoForAppraiser(app[2]));
    CPPUNIT_ASSERT_EQUAL(10, _m->getInfoForAppraiser(app[3]));

    CPPUNIT_ASSERT_EQUAL(0u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(0u, _m->getBankScoresCount());
  }

  void exceptionsTest()
  {
    // zero appraiser
    CPPUNIT_ASSERT_THROW(_m->addAppraiser(0, 100), ErrorResponseException);

    // duplicate appraiser
    CPPUNIT_ASSERT_THROW(_m->addAppraiser(_above100, 1342), ErrorResponseException);

    _m->beforeItemAdded(101);
    _m->beforeItemAdded(102);
    _m->beforeItemAdded(103);

    // zero source
    CPPUNIT_ASSERT_THROW(_m->setScoreFromAppraiser(0, 333, BasicAppraiserScore()),
                         ErrorResponseException);

    // no such appraiser
    TakeAllNumbers an;
    CPPUNIT_ASSERT_THROW(_m->getInfoForAppraiser(&an), ErrorResponseException);

    // no such item: 513
    CPPUNIT_ASSERT_THROW(_m->getScoresForItem(513), ErrorResponseException);
  }

  void oneScoreTest1()
  {
    CPPUNIT_ASSERT(scoreOK(185, "[1 0 1 0][0 0 0 1][0 0 0 -185]"));
    CPPUNIT_ASSERT(assertAppScores(185, "M . M n(-185)"));
    CPPUNIT_ASSERT_EQUAL(1u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(4u, _m->getBankScoresCount());
  }

  void oneScoreTest2()
  {
    CPPUNIT_ASSERT(scoreOK(152, "[1 1 1 0][0 0 0 1][0 0 0 -152]"));
    CPPUNIT_ASSERT(assertAppScores(152, "M M M n(-152)"));
  }

  void oneScoreTest3()
  {
    CPPUNIT_ASSERT(scoreOK(86, "[0 1 1 0][0 0 0 1][0 0 0 -86]"));
    CPPUNIT_ASSERT(assertAppScores(86, ". M M n(-86)"));
  }

  void oneScoreTest4()
  {
    CPPUNIT_ASSERT(scoreOK(93, "[0 0 1 0][0 0 0 1][0 0 0 -93]"));
    CPPUNIT_ASSERT(assertAppScores(93, ". . M n(-93)"));
  }

  void feedingTest()
  {
    CPPUNIT_ASSERT_EQUAL(false, _above100->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, _allEven->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, _takeAll->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, _smallerBetter->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(false, areAllAppraisersSatisfied());

    // Appraisers:
    // 40, "Three numbers above 100"
    // 30, "All even numbers represented"
    // 20, "Take all"
    // 10, "Smaller numbers are better"

    CPPUNIT_ASSERT_EQUAL(string("add 185: [1 0 1 0][0 0 0 1][0 0 0 -185]\n"), newNumberGetLog(185));
    CPPUNIT_ASSERT(assertAppScores(185, "M . M n(-185)"));
    CPPUNIT_ASSERT_EQUAL(false, _above100->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(string("add 152: [1 1 1 0][0 0 0 1][0 0 0 -152]\n"), newNumberGetLog(152));
    CPPUNIT_ASSERT_EQUAL(false, _above100->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(string("add 86: [0 1 1 0][0 0 0 1][0 0 0 -86]\n"), newNumberGetLog(86));
    CPPUNIT_ASSERT_EQUAL(false, _above100->isSatisfied());

    CPPUNIT_ASSERT_EQUAL(3u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(12u, _m->getBankScoresCount());

    CPPUNIT_ASSERT_EQUAL(string("add 130: [1 1 1 0][0 0 0 1][0 0 0 -130]\n"), newNumberGetLog(130));
    CPPUNIT_ASSERT_EQUAL(true, _above100->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(string("add 22: [0 1 1 0][0 0 0 1][0 0 0 -22]\n"), newNumberGetLog(22));
    CPPUNIT_ASSERT_EQUAL(true, _above100->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(string("add 93: [0 0 1 0][0 0 0 1][0 0 0 -93]\n"), newNumberGetLog(93));

    CPPUNIT_ASSERT_EQUAL(6u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(24u, _m->getBankScoresCount());

    CPPUNIT_ASSERT(assertAppScores(185, "M . M n(-185)"));
    // 138 is the forth number above 100
    // "Three numbers above 100" should
    // mark 185, 152, 130 as nice to have
    // and return nice to have for it
    string expected = ""
                      // the appraiser has std set inside so
                      // wil update in ascending manner
                      "upd 130: [0 1 1 0][1 0 0 1][0 0 0 -130]\n"
                      "upd 152: [0 1 1 0][1 0 0 1][0 0 0 -152]\n"
                      "upd 185: [0 0 1 0][1 0 0 1][0 0 0 -185]\n"
                      // finaly, an update following from
                      // appraisers score for 138 is made
                      "add 138: [0 1 1 0][1 0 0 1][0 0 0 -138]\n";
    CPPUNIT_ASSERT_EQUAL(expected, newNumberGetLog(138));
    CPPUNIT_ASSERT(assertAppScores(152, "n M M n(-152)"));
    CPPUNIT_ASSERT(assertAppScores(138, "n M M n(-138)"));
    // now numbers > 100 are only nice to have
    CPPUNIT_ASSERT_EQUAL(string("add 153: [0 0 1 0][1 0 0 1][0 0 0 -153]\n"), newNumberGetLog(153));
    CPPUNIT_ASSERT_EQUAL(string("add 48: [0 1 1 0][0 0 0 1][0 0 0 -48]\n"), newNumberGetLog(48));
    CPPUNIT_ASSERT_EQUAL(string("add 170: [0 1 1 0][1 0 0 1][0 0 0 -170]\n"), newNumberGetLog(170));
    CPPUNIT_ASSERT_EQUAL(string("add 29: [0 0 1 0][0 0 0 1][0 0 0 -29]\n"), newNumberGetLog(29));

    CPPUNIT_ASSERT_EQUAL(true, _above100->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, _allEven->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, _takeAll->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, _smallerBetter->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, areAllAppraisersSatisfied());

    CPPUNIT_ASSERT_EQUAL(11u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(44u, _m->getBankScoresCount());
  }

  void feedAndRemoveTest()
  {
    // Add and remove 157
    CPPUNIT_ASSERT_EQUAL(string("add 157: [1 0 1 0][0 0 0 1][0 0 0 -157]\n"), newNumberGetLog(157));
    CPPUNIT_ASSERT_EQUAL(false, _above100->isSatisfied());

    CPPUNIT_ASSERT_EQUAL(1u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(4u, _m->getBankScoresCount());

    CPPUNIT_ASSERT_EQUAL(string("del 157\n"), discardNumberGetLog(157));
    CPPUNIT_ASSERT_EQUAL(false, _above100->isSatisfied());

    CPPUNIT_ASSERT_EQUAL(0u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(0u, _m->getBankScoresCount());

    CPPUNIT_ASSERT_EQUAL(string("add 13: [0 0 1 0][0 0 0 1][0 0 0 -13]\n"), newNumberGetLog(13));
    CPPUNIT_ASSERT_EQUAL(string("add 188: [1 1 1 0][0 0 0 1][0 0 0 -188]\n"), newNumberGetLog(188));
    CPPUNIT_ASSERT_EQUAL(string("add 101: [1 0 1 0][0 0 0 1][0 0 0 -101]\n"), newNumberGetLog(101));
    // has 188, 101
    CPPUNIT_ASSERT_EQUAL(string("add 5: [0 0 1 0][0 0 0 1][0 0 0 -5]\n"), newNumberGetLog(5));
    CPPUNIT_ASSERT_EQUAL(false, _above100->isSatisfied());

    CPPUNIT_ASSERT_EQUAL(string("add 134: [1 1 1 0][0 0 0 1][0 0 0 -134]\n"), newNumberGetLog(134));
    // has 188, 101, 134
    CPPUNIT_ASSERT_EQUAL(true, _above100->isSatisfied());

    CPPUNIT_ASSERT_EQUAL(5u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(20u, _m->getBankScoresCount());

    CPPUNIT_ASSERT_EQUAL(string("del 101\n"), discardNumberGetLog(101));

    CPPUNIT_ASSERT_EQUAL(4u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(16u, _m->getBankScoresCount());
    // has 188, 134
    CPPUNIT_ASSERT_EQUAL(false, _above100->isSatisfied());

    CPPUNIT_ASSERT_EQUAL(string("add 555: [1 0 1 0][0 0 0 1][0 0 0 -555]\n"), newNumberGetLog(555));
    // has 188, 134, 555
    CPPUNIT_ASSERT_EQUAL(true, _above100->isSatisfied());

    string expected = ""
                      "upd 134: [0 1 1 0][1 0 0 1][0 0 0 -134]\n"
                      "upd 188: [0 1 1 0][1 0 0 1][0 0 0 -188]\n"
                      "upd 555: [0 0 1 0][1 0 0 1][0 0 0 -555]\n"
                      "add 160: [0 1 1 0][1 0 0 1][0 0 0 -160]\n";
    CPPUNIT_ASSERT_EQUAL(expected, newNumberGetLog(160));
    // has 188, 134, 555, 160
    CPPUNIT_ASSERT_EQUAL(true, _above100->isSatisfied());

    CPPUNIT_ASSERT_EQUAL(string("add 170: [0 1 1 0][1 0 0 1][0 0 0 -170]\n"), newNumberGetLog(170));
    // has 188, 134, 555, 160, 170
    CPPUNIT_ASSERT_EQUAL(true, _above100->isSatisfied());

    CPPUNIT_ASSERT_EQUAL(7u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(28u, _m->getBankScoresCount());

    CPPUNIT_ASSERT_EQUAL(string("del 188\n"), discardNumberGetLog(188));
    // has 134, 555, 160, 170
    CPPUNIT_ASSERT_EQUAL(true, _above100->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(6u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(24u, _m->getBankScoresCount());

    // Now we are going to remove 160.
    // _above100 is going to switch from 'nice to have'
    // to 'must have' for all its items
    expected = ""
               "upd 134: [1 1 1 0][0 0 0 1][0 0 0 -134]\n"
               "upd 170: [1 1 1 0][0 0 0 1][0 0 0 -170]\n"
               "upd 555: [1 0 1 0][0 0 0 1][0 0 0 -555]\n"
               "del 160\n";
    CPPUNIT_ASSERT_EQUAL(expected, discardNumberGetLog(160));
    CPPUNIT_ASSERT_EQUAL(true, _above100->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(5u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(20u, _m->getBankScoresCount());

    // Now delete also 134
    CPPUNIT_ASSERT_EQUAL(string("del 134\n"), discardNumberGetLog(134));
    CPPUNIT_ASSERT_EQUAL(false, _above100->isSatisfied());
    CPPUNIT_ASSERT_EQUAL(4u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(16u, _m->getBankScoresCount());
  }

  void simultaneousUpdateTest()
  {
    // Make new mediator with two appraisers
    PriorityScoreBuilder builder;
    _m = _memHandle.create<IntPositionalMediator>(builder, *_map);

    NumbersAboveX above10(10, 2);
    _m->addAppraiser(&above10, 0);

    NumbersAboveX above100(100, 1);
    _m->addAppraiser(&above100, 98);

    CPPUNIT_ASSERT_EQUAL(string("add 5: [0 0][0 0][0 0]\n"), newNumberGetLog(5));
    CPPUNIT_ASSERT_EQUAL(string("add 15: [0 1][0 0][0 0]\n"), newNumberGetLog(15));

    CPPUNIT_ASSERT_EQUAL(2u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(4u, _m->getBankScoresCount());

    CPPUNIT_ASSERT_EQUAL(false, above100.isSatisfied()); // has 0/1
    CPPUNIT_ASSERT_EQUAL(false, above10.isSatisfied()); // has 1/2
    CPPUNIT_ASSERT_EQUAL(false, areAllAppraisersSatisfied());

    CPPUNIT_ASSERT_EQUAL(string("add 105: [1 1][0 0][0 0]\n"), newNumberGetLog(105));
    CPPUNIT_ASSERT_EQUAL(true, above100.isSatisfied()); // has 1/1
    CPPUNIT_ASSERT_EQUAL(true, above10.isSatisfied()); // has 2/2
    CPPUNIT_ASSERT_EQUAL(true, areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(3u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(6u, _m->getBankScoresCount());

    // Now we add 130 and have update from both
    // We check final scores for items
    newNumber(130);
    CPPUNIT_ASSERT_EQUAL(4u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(8u, _m->getBankScoresCount());

    CPPUNIT_ASSERT_EQUAL(makeScore("[0 0][0 1][0 0]"), _map->getScore(15));
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 0][1 1][0 0]"), _map->getScore(105));
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 0][1 1][0 0]"), _map->getScore(130));

    _map->clearLog();

    CPPUNIT_ASSERT_EQUAL(true, above100.isSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, above10.isSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, areAllAppraisersSatisfied());

    // Now remove 15 to impact above10 and make him reappraise
    string expected = ""
                      // above 10
                      "upd 105: [0 1][1 0][0 0]\n"
                      "upd 130: [0 1][1 0][0 0]\n"
                      "del 15\n";
    CPPUNIT_ASSERT_EQUAL(expected, discardNumberGetLog(15));
    CPPUNIT_ASSERT_EQUAL(true, above100.isSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, above10.isSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(3u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(6u, _m->getBankScoresCount());

    // Now remove 105 to make above10 not satisfied
    // Also above100 will reappraise
    expected = ""
               // above 100
               "upd 130: [1 1][0 0][0 0]\n"
               "del 105\n";
    CPPUNIT_ASSERT_EQUAL(expected, discardNumberGetLog(105));
    CPPUNIT_ASSERT_EQUAL(true, above100.isSatisfied());
    CPPUNIT_ASSERT_EQUAL(false, above10.isSatisfied());
    CPPUNIT_ASSERT_EQUAL(false, areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(2u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(4u, _m->getBankScoresCount());
  }

  void bankCheck()
  {
    DummyUpdater* dummy_map = _memHandle.create<DummyUpdater>();
    _m = _memHandle.create<IntPositionalMediator>(*_builder, *dummy_map);
    addAppraisers();

    for (int i = 0; i < 9000; i += 3)
    {
      _m->beforeItemAdded(i);
      _m->beforeItemAdded(i + 1);
      _m->beforeItemAdded(i + 2);
      if (i >= 3)
      {
        _m->beforeItemRemoved(i - 3);
        _m->beforeItemRemoved(i - 2);
        _m->beforeItemRemoved(i - 1);
      }
    }
    CPPUNIT_ASSERT_EQUAL(3u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(12u, _m->getBankScoresCount());
  }

  void setScoreFromAppraiserTest()
  {
    DummyUpdater* dummy_map = _memHandle.create<DummyUpdater>();
    _m = _memHandle.create<IntPositionalMediator>(*_builder, *dummy_map);
    addAppraisers();

    _m->setScoreFromAppraiser(_allEven, 50, BasicAppraiserScore(BasicAppraiserScore::MUST_HAVE));
    CPPUNIT_ASSERT_EQUAL(1u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(1u, _m->getBankScoresCount());

    _m->setScoreFromAppraiser(_allEven, 50, BasicAppraiserScore(BasicAppraiserScore::IGNORE));
    CPPUNIT_ASSERT_EQUAL(1u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(1u, _m->getBankScoresCount());

    _m->setScoreFromAppraiser(
        _takeAll, 80, BasicAppraiserScore(BasicAppraiserScore::NICE_TO_HAVE, 17));
    CPPUNIT_ASSERT_EQUAL(2u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(2u, _m->getBankScoresCount());

    _m->setScoreFromAppraiser(
        _takeAll, 50, BasicAppraiserScore(BasicAppraiserScore::WANT_TO_REMOVE, 18));
    CPPUNIT_ASSERT_EQUAL(2u, _m->getBankItemsCount());
    CPPUNIT_ASSERT_EQUAL(3u, _m->getBankScoresCount());
  }

private:
  bool areAllAppraisersSatisfied() const
  {
    const std::vector<const IntPositionalMediator::Appraiser*>& appraisers = _m->getAppraisers();
    for (unsigned int i = 0; i < appraisers.size(); ++i)
    {
      if (!appraisers[i]->isSatisfied())
      {
        return false;
      }
    }
    return true;
  }

  void printStatus() const
  {
    printHeader();
    cout << *_map << endl;
  }

  void printHeader() const
  {
    cout << "APPRAISERS" << endl;
    cout << endl;
    cout << "Three numbers above 100" << endl;
    cout << "  All even numbers represented" << endl;
    cout << "    Take all" << endl;
    cout << "      Smaller numbers are better" << endl;
  }

  void addAppraisers()
  {
    // Since there are no appraisers
    CPPUNIT_ASSERT_EQUAL(true, areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _m->getAppraisers().size());

    _m->addAppraiser(_allEven, 30);
    _m->addAppraiser(_above100, 40);
    _m->addAppraiser(_takeAll, 20);
    _m->addAppraiser(_smallerBetter, 10);
  }

  PositionalScore makeScore(const std::string& s) const
  {
    return PositionalScoreFormatter::scoreFromString(s);
  }

  bool scoreOK(int n, const std::string& repr) const
  {
    const PositionalScore p = _m->beforeItemAdded(n);
    const bool ok = (makeScore(repr) == p);
    if (!ok)
    {
      std::cout << "Unexpected item score: " << p << std::endl;
    }
    return ok;
  }

  std::string appScoresToStr(int n) const
  {
    const IntPositionalMediator::AppraiserScoresMap& m = _m->getScoresForItem(n);
    // serialize
    // 40, "Three numbers above 100"
    // 30, "All even numbers represented"
    // 20, "Take all"
    // 10, "Smaller numbers are better"
    std::ostringstream out;
    CPPUNIT_ASSERT(m.find(_above100) != m.end());
    CPPUNIT_ASSERT(m.find(_allEven) != m.end());
    CPPUNIT_ASSERT(m.find(_takeAll) != m.end());
    CPPUNIT_ASSERT(m.find(_smallerBetter) != m.end());

    out << m.find(_above100)->second << " ";
    out << m.find(_allEven)->second << " ";
    out << m.find(_takeAll)->second << " ";
    out << m.find(_smallerBetter)->second;
    return out.str();
  }

  bool assertAppScores(int n, const std::string& app_scores_repr)
  {
    const std::string s = appScoresToStr(n);
    const bool ok = (s == app_scores_repr);
    if (!ok)
    {
      std::cout << "Unexpected appraiser scores (" << s << ")" << std::endl;
    }
    return ok;
  }

  void newNumber(int n)
  {
    const PositionalScore score = _m->beforeItemAdded(n);
    _map->addScore(n, score);
  }

  void discardNumber(int n)
  {
    _m->beforeItemRemoved(n);
    _map->discard(n);
  }

  string newNumberGetLog(int n)
  {
    newNumber(n);
    string mapLog = _map->getLog();
    _map->clearLog();
    return mapLog;
  }

  string discardNumberGetLog(int n)
  {
    discardNumber(n);
    string mapLog = _map->getLog();
    _map->clearLog();
    return mapLog;
  }

  typedef ScoringMediator<int, PriorityScoreBuilder> IntPositionalMediator;
  TestMemHandle _memHandle;

  // Appraisers
  NumbersAboveX* _above100;
  AllEvenNumbersRepresented* _allEven;
  TakeAllNumbers* _takeAll;
  SmallerNumbersAreBetter* _smallerBetter;

  PriorityScoreBuilder* _builder;
  SimpleScoreMap* _map;
  IntPositionalMediator* _m;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ScoringMediatorTest);

} // namespace tse
