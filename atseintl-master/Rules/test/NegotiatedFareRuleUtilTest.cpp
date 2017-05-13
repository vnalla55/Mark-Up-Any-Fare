//----------------------------------------------------------------------------
//  Copyright Sabre 2004
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
#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Customer.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "DBAccess/PrintOption.h"
#include "DBAccess/Tours.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleConst.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using boost::assign::operator+=;

namespace
{
class MockPaxTypeFare : public PaxTypeFare
{
public:
  MockPaxTypeFare(bool cat15HasSecurity = false) : PaxTypeFare()
  {
    _fareInfo.fareClass() = "MOCKMOCK";
    _fare.initialize(Fare::FS_ForeignDomestic, &_fareInfo, _fareMarket);
    _fare.setCat15HasSecurity(cat15HasSecurity);

    setFare(&_fare);
  }

private:
  Fare _fare;
  FareInfo _fareInfo;
  FareMarket _fareMarket;
};

class NegPaxTypeFare : public MockPaxTypeFare
{
public:
  NegPaxTypeFare(const Indicator& bspMethod = RuleConst::NRR_METHOD_BLANK,
                 const Indicator& displayCatType = RuleConst::NET_SUBMIT_FARE,
                 bool with979 = false,
                 bool withCommission = false,
                 Indicator indNetGross = RuleConst::BLANK,
                 const std::string& cat27TourCode = "",
                 int noSegs = 1,
                 const Indicator& cat35TourCodeType1 = RuleConst::BLANK,
                 const std::string& cat35TourCode1 = "",
                 const Indicator& cat35TourCodeType2 = RuleConst::BLANK,
                 const std::string& cat35TourCode2 = "",
                 const Indicator& tktFareDataInd1 = RuleConst::BLANK)
    : MockPaxTypeFare()
  {
    _fareClassAppInfo._displayCatType = displayCatType;
    fareClassAppInfo() = &_fareClassAppInfo;

    _negFareRest.netRemitMethod() = bspMethod;
    _negFareRest.negFareCalcTblItemNo() = with979 ? 1000 : 0;
    _negFareRest.commPercent() = withCommission ? 50 : 0;
    _negFareRest.netGrossInd() = indNetGross;
    _negFareRest.noSegs() = noSegs;
    _negFareRest.tourBoxCodeType1() = cat35TourCodeType1;
    _negFareRest.tourBoxCode1() = cat35TourCode1;
    _negFareRest.tourBoxCodeType2() = cat35TourCodeType2;
    _negFareRest.tourBoxCode2() = cat35TourCode2;
    _negFareRest.tktFareDataInd1() = tktFareDataInd1;
    _status.set(PaxTypeFare::PTF_Negotiated);
    _negRuleData.ruleItemInfo() = &_negFareRest;
    _negAllRuleData.fareRuleData = &_negRuleData;
    (*(paxTypeFareRuleDataMap()))[RuleConst::NEGOTIATED_RULE] = &_negAllRuleData;

    _tours.tourNo() = cat27TourCode;
    _toursRuleData.ruleItemInfo() = &_tours;
    _toursAllRuleData.fareRuleData = &_toursRuleData;

    (*(paxTypeFareRuleDataMap()))[RuleConst::TOURS_RULE] = &_toursAllRuleData;
  }

  NegPaxTypeFareRuleData& negRuleData() { return _negRuleData; }
  const NegPaxTypeFareRuleData& negRuleData() const { return _negRuleData; }

  NegFareRest& negFareRest() { return _negFareRest; }
  const NegFareRest& negFareRest() const { return _negFareRest; }

private:
  FareClassAppInfo _fareClassAppInfo;
  PaxTypeFare::PaxTypeFareAllRuleData _negAllRuleData;
  NegPaxTypeFareRuleData _negRuleData;
  NegFareRest _negFareRest;

  Tours _tours;
  PaxTypeFareRuleData _toursRuleData;
  PaxTypeFareAllRuleData _toursAllRuleData;
};

class NegotiatedFareRuleUtilMock : public NegotiatedFareRuleUtil
{
public:
  NegotiatedFareRuleUtilMock() : NegotiatedFareRuleUtil() {}

protected:
  virtual bool processNetRemitFareSelection(PricingTrx& trx,
                                            const FarePath& farePath,
                                            PricingUnit& pricingUnit,
                                            FareUsage& fareUsage,
                                            const NegFareRest& negFareRest) const
  {
    return true;
  }

  bool checkVendor(PricingTrx& trx, const VendorCode& vendor) const { return true; }
};
}

class NegotiatedFareRuleUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NegotiatedFareRuleUtilTest);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingPass);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingNetAmtFlipped);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingNetAmtNotFlipped);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnMultipleTourCode);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnTourCodeNotFound);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnInvalidItbtPsgCoupon);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnEmptyItbtPsgCoupon);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnItbtBspNotBlank);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnNotItbt);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnMultipleFareBox);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnMultipleBsp);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnMultipleNetGross);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnMixFares);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnFaresNotCombinable);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnNetSellingConflict);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnMultipleCommission);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnMixCommission);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnIssueSepareteTkt);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnInvalidNetRemitFare);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnInvalidNetRemitComm);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnMultipleValueCode);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnMultiplePrintOption);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnMixedFareBoxAmount);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnSystemError);
  CPPUNIT_TEST(testCheckWarningMsgForTicketingThrowOnInvalidNetRemitFareForByte101Conflict);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassNoWarning);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassNoWarningPricingIsCat35Net);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassNoWarningPricingIsNotCat35Net);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassNoWarningPricingIsCat35NetNetAmtWasFlipped);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassNoWarningPricingIsCat35NetNetAmtWasNotFlipped);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningInvalidNetRemitComm);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningAutoTktNotPermited);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassSetTrfRestrictedForAbacus);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassSetTrfRestrictedForNonAbacus);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningMixCommissionTraileMsgForAbacus);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningMultipleCommissionTraileMsgForAbacus);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningMultipleTourCode);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningTourCodeNotFound);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningNotItbt);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningInvalidItbtPsgCoupon);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningEmptyItbtPsgCoupon);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningMultipleFareBox);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningItbtBspNotBlank);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningMultipleNetGross);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningMultipleBsp);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningIssueSeparateTkt);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningMixFares);
  CPPUNIT_TEST(testCheckWarningMsgForPricingWarningNoNetFareAmountThrow);
  CPPUNIT_TEST(testCheckWarningMsgForPricingWarningFaresNotCombinableNoNetThrow);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningFaresNotCombinable);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningNetSellingConflict);
  CPPUNIT_TEST(testCheckWarningMsgForPricingWarningSystemErrorThrow);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningInvalidNetRemitFare);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningNetFareAmountExceedsFare);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningInvalidNetRemitFareWhenConflictTFDByte101);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningMultipleValueCode);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningMultiplePrintOption);
  CPPUNIT_TEST(testCheckWarningMsgForPricingPassWarningMixedFareBoxAmount);
  CPPUNIT_TEST(testHandleCat35ProcessingPassTktIndicatorTrue);
  CPPUNIT_TEST(testHandleCat35ProcessingPassTktIndicatorFalse);

  CPPUNIT_TEST(testProcessNegFareItbtPassWhenAxessUser);
  CPPUNIT_TEST(testProcessNegFareItbtPassWhenIsNotCat35Net);
  CPPUNIT_TEST(testProcessNegFareItbtCheckWarningMsgForInvalidNetRemComm);
  CPPUNIT_TEST(testProcessNegFareItbtThrowWhenNoAltPricing);
  CPPUNIT_TEST(testProcessNegFareItbtFailWhenAbacusUser);

  CPPUNIT_TEST(testProcessNetRemitPassWhenNoPricingUnits);
  CPPUNIT_TEST(testProcessNetRemitFalseNotCat35);
  CPPUNIT_TEST(testProcessNetRemitPass);

  CPPUNIT_TEST(testcheckFareBoxTextITBT_IFITAndBT);
  CPPUNIT_TEST(testcheckFareBoxTextITBT_IFBTAndIT);
  CPPUNIT_TEST(testcheckFareBoxTextITBT_IFBlankAndIT);
  CPPUNIT_TEST(testcheckFareBoxTextITBT_IFBlankAndBT);
  CPPUNIT_TEST(testcheckFareBoxTextITBT_IFBlankAndBlank);
  CPPUNIT_TEST(testcheckFareBox_Blank);
  CPPUNIT_TEST(testcheckFareBox_IT);
  CPPUNIT_TEST(testcheckFareBox_BT);
  CPPUNIT_TEST(testcheckFareBox_YT);
  CPPUNIT_TEST(testvalidateFareBoxText);
  CPPUNIT_TEST(testvalidateFareBoxText_FBT2Segs_Blank_And_IT);
  CPPUNIT_TEST(testvalidateFareBoxText_FBT2Segs_Blank_And_BT);
  CPPUNIT_TEST(testvalidateFareBoxText_FBT2Segs_IT_And_BT);
  CPPUNIT_TEST(testvalidateFareBoxText_FBT2Segs_BT_And_IT);
  CPPUNIT_TEST(testvalidateFareBoxText_FBT2Segs_IT_AND_BLANK);
  CPPUNIT_TEST(testvalidateFareBoxText_FBT2Segs_BT_AND_BLANK);
  CPPUNIT_TEST(testvalidateFareBoxText_FBT2Segs_IT_And_BT_Coupon2);
  CPPUNIT_TEST(testvalidateFareBoxText_FBT2Segs_BT_And_IT_Coupon2);

  CPPUNIT_TEST(testProcessCat35NetTicketingFareWhenMixedFares);
  CPPUNIT_TEST(testProcessCat35NetTicketingFareWhenMethodTypeInConflict);
  CPPUNIT_TEST(testProcessCat35NetTicketingFareWhenNetGrossIndInConflict);
  CPPUNIT_TEST(testProcessCat35NetTicketingFareWhenCommissionInConflictForTicketing);
  CPPUNIT_TEST(testProcessCat35NetTicketingFareWhenCommissionPercentInConflict);
  CPPUNIT_TEST(testProcessCat35NetTicketingFareWhenPreviousFareHasNoTourCode);
  CPPUNIT_TEST(testProcessCat35NetTicketingFareWhenPreviousFareHasTourCode);
  CPPUNIT_TEST(testProcessCat35NetTicketingFareWhenPreviousFareHasNoFareBox);
  CPPUNIT_TEST(testProcessCat35NetTicketingFareWhenPreviousFareHasFareBox);
  CPPUNIT_TEST(test_validateEndorsementsForAbacus_ConflictEndorsements_NoWarning);
  CPPUNIT_TEST(testIsNetRemitFare_NoData_False);
  CPPUNIT_TEST(testIsNetRemitFare_MethodBlank_NoT979_False);
  CPPUNIT_TEST(testIsNetRemitFare_MethodBlank_WithT979_False);
  CPPUNIT_TEST(testIsNetRemitFare_MethodNotBlank_DisplayT_NoT979_True);
  CPPUNIT_TEST(testIsNetRemitFare_MethodNotBlank_DisplayC_NoT979_True);
  CPPUNIT_TEST(testIsNetRemitFare_MethodNotBlank_DisplayL_NoT979_False);
  CPPUNIT_TEST(testIsNetRemitFare_MethodNotBlank_DisplayL_WithT979_True);
  CPPUNIT_TEST(test_validateTFDCombination_Fare1Byte101_F_Fare2Byte101_A_False);
  CPPUNIT_TEST(test_validateTFDCombination_Fare1Byte101_F_Fare2Byte101_F_True);
  CPPUNIT_TEST(testSetNetRemitTicketIndWhenByte101_F);
  CPPUNIT_TEST(testSetNetRemitTicketIndWhenByte101_A);
  CPPUNIT_TEST(testSetNetRemitTicketIndWhenInd_N_NoUNFBC);
  CPPUNIT_TEST(testSetNetRemitTicketIndWhenInd_B_NoUNFBC);
  CPPUNIT_TEST(testSetNetRemitTicketIndWhenInd_B_WithUNFBC);
  CPPUNIT_TEST(testValidateTFDCombinationWhenTFDvsTFDSPC_Fail);
  CPPUNIT_TEST(testValidateTFDCombinationWhenTFDvsTFDSPC_Pass);
  CPPUNIT_TEST(testValidateTFDCombinationWhenTFDSPCvsTFD_Fail);
  CPPUNIT_TEST(testValidateTFDCombinationWhenTFDSPCvsTFD_Pass);
  CPPUNIT_TEST(testValidateTFDCombinationWhenTFDSPCvsTFDSPC_Fail);
  CPPUNIT_TEST(testValidateTFDCombinationWhenTFDSPCvsTFDSPC_Pass);
  CPPUNIT_TEST(testValidateTFDCombinationWhenTFDSPCvsTFDSPCAllBlank_Pass);

  CPPUNIT_TEST(testValidateCxrTourCodeMap_Pass_SameCxrSameTc);
  CPPUNIT_TEST(testValidateCxrTourCodeMap_Pass_DiffCxrSameTc);
  CPPUNIT_TEST(testValidateCxrTourCodeMap_Fail_SameCxrDiffTc);
  CPPUNIT_TEST(testValidateCxrTourCodeMap_Pass_SameCxrEmptyAndDiffTc);
  CPPUNIT_TEST(testValidateCxrTourCodeMap_Pass_DiffCxrEmptyAndDiffTc);
  CPPUNIT_TEST(testValidateCxrTourCodeMap_Fail_DiffCxrDiffTc);

  CPPUNIT_TEST(testGetPrintOptionReturnDefault3WhenNoPrintOptionFiled);
  CPPUNIT_TEST(testGetPrintOptionReturnPOWhenPrintOptionFiled);
  CPPUNIT_TEST(testValidatePrintOptionCombinationTrueWhenOneFare);
  CPPUNIT_TEST(testValidatePrintOptionCombinationTrueWhenSamePrintOption);
  CPPUNIT_TEST(testValidatePrintOptionCombinationFalseWhenDiffPrintOption);
  CPPUNIT_TEST(testValidatePrintOptionCombinationTrueWhenSamePrintOptionDefault);
  CPPUNIT_TEST(testValidatePrintOptionCombinationFalseWhenDiffPrintOptionDefault);

  CPPUNIT_TEST(testGetPrintOptionInfoWhenPrintOptionNotFound);
  CPPUNIT_TEST(testGetPrintOptionInfoWhenFoundPrintOption);
  CPPUNIT_TEST(testPrepareTourCodeValueCodeByPOWhenValueCodeIsResizedTo14);
  CPPUNIT_TEST(testPrepareTourCodeValueCodeByPOWhenValueCodeIsResizedTo15);
  CPPUNIT_TEST(testPrepareTourCodeValueCodeByPOWhenPrintOption1);
  CPPUNIT_TEST(testPrepareTourCodeValueCodeByPOWhenPrintOption2);
  CPPUNIT_TEST(testPrepareTourCodeValueCodeByPOWhenPrintOption4);
  CPPUNIT_TEST(testSaveTourCodeForPQWhenTKTRequest);
  CPPUNIT_TEST(testSaveTourCodeForPQWhenTourCodeExists);

  CPPUNIT_TEST(testHasFareAmountNoCurrency);
  CPPUNIT_TEST(testHasFareAmountZero);
  CPPUNIT_TEST(testHasFareAmount);
  CPPUNIT_TEST(testHasFareBoxNetRemitMethodNull);
  CPPUNIT_TEST(testHasFareBoxNetRemitMethod1);
  CPPUNIT_TEST(testHasFareBoxNetRemitMethod2);
  CPPUNIT_TEST(testHasFareBoxNetRemitMethod3);
  CPPUNIT_TEST(testValidateFareBoxCombinationNotANetRemitMethod);
  CPPUNIT_TEST(testValidateFareBoxCombinationNoFareAmount);
  CPPUNIT_TEST(testValidateFareBoxCombinationOneValidFare);
  CPPUNIT_TEST(testValidateFareBoxCombinationTwoValidFares);
  CPPUNIT_TEST(testValidateFareBoxCombinationFirstFareNotValid);
  CPPUNIT_TEST(testValidateFareBoxCombinationSecondFareNotValid);
  CPPUNIT_TEST(testValidateNetRemitMethod1NegFareRestValid);
  CPPUNIT_TEST(testValidateNetRemitMethod1NegFareRestInvalidNet);
  CPPUNIT_TEST(testValidateNetRemitMethod1NegFareRestInvalidTypeCode);
  CPPUNIT_TEST(testValidateNetRemitMethod1NegFareRestNotMethod1);
  CPPUNIT_TEST(testValidateNetRemitMethod1FaresVectorValid);
  CPPUNIT_TEST(testValidateNetRemitMethod1FaresVectorInvalid);

  CPPUNIT_TEST(testIsItBtTicketingDataInvalid);
  CPPUNIT_TEST(testIsItBtTicketingDataInvalidWhenTypeLNoT979IT);
  CPPUNIT_TEST(testIsItBtTicketingDataValidWhenTypeLWithT979IT);
  CPPUNIT_TEST(testIsItBtTicketingDataValidWhenTypeTIT);
  CPPUNIT_TEST(testIsItBtTicketingDataValidWhenTypeCBT);
  CPPUNIT_TEST(testIsItBtTicketingDataInvalidWhenTypeTNotITBT);
  CPPUNIT_TEST(testIsItBtTicketingDataInvalidWhenTypeCNotITBT);
  CPPUNIT_TEST(testisRegularNetWhenTypeCITBT);
  CPPUNIT_TEST(testisRegularNetWhenTypeCNetRemit);
  CPPUNIT_TEST(testisRegularNetWhenTypeCPass);
  CPPUNIT_TEST(testcheckTktDataCat35FareFail);
  CPPUNIT_TEST(testcheckTktDataCat35FarePass);
  CPPUNIT_TEST(testcheckTktDataCat35FareAxessUserFail);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _checker = _memHandle.create<NegotiatedFareRuleUtilMock>();
    _farePath = _memHandle.create<FarePath>();
    _farePath->itin() = _memHandle.create<Itin>();
    _cNegFareData = _memHandle.create<CollectedNegFareData>();
  }

  void tearDown() { _memHandle.clear(); }

  void createPricingUnit(bool createCat35Rec3)
  {
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    FareUsage* fu = _memHandle.create<FareUsage>();
    PaxTypeFare* ptf =
        createCat35Rec3 ? _memHandle.create<NegPaxTypeFare>() : _memHandle.create<PaxTypeFare>();

    _farePath->pricingUnit().push_back(pu);
    fu->paxTypeFare() = ptf;
    pu->fareUsage().push_back(fu);
  }

  void createNetRemitPscResults(FareUsage* fareUsage, bool uniqueFareBasis)
  {
    FareUsage::TktNetRemitPscResult* netRemitResult =
        _memHandle.create<FareUsage::TktNetRemitPscResult>();
    NegFareRestExtSeq* restExtSeq = _memHandle.create<NegFareRestExtSeq>();
    if (uniqueFareBasis)
      restExtSeq->uniqueFareBasis() = "U2345678";
    netRemitResult->_tfdpscSeqNumber = restExtSeq;
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->segmentOrder() = 1;
    netRemitResult->_startTravelSeg = seg;
    netRemitResult->_endTravelSeg = seg;
    fareUsage->netRemitPscResults().push_back(*netRemitResult);
  }

  void testCheckWarningMsgForTicketingPass()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::NO_WARNING;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForTicketing(*_farePath, *_trx));
  }

  void testCheckWarningMsgForTicketingNetAmtFlipped()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::NO_WARNING;
    _checker->_netAmtWasFlipped = true;
    _checker->_netTotalAmt = 200.0;
    _farePath->setTotalNUCAmount(100.0);
    CPPUNIT_ASSERT(_checker->checkWarningMsgForTicketing(*_farePath, *_trx));
    CPPUNIT_ASSERT_EQUAL(200.0, _farePath->getTotalNUCAmount());
  }

  void testCheckWarningMsgForTicketingNetAmtNotFlipped()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::NO_WARNING;
    _checker->_netAmtWasFlipped = false;
    _checker->_netTotalAmt = 200.0;
    _farePath->setTotalNUCAmount(100.0);
    CPPUNIT_ASSERT(_checker->checkWarningMsgForTicketing(*_farePath, *_trx));
    CPPUNIT_ASSERT_EQUAL(100.0, _farePath->getTotalNUCAmount());
  }

  void checkWarningMsgForTicketingThrow(int warningMsg, int exceptionCode)
  {
    createTrx();
    _checker->_warningMsg = (NegotiatedFareRuleUtil::WARNING_MSG)warningMsg;
    try { _checker->checkWarningMsgForTicketing(*_farePath, *_trx); }
    catch (tse::ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT_EQUAL((tse::ErrorResponseException::ErrorResponseCode)exceptionCode,
                           ex.code());
      return;
    }
    catch (...)
    {
      CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown", false);
      return;
    }
    CPPUNIT_ASSERT_MESSAGE("Exception expected", false);
  }

  void testCheckWarningMsgForTicketingThrowOnMultipleTourCode()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::MULTIPLE_TOUR_CODE,
                                     ErrorResponseException::UTAT_MULTIPLE_TOUR_CODES);
  }

  void testCheckWarningMsgForTicketingThrowOnTourCodeNotFound()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::TOUR_CODE_NOT_FOUND,
                                     ErrorResponseException::UTAT_TOUR_CODE_NOT_FOUND);
  }

  void testCheckWarningMsgForTicketingThrowOnInvalidItbtPsgCoupon()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON,
                                     ErrorResponseException::UTAT_INVALID_TEXT_BOX_COMBO);
  }

  void testCheckWarningMsgForTicketingThrowOnEmptyItbtPsgCoupon()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::EMPTY_ITBT_PSG_COUPON,
                                     ErrorResponseException::UTAT_INVALID_TEXT_BOX_COMBO);
  }

  void testCheckWarningMsgForTicketingThrowOnItbtBspNotBlank()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::ITBT_BSP_NOT_BLANK,
                                     ErrorResponseException::NET_REMIT_FARE_PHASE_FOUR);
  }

  void testCheckWarningMsgForTicketingThrowOnNotItbt()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::NOT_ITBT,
                                     ErrorResponseException::UNABLE_TO_PROCESS_NEG_FARE_DATA);
  }

  void testCheckWarningMsgForTicketingThrowOnMultipleFareBox()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::MULTIPLE_FARE_BOX,
                                     ErrorResponseException::ISSUE_SEPARATE_TICKET);
  }

  void testCheckWarningMsgForTicketingThrowOnMultipleBsp()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::MULTIPLE_BSP,
                                     ErrorResponseException::ISSUE_SEPARATE_TICKET);
  }

  void testCheckWarningMsgForTicketingThrowOnMultipleNetGross()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::MULTIPLE_NET_GROSS,
                                     ErrorResponseException::ISSUE_SEPARATE_TICKET);
  }

  void testCheckWarningMsgForTicketingThrowOnMixFares()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::MIX_FARES,
                                     ErrorResponseException::ISSUE_SEPARATE_TICKET);
  }

  void testCheckWarningMsgForTicketingThrowOnFaresNotCombinable()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::FARES_NOT_COMBINABLE,
                                     ErrorResponseException::UTAT_NET_SELLING_AMOUNTS_CONFLICT);
  }

  void testCheckWarningMsgForTicketingThrowOnNetSellingConflict()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::NET_SELLING_CONFLICT,
                                     ErrorResponseException::UTAT_NET_SELLING_AMOUNTS_CONFLICT);
  }

  void testCheckWarningMsgForTicketingThrowOnMultipleCommission()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::MULTIPLE_COMMISSION,
                                     ErrorResponseException::UTAT_COMMISSIONS_NOT_COMBINABLE);
  }

  void testCheckWarningMsgForTicketingThrowOnMixCommission()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::MIX_COMMISSION,
                                     ErrorResponseException::UTAT_COMMISSIONS_NOT_COMBINABLE);
  }

  void testCheckWarningMsgForTicketingThrowOnIssueSepareteTkt()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::ISSUE_SEPARATE_TKT,
                                     ErrorResponseException::ISSUE_SEPARATE_TICKET);
  }

  void testCheckWarningMsgForTicketingThrowOnInvalidNetRemitFare()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::INVALID_NET_REMIT_FARE,
                                     ErrorResponseException::UNABLE_AUTO_TKT_INV_NET_REMIT_FARE);
  }

  void testCheckWarningMsgForTicketingThrowOnInvalidNetRemitComm()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::INVALID_NET_REMIT_COMM,
                                     ErrorResponseException::FARE_REQUIRE_COMM_PERCENT);
  }

  void testCheckWarningMsgForTicketingThrowOnMultipleValueCode()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::MULTIPLE_VALUE_CODE,
                                     ErrorResponseException::UTAT_MULTIPLE_VALUE_CODES);
  }

  void testCheckWarningMsgForTicketingThrowOnMultiplePrintOption()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::MULTIPLE_PRINT_OPTION,
                                     ErrorResponseException::UTAT_MULTIPLE_PRINT_OPTIONS);
  }

  void testCheckWarningMsgForTicketingThrowOnMixedFareBoxAmount()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::MIXED_FARE_BOX_AMT,
                                     ErrorResponseException::UNABLE_AUTO_TKT_INV_NET_REMIT_FARE);
  }

  void testCheckWarningMsgForTicketingThrowOnSystemError()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::SYSTEM_ERROR,
                                     ErrorResponseException::SYSTEM_ERROR);
  }

  void testCheckWarningMsgForTicketingThrowOnInvalidNetRemitFareForByte101Conflict()
  {
    checkWarningMsgForTicketingThrow(NegotiatedFareRuleUtil::CONFLICTING_TFD_BYTE101,
                                     ErrorResponseException::UNABLE_AUTO_TKT_INV_NET_REMIT_FARE);
  }

  void testCheckWarningMsgForPricingPassNoWarning()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::NO_WARNING;
    _trx->getOptions()->cat35Net() = 'N';
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
  }

  void testCheckWarningMsgForPricingPassNoWarningPricingIsCat35Net()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::NO_WARNING;
    _trx->getOptions()->cat35Net() = 'Y';
    _cNegFareData->trailerMsg() = "Test Msg";
    _checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath);
    CPPUNIT_ASSERT_EQUAL(std::string("NET FARE AMOUNT FOR INFORMATIONAL PURPOSES ONLY"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassNoWarningPricingIsNotCat35Net()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::NO_WARNING;
    _trx->getOptions()->cat35Net() = 'N';
    _cNegFareData->trailerMsg() = "Test Msg";
    _checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath);
    CPPUNIT_ASSERT_EQUAL(std::string("Test Msg"), _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassNoWarningPricingIsCat35NetNetAmtWasFlipped()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::NO_WARNING;
    _checker->_netAmtWasFlipped = true;
    _trx->getOptions()->cat35Net() = 'Y';
    _farePath->setTotalNUCAmount(100);
    _checker->_netTotalAmt = 200.0;
    _checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath);
    CPPUNIT_ASSERT_EQUAL(200.0, _farePath->getTotalNUCAmount());
  }

  void testCheckWarningMsgForPricingPassNoWarningPricingIsCat35NetNetAmtWasNotFlipped()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::NO_WARNING;
    _checker->_netAmtWasFlipped = false;
    _trx->getOptions()->cat35Net() = 'Y';
    _farePath->setTotalNUCAmount(100);
    _checker->_netTotalAmt = 200.0;
    _checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath);
    CPPUNIT_ASSERT_EQUAL(100.0, _farePath->getTotalNUCAmount());
  }

  void testCheckWarningMsgForPricingPassWarningInvalidNetRemitComm()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::INVALID_NET_REMIT_COMM;
    _cNegFareData->trailerMsg() = "Test Msg";
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("FARE REQUIRES COMMISSION PERCENT"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassWarningAutoTktNotPermited()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::AUTO_TKT_NOT_PERMITTED;
    _cNegFareData->trailerMsg() = "Test Msg";
    _farePath->tktRestricted() = false;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT(_farePath->tktRestricted());
    CPPUNIT_ASSERT_EQUAL(std::string("AUTO TICKETING NOT PERMITTED"), _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassSetTrfRestrictedForAbacus()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::MIX_COMMISSION;
    _checker->_abacusUser = true;
    _farePath->tfrRestricted() = false;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT(_farePath->tfrRestricted());
  }

  void testCheckWarningMsgForPricingPassSetTrfRestrictedForNonAbacus()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::MIX_COMMISSION;
    _checker->_abacusUser = false;
    _farePath->tfrRestricted() = false;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT(!_farePath->tfrRestricted());
  }

  void testCheckWarningMsgForPricingPassWarningMixCommissionTraileMsgForAbacus()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::MIX_COMMISSION;
    _checker->_abacusUser = true;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO AUTO TICKET - COMMISSIONS NOT COMBINABLE"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassWarningMultipleCommissionTraileMsgForAbacus()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::MULTIPLE_COMMISSION;
    _checker->_abacusUser = true;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO AUTO TICKET - COMMISSIONS NOT COMBINABLE"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassWarningMultipleTourCode()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::MULTIPLE_TOUR_CODE;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO AUTO TICKET - MULTIPLE TOUR CODES"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassWarningTourCodeNotFound()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::TOUR_CODE_NOT_FOUND;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO AUTO TICKET - TOUR CODE NOT FOUND"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassWarningNotItbt()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::NOT_ITBT;
    _farePath->tktRestricted() = false;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO AUTO TICKET - INVALID TEXT BOX COMBINATION"),
                         _cNegFareData->trailerMsg());
    CPPUNIT_ASSERT(_farePath->tktRestricted());
  }

  void testCheckWarningMsgForPricingPassWarningInvalidItbtPsgCoupon()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON;
    _farePath->tktRestricted() = false;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO AUTO TICKET - INVALID TEXT BOX COMBINATION"),
                         _cNegFareData->trailerMsg());
    CPPUNIT_ASSERT(_farePath->tktRestricted());
  }

  void testCheckWarningMsgForPricingPassWarningEmptyItbtPsgCoupon()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::EMPTY_ITBT_PSG_COUPON;
    _farePath->tktRestricted() = false;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO AUTO TICKET - INVALID TEXT BOX COMBINATION"),
                         _cNegFareData->trailerMsg());
    CPPUNIT_ASSERT(_farePath->tktRestricted());
  }

  void testCheckWarningMsgForPricingPassWarningMultipleFareBox()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::MULTIPLE_FARE_BOX;
    _farePath->tktRestricted() = false;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO AUTO TICKET - INVALID TEXT BOX COMBINATION"),
                         _cNegFareData->trailerMsg());
    CPPUNIT_ASSERT(_farePath->tktRestricted());
  }

  void testCheckWarningMsgForPricingPassWarningItbtBspNotBlank()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::ITBT_BSP_NOT_BLANK;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("NET REMIT FARE - PHASE 4 AND USE NET/ FOR TKT ISSUANCE"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassWarningMultipleNetGross()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::MULTIPLE_NET_GROSS;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO AUTO TICKET - COMMISSIONS NOT COMBINABLE"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassWarningMultipleBsp()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::MULTIPLE_BSP;
    _farePath->tktRestricted() = false;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("ISSUE SEPARATE TICKETS"), _cNegFareData->trailerMsg());
    CPPUNIT_ASSERT(_farePath->tktRestricted());
  }

  void testCheckWarningMsgForPricingPassWarningIssueSeparateTkt()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::ISSUE_SEPARATE_TKT;
    _farePath->tktRestricted() = false;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("ISSUE SEPARATE TICKETS"), _cNegFareData->trailerMsg());
    CPPUNIT_ASSERT(_farePath->tktRestricted());
  }

  void testCheckWarningMsgForPricingPassWarningMixFares()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::MIX_FARES;
    _farePath->tktRestricted() = false;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("ISSUE SEPARATE TICKETS"), _cNegFareData->trailerMsg());
    CPPUNIT_ASSERT(_farePath->tktRestricted());
  }

  void testCheckWarningMsgForPricingWarningNoNetFareAmountThrow()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::NO_NET_FARE_AMOUNT;
    try { _checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath); }
    catch (tse::ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT_EQUAL(ErrorResponseException::NO_NET_FARE_AMOUNT, ex.code());
      CPPUNIT_ASSERT_EQUAL(std::string("NO NET FARE AMOUNT"), _cNegFareData->trailerMsg());
      return;
    }
    catch (...)
    {
      CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown", false);
      return;
    }
    CPPUNIT_ASSERT_MESSAGE("Exception expected", false);
  }

  void testCheckWarningMsgForPricingWarningFaresNotCombinableNoNetThrow()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::FARES_NOT_COMBINABLE_NO_NET;
    try { _checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath); }
    catch (tse::ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT_EQUAL(ErrorResponseException::NO_NET_FARE_AMOUNT, ex.code());
      CPPUNIT_ASSERT_EQUAL(std::string("FARES NOT COMBINABLE"), _cNegFareData->trailerMsg());
      return;
    }
    catch (...)
    {
      CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown", false);
      return;
    }
    CPPUNIT_ASSERT_MESSAGE("Exception expected", false);
  }

  void testCheckWarningMsgForPricingPassWarningFaresNotCombinable()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::FARES_NOT_COMBINABLE;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO AUTO TICKET - NET/SELLING AMOUNTS CONFLICT"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassWarningNetSellingConflict()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::NET_SELLING_CONFLICT;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO AUTO TICKET - NET/SELLING AMOUNTS CONFLICT"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingWarningSystemErrorThrow()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::SYSTEM_ERROR;
    try { _checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath); }
    catch (tse::ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT_EQUAL(ErrorResponseException::SYSTEM_ERROR, ex.code());
      CPPUNIT_ASSERT_EQUAL(std::string("SYSTEM ERROR"), _cNegFareData->trailerMsg());
      return;
    }
    catch (...)
    {
      CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown", false);
      return;
    }
    CPPUNIT_ASSERT_MESSAGE("Exception expected", false);
  }

  void testCheckWarningMsgForPricingPassWarningInvalidNetRemitFare()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::INVALID_NET_REMIT_FARE;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID NET REMIT FARE - UNABLE TO AUTO TICKET"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassWarningNetFareAmountExceedsFare()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::NET_FARE_AMOUNT_EXCEEDS_FARE;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("NET AMOUNT EXCEEDS FARE - VERIFY NET AMOUNT"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassWarningInvalidNetRemitFareWhenConflictTFDByte101()
  {
    createTrx();
    createAgent();
    enableAbacus();

    _checker->_warningMsg = NegotiatedFareRuleUtil::CONFLICTING_TFD_BYTE101;
    _checker->_abacusUser = true;
    _checker->_isCmdPricing = true;

    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID NET REMIT FARE - UNABLE TO AUTO TICKET"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassWarningMultipleValueCode()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::MULTIPLE_VALUE_CODE;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO AUTO TICKET - MULTIPLE VALUE CODES"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassWarningMultiplePrintOption()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::MULTIPLE_PRINT_OPTION;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO AUTO TICKET - MULTIPLE PRINT OPTIONS"),
                         _cNegFareData->trailerMsg());
  }

  void testCheckWarningMsgForPricingPassWarningMixedFareBoxAmount()
  {
    createTrx();
    _checker->_warningMsg = NegotiatedFareRuleUtil::MIXED_FARE_BOX_AMT;
    CPPUNIT_ASSERT(_checker->checkWarningMsgForPricing(_cNegFareData, *_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID NET REMIT FARE - UNABLE TO AUTO TICKET"),
                         _cNegFareData->trailerMsg());
  }

  void testHandleCat35ProcessingPassTktIndicatorTrue()
  {
    createTrx();
    _checker->_indicatorTkt = true;
    _trx->getOptions()->cat35Net() = 'N';
    CPPUNIT_ASSERT(_checker->handleCat35Processing(_cNegFareData, *_trx, *_farePath));
  }

  void testHandleCat35ProcessingPassTktIndicatorFalse()
  {
    createTrx();
    _checker->_indicatorTkt = false;
    _trx->getOptions()->cat35Net() = 'N';
    CPPUNIT_ASSERT(_checker->handleCat35Processing(_cNegFareData, *_trx, *_farePath));
  }

  void testProcessNegFareItbtPassWhenAxessUser()
  {
    createTrx();
    createAgent();
    _trx->getOptions()->cat35Net() = 'N';
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1J";
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "AXES";

    CPPUNIT_ASSERT(_checker->processNegFareITBT(*_trx, *_farePath));
  }

  void testProcessNegFareItbtPassWhenIsNotCat35Net()
  {
    createTrx();
    createAgent();
    CPPUNIT_ASSERT(_checker->processNegFareITBT(*_trx, *_farePath));
  }

  void testProcessNegFareItbtCheckWarningMsgForInvalidNetRemComm()
  {
    createTrx();
    createAgent();
    _checker->_warningMsg = NegotiatedFareRuleUtil::NO_WARNING;
    _checker->_invalidNetRemComm = true;

    _checker->processNegFareITBT(*_trx, *_farePath);

    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::INVALID_NET_REMIT_COMM, _checker->getWarningMsg());
  }

  void testProcessNegFareItbtThrowWhenNoAltPricing()
  {
    createTrx();
    createAgent();
    _trx->getOptions()->cat35Net() = 'Y';

    try { _checker->processNegFareITBT(*_trx, *_farePath); }
    catch (tse::ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT_EQUAL(ErrorResponseException::NO_NET_FARE_AMOUNT, ex.code());
      CPPUNIT_ASSERT_EQUAL(std::string("NO NET FARE AMOUNT"),
                           _farePath->collectedNegFareData()->trailerMsg());
      return;
    }
    catch (...)
    {
      CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown", false);
      return;
    }
    CPPUNIT_ASSERT_MESSAGE("Exception expected", false);
  }

  void testProcessNegFareItbtFailWhenAbacusUser()
  {
    _checker->_abacusUser = true;
    createAltPricingTrx();
    createAgent();
    _trx->altTrxType() = AltPricingTrx::WPA;
    _trx->getOptions()->cat35Net() = 'Y';

    CPPUNIT_ASSERT(_checker->processNegFareITBT(*_trx, *_farePath));
  }

  void testProcessNetRemitPassWhenNoPricingUnits()
  {
    createTrx();
    CPPUNIT_ASSERT(_checker->processNetRemit(*_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::NO_WARNING, _checker->getWarningMsg());
  }

  void testProcessNetRemitFalseNotCat35()
  {
    createTrx();
    createPricingUnit(false);

    CPPUNIT_ASSERT(!_checker->processNetRemit(*_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::SYSTEM_ERROR, _checker->getWarningMsg());
  }

  void testProcessNetRemitPass()
  {
    createTrx();
    createPricingUnit(true);

    CPPUNIT_ASSERT(_checker->processNetRemit(*_trx, *_farePath));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::NO_WARNING, _checker->getWarningMsg());
  }

  void testcheckFareBoxTextITBT_IFITAndBT()
  {
    CPPUNIT_ASSERT(!_checker->checkFareBoxTextITBT("IT", "BT"));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON,
                         _checker->getWarningMsg());
  }

  void testcheckFareBoxTextITBT_IFBTAndIT()
  {
    CPPUNIT_ASSERT(!_checker->checkFareBoxTextITBT("BT", "IT"));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON,
                         _checker->getWarningMsg());
  }

  void testcheckFareBoxTextITBT_IFBlankAndIT()
  {
    CPPUNIT_ASSERT(!_checker->checkFareBoxTextITBT("", "IT"));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON,
                         _checker->getWarningMsg());
  }

  void testcheckFareBoxTextITBT_IFBlankAndBT()
  {
    CPPUNIT_ASSERT(!_checker->checkFareBoxTextITBT("", "BT"));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON,
                         _checker->getWarningMsg());
  }

  void testcheckFareBoxTextITBT_IFBlankAndBlank()
  {
    CPPUNIT_ASSERT(_checker->checkFareBoxTextITBT("", ""));
  }

  void testcheckFareBox_Blank() { CPPUNIT_ASSERT(!_checker->checkFareBox("")); }

  void testcheckFareBox_IT() { CPPUNIT_ASSERT(_checker->checkFareBox("IT")); }

  void testcheckFareBox_BT() { CPPUNIT_ASSERT(_checker->checkFareBox("BT")); }

  void testcheckFareBox_YT() { CPPUNIT_ASSERT(!_checker->checkFareBox("YT")); }

  void testvalidateFareBoxText()
  {
    NegFareRest negFareRest;

    createTrx();
    negFareRest.noSegs() = NegotiatedFareRuleUtil::ONE_SEGMENT;
    negFareRest.fareBoxText1() = NegotiatedFareRuleUtil::IT_TICKET;
    negFareRest.couponInd1() = NegotiatedFareRuleUtil::AUDIT_COUPON;

    CPPUNIT_ASSERT(!_checker->validateFareBoxText(&negFareRest));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::NOT_ITBT, _checker->getWarningMsg());
  }

  void testvalidateFareBoxText_FBT2Segs_Blank_And_IT()
  {
    NegFareRest negFareRest;

    createTrx();
    negFareRest.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    negFareRest.fareBoxText1() = "";
    negFareRest.fareBoxText2() = "IT";
    negFareRest.couponInd1() = NegotiatedFareRuleUtil::PSG_COUPON;
    CPPUNIT_ASSERT(!_checker->validateFareBoxText(&negFareRest));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON,
                         _checker->getWarningMsg());
  }

  void testvalidateFareBoxText_FBT2Segs_Blank_And_BT()
  {
    NegFareRest negFareRest;

    createTrx();
    negFareRest.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    negFareRest.fareBoxText1() = "";
    negFareRest.fareBoxText2() = "BT";
    negFareRest.couponInd1() = NegotiatedFareRuleUtil::PSG_COUPON;

    CPPUNIT_ASSERT(!_checker->validateFareBoxText(&negFareRest));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON,
                         _checker->getWarningMsg());
  }

  void testvalidateFareBoxText_FBT2Segs_IT_And_BT()
  {
    NegFareRest negFareRest;

    createTrx();
    negFareRest.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    negFareRest.fareBoxText1() = "IT";
    negFareRest.fareBoxText2() = "BT";
    negFareRest.couponInd1() = NegotiatedFareRuleUtil::PSG_COUPON;

    CPPUNIT_ASSERT(!_checker->validateFareBoxText(&negFareRest));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON,
                         _checker->getWarningMsg());
  }

  void testvalidateFareBoxText_FBT2Segs_BT_And_IT()
  {
    NegFareRest negFareRest;

    createTrx();
    negFareRest.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    negFareRest.fareBoxText1() = "BT";
    negFareRest.fareBoxText2() = "IT";
    negFareRest.couponInd1() = NegotiatedFareRuleUtil::PSG_COUPON;

    CPPUNIT_ASSERT(!_checker->validateFareBoxText(&negFareRest));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON,
                         _checker->getWarningMsg());
  }

  void testvalidateFareBoxText_FBT2Segs_IT_AND_BLANK()
  {
    NegFareRest negFareRest;

    createTrx();
    negFareRest.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    negFareRest.fareBoxText1() = "IT";
    negFareRest.fareBoxText2() = "";
    negFareRest.couponInd2() = NegotiatedFareRuleUtil::PSG_COUPON;
    CPPUNIT_ASSERT(!_checker->validateFareBoxText(&negFareRest));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON,
                         _checker->getWarningMsg());
  }

  void testvalidateFareBoxText_FBT2Segs_BT_AND_BLANK()
  {
    NegFareRest negFareRest;

    createTrx();
    negFareRest.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    negFareRest.fareBoxText1() = "BT";
    negFareRest.fareBoxText2() = "";
    negFareRest.couponInd2() = NegotiatedFareRuleUtil::PSG_COUPON;

    CPPUNIT_ASSERT(!_checker->validateFareBoxText(&negFareRest));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON,
                         _checker->getWarningMsg());
  }

  void testvalidateFareBoxText_FBT2Segs_IT_And_BT_Coupon2()
  {
    NegFareRest negFareRest;

    createTrx();
    negFareRest.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    negFareRest.fareBoxText1() = "IT";
    negFareRest.fareBoxText2() = "BT";
    negFareRest.couponInd2() = NegotiatedFareRuleUtil::PSG_COUPON;

    CPPUNIT_ASSERT(!_checker->validateFareBoxText(&negFareRest));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON,
                         _checker->getWarningMsg());
  }

  void testvalidateFareBoxText_FBT2Segs_BT_And_IT_Coupon2()
  {
    NegFareRest negFareRest;

    createTrx();
    negFareRest.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    negFareRest.fareBoxText1() = "BT";
    negFareRest.fareBoxText2() = "IT";
    negFareRest.couponInd2() = NegotiatedFareRuleUtil::PSG_COUPON;

    CPPUNIT_ASSERT(!_checker->validateFareBoxText(&negFareRest));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON,
                         _checker->getWarningMsg());
  }

