#include <string>

#include <boost/assign/list_of.hpp>

#include "Common/CustomerActivationUtil.h"
#include "Common/DateTime.h"
#include "Common/Config/DynamicConfigLoader.h"
#include "Common/TrxUtil.h"
#include "Common/Thread/PriorityQueueTimerTaskExecutor.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TicketingCxrTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{
using boost::assign::list_of;

template <typename T>
class TransactionBuilder
{
  TestMemHandle* _memHandle;
  T* _trx;

public:
  TransactionBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _trx = _memHandle->create<T>();
    setAltTrxType();
    setExcTrxType();
  }

  TransactionBuilder& setSchema(int major, int minor, int revision)
  {
    PricingRequest* request = _memHandle->create<PricingRequest>();
    request->majorSchemaVersion() = major;
    request->minorSchemaVersion() = minor;
    request->revisionSchemaVersion() = revision;

    _trx->setRequest(request);

    return *this;
  }

  TransactionBuilder& setAltTrxType()
  {
    _trx->altTrxType() = PricingTrx::WP;
    return *this;
  }

  TransactionBuilder& setExcTrxType()
  {
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    return *this;
  }

  T* build() { return _trx; }
};

template <>
TransactionBuilder<AltPricingTrx>&
TransactionBuilder<AltPricingTrx>::setAltTrxType()
{
  _trx->altTrxType() = PricingTrx::WPA;
  return *this;
}

template <>
TransactionBuilder<RexPricingTrx>&
TransactionBuilder<RexPricingTrx>::setExcTrxType()
{
  _trx->setExcTrxType(PricingTrx::AR_EXC_TRX);
  return *this;
}

template <>
TransactionBuilder<RefundPricingTrx>&
TransactionBuilder<RefundPricingTrx>::setExcTrxType()
{
  _trx->setExcTrxType(PricingTrx::AF_EXC_TRX);
  return *this;
}

template <>
TransactionBuilder<ExchangePricingTrx>&
TransactionBuilder<ExchangePricingTrx>::setExcTrxType()
{
  _trx->setExcTrxType(PricingTrx::EXC1_WITHIN_ME);
  return *this;
}

class TrxUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TrxUtilTest);
  CPPUNIT_TEST(testBuildFareMarket);
  CPPUNIT_TEST(testTicketingLoc);
  CPPUNIT_TEST(testSaleLoc);

  CPPUNIT_TEST(testIsExchangeOrTicketingTicketEntry);
  CPPUNIT_TEST(testIsExchangeOrTicketingPricingEntryReturnFalse);
  CPPUNIT_TEST(testIsExchangeOrTicketingCancelRestartReturnFalse);
  CPPUNIT_TEST(testIsExchangeOrTicketingCat31ReturnTrue);
  CPPUNIT_TEST(testIsExchangeOrTicketingNewMEReturnTrue);
  CPPUNIT_TEST(testIsExchangeOrTicketingExc1MEReturnTrue);
  CPPUNIT_TEST(testIsExchangeOrTicketingExc2MEReturnTrue);
  CPPUNIT_TEST(testIsExchangeOrTicketingDiagMEReturnTrue);
  CPPUNIT_TEST(testIsExchangeOrTicketingPortExchangeAMtypeReturnFalse);
  CPPUNIT_TEST(testIsExchangeOrTicketingPortExchangeFEtypeReturnTrue);

  CPPUNIT_TEST(testgetTicketingDTCat5_rexExc);
  CPPUNIT_TEST(testgetTicketingDTCat5_rexNew);
  CPPUNIT_TEST(testgetTicketingDTCat5_portExc);

  CPPUNIT_TEST(testIsDisableYYForExcItin_PricingTrx_ConfigYes);
  CPPUNIT_TEST(testIsDisableYYForExcItin_PricingTrx_ConfigNo);

  CPPUNIT_TEST(testIsDisableYYForExcItin_RexPricingTrx_ConfigNo_ExcItinYes);
  CPPUNIT_TEST(testIsDisableYYForExcItin_RexPricingTrx_ConfigYes_ExcItinYes);
  CPPUNIT_TEST(testIsDisableYYForExcItin_RexPricingTrx_ConfigNo_ExcItinNo);
  CPPUNIT_TEST(testIsDisableYYForExcItin_RexPricingTrx_ConfigYes_ExcItinNo);

  CPPUNIT_TEST(testIsDisableYYForExcItin_RefundPricingTrx_ConfigNo_ExcItinYes);
  CPPUNIT_TEST(testIsDisableYYForExcItin_RefundPricingTrx_ConfigYes_ExcItinYes);
  CPPUNIT_TEST(testIsDisableYYForExcItin_RefundPricingTrx_ConfigNo_ExcItinNo);
  CPPUNIT_TEST(testIsDisableYYForExcItin_RefundPricingTrx_ConfigYes_ExcItinNo);

  CPPUNIT_TEST(testCreateTrxAborterCreatesAborter);
  CPPUNIT_TEST(testCreateTrxAborterCreatesTicketingCxrAborter);
  CPPUNIT_TEST(testCreateTrxAborterCreatesTicketingCxrAborterFails);

  CPPUNIT_TEST(testGetTicketingDateReturnsDareFromRequest);
  CPPUNIT_TEST(testGetAlternateCurrencyRetursErrorCodeWhenNoOptions);
  CPPUNIT_TEST(testGetAlternateCurrencyRetursCurrencyFromOptions);

  CPPUNIT_TEST(testTicketingDTCat5ReturnsTicketingDateOfNewTrx);
  CPPUNIT_TEST(testTicketingDTCat5ReturnsCat5ForEx1);
  CPPUNIT_TEST(testTicketingDTCat5ReturnsCat5ForEx2);

  CPPUNIT_TEST(testGetFareMarket_govMatch);
  CPPUNIT_TEST(testGetFareMarket_govMatch_itinNotSet);
  CPPUNIT_TEST(testGetFareMarket_govMatch_anotherExcTrxType);
  CPPUNIT_TEST(testGetFareMarket_govMatch_anotherPhase);
  CPPUNIT_TEST(testGetFareMarket_govMatch_anotherTrxType);
  CPPUNIT_TEST(testGetFareMarket_govMatch_anotherActionCode);

  CPPUNIT_TEST(testGetFareMarket);
  CPPUNIT_TEST(testGetFareMarket_itinNotSet);
  CPPUNIT_TEST(testGetFareMarket_anotherExcTrxType);
  CPPUNIT_TEST(testGetFareMarket_anotherPhase);
  CPPUNIT_TEST(testGetFareMarket_anotherTrxType);
  CPPUNIT_TEST(testGetFareMarket_anotherActionCode);

  CPPUNIT_TEST(testConvertYYYYMMDDDateSuccess);
  CPPUNIT_TEST(testConvertYYYYMMDDDateFail);

  CPPUNIT_TEST(test_configureNetRemitActivationDate_on_cfgDate);
  CPPUNIT_TEST(test_configureNetRemitActivationDate_off_cfgDate);
  CPPUNIT_TEST(test_configureNetRemitActivationDate_mismatch_pcc);

  CPPUNIT_TEST(testIsFullMapRoutingActivated_Apply);
  CPPUNIT_TEST(testIsFullMapRoutingActivated_NotApply);

  CPPUNIT_TEST(testIsAtpcoRbdByCabinAnswerTableActivated_Apply);
  CPPUNIT_TEST(testIsAtpcoRbdByCabinAnswerTableActivated_NotApply);
  CPPUNIT_TEST(testIsAtpcoRbdAnswerTableActivatedShopping);

  CPPUNIT_TEST(testIsIataFareSelectionActivated_Apply);
  CPPUNIT_TEST(testIsIataFareSelectionActivated_NotApply);
  CPPUNIT_TEST(testIsIataFareSelectionApplicable_AncillaryTrx);
  CPPUNIT_TEST(testIsIataFareSelectionApplicable_TktFeesTrx);
  CPPUNIT_TEST(testIsIataFareSelectionApplicable_Exchange);
  CPPUNIT_TEST(testIsIataFareSelectionApplicable_Mip);
  CPPUNIT_TEST(testIsIataFareSelectionApplicable_Repricing);
  CPPUNIT_TEST(testIsIataFareSelectionApplicable_Pricing);
  CPPUNIT_TEST(testIsIataFareSelectionApplicable_GoverningCxrOverride);
  CPPUNIT_TEST(testCat05Override_NotApplyToPCC);
  CPPUNIT_TEST(testCat05Override_ApplyToAllCarriers);

  CPPUNIT_TEST(test_checkSchemaVersionForGlobalDisclosure_PricingRequest_1_1_0);
  CPPUNIT_TEST(test_checkSchemaVersionForGlobalDisclosure_PricingRequest_1_1_1);
  CPPUNIT_TEST(test_checkSchemaVersionForGlobalDisclosure_AltPricingRequest_1_0_0);
  CPPUNIT_TEST(test_checkSchemaVersionForGlobalDisclosure_AltPricingRequest_1_0_1);
  CPPUNIT_TEST(test_checkSchemaVersionForGlobalDisclosure_RexPricingTrx);
  CPPUNIT_TEST(test_checkSchemaVersionForGlobalDisclosure_RefundPricingTrx);
  CPPUNIT_TEST(test_checkSchemaVersionForGlobalDisclosure_ExchangePricingTrx);
  CPPUNIT_TEST(test_checkSchemaVersionForGlobalDisclosure_BaggageTrx_1_0_0);
  CPPUNIT_TEST(test_checkSchemaVersionForGlobalDisclosure_BaggageTrx_1_0_1);

  CPPUNIT_TEST(test_isAutomaticPfcTaxExemptionEnabled);
  CPPUNIT_TEST(test_areUSTaxesOnYQYREnabled);
  CPPUNIT_TEST(test_areUSTaxesOnYQYREnabled_nullTicketingAgent);

  CPPUNIT_TEST(test_IsCat35TFSFApplicable);
  CPPUNIT_SKIP_TEST(test_IsHistorical);
  CPPUNIT_SKIP_TEST(test_Not_IsHistorical);
  CPPUNIT_TEST(testIsFqFareRetailerEnabled);
  CPPUNIT_TEST(test_getOBFeeSubType);
  CPPUNIT_TEST(test_isAAAwardPricing_AAAward);
  CPPUNIT_TEST(test_isAAAwardPricing_TrxType);

  CPPUNIT_TEST(testIsCat35TFSFEnabled_pricing_airlinePcc);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_pricing_notAirlinePcc);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_mip_airlinePcc);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_mip_notAirlinePcc);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_is_airlinePcc);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_is_notAirlinePcc);

  CPPUNIT_TEST(testIsCat35TFSFEnabled_rexPricing_airlinePcc_netSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_rexPricing_notAirlinePcc_netSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_rexPricing_airlinePcc_notNetSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_rexPricing_notAirlinePcc_notNetSelling);

  CPPUNIT_TEST(testIsCat35TFSFEnabled_refundPricing_airlinePcc_netSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_refundPricing_notAirlinePcc_netSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_refundPricing_airlinePcc_notNetSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_refundPricing_notAirlinePcc_notNetSelling);

  CPPUNIT_TEST(testIsCat35TFSFEnabled_exchangePricing_airlinePcc_netSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_exchangePricing_notAirlinePcc_netSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_exchangePricing_airlinePcc_notNetSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_exchangePricing_notAirlinePcc_notNetSelling);

  CPPUNIT_TEST(testIsCat35TFSFEnabled_excMip_airlinePcc_netSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_excMip_notAirlinePcc_netSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_excMip_airlinePcc_notNetSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_excMip_notAirlinePcc_notNetSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_excIs_airlinePcc_netSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_excIs_notAirlinePcc_netSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_excIs_airlinePcc_notNetSelling);
  CPPUNIT_TEST(testIsCat35TFSFEnabled_excIs_notAirlinePcc_notNetSelling);

  CPPUNIT_TEST(testIsJcbCarrierList);

  CPPUNIT_SKIP_TEST(testIsIcerActivated_Apply);
  CPPUNIT_SKIP_TEST(testIsIcerActivated_NotApply);
  CPPUNIT_SKIP_TEST(testIsIcerActivatedWithDate_Apply);
  CPPUNIT_SKIP_TEST(testIsIcerActivatedWithDate_NotApply);

  //  CPPUNIT_TEST(testIntralineAvailabilityApply_shouldFail);
  //  CPPUNIT_TEST(testIntralineAvailabilityApply_shouldPass);
  //  CPPUNIT_TEST(testInterlineAvailabilityApply_shouldFail);
  //  CPPUNIT_TEST(testInterlineAvailabilityApply_shouldPass);

  CPPUNIT_TEST(testIsBspUser_nonGsa_returnTrue);
  CPPUNIT_TEST(testIsBspUser_nonBsp_returnFalse);
  CPPUNIT_TEST(testIsBspUser_bsp_returnTrue);

  CPPUNIT_TEST(testIsAbacusEndorsementHierarchyAtpcoFaresActive);
  CPPUNIT_TEST(testIsDateAdjustmentIndicatorActive);

  CPPUNIT_TEST_SUITE_END();

