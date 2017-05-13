//
// Copyright Sabre 2012-03-05
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//

#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/ShoppingTrx.h"
#include "Xform/XMLShoppingResponse.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestTaxResponseFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestPricingUnitFactory.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <algorithm>

namespace tse
{
class XMLShoppingResponseTaxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(XMLShoppingResponseTaxTest);
  CPPUNIT_TEST(testGenerateEmptyTAD);
  CPPUNIT_TEST(testGenerateTaxPfc);
  CPPUNIT_TEST(testGenerateTaxPfcSnap);
  CPPUNIT_TEST(testGenerateTaxExempt);
  CPPUNIT_TEST(testGenerateTaxOverrides);
  CPPUNIT_TEST(testGenerateTaxInformation);
  CPPUNIT_TEST(testGenerateTaxInformationSnap);
  CPPUNIT_TEST(testGenerateTaxExchangeInNormalTrx);
  CPPUNIT_TEST(testGenerateTaxExchangeInExchangeTrx);
  CPPUNIT_TEST(testGenerateTaxBreakDown);
  CPPUNIT_TEST(testGenerateTaxBreakDownInSnapTrx);
  CPPUNIT_TEST(testGenerateTaxBreakDownInExchangeTrx);
  CPPUNIT_TEST(testGenerateTaxInfoPerLeg);
  CPPUNIT_TEST(testGenerateTaxInfoPerLegForAward);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  ShoppingTrx* _trx;
  PricingOptions _options;
  PricingRequest _request;
  Agent _agent;
  CalcTotals* _calcTotals;
  Itin* _itin;
  FarePath* _farePath;
  TaxResponse* _taxResponse;

public:
  XMLShoppingResponseTaxTest()
  {
    _agent.agentLocation() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _agent.currencyCodeAgent() = "USD";
    _agent.agentCity() = "DFW";
    _request.ticketingDT() = DateTime(2012, 1, 31);
    _request.ticketingAgent() = &_agent;

    _taxResponse = TestTaxResponseFactory::create("/vobs/atseintl/Xform/test/data/TaxResponse.xml");
    CPPUNIT_ASSERT(_taxResponse);
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<ShoppingTrx>();
    _trx->setRequest(&_request);
    _trx->setOptions(&_options);

    _calcTotals = _memHandle.create<CalcTotals>();
    _calcTotals->convertedBaseFareCurrencyCode = "USD";
    _calcTotals->equivCurrencyCode = "USD";
    _calcTotals->getMutableFcTaxInfo().initialize(_trx, _calcTotals, 0, _taxResponse);

    _itin = _memHandle.create<Itin>();

    _farePath = _memHandle.create<FarePath>();
    _farePath->calculationCurrency() = "USD";
    _farePath->baseFareCurrency() = "USD";
    _farePath->setTotalNUCAmount(50.00);
    _farePath->pricingUnit().push_back(
        TestPricingUnitFactory::create("/vobs/atseintl/Xform/test/data/PricingUnit.xml"));
    _farePath->itin() = _itin;

    CPPUNIT_ASSERT(!_farePath->pricingUnit().front()->fareUsage().empty());
    CPPUNIT_ASSERT(_farePath->pricingUnit().front()->fareUsage().front());
    CPPUNIT_ASSERT(_farePath->pricingUnit().front()->fareUsage().front()->travelSeg().size() == 2);
    _farePath->pricingUnit().front()->fareUsage().front()->travelSeg()[0]->legId() = 0;
    _farePath->pricingUnit().front()->fareUsage().front()->travelSeg()[1]->legId() = 1;
    CPPUNIT_ASSERT(_farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare());
    _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare()->mileage() = 780;
    _calcTotals->farePath = _farePath;
  }

  void tearDown() { _memHandle.clear(); }

  std::string getResult(const XMLShoppingResponse& response)
  {
    static const std::string preamble = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    std::string result = boost::algorithm::trim_copy(response._writer.result());

    CPPUNIT_ASSERT(boost::algorithm::starts_with(result, preamble));
    result.erase(result.begin(), result.begin() + preamble.size());
    return result;
  }

  void testGenerateEmptyTAD()
  {
    XMLShoppingResponse response(*_trx);
    CalcTotals calcTotals;
    TaxResponse* taxResponse = 0;
    response.generateTAD(taxResponse, calcTotals);

    const std::string expected = "<TAD C40=\"\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, getResult(response));
  }

