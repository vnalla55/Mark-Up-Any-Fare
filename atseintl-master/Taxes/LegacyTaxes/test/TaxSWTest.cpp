#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Taxes/LegacyTaxes/TaxSW.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/Loc.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Diagnostic/Diag804Collector.h"

#include "DataModel/PricingRequest.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareUsage.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxType.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxRestrictionTransit.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "DataModel/Response.h"
#include "DataModel/Itin.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Trx.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Server/TseServer.h"
#include "Common/TseEnums.h"
#include "DataModel/PaxType.h"
#include "Common/Vendor.h"
#include "Common/DateTime.h"
#include "Common/TseUtil.h"
#include "Taxes/LegacyTaxes/TaxSW.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class TaxSWTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxSWTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxSWCase0);
  CPPUNIT_TEST(testTaxSWFromToCase1);
  CPPUNIT_TEST(testTaxSWFromToCase2);
  CPPUNIT_TEST(validTransitIndicatorsIfMultiCity1);
  CPPUNIT_TEST(validTransitIndicatorsIfMultiCity2);
  CPPUNIT_TEST(validTransitIndicatorsIfMultiCity3);
  CPPUNIT_TEST(validTransitIndicatorsIfMultiCity4);
  CPPUNIT_TEST_SUITE_END();

public:
  /**
   * Test the constructor.
   */
  void testConstructor();
  void testTaxSWCase0();
  void testTaxSWFromToCase1();
  void testTaxSWFromToCase2();
  void validTransitIndicatorsIfMultiCity1();
  void validTransitIndicatorsIfMultiCity2();
  void validTransitIndicatorsIfMultiCity3();
  void validTransitIndicatorsIfMultiCity4();

  void setUp();
  void tearDown();

  TravelSeg* _travelSegTo;
  TravelSeg* _travelSegFrom;
  TransitValidatorSW _transitValidator;
  TestMemHandle _memHandle;
};
}

using namespace std;
using namespace tse;

CPPUNIT_TEST_SUITE_REGISTRATION(TaxSWTest);

void
TaxSWTest::setUp()
{
  _memHandle.create<TestConfigInitializer>();
  _travelSegTo = new AirSeg;
  _travelSegFrom = new AirSeg;
}

void
TaxSWTest::tearDown()
{
  delete _travelSegTo;
  delete _travelSegFrom;
  _memHandle.clear();
}

void
TaxSWTest::testConstructor()
{
  try { TaxSW taxSW; }
  catch (...)
  {
    // If any error occured at all, then fail.
    CPPUNIT_ASSERT(false);
  }
}

/**
 * Case0 is the default case where if the loc[1|2]Types are spaces,
 * then we return true.
 **/
void
TaxSWTest::testTaxSWCase0()
{
  tse::PricingTrx trx;
  PricingRequest prx;
  trx.setRequest(&prx);
  tse::TaxResponse taxResponse;
  Diagnostic* diagroot = new Diagnostic(LegacyTaxDiagnostic24);
  Diag804Collector diag(*diagroot);

  taxResponse.diagCollector() = &diag;

  tse::TaxCodeReg taxCodeReg;
  uint16_t startIndex = 0;
  uint16_t endIndex = 0;

  // These values are probably defaults, but let's make sure
  taxCodeReg.loc1Type() = tse::LocType(' ');
  taxCodeReg.loc2Type() = tse::LocType(' ');
  taxCodeReg.tripType() = ' ';

  Loc origin;
  origin.loc() = string("NGO");
  origin.nation() = string("JP");

  Loc destination;
  destination.loc() = string("NRT");
  destination.nation() = string("JP");

  AirSeg ts;
  ts.origin() = &origin;
  ts.destination() = &destination;

  Itin itin;
  itin.travelSeg().push_back(&ts);

  FarePath fp;

  taxResponse.farePath() = &fp;
  taxResponse.farePath()->itin() = &itin;

  PricingUnit pricingUnit;
  FareUsage fareUsage;
  Fare fare;
  PaxTypeFare paxTypeFare;

  FareMarket fareMarket;
  fareMarket.travelSeg().push_back(&ts);

  fare.nucFareAmount() = 1000.00;

  FareInfo fareInfo;
  fareInfo._fareAmount = 1000.00;
  fareInfo._currency = "USD";

  TariffCrossRefInfo tariffRefInfo;
  tariffRefInfo._fareTariffCode = "TAFPBA";

  fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

  fareUsage.paxTypeFare() = &paxTypeFare;
  fareUsage.paxTypeFare()->setFare(&fare);
  fareUsage.totalFareAmount();

  fareUsage.travelSeg().push_back(&ts);

  taxResponse.farePath()->pricingUnit().push_back(&pricingUnit);
  taxResponse.farePath()->pricingUnit()[0]->fareUsage().push_back(&fareUsage);

  TaxSW taxSW;

  bool result = taxSW.validateTripTypes(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  CPPUNIT_ASSERT(result == true);
}

/**
 * Case1  tests TAX_ORIGIN behavior.
 *  geoMatch is false, but, the loc1ExclInd is yes, so the validation is true.
 **/
void
TaxSWTest::testTaxSWFromToCase1()
{
  TaxSW taxSW;

  tse::PricingTrx trx;

  PricingRequest prx;
  trx.setRequest(&prx);

  uint16_t startIndex = 0;
  uint16_t endIndex = 0;

  Loc origin;
  origin.loc() = string("NGO");
  origin.nation() = string("JP");

  Loc destination;
  destination.loc() = string("NRT");
  destination.nation() = string("JP");

  AirSeg ts;
  ts.origin() = &origin;
  ts.destination() = &destination;

  Itin itin;
  itin.travelSeg().push_back(&ts);

  FarePath fp;
  fp.itin() = &itin; // Set the itinerary in the fare path.

  tse::TaxResponse taxResponse;
  Diagnostic* diagroot = new Diagnostic(LegacyTaxDiagnostic24);
  Diag804Collector diag(*diagroot);

  taxResponse.diagCollector() = &diag;

  taxResponse.farePath() = &fp;

  tse::TaxCodeReg taxCodeReg;
  taxCodeReg.loc1Type() = tse::LocType('N');
  taxCodeReg.loc2Type() = tse::LocType('N');
  taxCodeReg.loc1() = tse::LocCode("JP");

  taxCodeReg.loc1ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);

  taxCodeReg.loc2() = tse::LocCode("JP");
  taxCodeReg.loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);

  taxCodeReg.tripType() = 'F';

  bool result = taxSW.validateTripTypes(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  CPPUNIT_ASSERT(result == true);

  taxCodeReg.loc2ExclInd() = tse::Indicator('Y' /* TAX_EXCLUDE */);

  result = taxSW.validateTripTypes(trx, taxResponse, taxCodeReg, startIndex, endIndex);

  CPPUNIT_ASSERT(result == false);
}

