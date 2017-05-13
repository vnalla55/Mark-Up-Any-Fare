//-------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/OBFeesUtils.h"

#include <boost/assign/list_of.hpp>

#include "Common/TrxUtil.h"
#include "DataModel/AltPricingDetailObFeesTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/TicketingFeesInfo.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestDataBuilders.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using boost::assign::list_of;

class OBFeesUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OBFeesUtilsTest);

  CPPUNIT_TEST(testPrepareOBFeeMsg);

  CPPUNIT_TEST(testGetLowestObFeeAmountWhenNoCurrency);
  CPPUNIT_TEST(testGetLowestObFeeAmountWhenCurrency);
  CPPUNIT_TEST(testCalculateResidualObFeeAmountWhenNoResidual);
  CPPUNIT_TEST(testCalculateResidualObFeeAmountWhenResidualFop);
  CPPUNIT_TEST(testCalculateResidualObFeeAmountWhenResidual);
  CPPUNIT_TEST(testCalculateResidualObFeeAmountWhenResidualFopNegative);

  CPPUNIT_TEST(testGetFeeRoundingCallControllingNationCodeReturnTrue);
  CPPUNIT_TEST(testGetFeeRoundingDoNotCallControllingNationCodeReturnTrue);

  CPPUNIT_TEST(testCalculateObFeeAmountFromAmountMaxWhenSameCur);
  CPPUNIT_TEST(testComputeMaximumOBFeesPercentWhenNoOB);
  CPPUNIT_TEST(testComputeMaximumOBFeesPercentWhenMax);

  CPPUNIT_TEST(testOBFeesForBALimitNotHit);
  CPPUNIT_TEST(testOBFeesForBAZeroNoFop);
  CPPUNIT_TEST(testOBFeesForBAZeroFop);
  CPPUNIT_TEST(testOBFeesForBANonZero);
  CPPUNIT_TEST(testOBFeesForBAMultiPTCOneMax);
  CPPUNIT_TEST(testOBFeesForBAMultiPTCOneMultiMaxSecond);
  CPPUNIT_TEST(testOBFeesForBAMultiPTCOneMultiMaxFirst);
  CPPUNIT_TEST(testOBFeesForBANoChargeY);
  CPPUNIT_TEST(testOBFeesForBAClearAndGetLast);

  CPPUNIT_TEST(testIsBinCorrectPass);
  CPPUNIT_TEST(testIsBinCorrectFailIf5Digit);
  CPPUNIT_TEST(testIsBinCorrectFailIfAlpha);

  CPPUNIT_TEST(testAddObFeeInfoWpan);

  CPPUNIT_TEST(testSetDefaultValidatingCxrForObFeesWithNoVclInfo);
  CPPUNIT_TEST(testSetDefaultValidatingCxrForObFeesWithDcxInfo);
  CPPUNIT_TEST(testSetDefaultValidatingCxrForObFeesWithNoDcxInfo);

  CPPUNIT_TEST_SUITE_END();

private:
  XMLConstruct* _construct;
  RexPricingTrx* _rexTrx;
  ExchangePricingTrx* _excTrx;
  FareCalcCollector* _rexFareCalcCollector;
  FareCalcCollector* _excFareCalcCollector;
  PricingTrx* _pTrx;
  CountrySettlementPlanInfo* _cspi;
  Itin* _itin;
  Agent* _pAgent;
  PricingRequest* _pRequest;
  PricingOptions* _pOptions;
  MockTseServer* _mTseServer;
  TestMemHandle _memH;
  Billing* _pBilling;
  TicketingFeesInfo* _feeInfo;
  CalcTotals _calcTotals;
  bool _limitMaxOBFees;