  void testGenerateTaxOverrides()
  {
    TaxOverride t1, t2, t3;
    t1.taxAmt() = 15;
    t1.taxCode() = "WA";
    t2.taxAmt() = 0;
    t2.taxCode() = "WB";
    t3.taxAmt() = 8;
    t3.taxCode() = "WC";
    _request.taxOverride().push_back(&t1);
    _request.taxOverride().push_back(&t2);
    _request.taxOverride().push_back(&t3);
    XMLShoppingResponse response(*_trx);
    response.generateTaxOverrides("USD");

    const std::string expected = "<TAX BC0=\"WA\" C6B=\"15.00\" C41=\"OV\"/>\n"
                                 "<TAX BC0=\"WC\" C6B=\"8.00\" C41=\"OV\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, getResult(response));
  }

  void testGenerateTaxPfc()
  {
    XMLShoppingResponse response(*_trx);
    response.generateTaxPfc(_calcTotals->pfcItems(), _calcTotals->taxCurrencyCode());

    const std::string expected = "<TAX BC0=\"XF\" C6B=\"123.00\" S04=\"PASSENGER FACILITY "
                                 "CHARGES\" S05=\"DFW\" C41=\"USD\" C6A=\"123.00\" A40=\"US\"/>\n"
                                 "<TAX BC0=\"XF\" C6B=\"500.00\" S04=\"PASSENGER FACILITY "
                                 "CHARGES\" S05=\"DFW\" C41=\"USD\" C6A=\"500.00\" A40=\"US\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, getResult(response));
  }

  void testGenerateTaxPfcSnap()
  {
    _trx->snapRequest() = true;
    XMLShoppingResponse response(*_trx);
    response.generateTaxPfc(_calcTotals->pfcItems(), _calcTotals->taxCurrencyCode());

    const std::string expected =
        "<TAX BC0=\"XF\" C6B=\"123.00\" S04=\"PASSENGER FACILITY CHARGES\" S05=\"DFW\" C41=\"USD\" "
        "C6A=\"123.00\" A40=\"US\" B00=\"UA\"/>\n"
        "<TAX BC0=\"XF\" C6B=\"500.00\" S04=\"PASSENGER FACILITY CHARGES\" S05=\"DFW\" C41=\"USD\" "
        "C6A=\"500.00\" A40=\"US\" B00=\"LA\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, getResult(response));
  }

  void testGenerateTaxExempt()
  {
    XMLShoppingResponse response(*_trx);
    response.generateTaxExempt(_calcTotals->getTaxExemptCodes());

    const std::string expected = "<TSM BC0=\"AY\" C6B=\"0\" S05=\"TE\" C41=\"TE\"/>\n"
                                 "<TSM BC0=\"CA\" C6B=\"0\" S05=\"TE\" C41=\"TE\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, getResult(response));
  }

  void testGenerateTaxInformation()
  {
    XMLShoppingResponse response(*_trx);
    std::vector<const PfcItem*> pfcItemsConst(_calcTotals->pfcItems().size());
    pfcItemsConst.assign(_calcTotals->pfcItems().cbegin(), _calcTotals->pfcItems().cend());

    response.generateTaxInformation(
        _calcTotals->taxRecords(), pfcItemsConst, _calcTotals->taxNoDec());

    const std::string expected =
        "<TSM BC0=\"RA\" C6B=\"100.50\" S05=\"\" S04=\"Tax applied.\" C6A=\"0.000000\" C41=\"\" "
        "A40=\"US\"/>\n"
        "<TSM BC0=\"RC\" C6B=\"18.50\" S05=\"\" S04=\"Another tax applied.\" C6A=\"0.000000\" "
        "C41=\"\" A40=\"PL\"/>\n"
        "<TSM BC0=\"RD\" C6B=\"0\" S05=\"TE\" C41=\"TE\"/>\n"
        "<TSM BC0=\"XF\" C6B=\"27.99\" S04=\"XF tax\" C6A=\"0.000000\" C41=\"\" A40=\"US\"/>\n"
        "<TSM BC0=\"XF\" C6B=\"500.00\" S05=\"DFW\" S04=\"XF tax with same value as some pfcitem\" "
        "C6A=\"0.000000\" C41=\"\" A40=\"US\"/>\n"
        "<TSM BC0=\"KR\" C6B=\"500.00\" S05=\"\" S04=\"KR tax with leg and carrier as some item\" "
        "C6A=\"0.000000\" C41=\"\" A40=\"US\"/>\n"
        "<TSM BC0=\"KR\" C6B=\"300.00\" S05=\"\" S04=\"another KR tax with leg and carrier as some "
        "item\" C6A=\"0.000000\" C41=\"\" A40=\"US\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, getResult(response));
  }

