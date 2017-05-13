#include <string>
#include <iostream>
#include <vector>

#include "Taxes/LegacyTaxes/TaxSL1.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "Taxes/LegacyTaxes/Tax.h"

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

#include "test/include/CppUnitHelperMacros.h"
#include "Common/TseCodeTypes.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include <unistd.h>

using namespace std;

namespace tse
{
class TaxSL1Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxSL1Test);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxSL1);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { TaxSL1Test taxSL1Test; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testTaxSL1()
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
    trx.getRequest()->ticketingDT() = DateTime(2004, 7, 14, 11, 0, 0);
    trx.getRequest()->ticketingAgent()->currencyCodeAgent() = "SLL";
    trx.getRequest()->ticketingAgent()->agentLocation() = &destination; // AAA in Sierra Leone

    AirSeg travelSeg1;
    AirSeg travelSeg2;

    travelSeg1.geoTravelType() = GeoTravelType::International;

    travelSeg1.origin() = &origin;
    travelSeg1.destination() = &destination;
    travelSeg1.departureDT() = (DateTime)10;

    travelSeg2.origin() = &destination;
    travelSeg2.destination() = &termination;
    travelSeg2.departureDT() = (DateTime)50;

    Itin itin;

    itin.tripCharacteristics().set(Itin::OneWay, true);
    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(&travelSeg1);
    farePath.itin()->travelSeg().push_back(&travelSeg2);

    itin.farePath().push_back(&farePath);

    TaxResponse taxResponse;
    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    diagroot->activate();
    Diag804Collector diag(*diagroot);

    taxResponse.diagCollector() = &diag;

    taxResponse.farePath() = &farePath;
    taxResponse.paxTypeCode() = std::string("ADT");

    TaxCodeReg taxCodeReg;
    taxCodeReg.specialProcessNo() = 33;
    taxCodeReg.expireDate() = DateTime(2004, 12, 31, 12, 0, 0);
    taxCodeReg.loc1Type() = tse::LocType('N');
    taxCodeReg.loc2Type() = tse::LocType('N');
    taxCodeReg.loc1() = tse::LocCode("SL");
    taxCodeReg.loc1ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    taxCodeReg.loc2() = tse::LocCode("SL");
    taxCodeReg.loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    taxCodeReg.taxCode() = std::string("SL1");
    taxCodeReg.effDate() = DateTime(2004, 7, 13, 12, 0, 0);
    taxCodeReg.discDate() = DateTime(2004, 12, 31, 12, 0, 0);
    ;
    taxCodeReg.firstTvlDate() = DateTime(2004, 7, 14, 12, 0, 0);
    taxCodeReg.lastTvlDate() = DateTime(2004, 7, 14, 13, 0, 0);
    taxCodeReg.nation() = std::string("SL");
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

    taxCodeReg.taxCur() = tse::CurrencyCode("SLL");
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
    uint16_t _travelSegStartIndex = 0;
    uint16_t _travelSegEndIndex = 1;

    PricingUnit pricingUnit;
    FareUsage fareUsage;
    Fare fare;
    PaxTypeFare paxTypeFare;

    FareMarket fareMarket;
    fareMarket.travelSeg().push_back(&travelSeg1);

    fare.nucFareAmount() = 10000.00;

    FareInfo fareInfo;
    fareInfo._fareAmount = 10000.00;
    fareInfo._currency = "SLL";

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._fareTariffCode = "TAFPBA";

    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    fareUsage.paxTypeFare() = &paxTypeFare;
    fareUsage.paxTypeFare()->setFare(&fare);
    fareUsage.totalFareAmount();

    taxResponse.farePath()->pricingUnit().push_back(&pricingUnit);
    taxResponse.farePath()->pricingUnit()[0]->fareUsage().push_back(&fareUsage);

    taxSL1.taxCreate(trx, taxResponse, taxCodeReg, _travelSegStartIndex, _travelSegEndIndex);

    TaxMap taxMap(trx.dataHandle());

    taxMap.initialize();

    TaxItem taxItem;

    Tax& tax = *taxMap.getSpecialTax(taxCodeReg.specialProcessNo());

    taxItem.buildTaxItem(trx, tax, taxResponse, taxCodeReg);

    taxResponse.taxItemVector().push_back(&taxItem);
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxSL1Test);
}
