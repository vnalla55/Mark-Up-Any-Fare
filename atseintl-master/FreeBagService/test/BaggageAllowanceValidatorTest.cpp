#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SvcFeesCxrResultingFCLInfo.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "DBAccess/TaxCarrierFlightInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncRequest.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Billing.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/BaggageAllowanceValidator.h"
#include "FreeBagService/BagValidationOpt.h"
#include "FreeBagService/test/AirSegBuilder.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "FreeBagService/test/S5Builder.h"
#include "FreeBagService/test/S7Builder.h"
#include "ServiceFees/OCFees.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using boost::assign::operator+=;

namespace
{
// MOCKS
class MyDataHandle : public DataHandleMock
{
public:
  MyDataHandle() {}

  const TaxCarrierFlightInfo* getTaxCarrierFlight(const VendorCode& vendor, int itemNo)
  {
    switch (itemNo)
    {
    case 1:
      return 0;
      break;
    case 2:
      return _memHandle.create<TaxCarrierFlightInfo>();
      break;
    case 3:
    {
      TaxCarrierFlightInfo* taxInfo = _memHandle.create<TaxCarrierFlightInfo>();
      taxInfo->segCnt() = 1;
      return taxInfo;
      break;
    }
    default:
    {
      CarrierFlightSeg* cxrSeg =
          new CarrierFlightSeg(); // this will be deleted by TaxCarrierFlightInfo
      cxrSeg->marketingCarrier() = "AA";
      cxrSeg->flt1() = -1;

      TaxCarrierFlightInfo* taxInfo = _memHandle.create<TaxCarrierFlightInfo>();
      taxInfo->segs() += cxrSeg;
      taxInfo->segCnt() = 1;

      return taxInfo;
      break;
    }
    }
  }

  const std::vector<SvcFeesSecurityInfo*>&
  getSvcFeesSecurity(const VendorCode& vendor, const int itemNo)
  {
    if (itemNo == 123)
    {
      SvcFeesSecurityInfo* info = _memHandle.create<SvcFeesSecurityInfo>();
      info->carrierGdsCode() = "XX";

      std::vector<SvcFeesSecurityInfo*>* infos =
          _memHandle.create<std::vector<SvcFeesSecurityInfo*>>();
      infos->push_back(info);

      return *infos;
    }
    else
      return DataHandleMock::getSvcFeesSecurity(vendor, itemNo);
  }

private:
  TestMemHandle _memHandle;
};
}

class BaggageAllowanceValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageAllowanceValidatorTest);

  CPPUNIT_TEST(testGetTvlDepartureDOW);
  CPPUNIT_TEST(testGetTvlDepartureDOW_Zero);

  CPPUNIT_TEST(testCheckTravelDate_True_StartSpecified);
  CPPUNIT_TEST(testCheckTravelDate_True_StopSpecified);
  CPPUNIT_TEST(testCheckTravelDate_True_BothSpecified);
  CPPUNIT_TEST(testCheckTravelDate_True_NoneSpecified);
  CPPUNIT_TEST(testCheckTravelDate_False_StartSpecified);
  CPPUNIT_TEST(testCheckTravelDate_False_StopSpecified);
  CPPUNIT_TEST(testCheckTravelDate_False_BothSpecified);
  CPPUNIT_TEST(testCheckTravelDate_False_NoneSpecified_After);
  CPPUNIT_TEST(testCheckTravelDate_False_NoneSpecified_Before);

  CPPUNIT_TEST(testCheckFrequentFlyerStatus_True);
  CPPUNIT_TEST(testCheckFrequentFlyerStatus_False);

  CPPUNIT_TEST(testCheckFrequentFlyerStatus_Fail_NoData);
  CPPUNIT_TEST(testCheckFrequentFlyerStatus_Fail_Carrier);
  CPPUNIT_TEST(testCheckFrequentFlyerStatus_Fail_Status);
  CPPUNIT_TEST(testCheckFrequentFlyerStatus_Pass);
  CPPUNIT_TEST(testCheckFrequentFlyerStatus_Softpass);

  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_Free);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_NoDefer);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_Defer_BTA);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_Defer_NotAllowed);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_Defer_SameCxr);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_Defer_O_UsDot);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_Defer_D_NonUsDot);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_Defer_Pass);

  CPPUNIT_TEST(testValidateStartAndStopTime_True_Empty);
  CPPUNIT_TEST(testValidateStartAndStopTime_True_Zeros);
  CPPUNIT_TEST(testValidateStartAndStopTime_True);
  CPPUNIT_TEST(testValidateStartAndStopTime_False_More);
  CPPUNIT_TEST(testValidateStartAndStopTime_False_Less);
  CPPUNIT_TEST(testValidateStartAndStopTime_False_Dow);

  CPPUNIT_TEST(testValidateStartAndStopTimeRange_False_Dow);
  CPPUNIT_TEST(testValidateStartAndStopTimeRange_True_OpenWithoutDate);
  CPPUNIT_TEST(testValidateStartAndStopTimeRange_True_StartDowEqual_NoStartTime);
  CPPUNIT_TEST(testValidateStartAndStopTimeRange_True_StartDowEqual);
  CPPUNIT_TEST(testValidateStartAndStopTimeRange_False_StartDowEqual);
  CPPUNIT_TEST(testValidateStartAndStopTimeRange_True_StopDowEqual_NoStopTime);
  CPPUNIT_TEST(testValidateStartAndStopTimeRange_True_StopDowEqual);
  CPPUNIT_TEST(testValidateStartAndStopTimeRange_False_StopDowEqual);
  CPPUNIT_TEST(testValidateStartAndStopTimeRange_True_DepartureDowInBetween);

  CPPUNIT_TEST(testCheckEquipmentType_True_NoPnr);
  CPPUNIT_TEST(testCheckEquipmentType_True_EqCodeEmpty);
  CPPUNIT_TEST(testCheckEquipmentType_True_EqCodeBlank);
  CPPUNIT_TEST(testCheckEquipmentType_True_OpenSegment);
  CPPUNIT_TEST(testCheckEquipmentType_True_MatchingEqType);
  CPPUNIT_TEST(testCheckEquipmentType_False_NonMatchingEqType);
  CPPUNIT_TEST(testCheckEquipmentType_False_EmptyEqType);
  CPPUNIT_TEST(testCheckEquipmentType_True_Softmatch);
  CPPUNIT_TEST(testCheckEquipmentTypeHiddenStops_True_MatchingEqType);
  CPPUNIT_TEST(testCheckEquipmentTypeHiddenStops_False_NonMatchingEqType);
  CPPUNIT_TEST(testCheckEquipmentTypeHiddenStops_False_EmptyEqType);
  CPPUNIT_TEST(testCheckEquipmentTypeHiddenStops_True_Softmatch);

  CPPUNIT_TEST(testCheckTourCode_True_Empty);
  CPPUNIT_TEST(testCheckTourCode_True_Cat35Match);
  CPPUNIT_TEST(testCheckTourCode_True_Cat27Match);
  CPPUNIT_TEST(testCheckTourCode_False_Cat35Match);
  CPPUNIT_TEST(testCheckTourCode_False_Cat27Match);
  CPPUNIT_TEST(testCheckTourCode_False_NoNegFareData);
  CPPUNIT_TEST(testCheckTourCode_False_NotCat35);
  CPPUNIT_TEST(testCheckTourCode_False_Cat35TCEmpty);

  CPPUNIT_TEST(testCheckStartStopTime_Blank);
  CPPUNIT_TEST(testCheckStartStopTime_Blank2);
  CPPUNIT_TEST(testCheckStartStopTime_Daily);
  CPPUNIT_TEST(testCheckStartStopTime_Range);
  CPPUNIT_TEST(testCheckStartStopTime_Default);

  CPPUNIT_TEST(testCheckDow_True_EmptyDow);
  CPPUNIT_TEST(testCheckDow_True);
  CPPUNIT_TEST(testCheckDow_False);

  CPPUNIT_TEST(testIsValidCarrierFlight_FailOnMarketing);
  CPPUNIT_TEST(testIsValidCarrierFlight_FailOnOperating_Open);
  CPPUNIT_TEST(testIsValidCarrierFlight_FailOnOperating_Air);
  CPPUNIT_TEST(testIsValidCarrierFlight_PassFlt);
  CPPUNIT_TEST(testIsValidCarrierFlight_PassFlt_OperatingEmpty);
  CPPUNIT_TEST(testIsValidCarrierFlight_PassFltNumber);

  CPPUNIT_TEST(testIsValidCarrierFlightNumber_Open);
  CPPUNIT_TEST(testIsValidCarrierFlightNumber_Fail_1);
  CPPUNIT_TEST(testIsValidCarrierFlightNumber_Fail_2);
  CPPUNIT_TEST(testIsValidCarrierFlightNumber_Fail_3);
  CPPUNIT_TEST(testIsValidCarrierFlightNumber_Pass_1);
  CPPUNIT_TEST(testIsValidCarrierFlightNumber_Pass_2);

  CPPUNIT_TEST(testCheckCarrierFlightApplT186_Itemno_Zero);
  CPPUNIT_TEST(testCheckCarrierFlightApplT186_186_Null);
  CPPUNIT_TEST(testCheckCarrierFlightApplT186_186_SegCnt_Zero);
  CPPUNIT_TEST(testCheckCarrierFlightApplT186_186_Segs_Empty);
  CPPUNIT_TEST(testCheckCarrierFlightApplT186_Defer_Pass);
  CPPUNIT_TEST(testCheckCarrierFlightApplT186_Defer_Fail);
  CPPUNIT_TEST(testCheckCarrierFlightApplT186_Free_Pass);
  CPPUNIT_TEST(testCheckCarrierFlightApplT186_Free_Fail);

  CPPUNIT_TEST(testIsValidResultingFareClass_Fail_On_Cxr);
  CPPUNIT_TEST(testIsValidResultingFareClass_Pass_Ancillary);
  CPPUNIT_TEST(testIsValidResultingFareClass_Pass);

  CPPUNIT_TEST(testConstructor_Normal);
  CPPUNIT_TEST(testConstructor_Ancillary);
  CPPUNIT_TEST(testConstructor_Ancillary_Hardmatch);

  CPPUNIT_TEST(testCheckSecurity_Normal);
  CPPUNIT_TEST(testCheckSecurity_Ancillary);

  CPPUNIT_TEST(testIsValidFareClassFareType_Pass);
  CPPUNIT_TEST(testIsValidFareClassFareType_Softpass);

  CPPUNIT_TEST(testMatchRuleTariff_Fail);
  CPPUNIT_TEST(testMatchRuleTariff_Softpass);

  CPPUNIT_TEST(testBuildFees);

  CPPUNIT_TEST(testSupplementFees);
  CPPUNIT_TEST(testSupplementFees_NotGuaranteed);

  CPPUNIT_TEST(test_checkCabinInSegment_noCabin_PASS);
  CPPUNIT_TEST(test_checkCabinInSegment_noSegment_PASS);
  CPPUNIT_TEST(test_checkCabinInSegment_nonAirSegment_PASS);
  CPPUNIT_TEST(test_checkRBDInSegment_noRBDItemNo_PASS);
  CPPUNIT_TEST(test_checkRBDInSegment_noRBDRecords_FAIL);
  CPPUNIT_TEST(test_checkRBDInSegment_noSegment_PASS);
  CPPUNIT_TEST(test_checkRBDInSegment_nonAirSegment_PASS);
  CPPUNIT_TEST(test_checkResultingFareClassInSegment_noT171ItemNo_PASS);
  CPPUNIT_TEST(test_checkResultingFareClassInSegment_noPaxTypeFare_FAIL);
  CPPUNIT_TEST(test_checkResultingFareClassInSegment_emptyCarrierResultingFCLInfoRecords_FAIL);
  CPPUNIT_TEST(test_checkOutputTicketDesignatorInSegment_empty_PASS);
  CPPUNIT_TEST(test_checkRuleInSegment_noRule_PASS);
  CPPUNIT_TEST(test_checkRuleInSegment_noPaxTypeFare_PASS);
  CPPUNIT_TEST(test_checkRuleTariffInSegment_noRuleTariffNumber_PASS);
  CPPUNIT_TEST(test_checkRuleTariffInSegment_noPaxTypeFare_PASS);
  CPPUNIT_TEST(test_checkFareIndInSegment_noPaxTypeFare_PASS);
  CPPUNIT_TEST(test_checkFareIndInSegment_emptyFareInd_PASS);
  CPPUNIT_TEST(test_checkCarrierFlightApplT186InSegment_noSegment_FAIL);
  CPPUNIT_TEST(test_checkCarrierFlightApplT186InSegment_nonAirSegment_FAIL);
  CPPUNIT_TEST(test_checkCarrierFlightApplT186InSegment_emptyT186ItemNo_PASS);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _mdh = _memHandle.create<MyDataHandle>();
    _validator = createValidator();
  }

  void tearDown() { _memHandle.clear(); }

  BaggageAllowanceValidator* createValidator(bool ancillaryBaggage = false, bool hardMatch = false)
  {
    _farePath = _memHandle.create<FarePath>();
    _farePath->itin() = _memHandle.create<Itin>();
    _farePath->itin()->setBaggageTripType(BaggageTripType::TO_FROM_US);
    _farePath->paxType() = _memHandle.create<PaxType>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());

    PricingRequest* request;
    if (ancillaryBaggage)
    {
      request = _memHandle.create<AncRequest>();
      static_cast<AncRequest*>(request)->majorSchemaVersion() = 2;
      static_cast<AncRequest*>(request)->hardMatchIndicator() = hardMatch;
    }
    else
    {
      request = _memHandle.create<PricingRequest>();
    }

    Agent* agent = _memHandle.create<Agent>();
    agent->vendorCrsCode() = "XX";
    request->ticketingAgent() = agent;

    _trx->billing() = _memHandle.create<Billing>();
    _trx->setRequest(request);

    BaggageTravel* bagTvl =
        BaggageTravelBuilder(&_memHandle).withTrx(_trx).withFarePath(_farePath).build();

    CheckedPoint* checkedPoint = _memHandle.create<CheckedPoint>();
    Ts2ss* ts2ss = _memHandle.create<Ts2ss>();
    BagValidationOpt opt(*bagTvl, *checkedPoint, *ts2ss, false, nullptr);
    return _memHandle.insert(new BaggageAllowanceValidator(opt));
  }

  void initFrqFlyerFatePath(const CarrierCode& cxr, uint16_t status)
  {
    PaxType::FreqFlyerTierWithCarrier* ffData =
        _memHandle.create<PaxType::FreqFlyerTierWithCarrier>();
    ffData->setCxr(cxr);
    ffData->setFreqFlyerTierLevel(status);

    _farePath->paxType()->freqFlyerTierWithCarrier() = {ffData};
  }

  PaxTypeFare* createFare(const CarrierCode& carrier = "YY")
  {
    FareInfo* info = _memHandle.create<FareInfo>();
    info->carrier() = carrier;

    TariffCrossRefInfo* tcrInfo = _memHandle.create<TariffCrossRefInfo>();
    tcrInfo->ruleTariff() = 0;

    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(info);
    fare->setTariffCrossRefInfo(tcrInfo);

    FareMarket* market = _memHandle.create<FareMarket>();

    FareClassAppInfo* fcaInfo = _memHandle.create<FareClassAppInfo>();
    fcaInfo->_ruleTariff = 0;

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    ptf->setFare(fare);
    ptf->fareMarket() = market;
    ptf->fareClassAppInfo() = fcaInfo;

    return ptf;
  }

  TestMemHandle _memHandle;
  DataHandleMock* _mdh;
  PricingTrx* _trx;
  FarePath* _farePath;
  BaggageAllowanceValidator* _validator;