protected:
  void createTrx()
  {
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
  }

  void createAltPricingTrx()
  {
    _trx = _memHandle.create<AltPricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->setRequest(_memHandle.create<PricingRequest>());
  }

  void createAgent()
  {
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx->getRequest()->ticketingAgent()->agentTJR() = _memHandle.create<Customer>();
  }

  void enableAbacus()
  {
    TrxUtil::abacusEnabled = true;
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1B";
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "ABAC";
  }

  void testProcessCat35NetTicketingFareWhenMixedFares()
  {
    createTrx();
    _cNegFareData->bspMethod() = RuleConst::NRR_METHOD_2;
    _checker->_isCmdPricing = true;
    _checker->_bspMethod = RuleConst::NRR_METHOD_BLANK;
    _checker->processCat35NetTicketingFare(_cNegFareData);
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::MIXED_FARES_COMBINATION,
                         _checker->getWarningMsg());
  }

  void testProcessCat35NetTicketingFareWhenMethodTypeInConflict()
  {
    createTrx();
    _cNegFareData->bspMethod() = RuleConst::NRR_METHOD_2;
    _checker->_isCmdPricing = true;
    _checker->_bspMethod = RuleConst::NRR_METHOD_3;
    _checker->processCat35NetTicketingFare(_cNegFareData);
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::MULTIPLE_BSP, _checker->getWarningMsg());
  }

  void testProcessCat35NetTicketingFareWhenNetGrossIndInConflict()
  {
    createTrx();
    _cNegFareData->indNetGross() = 'B';
    _checker->_isCmdPricing = true;
    _checker->_indNetGross = 'G';
    _checker->processCat35NetTicketingFare(_cNegFareData);
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::MULTIPLE_NET_GROSS, _checker->getWarningMsg());
  }

  void testProcessCat35NetTicketingFareWhenCommissionInConflictForTicketing()
  {
    createTrx();
    _cNegFareData->comPercent() = 5;
    _checker->_indicatorTkt = true;
    _checker->_currency = "USD";
    _checker->processCat35NetTicketingFare(_cNegFareData);
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::MIX_COMMISSION, _checker->getWarningMsg());
  }

  void testProcessCat35NetTicketingFareWhenCommissionPercentInConflict()
  {
    createTrx();
    _cNegFareData->comPercent() = 5;
    _checker->_isCmdPricing = true;
    _checker->_comPercent = 10;
    _checker->processCat35NetTicketingFare(_cNegFareData);
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::MULTIPLE_COMMISSION, _checker->getWarningMsg());
  }

  void testProcessCat35NetTicketingFareWhenPreviousFareHasNoTourCode()
  {
    createTrx();
    _cNegFareData->tourCode().clear();
    _checker->_tourCode = "ABC";
    _checker->processCat35NetTicketingFare(_cNegFareData);
    CPPUNIT_ASSERT_EQUAL(std::string("ABC"), _cNegFareData->tourCode());
  }

  void testProcessCat35NetTicketingFareWhenPreviousFareHasTourCode()
  {
    createTrx();
    _cNegFareData->tourCode() = "ABC";
    _checker->_tourCode = "XYZ";
    _checker->processCat35NetTicketingFare(_cNegFareData);
    CPPUNIT_ASSERT_EQUAL(std::string("ABC"), _cNegFareData->tourCode());
  }

  void testProcessCat35NetTicketingFareWhenPreviousFareHasNoFareBox()
  {
    createTrx();
    _cNegFareData->fareBox().clear();
    _checker->_fareBox = "IT";
    _checker->processCat35NetTicketingFare(_cNegFareData);
    CPPUNIT_ASSERT_EQUAL(std::string("IT"), _cNegFareData->fareBox());
  }

  void testProcessCat35NetTicketingFareWhenPreviousFareHasFareBox()
  {
    createTrx();
    _cNegFareData->fareBox() = "IT";
    _checker->_fareBox = "BT";
    _checker->processCat35NetTicketingFare(_cNegFareData);
    CPPUNIT_ASSERT_EQUAL(std::string("IT"), _cNegFareData->fareBox());
  }

  void test_validateEndorsementsForAbacus_ConflictEndorsements_NoWarning()
  {
    createTrx();
    createAgent();
    enableAbacus();

    _checker->_abacusUser = true;
    _checker->_endorsementTxt = "VALID ON BI ONLY";
    const std::string endorsement;
    CPPUNIT_ASSERT(!_checker->validateEndorsements(*_trx, endorsement));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::NO_WARNING, _checker->_warningMsg);
  }

  void testIsNetRemitFare_NoData_False()
  {
    CPPUNIT_ASSERT_EQUAL(false, _checker->isNetRemitFare(RuleConst::NET_SUBMIT_FARE, 0));
  }

  void testIsNetRemitFare_MethodBlank_NoT979_False()
  {
    NegFareRest negFareRest;

    negFareRest.netRemitMethod() = RuleConst::NRR_METHOD_BLANK;
    negFareRest.negFareCalcTblItemNo() = 0;

    CPPUNIT_ASSERT_EQUAL(false, _checker->isNetRemitFare(RuleConst::NET_SUBMIT_FARE, &negFareRest));
  }

  void testIsNetRemitFare_MethodBlank_WithT979_False()
  {
    NegFareRest negFareRest;

    negFareRest.netRemitMethod() = RuleConst::NRR_METHOD_BLANK;
    negFareRest.negFareCalcTblItemNo() = 1000;

    CPPUNIT_ASSERT_EQUAL(false, _checker->isNetRemitFare(RuleConst::NET_SUBMIT_FARE, &negFareRest));
  }

  void testIsNetRemitFare_MethodNotBlank_DisplayT_NoT979_True()
  {
    NegFareRest negFareRest;

    negFareRest.netRemitMethod() = RuleConst::NRR_METHOD_1;
    negFareRest.negFareCalcTblItemNo() = 0;

    CPPUNIT_ASSERT_EQUAL(true, _checker->isNetRemitFare(RuleConst::NET_SUBMIT_FARE, &negFareRest));
  }

  void testIsNetRemitFare_MethodNotBlank_DisplayC_NoT979_True()
  {
    NegFareRest negFareRest;

    negFareRest.netRemitMethod() = RuleConst::NRR_METHOD_1;
    negFareRest.negFareCalcTblItemNo() = 0;

    CPPUNIT_ASSERT_EQUAL(true,
                         _checker->isNetRemitFare(RuleConst::NET_SUBMIT_FARE_UPD, &negFareRest));
  }

  void testIsNetRemitFare_MethodNotBlank_DisplayL_NoT979_False()
  {
    NegFareRest negFareRest;

    negFareRest.netRemitMethod() = RuleConst::NRR_METHOD_1;
    negFareRest.negFareCalcTblItemNo() = 0;

    CPPUNIT_ASSERT_EQUAL(false, _checker->isNetRemitFare(RuleConst::SELLING_FARE, &negFareRest));
  }

  void testIsNetRemitFare_MethodNotBlank_DisplayL_WithT979_True()
  {
    NegFareRest negFareRest;

    negFareRest.netRemitMethod() = RuleConst::NRR_METHOD_1;
    negFareRest.negFareCalcTblItemNo() = 1000;

    CPPUNIT_ASSERT_EQUAL(true, _checker->isNetRemitFare(RuleConst::SELLING_FARE, &negFareRest));
  }

  void test_validateTFDCombination_Fare1Byte101_F_Fare2Byte101_A_False()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;

    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::NR_VALUE_F);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_2,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::NR_VALUE_A);
    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(false, _checker->validateTFDCombination(*_trx, paxTypeFares));
  }

  void test_validateTFDCombination_Fare1Byte101_F_Fare2Byte101_F_True()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;

    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::NR_VALUE_F);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_2,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::NR_VALUE_F);

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _checker->validateTFDCombination(*_trx, paxTypeFares));
  }

  void testSetNetRemitTicketIndWhenByte101_F()
  {
    _farePath->collectedNegFareData() = _cNegFareData;
    _checker->_tktFareDataInd = RuleConst::NR_VALUE_F;
    _checker->setNetRemitTicketInd(*_trx, *_farePath);
    CPPUNIT_ASSERT(_cNegFareData->netRemitTicketInd());
  }

  void testSetNetRemitTicketIndWhenByte101_A()
  {
    _farePath->collectedNegFareData() = _cNegFareData;
    _checker->_tktFareDataInd = RuleConst::NR_VALUE_A;
    _checker->setNetRemitTicketInd(*_trx, *_farePath);
    CPPUNIT_ASSERT(_cNegFareData->netRemitTicketInd());
  }

  void testSetNetRemitTicketIndWhenInd_N_NoUNFBC()
  {
    createTrx();
    createPricingUnit(true);
    _farePath->collectedNegFareData() = _cNegFareData;
    _checker->_tktFareDataInd = RuleConst::NR_VALUE_N;
    _checker->setNetRemitTicketInd(*_trx, *_farePath);
    CPPUNIT_ASSERT(!_cNegFareData->netRemitTicketInd());
  }

  void testSetNetRemitTicketIndWhenInd_B_NoUNFBC()
  {
    createTrx();
    createPricingUnit(true);
    createNetRemitPscResults(_farePath->pricingUnit().front()->fareUsage().front(), false);
    _farePath->collectedNegFareData() = _cNegFareData;
    _checker->_tktFareDataInd = RuleConst::NR_VALUE_B;
    _checker->setNetRemitTicketInd(*_trx, *_farePath);
    CPPUNIT_ASSERT(!_cNegFareData->netRemitTicketInd());
  }

  void testSetNetRemitTicketIndWhenInd_B_WithUNFBC()
  {
    createTrx();
    createPricingUnit(true);
    createNetRemitPscResults(_farePath->pricingUnit().front()->fareUsage().front(), true);
    _farePath->collectedNegFareData() = _cNegFareData;
    _checker->_tktFareDataInd = RuleConst::NR_VALUE_B;
    _checker->setNetRemitTicketInd(*_trx, *_farePath);
    CPPUNIT_ASSERT(_cNegFareData->netRemitTicketInd());
  }

  void testValidateTFDCombinationWhenTFDvsTFDSPC_Fail()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;

    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::NR_VALUE_F);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_2,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::BLANK);

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    NegFareRestExt negFareRestExt;
    paxTypeFare2.negRuleData().negFareRestExt() = &negFareRestExt;
    negFareRestExt.fareBasisAmtInd() = RuleConst::NR_VALUE_B;

    CPPUNIT_ASSERT(!_checker->validateTFDCombination(*_trx, paxTypeFares));
  }

  void testValidateTFDCombinationWhenTFDvsTFDSPC_Pass()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;

    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::NR_VALUE_F);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_2,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::BLANK);

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    NegFareRestExt negFareRestExt;
    paxTypeFare2.negRuleData().negFareRestExt() = &negFareRestExt;
    negFareRestExt.fareBasisAmtInd() = RuleConst::NR_VALUE_F;

    CPPUNIT_ASSERT(_checker->validateTFDCombination(*_trx, paxTypeFares));
  }

  void testValidateTFDCombinationWhenTFDSPCvsTFD_Fail()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;

    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::BLANK);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_2,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::NR_VALUE_F);

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    NegFareRestExt negFareRestExt;
    paxTypeFare1.negRuleData().negFareRestExt() = &negFareRestExt;
    negFareRestExt.fareBasisAmtInd() = RuleConst::NR_VALUE_B;

    CPPUNIT_ASSERT(!_checker->validateTFDCombination(*_trx, paxTypeFares));
  }

  void testValidateTFDCombinationWhenTFDSPCvsTFD_Pass()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;

    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::BLANK);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_2,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::NR_VALUE_B);

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    NegFareRestExt negFareRestExt;
    paxTypeFare1.negRuleData().negFareRestExt() = &negFareRestExt;
    negFareRestExt.fareBasisAmtInd() = RuleConst::NR_VALUE_B;

    CPPUNIT_ASSERT(_checker->validateTFDCombination(*_trx, paxTypeFares));
  }

  void testValidateTFDCombinationWhenTFDSPCvsTFDSPC_Fail()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;

    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::BLANK);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_2,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::BLANK);

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    NegFareRestExt negFareRestExt1;
    paxTypeFare1.negRuleData().negFareRestExt() = &negFareRestExt1;
    negFareRestExt1.fareBasisAmtInd() = RuleConst::NR_VALUE_B;

    NegFareRestExt negFareRestExt2;
    paxTypeFare2.negRuleData().negFareRestExt() = &negFareRestExt2;
    negFareRestExt2.fareBasisAmtInd() = RuleConst::NR_VALUE_F;

    CPPUNIT_ASSERT(!_checker->validateTFDCombination(*_trx, paxTypeFares));
  }

  void testValidateTFDCombinationWhenTFDSPCvsTFDSPC_Pass()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;

    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::BLANK);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_2,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::BLANK);

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    NegFareRestExt negFareRestExt1;
    paxTypeFare1.negRuleData().negFareRestExt() = &negFareRestExt1;
    negFareRestExt1.fareBasisAmtInd() = RuleConst::NR_VALUE_F;

    NegFareRestExt negFareRestExt2;
    paxTypeFare2.negRuleData().negFareRestExt() = &negFareRestExt2;
    negFareRestExt2.fareBasisAmtInd() = RuleConst::NR_VALUE_F;

    CPPUNIT_ASSERT(_checker->validateTFDCombination(*_trx, paxTypeFares));
  }

  void testValidateTFDCombinationWhenTFDSPCvsTFDSPCAllBlank_Pass()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;

    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::BLANK);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_2,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::BLANK);

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    NegFareRestExt negFareRestExt1;
    paxTypeFare1.negRuleData().negFareRestExt() = &negFareRestExt1;
    negFareRestExt1.fareBasisAmtInd() = RuleConst::BLANK;

    NegFareRestExt negFareRestExt2;
    paxTypeFare2.negRuleData().negFareRestExt() = &negFareRestExt2;
    negFareRestExt2.fareBasisAmtInd() = RuleConst::BLANK;

    CPPUNIT_ASSERT(_checker->validateTFDCombination(*_trx, paxTypeFares));
  }

  void testValidateCxrTourCodeMap_Pass_SameCxrSameTc()
  {
    std::multimap<CarrierCode, std::string> cxrTcMap;
    cxrTcMap.insert(std::pair<CarrierCode, std::string>("AA", "TC1"));
    cxrTcMap.insert(std::pair<CarrierCode, std::string>("AA", "TC1"));

    CPPUNIT_ASSERT(_checker->validateCxrTourCodeMap(cxrTcMap));
  }

  void testValidateCxrTourCodeMap_Pass_DiffCxrSameTc()
  {
    std::multimap<CarrierCode, std::string> cxrTcMap;
    cxrTcMap.insert(std::pair<CarrierCode, std::string>("AA", "TC1"));
    cxrTcMap.insert(std::pair<CarrierCode, std::string>("BB", "TC1"));

    CPPUNIT_ASSERT(_checker->validateCxrTourCodeMap(cxrTcMap));
  }

  void testValidateCxrTourCodeMap_Fail_SameCxrDiffTc()
  {
    std::multimap<CarrierCode, std::string> cxrTcMap;
    cxrTcMap.insert(std::pair<CarrierCode, std::string>("AA", "TC1"));
    cxrTcMap.insert(std::pair<CarrierCode, std::string>("AA", "TC2"));

    CPPUNIT_ASSERT(!_checker->validateCxrTourCodeMap(cxrTcMap));
  }

  void testValidateCxrTourCodeMap_Pass_SameCxrEmptyAndDiffTc()
  {
    std::multimap<CarrierCode, std::string> cxrTcMap;
    cxrTcMap.insert(std::pair<CarrierCode, std::string>("AA", "TC1"));
    cxrTcMap.insert(std::pair<CarrierCode, std::string>("AA", ""));

    CPPUNIT_ASSERT(_checker->validateCxrTourCodeMap(cxrTcMap));
  }

  void testValidateCxrTourCodeMap_Pass_DiffCxrEmptyAndDiffTc()
  {
    std::multimap<CarrierCode, std::string> cxrTcMap;
    cxrTcMap.insert(std::pair<CarrierCode, std::string>("AA", "TC1"));
    cxrTcMap.insert(std::pair<CarrierCode, std::string>("AB", ""));

    CPPUNIT_ASSERT(_checker->validateCxrTourCodeMap(cxrTcMap));
  }

  void testValidateCxrTourCodeMap_Fail_DiffCxrDiffTc()
  {
    std::multimap<CarrierCode, std::string> cxrTcMap;
    cxrTcMap.insert(std::pair<CarrierCode, std::string>("AA", "TC1"));
    cxrTcMap.insert(std::pair<CarrierCode, std::string>("BB", "TC2"));

    CPPUNIT_ASSERT(!_checker->validateCxrTourCodeMap(cxrTcMap));
  }

  void testGetPrintOptionReturnDefault3WhenNoPrintOptionFiled()
  {
    NegPaxTypeFare ptf(RuleConst::NRR_METHOD_2,
                       RuleConst::NET_SUBMIT_FARE,
                       false,
                       false,
                       RuleConst::BLANK,
                       "TEST",
                       1,
                       'T',
                       "TEST",
                       'T',
                       "TEST",
                       RuleConst::BLANK);

    Indicator testI = '3'; // default

    CPPUNIT_ASSERT_EQUAL(testI, NegotiatedFareRuleUtil::getPrintOption(ptf));
  }

  void testGetPrintOptionReturnPOWhenPrintOptionFiled()
  {
    NegPaxTypeFare ptf(RuleConst::NRR_METHOD_2,
                       RuleConst::NET_SUBMIT_FARE,
                       false,
                       false,
                       RuleConst::BLANK,
                       "TEST",
                       1,
                       'T',
                       "TEST",
                       'T',
                       "TEST",
                       RuleConst::BLANK);
    PrintOption po;
    ptf.negRuleData().printOption() = &po;
    po.printOption() = '1';

    Indicator testI = '1'; // PO=1

    CPPUNIT_ASSERT_EQUAL(testI, NegotiatedFareRuleUtil::getPrintOption(ptf));
  }

  void testValidatePrintOptionCombinationTrueWhenOneFare()
  {
    NegPaxTypeFare ptf(RuleConst::NRR_METHOD_2,
                       RuleConst::NET_SUBMIT_FARE,
                       false,
                       false,
                       RuleConst::BLANK,
                       "TEST",
                       1,
                       'T',
                       "TEST",
                       'T',
                       "TEST",
                       RuleConst::BLANK);
    PrintOption po;
    ptf.negRuleData().printOption() = &po;
    po.printOption() = '1';

    std::vector<const PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(&ptf);

    CPPUNIT_ASSERT(NegotiatedFareRuleUtil::validatePrintOptionCombination(paxTypeFares));
  }

  void testValidatePrintOptionCombinationTrueWhenSamePrintOption()
  {
    NegPaxTypeFare ptf1(RuleConst::NRR_METHOD_2,
                        RuleConst::NET_SUBMIT_FARE,
                        false,
                        false,
                        RuleConst::BLANK,
                        "TEST",
                        1,
                        'T',
                        "TEST",
                        'T',
                        "TEST",
                        RuleConst::BLANK);
    PrintOption po1;
    ptf1.negRuleData().printOption() = &po1;
    po1.printOption() = '2';
    NegPaxTypeFare ptf2(RuleConst::NRR_METHOD_2,
                        RuleConst::NET_SUBMIT_FARE,
                        false,
                        false,
                        RuleConst::BLANK,
                        "TEST",
                        1,
                        'T',
                        "TEST",
                        'T',
                        "TEST",
                        RuleConst::BLANK);
    PrintOption po2;
    ptf2.negRuleData().printOption() = &po2;
    po2.printOption() = '2';

    std::vector<const PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(&ptf1);
    paxTypeFares.push_back(&ptf2);

    CPPUNIT_ASSERT(NegotiatedFareRuleUtil::validatePrintOptionCombination(paxTypeFares));
  }

  void testValidatePrintOptionCombinationFalseWhenDiffPrintOption()
  {
    NegPaxTypeFare ptf1(RuleConst::NRR_METHOD_2,
                        RuleConst::NET_SUBMIT_FARE,
                        false,
                        false,
                        RuleConst::BLANK,
                        "TEST",
                        1,
                        'T',
                        "TEST",
                        'T',
                        "TEST",
                        RuleConst::BLANK);
    PrintOption po1;
    ptf1.negRuleData().printOption() = &po1;
    po1.printOption() = '2';
    NegPaxTypeFare ptf2(RuleConst::NRR_METHOD_2,
                        RuleConst::NET_SUBMIT_FARE,
                        false,
                        false,
                        RuleConst::BLANK,
                        "TEST",
                        1,
                        'T',
                        "TEST",
                        'T',
                        "TEST",
                        RuleConst::BLANK);
    PrintOption po2;
    ptf2.negRuleData().printOption() = &po2;
    po2.printOption() = '4';

    std::vector<const PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(&ptf1);
    paxTypeFares.push_back(&ptf2);

    CPPUNIT_ASSERT(!NegotiatedFareRuleUtil::validatePrintOptionCombination(paxTypeFares));
  }

  void testValidatePrintOptionCombinationTrueWhenSamePrintOptionDefault()
  {
    NegPaxTypeFare ptf1(RuleConst::NRR_METHOD_2,
                        RuleConst::NET_SUBMIT_FARE,
                        false,
                        false,
                        RuleConst::BLANK,
                        "TEST",
                        1,
                        'T',
                        "TEST",
                        'T',
                        "TEST",
                        RuleConst::BLANK);
    PrintOption po1;
    ptf1.negRuleData().printOption() = &po1;
    po1.printOption() = '3';
    NegPaxTypeFare ptf2(RuleConst::NRR_METHOD_2,
                        RuleConst::NET_SUBMIT_FARE,
                        false,
                        false,
                        RuleConst::BLANK,
                        "TEST",
                        1,
                        'T',
                        "TEST",
                        'T',
                        "TEST",
                        RuleConst::BLANK);

    std::vector<const PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(&ptf1);
    paxTypeFares.push_back(&ptf2);
    CPPUNIT_ASSERT(NegotiatedFareRuleUtil::validatePrintOptionCombination(paxTypeFares));
  }

  void testValidatePrintOptionCombinationFalseWhenDiffPrintOptionDefault()
  {
    NegPaxTypeFare ptf1(RuleConst::NRR_METHOD_2,
                        RuleConst::NET_SUBMIT_FARE,
                        false,
                        false,
                        RuleConst::BLANK,
                        "TEST",
                        1,
                        'T',
                        "TEST",
                        'T',
                        "TEST",
                        RuleConst::BLANK);
    PrintOption po1;
    ptf1.negRuleData().printOption() = &po1;
    po1.printOption() = '1';
    NegPaxTypeFare ptf2(RuleConst::NRR_METHOD_2,
                        RuleConst::NET_SUBMIT_FARE,
                        false,
                        false,
                        RuleConst::BLANK,
                        "TEST",
                        1,
                        'T',
                        "TEST",
                        'T',
                        "TEST",
                        RuleConst::BLANK);

    std::vector<const PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(&ptf1);
    paxTypeFares.push_back(&ptf2);
    CPPUNIT_ASSERT(!NegotiatedFareRuleUtil::validatePrintOptionCombination(paxTypeFares));
  }

  void testGetPrintOptionInfoWhenPrintOptionNotFound()
  {
    const PrintOption* printOption = 0;

    CPPUNIT_ASSERT_EQUAL(printOption, _checker->getPrintOptionInfo(*_trx, *_farePath));
  }

  PrintOption* prepareGetPrintOptionInfoWhenFoundPrintOptionCase()
  {
    createTrx();
    createPricingUnit(true);
    NegPaxTypeFare* negPaxTypeFare = static_cast<NegPaxTypeFare*>(
        _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare());
    negPaxTypeFare->negFareRest().netRemitMethod() = RuleConst::NRR_METHOD_1;
    PrintOption* printOption = _memHandle.create<PrintOption>();
    negPaxTypeFare->negRuleData().printOption() = printOption;

    return printOption;
  }

  void testGetPrintOptionInfoWhenFoundPrintOption()
  {
    const PrintOption* printOption = prepareGetPrintOptionInfoWhenFoundPrintOptionCase();

    CPPUNIT_ASSERT_EQUAL(printOption, _checker->getPrintOptionInfo(*_trx, *_farePath));
  }

  void testPrepareTourCodeValueCodeByPOWhenValueCodeIsResizedTo14()
  {
    _farePath->collectedNegFareData() = _cNegFareData;
    prepareGetPrintOptionInfoWhenFoundPrintOptionCase()->printOption() = '3';

    _cNegFareData->valueCode() = "123456789012345";
    std::string expectedResult = "12345678901234";
    _checker->prepareTourCodeValueCodeByPO(*_trx, *_farePath);

    CPPUNIT_ASSERT_EQUAL(expectedResult, _cNegFareData->valueCode());
  }

  void testPrepareTourCodeValueCodeByPOWhenValueCodeIsResizedTo15()
  {
    _farePath->collectedNegFareData() = _cNegFareData;
    prepareGetPrintOptionInfoWhenFoundPrintOptionCase()->printOption() = '3';

    _cNegFareData->tourCode() = "1234567890123456";
    std::string expectedResult = "123456789012345";
    _checker->prepareTourCodeValueCodeByPO(*_trx, *_farePath);

    CPPUNIT_ASSERT_EQUAL(expectedResult, _cNegFareData->tourCode());
  }

  void testPrepareTourCodeValueCodeByPOWhenPrintOption1()
  {
    _farePath->collectedNegFareData() = _cNegFareData;
    prepareGetPrintOptionInfoWhenFoundPrintOptionCase()->printOption() = '1';

    _cNegFareData->valueCode() = "1234567";
    _cNegFareData->tourCode() = "890123456";
    std::string expectedResult = "1234567/8901234";
    _checker->prepareTourCodeValueCodeByPO(*_trx, *_farePath);

    CPPUNIT_ASSERT_EQUAL(std::string("1234567"), _cNegFareData->valueCode());
    CPPUNIT_ASSERT_EQUAL(expectedResult, _cNegFareData->tourCode());
  }

  void testPrepareTourCodeValueCodeByPOWhenPrintOption2()
  {
    _farePath->collectedNegFareData() = _cNegFareData;
    prepareGetPrintOptionInfoWhenFoundPrintOptionCase()->printOption() = '2';

    _cNegFareData->valueCode() = "890123456";
    _cNegFareData->tourCode() = "1234567";
    std::string expectedResult = "1234567/8901234";
    _checker->prepareTourCodeValueCodeByPO(*_trx, *_farePath);

    CPPUNIT_ASSERT_EQUAL(std::string("890123456"), _cNegFareData->valueCode());
    CPPUNIT_ASSERT_EQUAL(expectedResult, _cNegFareData->tourCode());
  }

  void testPrepareTourCodeValueCodeByPOWhenPrintOption4()
  {
    _farePath->collectedNegFareData() = _cNegFareData;
    prepareGetPrintOptionInfoWhenFoundPrintOptionCase()->printOption() = '4';

    _cNegFareData->valueCode() = "12345678901234";
    _cNegFareData->tourCode() = "567890123456789";

    _checker->prepareTourCodeValueCodeByPO(*_trx, *_farePath);

    CPPUNIT_ASSERT_EQUAL(std::string("56789012345678"), _cNegFareData->valueCode());
    CPPUNIT_ASSERT_EQUAL(std::string("12345678901234"), _cNegFareData->tourCode());
  }

  void testSaveTourCodeForPQWhenTKTRequest()
  {
    _farePath->collectedNegFareData() = _cNegFareData;
    _cNegFareData->tourCode().clear();
    _checker->_indicatorTkt = true;
    _checker->saveTourCodeForPQ(*_trx, *_farePath);

    CPPUNIT_ASSERT(_cNegFareData->tourCode().empty());
  }

  void testSaveTourCodeForPQWhenTourCodeExists()
  {
    createTrx();
    createPricingUnit(true);
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST");
    _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare() = &paxTypeFare1;
    _farePath->collectedNegFareData() = _cNegFareData;
    _cNegFareData->tourCode().clear();
    _checker->_warningMsg = NegotiatedFareRuleUtil::NOT_ITBT;
    _checker->saveTourCodeForPQ(*_trx, *_farePath);
    CPPUNIT_ASSERT_EQUAL(std::string("TEST"), _cNegFareData->tourCode());
  }

  void testHasFareAmountNoCurrency()
  {
    NegFareRest nfr;
    nfr.cur11() = "";
    nfr.fareAmt1() = 12345;
    CPPUNIT_ASSERT_EQUAL(false, _checker->hasFareBoxAmount(&nfr));
  }

  void testHasFareAmountZero()
  {
    NegFareRest nfr;
    nfr.cur11() = "USD";
    nfr.fareAmt1() = 0;
    CPPUNIT_ASSERT_EQUAL(true, _checker->hasFareBoxAmount(&nfr));
  }

  void testHasFareAmount()
  {
    NegFareRest nfr;
    nfr.cur11() = "USD";
    nfr.fareAmt1() = 12345;
    CPPUNIT_ASSERT_EQUAL(true, _checker->hasFareBoxAmount(&nfr));
  }

  void testHasFareBoxNetRemitMethodNull()
  {
    CPPUNIT_ASSERT_EQUAL(false, _checker->hasFareBoxNetRemitMethod(0));
  }

  void testHasFareBoxNetRemitMethod1()
  {
    NegFareRest nfr;
    nfr.netRemitMethod() = '1';
    CPPUNIT_ASSERT_EQUAL(true, _checker->hasFareBoxNetRemitMethod(&nfr));
  }

  void testHasFareBoxNetRemitMethod2()
  {
    NegFareRest nfr;
    nfr.netRemitMethod() = '2';
    CPPUNIT_ASSERT_EQUAL(true, _checker->hasFareBoxNetRemitMethod(&nfr));
  }

  void testHasFareBoxNetRemitMethod3()
  {
    NegFareRest nfr;
    nfr.netRemitMethod() = '3';
    CPPUNIT_ASSERT_EQUAL(true, _checker->hasFareBoxNetRemitMethod(&nfr));
  }

  void testValidateFareBoxCombinationNotANetRemitMethod()
  {
    NegPaxTypeFare nptf('1');
    std::vector<const PaxTypeFare*> ptfs;
    ptfs += &nptf;
    CPPUNIT_ASSERT_EQUAL(true, _checker->validateFareBoxCombination(ptfs));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::NO_WARNING, _checker->_warningMsg);
  }

  void testValidateFareBoxCombinationNoFareAmount()
  {
    NegPaxTypeFare nptf('2');
    nptf.negFareRest().cur11() = "";
    nptf.negFareRest().fareAmt1() = 0;
    std::vector<const PaxTypeFare*> ptfs;
    ptfs += &nptf;
    CPPUNIT_ASSERT_EQUAL(true, _checker->validateFareBoxCombination(ptfs));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::NO_WARNING, _checker->_warningMsg);
  }

  void testValidateFareBoxCombinationOneValidFare()
  {
    NegPaxTypeFare nptf('2');
    nptf.negFareRest().cur11() = "USD";
    nptf.negFareRest().fareAmt1() = 12345;
    std::vector<const PaxTypeFare*> ptfs;
    ptfs += &nptf;
    CPPUNIT_ASSERT_EQUAL(true, _checker->validateFareBoxCombination(ptfs));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::NO_WARNING, _checker->_warningMsg);
  }

  void testValidateFareBoxCombinationTwoValidFares()
  {
    NegPaxTypeFare nptf('2');
    nptf.negFareRest().cur11() = "USD";
    nptf.negFareRest().fareAmt1() = 12345;
    std::vector<const PaxTypeFare*> ptfs;
    ptfs += &nptf, &nptf;
    CPPUNIT_ASSERT_EQUAL(true, _checker->validateFareBoxCombination(ptfs));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::NO_WARNING, _checker->_warningMsg);
  }

  void testValidateFareBoxCombinationFirstFareNotValid()
  {
    NegPaxTypeFare nptf1('2');
    nptf1.negFareRest().cur11() = "";
    nptf1.negFareRest().fareAmt1() = 0;
    NegPaxTypeFare nptf2('2');
    nptf2.negFareRest().cur11() = "USD";
    nptf2.negFareRest().fareAmt1() = 12345;
    std::vector<const PaxTypeFare*> ptfs;
    ptfs += &nptf1, &nptf2;
    CPPUNIT_ASSERT_EQUAL(false, _checker->validateFareBoxCombination(ptfs));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::MIXED_FARE_BOX_AMT, _checker->_warningMsg);
  }

  void testValidateFareBoxCombinationSecondFareNotValid()
  {
    NegPaxTypeFare nptf1('2');
    nptf1.negFareRest().cur11() = "USD";
    nptf1.negFareRest().fareAmt1() = 12345;
    NegPaxTypeFare nptf2('2');
    nptf2.negFareRest().cur11() = "";
    nptf2.negFareRest().fareAmt1() = 0;
    std::vector<const PaxTypeFare*> ptfs;
    ptfs += &nptf1, &nptf2;
    CPPUNIT_ASSERT_EQUAL(false, _checker->validateFareBoxCombination(ptfs));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareRuleUtil::MIXED_FARE_BOX_AMT, _checker->_warningMsg);
  }

  void testValidateNetRemitMethod1NegFareRestValid()
  {
    NegPaxTypeFare nptf1('2');
    nptf1.negFareRest().netRemitMethod() = RuleConst::NRR_METHOD_1;
    nptf1.negFareRest().netGrossInd() = 'G';
    nptf1.negFareRest().tourBoxCodeType1() = 'B';
    nptf1.negFareRest().tourBoxCodeType2() = ' ';
    CPPUNIT_ASSERT_EQUAL(true, _checker->validateNetRemitMethod1(&(nptf1.negFareRest())));
  }

  void testValidateNetRemitMethod1NegFareRestInvalidNet()
  {
    NegPaxTypeFare nptf1('2');
    nptf1.negFareRest().netRemitMethod() = RuleConst::NRR_METHOD_1;
    nptf1.negFareRest().netGrossInd() = 'N';
    nptf1.negFareRest().tourBoxCodeType1() = 'B';
    nptf1.negFareRest().tourBoxCodeType2() = ' ';
    CPPUNIT_ASSERT_EQUAL(false, _checker->validateNetRemitMethod1(&(nptf1.negFareRest())));
  }

  void testValidateNetRemitMethod1NegFareRestInvalidTypeCode()
  {
    NegPaxTypeFare nptf1('2');
    nptf1.negFareRest().netRemitMethod() = RuleConst::NRR_METHOD_1;
    nptf1.negFareRest().netGrossInd() = 'G';
    nptf1.negFareRest().tourBoxCodeType1() = ' ';
    nptf1.negFareRest().tourBoxCodeType2() = ' ';
    CPPUNIT_ASSERT_EQUAL(false, _checker->validateNetRemitMethod1(&(nptf1.negFareRest())));
  }

  void testValidateNetRemitMethod1NegFareRestNotMethod1()
  {
    NegPaxTypeFare nptf1('2');
    nptf1.negFareRest().netRemitMethod() = RuleConst::NRR_METHOD_2;
    nptf1.negFareRest().netGrossInd() = 'N';
    nptf1.negFareRest().tourBoxCodeType1() = ' ';
    nptf1.negFareRest().tourBoxCodeType2() = ' ';
    CPPUNIT_ASSERT_EQUAL(true, _checker->validateNetRemitMethod1(&(nptf1.negFareRest())));
  }

  void testValidateNetRemitMethod1FaresVectorValid()
  {
    NegPaxTypeFare nptf1('2');
    nptf1.negFareRest().netRemitMethod() = RuleConst::NRR_METHOD_1;
    nptf1.negFareRest().netGrossInd() = 'G';
    nptf1.negFareRest().tourBoxCodeType1() = 'B';
    nptf1.negFareRest().tourBoxCodeType2() = ' ';
    NegPaxTypeFare nptf2('2');
    nptf2.negFareRest().netRemitMethod() = RuleConst::NRR_METHOD_1;
    nptf2.negFareRest().netGrossInd() = 'G';
    nptf2.negFareRest().tourBoxCodeType1() = 'V';
    nptf2.negFareRest().tourBoxCodeType2() = ' ';
    std::vector<const PaxTypeFare*> fv;
    fv.push_back(&nptf1);
    fv.push_back(&nptf2);
    CPPUNIT_ASSERT_EQUAL(true, _checker->validateNetRemitMethod1(fv));
  }

  void testValidateNetRemitMethod1FaresVectorInvalid()
  {
    NegPaxTypeFare nptf1('2');
    nptf1.negFareRest().netRemitMethod() = RuleConst::NRR_METHOD_1;
    nptf1.negFareRest().netGrossInd() = 'G';
    nptf1.negFareRest().tourBoxCodeType1() = 'B';
    nptf1.negFareRest().tourBoxCodeType2() = ' ';
    NegPaxTypeFare nptf2('2');
    nptf2.negFareRest().netRemitMethod() = RuleConst::NRR_METHOD_1;
    nptf2.negFareRest().netGrossInd() = 'N';
    nptf2.negFareRest().tourBoxCodeType1() = 'V';
    nptf2.negFareRest().tourBoxCodeType2() = ' ';
    std::vector<const PaxTypeFare*> fv;
    fv.push_back(&nptf1);
    fv.push_back(&nptf2);
    CPPUNIT_ASSERT_EQUAL(false, _checker->validateNetRemitMethod1(fv));
  }

  void testIsItBtTicketingDataInvalid()
  {
    _farePath->collectedNegFareData() = 0;
    CPPUNIT_ASSERT_EQUAL(false, _checker->isItBtTicketingData(*_farePath));
  }

  void testIsItBtTicketingDataInvalidWhenTypeLNoT979IT()
  {
    _farePath->collectedNegFareData() = _cNegFareData;
    _cNegFareData->fareTypeCode() = RuleConst::SELLING_FARE;
    _cNegFareData->fareBox() = "IT";

    CPPUNIT_ASSERT_EQUAL(false, _checker->isItBtTicketingData(*_farePath));
  }

  void testIsItBtTicketingDataValidWhenTypeLWithT979IT()
  {
    _farePath->collectedNegFareData() = _cNegFareData;
    _cNegFareData->fareTypeCode() = RuleConst::SELLING_FARE_NOT_FOR_SEC;
    _cNegFareData->fareBox() = "IT";

    CPPUNIT_ASSERT_EQUAL(true, _checker->isItBtTicketingData(*_farePath));
  }

  void testIsItBtTicketingDataValidWhenTypeTIT()
  {
    _farePath->collectedNegFareData() = _cNegFareData;
    _cNegFareData->fareTypeCode() = RuleConst::NET_SUBMIT_FARE;
    _cNegFareData->fareBox() = "IT";

    CPPUNIT_ASSERT_EQUAL(true, _checker->isItBtTicketingData(*_farePath));
  }

  void testIsItBtTicketingDataValidWhenTypeCBT()
  {
    _farePath->collectedNegFareData() = _cNegFareData;
    _cNegFareData->fareTypeCode() = RuleConst::NET_SUBMIT_FARE_UPD;
    _cNegFareData->fareBox() = "BT";

    CPPUNIT_ASSERT_EQUAL(true, _checker->isItBtTicketingData(*_farePath));
  }

  void testIsItBtTicketingDataInvalidWhenTypeTNotITBT()
  {
    _farePath->collectedNegFareData() = _cNegFareData;
    _cNegFareData->fareTypeCode() = RuleConst::NET_SUBMIT_FARE;
    _cNegFareData->fareBox() = "BAD123";

    CPPUNIT_ASSERT_EQUAL(false, _checker->isItBtTicketingData(*_farePath));
  }

  void testIsItBtTicketingDataInvalidWhenTypeCNotITBT()
  {
    _farePath->collectedNegFareData() = _cNegFareData;
    _cNegFareData->fareTypeCode() = RuleConst::NET_SUBMIT_FARE_UPD;
    _cNegFareData->fareBox() = "BAD123";

    CPPUNIT_ASSERT_EQUAL(false, _checker->isItBtTicketingData(*_farePath));
  }

   void testisRegularNetWhenTypeCITBT()
  {
    createTrx();
    createPricingUnit(true);
    NegPaxTypeFare* negPaxTypeFare = static_cast<NegPaxTypeFare*>(
       _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare());
    negPaxTypeFare->negFareRest().netRemitMethod() = RuleConst::NRR_METHOD_1;
    _farePath->collectedNegFareData() = _cNegFareData;
    _cNegFareData->fareTypeCode() = RuleConst::NET_SUBMIT_FARE_UPD;
    _cNegFareData->fareBox() = "BAD123";

    CPPUNIT_ASSERT_EQUAL(false, _checker->isRegularNet(*_farePath));
  }

  void testisRegularNetWhenTypeCNetRemit()
  {
    createTrx();
    createPricingUnit(true);
    NegPaxTypeFare* negPaxTypeFare = static_cast<NegPaxTypeFare*>(
       _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare());
    negPaxTypeFare->negFareRest().netRemitMethod() = RuleConst::NRR_METHOD_1;
    _farePath->collectedNegFareData() = _cNegFareData;
    _cNegFareData->fareTypeCode() = RuleConst::NET_SUBMIT_FARE_UPD;

    CPPUNIT_ASSERT_EQUAL(false, _checker->isRegularNet(*_farePath));
  }

  void testisRegularNetWhenTypeCPass()
  {
    createTrx();
    createPricingUnit(true);
    _farePath->collectedNegFareData() = _cNegFareData;
    _cNegFareData->fareTypeCode() = RuleConst::NET_SUBMIT_FARE_UPD;

    CPPUNIT_ASSERT_EQUAL(true, _checker->isRegularNet(*_farePath));
  }

  void testcheckTktDataCat35FareFail()
  {
    NegPaxTypeFareRuleData* ruleData = _memHandle.create<NegPaxTypeFareRuleData>();
    NegFareRest negFareRest;
    createTrx();
    createPricingUnit(true);
    NegPaxTypeFare* negPaxTypeFare = static_cast<NegPaxTypeFare*>(
    _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare());
    negFareRest.netRemitMethod() = RuleConst::NRR_METHOD_1;
    negPaxTypeFare->negFareRest().netRemitMethod() = RuleConst::NRR_METHOD_1;
    _farePath->collectedNegFareData() = _cNegFareData;
    _cNegFareData->fareTypeCode() = RuleConst::NET_SUBMIT_FARE_UPD;
    bool result = _checker->checkTktDataCat35Fare(*_trx,
                                                  *_farePath,
                                                  *_farePath->pricingUnit().front()->fareUsage().front(),
                                                  _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare(),
                                                  &negFareRest,
                                                  _cNegFareData,
                                                  ruleData);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }

  void testcheckTktDataCat35FarePass()
  {
    NegPaxTypeFareRuleData* ruleData = _memHandle.create<NegPaxTypeFareRuleData>();
    NegFareRest negFareRest;
    createTrx();
    createPricingUnit(true);
    NegPaxTypeFare* negPaxTypeFare = static_cast<NegPaxTypeFare*>(
    _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare());
    negFareRest.netRemitMethod() = RuleConst::NRR_METHOD_BLANK;
    negPaxTypeFare->negFareRest().netRemitMethod() = BLANK;
    _farePath->collectedNegFareData() = _cNegFareData;
    _cNegFareData->fareTypeCode() = RuleConst::NET_SUBMIT_FARE_UPD;
    bool result = _checker->checkTktDataCat35Fare(*_trx,
                                                  *_farePath,
                                                  *_farePath->pricingUnit().front()->fareUsage().front(),
                                                  _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare(),
                                                  & negFareRest,
                                                  _cNegFareData,
                                                  ruleData);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testcheckTktDataCat35FareAxessUserFail()
  {
    NegPaxTypeFareRuleData* ruleData = _memHandle.create<NegPaxTypeFareRuleData>();
    NegFareRest negFareRest;
    createTrx();
    createPricingUnit(true);
    NegPaxTypeFare* negPaxTypeFare = static_cast<NegPaxTypeFare*>(
    _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare());
    negFareRest.netRemitMethod() = RuleConst::NRR_METHOD_3;
    negPaxTypeFare->negFareRest().netRemitMethod() = RuleConst::NRR_METHOD_3;
    _checker->_axessUser = true;
    _farePath->collectedNegFareData() = _cNegFareData;
    _cNegFareData->fareTypeCode() = RuleConst::NET_SUBMIT_FARE_UPD;
    bool result = _checker->checkTktDataCat35Fare(*_trx,
                                                  *_farePath,
                                                  *_farePath->pricingUnit().front()->fareUsage().front(),
                                                  _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare(),
                                                  & negFareRest,
                                                  _cNegFareData,
                                                  ruleData);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }


protected:
  NegotiatedFareRuleUtil* _checker;
  FarePath* _farePath;
  CollectedNegFareData* _cNegFareData;
  PricingTrx* _trx;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(NegotiatedFareRuleUtilTest);

} // tse
