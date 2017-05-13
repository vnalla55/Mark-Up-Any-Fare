#include <map>
#include <vector>

#include "test/include/CppUnitHelperMacros.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/SvcFeesCxrResultingFCLInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/Diag877Collector.h"
#include "ServiceFees/OCFees.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestFallbackUtil.h"

#include "ServiceFees/AncillaryPricingValidator.h"

namespace tse
{

using namespace std;

class AncillaryPricingValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AncillaryPricingValidatorTest);

  CPPUNIT_TEST(checkRBDWhenNoItemNo);
  CPPUNIT_TEST(checkRBDWhenNoRBDRecords);
  CPPUNIT_TEST(checkRBDWhenNoRBDAndCabinInRequest);
  CPPUNIT_TEST(checkRBDWhenNoRBDinReqButHasCabinAndNoAnyValidS7Records);
  CPPUNIT_TEST(checkRBDWhenNoRBDinReqButHasCabinAndNoAnyValidS7RBD);
  CPPUNIT_TEST(checkRBDWhenNoRBDinReqAndFailOnCabinCheck);
  CPPUNIT_TEST(checkRBDWhenNoRBDinReqAndPassOnCabinCheck);
  CPPUNIT_TEST(checkRBDWhenNoRBDInReqAndCabinNotFound);
  CPPUNIT_TEST(checkRBDWhenCheckRBDAndPass);
  CPPUNIT_TEST(checkRBDWhenCheckRBDAndFail);
  CPPUNIT_TEST(testCheckCabinDataWhenNoCabinInReq);
  CPPUNIT_TEST(testCheckCabinDataWhenPass);
  CPPUNIT_TEST(testCheckCabinDataWhenFail);
  CPPUNIT_TEST(testIsMissingDeptArvlTimeWhenTimeIsPopulated);
  CPPUNIT_TEST(testIsMissingDeptArvlTimeWhenNoTime);
  CPPUNIT_TEST(testCheckIntermediatePointWhenTimeIsPopulated);
  CPPUNIT_TEST(testCheckIntermediatePointWhenNoTimeInReqAndValidStopoverUnitInS7);
  CPPUNIT_TEST(testCheckStopCnxDestIndWhenTimeIsPopulated);
  CPPUNIT_TEST(testCheckStopCnxDestIndWhenNoTimeInReqAndValidStopoverUnitInS7);
  CPPUNIT_TEST(testCheckFrequentFlyerStatus_NoFreqFlyer);
  CPPUNIT_TEST(testCheckEquipmentType_NoEquipmentS7);
  CPPUNIT_TEST(testCheckEquipmentType_NoEquipmentSeg);
  CPPUNIT_TEST(testCheckEquipmentType_PassEquipment);
  CPPUNIT_TEST(testCheckEquipmentType_FailEquipment);
  CPPUNIT_TEST(testValidate_Pass_Single_S7);
  CPPUNIT_TEST(testValidate_SoftPass_First_S7_Pass_Second_S7);
  CPPUNIT_TEST(testValidate_SoftPass_First_S7_Fail_Second_Pass_Third_S7);
  CPPUNIT_TEST(testValidate_Fail);

  CPPUNIT_TEST(test_Validate_DDPass_False_Datanotfound);
  CPPUNIT_TEST(test_Validate_DDPass_True_Datanotfound);

  CPPUNIT_TEST(testIsValidFareClassFareTypeWhenSoftPassOnFareClass_T171HasFC);
  CPPUNIT_TEST(testIsValidFareClassFareTypeWhenPassOn_T171DoesNotHaveFC_and_FareType);
  CPPUNIT_TEST(testIsValidFareClassFareTypeWhen_FAIL_On_T171HasFC);

  CPPUNIT_TEST(testValidateLocation_AirportSameLoc);
  CPPUNIT_TEST(testValidateLocation_AirportDiffCity);
  CPPUNIT_TEST(testValidateLocation_AirportMatchCity);

  CPPUNIT_TEST(testIsValidRuleTariffIndWhenRuleIsPrivateAndFareIsPubilc);
  CPPUNIT_TEST(testMatchRuleTariffWhenNoFareTariff);
  CPPUNIT_TEST(testCheckRuleTariffWhenS7RuleTariffIsEmpty);
  CPPUNIT_TEST(testCheckRuleTariffWhenNoFBI);
  CPPUNIT_TEST(testCheckRuleWhenS7RuleIsEmpty);
  CPPUNIT_TEST(testCheckRuleWhenNoFBI);
  CPPUNIT_TEST(testCheckRuleWhenRuleIsEmptyInReq);
  CPPUNIT_TEST(testCheckRuleWhenFailRule);

  CPPUNIT_TEST(testValidate_Start_And_Stop_Time_Soft_Pass);
  CPPUNIT_TEST(testValidate_Start_And_Stop_Time_Pass);
  CPPUNIT_TEST(testValidate_Start_And_Stop_Time_Fail);

  CPPUNIT_TEST(testValidate_Start_And_Stop_Time_Soft_Pass_Dup);
  CPPUNIT_TEST(testValidate_Start_And_Stop_Time_Pass_Dup);

  CPPUNIT_TEST(testIsValidCarrierFlightNumber_T186_FLT_present_PASS);
  CPPUNIT_TEST(testIsValidCarrierFlightNumber_T186_FLT_present_SOFT_PASS);
  CPPUNIT_TEST(testIsValidCarrierFlightNumber_T186_FLT_present_FAIL_ON_FLIGHT);
  CPPUNIT_TEST(checkRBDWhenNoRBDinReqAndNoCabinAndNoAnyValidS7RBD);

  CPPUNIT_TEST(testCheckSecurityReturnTRUEWhenNoSecurityCodedAndS7Public);
  CPPUNIT_TEST(testCheckSecurityReturnFALSEWhenNoSecurityCodedAndS7Private);

  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_3_D);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_13_D);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_33_D);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_63_D);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_103_D);

  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_1_M);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_2_M);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_4_M);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_11_M);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_15_M);

  CPPUNIT_TEST(testIsStopover_MissingTime);
  CPPUNIT_TEST(testIsValidFareClassFareTypeWhenSoftPassOnFareTYPE_T171HasFC_And_FareType);

  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_0_D);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_5_H);
  CPPUNIT_TEST(testSetPBDate_When_advPurchUnit_DOW);
  CPPUNIT_TEST(
      testValidate_Request_Has_Hard_Match_Only_First_S7_was_SoftPass_Before_Now_Its_Fail_And_Second_S7_Pass);

  CPPUNIT_TEST(test_skipUpgradeCheck_upgradeCabin_BLANK_upgrade198_0);
  CPPUNIT_TEST(test_skipUpgradeCheck_upgradeCabin_NOT_BLANK_upgrade198_0);
  CPPUNIT_TEST(test_skipUpgradeCheck_upgradeCabin_BLANK_upgrade198_NOT_0);
  CPPUNIT_TEST(test_skipUpgradeCheck_upgradeCabin_NOT_BLANK_upgrade198_NOT_0);

  CPPUNIT_TEST(test_skipUpgradeCheck__fail_normally_hardMatched_s7_for_ACS_client);

  CPPUNIT_TEST(testValidate_s7_record_with_SA_service_group__BADDATA_scenario);
  CPPUNIT_TEST(
      testValidate_s7_records_sequence_with_SA_service_group__M70_request_hard_match_indicator_false);
  CPPUNIT_TEST(
      testValidate_s7_records_sequence_with_SA_service_group__M70_request_hard_match_indicator_true);

  CPPUNIT_TEST(testGetCabinWhenPass);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _farePath = _memHandle.create<FarePath>();
    _paxType = _memHandle.create<PaxType>();
    _farePath->paxType() = _paxType;
    _farePath->itin() = _memHandle.create<Itin>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _seg1 = createAirSeg("AA", "B", CabinType::BUSINESS_CLASS);
    _seg2 = createAirSeg("UA", "Y", CabinType::ECONOMY_CLASS);
    _seg3 = createAirSeg("UA", "A", CabinType::FIRST_CLASS);
    _trx->setRequest(_memHandle.create<AncRequest>());
    _trx->getRequest()->ticketingDT() = DateTime(2010, 1, 18);
    _ocFees = _memHandle.create<OCFees>();
    _subCodeInfo = _memHandle.create<SubCodeInfo>();
    _ocFees->subCodeInfo() = _subCodeInfo;

    _tvlSegs.push_back(_seg1);
    _tvlSegs.push_back(_seg2);
    _tvlSegs.push_back(_seg3);
    _endOfJourney = _tvlSegs.end();
    _farePath->itin()->travelSeg() = _tvlSegs;
    _diag = _memHandle.create<Diag877Collector>();
    createValidator(true, true, true);
    createMockValidator(true, true, true);
    _memHandle.insert(new MyDataHandle(_memHandle, _rbdInfo1, _rbdInfo2));
    setRBDInfo(_rbdInfo1, "UA", "A", "Y");
    setRBDInfo(_rbdInfo2, "AA", "B", "");
    _vendor = "SABR";
    _seg1->departureDT() = DateTime(2011, 1, 18);
    _seg1->arrivalDT() = DateTime(2011, 1, 18, 3, 0, 0);
    _seg2->departureDT() = DateTime(2011, 1, 18);
    _seg2->arrivalDT() = DateTime(2011, 1, 19, 3, 0, 0);
    _seg3->departureDT() = DateTime(2011, 1, 20);
    _seg3->arrivalDT() = DateTime(2011, 1, 20, 3, 0, 0);

    _seg1->origin() = TestLocFactory::create(LOC_NYCU);
    _seg1->destination() = TestLocFactory::create(LOC_YMQC);
    _seg2->origin() = _seg1->destination();
    _seg2->destination() = TestLocFactory::create(LOC_NRTJ);
    _seg3->origin() = _seg2->destination();
    _seg3->destination() = TestLocFactory::create(LOC_SINS);

    _seg1->segmentOrder() = 1;
    _seg2->segmentOrder() = 2;
    _seg3->segmentOrder() = 3;

    _trx->billing() = _memHandle.create<Billing>();
    _trx->billing()->requestPath() = UNKNOWN_PATH;
  }

  void tearDown() { _memHandle.clear(); }

  void createValidator(bool isInterational, bool isOneCarrier, bool isMarketingCarrier)
  {
    OcValidationContext ctx(*_trx, *_farePath->itin(), *_farePath->paxType(), _farePath);
    _validator = _memHandle.insert(new AncillaryPricingValidator(ctx,
                                                                 _tvlSegs.begin(),
                                                                 _tvlSegs.end(),
                                                                 _endOfJourney,
                                                                 _ts2ss,
                                                                 isInterational,
                                                                 isOneCarrier,
                                                                 isMarketingCarrier,
                                                                 _diag));
  }

  void createMockValidator(bool isInterational, bool isOneCarrier, bool isMarketingCarrier)
  {
    OcValidationContext ctx(*_trx, *_farePath->itin(), *_farePath->paxType(), _farePath);
    _mockValidator = _memHandle.insert(new AncillaryPricingValidatorMock(_memHandle,
                                                                         ctx,
                                                                         _tvlSegs.begin(),
                                                                         _tvlSegs.end(),
                                                                         _endOfJourney,
                                                                         _ts2ss,
                                                                         isInterational,
                                                                         isOneCarrier,
                                                                         isMarketingCarrier,
                                                                         _diag));
  }

  AirSeg* createAirSeg(CarrierCode cxr, BookingCode bkg, CabinType::CabinTypeNew cabin)
  {
    AirSeg* airSeg = _memHandle.create<AirSeg>();
    airSeg->carrier() = cxr;
    airSeg->setOperatingCarrierCode(cxr);
    airSeg->setBookingCode(bkg);
    airSeg->bookedCabin().setClass(cabin);
    return airSeg;
  }

  void
  setRBDInfo(SvcFeesResBkgDesigInfo& rbdInfo, CarrierCode cxr, BookingCode bkg1, BookingCode bkg2)
  {
    rbdInfo.carrier() = cxr;
    rbdInfo.bookingCode1() = bkg1;
    rbdInfo.bookingCode2() = bkg2;
  }

  void createS7(OptionalServicesInfo& optSvcInfo1)
  {
    optSvcInfo1.createDate() = DateTime(2010, 1, 18);
    optSvcInfo1.effDate() = DateTime(2010, 1, 18);
    optSvcInfo1.ticketEffDate() = DateTime(2010, 1, 18);
    optSvcInfo1.sectorPortionInd() = OptionalServicesValidator::SEC_POR_IND_PORTION;
    optSvcInfo1.stopoverUnit() = OptionalServicesValidator::CHAR_BLANK;
    optSvcInfo1.fromToWithinInd() = OptionalServicesValidator::FTW_FROM;
    optSvcInfo1.loc1().loc() = LOC_NYC;
    optSvcInfo1.loc1().locType() = LOCTYPE_CITY;
    optSvcInfo1.expireDate() = DateTime(2011, 1, 19);
    optSvcInfo1.discDate() = DateTime(2011, 1, 19);
    optSvcInfo1.ticketDiscDate() = DateTime(2011, 1, 19);
  }

  void createDiagnosticDDPassed(bool isDDPass)
  {
    _mockValidator->_diag =
        _memHandle.insert(new Diag877Collector(*_memHandle.create<Diagnostic>()));
    _mockValidator->_diag->activate();

    if (isDDPass)
      _trx->diagnostic().diagParamMap().insert(
          std::make_pair(Diagnostic::DISPLAY_DETAIL, "PASSED"));
  }

  void prepare_s7_records_sequence_for_M70_request_test_against_padis_codes_project()
  {
    uint32_t badData_t198RecordItemNo = 1;
    uint32_t existing_t198RecordItemNo = 2;

    OptionalServicesInfoTest* s7_1_softMatch = _memHandle.insert(new OptionalServicesInfoTest(1));
    createS7(*s7_1_softMatch);
    s7_1_softMatch->frequentFlyerStatus() = 3;
    s7_1_softMatch->upgrdServiceFeesResBkgDesigTblItemNo() = existing_t198RecordItemNo;

    OptionalServicesInfoTest* s7_2_hardMatch_withPadis =
        _memHandle.insert(new OptionalServicesInfoTest(2));
    createS7(*s7_2_hardMatch_withPadis);
    s7_2_hardMatch_withPadis->upgrdServiceFeesResBkgDesigTblItemNo() = existing_t198RecordItemNo;

    OptionalServicesInfoTest* s7_3_noMatch = _memHandle.insert(new OptionalServicesInfoTest(3));
    createS7(*s7_3_noMatch);
    s7_3_noMatch->upgrdServiceFeesResBkgDesigTblItemNo() = badData_t198RecordItemNo;

    OptionalServicesInfoTest* s7_4_hardMatch_withPadis =
        _memHandle.insert(new OptionalServicesInfoTest(4));
    createS7(*s7_4_hardMatch_withPadis);
    s7_4_hardMatch_withPadis->upgrdServiceFeesResBkgDesigTblItemNo() = existing_t198RecordItemNo;

    OptionalServicesInfoTest* s7_5_noMatch = _memHandle.insert(new OptionalServicesInfoTest(5));
    createS7(*s7_5_noMatch);
    s7_5_noMatch->upgrdServiceFeesResBkgDesigTblItemNo() = badData_t198RecordItemNo;

    OptionalServicesInfoTest* s7_6_hardMatch_noPadis =
        _memHandle.insert(new OptionalServicesInfoTest(6));
    createS7(*s7_6_hardMatch_noPadis);
    s7_6_hardMatch_noPadis->upgrdServiceFeesResBkgDesigTblItemNo() = 0;

    OptionalServicesInfoTest* s7_7_hardMatch_withPadis =
        _memHandle.insert(new OptionalServicesInfoTest(7));
    createS7(*s7_7_hardMatch_withPadis);
    s7_7_hardMatch_withPadis->upgrdServiceFeesResBkgDesigTblItemNo() = existing_t198RecordItemNo;

    PaxTypeFare* ptf = _memHandle.insert(new PaxTypeFare());
    Fare* fare = _memHandle.insert(new Fare());
    TariffCrossRefInfo* tcri = _memHandle.insert(new TariffCrossRefInfo());
    tcri->ruleTariff() = 1;
    fare->setTariffCrossRefInfo(tcri);
    ptf->setFare(fare);
    _mockValidator->_processedFares.insert(ptf);

    _subCodeInfo->serviceGroup() = "SA";

    ((AncillaryPricingValidatorMock*)_mockValidator)->_optSvcInfos.push_back(s7_1_softMatch);
    ((AncillaryPricingValidatorMock*)_mockValidator)
        ->_optSvcInfos.push_back(s7_2_hardMatch_withPadis);
    ((AncillaryPricingValidatorMock*)_mockValidator)->_optSvcInfos.push_back(s7_3_noMatch);
    ((AncillaryPricingValidatorMock*)_mockValidator)
        ->_optSvcInfos.push_back(s7_4_hardMatch_withPadis);
    ((AncillaryPricingValidatorMock*)_mockValidator)->_optSvcInfos.push_back(s7_5_noMatch);
    ((AncillaryPricingValidatorMock*)_mockValidator)
        ->_optSvcInfos.push_back(s7_6_hardMatch_noPadis);
    ((AncillaryPricingValidatorMock*)_mockValidator)
        ->_optSvcInfos.push_back(s7_7_hardMatch_withPadis);
  }

  void testValidate_Pass_Single_S7()
  {
    OptionalServicesInfo optSvcInfo;
    createS7(optSvcInfo);

    ((AncillaryPricingValidatorMock*)_mockValidator)->_optSvcInfos.push_back(&optSvcInfo);
    CPPUNIT_ASSERT(_mockValidator->validate(*_ocFees, false));
  }

  void testValidate_SoftPass_First_S7_Pass_Second_S7()
  {
    OptionalServicesInfo optSvcInfo1;
    createS7(optSvcInfo1);
    optSvcInfo1.frequentFlyerStatus() = 3;
    OptionalServicesInfo optSvcInfo2;
    createS7(optSvcInfo2);

    PaxTypeFare ptf;
    Fare fare;
    TariffCrossRefInfo tcri;
    tcri.ruleTariff() = 1;
    fare.setTariffCrossRefInfo(&tcri);
    ptf.setFare(&fare);
    _mockValidator->_processedFares.insert(&ptf);

    ((AncillaryPricingValidatorMock*)_mockValidator)->_optSvcInfos.push_back(&optSvcInfo1);
    ((AncillaryPricingValidatorMock*)_mockValidator)->_optSvcInfos.push_back(&optSvcInfo2);
    CPPUNIT_ASSERT(_mockValidator->validate(*_ocFees, false));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _ocFees->segCount());

    _ocFees->setSeg(0);
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_FREQFLYER_SOFT));
    _ocFees->setSeg(1);
    CPPUNIT_ASSERT(!_ocFees->isAnyS7SoftPass());
  }

  void testValidate_SoftPass_First_S7_Fail_Second_Pass_Third_S7()
  {
    OptionalServicesInfo optSvcInfo1;
    createS7(optSvcInfo1);
    optSvcInfo1.frequentFlyerStatus() = 3;

    OptionalServicesInfo optSvcInfo2;
    createS7(optSvcInfo2);
    optSvcInfo2.serviceFeesAccountCodeTblItemNo() = 1000;

    OptionalServicesInfo optSvcInfo3;
    createS7(optSvcInfo3);

    PaxTypeFare ptf;
    Fare fare;
    TariffCrossRefInfo tcri;
    tcri.ruleTariff() = 1;
    fare.setTariffCrossRefInfo(&tcri);
    ptf.setFare(&fare);
    _mockValidator->_processedFares.insert(&ptf);

    ((AncillaryPricingValidatorMock*)_mockValidator)->_optSvcInfos.push_back(&optSvcInfo1);
    ((AncillaryPricingValidatorMock*)_mockValidator)->_optSvcInfos.push_back(&optSvcInfo2);
    ((AncillaryPricingValidatorMock*)_mockValidator)->_optSvcInfos.push_back(&optSvcInfo3);
    CPPUNIT_ASSERT(_mockValidator->validate(*_ocFees, false));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _ocFees->segCount());

    _ocFees->setSeg(0);
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_FREQFLYER_SOFT));
    _ocFees->setSeg(1);
    CPPUNIT_ASSERT(!_ocFees->isAnyS7SoftPass());
  }

  void testValidate_Fail() { CPPUNIT_ASSERT(!_mockValidator->validate(*_ocFees, false)); }

  void test_Validate_DDPass_False_Datanotfound()
  {
    createDiagnosticDDPassed(false);
    CPPUNIT_ASSERT(_mockValidator->validate(*_ocFees) == false);
    CPPUNIT_ASSERT(_mockValidator->_diag->str().find("DATA NOT FOUND") != string::npos);
  }

  void test_Validate_DDPass_True_Datanotfound()
  {
    createDiagnosticDDPassed(true);
    CPPUNIT_ASSERT(_mockValidator->validate(*_ocFees) == false);
    CPPUNIT_ASSERT(_mockValidator->_diag->str().empty());
  }

  void checkRBDWhenNoItemNo()
  {
    CPPUNIT_ASSERT(_validator->checkRBD(_vendor, 0, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_RBD_SOFT));
  }

  void checkRBDWhenNoRBDRecords()
  {
    CPPUNIT_ASSERT(!_validator->checkRBD(_vendor, 1, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_RBD_SOFT));
  }

  void checkRBDWhenNoRBDAndCabinInRequest()
  {
    _rbdInfo1.carrier() = "LH";
    _rbdInfo2.carrier() = "DL";

    _seg1->setBookingCode("");
    _seg1->bookedCabin().setInvalidClass();

    _seg2->setBookingCode("");
    _seg2->bookedCabin().setInvalidClass();

    _seg3->setBookingCode("");
    _seg3->bookedCabin().setInvalidClass();

    CPPUNIT_ASSERT(_validator->checkRBD(_vendor, 2, *_ocFees));
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_RBD_SOFT));
  }

  void checkRBDWhenNoRBDinReqButHasCabinAndNoAnyValidS7Records()
  {
    _rbdInfo1.carrier() = "LH";
    _rbdInfo2.carrier() = "DL";

    _seg1->setBookingCode("");
    _seg2->setBookingCode("");
    _seg3->setBookingCode("");

    CPPUNIT_ASSERT(!_validator->checkRBD(_vendor, 2, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_RBD_SOFT));
  }

  void checkRBDWhenNoRBDinReqButHasCabinAndNoAnyValidS7RBD()
  {
    setRBDInfo(_rbdInfo1, "UA", "", "");
    setRBDInfo(_rbdInfo2, "AA", "", "");

    _seg1->setBookingCode("");
    _seg2->setBookingCode("");
    _seg3->setBookingCode("");

    CPPUNIT_ASSERT(!_validator->checkRBD(_vendor, 2, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_RBD_SOFT));
  }

  void checkRBDWhenNoRBDinReqAndFailOnCabinCheck()
  {
    _rbdInfo1.carrier() = "AA";
    _rbdInfo2.carrier() = "UA";

    _seg1->setBookingCode("");
    _seg2->setBookingCode("");
    _seg3->setBookingCode("");

    CPPUNIT_ASSERT(!_validator->checkRBD(_vendor, 2, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_RBD_SOFT));
  }

  void checkRBDWhenNoRBDinReqAndPassOnCabinCheck()
  {
    _seg1->setBookingCode("");
    _seg2->setBookingCode("");
    _seg3->setBookingCode("");

    CPPUNIT_ASSERT(_mockValidator->checkRBD(_vendor, 2, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_RBD_SOFT));
  }

  void checkRBDWhenNoRBDInReqAndCabinNotFound()
  {
    _seg1->setBookingCode("");
    _seg3->setBookingCode("");
    _seg2->setBookingCode("X");

    CPPUNIT_ASSERT(!_validator->checkRBD(_vendor, 2, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_RBD_SOFT));
  }

  void checkRBDWhenCheckRBDAndPass()
  {
    CPPUNIT_ASSERT(_validator->checkRBD(_vendor, 2, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_RBD_SOFT));
  }

  void checkRBDWhenCheckRBDAndFail()
  {
    _rbdInfo1.bookingCode2() = "X";

    CPPUNIT_ASSERT(!_validator->checkRBD(_vendor, 2, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_RBD_SOFT));
  }

  void testCheckCabinDataWhenNoCabinInReq()
  {
    AirSeg seg;
    seg.bookedCabin().setInvalidClass();
    CabinType cabin;
    CarrierCode carrier;

    CPPUNIT_ASSERT(_validator->checkCabinData(seg, cabin, carrier, *_ocFees));
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_CABIN_SOFT));
  }

  void testCheckCabinDataWhenPass()
  {
    AirSeg seg;
    seg.bookedCabin().setPremiumBusinessClass();
    seg.carrier() = "DL";
    CabinType cabin;
    CarrierCode carrier = "DL";
    cabin.setPremiumBusinessClass();

    CPPUNIT_ASSERT(_validator->checkCabinData(seg, cabin, carrier, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_CABIN_SOFT));
  }

  void testCheckCabinDataWhenFail()
  {
    AirSeg seg;
    seg.bookedCabin().setPremiumBusinessClass();
    seg.carrier() = "DL";
    CabinType cabin;
    CarrierCode carrier = "DL";
    cabin.setBusinessClass();

    CPPUNIT_ASSERT(!_validator->checkCabinData(seg, cabin, carrier, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_CABIN_SOFT));
  }