  void testGenerateTaxInformationSnap()
  {
    _trx->snapRequest() = true;
    XMLShoppingResponse response(*_trx);

    std::vector<const PfcItem*> pfcItemsConst(_calcTotals->pfcItems().size());
    pfcItemsConst.assign(_calcTotals->pfcItems().cbegin(), _calcTotals->pfcItems().cend());

    response.generateTaxInformation(
        _calcTotals->taxRecords(), pfcItemsConst, _calcTotals->taxNoDec());

    const std::string expected =
        "<TSM BC0=\"RA\" C6B=\"100.50\" B00=\"\" S05=\"\" S04=\"Tax applied.\" C6A=\"0.000000\" "
        "C41=\"\" A40=\"US\"/>\n"
        "<TSM BC0=\"RC\" C6B=\"18.50\" B00=\"\" S05=\"\" S04=\"Another tax applied.\" "
        "C6A=\"0.000000\" C41=\"\" A40=\"PL\"/>\n"
        "<TSM BC0=\"RD\" C6B=\"0\" S05=\"TE\" C41=\"TE\" B00=\"\"/>\n"
        "<TSM BC0=\"XF\" C6B=\"27.99\" B00=\"\" S04=\"XF tax\" C6A=\"0.000000\" C41=\"\" "
        "A40=\"US\"/>\n"
        "<TSM BC0=\"XF\" C6B=\"500.00\" B00=\"\" S04=\"XF tax with same value as some pfcitem\" "
        "C6A=\"0.000000\" C41=\"\" A40=\"US\"/>\n"
        "<TSM BC0=\"KR\" C6B=\"500.00\" B00=\"AA\" S05=\"\" S04=\"KR tax with leg and carrier as "
        "some item\" C6A=\"0.000000\" C41=\"\" A40=\"US\"/>\n"
        "<TSM BC0=\"KR\" C6B=\"300.00\" B00=\"AA\" S05=\"\" S04=\"another KR tax with leg and "
        "carrier as some item\" C6A=\"0.000000\" C41=\"\" A40=\"US\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, getResult(response));
  }

  void testGenerateTaxExchangeInNormalTrx()
  {
    XMLShoppingResponse response(*_trx);
    response.generateTaxExchange(*_calcTotals);

    // in normal transaction generateTaxExchange should do nothing
    const std::string expected = "";
    CPPUNIT_ASSERT_EQUAL(expected, getResult(response));
  }

  void testGenerateTaxExchangeInExchangeTrx()
  {
    _trx->setExcTrxType(PricingTrx::AR_EXC_TRX);
    XMLShoppingResponse response(*_trx);
    response.generateTaxExchange(*_calcTotals);

    const std::string expected =
        "<TBE BC0=\"AY\" C6B=\"0\" A04=\"\" S05=\"\" C40=\"\" A06=\"\" C6F=\"5\"/>\n"
        "<TBE BC0=\"CA2\" C6B=\"0\" A04=\"\" S05=\"\" C40=\"\" A06=\"\" C6D=\"5\" C6E=\"20\" "
        "C47=\"\" C6F=\"10\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, getResult(response));
  }

  void testGenerateTaxBreakDown()
  {
    XMLShoppingResponse response(*_trx);
    response.generateTaxBreakDown(
        _calcTotals->taxItems(), _calcTotals->taxRecords(), _calcTotals->taxNoDec());

    const std::string expected =
        "<TAX BC0=\"XP\" C6B=\"12.50\" S04=\"The description.\" S05=\"DFW\" C41=\"\" "
        "C6A=\"7.000000\"/>\n"
        "<TAX BC0=\"XP\" C6B=\"45.20\" S04=\"The description.\" S05=\"DFW\" C41=\"\" "
        "C6A=\"7.000000\"/>\n"
        "<TAX BC0=\"CA2\" C6B=\"74.30\" S04=\"Another tax.\" S05=\"MUC\" C41=\"EUR\" "
        "C6A=\"7.00\"/>\n"
        "<TAX BC0=\"CA2\" C6B=\"84.20\" S04=\"percentage tax\" S05=\"MUC\" C41=\"USD\" "
        "C6A=\"84.20\"/>\n"
        "<TAX BC0=\"KR\" C6B=\"500.00\" S04=\"multioccc\" S05=\"MUC\" C41=\"USD\" C6A=\"12.34\"/>";

    CPPUNIT_ASSERT_EQUAL(expected, getResult(response));
  }

