// ---------------------------------------------------------------------------
//
//  Copyright Sabre 2007
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#include "DataModel/Itin.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/Category4.h"
#include "Taxes/LegacyTaxes/TaxDisplayCAT.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

class TaxDisplayDiffTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayDiffTest);
  CPPUNIT_TEST(Test1);
  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:
  void setup() { _memHandle.create<SpecificTestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void Test1()
  {
    TaxTrx trx;
    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();
    trx.getRequest()->sequenceNumber() = 1500;
    trx.getRequest()->sequenceNumber2() = 1500;
    trx.getRequest()->categoryVec().push_back(4); // should differ from 1

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);
    // create

    TaxDisplayItem* tdi1 = _memHandle.create<TaxDisplayItem>();
    TaxDisplayItem* tdi2 = _memHandle.create<TaxDisplayItem>();

    TaxCodeReg taxCodeReg;
    TaxCodeReg taxCodeReg2;

    tdi1->taxCodeReg() = &taxCodeReg;
    taxCodeReg.tripType() = TripTypesValidator::TAX_FROM_TO;
    taxCodeReg.loc1ExclInd() = YES;
    taxCodeReg.loc1() = tse::LocCode("DE");
    taxCodeReg.loc2() = tse::LocCode("GB");
    taxCodeReg.loc1Type() = tse::Indicator('N');
    taxCodeReg.loc2Type() = tse::Indicator('N');

    tdi2->taxCodeReg() = &taxCodeReg2;
    taxCodeReg2.loc1ExclInd() = YES;
    taxCodeReg2.tripType() = TripTypesValidator::TAX_FROM_TO;
    taxCodeReg2.loc1() = tse::LocCode("DE");
    taxCodeReg2.loc2() = tse::LocCode("GB");
    taxCodeReg2.loc1Type() = tse::Indicator('N');
    taxCodeReg2.loc2Type() = tse::Indicator('N');

    trx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(tdi1);
    trx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(tdi2);
    // build cats...
    TaxDisplayCAT tdc;
    tdc.buildCats(trx);

    // identical, should be null
    CPPUNIT_ASSERT_EQUAL(tdi1->category4()->subCat1(), std::string(""));
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayDiffTest);
}