//  ANSWER
  void testGetCabinWhenPass()
  {
    BookingCode rbd = "B";
    CarrierCode code = "AA";

    CPPUNIT_ASSERT(_mockValidator->getCabin(code, *_seg1, rbd));
  }
// ANSWER

  void testIsMissingDeptArvlTimeWhenTimeIsPopulated()
  {
    _seg1->pssDepartureTime() = "60";
    _seg1->pssArrivalTime() = "100";
    _seg2->pssDepartureTime() = "160";
    _seg2->pssArrivalTime() = "200";
    _seg3->pssDepartureTime() = "260";
    _seg3->pssArrivalTime() = "300";

    CPPUNIT_ASSERT(!_validator->isMissingDeptArvlTime());
  }

  void testIsMissingDeptArvlTimeWhenNoTime()
  {
    _seg1->pssDepartureTime() = "60";
    _seg1->pssArrivalTime() = "100";
    _seg2->pssDepartureTime() = "160";
    _seg2->pssArrivalTime() = "";
    _seg3->pssDepartureTime() = "260";
    _seg3->pssArrivalTime() = "300";

    CPPUNIT_ASSERT(_validator->isMissingDeptArvlTime());
  }

  void testCheckIntermediatePointWhenTimeIsPopulated()
  {
    OptionalServicesInfo optSrvInfo;
    std::vector<TravelSeg*> passedLoc3Dest;
    optSrvInfo.stopoverUnit() = OptionalServicesValidator::CHAR_BLANK;

    CPPUNIT_ASSERT(_validator->checkIntermediatePoint(optSrvInfo, passedLoc3Dest, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_TIME_SOFT));
  }

  void testCheckIntermediatePointWhenNoTimeInReqAndValidStopoverUnitInS7()
  {
    OptionalServicesInfo optSrvInfo;
    std::vector<TravelSeg*> passedLoc3Dest;
    optSrvInfo.stopoverUnit() = 'D';
    _seg1->pssDepartureTime() = "";
    _seg1->pssArrivalTime() = "";

    CPPUNIT_ASSERT(_validator->checkIntermediatePoint(optSrvInfo, passedLoc3Dest, *_ocFees));
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_TIME_SOFT));
  }

  void testCheckStopCnxDestIndWhenTimeIsPopulated()
  {
    OptionalServicesInfo optSrvInfo;
    std::vector<TravelSeg*> passedLoc3Dest;
    optSrvInfo.stopoverUnit() = OptionalServicesValidator::CHAR_BLANK;

    CPPUNIT_ASSERT(_validator->checkStopCnxDestInd(optSrvInfo, passedLoc3Dest, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_TIME_SOFT));
  }

  void testCheckStopCnxDestIndWhenNoTimeInReqAndValidStopoverUnitInS7()
  {
    OptionalServicesInfo optSrvInfo;
    std::vector<TravelSeg*> passedLoc3Dest;
    optSrvInfo.stopCnxDestInd() = 'C';
    _seg1->pssDepartureTime() = "";
    _seg1->pssArrivalTime() = "";

    CPPUNIT_ASSERT(_validator->checkStopCnxDestInd(optSrvInfo, passedLoc3Dest, *_ocFees));
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_TIME_SOFT));
  }
  void testCheckFrequentFlyerStatus_NoFreqFlyer()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.frequentFlyerStatus() = 1;
    _farePath->paxType() = _memHandle.create<PaxType>();
    CPPUNIT_ASSERT(_validator->checkFrequentFlyerStatus(optSrvInfo, *_ocFees));
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_FREQFLYER_SOFT));
  }
  void testCheckEquipmentType_NoEquipmentS7()
  {
    OptionalServicesInfo optSrvInfo;
    CPPUNIT_ASSERT(_validator->checkEquipmentType(optSrvInfo, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_EQUIPMENT_SOFT));
  }
  void testCheckEquipmentType_NoEquipmentSeg()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.equipmentCode() = "S80";
    CPPUNIT_ASSERT(_validator->checkEquipmentType(optSrvInfo, *_ocFees));
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_EQUIPMENT_SOFT));
  }
  void testCheckEquipmentType_PassEquipment()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    optSrvInfo.equipmentCode() = "S80";
    _seg1->equipmentType() = _seg2->equipmentType() = _seg3->equipmentType() = "S80";
    CPPUNIT_ASSERT(_validator->checkEquipmentType(optSrvInfo, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_EQUIPMENT_SOFT));
  }
  void testCheckEquipmentType_FailEquipment()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    optSrvInfo.equipmentCode() = "S80";
    _seg1->equipmentType() = "XXX";
    CPPUNIT_ASSERT(!_validator->checkEquipmentType(optSrvInfo, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_EQUIPMENT_SOFT));
  }

  void testIsValidFareClassFareTypeWhenSoftPassOnFareTYPE_T171HasFC_And_FareType()
  {
    PaxTypeFare ptf;
    FareMarket fm;
    ptf.fareMarket() = &fm;
    ptf.fareMarket()->travelSeg().push_back(_seg1);
    _seg1->specifiedFbc() = "H2";

    Fare fare;
    FareInfo fareInfo;
    fareInfo.fareClass() = "H2";
    fare.setFareInfo(&fareInfo);
    ptf.setFare(&fare);
    SvcFeesCxrResultingFCLInfo info;
    info.resultingFCL() = "H2";
    info.fareType() = "XYZ";

    CPPUNIT_ASSERT_EQUAL(SOFTPASS_FARE_TYPE,
                         _validator->isValidFareClassFareType(&ptf, info, *_ocFees));
    CPPUNIT_ASSERT(_ocFees->isAnyS7SoftPass());
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_RESULTING_FARE_SOFT));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _ocFees->softMatchResultingFareClassT171().size());
  }

  void testIsValidFareClassFareTypeWhenSoftPassOnFareClass_T171HasFC()
  {
    PaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    fare.setFareInfo(&fareInfo);
    ptf.setFare(&fare);
    SvcFeesCxrResultingFCLInfo info;
    info.resultingFCL() = "H2";

    CPPUNIT_ASSERT_EQUAL(SOFTPASS_FARE_CLASS,
                         _validator->isValidFareClassFareType(&ptf, info, *_ocFees));
    CPPUNIT_ASSERT(_ocFees->isAnyS7SoftPass());
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_RESULTING_FARE_SOFT));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _ocFees->softMatchResultingFareClassT171().size());
  }

  void testIsValidFareClassFareTypeWhenPassOn_T171DoesNotHaveFC_and_FareType()
  {
    PaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    fare.setFareInfo(&fareInfo);
    ptf.setFare(&fare);
    SvcFeesCxrResultingFCLInfo info;

    CPPUNIT_ASSERT_EQUAL(PASS_T171, _validator->isValidFareClassFareType(&ptf, info, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->isAnyS7SoftPass());
  }

  void testIsValidFareClassFareTypeWhen_FAIL_On_T171HasFC()
  {
    PaxTypeFare ptf;
    FareMarket fm;
    ptf.fareMarket() = &fm;
    fm.governingCarrier() = "AA";
    Fare fare;
    FareInfo fareInfo;
    fareInfo.fareClass() = "Y26";
    fare.setFareInfo(&fareInfo);
    ptf.setFare(&fare);
    SvcFeesCxrResultingFCLInfo info;
    info.resultingFCL() = "H2";

    CPPUNIT_ASSERT_EQUAL(FAIL_ON_FARE_CLASS,
                         _validator->isValidFareClassFareType(&ptf, info, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->isAnyS7SoftPass());
  }

  LocKey& getLoc(LocTypeCode type, LocCode loc)
  {
    LocKey* ret = _memHandle.create<LocKey>();
    ret->loc() = loc;
    ret->locType() = type;
    return *ret;
  }

  void testValidateLocation_AirportSameLoc()
  {
    LocCode loc;
    CPPUNIT_ASSERT(_validator->validateLocation(
        "ATP", getLoc('P', "NYC"), *_seg1->origin(), "0", true, "AA", &loc));
  }

  void testValidateLocation_AirportDiffCity()
  {
    LocCode loc;
    CPPUNIT_ASSERT(!_validator->validateLocation(
        "ATP", getLoc('P', "LHR"), *_seg1->origin(), "0", true, "AA", &loc));
    CPPUNIT_ASSERT_EQUAL(LocCode(""), loc);
  }

  void testValidateLocation_AirportMatchCity()
  {
    LocCode loc;
    CPPUNIT_ASSERT(_validator->validateLocation(
        "ATP", getLoc('P', "JFK"), *_seg1->origin(), "0", true, "AA", &loc));
    CPPUNIT_ASSERT_EQUAL(LocCode("JFK"), loc);
  }

  void testIsValidRuleTariffIndWhenRuleIsPrivateAndFareIsPubilc()
  {
    TariffCategory tariffCategory = 0; // public
    CPPUNIT_ASSERT(_validator->isValidRuleTariffInd(
        OptionalServicesValidator::RULE_TARIFF_IND_PUBLIC, tariffCategory));
  }

  void prepareMatchRuleTariffWhenNoFareTariffCase(PaxTypeFare& ptf)
  {
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fare->setFareInfo(fareInfo);
    TariffCrossRefInfo* tariffCrossRefInfo = _memHandle.create<TariffCrossRefInfo>();
    fare->setTariffCrossRefInfo(tariffCrossRefInfo);
    tariffCrossRefInfo->ruleTariff() = 0;
    ptf.setFare(fare);

    FareClassAppInfo* fareClassAppInfo = _memHandle.create<FareClassAppInfo>();
    ptf.fareClassAppInfo() = fareClassAppInfo;
    fareClassAppInfo->_ruleTariff = 0;
  }

  void testMatchRuleTariffWhenNoFareTariff()
  {
    uint16_t ruleTariff;
    PaxTypeFare ptf;
    prepareMatchRuleTariffWhenNoFareTariffCase(ptf);

    CPPUNIT_ASSERT(_validator->matchRuleTariff(ruleTariff, ptf, *_ocFees));
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_RULETARIFF_SOFT));
  }

  void testCheckRuleTariffWhenS7RuleTariffIsEmpty()
  {
    CPPUNIT_ASSERT(_validator->checkRuleTariff(-1, *_ocFees));
  }

  void testCheckRuleTariffWhenNoFBI()
  {
    CPPUNIT_ASSERT(_validator->checkRuleTariff(1, *_ocFees));
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_RULETARIFF_SOFT));
  }

  void testCheckRuleWhenS7RuleIsEmpty()
  {
    RuleNumber rule = "";
    CPPUNIT_ASSERT(_validator->checkRule(rule, *_ocFees));
  }

  void testCheckRuleWhenNoFBI()
  {
    RuleNumber rule = "1234";
    CPPUNIT_ASSERT(_validator->checkRule(rule, *_ocFees));
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_RULE_SOFT));
  }

  void testCheckRuleWhenRuleIsEmptyInReq()
  {
    RuleNumber rule = "R123";
    PaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    fareInfo.ruleNumber() = "";
    fare.setFareInfo(&fareInfo);
    ptf.setFare(&fare);
    _validator->_processedFares.insert(&ptf);

    CPPUNIT_ASSERT(_validator->checkRule(rule, *_ocFees));
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_RULE_SOFT));
  }

  void testCheckRuleWhenFailRule()
  {
    RuleNumber rule = "R123";
    PaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    fareInfo.ruleNumber() = "A123";
    fare.setFareInfo(&fareInfo);
    ptf.setFare(&fare);
    _validator->_processedFares.insert(&ptf);

    CPPUNIT_ASSERT(!_validator->checkRule(rule, *_ocFees));
  }

  void testValidate_Start_And_Stop_Time_Soft_Pass()
  {
    _seg1->pssDepartureTime() = "";
    OptionalServicesInfo optSrvInfo;

    CPPUNIT_ASSERT(_validator->validateStartAndStopTime(optSrvInfo, *_ocFees));
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_TIME_SOFT));
  }

  void testValidate_Start_And_Stop_Time_Pass()
  {
    _seg1->pssDepartureTime() = "0600";
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.startTime() = 1000;
    optSrvInfo.stopTime() = 1250;
    CPPUNIT_ASSERT(_validator->validateStartAndStopTime(optSrvInfo, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_TIME_SOFT));
  }

  void testValidate_Start_And_Stop_Time_Fail()
  {
    _seg1->pssDepartureTime() = "0500";
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.startTime() = 1000;
    optSrvInfo.stopTime() = 1150;
    CPPUNIT_ASSERT(!_validator->validateStartAndStopTime(optSrvInfo, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_TIME_SOFT));
  }

  void testValidate_Start_And_Stop_Time_Soft_Pass_Dup()
  {
    _seg1->pssDepartureTime() = "";

    CPPUNIT_ASSERT(_validator->validateStartAndStopTime(*_ocFees));
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_TIME_SOFT));
  }

  void testValidate_Start_And_Stop_Time_Pass_Dup()
  {
    _seg1->pssDepartureTime() = "0600";
    CPPUNIT_ASSERT(_validator->validateStartAndStopTime(*_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_TIME_SOFT));
  }

  void testIsValidCarrierFlightNumber_T186_FLT_present_PASS()
  {
    CarrierFlightSeg cfsInfo;
    cfsInfo.flt1() = 100;
    _seg1->flightNumber() = 100;
    CPPUNIT_ASSERT_EQUAL(PASS_T186, _validator->isValidCarrierFlightNumber(*_seg1, cfsInfo));
  }

  void testIsValidCarrierFlightNumber_T186_FLT_present_SOFT_PASS()
  {
    CarrierFlightSeg cfsInfo;
    cfsInfo.flt1() = 100;
    CPPUNIT_ASSERT_EQUAL(SOFTPASS_FLIGHT, _validator->isValidCarrierFlightNumber(*_seg1, cfsInfo));
  }

  void testIsValidCarrierFlightNumber_T186_FLT_present_FAIL_ON_FLIGHT()
  {
    CarrierFlightSeg cfsInfo;
    CPPUNIT_ASSERT_EQUAL(FAIL_ON_FLIGHT, _validator->isValidCarrierFlightNumber(*_seg1, cfsInfo));
  }

  void testCheckSecurityReturnTRUEWhenNoSecurityCodedAndS7Public()
  {
    OptionalServicesInfo optSrvInfo;
    CPPUNIT_ASSERT(_validator->checkSecurity(optSrvInfo, *_ocFees));
  }

  void testCheckSecurityReturnFALSEWhenNoSecurityCodedAndS7Private()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.publicPrivateInd() = 'P';
    CPPUNIT_ASSERT(!_validator->checkSecurity(optSrvInfo, *_ocFees));
  }

  void checkRBDWhenNoRBDinReqAndNoCabinAndNoAnyValidS7RBD()
  {
    setRBDInfo(_rbdInfo1, "UA", "", "");
    setRBDInfo(_rbdInfo2, "AA", "", "");

    _seg1->setBookingCode("");
    _seg1->bookedCabin().setInvalidClass();
    _seg2->setBookingCode("");
    _seg2->bookedCabin().setInvalidClass();
    _seg3->setBookingCode("");
    _seg3->bookedCabin().setInvalidClass();

    CPPUNIT_ASSERT(!_validator->checkRBD(_vendor, 2, *_ocFees));
    CPPUNIT_ASSERT(!_ocFees->softMatchS7Status().isSet(OCFees::S7_RBD_SOFT));
  }

  void testSetPBDate_When_advPurchUnit_3_D()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "D";
    optSrvInfo.advPurchPeriod() = "03";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT_EQUAL(DateTime(2011, 3, 28), _ocFees->purchaseByDate());
  }

  void testSetPBDate_When_advPurchUnit_13_D()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "D";
    optSrvInfo.advPurchPeriod() = "13";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT_EQUAL(DateTime(2011, 3, 18), _ocFees->purchaseByDate());
  }

  void testSetPBDate_When_advPurchUnit_33_D()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "D";
    optSrvInfo.advPurchPeriod() = "33";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT_EQUAL(DateTime(2011, 2, 26), _ocFees->purchaseByDate());
  }

  void testSetPBDate_When_advPurchUnit_63_D()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "D";
    optSrvInfo.advPurchPeriod() = "63";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT_EQUAL(DateTime(2011, 1, 27), _ocFees->purchaseByDate());
  }

  void testSetPBDate_When_advPurchUnit_103_D()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "D";
    optSrvInfo.advPurchPeriod() = "103";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT_EQUAL(DateTime(2010, 12, 18), _ocFees->purchaseByDate());
  }

  void testSetPBDate_When_advPurchUnit_1_M()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "M";
    optSrvInfo.advPurchPeriod() = "01";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT_EQUAL(DateTime(2011, 2, 28), _ocFees->purchaseByDate());
  }

  void testSetPBDate_When_advPurchUnit_2_M()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "M";
    optSrvInfo.advPurchPeriod() = "02";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT_EQUAL(DateTime(2011, 1, 31), _ocFees->purchaseByDate());
  }

  void testSetPBDate_When_advPurchUnit_4_M()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "M";
    optSrvInfo.advPurchPeriod() = "04";
    _seg1->departureDT() = DateTime(2012, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT_EQUAL(DateTime(2011, 11, 30), _ocFees->purchaseByDate());
  }

  void testSetPBDate_When_advPurchUnit_11_M()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "M";
    optSrvInfo.advPurchPeriod() = "11";
    _seg1->departureDT() = DateTime(2012, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT_EQUAL(DateTime(2011, 4, 30), _ocFees->purchaseByDate());
  }

  void testSetPBDate_When_advPurchUnit_15_M()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "M";
    optSrvInfo.advPurchPeriod() = "15";
    _seg1->departureDT() = DateTime(2012, 3, 15);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT_EQUAL(DateTime(2010, 12, 15), _ocFees->purchaseByDate());
  }

  void testIsStopover_MissingTime()
  {
    OptionalServicesInfo optSrvInfo;
    _seg1->pssDepartureTime() = "60";
    _seg1->pssArrivalTime() = "100";
    _seg2->pssDepartureTime() = "160";
    _seg2->pssArrivalTime() = "";
    _seg3->pssDepartureTime() = "260";
    _seg3->pssArrivalTime() = "300";

    CPPUNIT_ASSERT(_validator->isStopover(optSrvInfo, _seg1, _seg3));
  }

  void testSetPBDate_When_advPurchUnit_0_D()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "D";
    optSrvInfo.advPurchPeriod() = "0";
    _seg1->departureDT() = DateTime(2011, 3, 31);
    DateTime dt = DateTime(2011, 9, 18, 8, 15, 0);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, dt));
    CPPUNIT_ASSERT_EQUAL(DateTime(2011, 9, 18, 8, 15, 0), _ocFees->purchaseByDate());
  }

  void testSetPBDate_When_advPurchUnit_5_H()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "H";
    optSrvInfo.advPurchPeriod() = "05";
    DateTime dt = _trx->ticketingDate();
    DateTime calc = dt.addSeconds(18000);

    _seg1->departureDT() = dt.nextDay();
    DateTime comp = _seg1->departureDT();
    DateTime compareDT = comp.subtractSeconds(18000);
    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, calc));
    CPPUNIT_ASSERT_EQUAL(compareDT, _ocFees->purchaseByDate());
  }

  void testSetPBDate_When_advPurchUnit_DOW()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.advPurchUnit() = "01";
    optSrvInfo.advPurchPeriod() = "FRI";
    uint32_t days = 0;
    int advPinDays = 0;
    DateTime dt = _trx->ticketingDate();
    _seg1->departureDT() = dt.addDays(10);

    _validator->getAdvPurPeriod(optSrvInfo, _seg1->departureDT(), days, advPinDays);
    if (days == 0 && advPinDays == 0)
      days = 7;
    days += advPinDays;

    DateTime comp = _seg1->departureDT();
    DateTime compareDT = comp.subtractDays(days);

    CPPUNIT_ASSERT(_validator->setPBDate(optSrvInfo, *_ocFees, _trx->ticketingDate()));
    CPPUNIT_ASSERT_EQUAL(compareDT, _ocFees->purchaseByDate());
  }

