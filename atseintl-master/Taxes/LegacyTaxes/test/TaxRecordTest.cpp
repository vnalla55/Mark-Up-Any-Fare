#include "Taxes/LegacyTaxes/TaxRecord.h"

#include "Common/Config/ConfigurableValuesPool.h"
#include "Common/Global.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Response.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxNation.h"
#include "FareCalc/FareCalcConsts.h"
#include "Taxes/Common/AtpcoTaxName.h"
#include "Taxes/LegacyTaxes/TaxApply.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Xform/DynamicConfig.h"

#include <iostream>
#include <string>
#include <vector>

#include <time.h>
#include <unistd.h>

using namespace std;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
public:
  const TaxNation*
  getTaxNation(const NationCode& key, const DateTime& date) override
  {
    auto found = std::find_if(_taxNationVector.begin(),
                              _taxNationVector.end(),
                              [&key](const TaxNation* taxNation)
                              {
                                 return taxNation->nation() == key;
                              });

    return found != _taxNationVector.end() ? *found : nullptr;
  }

  std::vector<TaxNation*>& taxNationVector() { return _taxNationVector; }

private:
  std::vector<TaxNation*> _taxNationVector;
};
} // anonymous namespace


class TaxRecordTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxRecordTest);
  CPPUNIT_TEST(testTaxRecordCase0);
  CPPUNIT_TEST(testTaxRecordCase1);
  CPPUNIT_TEST(testTaxRecordCase2);
  CPPUNIT_TEST(testTaxRecordTicketingOrderAtpco);
  CPPUNIT_TEST(testTaxRecordPreOrderAtpco_TicketAgentNationAsFirst);
  CPPUNIT_TEST(testTaxRecordPreOrderAtpco_OriginNationAsFirst);
  CPPUNIT_TEST(testTaxRecordTicketingOrder);
  CPPUNIT_TEST_SUITE_END();

