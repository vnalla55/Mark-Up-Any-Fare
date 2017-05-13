#include <boost/assign/std/vector.hpp>

#include "Common/BSRCurrencyConverter.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Currency.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/PaxTypeMatrix.h"
#include "DBAccess/SeatCabinCharacteristicInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "DBAccess/TaxNation.h"
#include "FareCalc/CalcTotals.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestLocFactory.h"

namespace tse
{
using boost::assign::operator+=;

namespace
{
class ServiceFeeUtilMock : public ServiceFeeUtil
{
public:
  ServiceFeeUtilMock(PricingTrx& trx) : ServiceFeeUtil(trx)
  {
    pax1.sabrePaxType() = "TC1";
    pax1.atpPaxType() = JCB;

    pax2.sabrePaxType() = "TC1";
    pax2.atpPaxType() = "SPA";

    paxV.push_back(&pax1);
    paxV.push_back(&pax2);

    jp.primeCur() = "JPY";
    jp.description() = "JAPAN";
    allNation.push_back(&jp);
    jpy.controllingEntityDesc() = "JAPAN";
    zwr.taxOverrideRoundingUnit() = 6;
  }

  bool checkMultiTransport(const LocCode& locCode1,
                           const LocCode& locCode2,
                           const CarrierCode& carrierCode,
                           GeoTravelType tvlType) const
  {
    return locCode1 == locCode2;
  }

  bool
  isStopOver(const TravelSeg* travelSeg, const TravelSeg* travelSegTo, const FareUsage* fareUsage)
      const
  {
    return true;
  }

  uint32_t getTPM(const Loc& market1,
                  const Loc& market2,
                  const std::vector<TravelSeg*>& travelSegs,
                  const FarePath& farePath) const
  {
    return abs(market1.loc()[0] - market2.loc()[0]);
  }

  const std::vector<const PaxTypeMatrix*>& getSabrePaxTypes(const PaxTypeCode& farePathPtc) const
  {
    return paxV;
  }

  const Currency* getCurrency_old(const CurrencyCode& currencyCode) const
  {
    if (currencyCode == "JPY")
      return &jpy;
    if (currencyCode == "ZWR")
      return &jpy;
    return NULL;
  }

  const TaxNation* getTaxNation_old(const NationCode& nationCode) const { return &us; }

  bool convertCurrency(Money& target,
                       const Money& source,
                       CurrencyConversionRequest::ApplicationType /*applType*/) const
  {
    if (target.code()[0] == source.code()[0])
      target.value() = source.value();
    else if (target.code()[0] > source.code()[0])
      target.value() = source.value() / 2;
    else
      target.value() = source.value() * 2;
    if (source.code() == "PLN")
      return false;
    return true;
  }

  // data members
  PaxTypeMatrix pax1, pax2;
  std::vector<const PaxTypeMatrix*> paxV;
  Currency usd;
  Currency jpy, zwr;
  TaxNation us;

  Nation jp;
  std::vector<Nation*> allNation;
};

struct CheckSeatCabinCharacteristic : std::unary_function<SeatCabinCharacteristicInfo*, bool>
{
  CheckSeatCabinCharacteristic(const CarrierCode& carrier, const Indicator& codeType)
    : _carrier(carrier), _codeType(codeType)
  {
  }

  bool operator()(const SeatCabinCharacteristicInfo* seatCabinCharacteristicInfo) const
  {
    return (seatCabinCharacteristicInfo->carrier() == _carrier) &&
           (seatCabinCharacteristicInfo->codeType() == _codeType);
  }

private:
  const CarrierCode& _carrier;
  const Indicator& _codeType;
};
}

class ServiceFeeUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ServiceFeeUtilTest);

  CPPUNIT_TEST(testIsInternationalJourneyType_Pass);
  CPPUNIT_TEST(testIsInternationalJourneyType_Fail);
  CPPUNIT_TEST(testIsInternationalJourneyType_Fail_NULL);
  CPPUNIT_TEST(testIsInternationalJourneyType_Fail_Arunk);
  CPPUNIT_TEST(testIsInternationalJourneyType_Fail_Surface);

  CPPUNIT_TEST(testIsRoundTripJourneyType_Pass_International);
  CPPUNIT_TEST(testIsRoundTripJourneyType_Pass_NotInternational);
  CPPUNIT_TEST(testIsRoundTripJourneyType_Fail_International);
  CPPUNIT_TEST(testIsRoundTripJourneyType_Fail_NotInternational);

  CPPUNIT_TEST(testGetJourneyDestination_OW_Intl);
  CPPUNIT_TEST(testGetJourneyDestination_OW_Dom);
  CPPUNIT_TEST(testGetJourneyDestination_RT_Intl);
  CPPUNIT_TEST(testGetJourneyDestination_RT_IntlButFartherDom);
  CPPUNIT_TEST(testGetJourneyDestination_RT_Dom);

  CPPUNIT_TEST(testCheckTicketingDates_Pass);
  CPPUNIT_TEST(testCheckTicketingDates_Fail_Eff);
  CPPUNIT_TEST(testCheckTicketingDates_Fail_Disc);
  CPPUNIT_TEST(testCheckTicketingDates_Pass_Inf);

  CPPUNIT_TEST(testMatchPaxType_Pass_SabrePtcMatchFeePtc);
  CPPUNIT_TEST(testMatchPaxType_Fail_SabrePtcNoMatchFeePtc);
  CPPUNIT_TEST(testMatchPaxType_Pass_ActualPtc);
  CPPUNIT_TEST(testMatchPaxType_Pass_FeePtcEmpty);
  CPPUNIT_TEST(testMatchPaxType_Fail_InputPtcNoMatchFeePtc);
  CPPUNIT_TEST(testMatchPaxType_Pass_InputPtcExactMatchFeePtc);
  CPPUNIT_TEST(testMatchPaxType_Fail_InputPtcNotMapToFeePtcYTH);
  CPPUNIT_TEST(testMatchPaxType_Fail_InputPtcNotMapToFeePtcINS1);

  CPPUNIT_TEST(testGetSortedFees_OneSeg);
  CPPUNIT_TEST(testGetSortedFees_ReversedOrder);
  CPPUNIT_TEST(testGetSortedFees_MixedOrder);
  CPPUNIT_TEST(testGetSortedFees_SortedBefore);
  CPPUNIT_TEST(testGetSortedFees_SubCode);
  CPPUNIT_TEST(testGetSortedFees_MultiPax);
  CPPUNIT_TEST(testGetSortedFees_Grouping);
  CPPUNIT_TEST(testGetSortedFees_NoGroupingAmount);
  CPPUNIT_TEST(testCreateOcFeesUsages_givenAncillaryPricingTrx_whenNotHandlingSecondCallForMonetaryDiscountTrx_shouldCreateUsagesForAllOcFees);
  CPPUNIT_TEST(testCreateOcFeesUsages_givenAncillaryPricingTrx_whenHandlingSecondCallForMonetaryDiscountTrx_shouldCreateUsagesOnlyForOcFeesWithPriceModification);
  CPPUNIT_TEST(testOcFeesUsagesMerger_SinglePax);
  CPPUNIT_TEST(testOcFeesUsagesMerger_MultiPax);
  CPPUNIT_TEST(testOcFeesUsagesMerger_MultiPaxOcLimit);

  CPPUNIT_TEST(testGetFeeRounding_Pass_CallControllingNationCode);
  CPPUNIT_TEST(testGetFeeRounding_Pass_DoNotCallControllingNationCode);
  CPPUNIT_TEST(testGetFeeRounding_Fail_CurrencyNotFound);

  CPPUNIT_TEST(testConvertOCFeesCurrency_WithCurrencyCode);
  CPPUNIT_TEST(testConvertOCFeesCurrency_WithCalcTotals);
  CPPUNIT_TEST(testConvertOCFeesCurrency_WithCurrencyOverride);
  CPPUNIT_TEST(testConvertOCFeesCurrency_ConversionFailed);

  CPPUNIT_TEST(testConvertServiceFeesGroupCurrency);
  CPPUNIT_TEST(testGetGroupConfigurationForCode);
  CPPUNIT_TEST(testRequestedOcFeeGroupData);

  CPPUNIT_TEST(testGetOCFeesSummary_Fail_Cfg);
  CPPUNIT_TEST(testGetOCFeesSummary_Fail_Requested);
  CPPUNIT_TEST(testGetOCFeesSummary_Fail_FeesMap);
  CPPUNIT_TEST(testGetOCFeesSummary_Fail_PaxType);
  CPPUNIT_TEST(testGetOCFeesSummary_Pass_BlankCommandName);
  CPPUNIT_TEST(testGetOCFeesSummary_Pass);

  CPPUNIT_TEST(testCollectSegmentStatus);

  CPPUNIT_TEST(testSetFareIndicator_19);
  CPPUNIT_TEST(testSetFareIndicator_20);
  CPPUNIT_TEST(testSetFareIndicator_21);
  CPPUNIT_TEST(testSetFareIndicator_22);
  CPPUNIT_TEST(testSetFareIndicator_25);
  CPPUNIT_TEST(testSetFareIndicator_35);

  CPPUNIT_TEST(testGetSortedFees_PaxWitkTktInfo_T1T2);
  CPPUNIT_TEST(testGetSortedFees_PaxWitkTktInfo_T2T1);
  CPPUNIT_TEST(testGetSortedFees_PaxWitkTktInfo_T1Blank);

  CPPUNIT_TEST(testCheckServiceGroupForAcs_Pass_BG);
  CPPUNIT_TEST(testCheckServiceGroupForAcs_Pass_PT);
  CPPUNIT_TEST(testCheckServiceGroupForAcs_Fail);

  CPPUNIT_TEST(testFill_OneCode);
  CPPUNIT_TEST(testFill_FiveCodes);
  CPPUNIT_TEST(testFill_NoCodes);
  CPPUNIT_TEST(testGetTranslatedPadisDescription);
  CPPUNIT_TEST(testGetTranslatedPadisDescription_abbreviated);
  CPPUNIT_TEST(testAddSeatCabinCharacteristic_Carrier_LH);
  CPPUNIT_TEST(testAddSeatCabinCharacteristic_Carrier_1P);
  CPPUNIT_TEST(testAddSeatCabinCharacteristic_Carrier_1Z_FULL_DESCRIPTION);
  CPPUNIT_TEST(testAddSeatCabinCharacteristic_Carrier_1Z_ABBREVIATIONS);
  CPPUNIT_TEST(testAddSeatCabinCharacteristic_Carrier_AU_ATPCO_ABBREVIATIONS);
  CPPUNIT_TEST(testFillOutOCFeeUsageByPadisDescription_OCFeeUsage_Present_Carrier_LH);
  CPPUNIT_TEST(testCreateOCFeesUsageForSingleS7_NO_Padis_Present_Carrier_LH);
  CPPUNIT_TEST(testCreateOCFeesUsageForSingleS7_Padis_Present_Carrier_LH);
  CPPUNIT_TEST(testCreateOCFeesUsageForSingleS7_IgnorePadis);
  CPPUNIT_TEST(testCreateOCFeesUsageForSingleS7_MultiSequence);
  CPPUNIT_TEST(testCreateOCFeesUsageForSingleS7_IgnorePadis_MultiSequence);

  CPPUNIT_TEST(testIsServiceGroupInvalidForAcs_True);
  CPPUNIT_TEST(testIsServiceGroupInvalidForAcs_False);

  CPPUNIT_TEST(testIsFeeFarePercentage_True_C);
  CPPUNIT_TEST(testIsFeeFarePercentage_True_P);
  CPPUNIT_TEST(testIsFeeFarePercentage_True_H);
  CPPUNIT_TEST(testIsFeeFarePercentage_False);

  CPPUNIT_TEST(testConvertFeeCurrencyStrategy_convertTaxItem);

  CPPUNIT_TEST(testCheckTicketingDates_TimeFailed);
  CPPUNIT_TEST(testCheckTicketingDates_TimeOk);

  CPPUNIT_TEST(test_isRequestFromAS_false);
  CPPUNIT_TEST(test_isRequestFromAS_true);

  CPPUNIT_TEST(test_isPerformEMDCheck_EmdForFlightRelatedServiceAndPrepaidBaggage);
  CPPUNIT_TEST(test_isPerformEMDCheck_NotEmdForFlightRelatedServiceAndPrepaidBaggage);
  CPPUNIT_TEST(test_isPerformEMDCheck_EmdForFlightRelatedServiceAndPrepaidBaggageGroup99);

  CPPUNIT_TEST_SUITE_END();