void
TaxSWTest::testTaxSWFromToCase2()
{
  TaxSW taxSW;

  tse::PricingTrx trx;

  PricingRequest prx;
  trx.setRequest(&prx);

  uint16_t startIndex = 0;
  uint16_t endIndex = 1;

  Loc origin;
  origin.loc() = string("NGO");
  origin.nation() = string("JP");

  Loc destination;
  destination.loc() = string("NRT");
  destination.nation() = string("JP");

  Loc origin2;
  origin2.loc() = string("NRT");
  origin2.nation() = string("JP");

  Loc destination2;
  destination2.loc() = string("LAX");
  destination2.nation() = string("US");

  AirSeg ts1;
  ts1.origin() = &origin;
  ts1.destination() = &destination;

  AirSeg ts2;
  ts2.origin() = &origin2;
  ts2.destination() = &destination2;

  Itin itin;
  itin.travelSeg().push_back(&ts1);
  itin.travelSeg().push_back(&ts2);

  FarePath fp;
  fp.itin() = &itin; // Set the itinerary in the fare path.

  tse::TaxResponse taxResponse;
  Diagnostic* diagroot = new Diagnostic(LegacyTaxDiagnostic24);

  Diag804Collector diag(*diagroot);

  taxResponse.diagCollector() = &diag;

  taxResponse.farePath() = &fp;

  tse::TaxCodeReg taxCodeReg;
  taxCodeReg.loc1Type() = tse::LocType('M');
  taxCodeReg.loc2Type() = tse::LocType('N');
  taxCodeReg.loc1() = tse::LocCode("NRT");

  taxCodeReg.loc1ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);

  taxCodeReg.loc2() = tse::LocCode("US");
  taxCodeReg.loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);

  taxCodeReg.tripType() = 'F';

  bool result = taxSW.validateTripTypes(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  CPPUNIT_ASSERT(result == false);
}

void
TaxSWTest::validTransitIndicatorsIfMultiCity1()
{
  _travelSegTo->origAirport() = "AAA";
  _travelSegFrom->destAirport() = "AAA";
  _travelSegTo->boardMultiCity() = "JFK";
  _travelSegFrom->offMultiCity() = "JFK";

  CPPUNIT_ASSERT(
      !_transitValidator.validTransitIndicatorsIfMultiCity(_travelSegTo, _travelSegFrom));
}

void
TaxSWTest::validTransitIndicatorsIfMultiCity2()
{
  _travelSegTo->origAirport() = "AAA";
  _travelSegFrom->destAirport() = "BBB";
  _travelSegTo->boardMultiCity() = "JFK";
  _travelSegFrom->offMultiCity() = "JFK";

  CPPUNIT_ASSERT(_transitValidator.validTransitIndicatorsIfMultiCity(_travelSegTo, _travelSegFrom));
}

void
TaxSWTest::validTransitIndicatorsIfMultiCity3()
{
  _travelSegTo->origAirport() = "AAA";
  _travelSegFrom->destAirport() = "AAA";
  _travelSegTo->boardMultiCity() = "JFK";
  _travelSegFrom->offMultiCity() = "CCC";

  CPPUNIT_ASSERT(
      !_transitValidator.validTransitIndicatorsIfMultiCity(_travelSegTo, _travelSegFrom));
}

void
TaxSWTest::validTransitIndicatorsIfMultiCity4()
{
  _travelSegTo->origAirport() = "AAA";
  _travelSegFrom->destAirport() = "BBB";
  _travelSegTo->boardMultiCity() = "JFK";
  _travelSegFrom->offMultiCity() = "CCC";

  CPPUNIT_ASSERT(
      !_transitValidator.validTransitIndicatorsIfMultiCity(_travelSegTo, _travelSegFrom));
}