  void testGenerateTaxBreakDownInSnapTrx()
  {
    _trx->snapRequest() = true;
    XMLShoppingResponse response(*_trx);
    response.generateTaxBreakDown(
        _calcTotals->taxItems(), _calcTotals->taxRecords(), _calcTotals->taxNoDec());

    const std::string expected = "<TAX BC0=\"XP\" C6B=\"12.50\" S04=\"The description.\" "
                                 "S05=\"DFW\" C41=\"\" C6A=\"7.000000\" B00=\"\"/>\n"
                                 "<TAX BC0=\"XP\" C6B=\"45.20\" S04=\"The description.\" "
                                 "S05=\"DFW\" C41=\"\" C6A=\"7.000000\" B00=\"\"/>\n"
                                 "<TAX BC0=\"CA2\" C6B=\"74.30\" S04=\"Another tax.\" S05=\"MUC\" "
                                 "C41=\"EUR\" C6A=\"7.00\" B00=\"\"/>\n"
                                 "<TAX BC0=\"CA2\" C6B=\"84.20\" S04=\"percentage tax\" "
                                 "S05=\"MUC\" C41=\"USD\" C6A=\"84.20\" B00=\"\"/>\n"
                                 "<TAX BC0=\"KR\" C6B=\"500.00\" S04=\"multioccc\" S05=\"MUC\" "
                                 "C41=\"USD\" C6A=\"12.34\" B00=\"AA\"/>";

    CPPUNIT_ASSERT_EQUAL(expected, getResult(response));
  }

  void testGenerateTaxBreakDownInExchangeTrx()
  {
    _trx->setExcTrxType(PricingTrx::AR_EXC_TRX);
    XMLShoppingResponse response(*_trx);
    response.generateTaxBreakDown(
        _calcTotals->taxItems(), _calcTotals->taxRecords(), _calcTotals->taxNoDec());

    const std::string expected =
        "<TAX BC0=\"XP\" C6B=\"12.50\" S04=\"The description.\" S05=\"DFW\" C41=\"\" "
        "C6A=\"7.000000\" PXF=\"F\" PXG=\"F\" PXH=\"F\" A05=\"M\" C6F=\"7\"/>\n"
        "<TAX BC0=\"XP\" C6B=\"45.20\" S04=\"The description.\" S05=\"DFW\" C41=\"\" "
        "C6A=\"7.000000\" PXF=\"F\" PXG=\"F\" PXH=\"F\" C79=\"DEN\" C80=\"88.20\" A05=\"M\" "
        "C6F=\"7\"/>\n"
        "<TAX BC0=\"CA2\" C6B=\"74.30\" S04=\"Another tax.\" S05=\"MUC\" C41=\"EUR\" C6A=\"7.00\" "
        "PXF=\"F\" PXG=\"F\" PXH=\"F\" A05=\"W\" C6D=\"3\" C6E=\"14\" C47=\"EUR\" C6F=\"7\"/>\n"
        "<TAX BC0=\"CA2\" C6B=\"84.20\" S04=\"percentage tax\" S05=\"MUC\" C41=\"USD\" "
        "C6A=\"84.20\" PXF=\"F\" PXG=\"F\" PXH=\"F\" A05=\"P\" C6D=\"3\" C6E=\"9\" C47=\"USD\" "
        "C6F=\"8\"/>\n"
        "<TAX BC0=\"KR\" C6B=\"500.00\" S04=\"multioccc\" S05=\"MUC\" C41=\"USD\" C6A=\"12.34\" "
        "PXF=\"F\" PXG=\"F\" PXH=\"F\" A05=\"P\" C6D=\"3\" C6E=\"9\" C47=\"USD\" C6F=\"8\"/>";

    CPPUNIT_ASSERT_EQUAL(expected, getResult(response));
  }

