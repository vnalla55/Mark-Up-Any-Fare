#include "Common/TseCodeTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxRequest.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxReissue.h"
#include "Taxes/LegacyTaxes/Reissue.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"

#include "test/include/CppUnitHelperMacros.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <time.h>
#include <unistd.h>
#include <vector>

using namespace std;

namespace tse
{
class TaxDisplayReissueTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayReissueTest);
  CPPUNIT_TEST(testBuildReissue);
  CPPUNIT_TEST_SUITE_END();

public:
  void testBuildReissue()
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

    taxCodeReg.taxCode() = "US2";

    TaxDisplayItem taxDisplayItem;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;

    Reissue treissue;
    TaxReissue taxreissue;
    taxDisplayItem.taxReissue() = &taxreissue;

    treissue.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("     TAX IS NON REFUNDABLE.\n"), treissue.subCat1());
    CPPUNIT_ASSERT_EQUAL(std::string("     NO SALE-REISSUE RESTRICTION.\n"), treissue.subCat2());
    // CPPUNIT_ASSERT_EQUAL(std::string("* TICKETING REISSUE CARRIER/S AA.\n"),treissue.subCat3());
    // CPPUNIT_ASSERT_EQUAL(std::string(""),treissue.subCat4());

    // taxCodeReg.taxCode()="CD2";
    // treissue.build(trx,taxDisplayItem);
    // CPPUNIT_ASSERT_EQUAL(std::string("* TAX IS REFUNDABLE.\n"),treissue.subCat1());
    /*CPPUNIT_ASSERT_EQUAL(std::string("* DOES NOT APPLY FOR REISSUES IN .\n"),treissue.subCat2());
    CPPUNIT_ASSERT_EQUAL(std::string("* TICKETING REISSUE CARRIER/S AA.\n"),treissue.subCat3());*/
    // CPPUNIT_ASSERT_EQUAL(std::string(""),treissue.subCat4()); no idea why here returns -0
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayReissueTest);
}