protected:
  class MyDataHandle : public DataHandleMock
  {
    TestMemHandle& _memHandle;
    SvcFeesResBkgDesigInfo& _rbdInfo1;
    SvcFeesResBkgDesigInfo& _rbdInfo2;

  public:
    MyDataHandle(TestMemHandle& memHandle,
                 SvcFeesResBkgDesigInfo& rbdInfo1,
                 SvcFeesResBkgDesigInfo& rbdInfo2)
      : _memHandle(memHandle), _rbdInfo1(rbdInfo1), _rbdInfo2(rbdInfo2)
    {
    }

    const Cabin*
    getCabin(const CarrierCode& carrier, const BookingCode& bookingCode, const DateTime& date)
    {
      Cabin* cabin = _memHandle.create<Cabin>();
      if (bookingCode == "A")
        cabin->cabin().setFirstClass();
      else if (bookingCode == "B")
        cabin->cabin().setBusinessClass();
      else if (bookingCode == "Y")
        cabin->cabin().setEconomyClass();
      else
        return DataHandleMock::getCabin(carrier, bookingCode, date);

      return cabin;
    }

    const std::vector<SvcFeesResBkgDesigInfo*>&
    getSvcFeesResBkgDesig(const VendorCode& vendor, const int itemNo)
    {
      std::vector<SvcFeesResBkgDesigInfo*>& rbdInfos =
          *_memHandle.create<std::vector<SvcFeesResBkgDesigInfo*> >();
      if (itemNo == 2)
      {
        rbdInfos.push_back(&_rbdInfo1);
        rbdInfos.push_back(&_rbdInfo2);
      }
      else if (itemNo == 1)
      { // do nothing
      }
      else
        return DataHandleMock::getSvcFeesResBkgDesig(vendor, itemNo);

      return rbdInfos;
    }
  };

  class OptionalServicesInfoTest : public OptionalServicesInfo
  {
  public:
    OptionalServicesInfoTest(size_t index) : OptionalServicesInfo(), _recordIndex(index) {}

    size_t _recordIndex;
  };

  class AncillaryPricingValidatorMock : public AncillaryPricingValidator
  {
  public:
    AncillaryPricingValidatorMock(
        TestMemHandle& memHandle,
        const OcValidationContext& ctx,
        const std::vector<TravelSeg*>::const_iterator segI,
        const std::vector<TravelSeg*>::const_iterator segIE,
        const std::vector<TravelSeg*>::const_iterator endOfJourney,
        const Ts2ss& ts2ss,
        bool isInternational,
        bool isOneCarrier,
        bool isMarketingCxr,
        Diag877Collector* diag)
      : AncillaryPricingValidator(ctx,
                                  segI,
                                  segIE,
                                  endOfJourney,
                                  ts2ss,
                                  isInternational,
                                  isOneCarrier,
                                  isMarketingCxr,
                                  diag),
        _memHandle(memHandle)
    {
    }

    const std::vector<OptionalServicesInfo*>&
    getOptionalServicesInfo(const SubCodeInfo& subCode) const
    {
      return _optSvcInfos;
    }

    bool
    isInLoc(const VendorCode& vendor, const LocKey& locKey, const Loc& loc, CarrierCode carrier)
        const
    {
      return !isdigit(loc.loc()[0]) && !isdigit(locKey.loc()[0]);
    }

    bool
    isInZone(const VendorCode& vendor, const LocCode& zone, const Loc& loc, CarrierCode carrier)
        const
    {
      return isdigit(loc.loc()[0]);
    }

    bool svcFeesAccountCodeValidate(const SvcFeesAccountCodeValidator& validator, int itemNo) const
    {
      return itemNo % 2;
    }

    const Cabin*
    getCabin(const CarrierCode& carrier, AirSeg& seg, const BookingCode& bookingCode) const
    {
      Cabin* cabin = _memHandle.create<Cabin>();
      if (bookingCode == "A")
        cabin->cabin().setFirstClass();
      else if (bookingCode == "B")
        cabin->cabin().setBusinessClass();
      else
        cabin->cabin().setEconomyClass();

      return cabin;
    }

    mutable std::vector<OptionalServicesInfo*> _optSvcInfos;
    TestMemHandle& _memHandle;
  };

  void
  testValidate_Request_Has_Hard_Match_Only_First_S7_was_SoftPass_Before_Now_Its_Fail_And_Second_S7_Pass()
  {
    OptionalServicesInfo optSvcInfo1;
    createS7(optSvcInfo1);
    optSvcInfo1.frequentFlyerStatus() = 3;
    OptionalServicesInfo optSvcInfo2;
    createS7(optSvcInfo2);

    PaxTypeFare ptf;
    Fare fare;
    TariffCrossRefInfo tcri;
    tcri.ruleTariff() = 1;
    fare.setTariffCrossRefInfo(&tcri);
    ptf.setFare(&fare);
    _mockValidator->_processedFares.insert(&ptf);

    ((AncillaryPricingValidatorMock*)_mockValidator)->_optSvcInfos.push_back(&optSvcInfo1);
    ((AncillaryPricingValidatorMock*)_mockValidator)->_optSvcInfos.push_back(&optSvcInfo2);
    CPPUNIT_ASSERT(_mockValidator->validate(*_ocFees, false));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _ocFees->segCount());

    _ocFees->setSeg(0);
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_FREQFLYER_SOFT));
    _ocFees->setSeg(1);
    CPPUNIT_ASSERT(!_ocFees->isAnyS7SoftPass());

    // hard match in the request, no soft match returned
    AncRequest* req = static_cast<AncRequest*>(_trx->getRequest());
    req->hardMatchIndicator() = true;

    _ocFees->setSeg(0);
    _ocFees->cleanOutCurrentSeg();
    _ocFees->segments().erase(_ocFees->segments().end() - 1);

    CPPUNIT_ASSERT(_mockValidator->validate(*_ocFees, false));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _ocFees->segCount());

    _ocFees->setSeg(0);
    CPPUNIT_ASSERT(!_ocFees->isAnyS7SoftPass());
  }

  void test_skipUpgradeCheck__fail_normally_hardMatched_s7_for_ACS_client()
  {
    uint32_t existing_t198RecordItemNo = 2;
    OptionalServicesInfo s7_normallyHardMatched_withPadis;
    s7_normallyHardMatched_withPadis.upgrdServiceFeesResBkgDesigTblItemNo() =
        existing_t198RecordItemNo;
    _subCodeInfo->serviceGroup() = "SA";

    // fail when client is ACS
    _trx->billing()->requestPath() = ACS_PO_ATSE_PATH;
    CPPUNIT_ASSERT(!_mockValidator->skipUpgradeCheck(s7_normallyHardMatched_withPadis, *_ocFees));

    // pass for other (not ACS) client
    _trx->billing()->requestPath() = UNKNOWN_PATH;
    CPPUNIT_ASSERT(_mockValidator->skipUpgradeCheck(s7_normallyHardMatched_withPadis, *_ocFees));
  }

  void testValidate_s7_record_with_SA_service_group__BADDATA_scenario()
  {
    uint32_t existing_t198RecordItemNo = 2;
    uint32_t itemId_for_which_there_are_no_T198_records = 6;

    OptionalServicesInfoTest s7_no_match_because_of_bad_data(0);
    createS7(s7_no_match_because_of_bad_data);
    s7_no_match_because_of_bad_data.upgrdServiceFeesResBkgDesigTblItemNo() =
        itemId_for_which_there_are_no_T198_records;

    OptionalServicesInfoTest s7_matched(1);
    createS7(s7_matched);
    s7_matched.upgrdServiceFeesResBkgDesigTblItemNo() = existing_t198RecordItemNo;

    _subCodeInfo->serviceGroup() = "SA";

    PaxTypeFare ptf;
    Fare fare;
    TariffCrossRefInfo tcri;
    tcri.ruleTariff() = 1;
    fare.setTariffCrossRefInfo(&tcri);
    ptf.setFare(&fare);
    _mockValidator->_processedFares.insert(&ptf);

    ((AncillaryPricingValidatorMock*)_mockValidator)
        ->_optSvcInfos.push_back(&s7_no_match_because_of_bad_data);
    ((AncillaryPricingValidatorMock*)_mockValidator)->_optSvcInfos.push_back(&s7_matched);

    _mockValidator->validate(*_ocFees, false);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _ocFees->segCount());
    CPPUNIT_ASSERT_EQUAL(
        size_t(1),
        ((const OptionalServicesInfoTest*)((const OptionalServicesInfo*)_ocFees->optFee()))
            ->_recordIndex);
  }

  void
  testValidate_s7_records_sequence_with_SA_service_group__M70_request_hard_match_indicator_false()
  {
    prepare_s7_records_sequence_for_M70_request_test_against_padis_codes_project();

    AncRequest* req = static_cast<AncRequest*>(_trx->getRequest());
    req->hardMatchIndicator() = false;


    CPPUNIT_ASSERT(_mockValidator->validate(*_ocFees, false));

    CPPUNIT_ASSERT_EQUAL(size_t(4), _ocFees->segCount());

    _ocFees->setSeg(0);
    CPPUNIT_ASSERT_EQUAL(
        size_t(1),
        ((const OptionalServicesInfoTest*)((const OptionalServicesInfo*)_ocFees->optFee()))
            ->_recordIndex);
    CPPUNIT_ASSERT(_ocFees->isAnyS7SoftPass());
    CPPUNIT_ASSERT(_ocFees->softMatchS7Status().isSet(OCFees::S7_FREQFLYER_SOFT));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _ocFees->padisData().size());

    _ocFees->setSeg(1);
    CPPUNIT_ASSERT_EQUAL(
        size_t(2),
        ((const OptionalServicesInfoTest*)((const OptionalServicesInfo*)_ocFees->optFee()))
            ->_recordIndex);
    CPPUNIT_ASSERT(!_ocFees->isAnyS7SoftPass());
    CPPUNIT_ASSERT_EQUAL(size_t(2), _ocFees->padisData().size());

    _ocFees->setSeg(2);
    CPPUNIT_ASSERT_EQUAL(
        size_t(4),
        ((const OptionalServicesInfoTest*)((const OptionalServicesInfo*)_ocFees->optFee()))
            ->_recordIndex);
    CPPUNIT_ASSERT(!_ocFees->isAnyS7SoftPass());
    CPPUNIT_ASSERT_EQUAL(size_t(2), _ocFees->padisData().size());

    _ocFees->setSeg(3);
    CPPUNIT_ASSERT_EQUAL(
        size_t(6),
        ((const OptionalServicesInfoTest*)((const OptionalServicesInfo*)_ocFees->optFee()))
            ->_recordIndex);
    CPPUNIT_ASSERT(!_ocFees->isAnyS7SoftPass());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _ocFees->padisData().size());

    _ocFees->setSeg(0);
    _ocFees->cleanOutCurrentSeg();
    _ocFees->segments().erase(_ocFees->segments().begin() + 1, _ocFees->segments().end());
  }

  void
  testValidate_s7_records_sequence_with_SA_service_group__M70_request_hard_match_indicator_true()
  {
    prepare_s7_records_sequence_for_M70_request_test_against_padis_codes_project();

    AncRequest* req = static_cast<AncRequest*>(_trx->getRequest());
    req->hardMatchIndicator() = true;


    CPPUNIT_ASSERT(_mockValidator->validate(*_ocFees, false));
    CPPUNIT_ASSERT_EQUAL(size_t(3), _ocFees->segCount());

    _ocFees->setSeg(0);
    CPPUNIT_ASSERT_EQUAL(
        size_t(2),
        ((const OptionalServicesInfoTest*)((const OptionalServicesInfo*)_ocFees->optFee()))
            ->_recordIndex);
    CPPUNIT_ASSERT(!_ocFees->isAnyS7SoftPass());
    CPPUNIT_ASSERT_EQUAL(size_t(2), _ocFees->padisData().size());

    _ocFees->setSeg(1);
    CPPUNIT_ASSERT_EQUAL(
        size_t(4),
        ((const OptionalServicesInfoTest*)((const OptionalServicesInfo*)_ocFees->optFee()))
            ->_recordIndex);
    CPPUNIT_ASSERT(!_ocFees->isAnyS7SoftPass());
    CPPUNIT_ASSERT_EQUAL(size_t(2), _ocFees->padisData().size());

    _ocFees->setSeg(2);
    CPPUNIT_ASSERT_EQUAL(
        size_t(6),
        ((const OptionalServicesInfoTest*)((const OptionalServicesInfo*)_ocFees->optFee()))
            ->_recordIndex);
    CPPUNIT_ASSERT(!_ocFees->isAnyS7SoftPass());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _ocFees->padisData().size());

    _ocFees->setSeg(0);
    _ocFees->cleanOutCurrentSeg();
    _ocFees->segments().erase(_ocFees->segments().begin() + 1, _ocFees->segments().end());
  }

  void test_skipUpgradeCheck_upgradeCabin_BLANK_upgrade198_0()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.upgradeCabin() = BLANK;
    optSrvInfo.upgrdServiceFeesResBkgDesigTblItemNo() = 0;
    CPPUNIT_ASSERT(_mockValidator->skipUpgradeCheck(optSrvInfo, *_ocFees));
  }

  void test_skipUpgradeCheck_upgradeCabin_NOT_BLANK_upgrade198_0()
  {

    OptionalServicesInfo optSrvInfo;
    optSrvInfo.upgradeCabin() = 'J';
    optSrvInfo.upgrdServiceFeesResBkgDesigTblItemNo() = 0;
    CPPUNIT_ASSERT(!_mockValidator->skipUpgradeCheck(optSrvInfo, *_ocFees));
  }

  void test_skipUpgradeCheck_upgradeCabin_BLANK_upgrade198_NOT_0()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.upgradeCabin() = BLANK;
    optSrvInfo.upgrdServiceFeesResBkgDesigTblItemNo() = 1;
    CPPUNIT_ASSERT(!_mockValidator->skipUpgradeCheck(optSrvInfo, *_ocFees));
  }

  void test_skipUpgradeCheck_upgradeCabin_NOT_BLANK_upgrade198_NOT_0()
  {
    OptionalServicesInfo optSrvInfo;
    optSrvInfo.upgradeCabin() = 'J';
    optSrvInfo.upgrdServiceFeesResBkgDesigTblItemNo() = 1;
    CPPUNIT_ASSERT(!_mockValidator->skipUpgradeCheck(optSrvInfo, *_ocFees));
  }

