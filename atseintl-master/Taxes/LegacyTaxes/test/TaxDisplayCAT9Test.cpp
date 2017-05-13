#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxRequest.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/Category9.h"
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
struct psg
{
  char fareZeroOnly;
  char showPsg;
  uint16_t minAge;
  uint16_t maxAge;
  std::string psgType;
};

struct psgRestriction
{
  char psgrExclInd;
  struct psg* psgs;
  std::string referenceValue;
};

struct psg testPsgData01[] = { { 'N', 'Y', 0, 0, "INF" }, { 0, 0, 0, 0, "" } };

struct psg testPsgData02[] = { { 'N', 'Y', 0, 0, "ADT" },
                               { 'N', 'Y', 0, 0, "INF" },
                               { 0, 0, 0, 0, "" } };

struct psg testPsgData03[] = { { 'N', 'Y', 0, 0, "ADT" },
                               { 'N', 'Y', 0, 5, "CNN" },
                               { 0, 0, 0, 0, "" } };
struct psg testPsgData04[] = { { 'N', 'Y', 0, 0, "ADT" },
                               { 'N', 'Y', 11, 0, "CNN" },
                               { 0, 0, 0, 0, "" } };
struct psg testPsgData05[] = { { 'Y', 'Y', 0, 0, "ADT" },
                               { 'Y', 'Y', 11, 0, "CNN" },
                               { 0, 0, 0, 0, "" } };
struct psg testPsgData06[] = { { 'Y', 'Y', 0, 0, "ADT" },
                               { 'Y', 'N', 11, 0, "CNN" },
                               { 'Y', 'Y', 5, 11, "CBE" },
                               { 0, 0, 0, 0, "" } };
struct psg testPsgData07[] = { { 'Y', 'Y', 0, 0, "ADT" },
                               { 'N', 'Y', 11, 0, "CNN" },
                               { 'Y', 'Y', 5, 11, "CBE" },
                               { 0, 0, 0, 0, "" } };
struct psg testPsgData08[] = { { 'N', 'Y', 0, 0, "ADT" },
                               { 'N', 'Y', 0, 0, "CNN" },
                               { 'N', 'Y', 0, 0, "CBE" },
                               { 0, 0, 0, 0, "" } };

struct psgRestriction testDataCat9[] = {
  { 'Y', testPsgData01, "* EXCEPT PASSENGER TYPE INF.\n" },
  { 'Y', testPsgData02, "* EXCEPT PASSENGER TYPE ADT, INF.\n" },
  { 'Y', testPsgData03, "* EXCEPT PASSENGER TYPE CNN AGES 0 TO 5, ADT.\n" },
  { 'Y', testPsgData04, "* EXCEPT PASSENGER TYPE CNN AGES 11 OR OLDER, ADT.\n" },
  { 'Y', testPsgData05, "* EXCEPT PASSENGER TYPE ADT WHEN FARE IS ZERO -0-,"
                        " CNN AGES 11 OR OLDER WHEN FARE IS ZERO -0-.\n" },
  { 'Y', testPsgData06, "* EXCEPT PASSENGER TYPE ADT WHEN FARE IS ZERO -0-,"
                        " CBE AGES 5 TO 11 WHEN FARE IS ZERO -0-.\n" },
  { 'Y', testPsgData07, "* EXCEPT PASSENGER TYPE ADT WHEN FARE IS ZERO -0-,"
                        " CNN AGES 11 OR OLDER,"
                        " CBE AGES 5 TO 11 WHEN FARE IS ZERO -0-.\n" },
  { 'N', testPsgData08, "* PASSENGER TYPE ADT, CNN, CBE.\n" }
};

class TaxDisplayCAT9Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT9Test);
  CPPUNIT_TEST(testBuildCAT9);
  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }

  void testBuildCAT9()
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
    Category9 category9;

    for (size_t i = 0; i < sizeof(testDataCat9) / sizeof(psgRestriction); ++i)
    {
      psgRestriction& data(testDataCat9[i]);
      taxCodeReg.psgrExclInd() = tse::Indicator(data.psgrExclInd);

      taxCodeReg.restrictionPsg().clear();
      category9.subCat1().clear();
      for (struct psg* p_psg = data.psgs; p_psg->fareZeroOnly != 0; p_psg++)
      {
        TaxRestrictionPsg* taxRestrictionPsg = _memHandle.create<TaxRestrictionPsg>();

        taxRestrictionPsg->psgType() = PaxTypeCode(p_psg->psgType);
        taxRestrictionPsg->showPsg() = p_psg->showPsg;
        taxRestrictionPsg->fareZeroOnly() = p_psg->fareZeroOnly;
        taxRestrictionPsg->minAge() = p_psg->minAge;
        taxRestrictionPsg->maxAge() = p_psg->maxAge;

        taxCodeReg.restrictionPsg().push_back(*taxRestrictionPsg);
      }

      category9.build(trx, taxDisplayItem);
      CPPUNIT_ASSERT_EQUAL(data.referenceValue, category9.subCat1());
    }
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT9Test);
}
