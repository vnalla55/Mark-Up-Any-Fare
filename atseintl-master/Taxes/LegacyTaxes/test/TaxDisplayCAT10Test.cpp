#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxRequest.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/Category10.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <time.h>
#include <unistd.h>
#include <vector>

using namespace std;

namespace tse
{
class TaxDisplayCAT10Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT10Test);
  CPPUNIT_TEST(testCreation);
  CPPUNIT_TEST(testFareDisplay);
  CPPUNIT_TEST(testFareDisplayFamily);
  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }
  void testCreation()
  {
    Category10* cat10 = _memHandle.create<Category10>();
    CPPUNIT_ASSERT(cat10 != 0);
  }

  void testFareDisplay()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxCodeReg taxCodeReg;
    TaxDisplayItem taxDisplayItem;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;

    taxCodeReg.fareclassExclInd() = tse::Indicator('N');
    taxCodeReg.restrictionFareClass().push_back("F");

    Category10 cat10;
    cat10.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT(cat10.subCat1() == "* FARE CLASS F.\n");
  }

  void testFareDisplayFamily()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxCodeReg taxCodeReg;
    TaxDisplayItem taxDisplayItem;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;

    taxCodeReg.fareclassExclInd() = tse::Indicator('N');
    taxCodeReg.restrictionFareClass().push_back("F-");

    Category10 cat10;
    cat10.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT(cat10.subCat1() == "* FARE CLASS/FAMILY F-.\n");

    taxCodeReg.fareclassExclInd() = tse::Indicator('Y');
    cat10.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT(cat10.subCat1() == "* EXCEPT FARE CLASS/FAMILY F-.\n");

    taxCodeReg.restrictionFareClass().push_back("C-");
    taxCodeReg.restrictionFareClass().push_back("FOW");
    cat10.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT(cat10.subCat1() == "* EXCEPT FARE CLASS/FAMILY F-, C-, FOW.\n");

    taxCodeReg.restrictionFareClass()[0] = "F";
    cat10.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT(cat10.subCat1() == "* EXCEPT FARE CLASS F, C-, FOW.\n");
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT10Test);
}