public:
  // TESTS
  void testGetTvlDepartureDOW()
  {
    AirSegBuilder(&_memHandle).withDepartureDT(DateTime(2012, 2, 1)).buildVec(_validator->_segI,
                                                                              _validator->_segIE);

    CPPUNIT_ASSERT_EQUAL((uint16_t)3, _validator->getTvlDepartureDOW());
  }

  void testGetTvlDepartureDOW_Zero()
  {
    AirSegBuilder(&_memHandle).withDepartureDT(DateTime(2012, 2, 5)).buildVec(_validator->_segI,
                                                                              _validator->_segIE);

    CPPUNIT_ASSERT_EQUAL((uint16_t)7, _validator->getTvlDepartureDOW());
  }

  void testCheckTravelDate_True_StartSpecified()
  {
    AirSegBuilder(&_memHandle).withDepartureDT(DateTime(2012, 2, 1)).buildVec(_validator->_segI,
                                                                              _validator->_segIE);

    CPPUNIT_ASSERT(_validator->checkTravelDate(
        S7Builder(&_memHandle).withStartDate(2012, 1, 1).withDiscDate(2012, 3, 1).buildRef()));
  }

  void testCheckTravelDate_True_StopSpecified()
  {
    AirSegBuilder(&_memHandle).withDepartureDT(DateTime(2012, 2, 1)).buildVec(_validator->_segI,
                                                                              _validator->_segIE);

    CPPUNIT_ASSERT(_validator->checkTravelDate(
        S7Builder(&_memHandle).withEffDate(2012, 1, 1).withStopDate(2012, 3, 1).buildRef()));
  }

  void testCheckTravelDate_True_BothSpecified()
  {
    AirSegBuilder(&_memHandle).withDepartureDT(DateTime(2012, 2, 1)).buildVec(_validator->_segI,
                                                                              _validator->_segIE);

    CPPUNIT_ASSERT(_validator->checkTravelDate(
        S7Builder(&_memHandle).withStartDate(2012, 1, 1).withStopDate(2012, 3, 1).buildRef()));
  }

  void testCheckTravelDate_True_NoneSpecified()
  {
    AirSegBuilder(&_memHandle).withDepartureDT(DateTime(2012, 2, 1)).buildVec(_validator->_segI,
                                                                              _validator->_segIE);

    CPPUNIT_ASSERT(_validator->checkTravelDate(
        S7Builder(&_memHandle).withEffDate(2012, 1, 1).withDiscDate(2012, 3, 1).buildRef()));
  }

  void testCheckTravelDate_False_StartSpecified()
  {
    AirSegBuilder(&_memHandle).withDepartureDT(DateTime(2012, 4, 1)).buildVec(_validator->_segI,
                                                                              _validator->_segIE);

    CPPUNIT_ASSERT(!_validator->checkTravelDate(
        S7Builder(&_memHandle).withStartDate(2012, 1, 1).withDiscDate(2012, 3, 1).buildRef()));
  }

  void testCheckTravelDate_False_StopSpecified()
  {
    AirSegBuilder(&_memHandle).withDepartureDT(DateTime(2012, 4, 1)).buildVec(_validator->_segI,
                                                                              _validator->_segIE);

    CPPUNIT_ASSERT(!_validator->checkTravelDate(
        S7Builder(&_memHandle).withEffDate(2012, 1, 1).withStopDate(2012, 3, 1).buildRef()));
  }

  void testCheckTravelDate_False_BothSpecified()
  {
    AirSegBuilder(&_memHandle).withDepartureDT(DateTime(2012, 4, 1)).buildVec(_validator->_segI,
                                                                              _validator->_segIE);

    CPPUNIT_ASSERT(!_validator->checkTravelDate(
        S7Builder(&_memHandle).withStartDate(2012, 1, 1).withStopDate(2012, 3, 1).buildRef()));
  }

  void testCheckTravelDate_False_NoneSpecified_After()
  {
    AirSegBuilder(&_memHandle).withDepartureDT(DateTime(2012, 4, 1)).buildVec(_validator->_segI,
                                                                              _validator->_segIE);

    CPPUNIT_ASSERT(!_validator->checkTravelDate(
        S7Builder(&_memHandle).withEffDate(2012, 1, 1).withDiscDate(2012, 3, 1).buildRef()));
  }

  void testCheckTravelDate_False_NoneSpecified_Before()
  {
    AirSegBuilder(&_memHandle).withDepartureDT(DateTime(2011, 12, 1)).buildVec(_validator->_segI,
                                                                               _validator->_segIE);

    CPPUNIT_ASSERT(!_validator->checkTravelDate(
        S7Builder(&_memHandle).withEffDate(2012, 1, 1).withDiscDate(2012, 3, 1).buildRef()));
  }

  void testCheckFrequentFlyerStatus_True()
  {
    OCFees fees;

    CPPUNIT_ASSERT(_validator->checkFrequentFlyerStatus(
        S7Builder(&_memHandle).withStatus(0).buildRef(), fees));
  }

  void testCheckFrequentFlyerStatus_False()
  {
    OCFees ocFees;

    CPPUNIT_ASSERT(!_validator->checkFrequentFlyerStatus(
        S7Builder(&_memHandle).withStatus(1).buildRef(), ocFees));
  }

  void testCheckFrequentFlyerStatus_Fail_NoData()
  {
    OCFees ocFees;
    OptionalServicesInfo s7;
    PaxType paxType;

    CPPUNIT_ASSERT(!_validator->checkFrequentFlyerStatusImpl(s7, ocFees));
  }

  void testCheckFrequentFlyerStatus_Fail_Carrier()
  {
    OCFees ocFees;
    initFrqFlyerFatePath("LH", 0);

    CPPUNIT_ASSERT(!_validator->checkFrequentFlyerStatusImpl(
        S7Builder(&_memHandle).withCarrier("AA").buildRef(), ocFees));
  }

  void testCheckFrequentFlyerStatus_Fail_Status()
  {
    OCFees ocFees;
    initFrqFlyerFatePath("AA", 2);

    CPPUNIT_ASSERT(!_validator->checkFrequentFlyerStatusImpl(
        S7Builder(&_memHandle).withCarrier("AA").withStatus(1).buildRef(), ocFees));
  }

  void testCheckFrequentFlyerStatus_Pass()
  {
    OCFees ocFees;
    initFrqFlyerFatePath("AA", 1);

    CPPUNIT_ASSERT(_validator->checkFrequentFlyerStatusImpl(
        S7Builder(&_memHandle).withCarrier("AA").withStatus(2).buildRef(), ocFees));
  }

  void testCheckFrequentFlyerStatus_Softpass()
  {
    OCFees ocFees;
    OptionalServicesInfo s7;

    CPPUNIT_ASSERT(createValidator(true, false)->checkFrequentFlyerStatusImpl(s7, ocFees));
    CPPUNIT_ASSERT(ocFees.softMatchS7Status().isSet(OCFees::S7_FREQFLYER_SOFT));
  }

  void initGetDeferTargetCxr(const bool usDot, const bool iata302 = true)
  {
    _validator->_isUsDot = usDot;
    _validator->_trx.ticketingDate() = iata302 ? DateTime(2100, 1, 1) : DateTime(2000, 1, 1);

    TravelSegPtrVec& segs = _validator->_baggageTravel.itin()->travelSeg();
    AirSeg* mss = _memHandle.create<AirSeg>();
    mss->setMarketingCarrierCode("MM"); // stands for Mss Marketing
    mss->setOperatingCarrierCode("MO");
    AirSeg* cxrTs = _memHandle.create<AirSeg>();
    cxrTs->setMarketingCarrierCode("CM");
    cxrTs->setOperatingCarrierCode("CO");
    segs += mss, cxrTs;

    BaggageTravel& bt = const_cast<BaggageTravel&>(_validator->_baggageTravel);
    bt._MSSJourney = segs.begin();
    bt._carrierTravelSeg = cxrTs;
  }

  void initDeferTargetCxr(const bool usDot, const CarrierCode cxr)
  {
    _validator->_isUsDot = usDot;
    _validator->_trx.ticketingDate() = DateTime(2100, 1, 1);
    _validator->_deferTargetCxr = cxr;
  }

  void testCheckServiceNotAvailNoCharge_Free()
  {
    OCFees fees;
    OptionalServicesInfo info;
    info.notAvailNoChargeInd() = OptionalServicesValidator::SERVICE_FREE_NO_EMD_ISSUED;
    CPPUNIT_ASSERT_EQUAL(PASS_S7, _validator->checkServiceNotAvailNoCharge(info, fees));
  }

  void testCheckServiceNotAvailNoCharge_NoDefer()
  {
    initDeferTargetCxr(true, "DC");

    OCFees fees;
    OptionalServicesInfo info;
    info.carrier() = "AA";
    info.notAvailNoChargeInd() = OptionalServicesValidator::SERVICE_NOT_AVAILABLE;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_NOT_AVAIL_NO_CHANGE,
                         _validator->checkServiceNotAvailNoCharge(info, fees));
  }

  void testCheckServiceNotAvailNoCharge_Defer_BTA()
  {
    initDeferTargetCxr(true, "DC");

    OCFees fees;
    OptionalServicesInfo info;
    info.carrier() = "AA";
    info.notAvailNoChargeInd() = BaggageAllowanceValidator::DEFER_BAGGAGE_RULES_FOR_MC;
    info.baggageTravelApplication() = 'A';
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_NOT_AVAIL_NO_CHANGE,
                         _validator->checkServiceNotAvailNoCharge(info, fees));
  }

  void testCheckServiceNotAvailNoCharge_Defer_NotAllowed()
  {
    initDeferTargetCxr(true, "");

    OCFees fees;
    OptionalServicesInfo info;
    info.carrier() = "AA";
    info.notAvailNoChargeInd() = BaggageAllowanceValidator::DEFER_BAGGAGE_RULES_FOR_MC;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_DEFER_BAGGAGE_RULE,
                         _validator->checkServiceNotAvailNoCharge(info, fees));
  }

  void testCheckServiceNotAvailNoCharge_Defer_SameCxr()
  {
    initDeferTargetCxr(true, "AA");

    OCFees fees;
    OptionalServicesInfo info;
    info.carrier() = "AA";
    info.notAvailNoChargeInd() = BaggageAllowanceValidator::DEFER_BAGGAGE_RULES_FOR_MC;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_DEFER_BAGGAGE_RULE,
                         _validator->checkServiceNotAvailNoCharge(info, fees));
  }

  void testCheckServiceNotAvailNoCharge_Defer_O_UsDot()
  {
    initDeferTargetCxr(true, "DC");

    OCFees fees;
    OptionalServicesInfo info;
    info.carrier() = "AA";
    info.notAvailNoChargeInd() = BaggageAllowanceValidator::DEFER_BAGGAGE_RULES_FOR_OPC;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_DEFER_BAGGAGE_RULE,
                         _validator->checkServiceNotAvailNoCharge(info, fees));
  }

  void testCheckServiceNotAvailNoCharge_Defer_D_NonUsDot()
  {
    initDeferTargetCxr(false, "DC");

    OCFees fees;
    OptionalServicesInfo info;
    info.carrier() = "AA";
    info.notAvailNoChargeInd() = BaggageAllowanceValidator::DEFER_BAGGAGE_RULES_FOR_MC;
    CPPUNIT_ASSERT_EQUAL(FAIL_S7_DEFER_BAGGAGE_RULE,
                         _validator->checkServiceNotAvailNoCharge(info, fees));
  }

  void testCheckServiceNotAvailNoCharge_Defer_Pass()
  {
    initDeferTargetCxr(true, "DC");

    OCFees fees;
    OptionalServicesInfo info;
    info.carrier() = "AA";
    info.notAvailNoChargeInd() = BaggageAllowanceValidator::DEFER_BAGGAGE_RULES_FOR_MC;
    CPPUNIT_ASSERT_EQUAL(PASS_S7_DEFER,
                         _validator->checkServiceNotAvailNoCharge(info, fees));
  }

  void testValidateStartAndStopTime_True_Empty()
  {
    AirSegBuilder(&_memHandle).withPssDepartureTime("").buildVec(_validator->_segI,
                                                                 _validator->_segIE);

    CPPUNIT_ASSERT(_validator->validateStartAndStopTime(
        S7Builder(&_memHandle).withStartTime(123).withStopTime(345).buildRef()));
  }

  void testValidateStartAndStopTime_True_Zeros()
  {
    AirSegBuilder(&_memHandle).withPssDepartureTime("123").buildVec(_validator->_segI,
                                                                    _validator->_segIE);

    CPPUNIT_ASSERT(_validator->validateStartAndStopTime(
        S7Builder(&_memHandle).withStartTime(0).withStopTime(0).buildRef()));
  }

  void testValidateStartAndStopTime_True()
  {
    AirSegBuilder(&_memHandle).withPssDepartureTime("5").buildVec(_validator->_segI,
                                                                  _validator->_segIE);

    CPPUNIT_ASSERT(_validator->validateStartAndStopTime(
        S7Builder(&_memHandle).withStartTime(4).withStopTime(6).buildRef()));
  }

  void testValidateStartAndStopTime_False_More()
  {
    AirSegBuilder(&_memHandle).withPssDepartureTime("7").buildVec(_validator->_segI,
                                                                  _validator->_segIE);

    CPPUNIT_ASSERT(!_validator->validateStartAndStopTime(
        S7Builder(&_memHandle).withStartTime(4).withStopTime(6).buildRef()));
  }

  void testValidateStartAndStopTime_False_Less()
  {
    AirSegBuilder(&_memHandle).withPssDepartureTime("3").buildVec(_validator->_segI,
                                                                  _validator->_segIE);

    CPPUNIT_ASSERT(!_validator->validateStartAndStopTime(
        S7Builder(&_memHandle).withStartTime(4).withStopTime(6).buildRef()));
  }

  void testValidateStartAndStopTime_False_Dow()
  {
    AirSegBuilder(&_memHandle)
        .withDepartureDT(DateTime(2012, 2, 1))
        .withPssDepartureDate("2012-02-01")
        .buildVec(_validator->_segI, _validator->_segIE);

    CPPUNIT_ASSERT(
        !_validator->validateStartAndStopTime(S7Builder(&_memHandle).withDow("12").buildRef()));
  }

  void testValidateStartAndStopTimeRange_False_Dow()
  {
    AirSegBuilder(&_memHandle)
        .withDepartureDT(DateTime(2012, 2, 1))
        .withPssDepartureDate("2012-02-01")
        .buildVec(_validator->_segI, _validator->_segIE);

    CPPUNIT_ASSERT(
        !_validator->validateStartAndStopTime(S7Builder(&_memHandle).withDow("12").buildRef()));
  }

  void testValidateStartAndStopTimeRange_True_OpenWithoutDate()
  {
    AirSegBuilder(&_memHandle).setType(Open).withPssDepartureDate("").buildVec(_validator->_segI,
                                                                               _validator->_segIE);

    CPPUNIT_ASSERT(_validator->validateStartAndStopTime(S7Builder(&_memHandle).buildRef()));
  }

  void testValidateStartAndStopTimeRange_True_StartDowEqual_NoStartTime()
  {
    AirSegBuilder(&_memHandle)
        .setType(Air)
        .withDepartureDT(DateTime(2012, 02, 01))
        .withPssDepartureTime("5")
        .buildVec(_validator->_segI, _validator->_segIE);

    CPPUNIT_ASSERT(_validator->validateStartAndStopTimeRange(
        S7Builder(&_memHandle).withDow("3").withStartTime(0).buildRef()));
  }

  void testValidateStartAndStopTimeRange_True_StartDowEqual()
  {
    AirSegBuilder(&_memHandle)
        .setType(Air)
        .withDepartureDT(DateTime(2012, 02, 01))
        .withPssDepartureTime("5")
        .buildVec(_validator->_segI, _validator->_segIE);

    CPPUNIT_ASSERT(_validator->validateStartAndStopTimeRange(
        S7Builder(&_memHandle).withDow("3").withStartTime(4).buildRef()));
  }

  void testValidateStartAndStopTimeRange_False_StartDowEqual()
  {
    AirSegBuilder(&_memHandle)
        .setType(Air)
        .withDepartureDT(DateTime(2012, 02, 01))
        .withPssDepartureTime("5")
        .buildVec(_validator->_segI, _validator->_segIE);

    CPPUNIT_ASSERT(!_validator->validateStartAndStopTimeRange(
        S7Builder(&_memHandle).withDow("3").withStartTime(6).buildRef()));
  }

  void testValidateStartAndStopTimeRange_True_StopDowEqual_NoStopTime()
  {
    AirSegBuilder(&_memHandle)
        .setType(Air)
        .withDepartureDT(DateTime(2012, 02, 01))
        .withPssDepartureTime("5")
        .buildVec(_validator->_segI, _validator->_segIE);

    CPPUNIT_ASSERT(_validator->validateStartAndStopTimeRange(
        S7Builder(&_memHandle).withDow("23").withStopTime(0).buildRef()));
  }

  void testValidateStartAndStopTimeRange_True_StopDowEqual()
  {
    AirSegBuilder(&_memHandle)
        .setType(Air)
        .withDepartureDT(DateTime(2012, 02, 01))
        .withPssDepartureTime("5")
        .buildVec(_validator->_segI, _validator->_segIE);

    CPPUNIT_ASSERT(_validator->validateStartAndStopTimeRange(
        S7Builder(&_memHandle).withDow("23").withStopTime(6).buildRef()));
  }

  void testValidateStartAndStopTimeRange_False_StopDowEqual()
  {
    AirSegBuilder(&_memHandle)
        .setType(Air)
        .withDepartureDT(DateTime(2012, 02, 01))
        .withPssDepartureTime("5")
        .buildVec(_validator->_segI, _validator->_segIE);

    CPPUNIT_ASSERT(!_validator->validateStartAndStopTimeRange(
        S7Builder(&_memHandle).withDow("23").withStopTime(4).buildRef()));
  }

  void testValidateStartAndStopTimeRange_True_DepartureDowInBetween()
  {
    AirSegBuilder(&_memHandle)
        .setType(Air)
        .withDepartureDT(DateTime(2012, 02, 01))
        .withPssDepartureTime("5")
        .buildVec(_validator->_segI, _validator->_segIE);

    CPPUNIT_ASSERT(_validator->validateStartAndStopTimeRange(
        S7Builder(&_memHandle).withDow("234").withStopTime(4).buildRef()));
  }

  void testCheckEquipmentType_True_NoPnr()
  {
    OptionalServicesInfo s7;
    OCFees ocFees;
    _trx->noPNRPricing() = true;

    CPPUNIT_ASSERT(_validator->checkEquipmentType(S7Builder(&_memHandle).buildRef(), ocFees));
  }

  void testCheckEquipmentType_True_EqCodeEmpty()
  {
    OCFees ocFees;

    CPPUNIT_ASSERT(_validator->checkEquipmentType(
        S7Builder(&_memHandle).withEquipment("").buildRef(), ocFees));
  }

  void testCheckEquipmentType_True_EqCodeBlank()
  {
    OCFees ocFees;

    CPPUNIT_ASSERT(_validator->checkEquipmentType(
        S7Builder(&_memHandle).withEquipment(static_cast<EquipmentType>(BLANK)).buildRef(),
        ocFees));
  }

  void testCheckEquipmentType_True_OpenSegment()
  {
    AirSegBuilder(&_memHandle).setType(Open).buildVec(_validator->_segI, _validator->_segIE);

    OCFees ocFees;

    CPPUNIT_ASSERT(_validator->checkEquipmentType(
        S7Builder(&_memHandle).withEquipment("ABC").buildRef(), ocFees));
  }

  void testCheckEquipmentType_True_MatchingEqType()
  {
    AirSegBuilder(&_memHandle).setType(Air).withEquipmentType("ABC").buildVec(_validator->_segI,
                                                                              _validator->_segIE);

    OCFees ocFees;

    CPPUNIT_ASSERT(_validator->checkEquipmentType(
        S7Builder(&_memHandle).withEquipment("ABC").buildRef(), ocFees));
  }

  void testCheckEquipmentType_False_NonMatchingEqType()
  {
    AirSegBuilder(&_memHandle).setType(Air).withEquipmentType("ABC").buildVec(_validator->_segI,
                                                                              _validator->_segIE);

    OCFees ocFees;

    CPPUNIT_ASSERT(!_validator->checkEquipmentType(
        S7Builder(&_memHandle).withEquipment("DEF").buildRef(), ocFees));
  }

  void testCheckEquipmentType_False_EmptyEqType()
  {
    AirSegBuilder(&_memHandle).setType(Air).withEquipmentType("").buildVec(_validator->_segI,
                                                                           _validator->_segIE);

    OCFees ocFees;

    _validator->_allowSoftMatch = false;

    CPPUNIT_ASSERT(!_validator->checkEquipmentType(
        S7Builder(&_memHandle).withEquipment("DEF").buildRef(), ocFees));
  }

  void testCheckEquipmentType_True_Softmatch()
  {
    AirSegBuilder(&_memHandle).setType(Air).withEquipmentType("").buildVec(_validator->_segI,
                                                                           _validator->_segIE);

    OCFees ocFees;

    _validator->_allowSoftMatch = true;

    CPPUNIT_ASSERT(_validator->checkEquipmentType(
        S7Builder(&_memHandle).withEquipment("DEF").buildRef(), ocFees));
    CPPUNIT_ASSERT(ocFees.softMatchS7Status().isSet(OCFees::S7_EQUIPMENT_SOFT));
  }

  void testCheckEquipmentTypeHiddenStops_True_MatchingEqType()
  {
    AirSegBuilder(&_memHandle)
        .setType(Air)
        .withHiddenStop("WAW")
        .withEquipmentTypes("ABC")
        .buildVec(_validator->_segI, _validator->_segIE);

    OCFees ocFees;

    CPPUNIT_ASSERT(_validator->checkEquipmentType(
        S7Builder(&_memHandle).withEquipment("ABC").buildRef(), ocFees));
  }

  void testCheckEquipmentTypeHiddenStops_False_NonMatchingEqType()
  {
    AirSegBuilder(&_memHandle)
        .setType(Air)
        .withHiddenStop("WAW")
        .withEquipmentTypes("EFG")
        .buildVec(_validator->_segI, _validator->_segIE);

    OCFees ocFees;

    CPPUNIT_ASSERT(!_validator->checkEquipmentType(
        S7Builder(&_memHandle).withEquipment("ABC").buildRef(), ocFees));
  }

  void testCheckEquipmentTypeHiddenStops_False_EmptyEqType()
  {
    AirSegBuilder(&_memHandle).setType(Air).withHiddenStop("WAW").buildVec(_validator->_segI,
                                                                           _validator->_segIE);

    OCFees ocFees;

    _validator->_allowSoftMatch = false;

    CPPUNIT_ASSERT(!_validator->checkEquipmentType(
        S7Builder(&_memHandle).withEquipment("DEF").buildRef(), ocFees));
  }

  void testCheckEquipmentTypeHiddenStops_True_Softmatch()
  {
    AirSegBuilder(&_memHandle).setType(Air).withHiddenStop("WAW").buildVec(_validator->_segI,
                                                                           _validator->_segIE);

    OCFees ocFees;

    _validator->_allowSoftMatch = true;

    CPPUNIT_ASSERT(_validator->checkEquipmentType(
        S7Builder(&_memHandle).withEquipment("DEF").buildRef(), ocFees));
    CPPUNIT_ASSERT(ocFees.softMatchS7Status().isSet(OCFees::S7_EQUIPMENT_SOFT));
  }

  void testCheckTourCode_True_Empty()
  {
    CPPUNIT_ASSERT(_validator->checkTourCode(S7Builder(&_memHandle).withTour("").buildRef()));
  }

  void testCheckTourCode_True_Cat35Match()
  {
    CollectedNegFareData nfd;
    nfd.indicatorCat35() = true;
    nfd.tourCode() = "ABC";
    _farePath->collectedNegFareData() = &nfd;

    CPPUNIT_ASSERT(_validator->checkTourCode(S7Builder(&_memHandle).withTour("ABC").buildRef()));
  }

  void testCheckTourCode_True_Cat27Match()
  {
    _farePath->cat27TourCode() = "ABC";

    CPPUNIT_ASSERT(_validator->checkTourCode(S7Builder(&_memHandle).withTour("ABC").buildRef()));
  }

  void testCheckTourCode_False_Cat35Match()
  {
    CollectedNegFareData nfd;
    nfd.indicatorCat35() = true;
    nfd.tourCode() = "DEF";
    _farePath->collectedNegFareData() = &nfd;

    CPPUNIT_ASSERT(!_validator->checkTourCode(S7Builder(&_memHandle).withTour("ABC").buildRef()));
  }

  void testCheckTourCode_False_Cat27Match()
  {
    _farePath->cat27TourCode() = "DEF";

    CPPUNIT_ASSERT(!_validator->checkTourCode(S7Builder(&_memHandle).withTour("ABC").buildRef()));
  }

  void testCheckTourCode_False_NoNegFareData()
  {
    CPPUNIT_ASSERT(!_validator->checkTourCode(S7Builder(&_memHandle).withTour("ABC").buildRef()));
  }

  void testCheckTourCode_False_NotCat35()
  {
    CollectedNegFareData nfd;
    nfd.indicatorCat35() = false;
    _farePath->collectedNegFareData() = &nfd;

    CPPUNIT_ASSERT(!_validator->checkTourCode(S7Builder(&_memHandle).withTour("ABC").buildRef()));
  }

  void testCheckTourCode_False_Cat35TCEmpty()
  {
    CollectedNegFareData nfd;
    nfd.indicatorCat35() = true;
    nfd.tourCode() = "";
    _farePath->collectedNegFareData() = &nfd;

    CPPUNIT_ASSERT(!_validator->checkTourCode(S7Builder(&_memHandle).withTour("ABC").buildRef()));
  }

  void testCheckStartStopTime_Blank()
  {
    bool skipDOWCheck(true);
    CPPUNIT_ASSERT(_validator->checkStartStopTime(
        S7Builder(&_memHandle)
            .withTimeApplication(OptionalServicesValidator::CHAR_BLANK)
            .buildRef(),
        skipDOWCheck));
    CPPUNIT_ASSERT(!skipDOWCheck);
  }

  void testCheckStartStopTime_Blank2()
  {
    bool skipDOWCheck(true);
    CPPUNIT_ASSERT(_validator->checkStartStopTime(
        S7Builder(&_memHandle)
            .withTimeApplication(OptionalServicesValidator::CHAR_BLANK2)
            .buildRef(),
        skipDOWCheck));
    CPPUNIT_ASSERT(!skipDOWCheck);
  }

  void testCheckStartStopTime_Daily()
  {
    AirSegBuilder(&_memHandle)
        .withPssDepartureTime("3")
        .setType(Open)
        .withPssDepartureDate("")
        .buildVec(_validator->_segI, _validator->_segIE);

    bool skipDOWCheck(true);
    CPPUNIT_ASSERT(!_validator->checkStartStopTime(
        S7Builder(&_memHandle)
            .withTimeApplication(BaggageAllowanceValidator::TIME_DAILY)
            .withStartTime(4)
            .withStopTime(6)
            .buildRef(),
        skipDOWCheck));
    CPPUNIT_ASSERT(skipDOWCheck);
  }

  void testCheckStartStopTime_Range()
  {
    AirSegBuilder(&_memHandle)
        .withPssDepartureTime("3")
        .setType(Open)
        .withPssDepartureDate("")
        .buildVec(_validator->_segI, _validator->_segIE);

    bool skipDOWCheck(true);
    CPPUNIT_ASSERT(_validator->checkStartStopTime(
        S7Builder(&_memHandle)
            .withTimeApplication(BaggageAllowanceValidator::TIME_RANGE)
            .withStartTime(4)
            .withStopTime(6)
            .buildRef(),
        skipDOWCheck));
    CPPUNIT_ASSERT(skipDOWCheck);
  }

  void testCheckStartStopTime_Default()
  {
    bool skipDOWCheck(true);
    CPPUNIT_ASSERT(!_validator->checkStartStopTime(
        S7Builder(&_memHandle).withTimeApplication('X').buildRef(), skipDOWCheck));
    CPPUNIT_ASSERT(skipDOWCheck);
  }

  void testCheckDow_True_EmptyDow()
  {
    AirSegBuilder(&_memHandle).withPssDepartureDate("2012-02-01").buildVec(_validator->_segI,
                                                                           _validator->_segIE);

    CPPUNIT_ASSERT(_validator->checkDOW(S7Builder(&_memHandle).withDow("").buildRef()));
  }

  void testCheckDow_True()
  {
    AirSegBuilder(&_memHandle)
        .withDepartureDT(DateTime(2012, 02, 01))
        .withPssDepartureDate("2012-02-01")
        .buildVec(_validator->_segI, _validator->_segIE);

    CPPUNIT_ASSERT(_validator->checkDOW(S7Builder(&_memHandle).withDow("234").buildRef()));
  }

  void testCheckDow_False()
  {
    AirSegBuilder(&_memHandle)
        .withDepartureDT(DateTime(2012, 02, 01))
        .withPssDepartureDate("2012-02-01")
        .buildVec(_validator->_segI, _validator->_segIE);

    CPPUNIT_ASSERT(!_validator->checkDOW(S7Builder(&_memHandle).withDow("456").buildRef()));
  }

  void testIsValidCarrierFlight_FailOnMarketing()
  {
    CarrierFlightSeg t186;
    t186.marketingCarrier() = "CD";

    CPPUNIT_ASSERT_EQUAL(
        FAIL_ON_MARK_CXR,
        _validator->isValidCarrierFlight(
            AirSegBuilder(&_memHandle).withMarketingCarrier("AB").buildRef(), t186));
  }

  void testIsValidCarrierFlight_FailOnOperating_Open()
  {
    CarrierFlightSeg t186;
    t186.marketingCarrier() = "AB";
    t186.operatingCarrier() = "CD";

    CPPUNIT_ASSERT_EQUAL(FAIL_ON_OPER_CXR,
                         _validator->isValidCarrierFlight(AirSegBuilder(&_memHandle)
                                                              .withMarketingCarrier("AB")
                                                              .withOperatingCarrier("CD")
                                                              .setType(Open)
                                                              .buildRef(),
                                                          t186));
  }

  void testIsValidCarrierFlight_FailOnOperating_Air()
  {
    CarrierFlightSeg t186;
    t186.marketingCarrier() = "AB";
    t186.operatingCarrier() = "AB";

    CPPUNIT_ASSERT_EQUAL(FAIL_ON_OPER_CXR,
                         _validator->isValidCarrierFlight(AirSegBuilder(&_memHandle)
                                                              .withMarketingCarrier("AB")
                                                              .withOperatingCarrier("CD")
                                                              .setType(Air)
                                                              .buildRef(),
                                                          t186));
  }

  void testIsValidCarrierFlight_PassFlt()
  {
    CarrierFlightSeg t186;
    t186.marketingCarrier() = "AB";
    t186.operatingCarrier() = "AB";
    t186.flt1() = -1;

    CPPUNIT_ASSERT_EQUAL(
        PASS_T186,
        _validator->isValidCarrierFlight(
            AirSegBuilder(&_memHandle).withMarketingCarrier("AB").setType(Open).buildRef(), t186));
  }

  void testIsValidCarrierFlight_PassFlt_OperatingEmpty()
  {
    CarrierFlightSeg t186;
    t186.marketingCarrier() = "AB";
    t186.operatingCarrier() = "";
    t186.flt1() = -1;

    CPPUNIT_ASSERT_EQUAL(
        PASS_T186,
        _validator->isValidCarrierFlight(
            AirSegBuilder(&_memHandle).withMarketingCarrier("AB").setType(Open).buildRef(), t186));
  }

  void testIsValidCarrierFlight_PassFltNumber()
  {
    CarrierFlightSeg t186;
    t186.marketingCarrier() = "AB";
    t186.operatingCarrier() = "AB";
    t186.flt1() = 0;

    CPPUNIT_ASSERT_EQUAL(PASS_T186,
                         _validator->isValidCarrierFlight(AirSegBuilder(&_memHandle)
                                                              .withMarketingCarrier("AB")
                                                              .withOperatingCarrier("AB")
                                                              .setType(Air)
                                                              .buildRef(),
                                                          t186));
  }

  void testIsValidCarrierFlightNumber_Open()
  {
    CarrierFlightSeg t186;

    CPPUNIT_ASSERT_EQUAL(PASS_T186,
                         _validator->isValidCarrierFlightNumber(
                             AirSegBuilder(&_memHandle).setType(Open).buildRef(), t186));
  }

  void testIsValidCarrierFlightNumber_Fail_1()
  {
    CarrierFlightSeg t186;
    t186.flt2() = 0;
    t186.flt1() = 456;

    CPPUNIT_ASSERT_EQUAL(
        FAIL_ON_FLIGHT,
        _validator->isValidCarrierFlightNumber(
            AirSegBuilder(&_memHandle).setType(Air).withFlightNo(123).buildRef(), t186));
  }

  void testIsValidCarrierFlightNumber_Fail_2()
  {
    CarrierFlightSeg t186;
    t186.flt2() = 456;
    t186.flt1() = 789;

    CPPUNIT_ASSERT_EQUAL(
        FAIL_ON_FLIGHT,
        _validator->isValidCarrierFlightNumber(
            AirSegBuilder(&_memHandle).setType(Air).withFlightNo(123).buildRef(), t186));
  }

  void testIsValidCarrierFlightNumber_Fail_3()
  {
    CarrierFlightSeg t186;
    t186.flt2() = 456;
    t186.flt1() = 123;

    CPPUNIT_ASSERT_EQUAL(
        FAIL_ON_FLIGHT,
        _validator->isValidCarrierFlightNumber(
            AirSegBuilder(&_memHandle).setType(Air).withFlightNo(789).buildRef(), t186));
  }

  void testIsValidCarrierFlightNumber_Pass_1()
  {
    CarrierFlightSeg t186;
    t186.flt2() = 0;
    t186.flt1() = 123;

    CPPUNIT_ASSERT_EQUAL(
        PASS_T186,
        _validator->isValidCarrierFlightNumber(
            AirSegBuilder(&_memHandle).setType(Air).withFlightNo(123).buildRef(), t186));
  }

  void testIsValidCarrierFlightNumber_Pass_2()
  {
    CarrierFlightSeg t186;
    t186.flt2() = 789;
    t186.flt1() = 123;

    CPPUNIT_ASSERT_EQUAL(
        PASS_T186,
        _validator->isValidCarrierFlightNumber(
            AirSegBuilder(&_memHandle).setType(Air).withFlightNo(456).buildRef(), t186));
  }

  void testCheckCarrierFlightApplT186_Itemno_Zero()
  {
    OptionalServicesInfo s7;
    OCFees ocFees;

    CPPUNIT_ASSERT(_validator->checkCarrierFlightApplT186(s7, 0, ocFees));
  }

  void testCheckCarrierFlightApplT186_186_Null()
  {
    OptionalServicesInfo s7;
    OCFees ocFees;

    CPPUNIT_ASSERT(!_validator->checkCarrierFlightApplT186(s7, 1, ocFees));
  }

  void testCheckCarrierFlightApplT186_186_SegCnt_Zero()
  {
    OptionalServicesInfo s7;
    OCFees ocFees;

    CPPUNIT_ASSERT(!_validator->checkCarrierFlightApplT186(s7, 2, ocFees));
  }

  void testCheckCarrierFlightApplT186_186_Segs_Empty()
  {
    OptionalServicesInfo s7;
    OCFees ocFees;

    CPPUNIT_ASSERT(!_validator->checkCarrierFlightApplT186(s7, 3, ocFees));
  }

  void testCheckCarrierFlightApplT186_Defer_Pass()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle).withMarketingCarrier("AA").build();

    const_cast<BaggageTravel&>(_validator->_baggageTravel)._MSSJourney = travelSegs.begin();
    _validator->_deferTargetCxr = "AA";

    OCFees ocFees;
    OptionalServicesInfo info;
    info.notAvailNoChargeInd() = BaggageAllowanceValidator::DEFER_BAGGAGE_RULES_FOR_MC;
    CPPUNIT_ASSERT(_validator->checkCarrierFlightApplT186(info, 10, ocFees));
  }

  void testCheckCarrierFlightApplT186_Defer_Fail()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle).withMarketingCarrier("LH").build();

    const_cast<BaggageTravel&>(_validator->_baggageTravel)._MSSJourney = travelSegs.begin();
    _validator->_deferTargetCxr = "AA";

    OCFees ocFees;
    OptionalServicesInfo info;
    info.notAvailNoChargeInd() = BaggageAllowanceValidator::DEFER_BAGGAGE_RULES_FOR_MC;
    CPPUNIT_ASSERT(!_validator->checkCarrierFlightApplT186(info, 10, ocFees));
  }

  void testCheckCarrierFlightApplT186_Free_Pass()
  {
    std::vector<TravelSeg*> travelSegs;

    travelSegs += AirSegBuilder(&_memHandle).withMarketingCarrier("AA").build();

    ArunkSeg arunkSeg;
    travelSegs += &arunkSeg;

    travelSegs += AirSegBuilder(&_memHandle).withMarketingCarrier("AA").build();

    _validator->_segI = travelSegs.begin();
    _validator->_segIE = travelSegs.end();

    OCFees ocFees;

    CPPUNIT_ASSERT(_validator->checkCarrierFlightApplT186(
        S7Builder(&_memHandle)
            .withNotAvailNoCharge(BaggageAllowanceValidator::SERVICE_FREE_NO_EMD_ISSUED)
            .buildRef(),
        10,
        ocFees));
  }

  void testCheckCarrierFlightApplT186_Free_Fail()
  {
    std::vector<TravelSeg*> travelSegs;

    travelSegs += AirSegBuilder(&_memHandle).withMarketingCarrier("AA").build();

    ArunkSeg arunkSeg;
    travelSegs += &arunkSeg;

    travelSegs += AirSegBuilder(&_memHandle).withMarketingCarrier("LH").build();

    _validator->_segI = travelSegs.begin();
    _validator->_segIE = travelSegs.end();

    OCFees ocFees;

    CPPUNIT_ASSERT(!_validator->checkCarrierFlightApplT186(
        S7Builder(&_memHandle)
            .withNotAvailNoCharge(BaggageAllowanceValidator::SERVICE_FREE_NO_EMD_ISSUED)
            .buildRef(),
        10,
        ocFees));
  }

  void testIsValidResultingFareClass_Fail_On_Cxr()
  {
    SvcFeesCxrResultingFCLInfo fclInfo;
    fclInfo.carrier() = "LT";
    PaxTypeFare* ptf = createFare("AA");
    OCFees ocFees;

    _validator->_isAncillaryBaggage = true;

    CPPUNIT_ASSERT_EQUAL(FAIL_ON_CXR, _validator->isValidResultingFareClass(ptf, fclInfo, ocFees));
  }

  void testIsValidResultingFareClass_Pass_Ancillary()
  {
    SvcFeesCxrResultingFCLInfo fclInfo;
    fclInfo.carrier() = "AA";
    PaxTypeFare* ptf = createFare("AA");
    OCFees ocFees;

    _validator->_isAncillaryBaggage = true;

    CPPUNIT_ASSERT_EQUAL(PASS_T171, _validator->isValidResultingFareClass(ptf, fclInfo, ocFees));
  }

  void testIsValidResultingFareClass_Pass()
  {
    SvcFeesCxrResultingFCLInfo fclInfo;
    fclInfo.carrier() = "LH";
    FareMarket fm;
    fm.governingCarrier() = "LH";
    PaxTypeFare* ptf = createFare("AA");
    ptf->fareMarket() = &fm;
    OCFees ocFees;

    _validator->_isAncillaryBaggage = false;

    CPPUNIT_ASSERT_EQUAL(PASS_T171, _validator->isValidResultingFareClass(ptf, fclInfo, ocFees));
  }

  void testConstructor_Normal()
  {
    BaggageAllowanceValidator* validator = createValidator(false);

    CPPUNIT_ASSERT(!validator->_isAncillaryBaggage);
  }

  void testConstructor_Ancillary()
  {
    BaggageAllowanceValidator* validator = createValidator(true);

    CPPUNIT_ASSERT(validator->_isAncillaryBaggage);
    CPPUNIT_ASSERT(validator->_allowSoftMatch);
  }

  void testConstructor_Ancillary_Hardmatch()
  {
    BaggageAllowanceValidator* validator = createValidator(true, true);

    CPPUNIT_ASSERT(validator->_isAncillaryBaggage);
    CPPUNIT_ASSERT(!validator->_allowSoftMatch);
  }

  void testCheckSecurity_Normal()
  {
    BaggageAllowanceValidator* validator = createValidator(false);

    CPPUNIT_ASSERT(!validator->checkSecurity(S7Builder(&_memHandle).withFeesSec(123).buildRef()));
  }

  void testCheckSecurity_Ancillary()
  {
    BaggageAllowanceValidator* validator = createValidator(true);

    CPPUNIT_ASSERT(!validator->checkSecurity(S7Builder(&_memHandle).withFeesSec(123).buildRef()));
  }

  void testIsValidFareClassFareType_Pass()
  {
    SvcFeesCxrResultingFCLInfo info;
    info.resultingFCL() = "ABC";
    OCFees ocFees;

    _validator->_allowSoftMatch = false;

    CPPUNIT_ASSERT_EQUAL(FAIL_ON_FARE_CLASS,
                         _validator->isValidFareClassFareType(createFare(), info, ocFees));
    CPPUNIT_ASSERT(!ocFees.softMatchS7Status().isSet(OCFees::S7_RESULTING_FARE_SOFT));
  }

  void testIsValidFareClassFareType_Softpass()
  {
    SvcFeesCxrResultingFCLInfo info;
    info.resultingFCL() = "ABC";
    OCFees ocFees;

    _validator->_allowSoftMatch = true;

    CPPUNIT_ASSERT_EQUAL(SOFTPASS_FARE_CLASS,
                         _validator->isValidFareClassFareType(createFare(), info, ocFees));
    CPPUNIT_ASSERT(ocFees.softMatchS7Status().isSet(OCFees::S7_RESULTING_FARE_SOFT));
  }

  void testMatchRuleTariff_Fail()
  {
    OCFees ocFees;
    PaxTypeFare* fare = createFare();
    _validator->_allowSoftMatch = false;

    bool result = _validator->matchRuleTariff(123, *fare, ocFees);

    CPPUNIT_ASSERT(!result);
    CPPUNIT_ASSERT(!ocFees.softMatchS7Status().isSet(OCFees::S7_RULETARIFF_SOFT));
  }

  void testMatchRuleTariff_Softpass()
  {
    OCFees ocFees;
    _validator->_allowSoftMatch = true;

    CPPUNIT_ASSERT(_validator->matchRuleTariff(123, *createFare(), ocFees));
    CPPUNIT_ASSERT(ocFees.softMatchS7Status().isSet(OCFees::S7_RULETARIFF_SOFT));
  }

  void testBuildFees()
  {
    SubCodeInfo* s5 = S5Builder(&_memHandle).build();

    OCFees* ocFees = _validator->buildFees(*s5);

    CPPUNIT_ASSERT(s5 == ocFees->subCodeInfo());
  }

  void testSupplementFees()
  {
    SubCodeInfo* s5 = S5Builder(&_memHandle).withCarrier("LT").build();
    OCFees ocFees;

    _validator->_itin.travelSeg().push_back(_memHandle.create<AirSeg>());
    _validator->_segI = _validator->_itin.travelSeg().begin();
    _validator->_segIE = _validator->_itin.travelSeg().end();

    _validator->supplementFees(*s5, &ocFees);

    CPPUNIT_ASSERT(s5 == ocFees.subCodeInfo());
    CPPUNIT_ASSERT_EQUAL(ocFees.carrierCode(), s5->carrier());
    CPPUNIT_ASSERT(ocFees.farePath() == _validator->_farePath);
    CPPUNIT_ASSERT(ocFees.travelStart() == *(_validator->_segI));
    CPPUNIT_ASSERT(ocFees.travelEnd() == *(_validator->_segIE - 1));
    CPPUNIT_ASSERT(ocFees.isFeeGuaranteed());
  }

  void testSupplementFees_NotGuaranteed()
  {
    SubCodeInfo* s5 = S5Builder(&_memHandle).build();
    OCFees ocFees;
    ocFees.optFee() = S7Builder(&_memHandle).withNotAvailNoCharge('X').build();

    _validator->_itin.travelSeg().push_back(_memHandle.create<AirSeg>());
    _validator->_segI = _validator->_itin.travelSeg().begin();
    _validator->_segIE = _validator->_itin.travelSeg().end();

    _validator->supplementFees(*s5, &ocFees);

    CPPUNIT_ASSERT(!ocFees.isFeeGuaranteed());
  }

  void test_checkCabinInSegment_noCabin_PASS()
  {
    AirSeg seg;

    Indicator cabin = BLANK;
    CPPUNIT_ASSERT(_validator->checkCabinInSegment(&seg, cabin));
  }
  void test_checkCabinInSegment_noSegment_PASS()
  {
    Indicator properCabin = 'T';

    TravelSeg* emptySegment = 0;
    CPPUNIT_ASSERT(_validator->checkCabinInSegment(emptySegment, properCabin));
  }
  void test_checkCabinInSegment_nonAirSegment_PASS()
  {
    Indicator properCabin = 'T';

    ArunkSeg arunkSeg;
    CPPUNIT_ASSERT(_validator->checkCabinInSegment(&arunkSeg, properCabin));
  }

  void test_checkRBDInSegment_noRBDItemNo_PASS()
  {
    std::vector<SvcFeesResBkgDesigInfo*> rbdInfos;
    OCFees fee;
    AirSeg seg;

    uint32_t emptyRBDitemNo = 0;
    CPPUNIT_ASSERT(_validator->checkRBDInSegment(&seg, fee, emptyRBDitemNo, rbdInfos));
  }
  void test_checkRBDInSegment_noRBDRecords_FAIL()
  {
    OCFees fee;
    AirSeg seg;
    uint32_t rbdItemNo = 1000;

    std::vector<SvcFeesResBkgDesigInfo*> emptyRbdInfos;
    CPPUNIT_ASSERT(!_validator->checkRBDInSegment(&seg, fee, rbdItemNo, emptyRbdInfos));
  }
  void test_checkRBDInSegment_noSegment_PASS()
  {
    SvcFeesResBkgDesigInfo rbdTableRecord;
    std::vector<SvcFeesResBkgDesigInfo*> rbdInfos;
    rbdInfos.push_back(&rbdTableRecord);
    OCFees fee;
    uint32_t rbdItemNo = 1000;

    CPPUNIT_ASSERT(_validator->checkRBDInSegment(0, fee, rbdItemNo, rbdInfos));
  }
  void test_checkRBDInSegment_nonAirSegment_PASS()
  {
    SvcFeesResBkgDesigInfo rbdTableRecord;
    std::vector<SvcFeesResBkgDesigInfo*> rbdInfos;
    rbdInfos.push_back(&rbdTableRecord);
    OCFees fee;
    uint32_t rbdItemNo = 1000;

    ArunkSeg arunk;
    CPPUNIT_ASSERT(_validator->checkRBDInSegment(&arunk, fee, rbdItemNo, rbdInfos));
  }

  void test_checkResultingFareClassInSegment_noT171ItemNo_PASS()
  {
    std::vector<SvcFeesCxrResultingFCLInfo*> resFCLInfo;
    PaxTypeFare fare;
    OCFees fee;

    uint32_t t171ItemNo = 0;
    CPPUNIT_ASSERT(
        _validator->checkResultingFareClassInSegment(&fare, t171ItemNo, fee, resFCLInfo));
  }
  void test_checkResultingFareClassInSegment_noPaxTypeFare_FAIL()
  {
    std::vector<SvcFeesCxrResultingFCLInfo*> resFCLInfo;
    OCFees fee;
    uint32_t t171ItemNo = 1000;

    PaxTypeFare* emptyFare = 0;
    CPPUNIT_ASSERT(
        !_validator->checkResultingFareClassInSegment(emptyFare, t171ItemNo, fee, resFCLInfo));
  }
  void test_checkResultingFareClassInSegment_emptyCarrierResultingFCLInfoRecords_FAIL()
  {
    uint32_t t171ItemNo = 1000;
    PaxTypeFare fare;
    OCFees fee;

    std::vector<SvcFeesCxrResultingFCLInfo*> emptyResFCLInfo;
    CPPUNIT_ASSERT(
        !_validator->checkResultingFareClassInSegment(&fare, t171ItemNo, fee, emptyResFCLInfo));
  }

  void test_checkOutputTicketDesignatorInSegment_empty_PASS()
  {
    OptionalServicesInfo s7;

    TravelSeg* emptySegment = 0;
    CPPUNIT_ASSERT(_validator->checkOutputTicketDesignatorInSegment(emptySegment, nullptr, s7));
  }

  void test_checkRuleInSegment_noRule_PASS()
  {
    PaxTypeFare fare;
    OCFees fee;

    RuleNumber emptyRule = "";
    CPPUNIT_ASSERT(_validator->checkRuleInSegment(&fare, emptyRule, fee));
  }
  void test_checkRuleInSegment_noPaxTypeFare_PASS()
  {
    OCFees fee;
    RuleNumber rule = "XXXX";

    PaxTypeFare* emptyFare = 0;
    CPPUNIT_ASSERT(_validator->checkRuleInSegment(emptyFare, rule, fee));
  }

  void test_checkRuleTariffInSegment_noRuleTariffNumber_PASS()
  {
    PaxTypeFare fare;
    OCFees fee;

    uint16_t emptyRuleTariffNo = (uint16_t)-1;
    CPPUNIT_ASSERT(_validator->checkRuleTariffInSegment(&fare, emptyRuleTariffNo, fee));
  }
  void test_checkRuleTariffInSegment_noPaxTypeFare_PASS()
  {
    OCFees fee;
    uint16_t ruleTariffNo = 22;

    PaxTypeFare* emptyFare = 0;
    CPPUNIT_ASSERT(_validator->checkRuleTariffInSegment(emptyFare, ruleTariffNo, fee));
  }

  void test_checkFareIndInSegment_noPaxTypeFare_PASS()
  {
    Indicator fareInd = 'X';

    PaxTypeFare* emptyFare = 0;
    CPPUNIT_ASSERT(_validator->checkFareIndInSegment(emptyFare, fareInd));
  }
  void test_checkFareIndInSegment_emptyFareInd_PASS()
  {
    PaxTypeFare fare;

    Indicator blankFareInd = BaggageAllowanceValidator::CHAR_BLANK;
    CPPUNIT_ASSERT(_validator->checkFareIndInSegment(&fare, blankFareInd));

    blankFareInd = BaggageAllowanceValidator::CHAR_BLANK2;
    CPPUNIT_ASSERT(_validator->checkFareIndInSegment(&fare, blankFareInd));
  }

  void test_checkCarrierFlightApplT186InSegment_noSegment_FAIL()
  {
    uint32_t t186ItemNo = 1000;
    VendorCode vendor = "XXXX";

    TravelSeg* emptySegment = 0;
    CPPUNIT_ASSERT(
        !_validator->checkCarrierFlightApplT186InSegment(emptySegment, vendor, t186ItemNo));
  }
  void test_checkCarrierFlightApplT186InSegment_nonAirSegment_FAIL()
  {
    uint32_t t186ItemNo = 1000;
    VendorCode vendor = "XXXX";

    ArunkSeg arunk;
    CPPUNIT_ASSERT(!_validator->checkCarrierFlightApplT186InSegment(&arunk, vendor, t186ItemNo));
  }
  void test_checkCarrierFlightApplT186InSegment_emptyT186ItemNo_PASS()
  {
    AirSeg air;
    VendorCode vendor = "XXXX";

    uint32_t emptyT186ItemNo = 0;
    CPPUNIT_ASSERT(_validator->checkCarrierFlightApplT186InSegment(&air, vendor, emptyT186ItemNo));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(BaggageAllowanceValidatorTest);
} // tse
