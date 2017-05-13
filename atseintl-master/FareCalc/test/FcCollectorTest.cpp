#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PrivateIndicator.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FcCollector.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FcMessage.h"
#include "Rules/RuleConst.h"

#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestLocFactory.h"
#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{
FALLBACKVALUE_DECL(fallbackValidatingCxrMultiSp);
FALLBACKVALUE_DECL(fallbackValidatingCarrierInItinOrderMultiSp);


using boost::assign::operator+=;

const std::string LOC_DFW = "/vobs/atseintl/test/testdata/data/LocDFW.xml";
const std::string LOC_LON = "/vobs/atseintl/test/testdata/data/LocLON.xml";

namespace FareCalc
{
class FarePathBuilder
{
private:
  TestMemHandle* _memHandle;
  FarePath* _farePath;

public:
  FarePathBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _farePath = _memHandle->create<FarePath>();
    _farePath->paxType() = _memHandle->create<PaxType>();
    _farePath->paxType()->number() = 1;
    PricingUnit* pu = _memHandle->create<PricingUnit>();
    _farePath->pricingUnit().push_back(pu);
  }

  FarePathBuilder& withPaxTypeCode(PaxTypeCode& paxType)
  {
    _farePath->paxType()->paxType() = paxType;
    return *this;
  }

  FarePathBuilder& withSegmentNumber(int segmentNumber, bool usDot = false)
  {
    std::vector<TravelSeg*>* travelSegments = _memHandle->create<std::vector<TravelSeg*> >();
    for (int i = 0; i < segmentNumber; ++i)
    {
      AirSeg* airSeg = _memHandle->create<AirSeg>();
      airSeg->pnrSegment() = i + 1;
      travelSegments->push_back(airSeg);
    }
    Itin* itin = _memHandle->create<Itin>();
    itin->setBaggageTripType(usDot ? BaggageTripType::TO_FROM_US : BaggageTripType::OTHER);
    itin->travelSeg() = *travelSegments;
    _farePath->itin() = itin;
    itin->farePath().push_back(_farePath);
    return *this;
  }

  FarePathBuilder& withBaggageResponse(std::string& baggageResponse)
  {
    _farePath->baggageResponse() = baggageResponse;
    return *this;
  }

  FarePathBuilder& withBaggageEmbargoesResponse(std::string& baggageEmbargoesResponse)
  {
    _farePath->baggageEmbargoesResponse() = baggageEmbargoesResponse;
    return *this;
  }

