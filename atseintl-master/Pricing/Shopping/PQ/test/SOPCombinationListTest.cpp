#include "test/include/CppUnitHelperMacros.h"
#include <ostream>

#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/SOPCombinationList.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
namespace shpq
{

std::ostream& operator<<(std::ostream& out, const SOPCombination& combination)
{
  out << "(" << combination.oSopVec[0] << "," << combination.oSopVec[1] << ")";
  return out;
}

class SOPCombinationListTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SOPCombinationListTest);
  CPPUNIT_TEST(testCreateSOPCombinationList1);
  CPPUNIT_TEST(testCreateSOPCombinationList2);
  CPPUNIT_TEST(testSOPCombinationAdd1);
  CPPUNIT_TEST(testSOPCombinationAdd2);
  CPPUNIT_TEST(testSOPCombinationErase1);
  CPPUNIT_TEST(testSOPCombinationErase2);
  CPPUNIT_TEST(testSOPCombinationRemoveCombinations1);
  CPPUNIT_TEST(testSOPCombinationRemoveCombinations2);
  CPPUNIT_TEST(testSOPCombinationCache1);
  CPPUNIT_TEST(testSOPCombinationCache2);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

private:
  void initTrx(ShoppingTrx& trx, unsigned sop1Size, unsigned sop2Size = 0)
  {
    trx.legs().push_back(ShoppingTrx::Leg());
    trx.legs()[0].sop().reserve(sop1Size);
    for (unsigned sopId = 0; sopId < sop1Size; ++sopId)
    {
      trx.legs()[0].sop().push_back(ShoppingTrx::SchedulingOption(0, sopId));
    }

    if (sop2Size > 0)
    {
      trx.legs().push_back(ShoppingTrx::Leg());
      trx.legs()[1].sop().reserve(sop2Size);
      for (unsigned sopId = 0; sopId < sop2Size; ++sopId)
      {
        trx.legs()[1].sop().push_back(ShoppingTrx::SchedulingOption(0, sopId));
      }
    }
  }

  SOPCombination makeCombination(int sop1, int sop2 = -1)
  {
    SOPCombination combination;
    combination.oSopVec[0] = sop1;
    if (sop2 >= 0)
    {
      combination.oSopVec[1] = sop2;
    }

    return combination;
  }

  void testCreateSOPCombinationList1()
  {
    ShoppingTrx trx;
    initTrx(trx, 10);
    SOPCombinationList combinations(trx);

    CPPUNIT_ASSERT_EQUAL(true, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), combinations.size());
    CPPUNIT_ASSERT(combinations.begin() == combinations.end());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), combinations._combinationsForSop.size());
    CPPUNIT_ASSERT_EQUAL(std::size_t(10), combinations._combinationsForSop[0].size());
  }

  void testCreateSOPCombinationList2()
  {
    ShoppingTrx trx;
    initTrx(trx, 10, 11);
    SOPCombinationList combinations(trx);

    CPPUNIT_ASSERT_EQUAL(true, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), combinations.size());
    CPPUNIT_ASSERT(combinations.begin() == combinations.end());
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), combinations._combinationsForSop.size());
    CPPUNIT_ASSERT_EQUAL(std::size_t(10), combinations._combinationsForSop[0].size());
    CPPUNIT_ASSERT_EQUAL(std::size_t(11), combinations._combinationsForSop[1].size());
  }

  void testSOPCombinationAdd1()
  {
    ShoppingTrx trx;
    initTrx(trx, 10);
    SOPCombinationList combinations(trx);

    combinations.push_back(makeCombination(1));
    combinations.push_back(makeCombination(9));

    CPPUNIT_ASSERT_EQUAL(false, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), combinations.size());

    SOPCombinationList::const_iterator i = combinations.begin();
    CPPUNIT_ASSERT_EQUAL(makeCombination(1), *i);
    CPPUNIT_ASSERT_EQUAL(true, i->_isLinked);
    ++i;
    CPPUNIT_ASSERT_EQUAL(makeCombination(9), *i);
    CPPUNIT_ASSERT_EQUAL(true, i->_isLinked);

    CPPUNIT_ASSERT_EQUAL(makeCombination(1), *combinations._combinationsForSop[0][1]);
    CPPUNIT_ASSERT_EQUAL(makeCombination(9), *combinations._combinationsForSop[0][9]);
  }

  void testSOPCombinationAdd2()
  {
    ShoppingTrx trx;
    initTrx(trx, 10, 7);
    SOPCombinationList combinations(trx);

    combinations.push_back(makeCombination(1, 3));
    combinations.push_back(makeCombination(9, 1));
    combinations.push_back(makeCombination(5, 2));
    combinations.push_back(makeCombination(0, 2));
    combinations.push_back(makeCombination(0, 1));

    CPPUNIT_ASSERT_EQUAL(false, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(5), combinations.size());

    SOPCombinationList::const_iterator i = combinations.begin();
    CPPUNIT_ASSERT_EQUAL(makeCombination(1, 3), *i);
    CPPUNIT_ASSERT_EQUAL(true, i->_isLinked);
    ++i;
    CPPUNIT_ASSERT_EQUAL(makeCombination(9, 1), *i);
    CPPUNIT_ASSERT_EQUAL(true, i->_isLinked);
    ++i;
    CPPUNIT_ASSERT_EQUAL(makeCombination(5, 2), *i);
    CPPUNIT_ASSERT_EQUAL(true, i->_isLinked);
    ++i;
    CPPUNIT_ASSERT_EQUAL(makeCombination(0, 2), *i);
    CPPUNIT_ASSERT_EQUAL(true, i->_isLinked);
    ++i;
    CPPUNIT_ASSERT_EQUAL(makeCombination(0, 1), *i);
    CPPUNIT_ASSERT_EQUAL(true, i->_isLinked);

    CPPUNIT_ASSERT_EQUAL(makeCombination(1, 3), *combinations._combinationsForSop[0][1]);
    CPPUNIT_ASSERT_EQUAL(makeCombination(1, 3), *combinations._combinationsForSop[1][3]);
    CPPUNIT_ASSERT_EQUAL(static_cast<SOPCombination*>(0),
                         combinations._combinationsForSop[0][1]->_next[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<SOPCombination*>(0),
                         combinations._combinationsForSop[1][3]->_next[0]);

    CPPUNIT_ASSERT_EQUAL(makeCombination(1, 3), *combinations._combinationsForSop[0][1]);
    CPPUNIT_ASSERT_EQUAL(makeCombination(1, 3), *combinations._combinationsForSop[1][3]);

    CPPUNIT_ASSERT_EQUAL(makeCombination(9, 1), *combinations._combinationsForSop[0][9]);
    CPPUNIT_ASSERT_EQUAL(makeCombination(9, 1), *combinations._combinationsForSop[1][1]->_next[1]);

    CPPUNIT_ASSERT_EQUAL(makeCombination(5, 2), *combinations._combinationsForSop[0][5]);
    CPPUNIT_ASSERT_EQUAL(makeCombination(5, 2), *combinations._combinationsForSop[1][2]->_next[1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<SOPCombination*>(0),
                         combinations._combinationsForSop[0][5]->_next[0]);

    CPPUNIT_ASSERT_EQUAL(makeCombination(0, 2), *combinations._combinationsForSop[0][0]->_next[0]);
    CPPUNIT_ASSERT_EQUAL(makeCombination(0, 2), *combinations._combinationsForSop[1][2]);

    CPPUNIT_ASSERT_EQUAL(makeCombination(0, 1), *combinations._combinationsForSop[0][0]);
    CPPUNIT_ASSERT_EQUAL(makeCombination(0, 1), *combinations._combinationsForSop[1][1]);
  }

  void testSOPCombinationErase1()
  {
    ShoppingTrx trx;
    initTrx(trx, 10);
    SOPCombinationList combinations(trx);

    combinations.push_back(makeCombination(1));
    combinations.push_back(makeCombination(9));

    CPPUNIT_ASSERT_EQUAL(false, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), combinations.size());

    SOPCombinationList::iterator i = combinations.begin();
    ++i;
    combinations.erase(i);
    CPPUNIT_ASSERT_EQUAL(false, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), combinations.size());
    CPPUNIT_ASSERT_EQUAL(false, combinations._combinationsForSop[0][9]->_isLinked);

    i = combinations.begin();
    combinations.erase(i);
    CPPUNIT_ASSERT_EQUAL(true, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), combinations.size());
    CPPUNIT_ASSERT_EQUAL(false, combinations._combinationsForSop[0][1]->_isLinked);
  }

  void testSOPCombinationErase2()
  {
    ShoppingTrx trx;
    initTrx(trx, 10, 5);
    SOPCombinationList combinations(trx);

    combinations.push_back(makeCombination(1, 0));
    combinations.push_back(makeCombination(9, 1));

    CPPUNIT_ASSERT_EQUAL(false, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), combinations.size());

    SOPCombinationList::iterator i = combinations.begin();
    i = combinations.erase(i);
    CPPUNIT_ASSERT_EQUAL(false, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), combinations.size());
    CPPUNIT_ASSERT_EQUAL(false, combinations._combinationsForSop[0][1]->_isLinked);
    CPPUNIT_ASSERT_EQUAL(false, combinations._combinationsForSop[1][0]->_isLinked);

    combinations.erase(i);
    CPPUNIT_ASSERT_EQUAL(true, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), combinations.size());
    CPPUNIT_ASSERT_EQUAL(false, combinations._combinationsForSop[0][9]->_isLinked);
    CPPUNIT_ASSERT_EQUAL(false, combinations._combinationsForSop[1][1]->_isLinked);
  }

  void testSOPCombinationRemoveCombinations1()
  {
    ShoppingTrx trx;
    initTrx(trx, 10);
    SOPCombinationList combinations(trx);

    combinations.push_back(makeCombination(1));
    combinations.push_back(makeCombination(9));

    combinations.removeCombinations(0, 1);
    CPPUNIT_ASSERT_EQUAL(false, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), combinations.size());
    CPPUNIT_ASSERT_EQUAL(makeCombination(9), *combinations.begin());

    combinations.removeCombinations(0, 9);
    CPPUNIT_ASSERT_EQUAL(true, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), combinations.size());

    combinations.removeCombinations(0, 0);
    CPPUNIT_ASSERT_EQUAL(true, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), combinations.size());
  }

  void testSOPCombinationRemoveCombinations2()
  {
    ShoppingTrx trx;
    initTrx(trx, 10, 15);
    SOPCombinationList combinations(trx);

    combinations.push_back(makeCombination(1, 0));
    combinations.push_back(makeCombination(9, 0));
    combinations.push_back(makeCombination(9, 4));
    combinations.push_back(makeCombination(9, 1));
    combinations.push_back(makeCombination(5, 0));
    combinations.push_back(makeCombination(1, 0));
    combinations.push_back(makeCombination(1, 2));
    combinations.push_back(makeCombination(1, 6));
    combinations.push_back(makeCombination(1, 14));

    combinations.removeCombinations(0, 1);
    CPPUNIT_ASSERT_EQUAL(false, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), combinations.size());

    SOPCombinationList::const_iterator i = combinations.begin();
    CPPUNIT_ASSERT_EQUAL(makeCombination(9, 0), *i);
    ++i;
    CPPUNIT_ASSERT_EQUAL(makeCombination(9, 4), *i);
    ++i;
    CPPUNIT_ASSERT_EQUAL(makeCombination(9, 1), *i);
    ++i;
    CPPUNIT_ASSERT_EQUAL(makeCombination(5, 0), *i);

    combinations.removeCombinations(1, 0);
    CPPUNIT_ASSERT_EQUAL(false, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), combinations.size());
    i = combinations.begin();
    CPPUNIT_ASSERT_EQUAL(makeCombination(9, 4), *i);
    ++i;
    CPPUNIT_ASSERT_EQUAL(makeCombination(9, 1), *i);

    combinations.removeCombinations(0, 9);
    CPPUNIT_ASSERT_EQUAL(true, combinations.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), combinations.size());
  }

  void testSOPCombinationCache1()
  {
    ShoppingTrx trx;
    initTrx(trx, 10, 15);
    SOPCombinationList combinations(trx);

    combinations.push_back(makeCombination(1, 0));
    combinations.push_back(makeCombination(9, 0));
    combinations.push_back(makeCombination(2, 4));

    SOPCombinationList::iterator i = combinations.begin();
    combinations.addCombinationToCache(*i);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), combinations._combinationsCache.size());

    ++i;
    ++i;

    combinations.addCombinationToCache(*i);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), combinations._combinationsCache.size());

    CPPUNIT_ASSERT_EQUAL(makeCombination(1, 0), *combinations.takeCombinationFromCache());
    CPPUNIT_ASSERT_EQUAL(makeCombination(2, 4), *combinations.takeCombinationFromCache());
    CPPUNIT_ASSERT(combinations.takeCombinationFromCache() == combinations.end());
  }

  void testSOPCombinationCache2()
  {
    ShoppingTrx trx;
    initTrx(trx, 10, 15);
    SOPCombinationList combinations(trx);

    combinations.push_back(makeCombination(1, 0));
    combinations.push_back(makeCombination(9, 0));
    combinations.push_back(makeCombination(2, 4));
    combinations.push_back(makeCombination(2, 5));

    SOPCombinationList::iterator i = combinations.begin();
    combinations.addCombinationToCache(*i);
    ++i;
    combinations.addCombinationToCache(*i);
    ++i;
    combinations.addCombinationToCache(*i);

    CPPUNIT_ASSERT_EQUAL(std::size_t(3), combinations._combinationsCache.size());

    i = combinations.begin();
    ++i;
    combinations.erase(i);

    CPPUNIT_ASSERT_EQUAL(makeCombination(1, 0), *combinations.takeCombinationFromCache());
    CPPUNIT_ASSERT_EQUAL(makeCombination(2, 4), *combinations.takeCombinationFromCache());
    CPPUNIT_ASSERT(combinations.takeCombinationFromCache() == combinations.end());
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SOPCombinationListTest);

} // namespace shpq
} // namespace tse
