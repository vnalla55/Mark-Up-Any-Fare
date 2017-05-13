
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
#include "Pricing/Shopping/Utils/UsageCounter.h"

#include <string>
#include <sstream>
#include <algorithm>

namespace tse
{

namespace utils
{

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class UsageCounterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(UsageCounterTest);
  CPPUNIT_TEST(emptyCounterTest);
  CPPUNIT_TEST(onlyElementsAdded);
  CPPUNIT_TEST(addElementsAndUpdateUsage);
  CPPUNIT_TEST(checkExceptions);
  CPPUNIT_TEST(streamOutputCheck);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _counter = _memHandle.create<UsageCounter<int> >();
  }

  void tearDown() { _memHandle.clear(); }

  void emptyCounterTest()
  {
    using namespace std;

    CPPUNIT_ASSERT_EQUAL((unsigned int)0, _counter->getNbrOfElements());
    CPPUNIT_ASSERT_EQUAL((unsigned int)0, _counter->getNbrOfUnusedElements());
    CPPUNIT_ASSERT_EQUAL((size_t)0, _counter->getUnusedElements().size());

    CPPUNIT_ASSERT_THROW(_counter->getUsageCount(10), tse::ErrorResponseException);
    CPPUNIT_ASSERT_THROW(_counter->increaseUsageCount(12), tse::ErrorResponseException);
    CPPUNIT_ASSERT_THROW(_counter->decreaseUsageCount(0), tse::ErrorResponseException);

    set<int> empty;
    CPPUNIT_ASSERT(empty == _counter->getAllElements());
  }

  void onlyElementsAdded()
  {
    using namespace std;

    _counter->addElement(1);
    _counter->addElement(3);
    // Object should not raise any error on duplication
    // for set of elements
    _counter->addElement(3);
    _counter->addElement(7);
    CPPUNIT_ASSERT_EQUAL((unsigned)3, _counter->getNbrOfElements());
    CPPUNIT_ASSERT_EQUAL((unsigned)3, _counter->getNbrOfUnusedElements());

    set<int> manual;
    manual.insert(1);
    manual.insert(3);
    manual.insert(7);
    CPPUNIT_ASSERT(manual == _counter->getUnusedElements());
    CPPUNIT_ASSERT(manual == _counter->getAllElements());

    CPPUNIT_ASSERT_EQUAL((unsigned)0, _counter->getUsageCount(3));
    CPPUNIT_ASSERT_EQUAL((unsigned)0, _counter->getUsageCount(7));
    CPPUNIT_ASSERT_EQUAL((unsigned)0, _counter->getUsageCount(1));

    CPPUNIT_ASSERT_THROW(_counter->getUsageCount(10), tse::ErrorResponseException);
    CPPUNIT_ASSERT_THROW(_counter->increaseUsageCount(12), tse::ErrorResponseException);
    CPPUNIT_ASSERT_THROW(_counter->decreaseUsageCount(0), tse::ErrorResponseException);
  }