  void testGenerateTaxInfoPerLeg()
  {
    XMLShoppingResponse response(*_trx);
    response.generateTaxInfoPerLeg(*_calcTotals);
    const std::string expected =
        "<LEG Q14=\"0\">\n"
        "  <TAD C40=\"USD\">\n"
        "    <TAX BC0=\"XP\" C6B=\"45.20\" S04=\"The description.\" S05=\"DFW\" C41=\"\" "
        "C6A=\"7.000000\"/>\n"
        "    <TAX BC0=\"CA2\" C6B=\"74.30\" S04=\"Another tax.\" S05=\"MUC\" C41=\"EUR\" "
        "C6A=\"7.00\"/>\n"
        "    <TAX BC0=\"XF\" C6B=\"123.00\" S04=\"PASSENGER FACILITY CHARGES\" S05=\"DFW\" "
        "C41=\"USD\" C6A=\"123.00\" A40=\"US\"/>\n"
        "    <TAX BC0=\"XF\" C6B=\"500.00\" S04=\"PASSENGER FACILITY CHARGES\" S05=\"DFW\" "
        "C41=\"USD\" C6A=\"500.00\" A40=\"US\"/>\n"
        "    <TSM BC0=\"XP\" C6B=\"45.20\" S05=\"DFW\" S04=\"The description.\" C6A=\"7.000000\" "
        "C41=\"\"/>\n"
        "    <TSM BC0=\"CA2\" C6B=\"74.30\" S05=\"MUC\" S04=\"Another tax.\" C6A=\"7.00\" "
        "C41=\"EUR\"/>\n"
        "    <TSM BC0=\"AY\" C6B=\"0\" S05=\"TE\" C41=\"TE\"/>\n"
        "    <TSM BC0=\"XF\" C6B=\"623.00\" S04=\"PASSENGER FACILITY CHARGE\" C6A=\"123.00\" "
        "C41=\"USD\" A40=\"US\"/>\n"
        "    <TSM BC0=\"AY\" C6B=\"0\" S05=\"TE\" C41=\"TE\"/>\n"
        "    <TSM BC0=\"CA\" C6B=\"0\" S05=\"TE\" C41=\"TE\"/>\n"
        "  </TAD>\n"
        "  <TOT C43=\"USD\" C5E=\"257.75\" C40=\"USD\" C5A=\"257.75\" C45=\"USD\" C5F=\"257.75\" "
        "C46=\"USD\" C65=\"742.50\" C56=\"1000.25\"/>\n"
        "</LEG>\n"
        "<LEG Q14=\"1\">\n"
        "  <TAD C40=\"USD\">\n"
        "    <TAX BC0=\"XP\" C6B=\"12.50\" S04=\"The description.\" S05=\"DFW\" C41=\"\" "
        "C6A=\"7.000000\"/>\n"
        "    <TAX BC0=\"CA2\" C6B=\"84.20\" S04=\"percentage tax\" S05=\"MUC\" C41=\"USD\" "
        "C6A=\"84.20\"/>\n"
        "    <TAX BC0=\"KR\" C6B=\"35.79\" S04=\"multioccc\" S05=\"MUC\" C41=\"USD\" "
        "C6A=\"12.34\"/>\n"
        "    <TSM BC0=\"XP\" C6B=\"12.50\" S05=\"DFW\" S04=\"The description.\" C6A=\"7.000000\" "
        "C41=\"\"/>\n"
        "    <TSM BC0=\"CA2\" C6B=\"84.20\" S05=\"MUC\" S04=\"percentage tax\" C6A=\"8.00\" "
        "C41=\"USD\"/>\n"
        "    <TSM BC0=\"KR\" C6B=\"35.79\" S05=\"MUC\" S04=\"multioccc\" C6A=\"8.00\" "
        "C41=\"USD\"/>\n"
        "  </TAD>\n"
        "  <TOT C43=\"USD\" C5E=\"257.75\" C40=\"USD\" C5A=\"257.75\" C45=\"USD\" C5F=\"257.75\" "
        "C46=\"USD\" C65=\"132.49\" C56=\"390.24\"/>\n"
        "</LEG>";

    CPPUNIT_ASSERT_EQUAL(expected, getResult(response));
  }

