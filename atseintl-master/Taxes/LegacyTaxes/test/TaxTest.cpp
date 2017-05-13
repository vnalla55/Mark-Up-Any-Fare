#include <string>
#include <vector>

#include <boost/assign/std/vector.hpp>

#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConverter.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Response.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxNation.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

using namespace std;
using namespace boost::assign;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  ~MyDataHandle() { _memHandle.clear(); }
  const TaxNation* getTaxNation(const NationCode& nation, const DateTime& date)
  {
    if (nation == "US")
    {
      TaxNation* ret = _memHandle.create<TaxNation>();
      ret->roundingRule() = NEAREST;
      ret->taxRoundingOverrideInd() = 'E';
      ret->taxCollectionInd() = 'A';
      ret->taxFarequoteInd() = 'I';
      ret->taxCodeOrder() += "US1", "ZP", "US2", "YC", "XY", "XA", "AY", "YZ1";
      return ret;
    }
    return DataHandleMock::getTaxNation(nation, date);
  }

  std::vector<TaxSpecConfigReg*>& getTaxSpecConfig(const TaxSpecConfigName& name)
  {
    std::vector<TaxSpecConfigReg*>& ret = *_memHandle.create<std::vector<TaxSpecConfigReg*> >();
    TaxSpecConfigReg* taxSpecConfigReg = _memHandle.create<TaxSpecConfigReg>();
    ret.push_back(taxSpecConfigReg);

    return ret;
  }
};
}

class TaxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxCase0);
  CPPUNIT_TEST(testTaxInternationalFareRoundingEmptyAmt);
  CPPUNIT_TEST(testTaxInternationalFareRounding);
  CPPUNIT_TEST(testValidateTransferTypeLoc1);
  CPPUNIT_TEST(testValidateSeqencePercentageTax);
  CPPUNIT_TEST(testValidateSeqenceNoTaxItems);
  CPPUNIT_TEST(testValidateSeqenceWrongTaxItem);
  CPPUNIT_TEST(testUpdateCalculationDetails);
  CPPUNIT_TEST(testValidateBaseTaxEmptyTaxItemVector);
  CPPUNIT_TEST(testValidateBaseTaxMatchedTaxItem);
  CPPUNIT_TEST(testValidateBaseTaxNotMatchedTaxItem);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    _memHandle.create<TestConfigInitializer>();
  }
  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { Tax tax; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testTaxCase0()
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

    TaxMap taxMap(trx.dataHandle());

    taxMap.initialize();

    Tax& tax = *taxMap.getSpecialTax(0);

    uint16_t startIndex = 0;
    uint16_t endIndex = 0;

    tax.validateLocRestrictions(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    tax.validateTripTypes(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    tax.validateRange(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    tax.validateTransit(trx, taxResponse, taxCodeReg, startIndex);

    tax.validateCarrierExemption(trx, taxResponse, taxCodeReg, startIndex);

    tax.validateEquipmentExemption(trx, taxResponse, taxCodeReg, startIndex);

    tax.validateFareClass(trx, taxResponse, taxCodeReg, startIndex);

    tax.applyTaxOnTax(trx, taxResponse, taxCodeReg);
    //    tax.adjustTax (trx, taxResponse, taxCodeReg);
    tax.doTaxRange(trx, taxResponse, startIndex, endIndex, taxCodeReg);
    tax.doTaxRound(trx, taxCodeReg);
  }

  void testTaxInternationalFareRoundingEmptyAmt()
  {
    Tax tax;
    CurrencyConversionFacade ccFacade;
    PricingTrx trx;
    MoneyAmount targetMoneyAmt;
    Money targetMoney(targetMoneyAmt, "USD");
    MoneyAmount sourceMoneyAmt = 0;
    tax.internationalFareRounding(ccFacade, trx, targetMoney, sourceMoneyAmt);
    CPPUNIT_ASSERT(sourceMoneyAmt == 0);
  }

  void testTaxInternationalFareRounding()
  {
    MoneyAmount moneyAmount = 10.5;
    CurrencyConverter curConverter;
    Money targetMoney(moneyAmount, "USD");

    RoundingFactor roundingUnit = 1.0;
    RoundingRule roundingRule = UP;

    if (curConverter.round(targetMoney, roundingUnit, roundingRule))
    {
      moneyAmount = targetMoney.value();
    }

    CPPUNIT_ASSERT(moneyAmount == 11);
  }

  void testValidateTransferTypeLoc1()
  {

    PricingTrx trx;
    TaxResponse taxResponse;
    TaxCodeReg taxCodeReg;
    PricingRequest request;
    trx.setRequest(&request);

    CPPUNIT_ASSERT(_tax.validateTransferTypeLoc1(trx, taxResponse, taxCodeReg, 0, 0));
  }

  void testValidateSeqencePercentageTax()
  {
    PricingTrx trx;
    TaxResponse taxResponse;
    TaxCodeReg taxCodeReg;
    PricingRequest request;
    trx.setRequest(&request);

    taxCodeReg.taxType() = 'P';
    uint16_t travelSegStartIndex = 0;
    uint16_t travelSegEndIndex = 0;

    CPPUNIT_ASSERT(_tax.validateSequence(
        trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex));
  }

  void testValidateSeqenceNoTaxItems()
  {
    PricingTrx trx;
    TaxResponse taxResponse;
    TaxCodeReg taxCodeReg;
    PricingRequest request;
    trx.setRequest(&request);

    taxCodeReg.taxType() = 'F';
    uint16_t travelSegStartIndex = 0;
    uint16_t travelSegEndIndex = 0;

    CPPUNIT_ASSERT(_tax.validateSequence(
        trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex));
  }

  void testValidateSeqenceWrongTaxItem()
  {
    PricingTrx trx;
    TaxResponse taxResponse;
    TaxCodeReg taxCodeReg;
    PricingRequest request;
    trx.setRequest(&request);

    uint16_t travelSegStartIndex = 0;
    uint16_t travelSegEndIndex = 0;

    // TaxCodeReg taxCodeReg1;
    // taxCodeReg1.taxCode() = "GB";
    TaxItem taxItem;
    taxItem.taxCode() = "GB";
    // taxItem.taxCodeReg() = &taxCodeReg1;
    taxCodeReg.taxType() = 'F';
    taxCodeReg.taxCode() = "GB5";
    taxResponse.taxItemVector().push_back(&taxItem);

    CPPUNIT_ASSERT(_tax.validateSequence(
        trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex));
  }

  void testUpdateCalculationDetails()
  {
    Tax tax;
    TaxItem taxItem;
    taxItem.taxCode() = "GB";
    taxItem.taxAmount() = 10;
    tax.updateCalculationDetails(&taxItem);
    CPPUNIT_ASSERT(tax.calculationDetails().taxableTaxSumAmount == 10);
    TaxItem taxItem2;
    taxItem2.taxCode() = "GB5";
    taxItem2.taxAmount() = 10;
    tax.updateCalculationDetails(&taxItem2);
    CPPUNIT_ASSERT(tax.calculationDetails().taxableTaxSumAmount == 20);
  }

  void testValidateBaseTaxEmptyTaxItemVector()
  {
    PricingTrx trx;
    TaxResponse response;
    TaxOnChangeFee tax;
    TaxCodeReg reg;

    CPPUNIT_ASSERT(tax.validateBaseTax(trx, response, reg) == false);
  }

  void testValidateBaseTaxMatchedTaxItem()
  {
    PricingTrx trx;
    TaxResponse response;
    TaxOnChangeFee tax;
    TaxCodeReg reg;

    reg.specConfigName() = "BASE_TAX";
    tax.getTaxSpecConfig(trx, reg);

    PricingRequest request;
    trx.setRequest(&request);
    trx.getRequest()->ticketingDT() = DateTime(2011, 7, 14, 11, 0, 0);

    TaxItem taxItem;
    response.taxItemVector().push_back(&taxItem);
    CPPUNIT_ASSERT(tax.validateBaseTax(trx, response, reg) == true);
  }

  void testValidateBaseTaxNotMatchedTaxItem()
  {
    PricingTrx trx;
    TaxResponse response;
    TaxOnChangeFee tax;
    TaxCodeReg reg;

    reg.specConfigName() = "";
    tax.getTaxSpecConfig(trx, reg);

    PricingRequest request;
    trx.setRequest(&request);
    trx.getRequest()->ticketingDT() = DateTime(2011, 7, 14, 11, 0, 0);

    TaxItem taxItem;
    response.taxItemVector().push_back(&taxItem);
    CPPUNIT_ASSERT(tax.validateBaseTax(trx, response, reg) == false);
  }

private:
  TestMemHandle _memHandle;
  Tax _tax;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxTest);
}
