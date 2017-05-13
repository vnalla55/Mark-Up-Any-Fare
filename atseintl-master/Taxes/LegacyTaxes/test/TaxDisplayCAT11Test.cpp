#include "Common/TseCodeTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxRequest.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/Category11.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <unistd.h>

#include <string>
#include <time.h>
#include <iostream>
#include <cstdio>
#include <vector>

using namespace std;

namespace tse
{

struct transitRestriction
{
  int hours;
  int minutes;
  char sameDay;
  char nextDay;
  char sameFlight;
  char transitOnly;
  int arrivalHours;
  int departureHours;
  int arrivalMinutes;
  int departureMinutes;
};

struct transitRestriction testDataCAT11[] = { { 12, 0, 'N', 'N', 'N', 'N', 0, 0, 0, 0 },
                                              { 12, 20, 'Y', 'N', 'N', 'N', 0, 0, 0, 0 },
                                              { 12, 0, 'N', 'Y', 'N', 'N', 0, 0, 0, 0 },
                                              { 12, 20, 'N', 'N', 'Y', 'N', 0, 0, 0, 0 },
                                              { 12, 0, 'N', 'N', 'N', 'N', 0, 0, 0, 0 },
                                              { 12, 20, 'N', 'N', 'N', 'N', 0, 5, 10, 5 },
                                              { 0, 0, 'N', 'N', 'N', 'N', 4, 0, 5, 5 },
                                              { 12, 20, 'N', 'N', 'N', 'N', 7, 9, 2, 25 },
                                              { 12, 0, 'N', 'N', 'N', 'Y', 0, 0, 0, 0 },
                                              { 12, 20, 'N', 'N', 'N', 'Y', 10, 5, 0, 0 },
                                              { 0, 0, 'N', 'N', 'N', 'Y', 5, 5, 0, 0 },
                                              { 12, 20, 'N', 'N', 'N', 'Y', 2, 25, 0, 0 },
                                              { 0, 0, 'N', 'N', 'N', 'N', 0, 0, 0, 0 } };

std::string testResultsCat11[] = { "* EXCEPT WHEN TRANSITING US WITHIN 12 HOURS.\n",
                                   "* EXCEPT WHEN TRANSITING US WITHIN 12 HOURS, THE SAME DAY.\n",
                                   "* EXCEPT WHEN TRANSITING US WITHIN 12 HOURS, THE NEXT DAY.\n",
                                   "* EXCEPT WHEN TRANSITING US WITHIN 12 HOURS.\n",
                                   "* EXCEPT WHEN TRANSITING US WITHIN 12 HOURS.\n",
                                   "* EXCEPT WHEN TRANSITING US WITHIN 12 HOURS.\n",
                                   "* EXCEPT WHEN TRANSITING US, ON FLIGHT ARRIVALS ON/AFTER 4:05 "
                                   "ON FLIGHT DEPARTIRES ON/AFTER 0:05.\n",
                                   "* EXCEPT WHEN TRANSITING US, ON FLIGHT ARRIVALS ON/AFTER 7:02 "
                                   "ON FLIGHT DEPARTIRES ON/AFTER 9:25 WITHIN 12 HOURS.\n",
                                   "* APPLIES WHEN TRANSITING US WITHIN 12 HOURS.\n",
                                   "* APPLIES WHEN TRANSITING US, ON FLIGHT ARRIVALS ON/AFTER "
                                   "10:00 ON FLIGHT DEPARTIRES ON/AFTER 5:00 WITHIN 12 HOURS.\n",
                                   "* APPLIES WHEN TRANSITING US, ON FLIGHT ARRIVALS ON/AFTER 5:00 "
                                   "ON FLIGHT DEPARTIRES ON/AFTER 5:00.\n",
                                   "* APPLIES WHEN TRANSITING US, ON FLIGHT ARRIVALS ON/AFTER 2:00 "
                                   "ON FLIGHT DEPARTIRES ON/AFTER 25:00 WITHIN 12 HOURS.\n",
                                   "* EXCEPT WHEN TRANSITING US.\n" };

class TaxDisplayCAT11Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT11Test);
  CPPUNIT_TEST(testBuildCAT11);
  CPPUNIT_TEST_SUITE_END();

public:
  void testBuildCAT11()
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
    taxDisplayItem.taxNation() = "US";
    Category11 category11;

    for (size_t i = 0; i < sizeof(testDataCAT11) / sizeof(transitRestriction); ++i)
    {
      transitRestriction& restriction(testDataCAT11[i]);

      TaxRestrictionTransit* taxRestrictionTransit = _memHandle.create<TaxRestrictionTransit>();
      taxRestrictionTransit->transitHours() = restriction.hours;
      taxRestrictionTransit->transitMinutes() = restriction.minutes;
      taxRestrictionTransit->sameDayInd() = restriction.sameDay;
      taxRestrictionTransit->nextDayInd() = restriction.nextDay;
      taxRestrictionTransit->transitTaxonly() = restriction.transitOnly;
      taxRestrictionTransit->flightArrivalHours() = restriction.arrivalHours;
      taxRestrictionTransit->flightDepartHours() = restriction.departureHours;
      taxRestrictionTransit->flightArrivalMinutes() = restriction.arrivalMinutes;
      taxRestrictionTransit->flightDepartMinutes() = restriction.departureMinutes;
      taxCodeReg.restrictionTransit().push_back(*taxRestrictionTransit);

      category11.build(trx, taxDisplayItem);

      std::string result(category11.subCat1());
      CPPUNIT_ASSERT_EQUAL(testResultsCat11[i], result);

      taxCodeReg.restrictionTransit().clear();
    }
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT11Test);
}