protected:
  TestMemHandle _memHandle;
  AncillaryPricingValidator* _validator;
  AncillaryPricingValidatorMock* _mockValidator;
  FarePath* _farePath;
  PaxType* _paxType;
  AirSeg* _seg1;
  AirSeg* _seg2;
  AirSeg* _seg3;
  PricingTrx* _trx;
  std::vector<TravelSeg*> _tvlSegs;
  std::vector<TravelSeg*>::const_iterator _endOfJourney;
  Ts2ss _ts2ss;
  Diag877Collector* _diag;
  OCFees* _ocFees;
  SubCodeInfo* _subCodeInfo;
  SvcFeesResBkgDesigInfo _rbdInfo1;
  SvcFeesResBkgDesigInfo _rbdInfo2;
  VendorCode _vendor;
  static const std::string LOC_NYCU, LOC_YMQC, LOC_NRTJ, LOC_SINS;
};

const std::string AncillaryPricingValidatorTest::LOC_NYCU =
    "/vobs/atseintl/test/testdata/data/LocNYC.xml",
                  AncillaryPricingValidatorTest::LOC_YMQC =
                      "/vobs/atseintl/test/testdata/data/LocYMQ.xml",
                  AncillaryPricingValidatorTest::LOC_NRTJ =
                      "/vobs/atseintl/test/testdata/data/LocNRT.xml",
                  AncillaryPricingValidatorTest::LOC_SINS =
                      "/vobs/atseintl/test/testdata/data/LocSIN.xml";

CPPUNIT_TEST_SUITE_REGISTRATION(AncillaryPricingValidatorTest);
}
