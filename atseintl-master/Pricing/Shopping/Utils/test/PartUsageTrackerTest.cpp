
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
#include "Pricing/Shopping/Utils/PartUsageTracker.h"
#include "Pricing/Shopping/Utils/test/DummyReceiver.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

namespace tse
{

namespace utils
{

const unsigned int DUMMY_VALUE_1 = 7;
const unsigned int DUMMY_VALUE_2 = 19;

const unsigned int DUMMY_PART_1 = 5;
const unsigned int DUMMY_PART_2 = 10;
const unsigned int DUMMY_PART_3 = 50;

const unsigned int THE_VALUE_OF_MULTIPLE = 100;

class IdentityDeconstructor
{
public:
  typedef int ItemType;
  typedef int PartType;
  typedef std::vector<int> PartVector;

  void insertItem(const int& n)
  {
    _parts.clear();
    _parts.push_back(n);
  }

  const PartVector& getItemParts() const { return _parts; }

private:
  PartVector _parts;
};

class DummyDeconstructor
{
public:
  typedef int ItemType;
  typedef int PartType;
  typedef std::vector<int> PartVector;

  void insertItem(const int& n)
  {
    _parts.clear();
    _parts.push_back(DUMMY_PART_1);
    _parts.push_back(DUMMY_PART_2);
    _parts.push_back(DUMMY_PART_3);
  }

  const PartVector& getItemParts() const { return _parts; }

private:
  PartVector _parts;
};

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class PartUsageTrackerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PartUsageTrackerTest);
  CPPUNIT_TEST(receiverSettingTest);
  CPPUNIT_TEST(receiverSettingFailureTest);
  CPPUNIT_TEST(addTestNoPredicateTrigger);
  CPPUNIT_TEST(addTestPredicateTrigger);
  CPPUNIT_TEST(addNoReceiverErrorTest);
  CPPUNIT_TEST(removeTestNoPredicateTrigger);
  CPPUNIT_TEST(removeTestPredicateTrigger);
  CPPUNIT_TEST(doubleAddTest);
  CPPUNIT_TEST(doubleAddIdentityDeconstructor);
  CPPUNIT_TEST(multipleAddRemoveTest);
  CPPUNIT_TEST(removeNoValueErrorTest);
  CPPUNIT_TEST(getItemsContainingPartTest);
  CPPUNIT_TEST(addDuplicateTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _true = _memHandle.create<TruePredicate>();
    _false = _memHandle.create<FalsePredicate>();
    _dr = _memHandle.create<ImplDummyReceiver>();
    _ttracker = _memHandle.create<TTracker>(*_true);
    _ftracker = _memHandle.create<FTracker>(*_false);
    _idtracker = _memHandle.create<IdTracker>(*_true);
  }

  void tearDown() { _memHandle.clear(); }

  void receiverSettingTest()
  {
    _ttracker->setReceiver(_dr);
    _ftracker->setReceiver(_dr);
    _idtracker->setReceiver(_dr);
  }

  void receiverSettingFailureTest()
  {
    CPPUNIT_ASSERT_THROW(_ttracker->setReceiver(0), tse::ErrorResponseException);
  }

  void addTestNoPredicateTrigger()
  {
    receiverSettingTest();
    _ftracker->add(DUMMY_VALUE_1);
    CPPUNIT_ASSERT_EQUAL(string(""), _dr->dumpLog());
  }

  void addTestPredicateTrigger()
  {
    receiverSettingTest();
    _ttracker->add(DUMMY_VALUE_1);
    // three triggers from keys: 5, 10, 50
    // but only one trigger on the item level
    CPPUNIT_ASSERT_EQUAL(string("7:1|"), _dr->dumpLog());
  }

  void addNoReceiverErrorTest()
  {
    CPPUNIT_ASSERT_THROW(_ttracker->add(DUMMY_VALUE_1), tse::ErrorResponseException);
  }

  void removeTestNoPredicateTrigger()
  {
    addTestNoPredicateTrigger();
    _ftracker->remove(DUMMY_VALUE_1);
    CPPUNIT_ASSERT_EQUAL(string(""), _dr->dumpLog());
  }

  void removeTestPredicateTrigger()
  {
    addTestPredicateTrigger();
    _ttracker->remove(DUMMY_VALUE_1);
    // three triggers from keys: 5, 10, 50
    // but only one trigger on the item level
    CPPUNIT_ASSERT_EQUAL(string("7:0|"), _dr->dumpLog());
  }

  void doubleAddTest()
  {
    addTestPredicateTrigger();
    _ttracker->add(DUMMY_VALUE_2);
    // 1st add: 5, 10, 50 -> 7
    // 2nd add: 5, 10, 50 -> 7, 19
    // <5, 10, 50> reduced to one event
    // but still for 2 items: 7, 19
    CPPUNIT_ASSERT_EQUAL(string("19:1|7:1|"), _dr->dumpLog());
  }

  void doubleAddIdentityDeconstructor()
  {
    receiverSettingTest();
    _idtracker->add(DUMMY_VALUE_1);
    CPPUNIT_ASSERT_EQUAL(string("7:1|"), _dr->dumpLog());
    _idtracker->add(DUMMY_VALUE_2);
    CPPUNIT_ASSERT_EQUAL(string("19:1|"), _dr->dumpLog());
  }

  void multipleAddRemoveTest()
  {
    for (unsigned int i = 0; i < THE_VALUE_OF_MULTIPLE; ++i)
    {
      removeTestPredicateTrigger();
    }
  }

  void removeNoValueErrorTest()
  {
    addTestNoPredicateTrigger();
    CPPUNIT_ASSERT_THROW(_ftracker->remove(DUMMY_VALUE_2), tse::ErrorResponseException);
  }

  void getItemsContainingPartTest()
  {
    addTestNoPredicateTrigger();
    _ftracker->add(DUMMY_VALUE_2);
    const FTracker::ItemSet& items = _ftracker->getItemsContainingPart(DUMMY_PART_1);
    CPPUNIT_ASSERT_EQUAL(size_t(2), items.size());
    CPPUNIT_ASSERT(items.find(DUMMY_VALUE_1) != items.end());
    CPPUNIT_ASSERT(items.find(DUMMY_VALUE_2) != items.end());
  }

  void addDuplicateTest()
  {
    addTestPredicateTrigger();
    CPPUNIT_ASSERT_THROW(_ttracker->add(DUMMY_VALUE_1), tse::ErrorResponseException);
  }

private:
  typedef PartUsageTracker<DummyDeconstructor, TruePredicate> TTracker;
  typedef PartUsageTracker<DummyDeconstructor, FalsePredicate> FTracker;
  typedef PartUsageTracker<IdentityDeconstructor, TruePredicate> IdTracker;
  typedef DummyReceiver<int> ImplDummyReceiver;

  TestMemHandle _memHandle;
  TruePredicate* _true;
  FalsePredicate* _false;
  ImplDummyReceiver* _dr;
  TTracker* _ttracker;
  FTracker* _ftracker;
  IdTracker* _idtracker;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PartUsageTrackerTest);

} // namespace utils

} // namespace tse