public:
  // other methods
  void setUp()
  {
    _memHandle.create<MockTseServer>();
    _dataHandleMock = _memHandle.create<ServiceFeeUtilDataHandleMock>();

    _itin = NULL;
    _farePath = NULL;
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _util = _memHandle.insert(new ServiceFeeUtilMock(*_trx));

    _paxTypeADT = _memHandle.create<PaxType>();
    _paxTypeCNN = _memHandle.create<PaxType>();
    _paxTypeTVL = _memHandle.create<PaxType>();
    _paxTypeADT->paxType() = ADULT;
    _paxTypeCNN->paxType() = CHILD;
    _paxTypeTVL->paxType() = "TVL";

    _locSFO = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    _locDFW = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _locNYC = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    _locLON = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");

    _subCodeA = _memHandle.create<SubCodeInfo>();
    _subCodeA->serviceSubTypeCode() = "A";
    _subCodeB = _memHandle.create<SubCodeInfo>();
    _subCodeB->serviceSubTypeCode() = "B";
    _subCodeC = _memHandle.create<SubCodeInfo>();
    _subCodeC->serviceSubTypeCode() = "C";
    _sfg = _memHandle.create<ServiceFeesGroup>();

    _rF = 0;
    _rND = 0;
    _rR = NONE;

    const std::string activationDate = "2000-01-01";
    TestConfigInitializer::setValue("EMD_VALIDATION_FLIGHT_RELATED_SERVICE_AND_PREPAID_BAGGAGE",
                                    activationDate,
                                    "EMD_ACTIVATION");
  }

  void tearDown()
  {
    _paxTypes.clear();
    _memHandle.clear();
  }

  void createTwoStopItin(const Loc* stop1, const Loc* stop2, const Loc* stop3)
  {
    AirSeg* air1 = createSegment();
    air1->pnrSegment() = 1;
    air1->segmentOrder() = 1;
    air1->origin() = stop1;
    air1->origAirport() = stop1->loc();
    air1->destination() = stop2;
    air1->destAirport() = stop2->loc();

    AirSeg* air2 = createSegment();
    air2->pnrSegment() = 2;
    air2->segmentOrder() = 2;
    air2->origin() = stop2;
    air2->origAirport() = stop2->loc();
    air2->destination() = stop3;
    air2->destAirport() = stop3->loc();

    _trx->travelSeg().push_back(air1);
    _trx->travelSeg().push_back(air2);

    TrxUtil::buildFareMarket(*_trx, _trx->travelSeg());

    _pricingUnit = _memHandle.create<PricingUnit>();
    _farePath->pricingUnit().push_back(_pricingUnit);

    _fareUsage = _memHandle.create<FareUsage>();
    _pricingUnit->fareUsage().push_back(_fareUsage);

    _fareUsage->travelSeg().push_back(air1);
    _fareUsage->travelSeg().push_back(air2);
  }

  AirSeg* createSegment(const Loc* origin = NULL, const Loc* destination = NULL, PaxType* paxType = nullptr)
  {
    if (paxType)
    {
      _farePath = _memHandle.create<FarePath>();
      _farePath->paxType() = paxType;
    }
    else
    {
      if (!_farePath)
      {
        _farePath = _memHandle.create<FarePath>();
        _farePath->paxType() = _paxTypeADT;
      }
    }
    if (!_itin)
      _itin = _memHandle.create<Itin>();
    _farePath->itin() = _itin;

    AirSeg* air = _memHandle.create<AirSeg>();
    air->origin() = origin;
    air->destination() = destination;

    _itin->travelSeg().push_back(air);

    return air;
  }

  Agent* createAgent()
  {
    Agent* agent = _memHandle.create<Agent>();
    agent->agentLocation() = _locDFW;
    agent->agentCity() = "DFW";
    agent->tvlAgencyPCC() = "HDQ";
    agent->mainTvlAgencyPCC() = "HDQ";
    agent->tvlAgencyIATA() = "XYZ";
    agent->homeAgencyIATA() = "XYZ";
    agent->agentFunctions() = "XYZ";
    agent->agentDuty() = "XYZ";
    agent->airlineDept() = "XYZ";
    agent->cxrCode() = "AA";
    agent->currencyCodeAgent() = "USD";
    agent->coHostID() = 9;
    agent->agentCommissionType() = "PERCENT";
    agent->agentCommissionAmount() = 10;

    return agent;
  }

  OCFees* addOcFeeToGroup(AirSeg* seg1,
                          AirSeg* seg2,
                          SubCodeInfo* subCodeInfo = NULL,
                          FarePath* farePath = NULL)
  {
    OCFees* fee = _memHandle.create<OCFees>();
    fee->subCodeInfo() = subCodeInfo ? subCodeInfo : _subCodeA;
    fee->travelStart() = seg1;
    fee->travelEnd() = seg2;

    _sfg->ocFeesMap()[farePath ? farePath : _farePath].push_back(fee);

    return fee;
  }

  OCFees* createOcFee(const CurrencyCode& code, MoneyAmount amount)
  {
    OCFees* fee = _memHandle.create<OCFees>();

    fee->feeCurrency() = code;
    fee->feeAmount() = amount;

    return fee;
  }

  void prepareServiceFeesGroup(PaxType* paxType = nullptr, PricingTrx* trx = nullptr)
  {
    if (!trx)
      trx = _trx;

    trx->getOptions()->currencyOverride() = "USD";

    if (!_farePath)
    {
      AirSeg* air1 = createSegment();
      air1->pnrSegment() = 1;
      air1->segmentOrder() = 1;
      air1->origin() = _locDFW;
      air1->origAirport() = _locDFW->loc();
      air1->destination() = _locNYC;
      air1->destAirport() = _locNYC->loc();

      _farePath->paxType() = paxType ? paxType : _paxTypeADT;
    }

    OCFees* ocFees = createOcFee("USD", 100);
    ocFees->travelStart() = *(_itin->travelSeg().begin());
    ocFees->travelEnd() = *(_itin->travelSeg().begin());
    ocFees->travelStart()->pnrSegment() = 1;
    ocFees->travelEnd()->pnrSegment() = 3;
    ocFees->carrierCode() = CarrierCode("1S");

    SubCodeInfo* subCode = _memHandle.create<SubCodeInfo>();
    subCode->serviceSubTypeCode() = "SUB";
    subCode->fltTktMerchInd() = 'A';
    OptionalServicesInfo* optionalServicesInfo = _memHandle.create<OptionalServicesInfo>();
    optionalServicesInfo->seqNo() = 2000;
    optionalServicesInfo->upgrdServiceFeesResBkgDesigTblItemNo() = 3000;
    ocFees->subCodeInfo() = subCode;
    ocFees->optFee() = optionalServicesInfo;

    OCFees::OCFeesSeg* newSeg = trx->dataHandle().create<OCFees::OCFeesSeg>();
    *newSeg = *ocFees->getCurrentSeg();
    newSeg->_ancPriceModification = std::make_pair(AncillaryIdentifier(*ocFees), AncillaryPriceModifier());
    ocFees->segments().push_back(newSeg);

    _sfg->groupCode() = "ML";
    _sfg->ocFeesMap()[_farePath].push_back(ocFees);
    _itin->ocFeesGroup().push_back(_sfg);

    OcFeeGroupConfig config;
    config.groupCode() = _sfg->groupCode();
    config.commandName() = "X";
    config.subTypeCodes().push_back(subCode->serviceSubTypeCode());

    trx->getOptions()->groupsSummaryConfigVec().push_back(config);

    RequestedOcFeeGroup reqConfig;
    reqConfig.groupCode() = _sfg->groupCode();
    reqConfig.numberOfItems() = 2;
    trx->getOptions()->serviceGroupsVec().push_back(reqConfig);
  }

  void prepareASRequest()
  {
    _trx->getRequest()->ticketingAgent() = createAgent();
    _trx->getRequest()->ticketingAgent()->agentTJR() = 0;
    Billing* billing = _memHandle.create<Billing>();
    _trx->billing() = billing;
    _trx->billing()->partitionID() = "NW";
    _trx->billing()->aaaCity() = "ORD";
  }

  Ts2ss&
  setupSegmentsAndSegmentStatuses()
  {
    _farePath = _memHandle.create<FarePath>();
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();
    _farePath->pricingUnit().push_back(pu1);
    _farePath->pricingUnit().push_back(pu2);
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    pu1->fareUsage().push_back(fu1);
    pu2->fareUsage().push_back(fu2);
    AirSeg* as1 = _memHandle.create<AirSeg>();
    AirSeg* as2 = _memHandle.create<AirSeg>();
    fu1->travelSeg().push_back(as1);
    fu2->travelSeg().push_back(as2);
    PaxTypeFare::SegmentStatus ss1;
    PaxTypeFare::SegmentStatus ss2;
    fu1->segmentStatus().push_back(ss1);
    fu2->segmentStatus().push_back(ss2);
    PaxTypeFare* ptf1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* ptf2 = _memHandle.create<PaxTypeFare>();
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    Ts2ss* ts2ss = _memHandle.create<Ts2ss>();
    ts2ss->insert(std::make_pair(as1, std::make_pair(&fu1->segmentStatus()[0], ptf1)));
    ts2ss->insert(std::make_pair(as2, std::make_pair(&fu2->segmentStatus()[0], ptf2)));

    return *ts2ss;
  }

  // TESTS

  void testIsInternationalJourneyType_Pass()
  {
    createTwoStopItin(_locDFW, _locNYC, _locLON);
    CPPUNIT_ASSERT(ServiceFeeUtil::isInternationalJourneyType(*_itin));
  }

  void testIsInternationalJourneyType_Fail()
  {
    createTwoStopItin(_locSFO, _locDFW, _locNYC);
    CPPUNIT_ASSERT(!ServiceFeeUtil::isInternationalJourneyType(*_itin));
  }

  void testIsInternationalJourneyType_Fail_NULL()
  {
    createTwoStopItin(_locSFO, _locDFW, _locNYC);
    _itin->travelSeg().push_back(NULL);

    CPPUNIT_ASSERT(!ServiceFeeUtil::isInternationalJourneyType(*_itin));
  }

  void testIsInternationalJourneyType_Fail_Arunk()
  {
    createTwoStopItin(_locDFW, _locNYC, _locLON);
    _itin->travelSeg()[1]->segmentType() = Arunk;

    CPPUNIT_ASSERT(!ServiceFeeUtil::isInternationalJourneyType(*_itin));
  }

  void testIsInternationalJourneyType_Fail_Surface()
  {
    createTwoStopItin(_locDFW, _locNYC, _locLON);
    _itin->travelSeg()[1]->segmentType() = Surface;

    CPPUNIT_ASSERT(!ServiceFeeUtil::isInternationalJourneyType(*_itin));
  }

  void testIsRoundTripJourneyType_Pass_International()
  {
    createTwoStopItin(_locDFW, _locLON, _locNYC);
    CPPUNIT_ASSERT(_util->isRoundTripJourneyType(*_itin, _carrierKL, true));
  }

  void testIsRoundTripJourneyType_Pass_NotInternational()
  {
    createTwoStopItin(_locDFW, _locSFO, _locDFW);
    CPPUNIT_ASSERT(_util->isRoundTripJourneyType(*_itin, _carrierKL, false));
  }

  void testIsRoundTripJourneyType_Fail_International()
  {
    createTwoStopItin(_locDFW, _locNYC, _locLON);
    CPPUNIT_ASSERT(!_util->isRoundTripJourneyType(*_itin, _carrierKL, true));
  }

  void testIsRoundTripJourneyType_Fail_NotInternational()
  {
    createTwoStopItin(_locDFW, _locSFO, _locNYC);
    CPPUNIT_ASSERT(!_util->isRoundTripJourneyType(*_itin, _carrierKL, false));
  }

  void testGetJourneyDestination_OW_Intl()
  {
    createTwoStopItin(_locDFW, _locSFO, _locLON);
    const Loc* dest = NULL;
    _util->getJourneyDestination(*_farePath, dest, false, true);

    CPPUNIT_ASSERT_EQUAL(_locLON->loc(), dest->loc());
  }

  void testGetJourneyDestination_OW_Dom()
  {
    createTwoStopItin(_locDFW, _locSFO, _locNYC);
    const Loc* dest = NULL;
    _util->getJourneyDestination(*_farePath, dest, false, false);

    CPPUNIT_ASSERT_EQUAL(_locNYC->loc(), dest->loc());
  }

  void testGetJourneyDestination_RT_Intl()
  {
    createTwoStopItin(_locDFW, _locLON, _locDFW);
    const Loc* dest = NULL;
    _util->getJourneyDestination(*_farePath, dest, true, true);

    CPPUNIT_ASSERT_EQUAL(_locLON->loc(), dest->loc());
  }

  void testGetJourneyDestination_RT_IntlButFartherDom()
  {
    createTwoStopItin(_locNYC, _locLON, _locDFW);
    const Loc* dest = NULL;
    _util->getJourneyDestination(*_farePath, dest, true, true);

    CPPUNIT_ASSERT_EQUAL(_locLON->loc(), dest->loc());
  }

  void testGetJourneyDestination_RT_Dom()
  {
    createTwoStopItin(_locDFW, _locNYC, _locDFW);
    const Loc* dest = NULL;
    _util->getJourneyDestination(*_farePath, dest, true, false);

    CPPUNIT_ASSERT_EQUAL(_locNYC->loc(), dest->loc());
  }

  void testCheckTicketingDates_Pass()
  {
    CPPUNIT_ASSERT(ServiceFeeUtil::checkIsDateBetween(
        DateTime(2010, 1, 1), DateTime(2010, 1, 1), DateTime(2010, 1, 1)));
  }

  void testCheckTicketingDates_Fail_Eff()
  {
    CPPUNIT_ASSERT(!ServiceFeeUtil::checkIsDateBetween(
                       DateTime(2010, 1, 2), DateTime(2011, 1, 1), DateTime(2010, 1, 1)));
  }

  void testCheckTicketingDates_Fail_Disc()
  {
    CPPUNIT_ASSERT(!ServiceFeeUtil::checkIsDateBetween(
                       DateTime(2009, 1, 1), DateTime(2009, 12, 31), DateTime(2010, 1, 1)));
  }

  void testCheckTicketingDates_Pass_Inf()
  {
    CPPUNIT_ASSERT(ServiceFeeUtil::checkIsDateBetween(DateTime(boost::date_time::pos_infin),
                                                      DateTime(boost::date_time::pos_infin),
                                                      DateTime(2010, 1, 1)));
  }

  void testCheckTicketingDates_TimeOk()
  {
    DateTime start(2009, 1, 1, 11, 28, 00);
    DateTime end(2009, 12, 31);
    DateTime between(2009, 1, 1, 11, 29, 00);

    CPPUNIT_ASSERT(ServiceFeeUtil::checkIsDateBetween(start, end, between));
  }

  void testCheckTicketingDates_TimeFailed()
  {
    DateTime start(2009, 1, 1, 11, 28, 00);
    DateTime end(2009, 12, 31);
    DateTime between(2009, 1, 1, 11, 27, 00);

    CPPUNIT_ASSERT(!ServiceFeeUtil::checkIsDateBetween(start, end, between));
  }

  void testMatchPaxType_Pass_SabrePtcMatchFeePtc()
  {
    CPPUNIT_ASSERT(_util->matchPaxType(ANY_CARRIER, *_paxTypeADT, JCB));
  }

  void testMatchPaxType_Fail_SabrePtcNoMatchFeePtc()
  {
    CPPUNIT_ASSERT(!_util->matchPaxType(ANY_CARRIER, *_paxTypeADT, SRC));
  }

  void testMatchPaxType_Pass_ActualPtc()
  {
    _paxTypes.push_back(_paxTypeTVL);
    _paxTypeTVL->paxType() = SRC;
    _paxTypeADT->actualPaxType()["**"] = &_paxTypes;
    CPPUNIT_ASSERT(_util->matchPaxType(ANY_CARRIER, *_paxTypeADT, SRC));
  }

  void testMatchPaxType_Pass_FeePtcEmpty()
  {
    CPPUNIT_ASSERT(_util->matchPaxType(ANY_CARRIER, *_paxTypeCNN, ""));
  }

  void testMatchPaxType_Fail_InputPtcNoMatchFeePtc()
  {
    _paxTypeCNN->paxType() = "ITF";
    CPPUNIT_ASSERT(!_util->matchPaxType(ANY_CARRIER, *_paxTypeCNN, ADULT));
  }

  void testMatchPaxType_Pass_InputPtcExactMatchFeePtc()
  {
    _paxTypeCNN->paxType() = INE;
    CPPUNIT_ASSERT(_util->matchPaxType(ANY_CARRIER, *_paxTypeCNN, INE));
  }

  void testMatchPaxType_Fail_InputPtcNotMapToFeePtcYTH()
  {
    CPPUNIT_ASSERT(!_util->matchPaxType(ANY_CARRIER, *_paxTypeADT, "YTH"));
  }

  void testMatchPaxType_Fail_InputPtcNotMapToFeePtcINS1()
  {
    _paxTypeCNN->paxType() = INS;
    CPPUNIT_ASSERT(!_util->matchPaxType(ANY_CARRIER, *_paxTypeCNN, INFANT));
  }

  void testGetSortedFees_OneSeg()
  {
    AirSeg* seg = createSegment();
    OCFees* fee = addOcFeeToGroup(seg, seg);

    CPPUNIT_ASSERT(ServiceFeeUtil::getSortedFees(*_sfg, _paxTypes)[0].fees() == fee);
  }

  void testGetSortedFees_ReversedOrder()
  {
    AirSeg* seg1 = createSegment(), *seg2 = createSegment();

    addOcFeeToGroup(seg2, seg2); // 3
    addOcFeeToGroup(seg1, seg2); // 2
    OCFees* fee = addOcFeeToGroup(seg1, seg1); // 1

    CPPUNIT_ASSERT(ServiceFeeUtil::getSortedFees(*_sfg, _paxTypes)[0].fees() == fee);
  }

  void testGetSortedFees_MixedOrder()
  {
    AirSeg* seg1 = createSegment(), *seg2 = createSegment();

    addOcFeeToGroup(seg2, seg2); // 3
    OCFees* fee = addOcFeeToGroup(seg1, seg1); // 1
    addOcFeeToGroup(seg1, seg2); // 2

    CPPUNIT_ASSERT(ServiceFeeUtil::getSortedFees(*_sfg, _paxTypes)[0].fees() == fee);
  }

  void testGetSortedFees_SortedBefore()
  {
    AirSeg* seg1 = createSegment(), *seg2 = createSegment();

    OCFees* fee = addOcFeeToGroup(seg1, seg1); // 1
    addOcFeeToGroup(seg1, seg2); // 2
    addOcFeeToGroup(seg2, seg2); // 3

    CPPUNIT_ASSERT(ServiceFeeUtil::getSortedFees(*_sfg, _paxTypes)[0].fees() == fee);
  }

  void testGetSortedFees_SubCode()
  {
    SubCodeInfo subCodeB;
    subCodeB.serviceSubTypeCode() = "B";

    AirSeg* seg1 = createSegment(), *seg2 = createSegment();

    addOcFeeToGroup(seg1, seg2); // 3
    addOcFeeToGroup(seg1, seg1, &subCodeB); // 2
    OCFees* fee = addOcFeeToGroup(seg1, seg1); // 1

    CPPUNIT_ASSERT(ServiceFeeUtil::getSortedFees(*_sfg, _paxTypes)[0].fees() == fee);
  }

  void testGetSortedFees_MultiPax()
  {
    AirSeg* seg = createSegment();

    FarePath fpCNN;
    fpCNN.paxType() = _paxTypeCNN;
    fpCNN.itin() = _itin;

    addOcFeeToGroup(seg, seg, _subCodeA, &fpCNN); // 2
    OCFees* fee = addOcFeeToGroup(seg, seg, _subCodeA); // 1

    CPPUNIT_ASSERT(ServiceFeeUtil::getSortedFees(*_sfg, _paxTypes)[0].fees() == fee);
  }

  void testGetSortedFees_Grouping()
  {
    AirSeg* seg = createSegment(_locSFO, _locDFW);

    addOcFeeToGroup(seg, seg);
    addOcFeeToGroup(seg, seg);
    addOcFeeToGroup(seg, seg);

    _paxTypes.push_back(_paxTypeADT);
    _paxTypes.push_back(_paxTypeCNN);
    _paxTypes.push_back(_paxTypeTVL);

    CPPUNIT_ASSERT_EQUAL((size_t)1, ServiceFeeUtil::getSortedFees(*_sfg, _paxTypes).size());
  }

  void testCreateOcFeesUsages_givenAncillaryPricingTrx_whenNotHandlingSecondCallForMonetaryDiscountTrx_shouldCreateUsagesForAllOcFees()
  {
    AncillaryPricingTrx* ancTrx = _memHandle.create<AncillaryPricingTrx>();
    ancTrx->setRequest(_memHandle.create<PricingRequest>());
    ancTrx->setOptions(_memHandle.create<PricingOptions>());
    _util = _memHandle.insert(new ServiceFeeUtilMock(*ancTrx));
    prepareServiceFeesGroup(nullptr, ancTrx);

    ServiceFeeUtil::createOCFeesUsages(*_sfg, *ancTrx);

    CPPUNIT_ASSERT_EQUAL(2UL, _sfg->ocFeesMap()[_farePath][0]->segCount());
    CPPUNIT_ASSERT_EQUAL(2UL, _sfg->ocFeesMap()[_farePath][0]->ocfeeUsage().size());
  }

  void testCreateOcFeesUsages_givenAncillaryPricingTrx_whenHandlingSecondCallForMonetaryDiscountTrx_shouldCreateUsagesOnlyForOcFeesWithPriceModification()
  {
    AncillaryPricingTrx* ancTrx = _memHandle.create<AncillaryPricingTrx>();
    ancTrx->setRequest(_memHandle.create<PricingRequest>());
    ancTrx->setOptions(_memHandle.create<PricingOptions>());
    _util = _memHandle.insert(new ServiceFeeUtilMock(*ancTrx));
    prepareServiceFeesGroup(nullptr, ancTrx);

    makeTrxSecondCallForMonetaryPricing(ancTrx);
    ServiceFeeUtil::createOCFeesUsages(*_sfg, *ancTrx);

    CPPUNIT_ASSERT_EQUAL(2UL, _sfg->ocFeesMap()[_farePath][0]->segCount());
    CPPUNIT_ASSERT_EQUAL(1UL, _sfg->ocFeesMap()[_farePath][0]->ocfeeUsage().size());
  }

  void makeTrxSecondCallForMonetaryPricing(AncillaryPricingTrx* ancTrx)
  {
    ancTrx->modifiableActivationFlags().setMonetaryDiscount(true);
    _itin->addAncillaryPriceModifier(AncillaryIdentifier(*(_sfg->ocFeesMap()[_farePath][0])), AncillaryPriceModifier());
    ancTrx->itin().push_back(_itin);
  }

  void testGetSortedFees_NoGroupingAmount()
  {
    AirSeg* seg = createSegment(_locSFO, _locDFW);

    addOcFeeToGroup(seg, seg);
    addOcFeeToGroup(seg, seg)->feeAmount() = 5;
    addOcFeeToGroup(seg, seg)->feeAmount() = 7;

    _paxTypes.push_back(_paxTypeADT);
    _paxTypes.push_back(_paxTypeCNN);
    _paxTypes.push_back(_paxTypeTVL);

    CPPUNIT_ASSERT_EQUAL((size_t)3, ServiceFeeUtil::getSortedFees(*_sfg, _paxTypes).size());
  }

  void printSfgToIds(const std::map<std::string, std::set<int>>& sfgToIds)
  {
    for (auto sfgAndIds : sfgToIds)
    {
      std::cout << "  SFG(" << sfgAndIds.first << "): ";

      for (auto id : sfgAndIds.second)
        std::cout << id << " ";

      std::cout << "\n";
    }
  }

  std::vector<PaxOCFeesUsages> getOcFeesUsagesFromSfgToIdsMap(const std::set<int>& ids,
                                                              ServiceFeeUtil::OcFeesUsagesMerger& merger)
  {
    std::vector<PaxOCFeesUsages> result;
    for (auto id : ids)
    {
      std::copy_if(merger.getAllOCFees().begin(), merger.getAllOCFees().end(),
                   std::back_inserter(result),
                   [&](const PaxOCFeesUsages& oc) { return oc.getId() == id; });
    }
    return result;
  }

  struct SegmentWithOCFees
  {
  public:
    SegmentWithOCFees(ServiceFeeUtilTest& test,
                      const Loc* origin,
                      const Loc* destination,
                      PaxType* paxType,
                      std::vector<std::pair<SubCodeInfo*, MoneyAmount>> fees) :
                        _test(test)
    {
      _segment = test.createSegment(origin, destination, paxType);
      _farePath = test._farePath;
      for (auto subCodeAndMoneyAmount : fees)
      {
        auto fee = test.addOcFeeToGroup(_segment, _segment, subCodeAndMoneyAmount.first);
        addFeeUsage(fee);
        fee->feeAmount() = subCodeAndMoneyAmount.second;
        _fees.push_back(fee);
      }
    }

    void addFeeUsage(OCFees* fee)
    {
      auto feeUsage = _test._memHandle.create<OCFeesUsage>();
      feeUsage->oCFees() = fee;
      feeUsage->farePath() = _farePath;
      auto optFee = _test._memHandle.create<OptionalServicesInfo>();
      feeUsage->oCFees()->getCurrentSeg()->_optFee = optFee;
      fee->ocfeeUsage().push_back(feeUsage);
    }

    AirSeg* _segment;
    std::vector<OCFees*> _fees;
    const FarePath* _farePath;

  private:
    ServiceFeeUtilTest& _test;
  };

  void testOcFeesUsagesMerger_SinglePax()
  {
    _sfg->groupCode() = "BG";

    auto seg0 = SegmentWithOCFees(*this, _locSFO, _locDFW, _paxTypeADT,
                                  {{_subCodeA, 0}, {_subCodeA, 0}, {_subCodeA, 1}});
    auto seg1 = SegmentWithOCFees(*this, _locDFW, _locNYC, _paxTypeADT,
                                  {{_subCodeA, 2}, {_subCodeA, 3}, {_subCodeA, 4}});
    auto seg2 = SegmentWithOCFees(*this, _locNYC, _locLON, _paxTypeADT,
                                  {{_subCodeB, 5}, {_subCodeB, 5}, {_subCodeB, 5}});

    _paxTypes.push_back(_paxTypeADT);
    ServiceFeeUtil::OcFeesUsagesMerger merger(_paxTypes, 0);
    auto mergedPaxOcFees = merger.mergeFeesUsagesForSfg(*_sfg);

    CPPUNIT_ASSERT_EQUAL(6UL, mergedPaxOcFees.size());
    CPPUNIT_ASSERT_EQUAL(false, merger.isMaxFeesCountReached());

    {
      auto sfgToIds = merger.getGroupedOCFeeIdsForFarePath(seg0._farePath);
      CPPUNIT_ASSERT(sfgToIds.find("BG") != sfgToIds.end());
      auto paxOcFees = getOcFeesUsagesFromSfgToIdsMap(sfgToIds.find("BG")->second, merger);
      CPPUNIT_ASSERT_EQUAL(2UL, paxOcFees.size());
      CPPUNIT_ASSERT_EQUAL(1L, std::count_if(paxOcFees.begin(), paxOcFees.end(), [&](PaxOCFeesUsages& pf) { return pf.fees()->feeAmount() == 0; }));
      CPPUNIT_ASSERT_EQUAL(1L, std::count_if(paxOcFees.begin(), paxOcFees.end(), [&](PaxOCFeesUsages& pf) { return pf.fees()->feeAmount() == 1; }));
    }
    {
      auto sfgToIds = merger.getGroupedOCFeeIdsForFarePath(seg1._farePath);
      CPPUNIT_ASSERT(sfgToIds.find("BG") != sfgToIds.end());
      auto paxOcFees = getOcFeesUsagesFromSfgToIdsMap(sfgToIds.find("BG")->second, merger);
      CPPUNIT_ASSERT_EQUAL(paxOcFees.size(), 3UL);
      CPPUNIT_ASSERT_EQUAL(1L, std::count_if(paxOcFees.begin(), paxOcFees.end(), [&](PaxOCFeesUsages& pf) { return pf.fees()->feeAmount() == 2; }));
      CPPUNIT_ASSERT_EQUAL(1L, std::count_if(paxOcFees.begin(), paxOcFees.end(), [&](PaxOCFeesUsages& pf) { return pf.fees()->feeAmount() == 3; }));
      CPPUNIT_ASSERT_EQUAL(1L, std::count_if(paxOcFees.begin(), paxOcFees.end(), [&](PaxOCFeesUsages& pf) { return pf.fees()->feeAmount() == 4; }));
    }
    {
      auto sfgToIds = merger.getGroupedOCFeeIdsForFarePath(seg2._farePath);
      CPPUNIT_ASSERT(sfgToIds.find("BG") != sfgToIds.end());
      auto paxOcFees = getOcFeesUsagesFromSfgToIdsMap(sfgToIds.find("BG")->second, merger);
      CPPUNIT_ASSERT_EQUAL(1UL, paxOcFees.size());
      CPPUNIT_ASSERT_EQUAL(1L, std::count_if(paxOcFees.begin(), paxOcFees.end(), [&](PaxOCFeesUsages& pf) { return pf.fees()->feeAmount() == 5; }));
    }
  }

  void testOcFeesUsagesMerger_MultiPax()
  {
    _sfg->groupCode() = "BG";

    auto seg0adt = SegmentWithOCFees(*this, _locSFO, _locDFW, _paxTypeADT,
                                     {{_subCodeA, 1}, {_subCodeA, 1}, {_subCodeB, 1}});
    auto seg0cnn = SegmentWithOCFees(*this, _locSFO, _locDFW, _paxTypeCNN,
                                     {{_subCodeA, 1}, {_subCodeA, 2}, {_subCodeA, 3}});
    auto seg1    = SegmentWithOCFees(*this, _locDFW, _locNYC, _paxTypeADT,
                                     {{_subCodeA, 2}, {_subCodeA, 3}, {_subCodeA, 4}});
    auto seg2    = SegmentWithOCFees(*this, _locNYC, _locLON, _paxTypeADT,
                                     {{_subCodeB, 1}, {_subCodeB, 1}, {_subCodeB, 1}});

    _paxTypes.push_back(_paxTypeADT);
    _paxTypes.push_back(_paxTypeCNN);
    ServiceFeeUtil::OcFeesUsagesMerger merger(_paxTypes, 9);
    auto mergedPaxOcFees = merger.mergeFeesUsagesForSfg(*_sfg);

    CPPUNIT_ASSERT_EQUAL(8UL, mergedPaxOcFees.size());
    CPPUNIT_ASSERT_EQUAL(false, merger.isMaxFeesCountReached());

    {
      auto sfgToIds = merger.getGroupedOCFeeIdsForFarePath(seg0adt._farePath);
      CPPUNIT_ASSERT(sfgToIds.find("BG") != sfgToIds.end());
      auto paxOcFees = getOcFeesUsagesFromSfgToIdsMap(sfgToIds.find("BG")->second, merger);
      CPPUNIT_ASSERT_EQUAL(2UL, paxOcFees.size());
      CPPUNIT_ASSERT_EQUAL(2L, std::count_if(paxOcFees.begin(), paxOcFees.end(), [&](PaxOCFeesUsages& pf) { return pf.fees()->feeAmount() == 1; }));
    }
    {
      auto sfgToIds = merger.getGroupedOCFeeIdsForFarePath(seg0cnn._farePath);
      CPPUNIT_ASSERT(sfgToIds.find("BG") != sfgToIds.end());
      auto paxOcFees = getOcFeesUsagesFromSfgToIdsMap(sfgToIds.find("BG")->second, merger);
      CPPUNIT_ASSERT_EQUAL(3UL, paxOcFees.size());
      CPPUNIT_ASSERT_EQUAL(1L, std::count_if(paxOcFees.begin(), paxOcFees.end(), [&](PaxOCFeesUsages& pf) { return pf.fees()->feeAmount() == 1; }));
      CPPUNIT_ASSERT_EQUAL(1L, std::count_if(paxOcFees.begin(), paxOcFees.end(), [&](PaxOCFeesUsages& pf) { return pf.fees()->feeAmount() == 2; }));
      CPPUNIT_ASSERT_EQUAL(1L, std::count_if(paxOcFees.begin(), paxOcFees.end(), [&](PaxOCFeesUsages& pf) { return pf.fees()->feeAmount() == 3; }));
    }
    {
      auto sfgToIds = merger.getGroupedOCFeeIdsForFarePath(seg1._farePath);
      CPPUNIT_ASSERT(sfgToIds.find("BG") != sfgToIds.end());
      auto paxOcFees = getOcFeesUsagesFromSfgToIdsMap(sfgToIds.find("BG")->second, merger);
      CPPUNIT_ASSERT_EQUAL(3UL, paxOcFees.size());
      CPPUNIT_ASSERT_EQUAL(1L, std::count_if(paxOcFees.begin(), paxOcFees.end(), [&](PaxOCFeesUsages& pf) { return pf.fees()->feeAmount() == 2; }));
      CPPUNIT_ASSERT_EQUAL(1L, std::count_if(paxOcFees.begin(), paxOcFees.end(), [&](PaxOCFeesUsages& pf) { return pf.fees()->feeAmount() == 3; }));
      CPPUNIT_ASSERT_EQUAL(1L, std::count_if(paxOcFees.begin(), paxOcFees.end(), [&](PaxOCFeesUsages& pf) { return pf.fees()->feeAmount() == 4; }));
    }
    {
      auto sfgToIds = merger.getGroupedOCFeeIdsForFarePath(seg2._farePath);
      CPPUNIT_ASSERT(sfgToIds.find("BG") != sfgToIds.end());
      auto paxOcFees = getOcFeesUsagesFromSfgToIdsMap(sfgToIds.find("BG")->second, merger);
      CPPUNIT_ASSERT_EQUAL(1UL, paxOcFees.size());
      CPPUNIT_ASSERT_EQUAL(1L, std::count_if(paxOcFees.begin(), paxOcFees.end(), [&](PaxOCFeesUsages& pf) { return pf.fees()->feeAmount() == 1; }));
    }
  }

  void testOcFeesUsagesMerger_MultiPaxOcLimit()
  {
    _sfg->groupCode() = "BG";

    auto seg0adt = SegmentWithOCFees(*this, _locSFO, _locDFW, _paxTypeADT,
                                     {{_subCodeA, 1}, {_subCodeA, 1}, {_subCodeB, 1}});
    auto seg0cnn = SegmentWithOCFees(*this, _locSFO, _locDFW, _paxTypeCNN,
                                     {{_subCodeA, 1}, {_subCodeA, 2}, {_subCodeA, 3}});
    auto seg1    = SegmentWithOCFees(*this, _locDFW, _locNYC, _paxTypeADT,
                                     {{_subCodeA, 2}, {_subCodeA, 3}, {_subCodeA, 4}});
    auto seg2    = SegmentWithOCFees(*this, _locNYC, _locLON, _paxTypeADT,
                                     {{_subCodeB, 1}, {_subCodeB, 1}, {_subCodeB, 1}});

    _paxTypes.push_back(_paxTypeADT);
    _paxTypes.push_back(_paxTypeCNN);
    ServiceFeeUtil::OcFeesUsagesMerger merger(_paxTypes, 3);
    auto mergedPaxOcFees = merger.mergeFeesUsagesForSfg(*_sfg);

    CPPUNIT_ASSERT_EQUAL(3UL, mergedPaxOcFees.size());
    CPPUNIT_ASSERT_EQUAL(true, merger.isMaxFeesCountReached());
  }

  void testGetFeeRounding_Pass_CallControllingNationCode()
  {
    CPPUNIT_ASSERT(_util->getFeeRounding_old("ZWR", _rF, _rND, _rR));
  }

  void testGetFeeRounding_Pass_DoNotCallControllingNationCode()
  {
    CPPUNIT_ASSERT(_util->getFeeRounding_old("JPY", _rF, _rND, _rR));
  }

  void testGetFeeRounding_Fail_CurrencyNotFound()
  {
    CPPUNIT_ASSERT(!_util->getFeeRounding_old("XXX", _rF, _rND, _rR));
  }

  void testConvertOCFeesCurrency_WithCurrencyCode()
  {
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        50, _util->convertOCFeeCurrency("USD", createOcFee("EUR", 100)).value(), 0.01);
  }

  void testConvertOCFeesCurrency_WithCalcTotals()
  {
    CalcTotals calcTotals;
    calcTotals.equivCurrencyCode = "";
    calcTotals.convertedBaseFareCurrencyCode = "USD";

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        50, _util->convertOCFeeCurrency(calcTotals, createOcFee("EUR", 100)).value(), 0.01);
  }

  void testConvertOCFeesCurrency_WithCurrencyOverride()
  {
    _trx->getOptions()->currencyOverride() = "USD";
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        50, _util->convertOCFeeCurrency(createOcFee("EUR", 100)).value(), 0.01);
  }

  void testConvertOCFeesCurrency_ConversionFailed()
  {
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        50, _util->convertOCFeeCurrency("USD", createOcFee("PLN", 100)).value(), 0.01);
  }

  void testConvertServiceFeesGroupCurrency()
  {
    _trx->getRequest()->ticketingAgent() = createAgent();

    OCFees* fee = addOcFeeToGroup(NULL, NULL);
    fee->feeCurrency() = "EUR";
    fee->feeAmount() = 100;

    _util->convertServiceFeesGroupCurrency(_sfg);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(50, _sfg->ocFeesMap()[_farePath][0]->feeAmount(), 0.01);
  }

  void testGetGroupConfigurationForCode()
  {
    prepareServiceFeesGroup();
    CPPUNIT_ASSERT_EQUAL(
        (const tse::OcFeeGroupConfig*)&_trx->getOptions()->groupsSummaryConfigVec()[0],
        _util->getGroupConfigurationForCode(_trx->getOptions()->groupsSummaryConfigVec(), "ML"));
  }

  void testRequestedOcFeeGroupData()
  {
    prepareServiceFeesGroup();
    CPPUNIT_ASSERT_EQUAL(
        (const tse::RequestedOcFeeGroup*)&_trx->getOptions()->serviceGroupsVec()[0],
        _util->getRequestedOcFeeGroupData(_trx->getOptions()->serviceGroupsVec(), "ML"));
  }

  void testGetOCFeesSummary_Fail_Cfg()
  {
    prepareServiceFeesGroup();
    _trx->getOptions()->groupsSummaryConfigVec().clear();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, _util->getOCFeesSummary(_itin).value(), 0.01);
  }

  void testGetOCFeesSummary_Fail_Requested()
  {
    prepareServiceFeesGroup();
    _trx->getOptions()->serviceGroupsVec().clear();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, _util->getOCFeesSummary(_itin).value(), 0.01);
  }

  void testGetOCFeesSummary_Fail_FeesMap()
  {
    prepareServiceFeesGroup();
    _sfg->ocFeesMap().clear();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, _util->getOCFeesSummary(_itin).value(), 0.01);
  }

  void testGetOCFeesSummary_Fail_PaxType()
  {
    prepareServiceFeesGroup(_paxTypeCNN);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(200, _util->getOCFeesSummary(_itin).value(), 0.01);
  }

  void testGetOCFeesSummary_Pass_BlankCommandName()
  {
    _paxTypeADT->number() = 1;
    _trx->paxType().push_back(_paxTypeADT);
    prepareServiceFeesGroup();
    _trx->getOptions()->groupsSummaryConfigVec()[0].commandName() = "";
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100, _util->getOCFeesSummary(_itin).value(), 0.01);
  }

  void testGetOCFeesSummary_Pass()
  {
    prepareServiceFeesGroup();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(200, _util->getOCFeesSummary(_itin).value(), 0.01);
  }
  void testSetFareIndicator_19()
  {
    PaxTypeFare ptf;
    _util->setFareIndicator(ptf, 19);
    CPPUNIT_ASSERT(ptf.isDiscounted());
  }
  void testSetFareIndicator_20()
  {
    PaxTypeFare ptf;
    _util->setFareIndicator(ptf, 20);
    CPPUNIT_ASSERT(ptf.isDiscounted());
  }
  void testSetFareIndicator_21()
  {
    PaxTypeFare ptf;
    _util->setFareIndicator(ptf, 21);
    CPPUNIT_ASSERT(ptf.isDiscounted());
  }
  void testSetFareIndicator_22()
  {
    PaxTypeFare ptf;
    _util->setFareIndicator(ptf, 22);
    CPPUNIT_ASSERT(ptf.isDiscounted());
  }
  void testSetFareIndicator_25()
  {
    PaxTypeFare ptf;
    _util->setFareIndicator(ptf, 25);
    CPPUNIT_ASSERT(ptf.isFareByRule());
  }
  void testSetFareIndicator_35()
  {
    PaxTypeFare ptf;
    _util->setFareIndicator(ptf, 35);
    CPPUNIT_ASSERT(ptf.isNegotiated());
  }

  void testCollectSegmentStatus()
  {
    Ts2ss& ts2ss = setupSegmentsAndSegmentStatuses();
    Ts2ss colectedTs2ss;
    _util->collectSegmentStatus(*_farePath, colectedTs2ss);

    CPPUNIT_ASSERT_EQUAL(size_t(2), colectedTs2ss.size());

    auto ts2ssI1 = ts2ss.begin();
    auto ts2ssI2 = colectedTs2ss.begin();

    CPPUNIT_ASSERT_EQUAL(ts2ssI1->first, ts2ssI2->first);
    CPPUNIT_ASSERT_EQUAL(ts2ssI1->second.first, ts2ssI2->second.first);
    CPPUNIT_ASSERT_EQUAL(ts2ssI1->second.second, ts2ssI2->second.second);
    ++ts2ssI1;
    ++ts2ssI2;
    CPPUNIT_ASSERT_EQUAL(ts2ssI1->first, ts2ssI2->first);
    CPPUNIT_ASSERT_EQUAL(ts2ssI1->second.first, ts2ssI2->second.first);
    CPPUNIT_ASSERT_EQUAL(ts2ssI1->second.second, ts2ssI2->second.second);
  }
  PaxType* createPaxTypeWithTkt(std::string pax, std::string t1, std::string t2)
  {
    PaxType* ret = _memHandle.create<PaxType>();
    ret->paxType() = pax;
    PaxType::TktInfo* info = _memHandle.create<PaxType::TktInfo>();
    info->tktRefNumber() = t1;
    ret->psgTktInfo().push_back(info);
    info = _memHandle.create<PaxType::TktInfo>();
    info->tktRefNumber() = t2;
    ret->psgTktInfo().push_back(info);
    return ret;
  }
  void testGetSortedFees_PaxWitkTktInfo_T1T2()
  {
    AirSeg* seg = createSegment();
    std::vector<const ServiceFeesGroup*> sfgs;
    sfgs.push_back(_sfg);
    FarePath fp1, fp2;
    fp1.itin() = fp2.itin() = _itin;
    fp1.paxType() = createPaxTypeWithTkt("PAX", "T1", "T3");
    fp2.paxType() = createPaxTypeWithTkt("PAX", "T2", "T4");

    OCFees* fee1 = addOcFeeToGroup(seg, seg, _subCodeA, &fp1); // 1
    addOcFeeToGroup(seg, seg, _subCodeA, &fp2); // 2

    CPPUNIT_ASSERT(ServiceFeeUtil::getSortedFeesForR7(sfgs, _paxTypes)[0].fees() == fee1);
  }
  void testGetSortedFees_PaxWitkTktInfo_T2T1()
  {
    AirSeg* seg = createSegment();
    std::vector<const ServiceFeesGroup*> sfgs;
    sfgs.push_back(_sfg);
    FarePath fp1, fp2;
    fp1.itin() = fp2.itin() = _itin;
    fp1.paxType() = createPaxTypeWithTkt("PAX", "T2", "T3");
    fp2.paxType() = createPaxTypeWithTkt("PAX", "T1", "T4");

    addOcFeeToGroup(seg, seg, _subCodeA, &fp1); // 2
    OCFees* fee2 = addOcFeeToGroup(seg, seg, _subCodeA, &fp2); // 1

    CPPUNIT_ASSERT(ServiceFeeUtil::getSortedFeesForR7(sfgs, _paxTypes)[0].fees() == fee2);
  }
  void testGetSortedFees_PaxWitkTktInfo_T1Blank()
  {
    AirSeg* seg = createSegment();
    std::vector<const ServiceFeesGroup*> sfgs;
    sfgs.push_back(_sfg);
    FarePath fp1;
    fp1.itin() = _itin;
    fp1.paxType() = createPaxTypeWithTkt("PAX", "T1", "T3");

    addOcFeeToGroup(seg, seg, _subCodeA, &fp1); // 2
    OCFees* fee2 = addOcFeeToGroup(seg, seg); // 1

    CPPUNIT_ASSERT(ServiceFeeUtil::getSortedFeesForR7(sfgs, _paxTypes)[0].fees() == fee2);
  }
  void testCheckServiceGroupForAcs_Pass_BG()
  {
    CPPUNIT_ASSERT(ServiceFeeUtil::checkServiceGroupForAcs(ServiceGroup("BG")));
  }
  void testCheckServiceGroupForAcs_Pass_PT()
  {
    CPPUNIT_ASSERT(ServiceFeeUtil::checkServiceGroupForAcs(ServiceGroup("PT")));
  }
  void testCheckServiceGroupForAcs_Fail()
  {
    CPPUNIT_ASSERT(!ServiceFeeUtil::checkServiceGroupForAcs(ServiceGroup("XX")));
  }

  void testFill_OneCode()
  {
    std::set<BookingCode> padisCodeSet;
    SvcFeesResBkgDesigInfo padis;
    padis.bookingCode1() = "A";
    _util->fill(padisCodeSet, &padis);
    CPPUNIT_ASSERT(1 == padisCodeSet.size());
    CPPUNIT_ASSERT(1 == padisCodeSet.count("A"));
  }

  void testFill_FiveCodes()
  {
    std::set<BookingCode> padisCodeSet;
    SvcFeesResBkgDesigInfo padis;
    padis.bookingCode1() = "A";
    padis.bookingCode2() = "B";
    padis.bookingCode3() = "C";
    padis.bookingCode4() = "D";
    padis.bookingCode5() = "E";
    _util->fill(padisCodeSet, &padis);

    CPPUNIT_ASSERT(5 == padisCodeSet.size());
    CPPUNIT_ASSERT(1 == padisCodeSet.count("A"));
    CPPUNIT_ASSERT(1 == padisCodeSet.count("B"));
    CPPUNIT_ASSERT(1 == padisCodeSet.count("C"));
    CPPUNIT_ASSERT(1 == padisCodeSet.count("D"));
    CPPUNIT_ASSERT(1 == padisCodeSet.count("E"));
  }

  void testFill_NoCodes()
  {
    std::set<BookingCode> padisCodeSet;
    SvcFeesResBkgDesigInfo padis;

    _util->fill(padisCodeSet, &padis);

    CPPUNIT_ASSERT(0 == padisCodeSet.size());
  }

  void testGetTranslatedPadisDescription()
  {
    DateTime travelDate;
    SvcFeesResBkgDesigInfo padis;

    // Unit test seat characteristics with less than 30 characters description field.
    padis.bookingCode1() = "W";
    padis.bookingCode2() = "ZX";
    padis.bookingCode4() = "B";
    CPPUNIT_ASSERT_EQUAL(std::string("SEAT WITH BASSINET"),
                         _util->getTranslatedPadisDescription(*_trx, "3B", travelDate, padis));

    // Unit test seat characteristics with more than 30 characters description field.
    padis.bookingCode1() = "W";
    padis.bookingCode2() = "ZX";
    padis.bookingCode4() = "BL";
    CPPUNIT_ASSERT_EQUAL(std::string("BSNET"),
                         _util->getTranslatedPadisDescription(*_trx, "3B", travelDate, padis));
  }

  void testGetTranslatedPadisDescription_abbreviated()
  {
    DateTime travelDate;
    SvcFeesResBkgDesigInfo padis;
    padis.bookingCode1() = "W";
    padis.bookingCode2() = "ZX";
    padis.bookingCode3() = "A";
    padis.bookingCode4() = "B";
    CPPUNIT_ASSERT_EQUAL(std::string("AISLE BSNET WLCHR ZX"),
                         _util->getTranslatedPadisDescription(*_trx, "1P", travelDate, padis));
  }

  void testAddSeatCabinCharacteristic_Carrier_LH()
  {
    OCFeesUsage* ocfUsage = createOCFeesUsage("LH");
    _itin->setTravelDate(DateTime::localTime());
    SvcFeesResBkgDesigInfo padis;
    padis.bookingCode1() = "W";
    padis.bookingCode2() = "ZX";
    padis.bookingCode3() = "A";
    padis.bookingCode4() = "B";
    _util->addSeatCabinCharacteristic(*_trx, *ocfUsage, padis);
    CPPUNIT_ASSERT_EQUAL(std::string("AISLE SEAT WITH BASSINET"), ocfUsage->upgradeT198CommercialName());
  }

  void testAddSeatCabinCharacteristic_Carrier_1P()
  {
    OCFeesUsage* ocfUsage = createOCFeesUsage("1P");
    _itin->setTravelDate(DateTime::localTime());
    SvcFeesResBkgDesigInfo padis;
    padis.bookingCode1() = "W";
    padis.bookingCode2() = "ZX";
    padis.bookingCode3() = "A";
    padis.bookingCode4() = "B";
    _util->addSeatCabinCharacteristic(*_trx, *ocfUsage, padis);
    CPPUNIT_ASSERT_EQUAL(std::string("AISLE BSNET WLCHR ZX"),
                         ocfUsage->upgradeT198CommercialName());
  }

  void testAddSeatCabinCharacteristic_Carrier_1Z_FULL_DESCRIPTION()
  {
    OCFeesUsage* ocfUsage = createOCFeesUsage("1Z");
    _itin->setTravelDate(DateTime::localTime());
    SvcFeesResBkgDesigInfo padis;
    padis.bookingCode1() = "W";
    padis.bookingCode2() = "K";
    padis.bookingCode3() = "E";
    _util->addSeatCabinCharacteristic(*_trx, *ocfUsage, padis);
    CPPUNIT_ASSERT_EQUAL(std::string("EXIT BULKHEAD WINDOW LEFT"),
                         ocfUsage->upgradeT198CommercialName());
  }

  void testAddSeatCabinCharacteristic_Carrier_1Z_ABBREVIATIONS()
  {
    OCFeesUsage* ocfUsage = createOCFeesUsage("1Z");
    _itin->setTravelDate(DateTime::localTime());
    SvcFeesResBkgDesigInfo padis;
    padis.bookingCode1() = "W";
    padis.bookingCode2() = "K";
    padis.bookingCode3() = "E";
    padis.bookingCode4() = "RS";
    _util->addSeatCabinCharacteristic(*_trx, *ocfUsage, padis);
    CPPUNIT_ASSERT_EQUAL(std::string("EXIT BULKH RSIDE WINDL"),
                         ocfUsage->upgradeT198CommercialName());
  }

  void testAddSeatCabinCharacteristic_Carrier_AU_ATPCO_ABBREVIATIONS()
  {
    OCFeesUsage* ocfUsage = createOCFeesUsage("AU");
    _itin->setTravelDate(DateTime::localTime());
    SvcFeesResBkgDesigInfo padis;
    padis.bookingCode1() = "AG";
    padis.bookingCode2() = "K";
    padis.bookingCode3() = "E";
    padis.bookingCode4() = "RS";
    padis.bookingCode5() = "A";
    _util->addSeatCabinCharacteristic(*_trx, *ocfUsage, padis);
    CPPUNIT_ASSERT_EQUAL(std::string("AISLE ADGAL EXIT BULKH RSIDE"),
                         ocfUsage->upgradeT198CommercialName());
  }

  // Tests for fillOutOCFeeUsageByPadisDescription() method
  void testFillOutOCFeeUsageByPadisDescription_OCFeeUsage_Present_Carrier_LH()
  {
    OCFees* ocfee = createOCFees("LH");
    _itin->setTravelDate(DateTime::localTime());
    SvcFeesResBkgDesigInfo padis;
    padis.bookingCode1() = "W";
    padis.bookingCode2() = "ZX";
    padis.bookingCode3() = "A";
    padis.bookingCode4() = "B";

    _util->fillOutOCFeeUsageByPadisDescription(*_trx, *ocfee, 0, padis);
    CPPUNIT_ASSERT_EQUAL(std::string("AISLE SEAT WITH BASSINET"),
                         ocfee->ocfeeUsage()[0]->upgradeT198CommercialName());
    CPPUNIT_ASSERT_EQUAL((size_t)1, ocfee->ocfeeUsage().size());
  }

  void testCreateOCFeesUsageForSingleS7_NO_Padis_Present_Carrier_LH()
  {
    OCFees* ocfee = createOCFees("LH");
    _itin->setTravelDate(DateTime::localTime());

    _util->createOCFeesUsageForSingleS7(*_trx, *ocfee, 0);
    CPPUNIT_ASSERT_EQUAL((size_t)0, ocfee->ocfeeUsage()[0]->upgradeT198CommercialName().size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, ocfee->ocfeeUsage().size());
  }

  void testCreateOCFeesUsageForSingleS7_Padis_Present_Carrier_LH()
  {
    OCFees* ocfee = createOCFees("LH");
    _itin->setTravelDate(DateTime::localTime());
    SvcFeesResBkgDesigInfo padis;
    padis.bookingCode1() = "W";
    padis.bookingCode2() = "ZX";
    padis.bookingCode3() = "A";
    padis.bookingCode4() = "B";
    ocfee->padisData().push_back(&padis);

    _util->createOCFeesUsageForSingleS7(*_trx, *ocfee, 0);
    CPPUNIT_ASSERT_EQUAL(std::string("AISLE SEAT WITH BASSINET"),
                         ocfee->ocfeeUsage()[0]->upgradeT198CommercialName());
    CPPUNIT_ASSERT_EQUAL((size_t)1, ocfee->ocfeeUsage().size());
  }

  void testCreateOCFeesUsageForSingleS7_IgnorePadis()
  {
    OCFees* ocfee = createOCFees("LH");
    _itin->setTravelDate(DateTime::localTime());
    SvcFeesResBkgDesigInfo padis;
    padis.bookingCode3() = "A";
    padis.bookingCode4() = "B";
    ocfee->padisData().push_back(&padis);

    const bool ignorePadis = true;
    _util->createOCFeesUsageForSingleS7(*_trx, *ocfee, 0, ignorePadis);

    const OCFeesUsage* ocFeesUsage = ocfee->ocfeeUsage()[0];
    CPPUNIT_ASSERT(1 == ocfee->ocfeeUsage().size());
    CPPUNIT_ASSERT(ocFeesUsage->upgradeT198CommercialName().empty());
    CPPUNIT_ASSERT(1 == ocFeesUsage->padisData().size());
  }

  void testCreateOCFeesUsageForSingleS7_MultiSequence()
  {
    OCFees* ocfee = createOCFees("LH");
    _itin->setTravelDate(DateTime::localTime());
    SvcFeesResBkgDesigInfo padis1;
    padis1.bookingCode1() = "A";
    ocfee->padisData().push_back(&padis1);
    SvcFeesResBkgDesigInfo padis2;
    padis2.bookingCode1() = "B";
    ocfee->padisData().push_back(&padis2);

    _util->createOCFeesUsageForSingleS7(*_trx, *ocfee, 0);

    const OCFeesUsage* ocFeesUsage1 = ocfee->ocfeeUsage()[0];
    const OCFeesUsage* ocFeesUsage2 = ocfee->ocfeeUsage()[1];
    CPPUNIT_ASSERT(2 == ocfee->ocfeeUsage().size());
    CPPUNIT_ASSERT(!ocFeesUsage1->upgradeT198CommercialName().empty());
    CPPUNIT_ASSERT(2 == ocFeesUsage1->padisData().size());
    CPPUNIT_ASSERT(!ocFeesUsage2->upgradeT198CommercialName().empty());
    CPPUNIT_ASSERT(2 == ocFeesUsage2->padisData().size());
  }

  void testCreateOCFeesUsageForSingleS7_IgnorePadis_MultiSequence()
  {
    OCFees* ocfee = createOCFees("LH");
    _itin->setTravelDate(DateTime::localTime());
    SvcFeesResBkgDesigInfo padis1;
    padis1.bookingCode1() = "A";
    ocfee->padisData().push_back(&padis1);
    SvcFeesResBkgDesigInfo padis2;
    padis2.bookingCode1() = "B";
    ocfee->padisData().push_back(&padis2);

    const bool ignorePadis = true;
    _util->createOCFeesUsageForSingleS7(*_trx, *ocfee, 0, ignorePadis);

    const OCFeesUsage* ocFeesUsage = ocfee->ocfeeUsage()[0];
    CPPUNIT_ASSERT(1 == ocfee->ocfeeUsage().size());
    CPPUNIT_ASSERT(ocFeesUsage->upgradeT198CommercialName().empty());
    CPPUNIT_ASSERT(2 == ocFeesUsage->padisData().size());
  }

  void testIsServiceGroupInvalidForAcs_True()
  {
    ServiceGroup group("XX");

    CPPUNIT_ASSERT(_util->isServiceGroupInvalidForAcs(group));
  }

  void testIsServiceGroupInvalidForAcs_False()
  {
    ServiceGroup group("SA");

    CPPUNIT_ASSERT(!_util->isServiceGroupInvalidForAcs(group));
  }

  void testIsFeeFarePercentage_True_C()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerMileageAppl() = 'C';

    CPPUNIT_ASSERT(_util->isFeeFarePercentage(s7));
  }

  void testIsFeeFarePercentage_True_P()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerMileageAppl() = 'P';

    CPPUNIT_ASSERT(_util->isFeeFarePercentage(s7));
  }

  void testIsFeeFarePercentage_True_H()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerMileageAppl() = 'H';

    CPPUNIT_ASSERT(_util->isFeeFarePercentage(s7));
  }

  void testIsFeeFarePercentage_False()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerMileageAppl() = 'X';

    CPPUNIT_ASSERT(!_util->isFeeFarePercentage(s7));
  }

  void testConvertFeeCurrencyStrategy_convertTaxItem()
  {
    MoneyAmount sourceFee = 100;
    CurrencyCode sourceCurr = "EUR";

    MoneyAmount targetFee = 50;
    CurrencyCode targetCurr = "USD";

    _trx->getOptions()->currencyOverride() = targetCurr;

    TaxItemFeeCurrencyConverter currencyExchanger(*_util);

    tse::OCFees::TaxItem taxItem;
    taxItem.setTaxAmount(sourceFee);
    taxItem.setCurrency(sourceCurr);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(targetFee, currencyExchanger(taxItem), 0.01);
  }

  void test_isRequestFromAS_false()
  {
    _trx->getRequest()->ticketingAgent() = createAgent();
    Billing* billing = _memHandle.create<Billing>();
    _trx->billing() = billing;
    CPPUNIT_ASSERT_EQUAL(false, TrxUtil::isRequestFromAS(*_trx));
  }

  void test_isRequestFromAS_true()
  {
    prepareASRequest();

    CPPUNIT_ASSERT_EQUAL(true, TrxUtil::isRequestFromAS(*_trx));
  }

  void test_isPerformEMDCheck_EmdForFlightRelatedServiceAndPrepaidBaggage()
  {
    prepareASRequest();
    SubCodeInfo subCode;
    _trx->billing()->requestPath() = AEBSO_PO_ATSE_PATH;
    _trx->modifiableActivationFlags().setEmdForFlightRelatedServiceAndPrepaidBaggage(true);

    CPPUNIT_ASSERT_EQUAL(true, ServiceFeeUtil::isPerformEMDCheck(*_trx, subCode));
  }

  void test_isPerformEMDCheck_NotEmdForFlightRelatedServiceAndPrepaidBaggage()
  {
    prepareASRequest();
    SubCodeInfo subCode;
    _trx->billing()->requestPath() = AEBSO_PO_ATSE_PATH;
    _trx->modifiableActivationFlags().setEmdForFlightRelatedServiceAndPrepaidBaggage(false);

    CPPUNIT_ASSERT_EQUAL(false, ServiceFeeUtil::isPerformEMDCheck(*_trx, subCode));
  }

  void test_isPerformEMDCheck_EmdForFlightRelatedServiceAndPrepaidBaggageGroup99()
  {
    prepareASRequest();
    SubCodeInfo subCode;
    subCode.serviceGroup() = "99";
    _trx->billing()->requestPath() = AEBSO_PO_ATSE_PATH;
    _trx->modifiableActivationFlags().setEmdForFlightRelatedServiceAndPrepaidBaggage(true);

    CPPUNIT_ASSERT_EQUAL(false, ServiceFeeUtil::isPerformEMDCheck(*_trx, subCode));
  }

  OCFeesUsage* createOCFeesUsage(CarrierCode cxr, ServiceGroup serviceGroup = "SA")
  {
    OCFees* ocFees = createOCFees(cxr);

    OCFeesUsage* ocFeeUsage = _memHandle.insert(new OCFeesUsage);
    ocFeeUsage->oCFees() = ocFees;
    ocFees->ocfeeUsage().push_back(ocFeeUsage);
    return ocFeeUsage;
  }

  OCFees* createOCFees(CarrierCode cxr, ServiceGroup serviceGroup = "SA")
  {
    OCFees* ocFees = _memHandle.insert(new OCFees);
    ocFees->carrierCode() = cxr;

    SubCodeInfo* subCodeInfo = _memHandle.insert(new SubCodeInfo);
    subCodeInfo->commercialName() = "PRIMARY SEAT ASSIGNMENT";
    subCodeInfo->serviceGroup() = serviceGroup;
    ocFees->subCodeInfo() = subCodeInfo;

    OptionalServicesInfo* optServicesInfo = _memHandle.insert(new OptionalServicesInfo);
    ocFees->optFee() = optServicesInfo;

    AirSeg* travelStartSeg = createSegment(_locLON, _locDFW);
    ocFees->travelStart() = travelStartSeg;
    ocFees->travelEnd() = travelStartSeg;
    ocFees->farePath() = _farePath;

    return ocFees;
  }