private:
  static const char* const AGENT_PCC;
  static const char* const AIRLINE_PCC;

  PricingRequest* _request;
  PricingTrx* _trx;
  Agent* _agent;
  TestMemHandle _memH;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _memH.create<RootLoggerGetOff>();

    _trx = _memH.create<PricingTrx>();
    _request = _memH.create<PricingRequest>();
    _trx->setRequest(_request);

    _agent = _memH.create<Agent>();
    _agent->tvlAgencyPCC() = "XYZ";
    _request->ticketingAgent() = _agent;

    _memH.create<MyDataHandle>();

    std::vector<CarrierCode> vec1{"MI", "SQ"};
    std::vector<CarrierCode> vec2{"4M", "LA", "LP", "XL"};
    std::vector<CarrierCode> vec3{"CX", "KA"};

    TrxUtil::intralineAvailabilityCxrs["SQ"] = vec1;
    TrxUtil::intralineAvailabilityCxrs["LAN"] = vec2;
    TrxUtil::intralineAvailabilityCxrs["CX"] = vec3;

    std::vector<CarrierCode> interV1{"4M", "LP", "XL"};
    std::vector<CarrierCode> interV2{"**"};
    std::vector<CarrierCode> interV3{"4M", "LA", "LP"};

    TrxUtil::interlineAvailabilityCxrs["LA"] = interV1;
    TrxUtil::interlineAvailabilityCxrs["LX"] = interV2;
    TrxUtil::interlineAvailabilityCxrs["XL"] = interV3;
    TimerTaskExecutor::setInstance(new PriorityQueueTimerTaskExecutor);
  }

  void tearDown()
  {
    _memH.clear();
    TimerTaskExecutor::destroyInstance();
  }

  void helperYYEnable31(const std::string& val)
  {
    TestConfigInitializer::setValue("REX_EXC_CAT31_YY_ENABLED", val, "FARESC_SVC", true);
  }

  void helperYYEnable33(const std::string& val)
  {
    TestConfigInitializer::setValue("REX_EXC_CAT33_YY_ENABLED", val, "FARESC_SVC", true);
  }

  void testBuildFareMarket()
  {
    // create the travel segments
    //
    Loc locDFW;
    locDFW.loc() = "DFW";
    locDFW.subarea() = "1";
    locDFW.area() = "2";
    locDFW.nation() = "US";
    locDFW.state() = "TX";
    locDFW.cityInd() = true;

    Loc locATL;
    locATL.loc() = "ATL";
    locATL.subarea() = "1";
    locATL.area() = "2";
    locATL.nation() = "US";
    locATL.state() = "GA";
    locATL.cityInd() = true;

    Loc locNYC;
    locNYC.loc() = "NYC";
    locNYC.subarea() = "1";
    locNYC.area() = "2";
    locNYC.nation() = "US";
    locNYC.state() = "NY";
    locNYC.cityInd() = true;

    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = &locDFW;
    air1.destination() = &locATL;

    AirSeg air2;
    air2.pnrSegment() = 2;
    air2.segmentOrder() = 2;
    air2.origin() = &locATL;
    air2.destination() = &locNYC;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);
    _trx->travelSeg().push_back(&air2);

    // call buildFareMarket
    //
    TrxUtil::buildFareMarket(*_trx, _trx->travelSeg());

    // test to see that the fare markets got built
    //
    CPPUNIT_ASSERT_MESSAGE("Error No Fare Markets Built", _trx->fareMarket().size());
  }

  void testTicketingLoc()
  {
    Loc* locNYC;
    _trx->dataHandle().get(locNYC);
    locNYC->loc() = "NYC";
    locNYC->subarea() = "1";
    locNYC->area() = "2";
    locNYC->nation() = "US";
    locNYC->state() = "NY";
    locNYC->cityInd() = true;

    _agent->agentLocation() = locNYC;
    _agent->currencyCodeAgent() = "USD";

    const Loc* ticketLoc = TrxUtil::ticketingLoc(*_trx);

    CPPUNIT_ASSERT_MESSAGE("Error testTicketingLoc1", ticketLoc->loc() == "NYC");
  }

  void testSaleLoc()
  {
    Loc* locNYC;
    _trx->dataHandle().get(locNYC);
    locNYC->loc() = "NYC";
    locNYC->subarea() = "1";
    locNYC->area() = "2";
    locNYC->nation() = "US";
    locNYC->state() = "NY";
    locNYC->cityInd() = true;

    _agent->agentLocation() = locNYC;
    _agent->currencyCodeAgent() = "USD";

    const Loc* ticketLoc = TrxUtil::saleLoc(*_trx);

    CPPUNIT_ASSERT_MESSAGE("Error testSaleLoc1", ticketLoc->loc() == "NYC");
  }

  void testIsExchangeOrTicketingTicketEntry()
  {
    _request->ticketEntry() = 'Y';
    CPPUNIT_ASSERT(TrxUtil::isExchangeOrTicketing(*_trx));
  }

  void testIsExchangeOrTicketingPricingEntryReturnFalse()
  {
    _request->ticketEntry() = 'N';
    CPPUNIT_ASSERT(!TrxUtil::isExchangeOrTicketing(*_trx));
  }

  void testIsExchangeOrTicketingCancelRestartReturnFalse()
  {
    _request->ticketEntry() = 'N';
    _trx->setExcTrxType(PricingTrx::CSO_EXC_TRX);
    CPPUNIT_ASSERT(!TrxUtil::isExchangeOrTicketing(*_trx));
  }

  void testIsExchangeOrTicketingCat31ReturnTrue()
  {
    _request->ticketEntry() = 'N';
    _trx->setExcTrxType(PricingTrx::AR_EXC_TRX);
    CPPUNIT_ASSERT(TrxUtil::isExchangeOrTicketing(*_trx));
  }

  void testIsExchangeOrTicketingNewMEReturnTrue()
  {
    _request->ticketEntry() = 'N';
    _trx->setExcTrxType(PricingTrx::NEW_WITHIN_ME);
    CPPUNIT_ASSERT(TrxUtil::isExchangeOrTicketing(*_trx));
  }

  void testIsExchangeOrTicketingExc1MEReturnTrue()
  {
    _request->ticketEntry() = 'N';
    _trx->setExcTrxType(PricingTrx::EXC1_WITHIN_ME);
    CPPUNIT_ASSERT(TrxUtil::isExchangeOrTicketing(*_trx));
  }

  void testIsExchangeOrTicketingExc2MEReturnTrue()
  {
    _request->ticketEntry() = 'N';
    _trx->setExcTrxType(PricingTrx::EXC2_WITHIN_ME);
    CPPUNIT_ASSERT(TrxUtil::isExchangeOrTicketing(*_trx));
  }

  void testIsExchangeOrTicketingDiagMEReturnTrue()
  {
    _request->ticketEntry() = 'N';
    _trx->setExcTrxType(PricingTrx::ME_DIAG_TRX);
    CPPUNIT_ASSERT(TrxUtil::isExchangeOrTicketing(*_trx));
  }

  void testIsExchangeOrTicketingPortExchangeAMtypeReturnFalse()
  {
    ExchangePricingTrx trx;
    trx.reqType() = AGENT_PRICING_MASK;
    trx.setExcTrxType(PricingTrx::PORT_EXC_TRX);
    trx.setRequest(_request);
    _request->ticketEntry() = 'N';
    CPPUNIT_ASSERT(!TrxUtil::isExchangeOrTicketing(trx));
  }

  void testIsExchangeOrTicketingPortExchangeFEtypeReturnTrue()
  {
    ExchangePricingTrx trx;
    trx.reqType() = FULL_EXCHANGE;
    trx.setExcTrxType(PricingTrx::PORT_EXC_TRX);
    trx.setRequest(_request);
    _request->ticketEntry() = 'N';
    CPPUNIT_ASSERT(TrxUtil::isExchangeOrTicketing(trx));
  }

  void testgetTicketingDTCat5_rexExc()
  {
    RexPricingTrx rexTrx;
    RexPricingOptions opt;
    rexTrx.setOptions(&opt);
    rexTrx.setAnalyzingExcItin(true);
    DateTime checkedDate(2008, 03, 31);
    rexTrx.setOriginalTktIssueDT() = checkedDate;
    DateTime returnedDate = TrxUtil::getTicketingDTCat5(rexTrx);
    CPPUNIT_ASSERT_EQUAL(checkedDate, returnedDate);
  }

  void testgetTicketingDTCat5_rexNew()
  {
    RexPricingTrx rexTrx;
    RexPricingOptions opt;
    rexTrx.setOptions(&opt);
    rexTrx.setAnalyzingExcItin(false);
    DateTime checkedDate(2008, 03, 31);
    rexTrx.currentTicketingDT() = checkedDate;
    DateTime returnedDate = TrxUtil::getTicketingDTCat5(rexTrx);
    CPPUNIT_ASSERT_EQUAL(checkedDate, returnedDate);
  }

  void testgetTicketingDTCat5_portExc()
  {
    ExchangePricingTrx excTrx;
    DateTime checkedDate(2008, 03, 31);
    excTrx.currentTicketingDT() = checkedDate;
    DateTime returnedDate = TrxUtil::getTicketingDTCat5(excTrx);
    CPPUNIT_ASSERT_EQUAL(checkedDate, returnedDate);
  }

  void testJourneyMileageApplyReturnFalseWhenCarrierNotInConfig()
  {
    std::string cfgCxr = "AA|CO|MX|NW";
    TestConfigInitializer::setValue("JOURNEY_MILEAGE_CARRIERS", cfgCxr, "PRICING_SVC");
    CPPUNIT_ASSERT(!TrxUtil::journeyMileageApply("SQ"));
  }

  void testJourneyMileageApplyReturnTrueWhenCarrierInConfig()
  {
    std::string cfgCxr = "AA|CO|MX|NW";
    TestConfigInitializer::setValue("JOURNEY_MILEAGE_CARRIERS", cfgCxr, "PRICING_SVC");
    CPPUNIT_ASSERT(TrxUtil::journeyMileageApply("NW"));
  }

  void testIntralineAvailabilityApply_shouldFail()
  {
    _trx->setTrxType(PricingTrx::MIP_TRX);
    Billing billing;
    billing.actionCode() = "WFR";
    _trx->billing() = &billing;

    // Current and next segment carriers from different groups
    CPPUNIT_ASSERT(!TrxUtil::intralineAvailabilityApply(*_trx, "MI", "LP"));
    // One intraline and another non-intraline carrier
    CPPUNIT_ASSERT(!TrxUtil::intralineAvailabilityApply(*_trx, "LA", "EK"));
    // Not intraline carriers
    CPPUNIT_ASSERT(!TrxUtil::intralineAvailabilityApply(*_trx, "AA", "EK"));
  }

  void testIntralineAvailabilityApply_shouldPass()
  {
    _trx->setTrxType(PricingTrx::MIP_TRX);
    Billing billing;
    billing.actionCode() = "WFR";
    _trx->billing() = &billing;

    CPPUNIT_ASSERT(TrxUtil::intralineAvailabilityApply(*_trx, "MI", "SQ"));
    CPPUNIT_ASSERT(TrxUtil::intralineAvailabilityApply(*_trx, "LP", "4M"));
    CPPUNIT_ASSERT(TrxUtil::intralineAvailabilityApply(*_trx, "LA", "XL"));
  }

  void testInterlineAvailabilityApply_shouldFail()
  {
    _trx->setTrxType(PricingTrx::MIP_TRX);
    // ZZ is not an interline carrier
    CPPUNIT_ASSERT(!TrxUtil::interlineAvailabilityApply(*_trx, "ZZ", "LP"));
    // LA is interline carrier only when partners are "4M","LP",or "XL"
    CPPUNIT_ASSERT(!TrxUtil::interlineAvailabilityApply(*_trx, "LA", "5M"));
    // XL is interline carrier only when partners are "4M","LA",or "LP"
    CPPUNIT_ASSERT(!TrxUtil::interlineAvailabilityApply(*_trx, "XL", "KA"));
  }

  void testInterlineAvailabilityApply_shouldPass()
  {
    _trx->setTrxType(PricingTrx::MIP_TRX);
    // LX partners with any carrier
    CPPUNIT_ASSERT(TrxUtil::interlineAvailabilityApply(*_trx, "LX", "88"));
    // LA is interline carrier when partners are "4M","LP",or "XL"
    CPPUNIT_ASSERT(TrxUtil::interlineAvailabilityApply(*_trx, "LA", "4M"));
    // XL is interline carrier when partners are "4M","LA",or "LP"
    CPPUNIT_ASSERT(TrxUtil::interlineAvailabilityApply(*_trx, "XL", "LP"));
  }

  void enableAbacus()
  {
    TrxUtil::enableAbacus();
    Customer* tjr = _memH.create<Customer>();
    tjr->crsCarrier() = "1B";
    tjr->hostName() = "ABAC";
    _agent->agentTJR() = tjr;
  }

  void setGNRTestData(DateTime activDate)
  {
    std::string projCode = "GNR";
    ActivationResult* acResult = _memH.create<ActivationResult>();
    acResult->finalActvDate() = activDate;

    acResult->isActivationFlag() = true;
    _trx->projCACMapData().insert(std::make_pair(projCode, acResult));
  }

  void testIsDisableYYForExcItin_PricingTrx_ConfigYes()
  {
    helperYYEnable31("Y");
    helperYYEnable33("Y");
    CPPUNIT_ASSERT(!TrxUtil::isDisableYYForExcItin(*_trx));
  }

  void testIsDisableYYForExcItin_PricingTrx_ConfigNo()
  {
    helperYYEnable31("N");
    helperYYEnable33("N");
    CPPUNIT_ASSERT(!TrxUtil::isDisableYYForExcItin(*_trx));
  }

  void testIsDisableYYForExcItin_RexPricingTrx_ConfigNo_ExcItinYes()
  {
    RexPricingTrx trx;
    RexPricingOptions opt;
    trx.setOptions(&opt);
    helperYYEnable31("N");
    trx.setAnalyzingExcItin(true);
    CPPUNIT_ASSERT(TrxUtil::isDisableYYForExcItin(trx));
  }

  void testIsDisableYYForExcItin_RexPricingTrx_ConfigYes_ExcItinYes()
  {
    RexPricingTrx trx;
    RexPricingOptions opt;
    trx.setOptions(&opt);
    helperYYEnable31("Y");
    trx.setAnalyzingExcItin(true);
    CPPUNIT_ASSERT(!TrxUtil::isDisableYYForExcItin(trx));
  }

  void testIsDisableYYForExcItin_RexPricingTrx_ConfigNo_ExcItinNo()
  {
    RexPricingTrx trx;
    RexPricingOptions opt;
    trx.setOptions(&opt);
    helperYYEnable31("N");
    trx.setAnalyzingExcItin(false);
    CPPUNIT_ASSERT(!TrxUtil::isDisableYYForExcItin(trx));
  }

  void testIsDisableYYForExcItin_RexPricingTrx_ConfigYes_ExcItinNo()
  {
    RexPricingTrx trx;
    RexPricingOptions opt;
    trx.setOptions(&opt);
    helperYYEnable31("Y");
    trx.setAnalyzingExcItin(false);
    CPPUNIT_ASSERT(!TrxUtil::isDisableYYForExcItin(trx));
  }

  void testIsDisableYYForExcItin_RefundPricingTrx_ConfigNo_ExcItinYes()
  {
    RefundPricingTrx trx;
    RexPricingOptions opt;
    trx.setOptions(&opt);
    helperYYEnable33("N");
    trx.setAnalyzingExcItin(true);
    CPPUNIT_ASSERT(TrxUtil::isDisableYYForExcItin(trx));
  }

  void testIsDisableYYForExcItin_RefundPricingTrx_ConfigYes_ExcItinYes()
  {
    RefundPricingTrx trx;
    RexPricingOptions opt;
    trx.setOptions(&opt);
    helperYYEnable33("Y");
    trx.setAnalyzingExcItin(true);
    CPPUNIT_ASSERT(!TrxUtil::isDisableYYForExcItin(trx));
  }

  void testIsDisableYYForExcItin_RefundPricingTrx_ConfigNo_ExcItinNo()
  {
    RefundPricingTrx trx;
    RexPricingOptions opt;
    trx.setOptions(&opt);
    helperYYEnable33("N");
    trx.setAnalyzingExcItin(false);
    CPPUNIT_ASSERT(!TrxUtil::isDisableYYForExcItin(trx));
  }

  void testIsDisableYYForExcItin_RefundPricingTrx_ConfigYes_ExcItinNo()
  {
    RefundPricingTrx trx;
    RexPricingOptions opt;
    trx.setOptions(&opt);
    helperYYEnable33("Y");
    trx.setAnalyzingExcItin(false);
    CPPUNIT_ASSERT(!TrxUtil::isDisableYYForExcItin(trx));
  }

  void testCreateTrxAborterCreatesAborter()
  {
    std::string cfgTime = "100";
    TestConfigInitializer::setValue("TRX_TIMEOUT", cfgTime, "TSE_SERVER");
    TrxUtil::createTrxAborter(*_trx);
    CPPUNIT_ASSERT(_trx->aborter());
  }

  void testCreateTrxAborterCreatesTicketingCxrAborter()
  {
    TicketingCxrTrx tcsTrx;
    std::string cfgTime = "180";
    TestConfigInitializer::setValue("TCS_TIMEOUT", cfgTime, "TSE_SERVER");
    TrxUtil::createTrxAborter(tcsTrx);
    CPPUNIT_ASSERT(tcsTrx.aborter());
  }

  void testCreateTrxAborterCreatesTicketingCxrAborterFails()
  {
    TestConfigInitializer::setValue("TCS_TIMEOUT", 0, "TSE_SERVER");
    TicketingCxrTrx tcsTrx;
    TrxUtil::createTrxAborter(tcsTrx);
    CPPUNIT_ASSERT(!tcsTrx.aborter());
  }

  void testGetTicketingDateReturnsDareFromRequest()
  {
    DateTime tktDate = DateTime(2009, 10, 23, 8, 15, 0);
    _request->ticketingDT() = tktDate;
    CPPUNIT_ASSERT_EQUAL(tktDate, TrxUtil::getTicketingDT(*_trx));
  }

  void testGetAlternateCurrencyRetursErrorCodeWhenNoOptions()
  {
    _trx->setOptions(0);
    CPPUNIT_ASSERT_EQUAL(TrxUtil::errorCCode, TrxUtil::getAlternateCurrency(*_trx));
  }

  void testGetAlternateCurrencyRetursCurrencyFromOptions()
  {
    PricingOptions options;
    options.alternateCurrency() = USD;
    _trx->setOptions(&options);
    CPPUNIT_ASSERT_EQUAL(USD, TrxUtil::getAlternateCurrency(*_trx));
  }

  void testTicketingDTCat5ReturnsTicketingDateOfNewTrx()
  {
    DateTime tktDate = DateTime(2009, 10, 23, 8, 15, 0);
    _request->ticketingDT() = tktDate;
    MultiExchangeTrx meTrx;
    meTrx.newPricingTrx() = _trx;
    _trx->setParentTrx(&meTrx);
    CPPUNIT_ASSERT_EQUAL(tktDate, TrxUtil::getTicketingDTCat5(*_trx));
  }

  void testTicketingDTCat5ReturnsCat5ForEx1()
  {
    DateTime exDate = DateTime(2009, 10, 23, 8, 15, 0);
    MultiExchangeTrx meTrx;
    meTrx.excPricingTrx1() = _trx;
    meTrx.cat5TktDT_Ex1() = exDate;
    _trx->setParentTrx(&meTrx);
    CPPUNIT_ASSERT_EQUAL(exDate, TrxUtil::getTicketingDTCat5(*_trx));
  }

  void testTicketingDTCat5ReturnsCat5ForEx2()
  {
    DateTime exDate = DateTime(2009, 10, 23, 8, 15, 0);
    MultiExchangeTrx meTrx;
    meTrx.excPricingTrx2() = _trx;
    meTrx.cat5TktDT_Ex2() = exDate;
    _trx->setParentTrx(&meTrx);
    CPPUNIT_ASSERT_EQUAL(exDate, TrxUtil::getTicketingDTCat5(*_trx));
  }

  void testGetFareMarket_govMatch_itinNotSet()
  {
    RexExchangeTrx trx;
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    trx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
    trx.setTrxType(PricingTrx::MIP_TRX);
    Billing billing;
    billing.actionCode() = "WFR";
    trx.billing() = &billing;
    const CarrierCode carrier("AA");
    DateTime date(2009, 10, 27);
    Itin itin;
    Itin newItin;
    FareMarket fareMarket;
    std::vector<TravelSeg*> travelSegs;

    trx.newItin().push_back(&newItin);

    const FareMarket* returndFareMarket = TrxUtil::getFareMarket(trx, carrier, travelSegs, date);

    CPPUNIT_ASSERT(0 == returndFareMarket);
  }

  void testGetFareMarket_govMatch_anotherExcTrxType()
  {
    RexExchangeTrx trx;
    trx.setExcTrxType(PricingTrx::NOT_EXC_TRX);
    trx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
    trx.setTrxType(PricingTrx::MIP_TRX);
    Billing billing;
    billing.actionCode() = "WFR";
    trx.billing() = &billing;
    const CarrierCode carrier("AA");
    DateTime date(2009, 10, 27);
    Itin itin;
    Itin newItin;
    FareMarket fareMarket;
    std::vector<TravelSeg*> travelSegs;

    trx.newItin().push_back(&newItin);

    const FareMarket* returndFareMarket =
        TrxUtil::getFareMarket(trx, carrier, travelSegs, date, &itin);

    CPPUNIT_ASSERT(0 == returndFareMarket);
  }

  void testGetFareMarket_govMatch_anotherPhase()
  {
    RexExchangeTrx trx;
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    trx.trxPhase() = RexPricingTrx::REPRICE_EXCITIN_PHASE;
    trx.setTrxType(PricingTrx::MIP_TRX);
    Billing billing;
    billing.actionCode() = "WFR";
    trx.billing() = &billing;
    const CarrierCode carrier("AA");
    DateTime date(2009, 10, 27);
    Itin itin;
    Itin newItin;
    FareMarket fareMarket;
    std::vector<TravelSeg*> travelSegs;

    trx.newItin().push_back(&newItin);

    const FareMarket* returndFareMarket =
        TrxUtil::getFareMarket(trx, carrier, travelSegs, date, &itin);

    CPPUNIT_ASSERT(0 == returndFareMarket);
  }

  void testGetFareMarket_govMatch_anotherTrxType()
  {
    RexExchangeTrx trx;
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    trx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
    trx.setTrxType(PricingTrx::PRICING_TRX);
    Billing billing;
    billing.actionCode() = "WFR";
    trx.billing() = &billing;
    const CarrierCode carrier("AA");
    DateTime date(2009, 10, 27);
    Itin itin;
    Itin newItin;
    FareMarket fareMarket;
    std::vector<TravelSeg*> travelSegs;

    trx.newItin().push_back(&newItin);

    const FareMarket* returndFareMarket =
        TrxUtil::getFareMarket(trx, carrier, travelSegs, date, &itin);

    CPPUNIT_ASSERT(0 == returndFareMarket);
  }

  void testGetFareMarket_govMatch_anotherActionCode()
  {
    RexExchangeTrx trx;
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    trx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
    trx.setTrxType(PricingTrx::MIP_TRX);
    Billing billing;
    billing.actionCode() = "WFR.C";
    trx.billing() = &billing;
    const CarrierCode carrier("AA");
    DateTime date(2009, 10, 27);
    Itin itin;
    Itin newItin;
    FareMarket fareMarket;
    std::vector<TravelSeg*> travelSegs;

    trx.newItin().push_back(&newItin);

    const FareMarket* returndFareMarket =
        TrxUtil::getFareMarket(trx, carrier, travelSegs, date, &itin);

    CPPUNIT_ASSERT(0 == returndFareMarket);
  }

  void testGetFareMarket_govMatch()
  {
    RexExchangeTrx trx;
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    trx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
    trx.setTrxType(PricingTrx::MIP_TRX);
    Billing billing;
    billing.actionCode() = "WFR";
    trx.billing() = &billing;
    const CarrierCode carrier("AA");
    DateTime date(2009, 10, 27);
    Itin itin;
    trx.newItin().push_back(&itin);
    FareMarket fareMarket;
    std::vector<TravelSeg*> travelSegs;

    itin.fareMarket().push_back(&fareMarket);
    FareMarket::RetrievalInfo retrievalInfo;
    retrievalInfo._date = DateTime(2009, 10, 27);
    fareMarket.retrievalInfo() = &retrievalInfo;
    fareMarket.governingCarrier() = "AA";

    Loc locDFW, locATL, locNYC;

    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = &locDFW;
    air1.destination() = &locATL;

    AirSeg air2;
    air2.pnrSegment() = 2;
    air2.segmentOrder() = 2;
    air2.origin() = &locATL;
    air2.destination() = &locNYC;

    fareMarket.travelSeg().push_back(&air1);
    fareMarket.travelSeg().push_back(&air2);
    travelSegs.push_back(&air1);
    travelSegs.push_back(&air2);

    const FareMarket* returndFareMarket =
        TrxUtil::getFareMarket(trx, carrier, travelSegs, date, &itin);

    CPPUNIT_ASSERT(&fareMarket == returndFareMarket);
  }

  void testGetFareMarket_itinNotSet()
  {
    RexExchangeTrx trx;
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    trx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
    trx.setTrxType(PricingTrx::MIP_TRX);
    Billing billing;
    billing.actionCode() = "WFR";
    trx.billing() = &billing;
    DateTime date(2009, 10, 27);
    Itin itin;
    Itin newItin;
    FareMarket fareMarket;
    std::vector<TravelSeg*> travelSegs;

    trx.newItin().push_back(&newItin);

    std::vector<FareMarket*> retFareMarket;
    TrxUtil::getFareMarket(trx, travelSegs, date, retFareMarket);

    CPPUNIT_ASSERT(retFareMarket.empty());
  }

  void testGetFareMarket_anotherExcTrxType()
  {
    RexExchangeTrx trx;
    trx.setExcTrxType(PricingTrx::NOT_EXC_TRX);
    trx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
    trx.setTrxType(PricingTrx::MIP_TRX);
    Billing billing;
    billing.actionCode() = "WFR";
    trx.billing() = &billing;
    DateTime date(2009, 10, 27);
    Itin itin;
    Itin newItin;
    FareMarket fareMarket;
    std::vector<TravelSeg*> travelSegs;

    trx.newItin().push_back(&newItin);

    std::vector<FareMarket*> retFareMarket;
    TrxUtil::getFareMarket(trx, travelSegs, date, retFareMarket, &itin);

    CPPUNIT_ASSERT(retFareMarket.empty());
  }

  void testGetFareMarket_anotherPhase()
  {
    RexExchangeTrx trx;
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    trx.trxPhase() = RexPricingTrx::REPRICE_EXCITIN_PHASE;
    trx.setTrxType(PricingTrx::MIP_TRX);
    Billing billing;
    billing.actionCode() = "WFR";
    trx.billing() = &billing;
    DateTime date(2009, 10, 27);
    Itin itin;
    Itin newItin;
    FareMarket fareMarket;
    std::vector<TravelSeg*> travelSegs;

    trx.newItin().push_back(&newItin);

    std::vector<FareMarket*> retFareMarket;
    TrxUtil::getFareMarket(trx, travelSegs, date, retFareMarket, &itin);

    CPPUNIT_ASSERT(retFareMarket.empty());
  }

  void testGetFareMarket_anotherTrxType()
  {
    RexExchangeTrx trx;
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    trx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
    trx.setTrxType(PricingTrx::PRICING_TRX);
    Billing billing;
    billing.actionCode() = "WFR";
    trx.billing() = &billing;
    DateTime date(2009, 10, 27);
    Itin itin;
    Itin newItin;
    FareMarket fareMarket;
    std::vector<TravelSeg*> travelSegs;

    trx.newItin().push_back(&newItin);

    std::vector<FareMarket*> retFareMarket;
    TrxUtil::getFareMarket(trx, travelSegs, date, retFareMarket, &itin);

    CPPUNIT_ASSERT(retFareMarket.empty());
  }

  void testGetFareMarket_anotherActionCode()
  {
    RexExchangeTrx trx;
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    trx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
    trx.setTrxType(PricingTrx::MIP_TRX);
    Billing billing;
    billing.actionCode() = "WFR.C";
    trx.billing() = &billing;
    DateTime date(2009, 10, 27);
    Itin itin;
    Itin newItin;
    FareMarket fareMarket;
    std::vector<TravelSeg*> travelSegs;

    trx.newItin().push_back(&newItin);

    std::vector<FareMarket*> retFareMarket;
    TrxUtil::getFareMarket(trx, travelSegs, date, retFareMarket, &itin);

    CPPUNIT_ASSERT(retFareMarket.empty());
  }

  void testGetFareMarket()
  {
    RexExchangeTrx trx;
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    trx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
    trx.setTrxType(PricingTrx::MIP_TRX);
    Billing billing;
    billing.actionCode() = "WFR";
    trx.billing() = &billing;
    DateTime date(2009, 10, 27);
    Itin itin;
    FareMarket fareMarket;
    std::vector<TravelSeg*> travelSegs;

    itin.fareMarket().push_back(&fareMarket);
    FareMarket::RetrievalInfo retrievalInfo;
    retrievalInfo._date = DateTime(2009, 10, 27);
    fareMarket.retrievalInfo() = &retrievalInfo;

    Loc locDFW, locATL, locNYC;

    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = &locDFW;
    air1.destination() = &locATL;

    AirSeg air2;
    air2.pnrSegment() = 2;
    air2.segmentOrder() = 2;
    air2.origin() = &locATL;
    air2.destination() = &locNYC;

    fareMarket.travelSeg().push_back(&air1);
    fareMarket.travelSeg().push_back(&air2);
    travelSegs.push_back(&air1);
    travelSegs.push_back(&air2);

    std::vector<FareMarket*> retFareMarket;
    TrxUtil::getFareMarket(trx, travelSegs, date, retFareMarket, &itin);

    CPPUNIT_ASSERT(&fareMarket == retFareMarket.front());
  }

  void testConvertYYYYMMDDDateSuccess()
  {
    std::string dateStr("2009-12-18");
    DateTime resultDT(2025, 1, 1);

    CPPUNIT_ASSERT(TrxUtil::convertYYYYMMDDDate(dateStr, resultDT));
    CPPUNIT_ASSERT(DateTime(2009, 12, 18) == resultDT);
  }

  void testConvertYYYYMMDDDateFail()
  {
    std::string dateStr("12/DEC/2009");
    DateTime resultDT(2025, 1, 1);

    CPPUNIT_ASSERT(!TrxUtil::convertYYYYMMDDDate(dateStr, resultDT));
    CPPUNIT_ASSERT(DateTime(2025, 1, 1) == resultDT);
  }

  /*
   * GNR_ACTIVATION_DATE
   */
  void test_configureNetRemitActivationDate_on_cfgDate()
  {
    DateTime tktDate = DateTime(2012, 03, 19);
    _request->ticketingDT() = tktDate;
    setGNRTestData(tktDate);
    CPPUNIT_ASSERT(TrxUtil::cat35LtypeEnabled(*_trx));
  }

  void test_configureNetRemitActivationDate_off_cfgDate()
  {
    DateTime tktDate = DateTime(2012, 03, 10);
    _request->ticketingDT() = tktDate;
    setGNRTestData(DateTime(2012, 03, 19));
    CPPUNIT_ASSERT(!TrxUtil::cat35LtypeEnabled(*_trx));
  }

  void test_configureNetRemitActivationDate_mismatch_pcc()
  {
    DateTime tktDate = DateTime(2012, 03, 19);
    _request->ticketingDT() = tktDate;
    setGNRTestData(tktDate);
    CPPUNIT_ASSERT(TrxUtil::cat35LtypeEnabled(*_trx));
  }

  class PtfStub : public PaxTypeFare
  {
    bool _isValid;

  public:
    PtfStub(bool isValid) : _isValid(isValid) {}
    virtual bool isValidForPricing() const { return _isValid; }
  };

  typedef std::pair<bool, std::vector<bool>> FmSetUpElement;

  void setUpFVOSurcharges(const std::vector<FmSetUpElement>& validFmValidPtf)
  {
    TestConfigInitializer::setValue("FVO_SURCHARGE_ON_NUM_FM", 1, "RULES_OPTIONS");
    TestConfigInitializer::setValue("FVO_SURCHARGE_ON_NUM_PTF", 3, "RULES_OPTIONS");

    for (const FmSetUpElement& breakIndValidPtf : validFmValidPtf)
    {
      _trx->fareMarket().push_back(_memH(new FareMarket));
      _trx->fareMarket().back()->setBreakIndicator(breakIndValidPtf.first);

      for (bool isValid : breakIndValidPtf.second)
        _trx->fareMarket().back()->allPaxTypeFare().push_back(_memH(new PtfStub(isValid)));
    }
  }

  void testIsFullMapRoutingActivated_Apply()
  {
    std::string dateString = "2013-06-16";
    TestConfigInitializer::setValue("FULL_MAP_ROUTING_ACTIVATION_DATE", dateString, "PRICING_SVC");
    _trx->ticketingDate() = DateTime(2013, 6, 19);

    CPPUNIT_ASSERT(TrxUtil::isFullMapRoutingActivated(*_trx));
  }

  void testIsFullMapRoutingActivated_NotApply()
  {
    std::string dateString = "2013-06-16";
    TestConfigInitializer::setValue("FULL_MAP_ROUTING_ACTIVATION_DATE", dateString, "PRICING_SVC");
    _trx->ticketingDate() = DateTime(2013, 6, 14);

    CPPUNIT_ASSERT(!TrxUtil::isFullMapRoutingActivated(*_trx));
  }

  void testIsIataFareSelectionActivated_Apply()
  {
    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue(
        "IATA_FARE_SELECTION_ACTIVATION_DATE", activationDate, "PRICING_SVC");
    DateTime dt(activationDate, 0);
    _trx->ticketingDate() = dt.addDays(2);

    CPPUNIT_ASSERT(TrxUtil::isIataFareSelectionActivated(*_trx));
  }

  void testIsIataFareSelectionActivated_NotApply()
  {
    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue(
        "IATA_FARE_SELECTION_ACTIVATION_DATE", activationDate, "PRICING_SVC");
    DateTime dt(activationDate, 0);
    _trx->ticketingDate() = dt.subtractDays(2);

    CPPUNIT_ASSERT(!TrxUtil::isIataFareSelectionActivated(*_trx));
  }

  void testIsIataFareSelectionApplicable_AncillaryTrx()
  {
    AncillaryPricingTrx ancillaryTrx;
    CPPUNIT_ASSERT(!TrxUtil::isIataFareSelectionApplicable(&ancillaryTrx));
  }

  void testIsIataFareSelectionApplicable_TktFeesTrx()
  {
    TktFeesPricingTrx tktFeesTrx;
    CPPUNIT_ASSERT(!TrxUtil::isIataFareSelectionApplicable(&tktFeesTrx));
  }

  void testIsIataFareSelectionApplicable_Exchange()
  {
    RexPricingTrx rexTrx;
    CPPUNIT_ASSERT(TrxUtil::isIataFareSelectionApplicable(&rexTrx));

    ExchangePricingTrx exTrx;
    CPPUNIT_ASSERT(TrxUtil::isIataFareSelectionApplicable(&exTrx));
  }

  void testIsIataFareSelectionApplicable_Mip()
  {
    ShoppingTrx mipTrx;
    mipTrx.setTrxType(PricingTrx::MIP_TRX);
    CPPUNIT_ASSERT(TrxUtil::isIataFareSelectionApplicable(&mipTrx));
  }

  void testIsIataFareSelectionApplicable_Repricing()
  {
    RepricingTrx repTrx;
    CPPUNIT_ASSERT(TrxUtil::isIataFareSelectionApplicable(&repTrx));
  }

  void testIsIataFareSelectionApplicable_Pricing()
  {
    CPPUNIT_ASSERT(TrxUtil::isIataFareSelectionApplicable(_trx));

    AltPricingTrx altTrx;
    CPPUNIT_ASSERT(TrxUtil::isIataFareSelectionApplicable(&altTrx));
  }

  void testIsIataFareSelectionApplicable_GoverningCxrOverride()
  {
    _request->governingCarrierOverrides().insert(std::pair<int16_t, CarrierCode>(1, "GC"));
    CPPUNIT_ASSERT(!TrxUtil::isIataFareSelectionApplicable(_trx));
  }

  void testCat05Override_NotApplyToPCC()
  {
    _request->ticketingDT() = DateTime::localTime();

    Customer* tjr = _memH.create<Customer>();
    tjr->cat05OverrideCode() = Alpha3Char(" ");
    _agent->agentTJR() = tjr;

    Itin newItin;
    _trx->itin().push_back(&newItin);
    TestConfigInitializer::setValue(
        "INFINI_CAT05_OVERRIDE_BKG_ACTIVATION_DATE", "2013-06-18", "PRICING_SVC", true);
    TrxUtil::setInfiniCat05BookingDTSkipFlagInItins(*_trx);
    bool retVal = (*(_trx->itin().begin()))->cat05BookingDateValidationSkip();
    CPPUNIT_ASSERT(!retVal);
  }

  void testCat05Override_ApplyToAllCarriers()
  {
    _request->ticketingDT() = DateTime::localTime();

    Customer* tjr = _memH.create<Customer>();
    tjr->cat05OverrideCode() = Alpha3Char("**");
    _agent->agentTJR() = tjr;

    Itin newItin;
    _trx->itin().push_back(&newItin);
    TestConfigInitializer::setValue(
        "INFINI_CAT05_OVERRIDE_BKG_ACTIVATION_DATE", "2013-06-18", "PRICING_SVC", true);
    TrxUtil::setInfiniCat05BookingDTSkipFlagInItins(*_trx);
    bool retVal = (*(_trx->itin().begin()))->cat05BookingDateValidationSkip();
    CPPUNIT_ASSERT(retVal);
  }

  void test_checkSchemaVersionForGlobalDisclosure_PricingRequest_1_1_0()
  {
    PricingTrx* pricingTrx = TransactionBuilder<PricingTrx>(&_memH).setSchema(1, 1, 0).build();
    CPPUNIT_ASSERT(!TrxUtil::checkSchemaVersionForGlobalDisclosure(*pricingTrx));
  }

  void test_checkSchemaVersionForGlobalDisclosure_PricingRequest_1_1_1()
  {
    PricingTrx* pricingTrx = TransactionBuilder<PricingTrx>(&_memH).setSchema(1, 1, 1).build();
    CPPUNIT_ASSERT(TrxUtil::checkSchemaVersionForGlobalDisclosure(*pricingTrx));
  }

  void test_checkSchemaVersionForGlobalDisclosure_AltPricingRequest_1_0_0()
  {
    AltPricingTrx* altPricingTrx =
        TransactionBuilder<AltPricingTrx>(&_memH).setSchema(1, 0, 0).build();
    CPPUNIT_ASSERT(!TrxUtil::checkSchemaVersionForGlobalDisclosure(*altPricingTrx));
  }

  void test_checkSchemaVersionForGlobalDisclosure_AltPricingRequest_1_0_1()
  {
    AltPricingTrx* altPricingTrx =
        TransactionBuilder<AltPricingTrx>(&_memH).setSchema(1, 0, 1).build();
    CPPUNIT_ASSERT(TrxUtil::checkSchemaVersionForGlobalDisclosure(*altPricingTrx));
  }

  void test_checkSchemaVersionForGlobalDisclosure_RexPricingTrx()
  {
    RexPricingTrx* rexPricingTrx = TransactionBuilder<RexPricingTrx>(&_memH).build();

    CPPUNIT_ASSERT(!rexPricingTrx->isNotExchangeTrx());
    CPPUNIT_ASSERT(TrxUtil::checkSchemaVersionForGlobalDisclosure(*rexPricingTrx));
  }

  void test_checkSchemaVersionForGlobalDisclosure_RefundPricingTrx()
  {
    RefundPricingTrx* refundPricingTrx = TransactionBuilder<RefundPricingTrx>(&_memH).build();

    CPPUNIT_ASSERT(!refundPricingTrx->isNotExchangeTrx());
    CPPUNIT_ASSERT(TrxUtil::checkSchemaVersionForGlobalDisclosure(*refundPricingTrx));
  }

  void test_checkSchemaVersionForGlobalDisclosure_ExchangePricingTrx()
  {
    ExchangePricingTrx* exchangePricingTrx = TransactionBuilder<ExchangePricingTrx>(&_memH).build();

    CPPUNIT_ASSERT(!exchangePricingTrx->isNotExchangeTrx());
    CPPUNIT_ASSERT(TrxUtil::checkSchemaVersionForGlobalDisclosure(*exchangePricingTrx));
  }

  void test_checkSchemaVersionForGlobalDisclosure_BaggageTrx_1_0_0()
  {
    BaggageTrx* baggageTrx = TransactionBuilder<BaggageTrx>(&_memH).setSchema(1, 0, 0).build();
    CPPUNIT_ASSERT(!TrxUtil::checkSchemaVersionForGlobalDisclosure(*baggageTrx));
  }

  void test_checkSchemaVersionForGlobalDisclosure_BaggageTrx_1_0_1()
  {
    BaggageTrx* baggageTrx = TransactionBuilder<BaggageTrx>(&_memH).setSchema(1, 0, 1).build();
    CPPUNIT_ASSERT(TrxUtil::checkSchemaVersionForGlobalDisclosure(*baggageTrx));
  }

  void test_isAutomaticPfcTaxExemptionEnabled()
  {
    Billing billing;
    billing.partitionID() = "YY";
    _trx->billing() = &billing;
    CPPUNIT_ASSERT(!TrxUtil::isAutomaticPfcTaxExemptionEnabled(*_trx));
    _agent->tvlAgencyPCC() = "";
    CPPUNIT_ASSERT(!TrxUtil::isAutomaticPfcTaxExemptionEnabled(*_trx));
    billing.partitionID() = "AA";
    CPPUNIT_ASSERT(TrxUtil::isAutomaticPfcTaxExemptionEnabled(*_trx));
  }

  void test_areUSTaxesOnYQYREnabled()
  {
    Billing billing;
    billing.partitionID() = "YY";
    _trx->billing() = &billing;
    CPPUNIT_ASSERT(!TrxUtil::areUSTaxesOnYQYREnabled(*_trx));
    _agent->tvlAgencyPCC() = "";
    CPPUNIT_ASSERT(!TrxUtil::areUSTaxesOnYQYREnabled(*_trx));
    billing.partitionID() = "AA";
    CPPUNIT_ASSERT(TrxUtil::areUSTaxesOnYQYREnabled(*_trx));
  }

  void test_areUSTaxesOnYQYREnabled_nullTicketingAgent()
  {
    _agent = 0;
    Billing billing;
    billing.partitionID() = "AA";
    CPPUNIT_ASSERT(!TrxUtil::areUSTaxesOnYQYREnabled(*_trx));
  }

  void test_IsCat35TFSFApplicable()
  {
    _trx->setTrxType(PricingTrx::MIP_TRX);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));

    _trx->setTrxType(PricingTrx::PRICING_TRX);
    _agent->tvlAgencyPCC() = "";
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));

    _agent->tvlAgencyPCC() = "80K2";
    CPPUNIT_ASSERT(TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void test_IsHistorical()
  {
    // set requested ticketing date to
    _agent->agentLocation() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTYO.xml");
    _trx->getRequest()->ticketingDT() = DateTime(2009, 05, 10, 23, 59, 0);

    CPPUNIT_ASSERT(TrxUtil::isHistorical(*_trx));
  }

  void test_Not_IsHistorical()
  {
    // set requested ticketing date to
    _agent->agentLocation() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTYO.xml");
    _trx->getRequest()->ticketingDT() = DateTime(9999, 05, 10, 23, 59, 0);
    // time difference between TYO and TUL is 14h
    CPPUNIT_ASSERT(!TrxUtil::isHistorical(*_trx));
  }

  void testIsFqFareRetailerEnabled() { CPPUNIT_ASSERT(TrxUtil::isFqFareRetailerEnabled(*_trx)); }

  void test_getOBFeeSubType()
  {
    ServiceSubTypeCode code;
    code = "FCA";
    CPPUNIT_ASSERT(TrxUtil::getOBFeeSubType(code) == OBFeeSubType::OB_F_TYPE);
    code = "FDA";
    CPPUNIT_ASSERT(TrxUtil::getOBFeeSubType(code) == OBFeeSubType::OB_F_TYPE);
    code = "T99";
    CPPUNIT_ASSERT(TrxUtil::getOBFeeSubType(code) == OBFeeSubType::OB_T_TYPE);
    code = "R11";
    CPPUNIT_ASSERT(TrxUtil::getOBFeeSubType(code) == OBFeeSubType::OB_R_TYPE);
    code = "C12";
    CPPUNIT_ASSERT(TrxUtil::getOBFeeSubType(code) == OBFeeSubType::OB_UNKNOWN);
  }

  void setupCat35TFSFEnabled(PricingTrx::TrxType trxType,
                             PricingTrx::ExcTrxType excType,
                             const std::string& pcc,
                             bool isNetSellingInd = false)
  {
    _agent->tvlAgencyPCC() = pcc;
    _trx->setTrxType(trxType);
    _trx->setExcTrxType(excType);

    if (excType != PricingTrx::NOT_EXC_TRX)
    {
      RexPricingOptions* op = _memH.create<RexPricingOptions>();
      op->setNetSellingIndicator(isNetSellingInd);
      _trx->setOptions(op);
    }
  }

  void testIsCat35TFSFEnabled_pricing_airlinePcc()
  {
    setupCat35TFSFEnabled(PricingTrx::PRICING_TRX, PricingTrx::NOT_EXC_TRX, AIRLINE_PCC);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_pricing_notAirlinePcc()
  {
    setupCat35TFSFEnabled(PricingTrx::PRICING_TRX, PricingTrx::NOT_EXC_TRX, AGENT_PCC);
    CPPUNIT_ASSERT(TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_mip_airlinePcc()
  {
    setupCat35TFSFEnabled(PricingTrx::MIP_TRX, PricingTrx::NOT_EXC_TRX, AIRLINE_PCC);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_mip_notAirlinePcc()
  {
    setupCat35TFSFEnabled(PricingTrx::MIP_TRX, PricingTrx::NOT_EXC_TRX, AGENT_PCC);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_is_airlinePcc()
  {
    setupCat35TFSFEnabled(PricingTrx::IS_TRX, PricingTrx::NOT_EXC_TRX, AIRLINE_PCC);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_is_notAirlinePcc()
  {
    setupCat35TFSFEnabled(PricingTrx::IS_TRX, PricingTrx::NOT_EXC_TRX, AGENT_PCC);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_rexPricing_airlinePcc_netSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::PRICING_TRX, PricingTrx::AR_EXC_TRX, AIRLINE_PCC, true);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_rexPricing_notAirlinePcc_netSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::PRICING_TRX, PricingTrx::AR_EXC_TRX, AGENT_PCC, true);
    CPPUNIT_ASSERT(TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_rexPricing_airlinePcc_notNetSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::PRICING_TRX, PricingTrx::AR_EXC_TRX, AIRLINE_PCC, false);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_rexPricing_notAirlinePcc_notNetSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::PRICING_TRX, PricingTrx::AR_EXC_TRX, AGENT_PCC, false);
    CPPUNIT_ASSERT(TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_exchangePricing_airlinePcc_netSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::PRICING_TRX, PricingTrx::PORT_EXC_TRX, AIRLINE_PCC, true);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_exchangePricing_notAirlinePcc_netSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::PRICING_TRX, PricingTrx::PORT_EXC_TRX, AGENT_PCC, true);
    CPPUNIT_ASSERT(TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_exchangePricing_airlinePcc_notNetSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::PRICING_TRX, PricingTrx::PORT_EXC_TRX, AIRLINE_PCC, false);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_exchangePricing_notAirlinePcc_notNetSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::PRICING_TRX, PricingTrx::PORT_EXC_TRX, AGENT_PCC, false);
    CPPUNIT_ASSERT(TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_refundPricing_airlinePcc_netSelling()
  {
    TestConfigInitializer::setValue("AUTOMATED_REFUND_CAT33_ENABLED", "Y", "TAX_SVC", true);
    setupCat35TFSFEnabled(PricingTrx::PRICING_TRX, PricingTrx::AF_EXC_TRX, AIRLINE_PCC, true);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_refundPricing_notAirlinePcc_netSelling()
  {
    TestConfigInitializer::setValue("AUTOMATED_REFUND_CAT33_ENABLED", "Y", "TAX_SVC", true);
    setupCat35TFSFEnabled(PricingTrx::PRICING_TRX, PricingTrx::AF_EXC_TRX, AGENT_PCC, true);
    CPPUNIT_ASSERT(TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_refundPricing_airlinePcc_notNetSelling()
  {
    TestConfigInitializer::setValue("AUTOMATED_REFUND_CAT33_ENABLED", "Y", "TAX_SVC", true);
    setupCat35TFSFEnabled(PricingTrx::PRICING_TRX, PricingTrx::AF_EXC_TRX, AIRLINE_PCC, false);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_refundPricing_notAirlinePcc_notNetSelling()
  {
    TestConfigInitializer::setValue("AUTOMATED_REFUND_CAT33_ENABLED", "Y", "TAX_SVC", true);
    setupCat35TFSFEnabled(PricingTrx::PRICING_TRX, PricingTrx::AF_EXC_TRX, AGENT_PCC, false);
    CPPUNIT_ASSERT(TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_excMip_airlinePcc_netSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::MIP_TRX, PricingTrx::AR_EXC_TRX, AIRLINE_PCC, true);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_excMip_notAirlinePcc_netSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::MIP_TRX, PricingTrx::AR_EXC_TRX, AGENT_PCC, true);
    CPPUNIT_ASSERT(TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_excMip_airlinePcc_notNetSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::MIP_TRX, PricingTrx::AR_EXC_TRX, AIRLINE_PCC, false);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_excMip_notAirlinePcc_notNetSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::MIP_TRX, PricingTrx::AR_EXC_TRX, AGENT_PCC, false);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_excIs_airlinePcc_netSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::IS_TRX, PricingTrx::EXC_IS_TRX, AIRLINE_PCC, true);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_excIs_notAirlinePcc_netSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::IS_TRX, PricingTrx::EXC_IS_TRX, AGENT_PCC, true);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_excIs_airlinePcc_notNetSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::IS_TRX, PricingTrx::EXC_IS_TRX, AIRLINE_PCC, false);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void testIsCat35TFSFEnabled_excIs_notAirlinePcc_notNetSelling()
  {
    setupCat35TFSFEnabled(PricingTrx::IS_TRX, PricingTrx::EXC_IS_TRX, AGENT_PCC, false);
    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));
  }

  void test_isAAAwardPricing_TrxType()
  {
    PricingTrx* pricingTrx = TransactionBuilder<PricingTrx>(&_memH).setSchema(1, 1, 0).build();

    pricingTrx->awardRequest() = true;
    Billing billing;
    billing.partitionID() = "AA";
    pricingTrx->billing() = &billing;

    CPPUNIT_ASSERT(TrxUtil::isAAAwardPricing(*pricingTrx));

    pricingTrx->setTrxType(PricingTrx::REPRICING_TRX);
    CPPUNIT_ASSERT(TrxUtil::isAAAwardPricing(*pricingTrx));

    pricingTrx->setTrxType(PricingTrx::MIP_TRX);
    CPPUNIT_ASSERT(!TrxUtil::isAAAwardPricing(*pricingTrx));
    pricingTrx->setTrxType(PricingTrx::IS_TRX);
    CPPUNIT_ASSERT(!TrxUtil::isAAAwardPricing(*pricingTrx));
    pricingTrx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
    CPPUNIT_ASSERT(!TrxUtil::isAAAwardPricing(*pricingTrx));
    pricingTrx->setTrxType(PricingTrx::ESV_TRX);
    CPPUNIT_ASSERT(!TrxUtil::isAAAwardPricing(*pricingTrx));
    pricingTrx->setTrxType(PricingTrx::FF_TRX);
    CPPUNIT_ASSERT(!TrxUtil::isAAAwardPricing(*pricingTrx));
    pricingTrx->setTrxType(PricingTrx::RESHOP_TRX);
    CPPUNIT_ASSERT(!TrxUtil::isAAAwardPricing(*pricingTrx));
  }

  void test_isAAAwardPricing_AAAward()
  {
    PricingTrx* pricingTrx = TransactionBuilder<PricingTrx>(&_memH).setSchema(1, 1, 0).build();

    pricingTrx->awardRequest() = true;
    Billing billing;
    billing.partitionID() = "AA";
    pricingTrx->billing() = &billing;

    CPPUNIT_ASSERT(TrxUtil::isAAAwardPricing(*pricingTrx));
    pricingTrx->awardRequest() = false;
    CPPUNIT_ASSERT(!TrxUtil::isAAAwardPricing(*pricingTrx));

    pricingTrx->awardRequest() = true;
    billing.partitionID() = "UA";
    CPPUNIT_ASSERT(!TrxUtil::isAAAwardPricing(*pricingTrx));
  }

  void testIsIcerActivated_Apply()
  {
    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue("ICER_ACTIVATION_DATE", activationDate, "COMMON_FUNCTIONAL");
    DateTime dt(activationDate, 0);
    _trx->ticketingDate() = dt.addDays(2);

    CPPUNIT_ASSERT(TrxUtil::isIcerActivated(*_trx));
  }

  void testIsIcerActivated_NotApply()
  {
    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue("ICER_ACTIVATION_DATE", activationDate, "COMMON_FUNCTIONAL");
    DateTime dt(activationDate, 0);
    _trx->ticketingDate() = dt.subtractDays(2);

    CPPUNIT_ASSERT(!TrxUtil::isIcerActivated(*_trx));
  }

  void testIsIcerActivatedWithDate_Apply()
  {
    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue("ICER_ACTIVATION_DATE", activationDate, "COMMON_FUNCTIONAL");
    DateTime dt(activationDate, 0);

    CPPUNIT_ASSERT(TrxUtil::isIcerActivated(*_trx, dt.addDays(2)));
  }

  void testIsJcbCarrierList()
  {
    std::string cfgCxr = "KK";
    TestConfigInitializer::setValue("IS_JCB_CARRIER", cfgCxr, "SHOPPING_SVC");

    CarrierCode carrier1 = "ZZ";
    CarrierCode carrier2 = "KK";
    CPPUNIT_ASSERT(!TrxUtil::isJcbCarrier(&carrier1));
    CPPUNIT_ASSERT(TrxUtil::isJcbCarrier(&carrier2));
  }

  void testIsIcerActivatedWithDate_NotApply()
  {
    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue("ICER_ACTIVATION_DATE", activationDate, "COMMON_FUNCTIONAL");
    DateTime dt(activationDate, 0);

    CPPUNIT_ASSERT(!TrxUtil::isIcerActivated(*_trx, dt.subtractDays(2)));
  }

  void testIsAtpcoRbdByCabinAnswerTableActivated_Apply()
  {
    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue(
        "ATPCO_RBDBYCABIN_ANSWER_TABLE_ACTIVATION_DATE", activationDate, "PRICING_SVC");
    DateTime dt(activationDate, 0);
    _trx->ticketingDate() = dt.addDays(2);
    _trx->dataHandle().setTicketDate(dt.addDays(2));

    CPPUNIT_ASSERT(TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(*_trx));
  }

  void testIsAtpcoRbdByCabinAnswerTableActivated_NotApply()
  {
    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue(
        "ATPCO_RBDBYCABIN_ANSWER_TABLE_ACTIVATION_DATE", activationDate, "PRICING_SVC");
    DateTime dt(activationDate, 0);
    _trx->ticketingDate() = dt.subtractDays(2);
    _trx->dataHandle().setTicketDate(dt.subtractDays(2));

    CPPUNIT_ASSERT(!TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(*_trx));
  }

  void testIsAtpcoRbdAnswerTableActivatedShopping()
  {
    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue(
        "ATPCO_RBDBYCABIN_ANSWER_TABLE_ACTIVATION_DATE", activationDate, "PRICING_SVC");
    DateTime dt(activationDate, 0);
    _trx->ticketingDate() = dt.addDays(2);
    _trx->dataHandle().setTicketDate(dt.addDays(2));

    CPPUNIT_ASSERT(TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(*_trx));

    _trx->ticketingDate() = dt.subtractDays(2);
    _trx->dataHandle().setTicketDate(dt.subtractDays(2));

    CPPUNIT_ASSERT(!TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(*_trx));
  }

  void testIsBspUser_nonGsa_returnTrue()
  {
    _trx->setValidatingCxrGsaApplicable(false);

    CPPUNIT_ASSERT(TrxUtil::isBspUser(*_trx));
  }

  void testIsBspUser_nonBsp_returnFalse()
  {
    _trx->setValidatingCxrGsaApplicable(true);
    CountrySettlementPlanInfo cspi;
    cspi.setSettlementPlanTypeCode("GEN");
    _trx->addCountrySettlementPlanInfo(&cspi);

    CPPUNIT_ASSERT(!TrxUtil::isBspUser(*_trx));
  }

  void testIsBspUser_bsp_returnTrue()
  {
    _trx->setValidatingCxrGsaApplicable(true);
    CountrySettlementPlanInfo cspi1, cspi2;
    cspi1.setSettlementPlanTypeCode("GEN");
    cspi2.setSettlementPlanTypeCode("BSP");

    _trx->addCountrySettlementPlanInfo(&cspi1);
    _trx->addCountrySettlementPlanInfo(&cspi2);

    CPPUNIT_ASSERT(TrxUtil::isBspUser(*_trx));
  }

  void testIsAbacusEndorsementHierarchyAtpcoFaresActive()
  {
    CPPUNIT_ASSERT(TrxUtil::isAbacusEndorsementHierarchyAtpcoFaresActive(*_trx));
  }

  void testIsDateAdjustmentIndicatorActive()
  {
    CPPUNIT_ASSERT(TrxUtil::isDateAdjustmentIndicatorActive(*_trx));
  }

  class MyDataHandle : public DataHandleMock
  {
    TestMemHandle _memHandle;

  public:
    bool getUtcOffsetDifference(const DSTGrpCode& dstgrp1,
                                const DSTGrpCode& dstgrp2,
                                short& utcoffset,
                                const DateTime& dateTime1,
                                const DateTime& dateTime2)
    {
      utcoffset = 0;
      return true;
    }
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(TrxUtilTest);

const char* const TrxUtilTest::AGENT_PCC = "80K2";
const char* const TrxUtilTest::AIRLINE_PCC = "";

} // tse
