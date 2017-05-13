#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Taxes/LegacyTaxes/TaxRange.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxSL1.h"

#include "DataModel/PricingRequest.h"
#include "DBAccess/Loc.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DataModel/Itin.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/Response.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "Common/DateTime.h"

#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/FareUsage.h"
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
#include "Common/ErrorResponseException.h"
#include "Diagnostic/Diag804Collector.h"

#include "Common/TseCodeTypes.h"

#include "test/include/CppUnitHelperMacros.h"
#include <unistd.h>
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;

namespace tse
{

class TaxRangeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxRangeTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxRangeCase0);
  CPPUNIT_TEST(testTaxRangeCase1);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { TaxRange taxRange; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testTaxRangeCase0()
  {
    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;
    trx.getRequest()->ticketingDT() = DateTime(2004, 7, 14, 11, 0, 0);

    Loc origin;
    Loc destination;

    origin.loc() = std::string("DFW");
    origin.nation() = std::string("US");

    destination.loc() = std::string("MIA");
    destination.nation() = std::string("US");
    trx.getRequest()->ticketingAgent()->agentLocation() = &destination;

    AirSeg travelSeg;
    travelSeg.geoTravelType() = GeoTravelType::Domestic;

    travelSeg.origin() = &origin;
    travelSeg.destination() = &destination;
    travelSeg.departureDT() = DateTime(2004, 7, 14, 11, 0, 0);
    Itin itin;

    itin.tripCharacteristics().set(Itin::OneWay, true);
    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(&travelSeg);

    itin.farePath().push_back(&farePath);

    TaxResponse taxResponse;
    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    Diag804Collector diag(*diagroot);

    taxResponse.diagCollector() = &diag;

    taxResponse.farePath() = &farePath;
    taxResponse.paxTypeCode() = std::string("ADT");

    TaxCodeReg taxCodeReg;
    taxCodeReg.specialProcessNo() = 0;
    taxCodeReg.expireDate() = DateTime(2004, 7, 14, 11, 0, 0);
    taxCodeReg.loc1Type() = tse::LocType('N');
    taxCodeReg.loc2Type() = tse::LocType('N');
    taxCodeReg.loc1() = tse::LocCode("MY");
    taxCodeReg.loc1ExclInd() = tse::Indicator('Y' /* TAX_EXCLUDE */);
    taxCodeReg.loc2() = tse::LocCode("MY");
    taxCodeReg.loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    taxCodeReg.taxCode() = std::string("MY");
    taxCodeReg.effDate() = DateTime(2004, 7, 14, 11, 0, 0);
    taxCodeReg.discDate() = DateTime(2004, 7, 14, 11, 0, 0);
    taxCodeReg.firstTvlDate() = DateTime(2004, 7, 14, 11, 0, 0);
    taxCodeReg.lastTvlDate() = DateTime(2004, 7, 14, 11, 0, 0);
    taxCodeReg.nation() = std::string("MY");
    taxCodeReg.taxfullFareInd() = tse::Indicator('N');
    taxCodeReg.taxequivAmtInd() = tse::Indicator('N');
    taxCodeReg.taxexcessbagInd() = tse::Indicator('N');
    taxCodeReg.tvlDateasoriginInd() = tse::Indicator('N');
    taxCodeReg.displayonlyInd() = tse::Indicator('N');
    taxCodeReg.feeInd() = tse::Indicator('N');
    taxCodeReg.interlinableTaxInd() = tse::Indicator('N');
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
    taxCodeReg.freeTktexempt() = tse::Indicator(' ');
    taxCodeReg.idTvlexempt() = tse::Indicator(' ');

    taxCodeReg.taxCur() = tse::CurrencyCode("US");
    taxCodeReg.taxAmt() = 10.0;

    taxCodeReg.fareclassExclInd() = tse::Indicator(' ');
    taxCodeReg.tktdsgExclInd() = tse::Indicator(' ');
    taxCodeReg.valcxrExclInd() = tse::Indicator(' ');

    taxCodeReg.exempequipExclInd() = tse::Indicator(' ');
    taxCodeReg.psgrExclInd() = tse::Indicator(' ');
    taxCodeReg.fareTypeExclInd() = tse::Indicator(' ');

    taxCodeReg.originLocExclInd() = tse::Indicator('N');
    taxCodeReg.originLocType() = tse::LocType(' ');
    taxCodeReg.originLoc() = tse::LocCode("");

    taxCodeReg.originLocExclInd() = tse::Indicator('N');
    taxCodeReg.originLocType() = tse::LocType(' ');
    taxCodeReg.originLoc() = tse::LocCode("");
    taxCodeReg.loc1Appl() = tse::Indicator(' ');
    taxCodeReg.loc2Appl() = tse::Indicator(' ');
    taxCodeReg.tripType() = tse::Indicator(' ');
    taxCodeReg.travelType() = tse::Indicator('D');
    taxCodeReg.itineraryType() = tse::Indicator('O');
    taxCodeReg.formOfPayment() = tse::Indicator(' ');
    taxCodeReg.taxOnTaxExcl() = tse::Indicator(' ');

    taxCodeReg.rangeType() = tse::Indicator(' ');
    taxCodeReg.rangeincrement() = 0;
    taxCodeReg.lowRange() = 50;
    taxCodeReg.highRange() = 1000;
    taxCodeReg.rangeInd() = tse::Indicator(' ');

    uint16_t startIndex = 0;
    uint16_t endIndex = 0;

    TaxRange taxRange;

    bool valid = taxRange.validateRange(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    CPPUNIT_ASSERT_EQUAL(true, valid);
  }

  void testTaxRangeCase1()
  {

    PricingTrx trx;
    trx.setOptions(_memHandle.create<PricingOptions>());

    Agent agent;

    Loc origin;
    Loc destination;
    Loc termination;

    origin.loc() = std::string("BRU");
    origin.nation() = std::string("BE");

    destination.loc() = std::string("FNA");
    destination.nation() = std::string("SL");

    termination.loc() = std::string("ROB");
    termination.nation() = std::string("LI");

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;
    DateTime dt;
    trx.getRequest()->ticketingDT() = dt;
    trx.getRequest()->ticketingAgent()->currencyCodeAgent() = "USD";
    trx.getRequest()->ticketingAgent()->agentLocation() = &destination; // AAA in Sierra Leone

    AirSeg travelSeg1;
    AirSeg travelSeg2;

    travelSeg1.geoTravelType() = GeoTravelType::International;

    travelSeg1.origin() = &origin;
    travelSeg1.destination() = &destination;
    travelSeg1.segmentOrder() = 1;
    travelSeg1.arrivalDT() = dt;
    travelSeg1.departureDT() = dt;

    trx.travelSeg().push_back(&travelSeg1);

    travelSeg2.origin() = &destination;
    travelSeg2.destination() = &termination;
    travelSeg2.segmentOrder() = 2;
    travelSeg2.arrivalDT() = dt;
    travelSeg2.departureDT() = dt;

    Itin itin;

    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(&travelSeg1);
    farePath.itin()->travelSeg().push_back(&travelSeg2);

    itin.farePath().push_back(&farePath);

    TaxResponse taxResponse;
    Diagnostic* diagroot = new Diagnostic(LegacyTaxDiagnostic24);
    Diag804Collector diag(*diagroot);

    taxResponse.diagCollector() = &diag;

    taxResponse.farePath() = &farePath;
    taxResponse.paxTypeCode() = std::string("ADT");

    TaxCodeReg taxCodeReg;
    taxCodeReg.specialProcessNo() = 33;
    taxCodeReg.expireDate() = dt;
    taxCodeReg.loc1Type() = tse::LocType('N');
    taxCodeReg.loc2Type() = tse::LocType('N');
    taxCodeReg.loc1() = tse::LocCode("JP");
    taxCodeReg.loc1ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    taxCodeReg.loc2() = tse::LocCode("JP");
    taxCodeReg.loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    taxCodeReg.taxCode() = std::string("JP1");
    taxCodeReg.effDate() = dt;
    taxCodeReg.discDate() = dt;
    taxCodeReg.firstTvlDate() = dt;
    taxCodeReg.lastTvlDate() = dt;
    taxCodeReg.nation() = std::string("JP");
    taxCodeReg.taxfullFareInd() = tse::Indicator('N');
    taxCodeReg.taxequivAmtInd() = tse::Indicator('N');
    taxCodeReg.taxexcessbagInd() = tse::Indicator('N');
    taxCodeReg.tvlDateasoriginInd() = tse::Indicator('N');
    taxCodeReg.displayonlyInd() = tse::Indicator('N');
    taxCodeReg.feeInd() = tse::Indicator('N');
    taxCodeReg.interlinableTaxInd() = tse::Indicator('N');
    taxCodeReg.showseparateInd() = tse::Indicator('N');

    taxCodeReg.posExclInd() = tse::Indicator('N');
    taxCodeReg.posLocType() = tse::LocType(' ');
    taxCodeReg.posLoc() = tse::LocCode("");

    taxCodeReg.poiExclInd() = tse::Indicator('Y');
    taxCodeReg.poiLocType() = tse::LocType('N');
    taxCodeReg.poiLoc() = tse::LocCode("SL");

    taxCodeReg.sellCurExclInd() = tse::Indicator('N');
    taxCodeReg.sellCur() = tse::CurrencyCode("SLL");

    taxCodeReg.occurrence() = tse::Indicator(' ');
    taxCodeReg.freeTktexempt() = tse::Indicator('N');
    taxCodeReg.idTvlexempt() = tse::Indicator('N');

    taxCodeReg.taxCur() = tse::CurrencyCode("JPY");
    taxCodeReg.taxAmt() = 800;
    taxCodeReg.taxType() = 'P';

    taxCodeReg.fareclassExclInd() = tse::Indicator('N');
    taxCodeReg.tktdsgExclInd() = tse::Indicator('N');
    taxCodeReg.valcxrExclInd() = tse::Indicator('N');

    taxCodeReg.exempequipExclInd() = tse::Indicator('N');
    taxCodeReg.psgrExclInd() = tse::Indicator('Y');
    taxCodeReg.fareTypeExclInd() = tse::Indicator('N');

    taxCodeReg.originLocExclInd() = tse::Indicator('N');
    taxCodeReg.originLocType() = tse::LocType(' ');
    taxCodeReg.originLoc() = tse::LocCode("");
    taxCodeReg.loc1Appl() = tse::Indicator(' ');
    taxCodeReg.loc2Appl() = tse::Indicator(' ');
    taxCodeReg.tripType() = tse::Indicator('B');
    taxCodeReg.travelType() = tse::Indicator('I');
    taxCodeReg.itineraryType() = tse::Indicator(' ');
    taxCodeReg.formOfPayment() = tse::Indicator('A');
    taxCodeReg.taxOnTaxExcl() = tse::Indicator('N');

    TaxSL1 taxSL1;

    PricingUnit pricingUnit;
    FareUsage fareUsage;
    Fare fare;
    PaxTypeFare paxTypeFare;

    FareMarket fareMarket;
    fareMarket.travelSeg().push_back(&travelSeg1);

    fare.nucFareAmount() = 10000.00;

    FareInfo fareInfo;
    fareInfo._fareAmount = 10000.00;
    fareInfo._currency = "JPY";

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._fareTariffCode = "TAFPBA";

    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    fareUsage.paxTypeFare() = &paxTypeFare;
    fareUsage.paxTypeFare()->setFare(&fare);
    fareUsage.totalFareAmount();

    fareUsage.travelSeg().push_back(&travelSeg1);
    fareUsage.travelSeg().push_back(&travelSeg2);

    taxResponse.farePath()->pricingUnit().push_back(&pricingUnit);
    taxResponse.farePath()->pricingUnit()[0]->fareUsage().push_back(&fareUsage);

    taxCodeReg.rangeType() = tse::Indicator('F');
    taxCodeReg.rangeincrement() = 0;
    taxCodeReg.lowRange() = 50;
    taxCodeReg.highRange() = 1000;

    taxCodeReg.rangeInd() = tse::Indicator(' ');

    uint16_t startIndex = 0;
    uint16_t endIndex = 0;

    TaxRange taxRange;

    bool valid = taxRange.validateRange(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    CPPUNIT_ASSERT_EQUAL(false, valid);

    taxCodeReg.rangeInd() = tse::Indicator('T');

    valid = taxRange.validateRange(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    CPPUNIT_ASSERT_EQUAL(false, valid);

    taxCodeReg.rangeInd() = tse::Indicator('A');

    valid = taxRange.validateRange(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    CPPUNIT_ASSERT_EQUAL(false, valid);

    trx.getRequest()->ticketingAgent()->currencyCodeAgent() = "USD";

    taxCodeReg.rangeInd() = tse::Indicator('T');

    valid = taxRange.validateRange(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    CPPUNIT_ASSERT_EQUAL(false, valid);

    taxCodeReg.rangeInd() = tse::Indicator('A');

    valid = taxRange.validateRange(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    CPPUNIT_ASSERT_EQUAL(false, valid);

    taxCodeReg.rangeincrement() = 2;
    taxCodeReg.rangeType() = 'F';
    taxCodeReg.rangeInd() = 'T';
    MoneyAmount moneyAmount = 10.0;
    CurrencyCode paymentCurrency = "USD";

    taxRange.applyRange(
        trx, taxResponse, moneyAmount, paymentCurrency, startIndex, endIndex, taxCodeReg);

    taxCodeReg.rangeInd() = 'A';

    taxRange.applyRange(
        trx, taxResponse, moneyAmount, paymentCurrency, startIndex, endIndex, taxCodeReg);

    taxCodeReg.rangeType() = 'M';
    taxCodeReg.rangeInd() = 'T';

    taxRange.applyRange(
        trx, taxResponse, moneyAmount, paymentCurrency, startIndex, endIndex, taxCodeReg);

    taxCodeReg.rangeInd() = 'A';

    taxRange.applyRange(
        trx, taxResponse, moneyAmount, paymentCurrency, startIndex, endIndex, taxCodeReg);
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxRangeTest);
}
