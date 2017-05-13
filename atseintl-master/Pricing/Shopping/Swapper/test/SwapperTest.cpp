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

#include "Pricing/Shopping/Swapper/Swapper.h"
#include "Pricing/Shopping/Swapper/PrioritySwapperFormatter.h"

#include "Pricing/Shopping/Swapper/test/AllEvenNumbersRepresented.h"
#include "Pricing/Shopping/Swapper/test/NumbersAboveX.h"
#include "Pricing/Shopping/Swapper/test/TakeAllNumbers.h"
#include "Pricing/Shopping/Swapper/test/SmallerNumbersAreBetter.h"

#include <boost/assign/std/vector.hpp>

#include <vector>
#include <iostream>
#include <sstream>

using namespace std;

namespace tse
{

using namespace swp;

using namespace boost::assign; // bring 'operator+=()' into scope

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class SwapperTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SwapperTest);
  CPPUNIT_TEST(initialStateTest);
  CPPUNIT_TEST(addItemNoAppraisersTest);
  CPPUNIT_TEST(itemDuplicateErrorTest);
  CPPUNIT_TEST(emptyItemAddErrorTest);
  CPPUNIT_TEST(addingAppraisersTest);
  CPPUNIT_TEST(getInfoForAppraiserTest);
  CPPUNIT_TEST(getDerivedInfoForAppraiserTest);
  CPPUNIT_TEST(getAppraiserScoresForItemTest);
  CPPUNIT_TEST(noSuchItemErrorTest);
  CPPUNIT_TEST(noSuchAppraiserErrorTest);
  CPPUNIT_TEST(appraisersIterationTest);
  CPPUNIT_TEST(simpleAddTest);
  CPPUNIT_TEST(resultIterationTest);
  CPPUNIT_TEST(simpleAddTest2);
  CPPUNIT_TEST(swappingTest1);
  CPPUNIT_TEST(swappingTest2);
  CPPUNIT_TEST(duplicateItemTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _s = _memHandle.create<Swp>(DEFAULT_CAPACITY);
  }

  void tearDown() { _memHandle.clear(); }

  void initialStateTest()
  {
    CPPUNIT_ASSERT_EQUAL(true, _s->areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(false, _s->isFull());
    CPPUNIT_ASSERT_EQUAL(false, _s->isDone());
    CPPUNIT_ASSERT_EQUAL(0u, _s->getTotalIterationsCount());
    CPPUNIT_ASSERT_EQUAL(0u, _s->getNoProgressIterationsCount());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _s->getAppraisers().size());
    CPPUNIT_ASSERT(_s->begin() == _s->end());
    CPPUNIT_ASSERT_EQUAL(0u, _s->getSize());
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CAPACITY, _s->getCapacity());
  }

  void addItemNoAppraisersTest() { CPPUNIT_ASSERT_THROW(_s->addItem(19), ErrorResponseException); }

  void itemDuplicateErrorTest()
  {
    addAppraisers();
    _s->addItem(9);
    CPPUNIT_ASSERT_THROW(_s->addItem(9), ErrorResponseException);
  }

  void emptyItemAddErrorTest()
  {
    addAppraisers();
    CPPUNIT_ASSERT_THROW(_s->addItem(0), ErrorResponseException);
  }

  void addingAppraisersTest()
  {
    addAppraisers();
    CPPUNIT_ASSERT_EQUAL(false, _s->areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(false, _s->isFull());
    CPPUNIT_ASSERT_EQUAL(false, _s->isDone());
    CPPUNIT_ASSERT_EQUAL(0u, _s->getTotalIterationsCount());
    CPPUNIT_ASSERT_EQUAL(0u, _s->getNoProgressIterationsCount());
    CPPUNIT_ASSERT_EQUAL(size_t(4), _s->getAppraisers().size());
    CPPUNIT_ASSERT(_s->begin() == _s->end());
    CPPUNIT_ASSERT_EQUAL(0u, _s->getSize());
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CAPACITY, _s->getCapacity());
  }

  void getInfoForAppraiserTest()
  {
    addAppraisers();
    CPPUNIT_ASSERT_EQUAL(20, _s->getInfoForAppraiser(_takeAll));
    CPPUNIT_ASSERT_EQUAL(10, _s->getInfoForAppraiser(_smallerBetter));
    CPPUNIT_ASSERT_EQUAL(30, _s->getInfoForAppraiser(_allEven));
    CPPUNIT_ASSERT_EQUAL(40, _s->getInfoForAppraiser(_above100));
  }

  void getDerivedInfoForAppraiserTest()
  {
    // checking ranks
    addAppraisers();
    CPPUNIT_ASSERT_EQUAL(2u, _s->getDerivedInfoForAppraiser(_takeAll));
    CPPUNIT_ASSERT_EQUAL(3u, _s->getDerivedInfoForAppraiser(_smallerBetter));
    CPPUNIT_ASSERT_EQUAL(1u, _s->getDerivedInfoForAppraiser(_allEven));
    CPPUNIT_ASSERT_EQUAL(0u, _s->getDerivedInfoForAppraiser(_above100));
  }

  void getAppraiserScoresForItemTest()
  {
    addAppraisers();
    _s->addItem(5);
    const Swp::AppraiserScoresMap& m = _s->getAppraiserScoresForItem(5);
    CPPUNIT_ASSERT_EQUAL(size_t(4), m.size());
    CPPUNIT_ASSERT(m.find(_above100) != m.end());
    CPPUNIT_ASSERT(m.find(_allEven) != m.end());
    CPPUNIT_ASSERT(m.find(_smallerBetter) != m.end());
    CPPUNIT_ASSERT(m.find(_takeAll) != m.end());

    CPPUNIT_ASSERT_EQUAL(std::string("n(-5)"), scoreToStr(m.find(_smallerBetter)->second));
    CPPUNIT_ASSERT_EQUAL(std::string("."), scoreToStr(m.find(_allEven)->second));
    CPPUNIT_ASSERT_EQUAL(std::string("M"), scoreToStr(m.find(_takeAll)->second));
    CPPUNIT_ASSERT_EQUAL(std::string("."), scoreToStr(m.find(_above100)->second));
  }

  void noSuchItemErrorTest()
  {
    addAppraisers();
    _s->addItem(5);
    CPPUNIT_ASSERT_THROW(_s->getAppraiserScoresForItem(19), ErrorResponseException);
  }

  void noSuchAppraiserErrorTest()
  {
    addAppraisers();

    CPPUNIT_ASSERT_THROW(_s->getInfoForAppraiser(0), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(_s->getDerivedInfoForAppraiser(0), ErrorResponseException);

    AllEvenNumbersRepresented aenr;

    CPPUNIT_ASSERT_THROW(_s->getInfoForAppraiser(&aenr), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(_s->getDerivedInfoForAppraiser(&aenr), ErrorResponseException);
  }

  void appraisersIterationTest()
  {
    addAppraisers();
    const std::vector<const Swp::Appraiser*>& app = _s->getAppraisers();
    CPPUNIT_ASSERT_EQUAL(size_t(4), app.size());
    CPPUNIT_ASSERT_EQUAL(string("All even numbers represented"), app[0]->toString());
    CPPUNIT_ASSERT_EQUAL(string("Take all numbers"), app[1]->toString());
    CPPUNIT_ASSERT_EQUAL(string("3 numbers above 100 (0 collected)"), app[2]->toString());
    CPPUNIT_ASSERT_EQUAL(string("Smaller numbers are better"), app[3]->toString());
  }

  void simpleAddTest()
  {
    addAppraisers();

    const Swp::AddResponse res = _s->addItem(17);
    CPPUNIT_ASSERT_EQUAL(false, _s->areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(false, _s->isFull());
    CPPUNIT_ASSERT_EQUAL(false, _s->isDone());
    CPPUNIT_ASSERT_EQUAL(1u, _s->getTotalIterationsCount());
    CPPUNIT_ASSERT_EQUAL(0u, _s->getNoProgressIterationsCount());
    CPPUNIT_ASSERT(_s->begin() != _s->end());
    CPPUNIT_ASSERT_EQUAL(1u, _s->getSize());
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CAPACITY, _s->getCapacity());
    CPPUNIT_ASSERT_EQUAL(0, res.first);
    CPPUNIT_ASSERT(predictScores("", res.second));
  }

  void resultIterationTest()
  {
    addAppraisers();
    _s->addItem(83);
    _s->addItem(188);
    _s->addItem(64);
    _s->addItem(24);
    _s->addItem(176);

    vector<int> ratingOrder;
    ratingOrder += 83, 64, 24, 188, 176;

    unsigned int i = 0;
    for (Swp::Iterator it = _s->begin(); it != _s->end(); ++it)
    {
      CPPUNIT_ASSERT_EQUAL(ratingOrder[i], it->key);
      ++i;
    }
  }

  void testReturnsCorrectItemSet()
  {
    addAppraisers();
    _s->addItem(DUMMY_ITEM_1);
    _s->addItem(DUMMY_ITEM_2);
    _s->addItem(DUMMY_ITEM_3);
    const Swp::ItemSet items = _s->getItems();
    CPPUNIT_ASSERT_EQUAL(size_t(3), items.size());
    CPPUNIT_ASSERT(items.find(DUMMY_ITEM_1) != items.end());
    CPPUNIT_ASSERT(items.find(DUMMY_ITEM_2) != items.end());
    CPPUNIT_ASSERT(items.find(DUMMY_ITEM_3) != items.end());
  }

  void simpleAddTest2()
  {
    addAppraisers();

    Swp::AddResponse res = _s->addItem(17);
    CPPUNIT_ASSERT_EQUAL(false, _s->areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(false, _s->isFull());
    CPPUNIT_ASSERT_EQUAL(false, _s->isDone());
    CPPUNIT_ASSERT_EQUAL(1u, _s->getTotalIterationsCount());
    CPPUNIT_ASSERT_EQUAL(0u, _s->getNoProgressIterationsCount());
    CPPUNIT_ASSERT(_s->begin() != _s->end());
    CPPUNIT_ASSERT_EQUAL(1u, _s->getSize());
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CAPACITY, _s->getCapacity());
    CPPUNIT_ASSERT_EQUAL(0, res.first);
    CPPUNIT_ASSERT(predictScores("", res.second));

    res = _s->addItem(153);
    CPPUNIT_ASSERT_EQUAL(false, _s->areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(false, _s->isFull());
    CPPUNIT_ASSERT_EQUAL(false, _s->isDone());
    CPPUNIT_ASSERT_EQUAL(2u, _s->getTotalIterationsCount());
    CPPUNIT_ASSERT_EQUAL(0u, _s->getNoProgressIterationsCount());
    CPPUNIT_ASSERT_EQUAL(2u, _s->getSize());
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CAPACITY, _s->getCapacity());
    CPPUNIT_ASSERT_EQUAL(0, res.first);
    CPPUNIT_ASSERT(predictScores("", res.second));

    res = _s->addItem(122);
    CPPUNIT_ASSERT_EQUAL(false, _s->areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(false, _s->isFull());
    CPPUNIT_ASSERT_EQUAL(false, _s->isDone());
    CPPUNIT_ASSERT_EQUAL(3u, _s->getTotalIterationsCount());
    CPPUNIT_ASSERT_EQUAL(0u, _s->getNoProgressIterationsCount());
    CPPUNIT_ASSERT_EQUAL(3u, _s->getSize());
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CAPACITY, _s->getCapacity());
    CPPUNIT_ASSERT_EQUAL(0, res.first);
    CPPUNIT_ASSERT(predictScores("", res.second));

    res = _s->addItem(144);
    CPPUNIT_ASSERT_EQUAL(true, _s->areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(false, _s->isFull());
    CPPUNIT_ASSERT_EQUAL(false, _s->isDone());
    CPPUNIT_ASSERT_EQUAL(4u, _s->getTotalIterationsCount());
    CPPUNIT_ASSERT_EQUAL(0u, _s->getNoProgressIterationsCount());
    CPPUNIT_ASSERT_EQUAL(4u, _s->getSize());
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CAPACITY, _s->getCapacity());
    CPPUNIT_ASSERT_EQUAL(0, res.first);
    CPPUNIT_ASSERT(predictScores("", res.second));

    res = _s->addItem(182);
    CPPUNIT_ASSERT_EQUAL(true, _s->areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, _s->isFull());
    CPPUNIT_ASSERT_EQUAL(true, _s->isDone());
    CPPUNIT_ASSERT_EQUAL(5u, _s->getTotalIterationsCount());
    CPPUNIT_ASSERT_EQUAL(0u, _s->getNoProgressIterationsCount());
    CPPUNIT_ASSERT_EQUAL(5u, _s->getSize());
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CAPACITY, _s->getCapacity());
    CPPUNIT_ASSERT_EQUAL(0, res.first);
    CPPUNIT_ASSERT(predictScores("", res.second));
    CPPUNIT_ASSERT(predictBottomItem(". . M n(-17)"));

    res = _s->addItem(11);
    CPPUNIT_ASSERT_EQUAL(true, _s->areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, _s->isFull());
    CPPUNIT_ASSERT_EQUAL(true, _s->isDone());
    CPPUNIT_ASSERT_EQUAL(6u, _s->getTotalIterationsCount());
    CPPUNIT_ASSERT_EQUAL(0u, _s->getNoProgressIterationsCount());
    CPPUNIT_ASSERT_EQUAL(5u, _s->getSize());
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CAPACITY, _s->getCapacity());
    CPPUNIT_ASSERT_EQUAL(17, res.first);
    CPPUNIT_ASSERT(predictScores(". . M n(-17)", res.second));
    CPPUNIT_ASSERT(predictBottomItem(". . M n(-11)"));

    res = _s->addItem(150);
    CPPUNIT_ASSERT_EQUAL(true, _s->areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, _s->isFull());
    CPPUNIT_ASSERT_EQUAL(true, _s->isDone());
    CPPUNIT_ASSERT_EQUAL(7u, _s->getTotalIterationsCount());
    CPPUNIT_ASSERT_EQUAL(0u, _s->getNoProgressIterationsCount());
    CPPUNIT_ASSERT_EQUAL(5u, _s->getSize());
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CAPACITY, _s->getCapacity());
    CPPUNIT_ASSERT_EQUAL(11, res.first);
    CPPUNIT_ASSERT(predictScores(". . M n(-11)", res.second));
    CPPUNIT_ASSERT(predictBottomItem("n . M n(-153)"));

    res = _s->addItem(57);
    CPPUNIT_ASSERT_EQUAL(true, _s->areAllAppraisersSatisfied());
    CPPUNIT_ASSERT_EQUAL(true, _s->isFull());
    CPPUNIT_ASSERT_EQUAL(true, _s->isDone());
    CPPUNIT_ASSERT_EQUAL(8u, _s->getTotalIterationsCount());
    CPPUNIT_ASSERT_EQUAL(1u, _s->getNoProgressIterationsCount());
    CPPUNIT_ASSERT_EQUAL(5u, _s->getSize());
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CAPACITY, _s->getCapacity());
    CPPUNIT_ASSERT_EQUAL(57, res.first);
    CPPUNIT_ASSERT(predictScores(". . M n(-57)", res.second));
    CPPUNIT_ASSERT(predictBottomItem("n . M n(-153)"));
  }

  void swappingTest1()
  {
    addAppraisers();
    CPPUNIT_ASSERT(checkAddingStatus(13, ADDED, 0));
    CPPUNIT_ASSERT(checkAddingStatus(200, ADDED, 0));
    CPPUNIT_ASSERT(checkAddingStatus(186, ADDED, 0));
    CPPUNIT_ASSERT(checkAddingStatus(14, ADDED, 0));
    CPPUNIT_ASSERT(checkAddingStatus(92, ADDED, 0));
    // Swapper full

    CPPUNIT_ASSERT(checkAddingStatus(162, SWAPPED, 0, 13));
    CPPUNIT_ASSERT(checkAddingStatus(44, SWAPPED, 0, 92));

    // Wont add since we have already three numbers > 100
    CPPUNIT_ASSERT(checkAddingStatus(149, NOT_ADDED, 1));
    CPPUNIT_ASSERT(checkAddingStatus(165, NOT_ADDED, 2));

    // 166 is even so also the second appraiser will like it
    // 44 is not 'nice to have' by the first appraiser
    // so will be swapped out
    CPPUNIT_ASSERT(checkAddingStatus(166, SWAPPED, 0, 44));
    CPPUNIT_ASSERT(checkAddingStatus(93, NOT_ADDED, 1));
    CPPUNIT_ASSERT(checkAddingStatus(92, NOT_ADDED, 2));
    CPPUNIT_ASSERT(checkAddingStatus(83, NOT_ADDED, 3));

    CPPUNIT_ASSERT(checkAddingStatus(194, SWAPPED, 0, 14));
    CPPUNIT_ASSERT(checkAddingStatus(128, SWAPPED, 0, 200));
    CPPUNIT_ASSERT(checkAddingStatus(102, SWAPPED, 0, 194));

    // 100 is not greater than 100 so no progress
    CPPUNIT_ASSERT(checkAddingStatus(100, NOT_ADDED, 1));
  }

  void swappingTest2()
  {
    addAppraisers();
    CPPUNIT_ASSERT(checkAddingStatus(29, ADDED, 0));
    CPPUNIT_ASSERT(checkAddingStatus(87, ADDED, 0));
    CPPUNIT_ASSERT(checkAddingStatus(13, ADDED, 0));
    CPPUNIT_ASSERT(checkAddingStatus(127, ADDED, 0));
    CPPUNIT_ASSERT(checkAddingStatus(99, ADDED, 0));
    // Swapper full

    CPPUNIT_ASSERT(checkAddingStatus(173, SWAPPED, 0, 99));
    CPPUNIT_ASSERT(checkAddingStatus(58, SWAPPED, 0, 87));
    CPPUNIT_ASSERT(checkAddingStatus(164, SWAPPED, 0, 29));
    CPPUNIT_ASSERT(checkAddingStatus(190, SWAPPED, 0, 13));
    CPPUNIT_ASSERT(checkAddingStatus(72, SWAPPED, 0, 173));
    CPPUNIT_ASSERT(checkAddingStatus(82, NOT_ADDED, 1));
    CPPUNIT_ASSERT(checkAddingStatus(153, NOT_ADDED, 2));
    CPPUNIT_ASSERT(checkAddingStatus(112, SWAPPED, 0, 127));
    CPPUNIT_ASSERT(checkAddingStatus(126, SWAPPED, 0, 72));
    CPPUNIT_ASSERT(checkAddingStatus(68, NOT_ADDED, 1));
  }

  void duplicateItemTest()
  {
    addAppraisers();
    _s->addItem(17);
    _s->addItem(56);
    _s->addItem(8);
    _s->addItem(192);
    _s->addItem(6);
    _s->addItem(256);
    _s->addItem(414);

    // Six is in the swapper and we try to add it
    // for the second time - exception
    CPPUNIT_ASSERT_THROW(_s->addItem(6), ErrorResponseException);

    // Warning: if something is not in the swapper,
    // we can try to add it freely for the second time
    _s->addItem(56);
  }

