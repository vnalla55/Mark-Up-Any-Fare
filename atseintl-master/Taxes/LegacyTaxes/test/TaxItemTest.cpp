#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Taxes/LegacyTaxes/TaxApply.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxMap.h"

#include "DataModel/PricingRequest.h"
#include "DBAccess/Loc.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/PaxType.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxType.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "DataModel/Response.h"
#include "DataModel/Itin.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include "test/include/CppUnitHelperMacros.h"
#include <unistd.h>
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;

namespace tse
{
class TaxItemTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxItemTest);
  CPPUNIT_TEST(testBuildTax);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testBuildTax()
  {
    PricingTrx trx;
    trx.setOptions(_memHandle.create<PricingOptions>());

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;
    trx.getRequest()->ticketingDT() = (DateTime)10;

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
    travelSeg.departureDT() = (DateTime)10;
    Itin itin;

    itin.tripCharacteristics().set(Itin::OneWay, true);
    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(&travelSeg);

    itin.farePath().push_back(&farePath);

    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;
    taxResponse.paxTypeCode() = std::string("ADT");

    TaxCodeReg taxCodeReg;
    taxCodeReg.specialProcessNo() = 0;
    taxCodeReg.expireDate() = (DateTime)999;
    taxCodeReg.loc1Type() = tse::LocType('N');
    taxCodeReg.loc2Type() = tse::LocType('N');
    taxCodeReg.loc1() = tse::LocCode("MY");
    taxCodeReg.loc1ExclInd() = tse::Indicator('Y' /* TAX_EXCLUDE */);
    taxCodeReg.loc2() = tse::LocCode("MY");
    taxCodeReg.loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    taxCodeReg.taxCode() = std::string("MY");
    taxCodeReg.effDate() = (DateTime)1;
    taxCodeReg.discDate() = (DateTime)999;
    taxCodeReg.firstTvlDate() = (DateTime)5;
    taxCodeReg.lastTvlDate() = (DateTime)20;
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

    taxCodeReg.taxCur() = tse::CurrencyCode("USD");
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

    TaxMap taxMap(trx.dataHandle());

    taxMap.initialize();

    Tax& tax = *taxMap.getSpecialTax(taxCodeReg.specialProcessNo());

    TaxItem taxItem;
    tax.taxOnTaxItems().push_back(&taxItem);

    TaxItem taxItem2;
    tax.taxOnTaxItems().push_back(&taxItem2);

    taxItem.buildTaxItem(trx, tax, taxResponse, taxCodeReg);

    CPPUNIT_ASSERT(tax.taxOnTaxItems().size() == taxItem.taxOnTaxItems().size());

    CPPUNIT_ASSERT(std::equal(tax.taxOnTaxItems().begin(),
                              tax.taxOnTaxItems().begin() + tax.taxOnTaxItems().size(),
                              taxItem.taxOnTaxItems().begin()));
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxItemTest);
}
