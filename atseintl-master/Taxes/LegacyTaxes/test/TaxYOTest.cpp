#include <string>
#include <time.h>
#include <iostream>

#include "Taxes/LegacyTaxes/TaxGB.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/PricingTrx.h"
#include "Common/DateTime.h"
#include "DBAccess/TaxCodeCabin.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;

namespace tse
{

class TaxYOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxYOTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxYO_TransitSpecial);
  CPPUNIT_TEST(testTaxYO_Transit);
  CPPUNIT_TEST(testTaxYO_RestrictionTransit);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor() { CPPUNIT_ASSERT_NO_THROW(TaxGB taxGB); }

  void testTaxYO_TransitSpecial()
  {
    // Set up the travel segment

    Loc* loc1 = _memHandle.create<Loc>();
    loc1->loc() = string("IOM");
    loc1->nation() = string("GB");

    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLGW.xml");

    Loc* loc3 = _memHandle.create<Loc>();
    loc3->loc() = string("KIN");
    loc3->nation() = string("JM");

    AirSeg* travelSeg1 = createAirSeg(1, loc1, loc2, "BA", 'N', (DateTime)172800, (DateTime)86400);

    AirSeg* travelSeg2 = createAirSeg(1, loc2, loc3, "BA", 'V', (DateTime)345600, (DateTime)259200);

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg1);
    itin->travelSeg().push_back(travelSeg2);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse.farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    char chardummy = ' ';

    TaxCodeReg taxCodeReg;
    TaxRestrictionTransit restrictionTransit;
    restrictionTransit.transitHours() = 24;
    restrictionTransit.transitMinutes() = -1;
    restrictionTransit.sameDayInd() = chardummy;
    restrictionTransit.nextDayInd() = chardummy;
    restrictionTransit.flightArrivalHours() = 0;
    restrictionTransit.flightArrivalMinutes() = 0;
    restrictionTransit.flightDepartHours() = 0;
    restrictionTransit.flightDepartMinutes() = 0;
    taxCodeReg.restrictionTransit().push_back(restrictionTransit);

    uint16_t travelSegIndex = 0;
    TaxGB taxGB;

    CPPUNIT_ASSERT(taxGB.validateTransit(_trx, _taxResponse, taxCodeReg, travelSegIndex));
  }

  void testTaxYO_Transit()
  {
    // Set up the travel segment
    Loc* loc1 = _memHandle.create<Loc>();
    loc1->loc() = string("IOM");
    loc1->nation() = string("GB");
    loc1->subarea() = "GB";
    loc1->cityInd() = true;
    loc1->area() = IATA_AREA2;

    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLGW.xml");
    Loc* loc3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCDG.xml");

    AirSeg* travelSeg1 = createAirSeg(1, loc1, loc2, "BA", 'N', (DateTime)172800, (DateTime)86400);

    AirSeg* travelSeg2 = createAirSeg(1, loc2, loc3, "BA", 'V', (DateTime)345600, (DateTime)259200);

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg1);
    itin->travelSeg().push_back(travelSeg2);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse.farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    TaxCodeReg taxCodeReg;
    taxCodeReg.specialProcessNo() = 38;
    taxCodeReg.expireDate() = DateTime(2004, 12, 31, 12, 0, 0);
    taxCodeReg.loc1Type() = tse::LocType('C');
    taxCodeReg.loc2Type() = tse::LocType('Z');
    taxCodeReg.loc1() = tse::LocCode("IOM");
    taxCodeReg.loc1ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    taxCodeReg.loc2() = tse::LocCode("0332");
    taxCodeReg.loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    taxCodeReg.taxCode() = std::string("YO1");
    taxCodeReg.effDate() = DateTime(2004, 7, 13, 12, 0, 0);
    taxCodeReg.discDate() = DateTime(2004, 12, 31, 12, 0, 0);
    ;
    taxCodeReg.firstTvlDate() = DateTime(2004, 7, 14, 12, 0, 0);
    taxCodeReg.lastTvlDate() = DateTime(2004, 12, 31, 23, 59, 59);
    taxCodeReg.nation() = std::string("GB");
    taxCodeReg.taxfullFareInd() = tse::Indicator('N');
    taxCodeReg.taxequivAmtInd() = tse::Indicator('N');
    taxCodeReg.taxexcessbagInd() = tse::Indicator('N');
    taxCodeReg.tvlDateasoriginInd() = tse::Indicator('N');
    taxCodeReg.displayonlyInd() = tse::Indicator('N');
    taxCodeReg.feeInd() = tse::Indicator('N');
    taxCodeReg.interlinableTaxInd() = tse::Indicator('Y');
    taxCodeReg.showseparateInd() = tse::Indicator('N');

    taxCodeReg.posExclInd() = tse::Indicator('N');
    taxCodeReg.posLocType() = tse::LocType(' ');
    taxCodeReg.posLoc() = tse::LocCode("");

    taxCodeReg.poiExclInd() = tse::Indicator('N');
    taxCodeReg.poiLocType() = tse::LocType(' ');
    taxCodeReg.poiLoc() = tse::LocCode("");

    taxCodeReg.sellCurExclInd() = tse::Indicator('N');
    taxCodeReg.sellCur() = tse::CurrencyCode("");

    taxCodeReg.occurrence() = tse::Indicator(' ');
    taxCodeReg.freeTktexempt() = tse::Indicator('N');
    taxCodeReg.idTvlexempt() = tse::Indicator('N');

    taxCodeReg.taxCur() = tse::CurrencyCode("GBP");
    taxCodeReg.taxAmt() = 100000;
    taxCodeReg.taxType() = 'F';

    taxCodeReg.fareclassExclInd() = tse::Indicator('N');
    taxCodeReg.tktdsgExclInd() = tse::Indicator('N');
    taxCodeReg.valcxrExclInd() = tse::Indicator('N');

    taxCodeReg.exempequipExclInd() = tse::Indicator('Y');
    taxCodeReg.psgrExclInd() = tse::Indicator('Y');
    taxCodeReg.fareTypeExclInd() = tse::Indicator('N');

    taxCodeReg.originLocExclInd() = tse::Indicator('N');
    taxCodeReg.originLocType() = tse::LocType(' ');
    taxCodeReg.originLoc() = tse::LocCode("");
    taxCodeReg.loc1Appl() = tse::Indicator('E');
    taxCodeReg.loc2Appl() = tse::Indicator(' ');
    taxCodeReg.tripType() = tse::Indicator('F');
    taxCodeReg.travelType() = tse::Indicator(' ');
    taxCodeReg.itineraryType() = tse::Indicator(' ');
    taxCodeReg.formOfPayment() = tse::Indicator('A');
    taxCodeReg.taxOnTaxExcl() = tse::Indicator('N');

    uint16_t startIndex = 0;
    uint16_t endIndex = 1;
    TaxGB taxGB;

    CPPUNIT_ASSERT(taxGB.validateTripTypes(_trx, _taxResponse, taxCodeReg, startIndex, endIndex));
  }

  void testTaxYO_RestrictionTransit()
  {
    // Set up the travel segment

    Loc* loc1 = _memHandle.create<Loc>();
    loc1->loc() = string("IOM");
    loc1->nation() = string("GB");
    loc1->subarea() = "GB";
    loc1->cityInd() = true;
    loc1->area() = IATA_AREA2;

    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCDG.xml");

    AirSeg* travelSeg1 = createAirSeg(1, loc1, loc2, "BA", 'N', (DateTime)172800, (DateTime)86400);

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg1);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    _taxResponse.farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    TaxCodeReg taxCodeReg;
    taxCodeReg.specialProcessNo() = 38;
    taxCodeReg.expireDate() = DateTime(2004, 12, 31, 12, 0, 0);
    taxCodeReg.loc1Type() = tse::LocType('C');
    taxCodeReg.loc2Type() = tse::LocType('Z');
    taxCodeReg.loc1() = tse::LocCode("IOM");
    taxCodeReg.loc1ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    taxCodeReg.loc2() = tse::LocCode("0332");
    taxCodeReg.loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    taxCodeReg.taxCode() = std::string("YO1");
    taxCodeReg.effDate() = DateTime(2004, 7, 13, 12, 0, 0);
    taxCodeReg.discDate() = DateTime(2004, 12, 31, 12, 0, 0);
    ;
    taxCodeReg.firstTvlDate() = DateTime(2004, 7, 14, 12, 0, 0);
    taxCodeReg.lastTvlDate() = DateTime(2004, 12, 31, 23, 59, 59);
    taxCodeReg.nation() = std::string("GB");
    taxCodeReg.taxfullFareInd() = tse::Indicator('N');
    taxCodeReg.taxequivAmtInd() = tse::Indicator('N');
    taxCodeReg.taxexcessbagInd() = tse::Indicator('N');
    taxCodeReg.tvlDateasoriginInd() = tse::Indicator('N');
    taxCodeReg.displayonlyInd() = tse::Indicator('N');
    taxCodeReg.feeInd() = tse::Indicator('N');
    taxCodeReg.interlinableTaxInd() = tse::Indicator('Y');
    taxCodeReg.showseparateInd() = tse::Indicator('N');

    taxCodeReg.posExclInd() = tse::Indicator('N');
    taxCodeReg.posLocType() = tse::LocType(' ');
    taxCodeReg.posLoc() = tse::LocCode("");

    taxCodeReg.poiExclInd() = tse::Indicator('N');
    taxCodeReg.poiLocType() = tse::LocType(' ');
    taxCodeReg.poiLoc() = tse::LocCode("");

    taxCodeReg.sellCurExclInd() = tse::Indicator('N');
    taxCodeReg.sellCur() = tse::CurrencyCode("");

    taxCodeReg.occurrence() = tse::Indicator(' ');
    taxCodeReg.freeTktexempt() = tse::Indicator('N');
    taxCodeReg.idTvlexempt() = tse::Indicator('N');

    taxCodeReg.taxCur() = tse::CurrencyCode("GBP");
    taxCodeReg.taxAmt() = 100000;
    taxCodeReg.taxType() = 'F';

    taxCodeReg.fareclassExclInd() = tse::Indicator('N');
    taxCodeReg.tktdsgExclInd() = tse::Indicator('N');
    taxCodeReg.valcxrExclInd() = tse::Indicator('N');

    taxCodeReg.exempequipExclInd() = tse::Indicator('Y');
    taxCodeReg.psgrExclInd() = tse::Indicator('Y');
    taxCodeReg.fareTypeExclInd() = tse::Indicator('N');

    taxCodeReg.originLocExclInd() = tse::Indicator('N');
    taxCodeReg.originLocType() = tse::LocType(' ');
    taxCodeReg.originLoc() = tse::LocCode("");
    taxCodeReg.loc1Appl() = tse::Indicator('E');
    taxCodeReg.loc2Appl() = tse::Indicator(' ');
    taxCodeReg.tripType() = tse::Indicator('F');
    taxCodeReg.travelType() = tse::Indicator(' ');
    taxCodeReg.itineraryType() = tse::Indicator(' ');
    taxCodeReg.formOfPayment() = tse::Indicator('A');
    taxCodeReg.taxOnTaxExcl() = tse::Indicator('N');

    char chardummy = ' ';

    TaxRestrictionTransit restrictionTransit;
    restrictionTransit.transitHours() = 24;
    restrictionTransit.transitMinutes() = -1;
    restrictionTransit.sameDayInd() = chardummy;
    restrictionTransit.nextDayInd() = chardummy;
    restrictionTransit.flightArrivalHours() = 0;
    restrictionTransit.flightArrivalMinutes() = 0;
    restrictionTransit.flightDepartHours() = 0;
    restrictionTransit.flightDepartMinutes() = 0;
    taxCodeReg.restrictionTransit().push_back(restrictionTransit);

    uint16_t startIndex = 0;
    uint16_t endIndex = 1;
    TaxGB taxGB;

    CPPUNIT_ASSERT(taxGB.validateTripTypes(_trx, _taxResponse, taxCodeReg, startIndex, endIndex));
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    // Set up the PricingTrx
    _trx.setRequest(&_request);
    _request.ticketingAgent() = &_agent;

    _agent.agentLocation() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
  }

  void tearDown() { _memHandle.clear(); }

  AirSeg* createAirSeg(int segment,
                       const Loc* origin,
                       const Loc* destination,
                       const CarrierCode& carrier,
                       const char bookingCode,
                       DateTime departureDate,
                       DateTime arrivalDate)
  {
    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->origin() = origin;
    travelSeg->destination() = destination;
    travelSeg->carrier() = carrier;
    travelSeg->setBookingCode(BookingCode(bookingCode));
    travelSeg->departureDT() = departureDate;
    travelSeg->arrivalDT() = arrivalDate;
    return travelSeg;
  }

private:
  PricingTrx _trx;
  PricingRequest _request;
  Agent _agent;
  TaxResponse _taxResponse;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxYOTest);
}