  FarePath* build() { return _farePath; }
};
class FcCollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FcCollectorTest);
  CPPUNIT_TEST(testConstructorWithPVTIndicator);
  CPPUNIT_TEST(testNeedProcessPVTIndicatorForMip);
  CPPUNIT_TEST(testNeedProcessPVTIndicatorForExchange);
  CPPUNIT_TEST(testNeedProcessPVTIndicatorForFareX);
  CPPUNIT_TEST(testNeedProcessPVTIndicatorForTicketing);
  CPPUNIT_TEST(testNeedProcessPVTIndicatorForCrxPartition);
  CPPUNIT_TEST(testNeedProcessPVTIndicatorForAgencyOnWp);
  CPPUNIT_TEST(testCollectPvtFareIndicatorForPublic);
  CPPUNIT_TEST(testCollectPvtFareIndicatorForPrivate);
  CPPUNIT_TEST(collectPvtIndicatorMessageFalse);
  CPPUNIT_TEST(collectPvtIndicatorMessageTrue);
  CPPUNIT_TEST(testCollectFareTotals_NR_VALUE_B);
  CPPUNIT_TEST(testCollectValidatingCarrierMessage_FromRequest);
  CPPUNIT_TEST(testCollectValidatingCarrierMessage_FromRequest_VcxrActive);
  CPPUNIT_TEST(testCollectValidatingCarrierMessage_FromItin);
  CPPUNIT_TEST(testCollectValidatingCarrierMessage_FromItin_VcxrActive);
  CPPUNIT_TEST(testCollectValidatingCarrierMessage_FromItinEmpty);
  CPPUNIT_TEST(testCollectValidatingCarrierMessage_FromItinEmpty_VcxrActive);
  CPPUNIT_TEST(testCollectValidatingCarrierMessage_SingleGsaSwap);
  CPPUNIT_TEST(testCollectValidatingCarrierMessage_SingleGsaSwap_VcxrActive);
  CPPUNIT_TEST(testCollectServiceFeesTemplate_Regular);
  CPPUNIT_TEST(testCollectServiceFeesTemplate_Issta);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_FromRequest);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_SpecifiedButNoVcxr);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_FromItin);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_FromItinEmpty);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_NoRequest);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_NoItin);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_SingleGsaSwap_RequestedVcxr);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_SingleGsaSwap_DefaultVcxr);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_ValidatingCxr_empty_Alternate_Has_Two);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_ValidatingCxr_empty_Alternate_NotFitInOneLine);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_MultipleSettlementPlans);
  CPPUNIT_TEST(testValidatingCxrCommissionAmount_NonGSA);
  CPPUNIT_TEST(testValidatingCxrCommissionAmount_GSA);
  CPPUNIT_TEST(testValidatingCxrCommissionAmount_ForMultipSp);

  CPPUNIT_TEST(testSetValCxrTrailerMessages_EmptyTrailerMsgCol);
  CPPUNIT_TEST(testSetValCxrTrailerMessages_BlankValCxrTrailerMsg);
  CPPUNIT_TEST(testSetValCxrTrailerMessages_ValCxrTrailerMsg);
  CPPUNIT_TEST(testSetValCxrTrailerMessages_MultipleValCxrTrailerMsg);
  CPPUNIT_TEST(testSetValCxrTrailerMessages_AlternateValCxrTrailerMsg);
  CPPUNIT_TEST(testSetValCxrTrailerMessages_OptionalValCxrTrailerMsg);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_NoValidatingCxr);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_BlankValidatingCxr);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_DefaultValidatingCxr);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_OptionalValidatingCxr);
  CPPUNIT_TEST(testBuildValidatingCarrierMessage_GSASwappedCxr);
  CPPUNIT_TEST(testPrepareValCxrMsgForMultiSp_NoDefaultValCxr);
  CPPUNIT_TEST(testPrepareValCxrMsgForMultiSp_DefaultValCxr);
  CPPUNIT_TEST(testPrepareValCxrMsgForMultiSp_OptionalValCxr);
  CPPUNIT_TEST(testPrepareValCxrMsgForMultiSp_SpecifiedValCxr);
  CPPUNIT_TEST(testPrepareValCxrMsgForMultiSp_GSASwappedCxr);
  CPPUNIT_TEST(test_constructCommTrailerMsgForDefaultValCxr_WithNoDefValCxr);
  CPPUNIT_TEST(test_constructCommTrailerMsgForDefaultValCxr_WithDefValCxrWithComm);
  CPPUNIT_TEST(test_constructCommTrailerMsgForAlternateValCxr_WithNoAltValCxr);
  CPPUNIT_TEST(test_constructCommTrailerMsgForAlternateValCxr_WithAltValCxrsWithSameComm);
  CPPUNIT_TEST(test_constructCommTrailerMsgForAlternateValCxr_WithAltValCxrsWithDiffComm);
  // Remove the below three cppunits when removing "fallbackAMCPhase2" fallback flag
  //CPPUNIT_TEST(test_constructCommTrailerMsgForDefaultValCxr_WithDefValCxrNoComm);
  //CPPUNIT_TEST(test_constructCommTrailerMsgForAlternateValCxr_WithAltValCxrsWithNoComm);
  //CPPUNIT_TEST(test_constructCommTrailerMsgForAlternateValCxr_Mixed);
  //CPPUNIT_TEST(test_constructCommTrailerMsgForOptionalValCxr);
  CPPUNIT_TEST(testCollectCommissionTrailerMessages);
  CPPUNIT_TEST(testCollectCommissionTrailerMessages_SameCommAmount);
  CPPUNIT_TEST(testDetermineCat15HasTextTableReturnTrue);
  CPPUNIT_TEST(testDetermineCat15HasTextTableReturnFalse);

  CPPUNIT_TEST(test_createDefaultTrailerMsg_WithoutValCxr);
  CPPUNIT_TEST(test_createDefaultTrailerMsg_WithValCxr);
  CPPUNIT_TEST(test_createDefaultTrailerMsg_WithSwappedCxr);

  CPPUNIT_TEST(test_createAltTrailerMsg_EmptyList);
  CPPUNIT_TEST(test_createAltTrailerMsg_OneAltCxr);
  CPPUNIT_TEST(test_createAltTrailerMsg_MoreAltCxr_InItinOrder);
  CPPUNIT_TEST(test_createAltTrailerMsg_MoreAltCxr_NotInItinOrder);
  CPPUNIT_TEST(test_createAltTrailerMsg_MoreAltCxr_NotInItinOrderWithSwappedCxr);
  CPPUNIT_TEST(test_createAltTrailerMsg_MoreAltCxr_NotInItinOrderWithSwappedCxrMultiSp);

  CPPUNIT_TEST(testCollect);
  CPPUNIT_TEST(testCollectMatchedAccCodeTrailerMessage);
  CPPUNIT_TEST(testIsDefaultVcxrFromPreferred_DefaultVcxrEmpty);
  CPPUNIT_TEST(testIsDefaultVcxrFromPreferred_preferredVcxrEmpty);
  CPPUNIT_TEST(testIsDefaultVcxrFromPreferred_Found);
  CPPUNIT_TEST(testIsDefaultVcxrFromPreferred_NotFound);
  CPPUNIT_TEST(testIsAcxrsAreNSPcxrs_Pass);
  CPPUNIT_TEST(testIsAcxrsAreNSPcxrs_Fail);
  CPPUNIT_TEST(testPrepareValCxrMsgForMultiSp_noSMV_noIEV);
  CPPUNIT_TEST(testPrepareValCxrMsgForMultiSp_noSMV_IEV);
  CPPUNIT_TEST(testCollectValidatingCarrierMessageForGSA_noSMV_noIEV);
  CPPUNIT_TEST(testCollectValidatingCarrierMessageForGSA_noSMV_IEV);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<FcCollectorDataHandleMock>();

    _fcalcCollector = _memHandle.create<FareCalcCollector>();
    _calcTotals = _memHandle.create<CalcTotals>();
    _calcTotals->negFareUsed = false;

    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);
    _option = _memHandle.create<PricingOptions>();
    _trx->setOptions(_option);
    _trx->setTrxType(PricingTrx::PRICING_TRX);

    _agent = _memHandle.create<Agent>();
    _customer = _memHandle.create<Customer>();
    _agent->tvlAgencyPCC() = "W0H3";
    _agent->agentTJR() = _customer;
    _agent->agentLocation() = TestLocFactory::create(LOC_DFW);
    _agent->currencyCodeAgent() = "USD";
    _request->ticketingAgent() = _agent;

    _farePath = _memHandle.create<FarePath>();
    _itin = _memHandle.create<Itin>();
    _trx->itin().push_back(_itin);
    _farePath->itin() = _itin;
    _paxType = _memHandle.create<PaxType>();
    _farePath->paxType() = _paxType;
    _farePath->paxType()->paxType() = "ADT";
    _farePath->paxType()->paxTypeInfo() = _memHandle.create<PaxTypeInfo>();
    _farePath->baseFareCurrency() = "USD";
    _trx->paxType().push_back(_paxType);
    createBasicFarePath();

    _trx->fareCalcCollector().push_back(_fcalcCollector);
    _fcalcCollector->addCalcTotals(_farePath, _calcTotals);

    _fcConfig = _memHandle.create<FareCalcConfig>();

    _customer->privateFareInd() = 'N';
    _fcCTest = _memHandle.insert(
        new FcCollector(_trx, _farePath, _fcConfig, _fcalcCollector, _calcTotals));

    _customer->privateFareInd() = 'Y';
    _fcC = _memHandle.insert(
        new FcCollector(_trx, _farePath, _fcConfig, _fcalcCollector, _calcTotals));

    ValidatingCxrGSAData v;
    _itin->validatingCxrGsaData() = &v;

  }

  void tearDown() {
    _memHandle.clear();
  }

  void setUpCrxPartitionTrx() { _trx->getRequest()->ticketingAgent()->tvlAgencyPCC().clear(); }
  void setUpTktTrx() { _trx->getRequest()->ticketEntry() = 'Y'; }
  void setUpFarexTrx() { _trx->getOptions()->fareX() = true; }
  void setUpExcTrx() { _trx->setExcTrxType(PricingTrx::AR_EXC_TRX); }
  void setUpMIPTrx() { _trx->setTrxType(PricingTrx::MIP_TRX); }
  void createBasicFarePath()
  {
    AirSeg* ts1 = _memHandle.create<AirSeg>();
    AirSeg* ts2 = _memHandle.create<AirSeg>();

    ts1->origin() = TestLocFactory::create(LOC_DFW);
    ts1->destination() = TestLocFactory::create(LOC_LON);
    ts2->origin() = TestLocFactory::create(LOC_LON);
    ts2->destination() = TestLocFactory::create(LOC_DFW);
    ts1->segmentOrder() = 0;
    ts2->segmentOrder() = 1;

    ts1->segmentType() = Air;
    ts2->segmentType() = Air;

    _itin->travelSeg() += ts1, ts2;
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    _farePath->pricingUnit() += pu;
  }
  FareUsage* createFareUsageWithNegFare(Indicator tktFareDataInd)
  {
    FareUsage* fu = _memHandle.create<FareUsage>();
    AirSeg* ts1 = _memHandle.create<AirSeg>();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    PaxTypeFareRuleData* ruleData = _memHandle.create<PaxTypeFareRuleData>();
    NegFareRest* negFareRest = _memHandle.create<NegFareRest>();
    NetRemitFarePath* nrFarePath = _memHandle.create<NetRemitFarePath>();
    Itin* itin = _memHandle.create<Itin>();

    nrFarePath->fareUsageMap().insert(std::make_pair(fu, fu));
    negFareRest->tktFareDataInd1() = tktFareDataInd;
    _farePath->netRemitFarePath() = nrFarePath;
    nrFarePath->itin() = itin;
    _itin->travelSeg() += ts1;
    fu->travelSeg() += ts1;
    fu->paxTypeFare() = ptf;
    fu->tktNetRemitFare() = ptf;
    ptf->setFare(fare);
    fare->setFareInfo(fareInfo);
    ptf->status().set(PaxTypeFare::PTF_Negotiated);
    ptf->setRuleData(RuleConst::NEGOTIATED_RULE, _trx->dataHandle(), ruleData);
    ptf->nucFareAmount() = 7;
    ruleData->baseFare() = ptf;
    ruleData->ruleItemInfo() = negFareRest;

    return fu;
  }
  void setupAxessAgent()
  {
    _customer->crsCarrier() = "1J";
    _customer->hostName() = "AXESS";

    _request->wpNettRequested() = 'Y';
  }
  void setSpValidatingCxrGsaData(FarePath& fp,
      ValidatingCxrGSAData* valCxrGsaData,
      std::initializer_list<SettlementPlanType> sp_list)
  {
    if (!fp.itin()->spValidatingCxrGsaDataMap())
      fp.itin()->spValidatingCxrGsaDataMap() = _trx->dataHandle().create<SpValidatingCxrGSADataMap>();

    SpValidatingCxrGSADataMap* ref = fp.itin()->spValidatingCxrGsaDataMap();
    if (ref)
    {
      for (auto sp : sp_list)
        ref->insert(std::pair<SettlementPlanType, const ValidatingCxrGSAData*>(sp, valCxrGsaData));
    }
  }


  void setAllValidatingCarriersForSp(
      const SettlementPlanType& sp,
      std::vector<CarrierCode>& valCxrs,
      const FarePath& fp)
  {
    auto its = fp.settlementPlanValidatingCxrs().find(sp);
    if ( its != fp.settlementPlanValidatingCxrs().end())
      valCxrs = its->second;
  }

  void collectPvtIndicatorMessageTrue()
  {
    FareUsage* fu = createFareUsageWithPrivatePTF(true);
    _fcC->collectPvtFareIndicator(fu);
    _fcC->collectPvtIndicatorMessage();
    CPPUNIT_ASSERT(!_calcTotals->fcMessage.empty());
  }

  void collectPvtIndicatorMessageFalse()
  {
    FareUsage* fu = createFareUsageWithPrivatePTF(false);
    _fcC->collectPvtFareIndicator(fu);
    _fcC->collectPvtIndicatorMessage();
    CPPUNIT_ASSERT(_calcTotals->fcMessage.empty());
  }

  void testCollectPvtFareIndicatorForPrivate()
  {
    CPPUNIT_ASSERT(_calcTotals->privateFareIndSeq == PrivateIndicator::NotPrivate);
    FareUsage* fu = createFareUsageWithPrivatePTF(true);
    _fcC->collectPvtFareIndicator(fu);
    CPPUNIT_ASSERT(_calcTotals->privateFareIndSeq != PrivateIndicator::NotPrivate);
  }

  void testCollectPvtFareIndicatorForPublic()
  {
    CPPUNIT_ASSERT(_calcTotals->privateFareIndSeq == PrivateIndicator::NotPrivate);
    FareUsage* fu = createFareUsageWithPrivatePTF(false);
    _fcC->collectPvtFareIndicator(fu);
    CPPUNIT_ASSERT(_calcTotals->privateFareIndSeq == PrivateIndicator::NotPrivate);
  }

  FareUsage* createFareUsageWithPrivatePTF(bool pvtTrf)
  {
    FareInfo* fi = _memHandle.create<FareInfo>();
    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fi);
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();

    FareClassAppInfo* fcai = _memHandle.create<FareClassAppInfo>();
    fcai->_displayCatType = 'T';
    ptf->fareClassAppInfo() = fcai;

    FareClassAppSegInfo* fcasi = _memHandle.create<FareClassAppSegInfo>();
    ptf->fareClassAppSegInfo() = fcasi;

    TariffCrossRefInfo* tcr = _memHandle.create<TariffCrossRefInfo>();
    if (pvtTrf)
      tcr->tariffCat() = 1; // private tariff
    else
      tcr->tariffCat() = 0; // public tariff
    fare->setTariffCrossRefInfo(tcr);
    ptf->setFare(fare);

    FareUsage* fuPvt = _memHandle.create<FareUsage>();
    fuPvt->paxTypeFare() = ptf;
    return fuPvt;
  }

  void testConstructorWithPVTIndicator()
  {
    CPPUNIT_ASSERT(!_fcCTest->needProcessPvtIndFromTJR());
    CPPUNIT_ASSERT(_fcC->needProcessPvtIndFromTJR());
  }

  void testNeedProcessPVTIndicatorForExchange()
  {
    setUpExcTrx();
    CPPUNIT_ASSERT(_fcC->needProcessPvtIndicator());
  }

  void testNeedProcessPVTIndicatorForFareX()
  {
    setUpFarexTrx();
    CPPUNIT_ASSERT(!_fcC->needProcessPvtIndicator());
  }

  void testNeedProcessPVTIndicatorForTicketing()
  {
    setUpTktTrx();
    CPPUNIT_ASSERT(_fcC->needProcessPvtIndicator());
  }

  void testNeedProcessPVTIndicatorForCrxPartition()
  {
    setUpCrxPartitionTrx();
    CPPUNIT_ASSERT(!_fcC->needProcessPvtIndicator());
  }

  void testNeedProcessPVTIndicatorForAgencyOnWp()
  {
    CPPUNIT_ASSERT(_fcC->needProcessPvtIndicator());
  }

  void testNeedProcessPVTIndicatorForMip()
  {
    setUpMIPTrx();
    CPPUNIT_ASSERT(!_fcC->needProcessPvtIndicator());
  }

  void testCollectFareTotals_NR_VALUE_B()
  {
    setupAxessAgent();
    _fcC->collectFareTotals(createFareUsageWithNegFare(RuleConst::NR_VALUE_B));
    CPPUNIT_ASSERT_EQUAL(1, int(_calcTotals->ticketFareAmount.size()));
    CPPUNIT_ASSERT_EQUAL(std::string("7.00"), _calcTotals->ticketFareAmount.front());
  }

  void setupCollectValidatingCarrierMessage(const CarrierCode& specifiedCxr,
                                            const CarrierCode& defaultValidatingCxr)
  {
    _trx->ticketingDate() = DateTime(2020, 1, 1);
    _fcConfig->valCxrDisplayOpt() = YES;
    _calcTotals->farePath = _farePath;
    _farePath->defaultValidatingCarrier() = defaultValidatingCxr;

    _request->validatingCarrier() = specifiedCxr;
    _itin->validatingCarrier() = defaultValidatingCxr;
  }

  void setupValidatingCxrTicketingDate()
  {
    _trx->ticketingDate() = DateTime(2026, 1, 1);
  }

  void testCollectValidatingCarrierMessage_FromRequest()
  {
    setupCollectValidatingCarrierMessage(CARRIER_9B, CARRIER_2R);

    _fcC->collectValidatingCarrierMessage();

    CPPUNIT_ASSERT( !_calcTotals->fcMessage.empty() );
    CPPUNIT_ASSERT_EQUAL( std::string("VALIDATING CARRIER SPECIFIED - 9B"),
                          _calcTotals->fcMessage.front().messageText() );
  }

  void testCollectValidatingCarrierMessage_FromRequest_VcxrActive()
  {
    _trx->setValidatingCxrGsaApplicable( true );
    const std::string expectedMsg = "VALIDATING CARRIER SPECIFIED - 9B";
    setupCollectValidatingCarrierMessage(CARRIER_9B, CARRIER_9B);
    setupValidatingCxrTicketingDate();

    _fcC->collectValidatingCarrierMessage();

    CPPUNIT_ASSERT( !_calcTotals->fcMessage.empty() );
    const std::string gotMsg = _calcTotals->fcMessage.front().messageText();
    CPPUNIT_ASSERT_EQUAL( expectedMsg, gotMsg );
  }

  void testCollectValidatingCarrierMessage_FromItin()
  {
    setupCollectValidatingCarrierMessage(EMPTY_STRING(), CARRIER_2R);

    _fcC->collectValidatingCarrierMessage();

    CPPUNIT_ASSERT( !_calcTotals->fcMessage.empty() );
    CPPUNIT_ASSERT_EQUAL( std::string("VALIDATING CARRIER - 2R"),
                          _calcTotals->fcMessage.front().messageText() );
  }

  void testCollectValidatingCarrierMessage_FromItin_VcxrActive()
  {
    _trx->setValidatingCxrGsaApplicable( true );
    const std::string expectedMsg = "VALIDATING CARRIER - 2R";
    setupCollectValidatingCarrierMessage(EMPTY_STRING(), CARRIER_2R);
    setupValidatingCxrTicketingDate();

    _fcC->collectValidatingCarrierMessage();

    CPPUNIT_ASSERT( !_calcTotals->fcMessage.empty() );
    const std::string gotMsg =_calcTotals->fcMessage.front().messageText();
    CPPUNIT_ASSERT_EQUAL( expectedMsg, gotMsg );
  }

  void testCollectValidatingCarrierMessage_FromItinEmpty()
  {
    setupCollectValidatingCarrierMessage(EMPTY_STRING(), EMPTY_STRING());

    _fcC->collectValidatingCarrierMessage();

    CPPUNIT_ASSERT( !_calcTotals->fcMessage.empty() );
    CPPUNIT_ASSERT_EQUAL( std::string("VALIDATING CARRIER - "),
                          _calcTotals->fcMessage.front().messageText() );
  }

  void testCollectValidatingCarrierMessage_FromItinEmpty_VcxrActive()
  {
    _trx->setValidatingCxrGsaApplicable( true );
    const std::string expectedMsg = "VALIDATING CARRIER - ";
    setupCollectValidatingCarrierMessage(EMPTY_STRING(), EMPTY_STRING());
    setupValidatingCxrTicketingDate();

    _fcC->collectValidatingCarrierMessage();

    CPPUNIT_ASSERT( !_calcTotals->fcMessage.empty() );
    const std::string gotMsg = _calcTotals->fcMessage.front().messageText();
    CPPUNIT_ASSERT_EQUAL( expectedMsg, gotMsg );
  }

  void testCollectValidatingCarrierMessage_SingleGsaSwap()
  {
    const std::string expectedMsg = "VALIDATING CARRIER - 2R";
    setupCollectValidatingCarrierMessage(EMPTY_STRING(), CARRIER_2R);
    _farePath->marketingCxrForDefaultValCxr() = "M1";

    _fcC->collectValidatingCarrierMessage();

    CPPUNIT_ASSERT( !_calcTotals->fcMessage.empty() );
    const std::string gotMsg = _calcTotals->fcMessage.front().messageText();
    CPPUNIT_ASSERT_EQUAL( expectedMsg, gotMsg );
  }

  void testCollectValidatingCarrierMessage_SingleGsaSwap_VcxrActive()
  {
    _trx->setValidatingCxrGsaApplicable( true );
    const std::string expectedMsg = "VALIDATING CARRIER - 2R PER GSA AGREEMENT WITH M1";
    setupCollectValidatingCarrierMessage(EMPTY_STRING(), CARRIER_2R);
    setupValidatingCxrTicketingDate();
    _farePath->marketingCxrForDefaultValCxr() = "M1";

    _fcC->collectValidatingCarrierMessage();

    CPPUNIT_ASSERT( !_calcTotals->fcMessage.empty() );
    const std::string gotMsg = _calcTotals->fcMessage.front().messageText();
    CPPUNIT_ASSERT_EQUAL( expectedMsg, gotMsg );
    CPPUNIT_ASSERT_EQUAL(FcMessage::VCX_SINGLE_GSA_SWAP,
        _calcTotals->fcMessage.front().messageType());
  }
  void testCollectMatchedAccCodeTrailerMessage()
  {
    _request = _memHandle.create<PricingRequest>();
    _request->isMultiAccCorpId() = true;
    _trx->altTrxType() = PricingTrx::WP;
    _trx->setRequest(_request);
    FareUsage* fu = _memHandle.create<FareUsage>();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    FarePath* fp = _memHandle.create<FarePath>();
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    ptf->matchedAccCode() = "ACCCODE";
    fu->paxTypeFare() = ptf;
    pu->fareUsage() += fu;
    fp->pricingUnit() += pu;
    fp->itin() = _itin;
    _calcTotals->pricingUnits[fu] = pu;

    _fcC->collectMatchedAccCodeTrailerMessage();
    const std::string expectedMsg = "CORP ID/ACCNT CODE USED: ACCCODE";
    const std::string gotMsg = _calcTotals->fcMessage.front().messageText();
    CPPUNIT_ASSERT_EQUAL(size_t(1),_calcTotals->fcMessage.size());
    CPPUNIT_ASSERT_EQUAL( expectedMsg, gotMsg );

    //Ticketing entry should display Corporate Id message

    _agent = _memHandle.create<Agent>();
    _customer = _memHandle.create<Customer>();
    _agent->tvlAgencyPCC() = "W0H3";
    _agent->agentTJR() = _customer;
    _agent->agentLocation() = TestLocFactory::create(LOC_DFW);
    _agent->currencyCodeAgent() = "USD";
    _request->ticketingAgent() = _agent;
    _trx->setRequest(_request);
    _trx->altTrxType() = PricingTrx::WP_WITH_RO;
    _trx->getRequest()->ticketEntry() = 'T';
    _calcTotals->fcMessage.clear();
    _fcC->collectMatchedAccCodeTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(size_t(1),_calcTotals->fcMessage.size());

    //AREX for TN should  display Corporate Id message

    setUpExcTrx();
    _agent = _memHandle.create<Agent>();
    _customer = _memHandle.create<Customer>();
    _agent->tvlAgencyPCC() = "W0H3";
    _agent->agentTJR() = _customer;
    _agent->agentLocation() = TestLocFactory::create(LOC_DFW);
    _agent->currencyCodeAgent() = "USD";
    _request->ticketingAgent() = _agent;
    _trx->setRequest(_request);
    _trx->altTrxType() = PricingTrx::WP_WITH_RO;
    _trx->getRequest()->ticketEntry() = 'N';
    _calcTotals->fcMessage.clear();
    _fcC->collectMatchedAccCodeTrailerMessage();

    //AREX for Airline should NOT display Corporate Id message

    CPPUNIT_ASSERT_EQUAL(size_t(1),_calcTotals->fcMessage.size());
    _agent->tvlAgencyPCC() = "";
    _trx->altTrxType() = PricingTrx::WP_WITH_RO;
    _calcTotals->fcMessage.clear();
    _fcC->collectMatchedAccCodeTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(size_t(0),_calcTotals->fcMessage.size());

  }
  void testBuildValidatingCarrierMessage_FromRequest()
  {
    FcMessage::MessageType msgType = FcMessage::WARNING;
    const std::string expectedMsg = "VALIDATING CARRIER SPECIFIED - 9B";
    setupCollectValidatingCarrierMessage( CARRIER_9B, CARRIER_9B );

    const std::string actualVcxrMsg = _fcC->buildValidatingCarrierMessage(msgType);

    CPPUNIT_ASSERT_EQUAL( expectedMsg, actualVcxrMsg );
    CPPUNIT_ASSERT_EQUAL(FcMessage::WARNING, msgType);
  }

  void testCollectServiceFeesTemplate_Regular()
  {
    _fcC->collectServiceFeesTemplate();
    CPPUNIT_ASSERT(_calcTotals->fcMessage.empty());
  }

  void testCollectServiceFeesTemplate_Issta()
  {
    _option->setServiceFeesTemplateRequested();
    _fcC->collectServiceFeesTemplate();
    CPPUNIT_ASSERT(!_calcTotals->fcMessage.empty());
  }

  void testBuildValidatingCarrierMessage_SpecifiedButNoVcxr()
  {
    FcMessage::MessageType msgType = FcMessage::WARNING;
    const std::string expectedMsg = "VALIDATING CARRIER - ";
    setupCollectValidatingCarrierMessage(CARRIER_9B, EMPTY_STRING());

    const std::string actualVcxrMsg = _fcC->buildValidatingCarrierMessage(msgType);

    CPPUNIT_ASSERT_EQUAL( expectedMsg, actualVcxrMsg );
    CPPUNIT_ASSERT_EQUAL(FcMessage::WARNING, msgType);
  }

  void testBuildValidatingCarrierMessage_FromItin()
  {
    FcMessage::MessageType msgType = FcMessage::WARNING;
    const std::string expectedMsg = "VALIDATING CARRIER - 2R";
    setupCollectValidatingCarrierMessage(EMPTY_STRING(), CARRIER_2R);

    const std::string actualVcxrMsg = _fcC->buildValidatingCarrierMessage(msgType);

    CPPUNIT_ASSERT_EQUAL( expectedMsg, actualVcxrMsg );
  }

  void testBuildValidatingCarrierMessage_FromItinEmpty()
  {
    FcMessage::MessageType msgType = FcMessage::WARNING;
    const std::string expectedMsg = "VALIDATING CARRIER - ";
    setupCollectValidatingCarrierMessage(EMPTY_STRING(), EMPTY_STRING());

    const std::string actualVcxrMsg = _fcC->buildValidatingCarrierMessage(msgType);

    CPPUNIT_ASSERT_EQUAL( expectedMsg, actualVcxrMsg );
  }

  void testBuildValidatingCarrierMessage_NoRequest()
  {
    FcMessage::MessageType msgType = FcMessage::WARNING;
    const std::string expectedMsg = "VALIDATING CARRIER - ";
    setupCollectValidatingCarrierMessage(EMPTY_STRING(), EMPTY_STRING());
    _trx->setRequest( 0 );

    const std::string actualVcxrMsg = _fcC->buildValidatingCarrierMessage(msgType);

    CPPUNIT_ASSERT_EQUAL( expectedMsg, actualVcxrMsg );
  }

  void testBuildValidatingCarrierMessage_NoItin()
  {
    FcMessage::MessageType msgType = FcMessage::WARNING;
    const std::string expectedMsg = "VALIDATING CARRIER - ";
    setupCollectValidatingCarrierMessage(EMPTY_STRING(), EMPTY_STRING());
    _farePath->itin() = 0;

    const std::string actualVcxrMsg = _fcC->buildValidatingCarrierMessage(msgType);

    CPPUNIT_ASSERT_EQUAL( expectedMsg, actualVcxrMsg );
  }

  void testBuildValidatingCarrierMessage_SingleGsaSwap_RequestedVcxr()
  {
    FcMessage::MessageType msgType = FcMessage::WARNING;
    const std::string expectedMsg = "VALIDATING CARRIER - 2R PER GSA AGREEMENT WITH 9B";
    setupCollectValidatingCarrierMessage( CARRIER_9B, CARRIER_2R );
    _farePath->marketingCxrForDefaultValCxr() = CARRIER_9B;

    const std::string actualVcxrMsg = _fcC->buildValidatingCarrierMessage(msgType);

    CPPUNIT_ASSERT_EQUAL( expectedMsg, actualVcxrMsg );
    CPPUNIT_ASSERT_EQUAL(FcMessage::VCX_SINGLE_GSA_SWAP, msgType);
  }

  void testBuildValidatingCarrierMessage_SingleGsaSwap_DefaultVcxr()
  {
    FcMessage::MessageType msgType = FcMessage::WARNING;
    const std::string expectedMsg = "VALIDATING CARRIER - 2R PER GSA AGREEMENT WITH M1";
    setupCollectValidatingCarrierMessage(EMPTY_STRING(), CARRIER_2R);
    _farePath->marketingCxrForDefaultValCxr() = "M1";

    const std::string actualVcxrMsg = _fcC->buildValidatingCarrierMessage(msgType);

    CPPUNIT_ASSERT_EQUAL( expectedMsg, actualVcxrMsg );
    CPPUNIT_ASSERT_EQUAL(FcMessage::VCX_SINGLE_GSA_SWAP, msgType);
  }

  void testBuildValidatingCarrierMessage_ValidatingCxr_empty_Alternate_Has_Two()
  {
    FcMessage::MessageType msgType = FcMessage::WARNING;
    const std::string expectedMsg = "ALTERNATE VALIDATING CARRIER/S - AF DL ";
    _farePath->validatingCarriers().push_back("DL");
    _farePath->validatingCarriers().push_back("AF");

    AirSeg* airSeg1 = dynamic_cast<AirSeg*>(_itin->travelSeg().front());
    airSeg1->setMarketingCarrierCode("KL");
    AirSeg* airSeg2 = dynamic_cast<AirSeg*>(_itin->travelSeg().back());
    airSeg2->setMarketingCarrierCode("KL");


    ValidatingCxrGSAData v;
    _itin->validatingCxrGsaData() = &v;

    v.gsaSwapMap()["KL"].insert("DL");
    v.gsaSwapMap()["KL"].insert("AF");

    setupCollectValidatingCarrierMessage(EMPTY_STRING(), EMPTY_STRING());

    const std::string actualVcxrMsg = _fcC->buildValidatingCarrierMessage(msgType);

    CPPUNIT_ASSERT_EQUAL( expectedMsg, actualVcxrMsg );
    CPPUNIT_ASSERT_EQUAL(size_t(1),_calcTotals->fcMessage.size());
    CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER - "),
                         _calcTotals->fcMessage.front().messageText());
  }

  void testBuildValidatingCarrierMessage_ValidatingCxr_empty_Alternate_NotFitInOneLine()
  {
    FcMessage::MessageType msgType = FcMessage::WARNING;
    const std::string expectedMsg = "MU PP ";
    _farePath->validatingCarriers().push_back("PP");
    _farePath->validatingCarriers().push_back("BA");
    _farePath->validatingCarriers().push_back("AB");
    _farePath->validatingCarriers().push_back("BB");
    _farePath->validatingCarriers().push_back("CC");
    _farePath->validatingCarriers().push_back("LL");
    _farePath->validatingCarriers().push_back("MM");
    _farePath->validatingCarriers().push_back("MH");
    _farePath->validatingCarriers().push_back("MU");
    _farePath->validatingCarriers().push_back("BI");
    _farePath->validatingCarriers().push_back("II");
    _farePath->validatingCarriers().push_back("AB");

    AirSeg* airSeg1 = dynamic_cast<AirSeg*>(_itin->travelSeg().front());
    airSeg1->setMarketingCarrierCode("LA");
    AirSeg* airSeg2 = dynamic_cast<AirSeg*>(_itin->travelSeg().back());
    airSeg2->setMarketingCarrierCode("AA");


    ValidatingCxrGSAData v;
    _itin->validatingCxrGsaData() = &v;

    v.gsaSwapMap()["LA"].insert("LL");
    v.gsaSwapMap()["LA"].insert("MM");
    v.gsaSwapMap()["LA"].insert("AB");
    v.gsaSwapMap()["LA"].insert("BA");
    v.gsaSwapMap()["LA"].insert("BB");
    v.gsaSwapMap()["LA"].insert("CC");
    v.gsaSwapMap()["AA"].insert("MH");
    v.gsaSwapMap()["AA"].insert("MU");
    v.gsaSwapMap()["AA"].insert("PP");
    v.gsaSwapMap()["AA"].insert("BI");
    v.gsaSwapMap()["AA"].insert("II");

    setupCollectValidatingCarrierMessage(EMPTY_STRING(), EMPTY_STRING());

    const std::string actualVcxrMsg = _fcC->buildValidatingCarrierMessage(msgType);

    CPPUNIT_ASSERT_EQUAL( expectedMsg, actualVcxrMsg );
    CPPUNIT_ASSERT_EQUAL(size_t(2),_calcTotals->fcMessage.size());
    CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER - "),
                         _calcTotals->fcMessage.front().messageText());
    CPPUNIT_ASSERT_EQUAL(std::string("ALTERNATE VALIDATING CARRIER/S - AB BA BB CC LL MM BI II MH"),
                         _calcTotals->fcMessage[1].messageText());
  }

   // Multiple carriers on the settlement plan.
   void testBuildValidatingCarrierMessage_MultipleSettlementPlans()
   {
     _trx->setValidatingCxrGsaApplicable(true);
     _customer->settlementPlans() = "BSPGEN";
     const CarrierCode VC1 = "AR";
     const CarrierCode VC2 = "4M";
     const CarrierCode VC3 = "AV";
     const CarrierCode VC4 = "HR";
     const CarrierCode VC5 = "YO";

     const SettlementPlanType SP1 = "BSP";
     const SettlementPlanType SP2 = "GEN";
     const SettlementPlanType SP3 = "KRY";

     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->defaultValCxrPerSp()[SP1] = "";
     farePath->defaultValCxrPerSp()[SP2] = VC2;
     farePath->defaultValCxrPerSp()[SP3] = VC3;
     farePath->validatingCarriers().push_back(VC1);
     farePath->validatingCarriers().push_back(VC2);
     farePath->validatingCarriers().push_back(VC3);
     farePath->validatingCarriers().push_back(VC4);
     farePath->validatingCarriers().push_back(VC5);
     farePath->processed() = true;
     farePath->itin() = _itin;
     std::vector<SettlementPlanType> spTypes;
     spTypes.push_back( SP1 );
     spTypes.push_back( SP2 );
     spTypes.push_back( SP3 );

     farePath->settlementPlanValidatingCxrs()[SP1].push_back( VC4 );
     farePath->settlementPlanValidatingCxrs()[SP2].push_back( VC2 );
     farePath->settlementPlanValidatingCxrs()[SP2].push_back( VC1 );
     farePath->settlementPlanValidatingCxrs()[SP2].push_back( VC3 );
     farePath->settlementPlanValidatingCxrs()[SP3].push_back( VC3 );

     vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
     vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
     vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
     vcx::ValidatingCxrData vcxrData;
     vcxrData.participatingCxrs.push_back(pc1);
     vcxrData.participatingCxrs.push_back(pc2);
     vcxrData.participatingCxrs.push_back(pc3);
     vcxrData.ticketType = vcx::ETKT_PREF;

     ValidatingCxrGSAData gsaData;
     ValidatingCxrGSAData gsaData1;
     gsaData.validatingCarriersData()[VC1] = vcxrData;
     gsaData.validatingCarriersData()[VC2] = vcxrData;
     gsaData1.validatingCarriersData()[VC3] = vcxrData;
     gsaData1.isNeutralValCxr() = true;

     setSpValidatingCxrGsaData(*farePath, &gsaData, {SP2, SP3});
     setSpValidatingCxrGsaData(*farePath, &gsaData1, {SP1});

     setupCollectValidatingCarrierMessage(EMPTY_STRING(), EMPTY_STRING());
     _calcTotals->farePath = farePath;
     _fcC->collectValidatingCarrierMessage();

     CPPUNIT_ASSERT_EQUAL(size_t(8),_calcTotals->fcMessage.size());
     CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER"),
         _calcTotals->fcMessage.front().messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - "),
         _calcTotals->fcMessage[1].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("GEN - 4M"),
         _calcTotals->fcMessage[2].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("KRY - AV"),
         _calcTotals->fcMessage[3].messageText());
   }

  void testValidatingCxrCommissionAmount_NonGSA()
  {
    _trx->setValidatingCxrGsaApplicable( false );
    _trx->getRequest()->validatingCarrier() = "";
    _itin->validatingCarrier() = "9B";
    _trx->ticketingDate() = DateTime(2020, 1, 1);
    _fcConfig->valCxrDisplayOpt() = YES;

    FarePath* farePath = FarePathBuilder(&_memHandle).build();
    farePath->processed() = true;
    farePath->itin() = _itin;

    _calcTotals->convertedBaseFareCurrencyCode = "USD";
    _calcTotals->farePath = farePath;
    _fcC->collectValidatingCarrierMessage();

    CPPUNIT_ASSERT( !_calcTotals->fcMessage.empty() );
    CPPUNIT_ASSERT_EQUAL(size_t(1), _calcTotals->fcMessage.size());
    CPPUNIT_ASSERT_EQUAL( std::string("VALIDATING CARRIER - 9B"),
        _calcTotals->fcMessage.front().messageText() );
  }

  void testValidatingCxrCommissionAmount_GSA()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    _trx->setValidatingCxrGsaApplicable( true );
    _trx->getRequest()->validatingCarrier() = "";
    _itin->validatingCarrier() = "9B";
    _trx->ticketingDate() = DateTime(2020, 1, 1);
    _fcConfig->valCxrDisplayOpt() = YES;

    FarePath* farePath = FarePathBuilder(&_memHandle).build();
    farePath->processed() = true;
    farePath->itin() = _itin;
    farePath->defaultValidatingCarrier() = "9B";

    _calcTotals->convertedBaseFareCurrencyCode = "USD";
    _calcTotals->farePath = farePath;
    _fcC->collectValidatingCarrierMessage();

    CPPUNIT_ASSERT( !_calcTotals->fcMessage.empty() );
    CPPUNIT_ASSERT_EQUAL( std::string("VALIDATING CARRIER - 9B"),
        _calcTotals->fcMessage.front().messageText() );
  }

   void testValidatingCxrCommissionAmount_ForMultipSp()
   {
     _trx->setValidatingCxrGsaApplicable(true);
     _customer->settlementPlans() = "BSPGEN";
     const CarrierCode AR = "AR";
     const CarrierCode FM = "4M";
     const CarrierCode AV = "AV";
     const CarrierCode HR = "HR";
     const CarrierCode YO = "YO";

     const SettlementPlanType BSP = "BSP";
     const SettlementPlanType GEN = "GEN";
     const SettlementPlanType KRY = "KRY";

     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->defaultValCxrPerSp()[BSP] = "";
     farePath->defaultValCxrPerSp()[GEN] = FM;
     farePath->defaultValCxrPerSp()[KRY] = AV;
     farePath->validatingCarriers().push_back(AR);
     farePath->validatingCarriers().push_back(FM);
     farePath->validatingCarriers().push_back(AV);
     farePath->validatingCarriers().push_back(HR);
     farePath->validatingCarriers().push_back(YO);
     farePath->processed() = true;
     farePath->itin() = _itin;
     std::vector<SettlementPlanType> spTypes;
     spTypes.push_back( BSP );
     spTypes.push_back( GEN );
     spTypes.push_back( KRY );

     farePath->settlementPlanValidatingCxrs()[BSP].push_back( HR );
     farePath->settlementPlanValidatingCxrs()[GEN].push_back( FM );
     farePath->settlementPlanValidatingCxrs()[GEN].push_back( AR );
     farePath->settlementPlanValidatingCxrs()[GEN].push_back( AV );
     farePath->settlementPlanValidatingCxrs()[KRY].push_back( AV );

     vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
     vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
     vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
     vcx::ValidatingCxrData vcxrData;
     vcxrData.participatingCxrs.push_back(pc1);
     vcxrData.participatingCxrs.push_back(pc2);
     vcxrData.participatingCxrs.push_back(pc3);
     vcxrData.ticketType = vcx::ETKT_PREF;

     ValidatingCxrGSAData gsaData;
     ValidatingCxrGSAData gsaData1;
     gsaData.validatingCarriersData()[AR] = vcxrData;
     gsaData.validatingCarriersData()[FM] = vcxrData;
     gsaData1.validatingCarriersData()[AV] = vcxrData;
     gsaData1.isNeutralValCxr() = true;

     setSpValidatingCxrGsaData(*farePath, &gsaData, {GEN, KRY});
     setSpValidatingCxrGsaData(*farePath, &gsaData1, {BSP});
     setupCollectValidatingCarrierMessage(EMPTY_STRING(), EMPTY_STRING());

     _calcTotals->farePath = farePath;
     _calcTotals->convertedBaseFareCurrencyCode = "USD";
     _fcC->collectValidatingCarrierMessage();

     CPPUNIT_ASSERT_EQUAL(size_t(8),_calcTotals->fcMessage.size());
     CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER"),
         _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - "),
         _calcTotals->fcMessage[1].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("GEN - 4M"),
         _calcTotals->fcMessage[2].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("KRY - AV"),
         _calcTotals->fcMessage[3].messageText());

     CPPUNIT_ASSERT_EQUAL(std::string("ALTERNATE VALIDATING CARRIER/S"),
         _calcTotals->fcMessage[4].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("GEN - AR AV"), _calcTotals->fcMessage[5].messageText());

     CPPUNIT_ASSERT_EQUAL(std::string("OPTIONAL VALIDATING CARRIER"),
         _calcTotals->fcMessage[6].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - HR"), _calcTotals->fcMessage[7].messageText());
   }

   void testSetValCxrTrailerMessages_EmptyTrailerMsgCol()
   {
     std::vector<std::string> trailerMsgCol;
     _fcC->setValCxrTrailerMessages("", trailerMsgCol);
     CPPUNIT_ASSERT_EQUAL(size_t(0),_calcTotals->fcMessage.size());
   }

   void testSetValCxrTrailerMessages_BlankValCxrTrailerMsg()
   {
     // BLANK ValidatingCxr Test
     std::string header = "VALIDATING CARRIER";
     std::vector<std::string> trailerMsgCol;
     trailerMsgCol.push_back("BSP - ");
     _fcC->setValCxrTrailerMessages(header, trailerMsgCol);
     CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER"), _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - "), _calcTotals->fcMessage[1].messageText());
   }

   void testSetValCxrTrailerMessages_ValCxrTrailerMsg()
   {
     // Single ValidatingCxr Test
     std::string header = "VALIDATING CARRIER";
     std::vector<std::string> trailerMsgCol;
     trailerMsgCol.push_back("BSP - AA");
     _fcC->setValCxrTrailerMessages(header, trailerMsgCol);
     CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER"), _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(FcMessage::VALIDATINGCXR, _calcTotals->fcMessage[0].messageSubType());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA"), _calcTotals->fcMessage[1].messageText());
   }

   void testSetValCxrTrailerMessages_MultipleValCxrTrailerMsg()
   {
     // Multiple ValidatingCxr Test
     std::string header = "VALIDATING CARRIER";
     std::vector<std::string> trailerMsgCol;
     trailerMsgCol.push_back("BSP - AA");
     trailerMsgCol.push_back("GEN - AA");
     _fcC->setValCxrTrailerMessages(header, trailerMsgCol);
     CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER"), _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(FcMessage::VALIDATINGCXR, _calcTotals->fcMessage[0].messageSubType());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA"), _calcTotals->fcMessage[1].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("GEN - AA"), _calcTotals->fcMessage[2].messageText());
   }

   void testSetValCxrTrailerMessages_AlternateValCxrTrailerMsg()
   {
     // Alternate ValidatingCxr Test
     std::string header = "ALTERNATE VALIDATING CARRIER/S";
     std::vector<std::string> trailerMsgCol;
     trailerMsgCol.push_back("BSP - AA BA");
     trailerMsgCol.push_back("GEN - AA");
     _fcC->setValCxrTrailerMessages(header, trailerMsgCol);
     CPPUNIT_ASSERT_EQUAL(std::string("ALTERNATE VALIDATING CARRIER/S"), _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(FcMessage::VALIDATINGCXR, _calcTotals->fcMessage[0].messageSubType());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA BA"), _calcTotals->fcMessage[1].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("GEN - AA"), _calcTotals->fcMessage[2].messageText());
   }

   void testSetValCxrTrailerMessages_OptionalValCxrTrailerMsg()
   {
     // Optional ValidatingCxr Test
     std::string header = "OPTIONAL VALIDATING CARRIER";
     std::vector<std::string> trailerMsgCol;
     trailerMsgCol.push_back("BSP - HR YO");
     _fcC->setValCxrTrailerMessages(header, trailerMsgCol);
     CPPUNIT_ASSERT_EQUAL(std::string("OPTIONAL VALIDATING CARRIER"), _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(FcMessage::VALIDATINGCXR, _calcTotals->fcMessage[0].messageSubType());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - HR YO"), _calcTotals->fcMessage[1].messageText());
   }

   void testBuildValidatingCarrierMessage_NoValidatingCxr()
   {
     std::vector<CarrierCode> marketingCxrs;
     std::vector<CarrierCode> valCxrs;
     std::vector<std::string> defValCxrMsgCol;
     std::vector<std::string> altValCxrMsgCol;
     std::vector<std::string> optValCxrMsgCol;

     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     _calcTotals->farePath = farePath;
     farePath->itin() = _itin;

     const SettlementPlanType sp("BSP");
     setAllValidatingCarriersForSp(sp, valCxrs, *farePath);
     _fcC->buildValidatingCarrierMessage(*_calcTotals->farePath,
         "BSP",
         "",
         marketingCxrs,
         valCxrs,
         defValCxrMsgCol,
         altValCxrMsgCol,
         optValCxrMsgCol);
     CPPUNIT_ASSERT_EQUAL(size_t(1), defValCxrMsgCol.size());
     CPPUNIT_ASSERT_EQUAL(size_t(0), altValCxrMsgCol.size());
     CPPUNIT_ASSERT_EQUAL(size_t(0), optValCxrMsgCol.size());
   }

   void testBuildValidatingCarrierMessage_BlankValidatingCxr()
   {
     const SettlementPlanType sp("BSP");
     std::vector<CarrierCode> marketingCxrs;
     std::vector<CarrierCode> valCxrs;
     std::vector<std::string> defValCxrMsgCol;
     std::vector<std::string> altValCxrMsgCol;
     std::vector<std::string> optValCxrMsgCol;

     CarrierCode AA("AA");
     CarrierCode BA("BA");
     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->defaultValCxrPerSp()[sp]="";
     farePath->settlementPlanValidatingCxrs()[sp].push_back(AA);
     farePath->settlementPlanValidatingCxrs()[sp].push_back(BA);
     farePath->itin() = _itin;

     _calcTotals->farePath = farePath;
     setAllValidatingCarriersForSp(sp, valCxrs, *farePath);
     for(const auto& pair : _calcTotals->farePath->defaultValCxrPerSp())
       _fcC->buildValidatingCarrierMessage(*_calcTotals->farePath,
           pair.first,
           pair.second,
           marketingCxrs,
           valCxrs,
           defValCxrMsgCol,
           altValCxrMsgCol,
           optValCxrMsgCol);
     CPPUNIT_ASSERT_EQUAL(size_t(1), defValCxrMsgCol.size());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - "), defValCxrMsgCol[0]);
     CPPUNIT_ASSERT_EQUAL(size_t(1), altValCxrMsgCol.size());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA BA"), altValCxrMsgCol[0]);
     CPPUNIT_ASSERT_EQUAL(size_t(0), optValCxrMsgCol.size());
   }

   void testBuildValidatingCarrierMessage_DefaultValidatingCxr()
   {
     const SettlementPlanType sp("BSP");
     std::vector<CarrierCode> marketingCxrs;
     std::vector<CarrierCode> valCxrs;
     std::vector<std::string> defValCxrMsgCol;
     std::vector<std::string> altValCxrMsgCol;
     std::vector<std::string> optValCxrMsgCol;

     CarrierCode AA("AA");
     CarrierCode BA("BA");
     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->defaultValCxrPerSp()[sp]=AA;
     farePath->settlementPlanValidatingCxrs()[sp].push_back(AA);
     farePath->settlementPlanValidatingCxrs()[sp].push_back(BA);
     farePath->itin() = _itin;

     _calcTotals->farePath = farePath;
     setAllValidatingCarriersForSp(sp, valCxrs, *farePath);
     for(const auto& pair : _calcTotals->farePath->defaultValCxrPerSp())
       _fcC->buildValidatingCarrierMessage(*_calcTotals->farePath,
           pair.first,
           pair.second,
           marketingCxrs,
           valCxrs,
           defValCxrMsgCol,
           altValCxrMsgCol,
           optValCxrMsgCol);
     CPPUNIT_ASSERT_EQUAL(size_t(1), defValCxrMsgCol.size());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA"), defValCxrMsgCol[0]);
     CPPUNIT_ASSERT_EQUAL(size_t(1), altValCxrMsgCol.size());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - BA"), altValCxrMsgCol[0]);
     CPPUNIT_ASSERT_EQUAL(size_t(0), optValCxrMsgCol.size());
   }

   void testBuildValidatingCarrierMessage_OptionalValidatingCxr()
   {
     const SettlementPlanType sp("BSP");
     std::vector<CarrierCode> marketingCxrs;
     std::vector<CarrierCode> valCxrs;
     std::vector<std::string> defValCxrMsgCol;
     std::vector<std::string> altValCxrMsgCol;
     std::vector<std::string> optValCxrMsgCol;

     CarrierCode AA("AA");
     CarrierCode HR("HR");
     CarrierCode YO("YO");
     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->defaultValCxrPerSp()[sp]=AA;
     farePath->settlementPlanValidatingCxrs()[sp].push_back(AA);
     farePath->settlementPlanValidatingCxrs()[sp].push_back(HR);
     farePath->settlementPlanValidatingCxrs()[sp].push_back(YO);

     SpValidatingCxrGSADataMap spGsaDataMap;
     ValidatingCxrGSAData v;
     v.isNeutralValCxr() = true;
     spGsaDataMap[sp] = &v;
     _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

     farePath->itin() = _itin;

     _calcTotals->farePath = farePath;
     setAllValidatingCarriersForSp(sp, valCxrs, *farePath);
     for(const auto& pair : _calcTotals->farePath->defaultValCxrPerSp())
       _fcC->buildValidatingCarrierMessage(*_calcTotals->farePath,
           pair.first,
           pair.second,
           marketingCxrs,
           valCxrs,
           defValCxrMsgCol,
           altValCxrMsgCol,
           optValCxrMsgCol);
     CPPUNIT_ASSERT_EQUAL(size_t(1), defValCxrMsgCol.size());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA"), defValCxrMsgCol[0]);
     CPPUNIT_ASSERT_EQUAL(size_t(0), altValCxrMsgCol.size());
     CPPUNIT_ASSERT_EQUAL(size_t(1), optValCxrMsgCol.size());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - HR YO"), optValCxrMsgCol[0]);
   }

   void testBuildValidatingCarrierMessage_GSASwappedCxr()
   {
     const SettlementPlanType sp("BSP");
     std::vector<CarrierCode> marketingCxrs;
     std::vector<CarrierCode> valCxrs;
     std::vector<std::string> defValCxrMsgCol;
     std::vector<std::string> altValCxrMsgCol;
     std::vector<std::string> optValCxrMsgCol;

     CarrierCode AA("AA");
     CarrierCode BA("BA");
     CarrierCode HR("HR");
     CarrierCode YO("YO");

     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->marketingCxrForDefaultValCxrPerSp()[sp] = BA;
     farePath->defaultValCxrPerSp()[sp]=AA;
     farePath->settlementPlanValidatingCxrs()[sp].push_back(AA);
     farePath->settlementPlanValidatingCxrs()[sp].push_back(HR);
     farePath->settlementPlanValidatingCxrs()[sp].push_back(YO);

     SpValidatingCxrGSADataMap spGsaDataMap;
     ValidatingCxrGSAData v;
     v.isNeutralValCxr() = true;
     spGsaDataMap[sp] = &v;
     _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

     farePath->itin() = _itin;

     _calcTotals->farePath = farePath;
     setAllValidatingCarriersForSp(sp, valCxrs, *farePath);
     for(const auto& pair : _calcTotals->farePath->defaultValCxrPerSp())
       _fcC->buildValidatingCarrierMessage(*_calcTotals->farePath,
           pair.first,
           pair.second,
           marketingCxrs,
           valCxrs,
           defValCxrMsgCol,
           altValCxrMsgCol,
           optValCxrMsgCol);
     CPPUNIT_ASSERT_EQUAL(size_t(1), defValCxrMsgCol.size());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA PER GSA AGREEMENT WITH BA"), defValCxrMsgCol[0]);
     CPPUNIT_ASSERT_EQUAL(size_t(0), altValCxrMsgCol.size());
     CPPUNIT_ASSERT_EQUAL(size_t(1), optValCxrMsgCol.size());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - HR YO"), optValCxrMsgCol[0]);
   }

   // No Default with Alternated Validating Carrier with one Settlement Plan Test
   void testPrepareValCxrMsgForMultiSp_NoDefaultValCxr()
   {
     std::vector<std::string> defValCxrMsgCol;
     std::vector<std::string> altValCxrMsgCol;
     std::vector<std::string> optValCxrMsgCol;

     SettlementPlanType BSP("BSP");
     CarrierCode AA("AA");
     CarrierCode BA("BA");

     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->defaultValCxrPerSp()[BSP]="";
     farePath->settlementPlanValidatingCxrs()[BSP].push_back(AA);
     farePath->settlementPlanValidatingCxrs()[BSP].push_back(BA);
     farePath->itin() = _itin;
     _calcTotals->farePath = farePath;

     _fcC->prepareValCxrMsgForMultiSp();
     CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER"), _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - "), _calcTotals->fcMessage[1].messageText());

     CPPUNIT_ASSERT_EQUAL(std::string("ALTERNATE VALIDATING CARRIER/S"), _calcTotals->fcMessage[2].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA BA"), _calcTotals->fcMessage[3].messageText());
   }

   // Default and Alternate Validating Carrier Test with one Settlement Plan
   void testPrepareValCxrMsgForMultiSp_DefaultValCxr()
   {
     std::vector<std::string> defValCxrMsgCol;
     std::vector<std::string> altValCxrMsgCol;
     std::vector<std::string> optValCxrMsgCol;

     SettlementPlanType BSP("BSP");
     CarrierCode AA("AA");
     CarrierCode BA("BA");

     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->defaultValCxrPerSp()[BSP] = AA;
     farePath->settlementPlanValidatingCxrs()[BSP].push_back(AA);
     farePath->settlementPlanValidatingCxrs()[BSP].push_back(BA);
     farePath->itin() = _itin;
     _calcTotals->farePath = farePath;

     _fcC->prepareValCxrMsgForMultiSp();
     CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER"), _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA"), _calcTotals->fcMessage[1].messageText());

     CPPUNIT_ASSERT_EQUAL(std::string("ALTERNATE VALIDATING CARRIER/S"), _calcTotals->fcMessage[2].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - BA"), _calcTotals->fcMessage[3].messageText());
   }

   // Default, Alternates, and Optional Validating Carriers
   // Two Settlement Plans
   void testPrepareValCxrMsgForMultiSp_OptionalValCxr()
   {
     std::vector<std::string> defValCxrMsgCol;
     std::vector<std::string> altValCxrMsgCol;
     std::vector<std::string> optValCxrMsgCol;

     SettlementPlanType BSP("BSP");
     SettlementPlanType GEN("GEN");

     CarrierCode AA("AA");
     CarrierCode BA("BA");
     CarrierCode HR("HR");
     CarrierCode YO("YO");

     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->defaultValCxrPerSp()[BSP]=AA;
     farePath->defaultValCxrPerSp()[GEN]=BA;
     farePath->settlementPlanValidatingCxrs()[BSP].push_back(AA);
     farePath->settlementPlanValidatingCxrs()[GEN].push_back(BA);
     farePath->settlementPlanValidatingCxrs()[GEN].push_back(HR);
     farePath->settlementPlanValidatingCxrs()[GEN].push_back(YO);

     SpValidatingCxrGSADataMap spGsaDataMap;
     ValidatingCxrGSAData v;
     v.isNeutralValCxr() = true;
     spGsaDataMap[GEN] = &v;
     _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

     farePath->itin() = _itin;

     _calcTotals->farePath = farePath;
     farePath->itin() = _itin;
     _calcTotals->farePath = farePath;

     _fcC->prepareValCxrMsgForMultiSp();
     CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER"), _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA"), _calcTotals->fcMessage[1].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("GEN - BA"), _calcTotals->fcMessage[2].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("OPTIONAL VALIDATING CARRIER"), _calcTotals->fcMessage[3].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("GEN - HR YO"), _calcTotals->fcMessage[4].messageText());
   }

   // Specified Validating Carrier Test with single Settlement Plan
   void testPrepareValCxrMsgForMultiSp_SpecifiedValCxr()
   {
     std::vector<std::string> defValCxrMsgCol;
     std::vector<std::string> altValCxrMsgCol;
     std::vector<std::string> optValCxrMsgCol;

     SettlementPlanType BSP("BSP");
     CarrierCode AA("AA");

     _trx->getRequest()->validatingCarrier() = AA;

     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->defaultValCxrPerSp()[BSP] = AA;
     farePath->settlementPlanValidatingCxrs()[BSP].push_back(AA);
     farePath->itin() = _itin;

     _calcTotals->farePath = farePath;

     _fcC->prepareValCxrMsgForMultiSp();

     CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER SPECIFIED "), _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA"), _calcTotals->fcMessage[1].messageText());
   }

   void testPrepareValCxrMsgForMultiSp_GSASwappedCxr()
   {
     std::vector<std::string> defValCxrMsgCol;
     std::vector<std::string> altValCxrMsgCol;
     std::vector<std::string> optValCxrMsgCol;

     SettlementPlanType BSP("BSP");
     CarrierCode AA("AA");
     CarrierCode BA("BA");

     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->marketingCxrForDefaultValCxrPerSp()[BSP] = BA;
     farePath->defaultValCxrPerSp()[BSP]=AA;
     farePath->settlementPlanValidatingCxrs()[BSP].push_back(AA);

     farePath->itin() = _itin;

     _calcTotals->farePath = farePath;
     _fcC->prepareValCxrMsgForMultiSp();

     CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER"), _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA PER GSA AGREEMENT WITH BA"), _calcTotals->fcMessage[1].messageText());
   }

   // with no def val cxr
   // with def val cxr without comm
   // with def val cxr with comm
   void test_constructCommTrailerMsgForDefaultValCxr_WithNoDefValCxr()
   {
     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->processed() = true;
     farePath->itin() = _itin;

     _trx->setValidatingCxrGsaApplicable(true);
     _calcTotals->farePath = farePath;

     const CarrierCode defValCxr = "";
     const std::string expMsg("");

     _fcC->constructCommTrailerMsgForDefaultValCxr(*farePath, defValCxr, "");
     CPPUNIT_ASSERT_EQUAL(size_t(0),_calcTotals->fcMessage.size());
   }

   void test_constructCommTrailerMsgForDefaultValCxr_WithDefValCxrNoComm()
   {
     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->processed() = true;
     farePath->itin() = _itin;

     _trx->setValidatingCxrGsaApplicable(true);
     _calcTotals->farePath = farePath;

     const CarrierCode defValCxr = "AA";
     const std::string expMsg("AGENCY COMMISSION DATA NOT FOUND FOR VAL CXR AA");
     _fcC->constructCommTrailerMsgForDefaultValCxr(*farePath, defValCxr, "");
     CPPUNIT_ASSERT_EQUAL(expMsg, _calcTotals->fcMessage[0].messageText());
   }

   void test_constructCommTrailerMsgForDefaultValCxr_WithDefValCxrWithComm()
   {
     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->processed() = true;
     farePath->itin() = _itin;
     farePath->storeCommissionForValidatingCarrier("AA", 123.45);

     _trx->setValidatingCxrGsaApplicable(true);
     _calcTotals->equivCurrencyCode = "USD";
     _calcTotals->equivNoDec = 2;
     _calcTotals->farePath = farePath;

     const CarrierCode defValCxr = "AA";
     const std::string expMsg("AGENCY COMMISSION VAL CARRIER AA - USD123.45");
     _fcC->constructCommTrailerMsgForDefaultValCxr(*farePath, defValCxr, "");
     CPPUNIT_ASSERT_EQUAL(expMsg, _calcTotals->fcMessage[0].messageText());
   }

   void test_constructCommTrailerMsgForAlternateValCxr_WithNoAltValCxr()
   {
     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->processed() = true;
     farePath->itin() = _itin;

     _trx->setValidatingCxrGsaApplicable(true);
     _calcTotals->farePath = farePath;

     _fcC->constructCommTrailerMsgForAlternateValCxr(*farePath, "ALT");
     CPPUNIT_ASSERT_EQUAL(size_t(0),_calcTotals->fcMessage.size());
   }

   void test_constructCommTrailerMsgForAlternateValCxr_WithAltValCxrsWithNoComm()
   {
     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->processed() = true;
     farePath->itin() = _itin;
     farePath->validatingCarriers().push_back("CA");
     farePath->validatingCarriers().push_back("BA");

     _trx->setValidatingCxrGsaApplicable(true);
     _calcTotals->farePath = farePath;

     const std::string expMsg("AGENCY COMMISSION DATA NOT FOUND FOR ALT VAL CXR BA/CA");
     _fcC->constructCommTrailerMsgForAlternateValCxr(*farePath, "ALT");
     CPPUNIT_ASSERT_EQUAL(expMsg, _calcTotals->fcMessage[0].messageText());
   }

   void test_constructCommTrailerMsgForAlternateValCxr_WithAltValCxrsWithSameComm()
   {
     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->processed() = true;
     farePath->itin() = _itin;
     farePath->validatingCarriers().push_back("BA");
     farePath->validatingCarriers().push_back("CA");
     farePath->storeCommissionForValidatingCarrier("BA", 123.45);
     farePath->storeCommissionForValidatingCarrier("CA", 123.45);

     _trx->setValidatingCxrGsaApplicable(true);
     _calcTotals->equivCurrencyCode = "USD";
     _calcTotals->equivNoDec = 2;
     _calcTotals->farePath = farePath;

     const std::string expMsg("AGENCY COMMISSION ALT VAL CARRIER/S BA/CA - USD123.45");
     _fcC->constructCommTrailerMsgForAlternateValCxr(*farePath, "ALT");
     CPPUNIT_ASSERT_EQUAL(expMsg, _calcTotals->fcMessage[0].messageText());
   }

   void test_constructCommTrailerMsgForAlternateValCxr_WithAltValCxrsWithDiffComm()
   {
     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->processed() = true;
     farePath->itin() = _itin;
     farePath->validatingCarriers().push_back("BA");
     farePath->validatingCarriers().push_back("CA");
     farePath->storeCommissionForValidatingCarrier("BA", 123.45);
     farePath->storeCommissionForValidatingCarrier("CA", 223.45);

     _trx->setValidatingCxrGsaApplicable(true);
     _calcTotals->equivCurrencyCode = "USD";
     _calcTotals->equivNoDec = 2;
     _calcTotals->farePath = farePath;

     const std::string expMsg1("AGENCY COMMISSION ALT VAL CARRIER/S BA - USD123.45");
     const std::string expMsg2("AGENCY COMMISSION ALT VAL CARRIER/S CA - USD223.45");

     _fcC->constructCommTrailerMsgForAlternateValCxr(*farePath, "ALT");
     CPPUNIT_ASSERT_EQUAL(expMsg2, _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(expMsg1, _calcTotals->fcMessage[1].messageText());
   }

   void test_constructCommTrailerMsgForAlternateValCxr_Mixed()
   {
     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->processed() = true;
     farePath->itin() = _itin;
     farePath->validatingCarriers().push_back("BA");
     farePath->validatingCarriers().push_back("CA");
     farePath->validatingCarriers().push_back("DA");
     farePath->validatingCarriers().push_back("FA");
     farePath->validatingCarriers().push_back("EA");
     farePath->storeCommissionForValidatingCarrier("BA", 123.45);
     farePath->storeCommissionForValidatingCarrier("CA", 223.45);
     farePath->storeCommissionForValidatingCarrier("DA", 123.45);

     _trx->setValidatingCxrGsaApplicable(true);
     _calcTotals->equivCurrencyCode = "USD";
     _calcTotals->equivNoDec = 2;
     _calcTotals->farePath = farePath;

     const std::string expMsg1("AGENCY COMMISSION ALT VAL CARRIER/S CA - USD223.45");
     const std::string expMsg2("AGENCY COMMISSION ALT VAL CARRIER/S BA/DA - USD123.45");
     const std::string expMsg3("AGENCY COMMISSION DATA NOT FOUND FOR ALT VAL CXR EA/FA");

     _fcC->constructCommTrailerMsgForAlternateValCxr(*farePath, "ALT");
     CPPUNIT_ASSERT_EQUAL(size_t(3),_calcTotals->fcMessage.size());
     CPPUNIT_ASSERT_EQUAL(expMsg1, _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(expMsg2, _calcTotals->fcMessage[1].messageText());
     CPPUNIT_ASSERT_EQUAL(expMsg3, _calcTotals->fcMessage[2].messageText());
   }

   void test_constructCommTrailerMsgForOptionalValCxr()
   {
    ValidatingCxrGSAData v;
    v.isNeutralValCxr() = true;
    _itin->validatingCxrGsaData() = &v;

     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->processed() = true;
     farePath->itin() = _itin;
     farePath->defaultValidatingCarrier() = "";
     farePath->validatingCarriers().push_back("HO");
     farePath->validatingCarriers().push_back("YO");
     farePath->validatingCarriers().push_back("LO");
     farePath->storeCommissionForValidatingCarrier("HO", 123.45);
     farePath->storeCommissionForValidatingCarrier("YO", 223.45);

     _trx->setValidatingCxrGsaApplicable(true);
     _calcTotals->equivCurrencyCode = "USD";
     _calcTotals->equivNoDec = 2;
     _calcTotals->farePath = farePath;

     const std::string expMsg1("AGENCY COMMISSION OPT VAL CARRIER/S HO - USD123.45");
     const std::string expMsg2("AGENCY COMMISSION OPT VAL CARRIER/S YO - USD223.45");
     const std::string expMsg3("AGENCY COMMISSION DATA NOT FOUND FOR OPT VAL CXR LO");

     _fcC->collectCommissionTrailerMessages();
     CPPUNIT_ASSERT_EQUAL(size_t(3),_calcTotals->fcMessage.size());
     CPPUNIT_ASSERT_EQUAL(expMsg2, _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(expMsg1, _calcTotals->fcMessage[1].messageText());
     CPPUNIT_ASSERT_EQUAL(expMsg3, _calcTotals->fcMessage[2].messageText());
   }

   void testCollectCommissionTrailerMessages()
   {
     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->processed() = true;
     farePath->itin() = _itin;
     farePath->defaultValidatingCarrier() = "AA";
     farePath->validatingCarriers().push_back("BA");
     farePath->validatingCarriers().push_back("CA");
     farePath->storeCommissionForValidatingCarrier("AA", 123.45);
     farePath->storeCommissionForValidatingCarrier("BA", 223.45);
     farePath->storeCommissionForValidatingCarrier("CA", 323.45);
     farePath->isAgencyCommissionQualifies() = true;

     _trx->setValidatingCxrGsaApplicable(true);
     _calcTotals->equivCurrencyCode = "USD";
     _calcTotals->equivNoDec = 2;
     _calcTotals->farePath = farePath;
     _fcC->collectCommissionTrailerMessages();

     CPPUNIT_ASSERT_EQUAL(size_t(3),_calcTotals->fcMessage.size());
     CPPUNIT_ASSERT_EQUAL(std::string("AGENCY COMMISSION VAL CARRIER AA - USD123.45"),
         _calcTotals->fcMessage[0].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("AGENCY COMMISSION ALT VAL CARRIER/S CA - USD323.45"),
         _calcTotals->fcMessage[1].messageText());
     CPPUNIT_ASSERT_EQUAL(std::string("AGENCY COMMISSION ALT VAL CARRIER/S BA - USD223.45"),
         _calcTotals->fcMessage[2].messageText());
   }

   void testCollectCommissionTrailerMessages_SameCommAmount()
   {
     FarePath* farePath = FarePathBuilder(&_memHandle).build();
     farePath->processed() = true;
     farePath->itin() = _itin;
     farePath->defaultValidatingCarrier() = "AA";
     farePath->validatingCarriers().push_back("BA");
     farePath->validatingCarriers().push_back("CA");
     farePath->storeCommissionForValidatingCarrier("AA", 123.45);
     farePath->storeCommissionForValidatingCarrier("CA", 123.45);
     farePath->storeCommissionForValidatingCarrier("BA", 123.45);
     farePath->isAgencyCommissionQualifies() = true;

     _trx->setValidatingCxrGsaApplicable(true);
     _calcTotals->equivCurrencyCode = "USD";
     _calcTotals->equivNoDec = 2;
     _calcTotals->farePath = farePath;
     _fcC->collectCommissionTrailerMessages();

     CPPUNIT_ASSERT_EQUAL(size_t(2),_calcTotals->fcMessage.size());
     CPPUNIT_ASSERT_EQUAL(std::string("AGENCY COMMISSION VAL CARRIER AA - USD123.45"),
         _calcTotals->fcMessage[0].messageText());

     CPPUNIT_ASSERT_EQUAL(std::string("AGENCY COMMISSION ALT VAL CARRIER/S BA/CA - USD123.45"),
         _calcTotals->fcMessage[1].messageText());
   }

   void testCollect()
   {
     setupCollectValidatingCarrierMessage(EMPTY_STRING(), EMPTY_STRING());

     _fcC->collect();

     CPPUNIT_ASSERT(!_calcTotals->fcMessage.empty());

     CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER - "),
         _calcTotals->fcMessage.front().messageText());
   }

   void testDetermineCat15HasTextTableReturnTrue()
   {
     FareUsage* fu = _memHandle.create<FareUsage>();
     PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
     FarePath* fp = _memHandle.create<FarePath>();
     PricingUnit* pu = _memHandle.create<PricingUnit>();
     Itin* itin = _memHandle.create<Itin>();
     ptf->setCat15HasT996FT(true);
     fu->paxTypeFare() = ptf;
     pu->fareUsage() += fu;
     fp->pricingUnit() += pu;
     fp->itin() = _itin;
     itin->farePath() += fp;

     CPPUNIT_ASSERT(_fcC->determineCat15HasTextTable(*itin));
   }

  void testDetermineCat15HasTextTableReturnFalse()
  {
    FareUsage* fu = _memHandle.create<FareUsage>();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    FarePath* fp = _memHandle.create<FarePath>();
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    Itin* itin = _memHandle.create<Itin>();
    ptf->setCat15HasT996FR(false);
    fu->paxTypeFare() = ptf;
    pu->fareUsage() += fu;
    fp->pricingUnit() += pu;
    fp->itin() = _itin;
    itin->farePath() += fp;

    CPPUNIT_ASSERT(!_fcC->determineCat15HasTextTable(*itin));
  }

  void test_createDefaultTrailerMsg_WithoutValCxr()
  {
    const SettlementPlanType sp("BSP");
    CarrierCode defValCxr("");
    FarePath* fp = FarePathBuilder(&_memHandle).build();
    std::string defaultMsg = _fcC->createDefaultTrailerMsg(*fp, sp, defValCxr);
    CPPUNIT_ASSERT_EQUAL(std::string("BSP - "), defaultMsg);
  }

  void test_createDefaultTrailerMsg_WithValCxr()
  {
    const SettlementPlanType sp("BSP");
    CarrierCode defValCxr("AA");
    FarePath* fp = FarePathBuilder(&_memHandle).build();
    std::string defaultMsg = _fcC->createDefaultTrailerMsg(*fp, sp, defValCxr);
    CPPUNIT_ASSERT_EQUAL(std::string(""), defaultMsg);
  }

  void test_createDefaultTrailerMsg_WithSwappedCxr()
  {
    const SettlementPlanType sp("BSP");
    CarrierCode defValCxr("AA");
    FarePath* fp = FarePathBuilder(&_memHandle).build();
    fp->marketingCxrForDefaultValCxrPerSp()[sp] = "BA";
    fp->defaultValCxrPerSp()[sp]="AA";
    fp->settlementPlanValidatingCxrs()[sp].push_back("AA");
    std::string defaultMsg = _fcC->createDefaultTrailerMsg(*fp, sp, defValCxr);
    CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA PER GSA AGREEMENT WITH BA"), defaultMsg);
  }

  void test_createAltTrailerMsg_EmptyList()
  {
    const SettlementPlanType sp("BSP");
    FarePath* fp = FarePathBuilder(&_memHandle).build();
    std::vector<CarrierCode> mktCxrs;
    std::vector<CarrierCode> altCxrs;
    std::string altTrailerMsg = _fcC->createAltTrailerMsg(*fp, sp, mktCxrs, altCxrs);
    CPPUNIT_ASSERT_EQUAL(std::string(""), altTrailerMsg);
  }

  void test_createAltTrailerMsg_OneAltCxr()
  {
    const SettlementPlanType sp("BSP");
    FarePath* fp = FarePathBuilder(&_memHandle).build();
    std::vector<CarrierCode> mktCxrs;
    std::vector<CarrierCode> altCxrs;
    altCxrs.push_back("AA");
    std::string altTrailerMsg = _fcC->createAltTrailerMsg(*fp, sp, mktCxrs, altCxrs);
    CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA"), altTrailerMsg);
  }

  void test_createAltTrailerMsg_MoreAltCxr_InItinOrder()
  {
    const SettlementPlanType sp("BSP");
    FarePath* fp = FarePathBuilder(&_memHandle).build();
    std::vector<CarrierCode> mktCxrs;
    mktCxrs.push_back("AA"); mktCxrs.push_back("BA");
    std::vector<CarrierCode> altCxrs;
    altCxrs.push_back("AA"); altCxrs.push_back("BA");
    std::string altTrailerMsg = _fcC->createAltTrailerMsg(*fp, sp, mktCxrs, altCxrs);
    CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA BA"), altTrailerMsg);
  }

  void test_createAltTrailerMsg_MoreAltCxr_NotInItinOrder()
  {
    const SettlementPlanType sp("BSP");
    FarePath* fp = FarePathBuilder(&_memHandle).build();
    std::vector<CarrierCode> mktCxrs;
    mktCxrs.push_back("AA"); mktCxrs.push_back("BA");
    std::vector<CarrierCode> altCxrs;
    altCxrs.push_back("BA"); altCxrs.push_back("AA");
    std::string altTrailerMsg = _fcC->createAltTrailerMsg(*fp, sp, mktCxrs, altCxrs);
    CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA BA"), altTrailerMsg);
  }

  void test_createAltTrailerMsg_MoreAltCxr_NotInItinOrderWithSwappedCxr()
  {
    fallback::value::fallbackValidatingCarrierInItinOrderMultiSp.set(true);
    const SettlementPlanType sp("BSP");
    ValidatingCxrGSAData v;
    v.gsaSwapMap()["LA"].insert("LL");
    _itin->validatingCxrGsaData() = &v;

    FarePath* fp = FarePathBuilder(&_memHandle).build();
    fp->itin() = _itin;
    std::vector<CarrierCode> mktCxrs;
    mktCxrs.push_back("AA");
    mktCxrs.push_back("LA");
    mktCxrs.push_back("BA");

    std::vector<CarrierCode> altCxrs;
    altCxrs.push_back("BA");
    altCxrs.push_back("AA");
    altCxrs.push_back("LL");

    std::string altTrailerMsg = _fcC->createAltTrailerMsg(*fp, sp, mktCxrs, altCxrs);
    CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA LL BA"), altTrailerMsg);
  }

  void test_createAltTrailerMsg_MoreAltCxr_NotInItinOrderWithSwappedCxrMultiSp()
   {
     const SettlementPlanType sp1("ARC");
     ValidatingCxrGSAData v1;
     v1.gsaSwapMap()["KA"].insert("CX");

     const SettlementPlanType sp2("SAT");
     ValidatingCxrGSAData v2;

     SpValidatingCxrGSADataMap _spValidatingCxrGsaDataMap;
     _spValidatingCxrGsaDataMap[sp1] = &v1;
     _spValidatingCxrGsaDataMap[sp2] = &v2;

     _itin->spValidatingCxrGsaDataMap() = &_spValidatingCxrGsaDataMap;

     FarePath* fp = FarePathBuilder(&_memHandle).build();
     fp->itin() = _itin;
     std::vector<CarrierCode> mktCxrs;
     mktCxrs.push_back("BR");
     mktCxrs.push_back("KA");
     mktCxrs.push_back("9W");
     mktCxrs.push_back("CX");

     std::vector<CarrierCode> altCxrs;
     altCxrs.push_back("9W");
     altCxrs.push_back("CX");

     std::string altTrailerMsg = _fcC->createAltTrailerMsg(*fp, sp1, mktCxrs, altCxrs);
     CPPUNIT_ASSERT_EQUAL(std::string("ARC - CX 9W"), altTrailerMsg);

     altTrailerMsg.empty();

     altTrailerMsg = _fcC->createAltTrailerMsg(*fp, sp2, mktCxrs, altCxrs);
     CPPUNIT_ASSERT_EQUAL(std::string("SAT - 9W CX"), altTrailerMsg);


   }

  void testIsDefaultVcxrFromPreferred_DefaultVcxrEmpty()
  {
    const CarrierCode defaultVcxr = "";

    _trx->getRequest()->preferredVCs().push_back("AF");
    _trx->getRequest()->preferredVCs().push_back("LH");
    _trx->getRequest()->preferredVCs().push_back("BA");

    CPPUNIT_ASSERT(!_fcC->isDefaultVcxrFromPreferred(defaultVcxr));

  }

  void testIsDefaultVcxrFromPreferred_preferredVcxrEmpty()
  {

    const CarrierCode defaultVcxr = "AA";

    CPPUNIT_ASSERT(!_fcC->isDefaultVcxrFromPreferred(defaultVcxr));

  }

  void testIsDefaultVcxrFromPreferred_Found()
  {
    const CarrierCode defaultVcxr = "LH";

    _trx->getRequest()->preferredVCs().push_back("AF");
    _trx->getRequest()->preferredVCs().push_back("LH");
    _trx->getRequest()->preferredVCs().push_back("BA");

    CPPUNIT_ASSERT(_fcC->isDefaultVcxrFromPreferred(defaultVcxr));

  }
  void testIsDefaultVcxrFromPreferred_NotFound()
  {
    const CarrierCode defaultVcxr = "AA";

    _trx->getRequest()->preferredVCs().push_back("AF");
    _trx->getRequest()->preferredVCs().push_back("LH");
    _trx->getRequest()->preferredVCs().push_back("BA");

    CPPUNIT_ASSERT(!_fcC->isDefaultVcxrFromPreferred(defaultVcxr));

  }
  void testIsAcxrsAreNSPcxrs_Pass()
  {
    std::vector<CarrierCode> v1, v2;
    v1.push_back("AA");
    v1.push_back("AF");
    v1.push_back("BA");
    v1.push_back("UA");
    v1.push_back("LH");

    v2.push_back("AF");
    v2.push_back("LH");

    CPPUNIT_ASSERT(_fcC->isAcxrsAreNSPcxrs(v2, v1));

  }

  void testIsAcxrsAreNSPcxrs_Fail()
  {
    std::vector<CarrierCode> v1, v2;
    v1.push_back("AA");
    v1.push_back("AF");
    v1.push_back("BA");
    v1.push_back("UA");
    v1.push_back("LH");

    v2.push_back("AF");
    v2.push_back("JA");

    CPPUNIT_ASSERT(!_fcC->isAcxrsAreNSPcxrs(v2, v1));

  }

  void testPrepareValCxrMsgForMultiSp_noSMV_noIEV()
  {
    std::vector<std::string> defValCxrMsgCol;
    std::vector<std::string> altValCxrMsgCol;
    std::vector<std::string> optValCxrMsgCol;

    SettlementPlanType BSP("BSP");
    SettlementPlanType NSP("NSP");
    CarrierCode AA("AA");
    CarrierCode BA("BA");
    CarrierCode LH("LH");
    CarrierCode AF("AF");
    CarrierCode DL("DL");

    FarePath* farePath = FarePathBuilder(&_memHandle).build();
    farePath->defaultValCxrPerSp()[BSP] = AA;
    farePath->settlementPlanValidatingCxrs()[BSP].push_back(AA);
    farePath->settlementPlanValidatingCxrs()[BSP].push_back(BA);

    farePath->defaultValCxrPerSp()[NSP] = LH;
    farePath->settlementPlanValidatingCxrs()[NSP].push_back(LH);
    farePath->settlementPlanValidatingCxrs()[NSP].push_back(DL);
    farePath->settlementPlanValidatingCxrs()[NSP].push_back(AF);

    _trx->getRequest()->spvInd() = tse::spValidator::noSMV_noIEV;


    farePath->itin() = _itin;
    _calcTotals->farePath = farePath;

    _fcC->prepareValCxrMsgForMultiSp();
    CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER"), _calcTotals->fcMessage[0].messageText());
    CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA"), _calcTotals->fcMessage[1].messageText());

    CPPUNIT_ASSERT_EQUAL(std::string("ALTERNATE VALIDATING CARRIER/S"), _calcTotals->fcMessage[2].messageText());
    CPPUNIT_ASSERT_EQUAL(std::string("BSP - BA"), _calcTotals->fcMessage[3].messageText());

    CPPUNIT_ASSERT_EQUAL(std::string("NO SETTLEMENT PLAN/NO IET VALIDATION"), _calcTotals->fcMessage[4].messageText());
  }

  void testPrepareValCxrMsgForMultiSp_noSMV_IEV()
    {
      std::vector<std::string> defValCxrMsgCol;
      std::vector<std::string> altValCxrMsgCol;
      std::vector<std::string> optValCxrMsgCol;

      SettlementPlanType BSP("BSP");
      SettlementPlanType NSP("NSP");
      CarrierCode AA("AA");
      CarrierCode BA("BA");
      CarrierCode LH("LH");
      CarrierCode AF("AF");
      CarrierCode DL("DL");

      FarePath* farePath = FarePathBuilder(&_memHandle).build();
      farePath->defaultValCxrPerSp()[BSP] = AA;
      farePath->settlementPlanValidatingCxrs()[BSP].push_back(AA);
      farePath->settlementPlanValidatingCxrs()[BSP].push_back(BA);

      farePath->defaultValCxrPerSp()[NSP] = LH;
      farePath->settlementPlanValidatingCxrs()[NSP].push_back(LH);
      farePath->settlementPlanValidatingCxrs()[NSP].push_back(DL);
      farePath->settlementPlanValidatingCxrs()[NSP].push_back(AF);

      SpValidatingCxrGSADataMap spGsaDataMap;
      ValidatingCxrGSAData vcxrGSAData;


      vcxrGSAData.validatingCarriersData()[LH].interlineValidCountries.push_back("GB");
      vcxrGSAData.validatingCarriersData()[LH].interlineValidCountries.push_back("FR");

      vcxrGSAData.validatingCarriersData()[DL].interlineValidCountries.push_back("US");
      vcxrGSAData.validatingCarriersData()[DL].interlineValidCountries.push_back("CA");

      vcxrGSAData.validatingCarriersData()[AF].interlineValidCountries.push_back("GB");
      vcxrGSAData.validatingCarriersData()[AF].interlineValidCountries.push_back("FR");

      spGsaDataMap[NSP] = &vcxrGSAData;

      _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

     _trx->getRequest()->spvInd() = tse::spValidator::noSMV_IEV;

     farePath->itin() = _itin;
      _calcTotals->farePath = farePath;

      _fcC->prepareValCxrMsgForMultiSp();
      CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER"), _calcTotals->fcMessage[0].messageText());
      CPPUNIT_ASSERT_EQUAL(std::string("BSP - AA"), _calcTotals->fcMessage[1].messageText());

      CPPUNIT_ASSERT_EQUAL(std::string("ALTERNATE VALIDATING CARRIER/S"), _calcTotals->fcMessage[2].messageText());
      CPPUNIT_ASSERT_EQUAL(std::string("BSP - BA"), _calcTotals->fcMessage[3].messageText());

      CPPUNIT_ASSERT_EQUAL(std::string("NO SETTLEMENT PLAN WITH IET"), _calcTotals->fcMessage[4].messageText());
      CPPUNIT_ASSERT_EQUAL(std::string("IET VALIDATION CA - DL"), _calcTotals->fcMessage[5].messageText());
      CPPUNIT_ASSERT_EQUAL(std::string("IET VALIDATION FR - AF LH"), _calcTotals->fcMessage[6].messageText());
      CPPUNIT_ASSERT_EQUAL(std::string("IET VALIDATION GB - AF LH"), _calcTotals->fcMessage[7].messageText());
      CPPUNIT_ASSERT_EQUAL(std::string("IET VALIDATION US - DL"), _calcTotals->fcMessage[8].messageText());
    }

  void testCollectValidatingCarrierMessageForGSA_noSMV_noIEV()
  {
    _trx->setValidatingCxrGsaApplicable( true );
    const std::string expectedMsg = "VALIDATING CARRIER - 2R";
    const std::string expectedNSPMsg = "NO SETTLEMENT PLAN/NO IET VALIDATION";
    setupCollectValidatingCarrierMessage(EMPTY_STRING(), CARRIER_2R);
    setupValidatingCxrTicketingDate();

    SettlementPlanType NSP("NSP");
    CarrierCode LH("LH");
    CarrierCode AF("AF");
    CarrierCode DL("DL");

    _farePath->defaultValCxrPerSp()[NSP] = LH;
    _farePath->settlementPlanValidatingCxrs()[NSP].push_back(LH);
    _farePath->settlementPlanValidatingCxrs()[NSP].push_back(DL);
    _farePath->settlementPlanValidatingCxrs()[NSP].push_back(AF);

    _trx->getRequest()->spvInd() = tse::spValidator::noSMV_noIEV;
    _trx->getRequest()->spvCxrsCode().push_back("LH");
    _trx->getRequest()->spvCxrsCode().push_back("AL");
    _trx->getRequest()->spvCxrsCode().push_back("DL");


    _fcC->collectValidatingCarrierMessageForGSA();

    CPPUNIT_ASSERT( !_calcTotals->fcMessage.empty() );
    CPPUNIT_ASSERT_EQUAL(expectedMsg, _calcTotals->fcMessage[0].messageText());
    CPPUNIT_ASSERT_EQUAL(expectedNSPMsg, _calcTotals->fcMessage[1].messageText());

  }

  void testCollectValidatingCarrierMessageForGSA_noSMV_IEV()
  {
    _trx->setValidatingCxrGsaApplicable( true );
    const std::string expectedMsg = "VALIDATING CARRIER - 2R";
    const std::string expectedNSPMsg = "NO SETTLEMENT PLAN WITH IET";
    setupCollectValidatingCarrierMessage(EMPTY_STRING(), CARRIER_2R);
    setupValidatingCxrTicketingDate();

    SettlementPlanType NSP("NSP");
    CarrierCode LH("LH");
    CarrierCode AF("AF");
    CarrierCode DL("DL");

    _farePath->defaultValCxrPerSp()[NSP] = LH;
    _farePath->settlementPlanValidatingCxrs()[NSP].push_back(LH);
    _farePath->settlementPlanValidatingCxrs()[NSP].push_back(DL);
    _farePath->settlementPlanValidatingCxrs()[NSP].push_back(AF);

    SpValidatingCxrGSADataMap spGsaDataMap;
    ValidatingCxrGSAData vcxrGSAData;


    vcxrGSAData.validatingCarriersData()[LH].interlineValidCountries.push_back("GB");
    vcxrGSAData.validatingCarriersData()[LH].interlineValidCountries.push_back("FR");

    vcxrGSAData.validatingCarriersData()[DL].interlineValidCountries.push_back("US");
    vcxrGSAData.validatingCarriersData()[DL].interlineValidCountries.push_back("CA");

    vcxrGSAData.validatingCarriersData()[AF].interlineValidCountries.push_back("GB");
    vcxrGSAData.validatingCarriersData()[AF].interlineValidCountries.push_back("FR");

    spGsaDataMap[NSP] = &vcxrGSAData;

    _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

    _trx->getRequest()->spvInd() = tse::spValidator::noSMV_IEV;
    _trx->getRequest()->spvCxrsCode().push_back("LH");
    _trx->getRequest()->spvCxrsCode().push_back("AL");
    _trx->getRequest()->spvCxrsCode().push_back("DL");


    _fcC->collectValidatingCarrierMessageForGSA();

    CPPUNIT_ASSERT( !_calcTotals->fcMessage.empty() );
    CPPUNIT_ASSERT_EQUAL(expectedMsg, _calcTotals->fcMessage[0].messageText());
    CPPUNIT_ASSERT_EQUAL(expectedNSPMsg, _calcTotals->fcMessage[1].messageText());
    CPPUNIT_ASSERT_EQUAL(std::string("IET VALIDATION CA - DL"), _calcTotals->fcMessage[2].messageText());
    CPPUNIT_ASSERT_EQUAL(std::string("IET VALIDATION FR - AF LH"), _calcTotals->fcMessage[3].messageText());
    CPPUNIT_ASSERT_EQUAL(std::string("IET VALIDATION GB - AF LH"), _calcTotals->fcMessage[4].messageText());
    CPPUNIT_ASSERT_EQUAL(std::string("IET VALIDATION US - DL"), _calcTotals->fcMessage[5].messageText());

  }

protected:
  TestMemHandle _memHandle;

  PricingTrx* _trx;
  PricingRequest* _request;
  PricingOptions *_option;
  FarePath* _farePath;
  Itin* _itin;
  PaxType* _paxType;
  FcCollector* _fcCTest;
  FcCollector* _fcC;
  Customer *_customer;
  Agent *_agent;
  FareCalcConfig* _fcConfig;
  FareCalcCollector* _fcalcCollector;
  CalcTotals* _calcTotals;

  class FcCollectorDataHandleMock : public DataHandleMock
  {
  public:
    NUCInfo*
    getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date)
    {
      static NUCInfo nucInfo;
      return &nucInfo;
    }
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(FcCollectorTest);
}
} // tse