private:

  static const int DUMMY_ITEM_1 = 7;
  static const int DUMMY_ITEM_2 = 11;
  static const int DUMMY_ITEM_3 = 254;

  // For scalar types value-initialization performs zero-initialization
  // When swapper returns ItemType() for int, it just returns 0
  typedef Swapper<int> Swp;
  typedef PriorityScoreFormatter<int> ScoreFormatter;

  enum AddStatus
  {
    NOT_ADDED = 0,
    SWAPPED,
    ADDED
  };

  void addAppraisers()
  {
    int threshold = 100;
    unsigned int quantity = 3;
    _above100 = _memHandle.create<NumbersAboveX>(threshold, quantity);
    _allEven = _memHandle.create<AllEvenNumbersRepresented>();
    _takeAll = _memHandle.create<TakeAllNumbers>();
    _smallerBetter = _memHandle.create<SmallerNumbersAreBetter>();

    _s->addAppraiser(_allEven, 30);
    _s->addAppraiser(_takeAll, 20);
    _s->addAppraiser(_above100, 40);
    _s->addAppraiser(_smallerBetter, 10);
  }

  bool checkAddingStatus(int n,
                         AddStatus expectedStatus,
                         unsigned int expectedNPIC,
                         int expectedSwappedOut = -1)
  {
    const Swp::AddResponse res = _s->addItem(n);

    if (expectedStatus == ADDED)
    {
      if (res.first != 0)
      {
        cout << "Expected status ADDED while res = " << res.first << endl;
        return false;
      }
    }
    else if (expectedStatus == SWAPPED)
    {
      if (res.first != expectedSwappedOut)
      {
        cout << "Expected status SWAPPED and res = " << expectedSwappedOut
             << " while res = " << res.first << endl;
        return false;
      }
    }
    else
    {
      if (res.first != n)
      {
        cout << "Expected status NOT_ADDED while res = " << res.first << endl;
        return false;
      }
    }

    const unsigned int npic = _s->getNoProgressIterationsCount();
    if (npic != expectedNPIC)
    {
      cout << "Expected NPIC = " << expectedNPIC << ", actual " << npic << endl;
      return false;
    }
    return true;
  }

  void print() { cout << *_s; }

  std::string scoreToStr(const BasicAppraiserScore& s)
  {
    std::ostringstream out;
    out << s;
    return out.str();
  }

  bool predictScores(const std::string& expected, const Swp::AppraiserScoresMap& m)
  {
    const std::string actual = ScoreFormatter(*_s).formatScores(m);
    if (expected != actual)
    {
      cout << "expected [" << expected << "] != actual [" << actual << "]" << endl;
      return false;
    }
    return true;
  }

  bool predictBottomItem(const std::string& expected)
  {
    Swp::Iterator it = _s->begin();
    const Swp::AppraiserScoresMap& m = _s->getAppraiserScoresForItem(it->key);
    return predictScores(expected, m);
  }

  static const unsigned int DEFAULT_CAPACITY;

  TestMemHandle _memHandle;

  // Appraisers
  NumbersAboveX* _above100;
  AllEvenNumbersRepresented* _allEven;
  TakeAllNumbers* _takeAll;
  SmallerNumbersAreBetter* _smallerBetter;

  Swp* _s;
};

const unsigned int SwapperTest::DEFAULT_CAPACITY = 5;

CPPUNIT_TEST_SUITE_REGISTRATION(SwapperTest);

} // namespace tse
