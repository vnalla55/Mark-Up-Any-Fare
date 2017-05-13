#include "Common/TseCodeTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxRequest.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/Category7.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <time.h>
#include <vector>

using namespace std;

namespace tse
{

struct exceptionCxr
{
  std::string carrier;
  int flight1;
  int flight2;
  char direction;
  std::string airport1;
  std::string airport2;
};

struct exceptionCxr testDataCAT7[] = { { "AA", 0, 0, ' ', "", "" },
                                       { "BA", 0, 0, ' ', "", "" },
                                       { "AA", 100, 0, 'F', "DFW", "MIA" },
                                       { "AA", 500, 700, 'F', "LHR", "MUC" },
                                       { "AA", 100, 0, 'B', "BOS", "MIA" },
                                       { "AA", 500, 700, 'B', "MIA", "HNL" },
                                       { "AA", 350, 0, ' ', "", "" },
                                       { "LH", 220, 440, ' ', "", "" },
                                       { "LH", 670, 880, ' ', "", "" },
                                       { "DL", 100, 0, 'F', "DFW", "MIA" },
                                       { "DL", 500, 700, 'F', "LHR", "MUC" },
                                       { "DL", 100, 0, 'B', "BOS", "MIA" },
                                       { "DL", 500, 700, 'B', "MIA", "HNL" },
                                       { "CO", 500, 700, 'F', "LHR", "MUC" },
                                       { "NW", 100, 0, 'B', "BOS", "MIA" } };

class TaxDisplayCAT7Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT7Test);
  CPPUNIT_TEST(testBuildCAT7);
  CPPUNIT_TEST_SUITE_END();

public:
  void testBuildCAT7()
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
    Category7 category7;

    for (size_t i = 0; i < sizeof(testDataCAT7) / sizeof(exceptionCxr); ++i)
    {
      TaxExemptionCarrier* taxExemptionCarrier = _memHandle.create<TaxExemptionCarrier>();

      exceptionCxr& data(testDataCAT7[i]);
      taxExemptionCarrier->carrier() = data.carrier;
      taxExemptionCarrier->direction() = data.direction;
      taxExemptionCarrier->flight1() = data.flight1;
      taxExemptionCarrier->flight2() = data.flight2;
      taxExemptionCarrier->airport1() = data.airport1;
      taxExemptionCarrier->airport2() = data.airport2;
      taxCodeReg.exemptionCxr().push_back(*taxExemptionCarrier);
    }

    category7.build(trx, taxDisplayItem);

    CPPUNIT_ASSERT_EQUAL(std::string("* APPLIES ON CARRIER/S -\n  AA, BA.\n"), category7.subCat1());
    CPPUNIT_ASSERT_EQUAL(std::string("AA-BETWEEN- BOS AND MIA ON FLIGHT 100,\n"
                                     "            MIA AND HNL ON FLIGHT 500 TO 700.\n"
                                     "  -FROM   - DFW TO MIA ON FLIGHT 100,\n"
                                     "            LHR TO MUC ON FLIGHT 500 TO 700.\n"
                                     " - ON FLIGHT 350.\n"
                                     "LH- ON FLIGHT 220 TO 440, ON FLIGHT 670 TO 880.\n"
                                     "DL-BETWEEN- BOS AND MIA ON FLIGHT 100,\n"
                                     "            MIA AND HNL ON FLIGHT 500 TO 700.\n"
                                     "  -FROM   - DFW TO MIA ON FLIGHT 100,\n"
                                     "            LHR TO MUC ON FLIGHT 500 TO 700.\n"
                                     "CO-FROM-    LHR TO MUC ON FLIGHT 500 TO 700.\n"
                                     "NW-BETWEEN- BOS AND MIA ON FLIGHT 100.\n"),
                         category7.subCat2());
    CPPUNIT_ASSERT_EQUAL(EMPTY_STRING(), category7.subCat3());
    CPPUNIT_ASSERT_EQUAL(EMPTY_STRING(), category7.subCat4());
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT7Test);
}
