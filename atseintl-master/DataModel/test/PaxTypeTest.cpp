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

#include "test/include/CppUnitHelperMacros.h"
#include "Common/DateTime.h"
#include "DataModel/PaxType.h"
#include "DBAccess/PaxTypeInfo.h"

#include "test/testdata/TestLocFactory.h"

using namespace tse;
using namespace std;

namespace tse
{
class PaxTypeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PaxTypeTest);
  CPPUNIT_TEST(testOperaterEqualEqualEmpty);
  CPPUNIT_TEST(testOperaterEqualEqual);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualPaxType);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualNumber);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualAge);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualStateCode);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualTotalPaxNumber);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualInputOrder);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualVendorCode);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualPaxTypeInfo);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualActualPaxType);
  CPPUNIT_TEST_SUITE_END();

public:
  void testOperaterEqualEqualEmpty()
  {
    PaxType paxType1;
    PaxType paxType2;
    CPPUNIT_ASSERT(paxType1 == paxType2);
  }

  void testOperaterEqualEqual()
  {
    PaxType paxType1, paxType2;
    setupEqualPaxTypes(paxType1, paxType2);
    CPPUNIT_ASSERT(paxType1 == paxType2);
  }

  void testOperaterEqualEqualNotEqualPaxType()
  {
    PaxType paxType1, paxType2;
    setupEqualPaxTypes(paxType1, paxType2);
    paxType2.paxType() = "INF";
    CPPUNIT_ASSERT(!(paxType1 == paxType2));
  }

  void testOperaterEqualEqualNotEqualNumber()
  {
    PaxType paxType1, paxType2;
    setupEqualPaxTypes(paxType1, paxType2);
    paxType2.number() = 3;
    CPPUNIT_ASSERT(!(paxType1 == paxType2));
  }

  void testOperaterEqualEqualNotEqualAge()
  {
    PaxType paxType1, paxType2;
    setupEqualPaxTypes(paxType1, paxType2);
    paxType2.age() = 2;
    CPPUNIT_ASSERT(!(paxType1 == paxType2));
  }

  void testOperaterEqualEqualNotEqualStateCode()
  {
    PaxType paxType1, paxType2;
    setupEqualPaxTypes(paxType1, paxType2);
    paxType2.stateCode() = "AR";
    CPPUNIT_ASSERT(!(paxType1 == paxType2));
  }

  void testOperaterEqualEqualNotEqualTotalPaxNumber()
  {
    PaxType paxType1, paxType2;
    setupEqualPaxTypes(paxType1, paxType2);
    paxType2.totalPaxNumber() = 3;
    CPPUNIT_ASSERT(!(paxType1 == paxType2));
  }

  void testOperaterEqualEqualNotEqualInputOrder()
  {
    PaxType paxType1, paxType2;
    setupEqualPaxTypes(paxType1, paxType2);
    paxType2.inputOrder() = 0;
    CPPUNIT_ASSERT(!(paxType1 == paxType2));
  }

  void testOperaterEqualEqualNotEqualVendorCode()
  {
    PaxType paxType1, paxType2;
    setupEqualPaxTypes(paxType1, paxType2);
    paxType2.vendorCode() = "BMD";
    CPPUNIT_ASSERT(!(paxType1 == paxType2));
  }

  void testOperaterEqualEqualNotEqualPaxTypeInfo()
  {
    PaxType paxType1, paxType2;
    setupEqualPaxTypes(paxType1, paxType2);
    PaxTypeInfo paxTypeInfo;
    paxType2.paxTypeInfo() = &paxTypeInfo;
    CPPUNIT_ASSERT(!(paxType1 == paxType2));
  }

  void testOperaterEqualEqualNotEqualActualPaxType()
  {
    PaxType paxType1, paxType2;
    setupEqualPaxTypes(paxType1, paxType2);
    paxType2.actualPaxType().clear();
    CPPUNIT_ASSERT(!(paxType1 == paxType2));
  }

  void setupEqualPaxTypes(PaxType& paxType1, PaxType& paxType2)
  {
    paxType1.paxType() = "ADT";
    paxType2.paxType() = "ADT";
    paxType1.number() = 5;
    paxType2.number() = 5;
    paxType1.age() = 23;
    paxType2.age() = 23;
    paxType1.stateCode() = "TX";
    paxType2.stateCode() = "TX";
    paxType1.totalPaxNumber() = 1;
    paxType2.totalPaxNumber() = 1;
    paxType1.inputOrder() = 1;
    paxType2.inputOrder() = 1;
    paxType1.vendorCode() = "ATA";
    paxType2.vendorCode() = "ATA";
    paxType1.paxTypeInfo() = &_paxTypeInfo;
    paxType2.paxTypeInfo() = &_paxTypeInfo;
    _paxTypes.push_back(&paxType1);
    paxType1.actualPaxType().insert(make_pair(CarrierCode("AA"), &_paxTypes));
    paxType2.actualPaxType().insert(make_pair(CarrierCode("AA"), &_paxTypes));
  }

private:
  PaxTypeInfo _paxTypeInfo;
  std::vector<PaxType*> _paxTypes;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PaxTypeTest);
}