private:
  class ServiceFeeUtilDataHandleMock : public DataHandleMock
  {
    TestMemHandle _memHandle;

    std::vector<SeatCabinCharacteristicInfo*>* seatCabinCharacteristicInfoVector;

    SeatCabinCharacteristicInfo*
    getSeatCabinCharacteristic(const CarrierCode& carrier,
                               const Indicator& codeType,
                               const SeatCabinCode& seatCabinCode,
                               const std::string& codeDescription,
                               const std::string& displayDescription,
                               const std::string& abbreviatedDescription)
    {
      SeatCabinCharacteristicInfo* seatCabinCharacteristic =
          _memHandle.create<SeatCabinCharacteristicInfo>();
      seatCabinCharacteristic->carrier() = carrier;
      seatCabinCharacteristic->codeType() = codeType;
      seatCabinCharacteristic->seatCabinCode() = seatCabinCode;
      seatCabinCharacteristic->codeDescription() = codeDescription;
      seatCabinCharacteristic->displayDescription() = displayDescription;
      seatCabinCharacteristic->abbreviatedDescription() = abbreviatedDescription;
      return seatCabinCharacteristic;
    }

  public:
    ServiceFeeUtilDataHandleMock()
    {
      seatCabinCharacteristicInfoVector =
          _memHandle.create<std::vector<SeatCabinCharacteristicInfo*> >();
      seatCabinCharacteristicInfoVector->push_back(
          getSeatCabinCharacteristic("2P", 'S', "XX", "WINDOW", "WINDOW", "WINDO"));
      seatCabinCharacteristicInfoVector->push_back(
          getSeatCabinCharacteristic("1P", 'S', "W", "Wheelchairr", "WHEELCHAIR", "WLCHR"));
      seatCabinCharacteristicInfoVector->push_back(
          getSeatCabinCharacteristic("1P", 'S', "ZX", "test zxc", "TEST ZXC", "ZX"));
      seatCabinCharacteristicInfoVector->push_back(
          getSeatCabinCharacteristic("1P", 'F', "ZX", "test zxc update", "TEST ZXC UPDATE", "ZX"));
      seatCabinCharacteristicInfoVector->push_back(getSeatCabinCharacteristic(
          "AP", 'F', "QA", "QA code for QA cabin", "QA CODE FOR QA CABIN", "QA"));
      seatCabinCharacteristicInfoVector->push_back(
          getSeatCabinCharacteristic("3B", 'S', "A", "Aisle Seat", "AISLE SEAT", "AISLE"));
      seatCabinCharacteristicInfoVector->push_back(
          getSeatCabinCharacteristic("1Z", 'S', "W", "WINDOW LEFT SIDE", "WINDOW LEFT", "WINDL"));
      seatCabinCharacteristicInfoVector->push_back(
          getSeatCabinCharacteristic("1Y", 'S', "W", "WINDOW RIGHT SIDE", "WINDOW RIGHT", "WINDR"));
      seatCabinCharacteristicInfoVector->push_back(
          getSeatCabinCharacteristic("**", 'S', "A", "AISLE", "AISLE", "AISLE"));
      seatCabinCharacteristicInfoVector->push_back(getSeatCabinCharacteristic(
          "**", 'S', "B", "SEAT WITH BASSINET FACILITY", "SEAT WITH BASSINET", "BSNET"));
      seatCabinCharacteristicInfoVector->push_back(
          getSeatCabinCharacteristic("**", 'S', "LS", "LEFT SIDE", "LEFT SIDE", "LSIDE"));
      seatCabinCharacteristicInfoVector->push_back(
          getSeatCabinCharacteristic("**", 'S', "RS", "RIGHT SIDE", "RIGHT SIDE", "RSIDE"));
      seatCabinCharacteristicInfoVector->push_back(
          getSeatCabinCharacteristic("**", 'S', "E", "EXIT", "EXIT", "EXIT"));
      seatCabinCharacteristicInfoVector->push_back(
          getSeatCabinCharacteristic("**", 'S', "K", "BULKHEAD", "BULKHEAD", "BULKH"));
      seatCabinCharacteristicInfoVector->push_back(getSeatCabinCharacteristic(
          "**", 'S', "AG", "ADJACENT TO GALLERY", "GALLERY IS ADJACENT", "ADGAL"));
      // Add a seat characteristics with more than 30 characters description field.
      seatCabinCharacteristicInfoVector->push_back(
          getSeatCabinCharacteristic("**",
                                     'S',
                                     "BL",
                                     "SEAT WITH BASSINET FACILITY MORE THAN THIRTY CHARACTERS",
                                     "SEAT WITH BASSINET MORE THAN THIRTY CHARACTERS",
                                     "BSNET"));
    }

    const std::vector<SeatCabinCharacteristicInfo*>&
    getSeatCabinCharacteristicInfo(const CarrierCode& carrier,
                                   const Indicator& codeType,
                                   const DateTime& travelDate)
    {
      std::vector<SeatCabinCharacteristicInfo*>* result =
          _memHandle.create<std::vector<SeatCabinCharacteristicInfo*> >();

      remove_copy_if(seatCabinCharacteristicInfoVector->begin(),
                     seatCabinCharacteristicInfoVector->end(),
                     std::back_inserter(*result),
                     std::not1(CheckSeatCabinCharacteristic(carrier, codeType)));

      return *result;
    }
  };

  static const CarrierCode _carrierKL;
  const Loc* _locSFO;
  const Loc* _locDFW;
  const Loc* _locNYC;
  const Loc* _locLON;

  TestMemHandle _memHandle;
  std::vector<PaxType*> _paxTypes;
  Itin* _itin;
  PricingTrx* _trx;
  FarePath* _farePath;
  PricingUnit* _pricingUnit;
  FareUsage* _fareUsage;
  ServiceFeeUtil* _util;
  SubCodeInfo* _subCodeA;
  SubCodeInfo* _subCodeB;
  SubCodeInfo* _subCodeC;
  ServiceFeesGroup* _sfg;
  PaxType* _paxTypeADT;
  PaxType* _paxTypeCNN;
  PaxType* _paxTypeTVL;
  RoundingFactor _rF;
  CurrencyNoDec _rND;
  RoundingRule _rR;
  ServiceFeeUtilDataHandleMock* _dataHandleMock;
};

const CarrierCode ServiceFeeUtilTest::_carrierKL = "KL";

CPPUNIT_TEST_SUITE_REGISTRATION(ServiceFeeUtilTest);
}
