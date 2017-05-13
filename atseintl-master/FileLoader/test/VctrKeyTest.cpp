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
#include <set>
#include <time.h>
#include "test/include/CppUnitHelperMacros.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/FareInfo.h"
#include "FileLoader/VctrKey.h"
#include "DBAccess/TSEDateInterval.h"

using namespace tse;
using namespace std;

namespace tse
{
class VctrKeyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(VctrKeyTest);
  CPPUNIT_TEST(testOperatorLessThanForEqualValues);
  CPPUNIT_TEST(testOperatorLessThanForVendor);
  CPPUNIT_TEST(testOperatorLessThanForCarrier);
  CPPUNIT_TEST(testOperatorLessThanForFareTariff);
  CPPUNIT_TEST(testOperatorLessThanForRuleNumber);
  CPPUNIT_TEST(testStreamingOperator);
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
    VctrKey key1(_fareInfo1);
    VctrKey key2(_fareInfo2);
    CPPUNIT_ASSERT(!(key1 < key2));
    CPPUNIT_ASSERT(!(key2 < key1));
  }

  void testOperatorLessThanForVendor()
  {
    _fareInfo1._vendor = "ATP";
    VctrKey key1(_fareInfo1);
    VctrKey key2(_fareInfo2);
    CPPUNIT_ASSERT(!(key1 < key2));
    CPPUNIT_ASSERT((key2 < key1));
  }

  void testOperatorLessThanForCarrier()
  {
    _fareInfo1._carrier = "CO";
    VctrKey key1(_fareInfo1);
    VctrKey key2(_fareInfo2);
    CPPUNIT_ASSERT(!(key1 < key2));
    CPPUNIT_ASSERT((key2 < key1));
  }

  void testOperatorLessThanForFareTariff()
  {
    _fareInfo1._fareTariff = 50;
    VctrKey key1(_fareInfo1);
    VctrKey key2(_fareInfo2);
    CPPUNIT_ASSERT(!(key1 < key2));
    CPPUNIT_ASSERT((key2 < key1));
  }

  void testOperatorLessThanForRuleNumber()
  {
    _fareInfo1._ruleNumber = "P100";
    VctrKey key1(_fareInfo1);
    VctrKey key2(_fareInfo2);
    CPPUNIT_ASSERT(!(key1 < key2));
    CPPUNIT_ASSERT((key2 < key1));
  }

  void testStreamingOperator()
  {
    std::ostringstream ostr;
    _fareInfo1._vendor = "ATP";
    _fareInfo1._carrier = "CO";
    _fareInfo1._fareTariff = 50;
    _fareInfo1._ruleNumber = "P100";
    VctrKey key1(_fareInfo1);
    ostr << key1;
    string expected = "ATP CO 50 P100";
    CPPUNIT_ASSERT_EQUAL(expected, ostr.str());
  }

private:
  FareInfo _fareInfo1;
  FareInfo _fareInfo2;
  TSEDateInterval _effInterval1;
  TSEDateInterval _effInterval2;
};
CPPUNIT_TEST_SUITE_REGISTRATION(VctrKeyTest);
} // namespace