public:

  void setUp()
  {
    Message::fillMsgErrCodeMap();
    _mTseServer = _memH.insert(new MockTseServer);
    _construct = _memH.insert(new XMLConstruct);
    _rexTrx = _memH.insert(new RexPricingTrx);
    _excTrx = _memH.insert(new ExchangePricingTrx);
    _rexFareCalcCollector = _memH.insert(new FareCalcCollector);
    _excFareCalcCollector = _memH.insert(new FareCalcCollector);
    _pTrx = _memH.insert(new PricingTrx);
    _pRequest = _memH.insert(new PricingRequest);
    _pOptions = _memH.insert(new PricingOptions);
    _pAgent = _memH.insert(new Agent);
    _pBilling = _memH.insert(new Billing);
    _pOptions->currencyOverride() = "USD";
    _itin = _memH.insert(new Itin);
    _cspi = _memH.insert(new CountrySettlementPlanInfo);
    _pRequest->ticketingAgent() = _pAgent;
    _pTrx->setRequest(_pRequest);
    _pTrx->setOptions(_pOptions);
    _pTrx->itin().push_back(_itin);
    _pTrx->billing() = _pBilling;
    _pTrx->ticketingDate() = DateTime::localTime();
    _pTrx->countrySettlementPlanInfo() = _cspi;
    _limitMaxOBFees = false;

    _pRequest->ticketingDT() = DateTime::localTime();

    _feeInfo = _memH.create<TicketingFeesInfo>();
  }

  void tearDown() { _memH.clear(); }

  void createAgent(Loc& loc, Agent& agent)
  {
    loc.loc() = "DFW";
    loc.subarea() = "1";
    loc.area() = "2";
    loc.nation() = "US";
    loc.state() = "TX";
    loc.cityInd() = true;

    agent.agentLocation() = &loc;
    agent.agentCity() = "DFW";
    agent.tvlAgencyPCC() = "HDQ";
    agent.mainTvlAgencyPCC() = "HDQ";
    agent.tvlAgencyIATA() = "XYZ";
    agent.homeAgencyIATA() = "XYZ";
    agent.agentFunctions() = "XYZ";
    agent.agentDuty() = "XYZ";
    agent.airlineDept() = "XYZ";
    agent.cxrCode() = "AA";
    agent.currencyCodeAgent() = "USD";
    agent.coHostID() = 9;
    agent.agentCommissionType() = "PERCENT";
    agent.agentCommissionAmount() = 10;
  }

  TicketingFeesInfo* createTFInfo(const MoneyAmount amount,
                                  const CurrencyCode& cur,
                                  const FopBinNumber fop)
  {
    TicketingFeesInfo& tfi = *_memH(new TicketingFeesInfo);
    tfi.feeAmount() = amount;
    tfi.noDec() = 2;
    tfi.cur() = cur;
    tfi.serviceTypeCode() = "OB";
    tfi.serviceSubTypeCode() = "FDA";
    tfi.fopBinNumber() = fop;
    return &tfi;
  }

  PaxDetail* createPaxDetail(const MoneyAmount amt,
                             const MoneyAmount taxes,
                             const CurrencyCode cur)
  {
    PaxDetail& pd = *_memH(new PaxDetail);
    pd.totalTaxes() = taxes;
    pd.equivalentAmount() = amt;
    pd.equivalentCurrencyCode() = cur;
    pd.baseFareAmount() = amt;
    pd.baseCurrencyCode() = cur;
    return &pd;
  }

  void testPrepareOBFeeMsg()
  {
    Loc loc;
    createAgent(loc, *_pAgent);
    _pRequest->ticketingAgent() = _pAgent;

    _calcTotals.equivCurrencyCode = "EUR";
    _calcTotals.convertedBaseFareCurrencyCode = "";
    _calcTotals.equivFareAmount = 1000;

    _feeInfo->feeAmount() = 1.55;
    _feeInfo->noDec() = 2;
    _feeInfo->cur() = "EUR";
    _feeInfo->serviceTypeCode() = "OB";
    _feeInfo->serviceSubTypeCode() = "FDA";
    _feeInfo->fopBinNumber() = "528159";

    _pTrx->getRequest()->collectOBFee() = 'Y';
    _pTrx->getRequest()->setCollectTTypeOBFee(true);

    std::string result = OBFeesUtils::prepareObFeeMsg(*_pTrx, _calcTotals, _feeInfo, false);

    std::string expected = " OBFDA - CC NBR BEGINS WITH 528159      1.55        1001.55";

    CPPUNIT_ASSERT(result.find(expected) != std::string::npos);
  }

  void testGetLowestObFeeAmountWhenNoCurrency()
  {
    CurrencyCode maxFeeCur;
    MoneyAmount calcAmount = 100.00;
    MoneyAmount maxAmount = 20.00;

    MoneyAmount lowestAmt = OBFeesUtils::getLowestObFeeAmount(maxFeeCur, calcAmount, maxAmount);
    CPPUNIT_ASSERT_EQUAL(lowestAmt, calcAmount);
  }

  void testGetLowestObFeeAmountWhenCurrency()
  {
    CurrencyCode maxFeeCur = "GBP";
    MoneyAmount calcAmount = 100.00;
    MoneyAmount maxAmount = 20.00;

    MoneyAmount lowestAmt = OBFeesUtils::getLowestObFeeAmount(maxFeeCur, calcAmount, maxAmount);
    CPPUNIT_ASSERT_EQUAL(lowestAmt, maxAmount);
  }

  void testCalculateResidualObFeeAmountWhenNoResidual()
  {
    _pTrx->getRequest()->chargeResidualInd() = false;
    _pTrx->getRequest()->paymentAmountFop() = 1000;
    _feeInfo->feePercent() = 5;
    MoneyAmount totalAmt = 200;
    MoneyAmount testAmount = 50;

    MoneyAmount calcAmt = OBFeesUtils::calculateResidualObFeeAmount(*_pTrx, totalAmt, _feeInfo);

    CPPUNIT_ASSERT_EQUAL(calcAmt, testAmount);
  }

  void testCalculateResidualObFeeAmountWhenResidualFop()
  {
    _pTrx->getRequest()->chargeResidualInd() = true;
    _pTrx->getRequest()->paymentAmountFop() = 100;
    _feeInfo->feePercent() = 5;
    MoneyAmount totalAmt = 300;
    MoneyAmount testAmount = 10;

    MoneyAmount calcAmt = OBFeesUtils::calculateResidualObFeeAmount(*_pTrx, totalAmt, _feeInfo);

    CPPUNIT_ASSERT_EQUAL(calcAmt, testAmount);
  }

  void testCalculateResidualObFeeAmountWhenResidualFopNegative()
  {
    _pTrx->getRequest()->chargeResidualInd() = true;
    _pTrx->getRequest()->paymentAmountFop() = 100;
    _feeInfo->feePercent() = 5;
    MoneyAmount totalAmt = 30;
    MoneyAmount testAmount = 0.0;

    MoneyAmount calcAmt = OBFeesUtils::calculateResidualObFeeAmount(*_pTrx, totalAmt, _feeInfo);

    CPPUNIT_ASSERT_EQUAL(calcAmt, testAmount);
  }

  void testCalculateResidualObFeeAmountWhenResidual()
  {
    _pTrx->getRequest()->chargeResidualInd() = true;
    _pTrx->getRequest()->paymentAmountFop() = 0;
    _feeInfo->feePercent() = 5;
    MoneyAmount totalAmt = 200;
    MoneyAmount testAmount = 10;

    MoneyAmount calcAmt = OBFeesUtils::calculateResidualObFeeAmount(*_pTrx, totalAmt, _feeInfo);

    CPPUNIT_ASSERT_EQUAL(calcAmt, testAmount);
  }

  void testGetFeeRoundingCallControllingNationCodeReturnTrue()
  {
    _pTrx->ticketingDate() = DateTime::localTime();
    CurrencyCode curr = "ZWR";
    RoundingFactor rF = 0;
    CurrencyNoDec rND = 0;
    RoundingRule rR = NONE;
    CPPUNIT_ASSERT(OBFeesUtils::getFeeRounding(*_pTrx, curr, rF, rND, rR));
  }

  void testGetFeeRoundingDoNotCallControllingNationCodeReturnTrue()
  {
    _pTrx->ticketingDate() = DateTime::localTime();
    CurrencyCode curr = "JPY";
    RoundingFactor rF = 0;
    CurrencyNoDec rND = 0;
    RoundingRule rR = NONE;
    CPPUNIT_ASSERT(OBFeesUtils::getFeeRounding(*_pTrx, curr, rF, rND, rR));
  }

  void testCalculateObFeeAmountFromAmountMaxWhenSameCur()
  {
    MoneyAmount feeAmt = 10.0;
    CalcTotals calcTotals;
    calcTotals.equivCurrencyCode = "NUC";
    _feeInfo->feePercent() = 5;
    _feeInfo->feeAmount() = feeAmt;
    _feeInfo->cur() = "NUC";

    MoneyAmount amt =
        OBFeesUtils::calculateObFeeAmountFromAmountMax(*_pTrx, calcTotals, _feeInfo);
    CPPUNIT_ASSERT_EQUAL(feeAmt, amt);
  }

  void testComputeMaximumOBFeesPercentWhenNoOB()
  {
    CalcTotals calcTotals;
    FarePath fp;
    calcTotals.farePath = &fp;

    std::pair<const TicketingFeesInfo*, MoneyAmount> pair =
        OBFeesUtils::computeMaximumOBFeesPercent(*_pTrx, calcTotals);
    CPPUNIT_ASSERT(!pair.first);
  }

  void makeCalcTotals(const std::vector<MoneyAmount>& amounts,
                      const std::vector<CurrencyCode>& currencies,
                      const std::vector<FopBinNumber>& fops)
  {
    FarePath* fp = _memH(new FarePath);
    fp->processed() = true;
    std::vector<MoneyAmount>::const_iterator ai = amounts.begin();
    std::vector<MoneyAmount>::const_iterator aie = amounts.end();
    for (unsigned i = 0; ai != aie; ++ai, ++i)
    {
      fp->collectedTktOBFees().push_back(_memH(new TicketingFeesInfo));
      fp->collectedTktOBFees().back()->feeAmount() = amounts[i];
      fp->collectedTktOBFees().back()->cur() = currencies[i];
      fp->collectedTktOBFees().back()->fopBinNumber() = fops[i];
    }

    CalcTotals* ct = _memH(new CalcTotals);
    ct->equivCurrencyCode = NUC;
    ct->farePath = fp;

    _rexFareCalcCollector->passengerCalcTotals().push_back(ct);

    Itin* itin = _memH(new Itin);
    _pTrx->itin().push_back(itin);
    _pTrx->itin().back()->farePath().push_back(fp);
  }

  void OBFeesForBASetUp()
  {
    std::string configValue;
    Global::config().getValue("PRICING_OPTION_MAX_LIMIT", configValue, "TICKETING_FEE_OPT_MAX");
    TestConfigInitializer::setValue("PRICING_OPTION_MAX_LIMIT", "2", "TICKETING_FEE_OPT_MAX", true);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(2), TrxUtil::getConfigOBFeeOptionMaxLimit());

    _pRequest->collectOBFee() = 'Y';
  }

  void testComputeMaximumOBFeesPercentWhenMax()
  {
    OBFeesForBASetUp();
    MoneyAmount amtMax = 5.0;
    makeCalcTotals(list_of(amtMax)(3.0), list_of(NUC)(NUC), list_of("123")("271"));
    CalcTotals& calcTotals = *(_rexFareCalcCollector->passengerCalcTotals().front());

    std::pair<const TicketingFeesInfo*, MoneyAmount> pair =
        OBFeesUtils::computeMaximumOBFeesPercent(*_pTrx, calcTotals);
    CPPUNIT_ASSERT(pair.first);
    CPPUNIT_ASSERT_EQUAL(pair.second, amtMax);
  }

  void testOBFeesForBALimitNotHit()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(0.0)(0.0), list_of(NUC)(NUC), list_of("")(""));
    OBFeesUtils::checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    CPPUNIT_ASSERT_EQUAL(size_t(2),
                         _rexFareCalcCollector->passengerCalcTotals()
                             .front()
                             ->farePath->collectedTktOBFees()
                             .size());
  }

  void testOBFeesForBAZeroNoFop()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(0.0)(0.0)(0.0), list_of(NUC)(NUC)(NUC), list_of("")("")(""));
    OBFeesUtils::checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    CPPUNIT_ASSERT_EQUAL(size_t(1),
                         _rexFareCalcCollector->passengerCalcTotals()
                             .front()
                             ->farePath->collectedTktOBFees()
                             .size());
  }

  void testOBFeesForBAZeroFop()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(0.0)(0.0)(0.0), list_of(NUC)(NUC)(NUC), list_of("")("")("221B"));
    OBFeesUtils::checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    CPPUNIT_ASSERT(_rexFareCalcCollector->passengerCalcTotals()
                       .front()
                       ->farePath->collectedTktOBFees()
                       .empty());
  }

  void testOBFeesForBANonZero()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(1.0)(2.0)(0.5), list_of(NUC)(NUC)(NUC), list_of("")("")(""));
    OBFeesUtils::checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        2.0,
        _rexFareCalcCollector->passengerCalcTotals().front()->farePath->maximumObFee()->feeAmount(),
        EPSILON);
  }

  void testOBFeesForBAMultiPTCOneMax()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(0.0)(0.0)(0.0), list_of(NUC)(NUC)(NUC), list_of("")("")(""));
    makeCalcTotals(list_of(0.0)(5.0), list_of(NUC)(NUC), list_of("")("314"));
    OBFeesUtils::checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    CPPUNIT_ASSERT(_rexFareCalcCollector->passengerCalcTotals()
                       .front()
                       ->farePath->collectedTktOBFees()
                       .empty());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        5.0,
        _rexFareCalcCollector->passengerCalcTotals().back()->farePath->maximumObFee()->feeAmount(),
        EPSILON);
  }

  void testOBFeesForBAMultiPTCOneMultiMaxSecond()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(0.0)(3.0)(0.0), list_of(NUC)(NUC)(NUC), list_of("")("271")(""));
    makeCalcTotals(list_of(0.0)(5.0), list_of(NUC)(NUC), list_of("")("314"));
    makeCalcTotals(list_of(3.0)(4.5), list_of(NUC)(NUC), list_of("")(""));
    OBFeesUtils::checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    CPPUNIT_ASSERT(_rexFareCalcCollector->passengerCalcTotals()
                       .front()
                       ->farePath->collectedTktOBFees()
                       .empty());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        5.0,
        _rexFareCalcCollector->passengerCalcTotals().back()->farePath->maximumObFee()->feeAmount(),
        EPSILON);
  }

  void testOBFeesForBAMultiPTCOneMultiMaxFirst()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(0.0)(3.0)(8.0), list_of(NUC)(NUC)(NUC), list_of("")("271")(""));
    makeCalcTotals(list_of(0.0)(5.0), list_of(NUC)(NUC), list_of("")("314"));
    OBFeesUtils::checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    CPPUNIT_ASSERT(_rexFareCalcCollector->passengerCalcTotals()
                       .front()
                       ->farePath->collectedTktOBFees()
                       .empty());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        8.0,
        _rexFareCalcCollector->passengerCalcTotals().back()->farePath->maximumObFee()->feeAmount(),
        EPSILON);
  }

  void testOBFeesForBANoChargeY()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(1.0)(0.0)(0.5), list_of(NUC)(NUC)(NUC), list_of("")("")(""));
    _rexFareCalcCollector->passengerCalcTotals().front()->farePath->collectedTktOBFees()
        [1]->noCharge() = 'Y';
    OBFeesUtils::checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    // Two ASSERTs below commented out due to code change in the PricingResponseFormatter.cpp 0n
    // Apr-28-2011
    //        CPPUNIT_ASSERT_EQUAL('Y',
    // _rexFareCalcCollector->passengerCalcTotals().front()->farePath->collectedTktOBFees().front()->noCharge());
    //        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0,
    // _rexFareCalcCollector->passengerCalcTotals().front()->farePath->collectedTktOBFees().front()->feeAmount(),
    // EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        1.0,
        _rexFareCalcCollector->passengerCalcTotals().front()->farePath->maximumObFee()->feeAmount(),
        EPSILON);
    CPPUNIT_ASSERT(_rexFareCalcCollector->passengerCalcTotals()
                       .front()
                       ->farePath->collectedTktOBFees()
                       .empty());
  }

  void testOBFeesForBAClearAndGetLast()
  {
    std::vector<CalcTotals*> ct;

    for (int i = 0; i < 4; ++i)
    {
      ct.push_back(_memH(new CalcTotals));
      FarePath* fp = _memH(new FarePath);
      fp->processed() = true;
      fp->collectedTktOBFees().push_back(_memH(new TicketingFeesInfo));
      ct.back()->farePath = fp;
    }

    CPPUNIT_ASSERT_EQUAL(ct.back()->farePath, OBFeesUtils::clearAllFeesAndGetLastPTC(ct));
    for (int i = 0; i < 4; ++i)
      CPPUNIT_ASSERT(ct[i]->farePath->collectedTktOBFees().empty());
  }

  void testIsBinCorrectPass()
  {
    FopBinNumber fop = "123456";

    CPPUNIT_ASSERT(OBFeesUtils::isBinCorrect(fop));
  }

  void testIsBinCorrectFailIf5Digit()
  {
    FopBinNumber fop = "12345";

    CPPUNIT_ASSERT(!OBFeesUtils::isBinCorrect(fop));
  }

  void testIsBinCorrectFailIfAlpha()
  {
    FopBinNumber fop = "123H56";

    CPPUNIT_ASSERT(!OBFeesUtils::isBinCorrect(fop));
  }

  void testAddObFeeInfoWpan()
  {
    FarePath fp;
    fp.collectedTktOBFees() = { createTFInfo(10.0, "USD", "123456") };
    Itin itin;
    itin.farePath().push_back(&fp);

    AltPricingDetailObFeesTrx trx;
    trx.setRequest(_pRequest);
    _pRequest->collectOBFee() = 'Y';
    trx.itin().push_back(&itin);
    trx.paxDetails().push_back(createPaxDetail(100.0, 0.0, "USD"));
    trx.accompRestrictionVec().resize(1);

    std::string& response = trx.accompRestrictionVec().front().selectionXml();
    response = "<SUM><PXI></PXI></SUM>";

    OBFeesUtils::addObFeeInfoWpan(trx);
    std::string expected =
        "<SUM><PXI>"
        "<OBF SF0=\"OBFDA\" SF1=\"10.00\" STA=\"110.00\" SF2=\"123456\" SF3=\"   \" SF4=\" \"/>"
        "</PXI></SUM>";
    CPPUNIT_ASSERT_EQUAL(expected, response);
  }

  void testSetDefaultValidatingCxrForObFeesWithNoVclInfo()
  {
    PaxDetail pd;
    FarePath fp;
    OBFeesUtils::setDefaultValidatingCxrForObFees(pd, fp);
    CPPUNIT_ASSERT(fp.defaultValidatingCarrier().empty());
  }

  void testSetDefaultValidatingCxrForObFeesWithDcxInfo()
  {
    std::string vclInfo = "<VCL P3L=\"F\" SM0=\"BSP\" VC0=\"T\"><DCX B00=\"9W\" "
                          "TT0=\"ETKTREQ\"><PCX B00=\"BA\" VC1=\"STD\"/></DCX></VCL>";
    PaxDetail pd;
    pd.vclInfo() = vclInfo;
    FarePath fp;
    OBFeesUtils::setDefaultValidatingCxrForObFees(pd, fp);
    CarrierCode cxr = "9W";
    CPPUNIT_ASSERT_EQUAL(fp.defaultValidatingCarrier(), cxr);
  }

  void testSetDefaultValidatingCxrForObFeesWithNoDcxInfo()
  {
    std::string vclInfo = "<VCL P3L=\"F\" SM0=\"BSP\" VC0=\"T\"></VCL>";
    PaxDetail pd;
    pd.vclInfo() = vclInfo;
    FarePath fp;
    OBFeesUtils::setDefaultValidatingCxrForObFees(pd, fp);
    CPPUNIT_ASSERT(fp.defaultValidatingCarrier().empty());
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(OBFeesUtilsTest);
}