public:
  TaxRecordTest() {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _dataHandle = _memHandle.create<MyDataHandle>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testTaxRecordCase0()
  {
    PricingTrx trx;
    trx.setOptions(_memHandle.create<PricingOptions>());

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;

    DateTime dt;

    trx.getRequest()->ticketingDT() = dt.localTime();

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
    DCFactory* factory = DCFactory::instance();
    DiagCollector& diagCollector = *(factory->create(trx));

    taxResponse.diagCollector() = &diagCollector;

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

    TaxItem taxItem;

    Tax& tax = *taxMap.getSpecialTax(taxCodeReg.specialProcessNo());

    taxItem.buildTaxItem(trx, tax, taxResponse, taxCodeReg);

    taxResponse.taxItemVector().push_back(&taxItem);

    TaxRecord taxRecord;

    taxRecord.buildTicketLine(trx, taxResponse);
  }

  void testTaxRecordCase1()
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
    DCFactory* factory = DCFactory::instance();
    DiagCollector& diagCollector = *(factory->create(trx));

    taxResponse.diagCollector() = &diagCollector;

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
    taxCodeReg.taxCode() = std::string("GB4");
    taxCodeReg.effDate() = (DateTime)1;
    taxCodeReg.discDate() = (DateTime)999;
    taxCodeReg.firstTvlDate() = (DateTime)5;
    taxCodeReg.lastTvlDate() = (DateTime)20;
    taxCodeReg.nation() = std::string("GB");
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
    taxCodeReg.multioccconvrndInd() = tse::Indicator('Y');

    TaxMap taxMap(trx.dataHandle());

    taxMap.initialize();

    TaxItem taxItem;

    Tax& tax = *taxMap.getSpecialTax(taxCodeReg.specialProcessNo());

    taxItem.buildTaxItem(trx, tax, taxResponse, taxCodeReg);

    taxResponse.taxItemVector().push_back(&taxItem);

    TaxCodeReg taxCodeReg2;
    taxCodeReg2.specialProcessNo() = 0;
    taxCodeReg2.expireDate() = (DateTime)999;
    taxCodeReg2.loc1Type() = tse::LocType('N');
    taxCodeReg2.loc2Type() = tse::LocType('N');
    taxCodeReg2.loc1() = tse::LocCode("MY");
    taxCodeReg2.loc1ExclInd() = tse::Indicator('Y' /* TAX_EXCLUDE */);
    taxCodeReg2.loc2() = tse::LocCode("US");
    taxCodeReg2.loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    taxCodeReg2.taxCode() = std::string("YC");
    taxCodeReg2.effDate() = (DateTime)1;
    taxCodeReg2.discDate() = (DateTime)999;
    taxCodeReg2.firstTvlDate() = (DateTime)5;
    taxCodeReg2.lastTvlDate() = (DateTime)20;
    taxCodeReg2.nation() = std::string("US");
    taxCodeReg2.taxfullFareInd() = tse::Indicator('N');
    taxCodeReg2.taxequivAmtInd() = tse::Indicator('N');
    taxCodeReg2.taxexcessbagInd() = tse::Indicator('N');
    taxCodeReg2.tvlDateasoriginInd() = tse::Indicator('N');
    taxCodeReg2.displayonlyInd() = tse::Indicator('N');
    taxCodeReg2.feeInd() = tse::Indicator('N');
    taxCodeReg2.interlinableTaxInd() = tse::Indicator('N');
    taxCodeReg2.showseparateInd() = tse::Indicator('Y');

    taxCodeReg2.posExclInd() = tse::Indicator('N');
    taxCodeReg2.posLocType() = tse::LocType(' ');
    taxCodeReg2.posLoc() = tse::LocCode("");

    taxCodeReg2.poiExclInd() = tse::Indicator('N');
    taxCodeReg2.poiLocType() = tse::LocType(' ');
    taxCodeReg2.poiLoc() = tse::LocCode("");

    taxCodeReg2.sellCurExclInd() = tse::Indicator('N');
    taxCodeReg2.sellCur() = tse::CurrencyCode("");

    taxCodeReg2.occurrence() = tse::Indicator(' ');
    taxCodeReg2.freeTktexempt() = tse::Indicator(' ');
    taxCodeReg2.idTvlexempt() = tse::Indicator(' ');

    taxCodeReg2.taxCur() = tse::CurrencyCode("USD");
    taxCodeReg2.taxAmt() = 40.0;

    taxCodeReg2.fareclassExclInd() = tse::Indicator(' ');
    taxCodeReg2.tktdsgExclInd() = tse::Indicator(' ');
    taxCodeReg2.valcxrExclInd() = tse::Indicator(' ');

    taxCodeReg2.exempequipExclInd() = tse::Indicator(' ');
    taxCodeReg2.psgrExclInd() = tse::Indicator(' ');
    taxCodeReg2.fareTypeExclInd() = tse::Indicator(' ');

    taxCodeReg2.originLocExclInd() = tse::Indicator('N');
    taxCodeReg2.originLocType() = tse::LocType(' ');
    taxCodeReg2.originLoc() = tse::LocCode("");

    taxCodeReg2.originLocExclInd() = tse::Indicator('N');
    taxCodeReg2.originLocType() = tse::LocType(' ');
    taxCodeReg2.originLoc() = tse::LocCode("");
    taxCodeReg2.loc1Appl() = tse::Indicator(' ');
    taxCodeReg2.loc2Appl() = tse::Indicator(' ');
    taxCodeReg2.tripType() = tse::Indicator(' ');
    taxCodeReg2.travelType() = tse::Indicator('D');
    taxCodeReg2.itineraryType() = tse::Indicator('O');
    taxCodeReg2.formOfPayment() = tse::Indicator(' ');
    taxCodeReg2.taxOnTaxExcl() = tse::Indicator(' ');
    taxCodeReg2.multioccconvrndInd() = tse::Indicator('Y');

    TaxItem taxItem2;

    taxItem2.buildTaxItem(trx, tax, taxResponse, taxCodeReg2);

    taxResponse.taxItemVector().push_back(&taxItem2);

    TaxRecord taxRecord;

    taxRecord.buildTicketLine(trx, taxResponse);
  }

  void testTaxRecordCase2()
  {
    PricingTrx trx;
    trx.setOptions(_memHandle.create<PricingOptions>());

    PricingRequest request;
    trx.setRequest(&request);

    TaxResponse taxResponse;
    DCFactory* factory = DCFactory::instance();
    DiagCollector& diagCollector = *(factory->create(trx));

    taxResponse.diagCollector() = &diagCollector;

    TaxRecord taxRecord1;
    TaxRecord taxRecord2;
    TaxRecord taxRecord3;
    TaxRecord taxRecord4;
    TaxRecord taxRecord5;

    taxRecord1._taxRollXTNotAllowedInd = 'Y';
    taxRecord2.setTaxAmount(0.0);
    taxRecord3.setTaxAmount(10.0);
    taxRecord4.setTaxAmount(10.0);
    taxRecord5.setTaxAmount(10.0);

    trx.getRequest()->numberTaxBoxes() = 2;
    taxResponse.taxRecordVector().push_back(&taxRecord1);
    taxResponse.taxRecordVector().push_back(&taxRecord2);
    taxResponse.taxRecordVector().push_back(&taxRecord3);
    taxResponse.taxRecordVector().push_back(&taxRecord4);
    taxResponse.taxRecordVector().push_back(&taxRecord5);

    taxRecord1.ticketBoxCompression(trx, taxResponse);

    CPPUNIT_ASSERT_EQUAL('Y', taxRecord4.taxRolledXTInd());
    CPPUNIT_ASSERT_EQUAL('Y', taxRecord5.taxRolledXTInd());
  }

  void testTaxRecordTicketingOrder()
  {

    TaxRecord record1, record2, record3, record4, record5, record6;
    record1.setTaxCode("TC1");
    record2.setTaxCode("TC2");
    record3.setTaxCode("TC3");
    record4.setTaxCode("TC4");
    record5.setTaxCode("TC5");
    record6.setTaxCode("TC6");

    TaxResponse taxResponse;
    taxResponse.taxRecordVector().push_back(&record1);
    taxResponse.taxRecordVector().push_back(&record2);
    taxResponse.taxRecordVector().push_back(&record3);
    taxResponse.taxRecordVector().push_back(&record4);
    taxResponse.taxRecordVector().push_back(&record5);
    taxResponse.taxRecordVector().push_back(&record6);

    TaxNation taxNation;
    taxNation.taxOrderTktIssue().push_back( TaxOrderTktIssue("TC5", "000") ); // record5
    taxNation.taxOrderTktIssue().push_back( TaxOrderTktIssue("TC4", "AAA") ); // record4
    taxNation.taxOrderTktIssue().push_back( TaxOrderTktIssue("TC3", "000") ); // record3
    taxNation.taxOrderTktIssue().push_back( TaxOrderTktIssue("TC2", "AAA") ); // record2

    TaxRecord taxRecord;
    taxRecord.sortLegacy(taxNation.taxOrderTktIssue(), taxResponse);

    CPPUNIT_ASSERT("TC5" == taxResponse.taxRecordVector()[0]->taxCode());
    CPPUNIT_ASSERT("TC3" == taxResponse.taxRecordVector()[1]->taxCode());
    CPPUNIT_ASSERT("TC2" == taxResponse.taxRecordVector()[2]->taxCode());
    CPPUNIT_ASSERT("TC4" == taxResponse.taxRecordVector()[3]->taxCode());
    CPPUNIT_ASSERT("TC1" == taxResponse.taxRecordVector()[4]->taxCode());
    CPPUNIT_ASSERT("TC6" == taxResponse.taxRecordVector()[5]->taxCode());
  }

  void testTaxRecordTicketingOrderAtpco()
  {
    AtpcoTaxName taxName1, taxName2, taxName3, taxName4, taxName5, taxName6;
    taxName1.taxCode = "C1";
    taxName1.taxType = "AAA";
    taxName2.taxCode = "C2";
    taxName2.taxType = "AAA";
    taxName3.taxCode = "C3";
    taxName3.taxType = "AAA";
    taxName4.taxCode = "C4";
    taxName4.taxType = "AAA";
    taxName5.taxCode = "C5";
    taxName5.taxType = "AAA";
    taxName6.taxCode = "C6";
    taxName6.taxType = "AAA";

    TaxRecord record1, record2, record3, record4, record5, record6;
    record1.setAtpcoTaxName(taxName1);
    record1.setTaxCode("TC1");
    record2.setAtpcoTaxName(taxName2);
    record2.setTaxCode("TC2");
    record3.setAtpcoTaxName(taxName3);
    record3.setTaxCode("TC3");
    record4.setAtpcoTaxName(taxName4);
    record4.setTaxCode("TC4");
    record5.setAtpcoTaxName(taxName5);
    record5.setTaxCode("TC5");
    record6.setAtpcoTaxName(taxName6);
    record6.setTaxCode("TC6");

    TaxResponse taxResponse;
    taxResponse.taxRecordVector().push_back(&record1);
    taxResponse.taxRecordVector().push_back(&record2);
    taxResponse.taxRecordVector().push_back(&record3);
    taxResponse.taxRecordVector().push_back(&record4);
    taxResponse.taxRecordVector().push_back(&record5);
    taxResponse.taxRecordVector().push_back(&record6);

    TaxNation taxNation;
    taxNation.taxOrderTktIssue().push_back( TaxOrderTktIssue("C5", "000") ); // record5
    taxNation.taxOrderTktIssue().push_back( TaxOrderTktIssue("C4", "AAA") ); // record4
    taxNation.taxOrderTktIssue().push_back( TaxOrderTktIssue("C3", "000") ); // record3
    taxNation.taxOrderTktIssue().push_back( TaxOrderTktIssue("C2", "AAA") ); // record2

    TaxRecord taxRecord;
    taxRecord.sortAtpco(taxNation.taxOrderTktIssue(), taxResponse);

    CPPUNIT_ASSERT("TC4" == taxResponse.taxRecordVector()[0]->taxCode());
    CPPUNIT_ASSERT("TC2" == taxResponse.taxRecordVector()[1]->taxCode());
    CPPUNIT_ASSERT("TC3" == taxResponse.taxRecordVector()[2]->taxCode());
    CPPUNIT_ASSERT("TC1" == taxResponse.taxRecordVector()[3]->taxCode());
    CPPUNIT_ASSERT("TC5" == taxResponse.taxRecordVector()[4]->taxCode());
    CPPUNIT_ASSERT("TC6" == taxResponse.taxRecordVector()[5]->taxCode());
  }

  void createPreOrderAtpcoData(PricingTrx& trx, TaxResponse& taxResponse)
  {
    // add nations with tax order
    TaxNation *nation1, *nation2, *nation3;
    nation1 = _memHandle.create<TaxNation>();
    nation1->nation() = "TW";
    nation1->taxCodeOrder().push_back("TW");
    nation2 = _memHandle.create<TaxNation>();
    nation2->nation() = "BR";
    nation2->taxCodeOrder().push_back("BR");
    nation3 = _memHandle.create<TaxNation>();
    nation3->nation() = "US";
    nation3->taxCodeOrder().push_back("YC");
    nation3->taxCodeOrder().push_back("XY");
    nation3->taxCodeOrder().push_back("US");
    nation3->taxCodeOrder().push_back("AY");

    _dataHandle->taxNationVector().clear();
    _dataHandle->taxNationVector().push_back(nation1);
    _dataHandle->taxNationVector().push_back(nation2);
    _dataHandle->taxNationVector().push_back(nation3);

    // create itin (TW - BR - US - TW)
    Loc *locTW, *locBR, *locUS;
    locTW = _memHandle.create<Loc>();
    locTW->nation() = "TW";
    locBR = _memHandle.create<Loc>();
    locBR->nation() = "BR";
    locUS = _memHandle.create<Loc>();
    locUS->nation() = "US";

    AirSeg *airSegTW_BR, *airSegBR_US, *airSegUS_TW;
    airSegTW_BR = _memHandle.create<AirSeg>();
    airSegTW_BR->origin() = locTW;
    airSegTW_BR->destination() = locBR;
    airSegBR_US = _memHandle.create<AirSeg>();
    airSegBR_US->origin() = locBR;
    airSegBR_US->destination() = locUS;
    airSegUS_TW = _memHandle.create<AirSeg>();
    airSegUS_TW->origin() = locUS;
    airSegUS_TW->destination() = locTW;

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(airSegTW_BR);
    itin->travelSeg().push_back(airSegBR_US);
    itin->travelSeg().push_back(airSegUS_TW);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    taxResponse.farePath() = farePath;

    // add TaxRecords
    TaxRecord *taxTW, *taxBR, *taxYC, *taxXY, *taxUS, *taxAY, *taxYQ, *taxYR, *taxXF;
    taxTW = _memHandle.create<TaxRecord>();
    taxTW->setTaxCode("TW");
    taxBR = _memHandle.create<TaxRecord>();
    taxBR->setTaxCode("BR");
    taxYC = _memHandle.create<TaxRecord>();
    taxYC->setTaxCode("YC");
    taxXY = _memHandle.create<TaxRecord>();
    taxXY->setTaxCode("XY");
    taxUS = _memHandle.create<TaxRecord>();
    taxUS->setTaxCode("US");
    taxAY = _memHandle.create<TaxRecord>();
    taxAY->setTaxCode("AY");

    // special taxes, XF should be always on bottom,
    // YQ/YR - depends on sorting first nation - origin or ticket agent
    taxYQ = _memHandle.create<TaxRecord>();
    taxYQ->setTaxCode("YQ");
    taxYR = _memHandle.create<TaxRecord>();
    taxYR->setTaxCode("YR");
    taxXF = _memHandle.create<TaxRecord>();
    taxXF->setTaxCode("XF");

    // add TaxRecords in mixed order
    taxResponse.taxRecordVector().push_back(taxXY);
    taxResponse.taxRecordVector().push_back(taxAY);
    taxResponse.taxRecordVector().push_back(taxYR);
    taxResponse.taxRecordVector().push_back(taxUS);
    taxResponse.taxRecordVector().push_back(taxYQ);
    taxResponse.taxRecordVector().push_back(taxYC);
    taxResponse.taxRecordVector().push_back(taxBR);
    taxResponse.taxRecordVector().push_back(taxXF);
    taxResponse.taxRecordVector().push_back(taxTW);

    // some trx's 'must be' fields
    FareCalcConfig* fareCalcConfig = _memHandle.create<FareCalcConfig>();
    trx.fareCalcConfig() = fareCalcConfig;

    PricingOptions* pricingOptions = _memHandle.create<PricingOptions>();
    trx.setOptions(pricingOptions);

    PricingRequest* pricingRequest = _memHandle.create<PricingRequest>();
    trx.setRequest(pricingRequest);
  }

  void testTaxRecordPreOrderAtpco_TicketAgentNationAsFirst()
  {
    PricingTrx trx;
    TaxResponse taxResponse;
    createPreOrderAtpcoData(trx, taxResponse);

    Loc agentLocation;
    agentLocation.nation() = "BR";

    Agent ticketingAgent;
    ticketingAgent.agentLocation() = &agentLocation;
    trx.getRequest()->ticketingAgent() = &ticketingAgent;

    TaxRecord taxRecord;
    taxRecord.preSortAtpco(trx, taxResponse);

    CPPUNIT_ASSERT_EQUAL(TaxCode("YQ"), taxResponse.taxRecordVector()[0]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("YR"), taxResponse.taxRecordVector()[1]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("BR"), taxResponse.taxRecordVector()[2]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("TW"), taxResponse.taxRecordVector()[3]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("YC"), taxResponse.taxRecordVector()[4]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("XY"), taxResponse.taxRecordVector()[5]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("US"), taxResponse.taxRecordVector()[6]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("AY"), taxResponse.taxRecordVector()[7]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("XF"), taxResponse.taxRecordVector()[8]->taxCode());
  }

  void testTaxRecordPreOrderAtpco_OriginNationAsFirst()
  {
    PricingTrx trx;
    TaxResponse taxResponse;
    createPreOrderAtpcoData(trx, taxResponse);
    trx.fareCalcConfig()->taxPlacementInd() = FareCalcConsts::FC_THREE;

    TaxRecord taxRecord;
    taxRecord.preSortAtpco(trx, taxResponse);

    CPPUNIT_ASSERT_EQUAL(TaxCode("TW"), taxResponse.taxRecordVector()[0]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("BR"), taxResponse.taxRecordVector()[1]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("YC"), taxResponse.taxRecordVector()[2]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("XY"), taxResponse.taxRecordVector()[3]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("US"), taxResponse.taxRecordVector()[4]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("AY"), taxResponse.taxRecordVector()[5]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("YR"), taxResponse.taxRecordVector()[6]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("YQ"), taxResponse.taxRecordVector()[7]->taxCode());
    CPPUNIT_ASSERT_EQUAL(TaxCode("XF"), taxResponse.taxRecordVector()[8]->taxCode());
  }

private:
  MyDataHandle* _dataHandle;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxRecordTest);
}
