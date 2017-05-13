//-------------------------------------------------------------------
//  Copyright Sabre 2009
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

#include <iostream>
#include <time.h>
#include "test/include/CppUnitHelperMacros.h"

#include "DBAccess/FareInfo.h"
#include "Common/FareDisplayUtil.h"
#include "FileLoader/FareSortKey.h"
#include "DBAccess/TSEDateInterval.h"

using namespace tse;
using namespace std;

namespace tse
{
class FareSortKeyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareSortKeyTest);
  CPPUNIT_TEST(testOperatorLessThanForEqualValues);
  CPPUNIT_TEST(testOperatorLessThanForVendor);
  CPPUNIT_TEST(testOperatorLessThanForCarrier);
  CPPUNIT_TEST(testOperatorLessThanForFareTariff);
  CPPUNIT_TEST(testOperatorLessThanForCurrency);
  CPPUNIT_TEST(testOperatorLessThanForFareClass);
  CPPUNIT_TEST(testOperatorLessThanForSequenceNumber);
  CPPUNIT_TEST(testOperatorLessThanForLinkNumber);
  CPPUNIT_TEST(testOperatorLessThanForCreateDate);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _fareInfo1.effInterval() = _effInterval1;
    _fareInfo2.effInterval() = _effInterval2;
    _fareInfo1.createDate() = DateTime(2007, 1, 1, 8, 0, 0);
    _fareInfo2.createDate() = DateTime(2007, 1, 1, 8, 0, 0);
  }

  void testOperatorLessThanForEqualValues()
  {
    FareSortKey key1(_fareInfo1);
    FareSortKey key2(_fareInfo2);
    CPPUNIT_ASSERT(!(key1 < key2));
    CPPUNIT_ASSERT(!(key2 < key1));
  }

  void testOperatorLessThanForVendor()
  {
    _fareInfo1._vendor = 'A';
    FareSortKey key1(_fareInfo1);
    FareSortKey key2(_fareInfo2);
    CPPUNIT_ASSERT(!(key1 < key2));
    CPPUNIT_ASSERT((key2 < key1));
  }

  void testOperatorLessThanForCarrier()
  {
    _fareInfo1._carrier = "AA";
    FareSortKey key1(_fareInfo1);
    FareSortKey key2(_fareInfo2);
    CPPUNIT_ASSERT(!(key1 < key2));
    CPPUNIT_ASSERT((key2 < key1));
  }

  void testOperatorLessThanForFareTariff()
  {
    _fareInfo1._fareTariff = 50;
    FareSortKey key1(_fareInfo1);
    FareSortKey key2(_fareInfo2);
    CPPUNIT_ASSERT(!(key1 < key2));
    CPPUNIT_ASSERT((key2 < key1));
  }

  void testOperatorLessThanForCurrency()
  {
    _fareInfo1._currency = "USD";
    FareSortKey key1(_fareInfo1);
    FareSortKey key2(_fareInfo2);
    CPPUNIT_ASSERT(!(key1 < key2));
    CPPUNIT_ASSERT((key2 < key1));
  }

  void testOperatorLessThanForFareClass()
  {
    _fareInfo1._fareClass = "E";
    FareSortKey key1(_fareInfo1);
    FareSortKey key2(_fareInfo2);
    CPPUNIT_ASSERT(!(key1 < key2));
    CPPUNIT_ASSERT((key2 < key1));
  }

  void testOperatorLessThanForSequenceNumber()
  {
    _fareInfo1._sequenceNumber = 392;
    FareSortKey key1(_fareInfo1);
    FareSortKey key2(_fareInfo2);
    CPPUNIT_ASSERT(!(key1 < key2));
    CPPUNIT_ASSERT((key2 < key1));
  }

  void testOperatorLessThanForLinkNumber()
  {
    _fareInfo1._linkNumber = 33;
    FareSortKey key1(_fareInfo1);
    FareSortKey key2(_fareInfo2);
    CPPUNIT_ASSERT(!(key1 < key2));
    CPPUNIT_ASSERT((key2 < key1));
  }

  void testOperatorLessThanForCreateDate()
  {
    _fareInfo1.createDate() = DateTime(2008, 1, 1, 8, 0, 0);
    FareSortKey key1(_fareInfo1);
    FareSortKey key2(_fareInfo2);
    CPPUNIT_ASSERT(!(key1 < key2));
    CPPUNIT_ASSERT((key2 < key1));
  }

private:
  FareInfo _fareInfo1;
  FareInfo _fareInfo2;
  TSEDateInterval _effInterval1;
  TSEDateInterval _effInterval2;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareSortKeyTest);
}