  void testGenerateTaxInfoPerLegForAward()
  {
    _trx->awardRequest() = true;
    XMLShoppingResponse response(*_trx);

    response.generateTaxInfoPerLeg(*_calcTotals);
    const std::string expected =
        "<LEG Q14=\"0\">\n"
        "  <TAD C40=\"USD\">\n"
        "    <TAX BC0=\"XP\" C6B=\"45.20\" S04=\"The description.\" S05=\"DFW\" C41=\"\" "
        "C6A=\"7.000000\"/>\n"
        "    <TAX BC0=\"CA2\" C6B=\"74.30\" S04=\"Another tax.\" S05=\"MUC\" C41=\"EUR\" "
        "C6A=\"7.00\"/>\n"
        "    <TAX BC0=\"XF\" C6B=\"123.00\" S04=\"PASSENGER FACILITY CHARGES\" S05=\"DFW\" "
        "C41=\"USD\" C6A=\"123.00\" A40=\"US\"/>\n"
        "    <TAX BC0=\"XF\" C6B=\"500.00\" S04=\"PASSENGER FACILITY CHARGES\" S05=\"DFW\" "
        "C41=\"USD\" C6A=\"500.00\" A40=\"US\"/>\n"
        "    <TSM BC0=\"XP\" C6B=\"45.20\" S05=\"DFW\" S04=\"The description.\" C6A=\"7.000000\" "
        "C41=\"\"/>\n"
        "    <TSM BC0=\"CA2\" C6B=\"74.30\" S05=\"MUC\" S04=\"Another tax.\" C6A=\"7.00\" "
        "C41=\"EUR\"/>\n"
        "    <TSM BC0=\"AY\" C6B=\"0\" S05=\"TE\" C41=\"TE\"/>\n"
        "    <TSM BC0=\"XF\" C6B=\"623.00\" S04=\"PASSENGER FACILITY CHARGE\" C6A=\"123.00\" "
        "C41=\"USD\" A40=\"US\"/>\n"
        "    <TSM BC0=\"AY\" C6B=\"0\" S05=\"TE\" C41=\"TE\"/>\n"
        "    <TSM BC0=\"CA\" C6B=\"0\" S05=\"TE\" C41=\"TE\"/>\n"
        "  </TAD>\n"
        "  <TOT C43=\"USD\" C5E=\"257.75\" C40=\"USD\" C5A=\"257.75\" C45=\"USD\" C5F=\"257.75\" "
        "C46=\"USD\" C65=\"742.50\" C56=\"1000.25\" C5L=\"390\"/>\n"
        "</LEG>\n"
        "<LEG Q14=\"1\">\n"
        "  <TAD C40=\"USD\">\n"
        "    <TAX BC0=\"XP\" C6B=\"12.50\" S04=\"The description.\" S05=\"DFW\" C41=\"\" "
        "C6A=\"7.000000\"/>\n"
        "    <TAX BC0=\"CA2\" C6B=\"84.20\" S04=\"percentage tax\" S05=\"MUC\" C41=\"USD\" "
        "C6A=\"84.20\"/>\n"
        "    <TAX BC0=\"KR\" C6B=\"35.79\" S04=\"multioccc\" S05=\"MUC\" C41=\"USD\" "
        "C6A=\"12.34\"/>\n"
        "    <TSM BC0=\"XP\" C6B=\"12.50\" S05=\"DFW\" S04=\"The description.\" C6A=\"7.000000\" "
        "C41=\"\"/>\n"
        "    <TSM BC0=\"CA2\" C6B=\"84.20\" S05=\"MUC\" S04=\"percentage tax\" C6A=\"8.00\" "
        "C41=\"USD\"/>\n"
        "    <TSM BC0=\"KR\" C6B=\"35.79\" S05=\"MUC\" S04=\"multioccc\" C6A=\"8.00\" "
        "C41=\"USD\"/>\n"
        "  </TAD>\n"
        "  <TOT C43=\"USD\" C5E=\"257.75\" C40=\"USD\" C5A=\"257.75\" C45=\"USD\" C5F=\"257.75\" "
        "C46=\"USD\" C65=\"132.49\" C56=\"390.24\" C5L=\"390\"/>\n"
        "</LEG>";

    CPPUNIT_ASSERT_EQUAL(expected, getResult(response));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(XMLShoppingResponseTaxTest);
}
