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
#include "Pricing/Shopping/Utils/CountingMultimap.h"
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

const unsigned int DUMMY_KEY_1 = 5;
const unsigned int DUMMY_KEY_2 = 19;

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class CountingMultimapTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CountingMultimapTest);
  CPPUNIT_TEST(receiverSettingTest);
  CPPUNIT_TEST(receiverSettingFailureTest);
  CPPUNIT_TEST(addTestNoPredicateTrigger);
  CPPUNIT_TEST(addTestPredicateTrigger);
  CPPUNIT_TEST(triggerForTwoValues);
  CPPUNIT_TEST(addNoReceiverErrorTest);
  CPPUNIT_TEST(removeTestNoPredicateTrigger);
  CPPUNIT_TEST(removeTestPredicateTrigger);
  CPPUNIT_TEST(removeForTwoValues);
  CPPUNIT_TEST(removeNoKeyErrorTest);
  CPPUNIT_TEST(removeNoValueErrorTest);
  CPPUNIT_TEST(getValuesTest);
  CPPUNIT_TEST(addDuplicateErrorTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _true = _memHandle.create<TruePredicate>();
    _false = _memHandle.create<FalsePredicate>();
    _dr = _memHandle.create<ImplDummyReceiver>();
    _tmap = _memHandle.create<Tmap>(*_true);
    _fmap = _memHandle.create<Fmap>(*_false);
  }

  void tearDown() { _memHandle.clear(); }

  void receiverSettingTest()
  {
    _tmap->setReceiver(_dr);
    _fmap->setReceiver(_dr);
  }

  void receiverSettingFailureTest()
  {
    CPPUNIT_ASSERT_THROW(_tmap->setReceiver(0), tse::ErrorResponseException);
  }

  void addTestNoPredicateTrigger()
  {
    receiverSettingTest();
    _fmap->add(DUMMY_KEY_1, "Bach");
    CPPUNIT_ASSERT_EQUAL(string(""), _dr->dumpLog());
  }

  void addTestPredicateTrigger()
  {
    receiverSettingTest();
    _tmap->add(DUMMY_KEY_1, "Bach");
    CPPUNIT_ASSERT_EQUAL(string("Bach:1|"), _dr->dumpLog());
  }

  void triggerForTwoValues()
  {
    addTestPredicateTrigger();
    _tmap->add(DUMMY_KEY_1, "KatieMelua");
    CPPUNIT_ASSERT_EQUAL(string("Bach:1|KatieMelua:1|"), _dr->dumpLog());
  }

  void addNoReceiverErrorTest()
  {
    CPPUNIT_ASSERT_THROW(_tmap->add(DUMMY_KEY_1, "Bach"), tse::ErrorResponseException);
  }

  void removeTestNoPredicateTrigger()
  {
    addTestNoPredicateTrigger();
    _fmap->remove(DUMMY_KEY_1, "Bach");
    CPPUNIT_ASSERT_EQUAL(string(""), _dr->dumpLog());
  }

  void removeTestPredicateTrigger()
  {
    addTestPredicateTrigger();
    _tmap->remove(DUMMY_KEY_1, "Bach");
    CPPUNIT_ASSERT_EQUAL(string("Bach:0|"), _dr->dumpLog());
  }

  void removeForTwoValues()
  {
    triggerForTwoValues();
    _tmap->remove(DUMMY_KEY_1, "Bach");
    CPPUNIT_ASSERT_EQUAL(string("Bach:0|KatieMelua:0|"), _dr->dumpLog());
  }

  void removeNoKeyErrorTest()
  {
    addTestNoPredicateTrigger();
    CPPUNIT_ASSERT_THROW(_fmap->remove(DUMMY_KEY_2, "Bach"), tse::ErrorResponseException);
  }

  void removeNoValueErrorTest()
  {
    addTestNoPredicateTrigger();
    CPPUNIT_ASSERT_THROW(_fmap->remove(DUMMY_KEY_1, "Calculator"), tse::ErrorResponseException);
  }

  void getValuesTest()
  {
    addTestNoPredicateTrigger();
    _fmap->add(DUMMY_KEY_1, "Beethoven");
    const Fmap::ValueSet& vv = _fmap->getValues(DUMMY_KEY_1);
    CPPUNIT_ASSERT_EQUAL(size_t(2), vv.size());
    CPPUNIT_ASSERT(vv.find("Bach") != vv.end());
    CPPUNIT_ASSERT(vv.find("Beethoven") != vv.end());
  }

  void addDuplicateErrorTest()
  {
    addTestPredicateTrigger();
    CPPUNIT_ASSERT_THROW(_tmap->add(DUMMY_KEY_1, "Bach"), tse::ErrorResponseException);
  }

private:
  typedef CountingMultimap<int, string, TruePredicate> Tmap;
  typedef CountingMultimap<int, string, FalsePredicate> Fmap;
  typedef DummyReceiver<string> ImplDummyReceiver;

  TestMemHandle _memHandle;
  TruePredicate* _true;
  FalsePredicate* _false;
  ImplDummyReceiver* _dr;
  Tmap* _tmap;
  Fmap* _fmap;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CountingMultimapTest);

} // namespace utils

} // namespace tse