  void addElementsAndUpdateUsage()
  {
    using namespace std;

    _counter->addElement(1);
    _counter->addElement(3);
    _counter->addElement(3);
    _counter->addElement(7);
    CPPUNIT_ASSERT_EQUAL((unsigned)3, _counter->getNbrOfElements());
    CPPUNIT_ASSERT_EQUAL((unsigned)3, _counter->getNbrOfUnusedElements());

    CPPUNIT_ASSERT_EQUAL((unsigned)0, _counter->getUsageCount(7));
    CPPUNIT_ASSERT_EQUAL((unsigned)0, _counter->getUsageCount(3));
    CPPUNIT_ASSERT_EQUAL((unsigned)0, _counter->getUsageCount(1));

    set<int> manual;
    manual.insert(1);
    manual.insert(3);
    manual.insert(7);
    CPPUNIT_ASSERT(manual == _counter->getUnusedElements());
    CPPUNIT_ASSERT(manual == _counter->getAllElements());

    _counter->increaseUsageCount(1);
    CPPUNIT_ASSERT(manual == _counter->getAllElements());
    _counter->increaseUsageCount(1);
    _counter->increaseUsageCount(1);
    CPPUNIT_ASSERT_EQUAL((unsigned)3, _counter->getUsageCount(1));
    _counter->increaseUsageCount(3);
    CPPUNIT_ASSERT_EQUAL((unsigned)1, _counter->getUsageCount(3));
    CPPUNIT_ASSERT(manual == _counter->getAllElements());
    _counter->increaseUsageCount(7);
    CPPUNIT_ASSERT_EQUAL((unsigned)1, _counter->getUsageCount(7));
    _counter->increaseUsageCount(7);
    CPPUNIT_ASSERT_EQUAL((unsigned)2, _counter->getUsageCount(7));

    CPPUNIT_ASSERT_EQUAL((size_t)0, _counter->getUnusedElements().size());
    CPPUNIT_ASSERT_EQUAL((unsigned)3, _counter->getNbrOfElements());
    CPPUNIT_ASSERT_EQUAL((unsigned)0, _counter->getNbrOfUnusedElements());
    CPPUNIT_ASSERT(manual == _counter->getAllElements());

    _counter->decreaseUsageCount(3);
    set<int> tmpUnused;
    tmpUnused.insert(3);
    CPPUNIT_ASSERT(tmpUnused == _counter->getUnusedElements());
    CPPUNIT_ASSERT(manual == _counter->getAllElements());
    CPPUNIT_ASSERT_EQUAL((unsigned)3, _counter->getNbrOfElements());
    CPPUNIT_ASSERT_EQUAL((unsigned)1, _counter->getNbrOfUnusedElements());

    _counter->decreaseUsageCount(7);
    _counter->decreaseUsageCount(7);
    tmpUnused.insert(7);
    CPPUNIT_ASSERT(tmpUnused == _counter->getUnusedElements());
    CPPUNIT_ASSERT(manual == _counter->getAllElements());
    CPPUNIT_ASSERT_EQUAL((unsigned)3, _counter->getNbrOfElements());
    CPPUNIT_ASSERT_EQUAL((unsigned)2, _counter->getNbrOfUnusedElements());

    _counter->decreaseUsageCount(1);
    _counter->decreaseUsageCount(1);
    _counter->decreaseUsageCount(1);
    CPPUNIT_ASSERT(manual == _counter->getUnusedElements());
    CPPUNIT_ASSERT_EQUAL((unsigned)3, _counter->getNbrOfElements());
    CPPUNIT_ASSERT_EQUAL((unsigned)3, _counter->getNbrOfUnusedElements());
  }

  void checkExceptions()
  {
    using namespace std;

    _counter->addElement(1);
    _counter->addElement(3);
    _counter->addElement(7);

    for (unsigned int i = 0; i < 10; ++i)
    {
      _counter->increaseUsageCount(7);
      CPPUNIT_ASSERT_EQUAL((i + 1), _counter->getUsageCount(7));
    }

    for (unsigned int i = 0; i < 10; ++i)
    {
      _counter->decreaseUsageCount(7);
      CPPUNIT_ASSERT_EQUAL((10 - i - 1), _counter->getUsageCount(7));
    }
    CPPUNIT_ASSERT_THROW(_counter->decreaseUsageCount(7), tse::ErrorResponseException);
  }

  void streamOutputCheck()
  {
    using namespace std;

    _counter->addElement(25);
    _counter->addElement(52);
    _counter->addElement(7);
    _counter->addElement(3);
    _counter->addElement(3);
    _counter->addElement(1);

    _counter->increaseUsageCount(7);
    _counter->increaseUsageCount(3);
    _counter->increaseUsageCount(52);
    _counter->decreaseUsageCount(3);
    _counter->increaseUsageCount(3);
    _counter->increaseUsageCount(3);
    _counter->increaseUsageCount(52);
    _counter->increaseUsageCount(7);
    _counter->increaseUsageCount(7);
    _counter->increaseUsageCount(7);
    _counter->increaseUsageCount(7);

    // 2 x 3
    // 5 x 7
    // 2 x 52

    ostringstream out;
    out << *_counter;
    string s = out.str();

    // Replace all \t with space
    replace(s.begin(), s.end(), '\t', ' ');

    string REF = "{UsageCounter total elements: 5, unused elements: 2\n"
                 "Set of unused elements:\n"
                 "1\n"
                 "25\n"
                 "Usage count map:\n"
                 "element: 3 usages: 2\n"
                 "element: 7 usages: 5\n"
                 "element: 52 usages: 2\n"
                 "}";

    CPPUNIT_ASSERT_EQUAL(REF, s);
  }

private:
  TestMemHandle _memHandle;

  UsageCounter<int>* _counter;
};

CPPUNIT_TEST_SUITE_REGISTRATION(UsageCounterTest);

} // namespace utils

} // namespace tse
