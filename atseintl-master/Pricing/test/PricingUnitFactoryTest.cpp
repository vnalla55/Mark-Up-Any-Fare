#include <boost/assign/std/vector.hpp>

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/TrxUtil.h"
#include "Common/Utils/CommonUtils.h"
#include "Common/VecMap.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/TaxCodeReg.h"
#include "Diagnostic/Diag605Collector.h"
#include "Pricing/Combinations.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/PricingUnitFactory.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/PU.h"
#include "Rules/RuleConst.h"
#include "test/DBAccessMock/DataHandleMock.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{
FALLBACKVALUE_DECL(fallbackValidatingCxrMultiSp);
using namespace boost::assign;

class PricingUnitFactoryTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingUnitFactoryTest);

  CPPUNIT_TEST(testNegotiatedFarePriority);
  CPPUNIT_TEST(testNegotiatedFarePriorityIsDefaultWhenNoFares);
  CPPUNIT_TEST(testNegotiatedFarePriorityIsDefaultWhenOneNonNegFare);
  CPPUNIT_TEST(testNegotiatedFarePriorityIsDefaultWhenNoNegFares);
  CPPUNIT_TEST(testNegotiatedFarePriorityIsDefaultWhenOneNegFare);
  CPPUNIT_TEST(testNegotiatedFarePriorityIsLowWhenFaresHaveDifferentRules);
  CPPUNIT_TEST(testNegotiatedFarePriorityIsLowWhenFaresHaveDifferentRulesSameR3s);
  CPPUNIT_TEST(testNegotiatedFarePriorityIsDefaultWhenFaresHaveSameRulesSameR3s);
  CPPUNIT_TEST(testNegotiatedFarePriorityIsLowWhenFaresHaveSameRulesDifferentR3s);

  CPPUNIT_TEST(testHigherPriorityIsOnTopWhenHigherPriorityPupqItemIsPushedLast);
  CPPUNIT_TEST(testHigherPriorityIsOnTopWhenHigherPriorityPupqItemIsPushedFirst);

  CPPUNIT_TEST(testIsPUWithSameFares_TrueWhenSamePTF_Ptr);
  CPPUNIT_TEST(testIsPUWithSameFares_FalseWhenDiffPTF_Ptr);

  CPPUNIT_TEST(testIsValidFareForCxrSpecificOpenJawTrueWhenOJNotActive);
  CPPUNIT_TEST(testIsValidFareForCxrSpecificOpenJawFalseNLfareWhenDOJActive);
  CPPUNIT_TEST(testIsValidFareForCxrSpecificOpenJawTrueSPfareWhenDOJActive);
  CPPUNIT_TEST(testIsValidFareForCxrSpecificOpenJawFalseYYfareWhenTOJ);
  CPPUNIT_TEST(testIsValidFareForCxrSpecificOpenJawTrueCxrfareWhenTOJ);
  CPPUNIT_TEST(testIsValidFareForCxrSpecificOpenJawFalseMultiGovCxrfareWhenTOJ);
  CPPUNIT_TEST(testIsValidFareForCxrSpecificOpenJawTrueSPfareWhenOOJ);
  CPPUNIT_TEST(testIsValidFareForCxrSpecificOpenJawFalseNLYYfareWhenOOJ);
  CPPUNIT_TEST(testIsValidFareForCxrSpecificOpenJawFalseNotAllowNLCxrfareWhenOOJ);
  CPPUNIT_TEST(testIsValidFareForCxrSpecificOpenJawTrueAllowNLCxrfareWhenOOJ);

  CPPUNIT_TEST(testCheckOJRec2Cat10Indicator_SamePointTableApplicable);
  CPPUNIT_TEST(testCheckOJRec2Cat10Indicator_NotSameCxr);
  CPPUNIT_TEST(testCheckOJRec2Cat10Indicator_DestOpenJawReq);
  CPPUNIT_TEST(testCheckOJRec2Cat10Indicator_OriginOpenJawReq);
  CPPUNIT_TEST(testCheckOJRec2Cat10Indicator_DiffCountryDOJRequired);
  CPPUNIT_TEST(testCheckOJRec2Cat10Indicator_DiffCountrySOJRequired);
  CPPUNIT_TEST(testCheckOJRec2Cat10Indicator_SameCountrySOJReq);
  CPPUNIT_TEST(testCheckOJRec2Cat10Indicator_SameCountrySOJRequired);
  CPPUNIT_TEST(testCheckOJRec2Cat10Indicator_SameCountryDOJRequired);
  CPPUNIT_TEST(testCheckOJRec2Cat10Indicator_NotValidForDOJ);
  CPPUNIT_TEST(testCheckOJRec2Cat10Indicator_NotValidForSOJ);

  CPPUNIT_TEST(testCheckOJSurfaceRestriction_DoubleOJ_newLogic);
  CPPUNIT_TEST(testCheckOJSurfaceRestriction_OrigOJ_newLogic);
  CPPUNIT_TEST(testCheckOJSurfaceRestriction_DoubleOJ_oldLogic);
  CPPUNIT_TEST(testCheckOJSurfaceRestriction_OrigOJ_oldLogic);

  CPPUNIT_TEST(testCheckSingleCurrency_Fail);
  CPPUNIT_TEST(testCheckSingleCurrency_AltCurrencyRequest_Fail);
  CPPUNIT_TEST(testCheckSingleCurrency_AltCurrencyRequest_Pass);
  CPPUNIT_TEST(testCheckSingleCurrency_RequestedPaxTypeAlphaAlphaNumeric);
  CPPUNIT_TEST(testCheckSingleCurrency_PrimaryPaxTypeADULT_RequestedPaxTypeAlphaNumericNumeric);
  CPPUNIT_TEST(testCheckSingleCurrency_PrimaryPaxTypeADULT_RequestedPaxTypeCHILD);
  CPPUNIT_TEST(testCheckSingleCurrency_OutboundCurrency);
  CPPUNIT_TEST(testCheckSingleCurrency_OutboundAseanCurrencies);
  CPPUNIT_TEST(testCheckSingleCurrency_InboundCurrency);
  CPPUNIT_TEST(testCheckSingleCurrency_InboundAseanCurrencies);

  CPPUNIT_TEST(testReadTaxAndConvertCurrency_taxResponseEmpty);
  CPPUNIT_TEST(testReadTaxAndConvertCurrency_taxResponseNotSet);
  CPPUNIT_TEST(testReadTaxAndConvertCurrency_OutBoundYQFTaxCharged);
  CPPUNIT_TEST(testReadTaxAndConvertCurrency_InBoundYQFTaxCharged);
  CPPUNIT_TEST(testReadTaxAndConvertCurrency_OutBoundYQITaxCharged);
  CPPUNIT_TEST(testReadTaxAndConvertCurrency_InBoundYQITaxCharged);
  CPPUNIT_TEST(testReadTaxAndConvertCurrency_OutBoundYRITaxCharged);
  CPPUNIT_TEST(testReadTaxAndConvertCurrency_InBoundYRITaxCharged);
  CPPUNIT_TEST(testReadTaxAndConvertCurrency_NoFeeAppl);
  CPPUNIT_TEST(testReadTaxAndConvertCurrency_RevertOrder);

  CPPUNIT_SKIP_TEST(testBuildCxrFarePricingUnit_UsePrevFareUsage_Pass);
  CPPUNIT_SKIP_TEST(testBuildCxrFarePricingUnit_UsePrevFareUsage_Fail);
  CPPUNIT_SKIP_TEST(testBuildCxrFarePricingUnit_BuildFareUsage_Pass);
  CPPUNIT_SKIP_TEST(testBuildCxrFarePricingUnit_IndustryCarrier);
  CPPUNIT_SKIP_TEST(testBuildCxrFarePricingUnit_xPoint);

  CPPUNIT_TEST(testBuildNextLevelPricinUnit_PqEmpty);
  CPPUNIT_TEST(testBuildNextLevelPricinUnit_InvalidItem);
  CPPUNIT_TEST(testBuildNextLevelPricinUnit_ValidItem);
  CPPUNIT_TEST(testBuildNextLevelPricinUnit_ShutdownFactory);
  CPPUNIT_TEST(testBuildNextLevelPricinUnit_RequestPUCount);

  CPPUNIT_SKIP_TEST(testBuildPricingUnit_UsePrevFareUsage_Pass);
  CPPUNIT_SKIP_TEST(testBuildPricingUnit_UsePrevFareUsage_Fail);
  CPPUNIT_SKIP_TEST(testBuildPricingUnit_BuildFareUsage_Pass);
  CPPUNIT_SKIP_TEST(testBuildPricingUnit_xPoint);

  CPPUNIT_TEST(testSameBookingCodes_FailOnSize);
  CPPUNIT_TEST(testSameBookingCodes_FailOnValue);
  CPPUNIT_TEST(testSameBookingCodes_Pass);

  CPPUNIT_TEST(testGetNextCxrFarePUPQItemImpl_NoPUPQItem);
  CPPUNIT_SKIP_TEST(testGetNextCxrFarePUPQItemImpl_PUPQItem_LowPriority);
  CPPUNIT_SKIP_TEST(testGetNextCxrFarePUPQItemImpl_PUPQItem_DefaultPriority);
  CPPUNIT_TEST(testGetNextCxrFarePUPQItemImpl_ValidCxrPUPQItem_FarePriority);
  CPPUNIT_TEST(testGetNextCxrFarePUPQItemImpl_ValidCxrPUPQItem_CxrFareComboIdx);
  CPPUNIT_TEST(testGetNextCxrFarePUPQItemImpl_ValidCxrPUPQItem_CxrFareComboIdxWithXPoint);
  CPPUNIT_TEST(testGetNextCxrFarePUPQItemImpl_CxrFareCombo_Done);

  CPPUNIT_TEST(testCheckRTCTIATAExceptions_DifferentMilagePercentage);
  CPPUNIT_TEST(testCheckRTCTIATAExceptions_TravelWithinUsca);
  CPPUNIT_TEST(testCheckRTCTIATAExceptions_SameNucFareAmount);
  CPPUNIT_TEST(testCheckRTCTIATAExceptions_CarriersApplyDifferentNuc);
  CPPUNIT_TEST(testCheckRTCTIATAExceptions_SpecialNormalFaresCombo);
  CPPUNIT_TEST(testCheckRTCTIATAExceptions_DifferentCarrier);
  CPPUNIT_TEST(testCheckRTCTIATAExceptions_DifferentCabin);

  // test case is obsolete with new RTCT logic
  CPPUNIT_SKIP_TEST(testCheckRTCTIATAExceptions_Cat25OrCat35);

  CPPUNIT_TEST(testCheckRTCTIATAExceptions_DifferentCat2);
  CPPUNIT_TEST(testCheckRTCTIATAExceptions_SameCat2);

  CPPUNIT_TEST(testGetNextCxrFarePricinUnit_Fail_Empty);
  CPPUNIT_TEST(testGetNextCxrFarePricinUnit_FailNotValid);
  CPPUNIT_TEST(testGetNextCxrFarePricinUnit_Pass);

  CPPUNIT_TEST(testIsPricingUnitValid_Pass);
  CPPUNIT_TEST(testIsPricingUnitValid_Fail_PULevelCombinability);

  // Record 1
  CPPUNIT_TEST(testCheckRec1Indicator_Different_DowType);
  CPPUNIT_TEST(testCheckRec1Indicator_Same_DowType);
  CPPUNIT_TEST(testCheckRec1Indicator_Blank_DowType);
  CPPUNIT_TEST(testCheckRec1Indicator_Different_SeasonType);
  CPPUNIT_TEST(testCheckRec1Indicator_Same_SeasonType);
  CPPUNIT_TEST(testCheckRec1Indicator_Blank_SeasonType);

  // Record 2
  CPPUNIT_TEST(testCheckRec2Indicator_Different_DowType);
  CPPUNIT_TEST(testCheckRec2Indicator_Same_DowType);
  CPPUNIT_TEST(testCheckRec2Indicator_Blank_DowType);
  CPPUNIT_TEST(testCheckRec2Indicator_Different_SeasonType);
  CPPUNIT_TEST(testCheckRec2Indicator_Same_SeasonType);
  CPPUNIT_TEST(testCheckRec2Indicator_Blank_SeasonType);

  /// overload method for checkRec2Indicator
  CPPUNIT_TEST(testCheckRec2IndicatorForGfr_Different_DowType);
  CPPUNIT_TEST(testCheckRec2IndicatorForGfr_Same_ItemInfo);
  CPPUNIT_TEST(testCheckRec2IndicatorForGfr_Different_ItemNo);
  CPPUNIT_TEST(testCheckRec2IndicatorForGfr_Different_RelationalInd);

  // Record 3
  CPPUNIT_TEST(testCheckRec3Indicator_Cat2_Different_ItemNo);
  CPPUNIT_TEST(testCheckRec3Indicator_Cat2_Same_ItemNo);
  CPPUNIT_TEST(testCheckRec3Indicator_Cat2_Different_Base_ItemNo);
  CPPUNIT_TEST(testCheckRec3Indicator_Cat2_Different_RelationalInd);
  CPPUNIT_TEST(testCheckRec3Indicator_Cat3_Different_RelationalInd);

  CPPUNIT_TEST(testCheckNonDirectionalFare_MixedDirFareMarket_OneCurency_Pass);
  CPPUNIT_TEST(testCheckNonDirectionalFare_MixedDirFareMarket_OneCurency_Fail);
  CPPUNIT_TEST(testCheckNonDirectionalFare_MixedDirFareMarket_MixedCurency);
  CPPUNIT_TEST(testCheckNonDirectionalFare_OneDirFareMarket_OneCurrency_Pass);
  CPPUNIT_TEST(testCheckNonDirectionalFare_OneDirFareMarket_OneCurrency_Fail);
  CPPUNIT_TEST(testCheckNonDirectionalFare_OneDirFareMarket_MixedCurency);

  CPPUNIT_TEST(testDisplayPricingUnit);
  CPPUNIT_TEST(testDisplayPricingUnitMip);

  CPPUNIT_TEST(checkBrandParity);

  CPPUNIT_TEST(testIsValidPUForValidatingCxr_NoValidatingCxr);
  CPPUNIT_TEST(testIsValidPUForValidatingCxr_NoValidatingCxrForMultiSp);
  CPPUNIT_TEST(testIsValidPUForValidatingCxr_ValidCxrList);
  CPPUNIT_TEST(testIsValidPUForValidatingCxr_ValidCxrListMultiSp);
  CPPUNIT_TEST(testIsValidPUForValidatingCxr_Failed);
  CPPUNIT_TEST(testIsValidPUForValidatingCxr_FailedMultiSp);

  CPPUNIT_TEST(testCheckSOJFareRestrictionNLNL);
  CPPUNIT_TEST(testCheckSOJFareRestrictionNLSP);
  CPPUNIT_TEST(testCheckSOJFareRestrictionSPNL);
  CPPUNIT_TEST(testCheckSOJFareRestrictionSPSP);

  CPPUNIT_TEST(testIsFareValidForTraditionalVC_singleTraditionalVC_PASS);
  CPPUNIT_TEST(testIsFareValidForTraditionalVC_singleTraditionalVC_FAIL);
  CPPUNIT_TEST(testIsFareValidForTraditionalVC_multiTraditionalVC_FAIL);
  CPPUNIT_TEST(testIsFareValidForTraditionalVC_multiTraditionalVC_PASS);

  CPPUNIT_TEST_SUITE_END();

public:
  PricingTrx* _trx;

  PricingUnitFactory* _factory;
  PUPQItem _pupqItem, _pupqItem2, _pupqItem3;

  PricingUnit* _pricingUnit, *_pricingUnit2, *_pricingUnit3;

  FareUsage* _fareUsage1;
  FareUsage* _fareUsage2;
  FareUsage* _fareUsage3;
  PaxTypeFare* _paxTypeFare1;
  PaxTypeFare* _paxTypeFare2;
  PaxTypeFare* _paxTypeFare3;
  Fare* _fare1;
  Fare* _fare2;
  FareInfo* _fareInfo1;
  FareInfo* _fareInfo2;
  TestMemHandle _memH;
  DiagCollector* _diag;
  FareMarket* _fareMarket1;
  FareMarket* _fareMarket2;

  FareClassAppInfo _fca1;
  PU* _pu;
  MergedFareMarket* _mfm;

  BrandProgram* _bProgram1;
  BrandProgram* _bProgram2;
  BrandInfo* _brand1;
  BrandInfo* _brand2;
  BrandInfo* _brand3;
  BrandInfo* _brand4;

  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _memH.create<MyDataHandle>();
    _trx = _memH.create<PricingTrx>();
    _trx->setRequest(_memH.create<PricingRequest>());

    _pricingUnit = _memH(new PricingUnit);
    _pricingUnit2 = _memH(new PricingUnit);
    _pricingUnit3 = _memH(new PricingUnit);
    _fareUsage1 = _memH(new FareUsage);
    _fareUsage2 = _memH(new FareUsage);
    _fareUsage3 = _memH(new FareUsage);
    _paxTypeFare1 = _memH(new PaxTypeFare);
    _paxTypeFare2 = _memH(new PaxTypeFare);
    _paxTypeFare3 = _memH(new PaxTypeFare);

    _fare1 = _memH(new Fare);
    _fareInfo1 = _memH(new FareInfo);
    _fare2 = _memH(new Fare);
    _fareInfo2 = _memH(new FareInfo);

    _pu = _memH(new PU);

    _factory = _memH(new PricingUnitFactory);
    _factory->_trx = _trx;
    _factory->_paxType = _memH(new PaxType);

    _factory->pu() = _pu;

    _pupqItem.pricingUnit() = _pricingUnit;
    _paxTypeFare1->fareClassAppInfo() = &_fca1;

    _diag = createDiag605();

    _fareMarket1 = _memH(new FareMarket);
    _fareMarket2 = _memH(new FareMarket);

    _fareMarket1->origin() = _memH(new Loc);
    _fareMarket1->destination() = _memH(new Loc);
    _fareMarket2->origin() = _memH(new Loc);
    _fareMarket2->destination() = _memH(new Loc);

    _mfm = _memH(new MergedFareMarket);

    _bProgram1 = _memH.create<BrandProgram>();
    _bProgram2 = _memH.create<BrandProgram>();

    _brand1 = _memH.create<BrandInfo>();
    _brand1->brandCode() = "B1";
    _brand2 = _memH.create<BrandInfo>();
    _brand2->brandCode() = "B2";
    _brand3 = _memH.create<BrandInfo>();
    _brand3->brandCode() = "B3";
    _brand4 = _memH.create<BrandInfo>();
    _brand4->brandCode() = "B4";
  }

  void tearDown() { _memH.clear(); }

  void createPaxTypeFare(PaxTypeFare* paxTypeFare, Fare* fare, FareInfo* fareInfo)
  {
    fareInfo->_carrier = "CO";
    fare->setFareInfo(fareInfo);
    paxTypeFare->setFare(fare);
  }

  void setSpecialEuropeanDoubleOJ()
  {
    PricingUnit::PUSubType ojType = PricingUnit::DOUBLE_OPENJAW;
    _pu->puSubType() = ojType;
    _pu->specialEuropeanDoubleOJ() = true;
    _pu->invalidateYYForTOJ() = false;
    _pu->inDiffCntrySameSubareaForOOJ() = false;
  }

  void setDestOJToCheckInvalidateYY()
  {
    PricingUnit::PUSubType ojType = PricingUnit::DEST_OPENJAW;
    _pu->puSubType() = ojType;
    _pu->specialEuropeanDoubleOJ() = false;
    _pu->invalidateYYForTOJ() = true;
    _pu->inDiffCntrySameSubareaForOOJ() = false;
  }

  void setOriginOJInDiffCountrySameSubarea()
  {
    PricingUnit::PUSubType ojType = PricingUnit::ORIG_OPENJAW;
    _pu->puSubType() = ojType;
    _pu->specialEuropeanDoubleOJ() = false;
    _pu->invalidateYYForTOJ() = false;
    _pu->inDiffCntrySameSubareaForOOJ() = true;
  }

  void checkBrandParity()
  {
    /*
        PricingRequest* pRequest = _memH.create<PricingRequest>();
        _trx->setRequest(pRequest);
        _trx->getRequest()->setBrandedFaresRequest(true);

        BrandProgramData* brandData1 = _memH.create<BrandProgramData>();
        BrandProgramData* brandData2 = _memH.create<BrandProgramData>();
        BrandProgramData* brandData3 = _memH.create<BrandProgramData>();
        BrandProgramData* brandData4 = _memH.create<BrandProgramData>();
        brandData1->_brandProgram = _bProgram1;
        brandData1->_brand = _brand1;

        brandData2->_brandProgram = _bProgram1;
        brandData2->_brand = _brand2;

        brandData3->_brandProgram = _bProgram2;
        brandData3->_brand = _brand3;

        brandData4->_brandProgram = _bProgram2;
        brandData4->_brand = _brand4;

        _paxTypeFare1->_brandProgramDataVec.push_back(brandData1);
        _paxTypeFare1->_brandProgramDataVec.push_back(brandData2);
        _paxTypeFare1->_brandProgramDataVec.push_back(brandData3);
        _paxTypeFare1->_brandProgramDataVec.push_back(brandData4);

        _fareUsage1->paxTypeFare() = _paxTypeFare1;
        _pricingUnit->fareUsage() += _fareUsage1;

        bool result = _factory->hasBrandParity(*_pricingUnit);

        CPPUNIT_ASSERT(result == true);

        _paxTypeFare2->_brandProgramDataVec.push_back(brandData1);
        _paxTypeFare2->_brandProgramDataVec.push_back(brandData2);

        _fareUsage2->paxTypeFare() = _paxTypeFare2;
        _pricingUnit->fareUsage() += _fareUsage2;

        result = _factory->hasBrandParity(*_pricingUnit);
        CPPUNIT_ASSERT(result == true);

        _paxTypeFare3->_brandProgramDataVec.push_back(brandData3);
        _paxTypeFare3->_brandProgramDataVec.push_back(brandData4);

        _fareUsage3->paxTypeFare() = _paxTypeFare3;
        _pricingUnit->fareUsage() += _fareUsage3;

        result = _factory->hasBrandParity(*_pricingUnit);
        CPPUNIT_ASSERT(result == false);

        _paxTypeFare3->_brandProgramDataVec.push_back(brandData2);
        result = _factory->hasBrandParity(*_pricingUnit);
        CPPUNIT_ASSERT(result == true);

        _paxTypeFare3->_brandProgramDataVec.clear();
        result = _factory->hasBrandParity(*_pricingUnit);
        CPPUNIT_ASSERT(result == false);
    */
  }

  void createNegotiatedPaxTypeFare(FareUsage& fareUsage,
                                   PaxTypeFare& paxTypeFare,
                                   Fare* fare,
                                   FareInfo* fareInfo,
                                   const RuleNumber& ruleNumber)
  {
    paxTypeFare.status().set(PaxTypeFare::PTF_Negotiated);
    fareInfo->_ruleNumber = ruleNumber;
    fare->setFareInfo(fareInfo);
    paxTypeFare.setFare(fare);
    fareUsage.paxTypeFare() = &paxTypeFare;
  }

  void createNegotiatedPaxTypeFareRuleData(PaxTypeFare& paxTypeFare, const uint32_t& itemNo)
  {
    NegPaxTypeFareRuleData* ruleData(_memH(new NegPaxTypeFareRuleData));
    CategoryRuleItemInfo* ruleItemInfo(_memH(new CategoryRuleItemInfo));
    ruleItemInfo->setItemNo(itemNo);
    ruleData->categoryRuleItemInfo() = ruleItemInfo;
    paxTypeFare.setRuleData(RuleConst::NEGOTIATED_RULE, _trx->dataHandle(), ruleData);
  }

  void testNegotiatedFarePriority() { CPPUNIT_ASSERT(DEFAULT_PRIORITY < PRIORITY_LOW); }

  void testNegotiatedFarePriorityIsDefaultWhenNoFares()
  {
    _factory->setNegotiatedFarePriority(&_pupqItem);
    CPPUNIT_ASSERT_EQUAL(DEFAULT_PRIORITY, _pupqItem.priorityStatus().negotiatedFarePriority());
  }

  void testNegotiatedFarePriorityIsDefaultWhenOneNonNegFare()
  {
    _fareUsage1->paxTypeFare() = _paxTypeFare1;
    _pricingUnit->fareUsage() += _fareUsage1;
    _factory->setNegotiatedFarePriority(&_pupqItem);
    CPPUNIT_ASSERT_EQUAL(DEFAULT_PRIORITY, _pupqItem.priorityStatus().negotiatedFarePriority());
  }

  void testNegotiatedFarePriorityIsDefaultWhenNoNegFares()
  {
    CPPUNIT_ASSERT(!_paxTypeFare1->isNegotiated());
    _fareUsage1->paxTypeFare() = _paxTypeFare1;
    CPPUNIT_ASSERT(!_paxTypeFare2->isNegotiated());
    _fareUsage2->paxTypeFare() = _paxTypeFare2;
    _pricingUnit->fareUsage() += _fareUsage1, _fareUsage2;
    _factory->setNegotiatedFarePriority(&_pupqItem);
    CPPUNIT_ASSERT_EQUAL(DEFAULT_PRIORITY, _pupqItem.priorityStatus().negotiatedFarePriority());
  }

  void testNegotiatedFarePriorityIsDefaultWhenOneNegFare()
  {
    createNegotiatedPaxTypeFare(*_fareUsage1, *_paxTypeFare1, _fare1, _fareInfo1, "XX01");
    _pricingUnit->fareUsage() += _fareUsage1;
    _factory->setNegotiatedFarePriority(&_pupqItem);
    CPPUNIT_ASSERT_EQUAL(DEFAULT_PRIORITY, _pupqItem.priorityStatus().negotiatedFarePriority());
  }

  void testNegotiatedFarePriorityIsLowWhenFaresHaveDifferentRules()
  {
    createNegotiatedPaxTypeFare(*_fareUsage1, *_paxTypeFare1, _fare1, _fareInfo1, "XX01");
    createNegotiatedPaxTypeFare(*_fareUsage2, *_paxTypeFare2, _fare2, _fareInfo2, "XX02");
    _pricingUnit->fareUsage() += _fareUsage1, _fareUsage2;
    _factory->setNegotiatedFarePriority(&_pupqItem);
    CPPUNIT_ASSERT_EQUAL(PRIORITY_LOW, _pupqItem.priorityStatus().negotiatedFarePriority());
  }

  void testNegotiatedFarePriorityIsLowWhenFaresHaveDifferentRulesSameR3s()
  {
    createNegotiatedPaxTypeFare(*_fareUsage1, *_paxTypeFare1, _fare1, _fareInfo1, "XX01");
    createNegotiatedPaxTypeFareRuleData(*_paxTypeFare1, 11111);
    createNegotiatedPaxTypeFare(*_fareUsage2, *_paxTypeFare2, _fare2, _fareInfo2, "XX02");
    createNegotiatedPaxTypeFareRuleData(*_paxTypeFare2, 11111);
    _pricingUnit->fareUsage() += _fareUsage1, _fareUsage2;
    _factory->setNegotiatedFarePriority(&_pupqItem);
    CPPUNIT_ASSERT_EQUAL(PRIORITY_LOW, _pupqItem.priorityStatus().negotiatedFarePriority());
  }

  void testNegotiatedFarePriorityIsDefaultWhenFaresHaveSameRulesSameR3s()
  {
    createNegotiatedPaxTypeFare(*_fareUsage1, *_paxTypeFare1, _fare1, _fareInfo1, "XX01");
    createNegotiatedPaxTypeFareRuleData(*_paxTypeFare1, 11111);
    createNegotiatedPaxTypeFare(*_fareUsage2, *_paxTypeFare2, _fare2, _fareInfo2, "XX01");
    createNegotiatedPaxTypeFareRuleData(*_paxTypeFare2, 11111);
    _pricingUnit->fareUsage() += _fareUsage1, _fareUsage2;
    _factory->setNegotiatedFarePriority(&_pupqItem);
    CPPUNIT_ASSERT_EQUAL(DEFAULT_PRIORITY, _pupqItem.priorityStatus().negotiatedFarePriority());
  }

  void testNegotiatedFarePriorityIsLowWhenFaresHaveSameRulesDifferentR3s()
  {
    createNegotiatedPaxTypeFare(*_fareUsage1, *_paxTypeFare1, _fare1, _fareInfo1, "XX01");
    createNegotiatedPaxTypeFareRuleData(*_paxTypeFare1, 11111);
    createNegotiatedPaxTypeFare(*_fareUsage2, *_paxTypeFare2, _fare2, _fareInfo2, "XX01");
    createNegotiatedPaxTypeFareRuleData(*_paxTypeFare2, 22222);
    _pricingUnit->fareUsage() += _fareUsage1, _fareUsage2;
    _factory->setNegotiatedFarePriority(&_pupqItem);
    CPPUNIT_ASSERT_EQUAL(PRIORITY_LOW, _pupqItem.priorityStatus().negotiatedFarePriority());
  }

  void testHigherPriorityIsOnTopWhenHigherPriorityPupqItemIsPushedLast()
  {
    createPupqItem(_pupqItem, *_pricingUnit, PRIORITY_LOW);
    createPupqItem(_pupqItem2, *_pricingUnit2, PRIORITY_LOW);
    createPupqItem(_pupqItem3, *_pricingUnit3, DEFAULT_PRIORITY);
    _factory->searchAlwaysLowToHigh() = true;
    _factory->pqPush(&_pupqItem);
    _factory->pqPush(&_pupqItem2);
    _factory->pqPush(&_pupqItem3);
    assertPopFromPriorityStack(DEFAULT_PRIORITY);
    assertPopFromPriorityStack(PRIORITY_LOW);
    assertPopFromPriorityStack(PRIORITY_LOW);
  }

  void testHigherPriorityIsOnTopWhenHigherPriorityPupqItemIsPushedFirst()
  {
    createPupqItem(_pupqItem, *_pricingUnit, DEFAULT_PRIORITY);
    createPupqItem(_pupqItem2, *_pricingUnit2, PRIORITY_LOW);

    _factory->searchAlwaysLowToHigh() = true;

    _factory->pqPush(&_pupqItem);
    _factory->pqPush(&_pupqItem2);

    assertPopFromPriorityStack(DEFAULT_PRIORITY);
    assertPopFromPriorityStack(PRIORITY_LOW);
  }

  void assertPopFromPriorityStack(PRIORITY expectedPriority)
  {
    CPPUNIT_ASSERT_EQUAL(expectedPriority, _factory->pqTop()->priorityStatus().negotiatedFarePriority());
    _factory->pqPop();
  }

  void createPupqItem(PUPQItem& pupqItem, PricingUnit& pricingUnit, PRIORITY priority)
  {
    pupqItem.pricingUnit() = &pricingUnit;
    pricingUnit.setTotalPuNucAmount(100);
    pupqItem.mutablePriorityStatus().setNegotiatedFarePriority(priority);
  }

  void testIsPUWithSameFares_TrueWhenSamePTF_Ptr()
  {
    _pricingUnit->fareUsage().push_back(_fareUsage1);
    _fareUsage1->paxTypeFare() = _paxTypeFare1;
    _pricingUnit2->fareUsage().push_back(_fareUsage2);
    _fareUsage2->paxTypeFare() = _paxTypeFare1;
    CPPUNIT_ASSERT(_pricingUnit->isPUWithSameFares(*_pricingUnit2));
  }

  void testIsPUWithSameFares_FalseWhenDiffPTF_Ptr()
  {
    _pricingUnit->fareUsage().push_back(_fareUsage1);
    _fareUsage1->paxTypeFare() = _paxTypeFare1;
    _pricingUnit2->fareUsage().push_back(_fareUsage2);
    _fareUsage2->paxTypeFare() = _paxTypeFare2;
    CPPUNIT_ASSERT(!_pricingUnit->isPUWithSameFares(*_pricingUnit2));
  }

  CombinabilityRuleInfo& createCombinabilityRuleInfo(Indicator sojorigIndestInd = ' ',
                                                     Indicator dojInd = ' ',
                                                     Indicator sojInd = ' ')
  {
    CombinabilityRuleInfo* inf = _memH(new CombinabilityRuleInfo);
    inf->dojSameCarrierInd() = Combinations::SAME_CARRIER;
    inf->sojorigIndestInd() = sojorigIndestInd;
    inf->dojInd() = dojInd;
    inf->sojInd() = sojInd;
    return *inf;
  }

  FareMarket* createFareMarket()
  {
    FareMarket* fm = _memH(new FareMarket);
    fm->paxTypeCortege().resize(1);
    fm->governingCarrierPref() = _memH(new CarrierPreference);
    return fm;
  }

  PaxTypeFare& createPaxTypeFare(CarrierCode cc = "CX", TariffNumber rt = 0)
  {
    FareInfo* fi = _memH(new FareInfo);
    fi->carrier() = cc;
    fi->currency() = NUC;

    TariffCrossRefInfo* tcr = _memH(new TariffCrossRefInfo);
    tcr->ruleTariff() = rt;

    Fare* f = _memH(new Fare);
    f->setTariffCrossRefInfo(tcr);

    f->setFareInfo(fi);
    PaxTypeFare* ptf = _memH(new PaxTypeFare);
    ptf->setFare(f);
    ptf->fareMarket() = createFareMarket();
    FareClassAppInfo* fca = _memH(new FareClassAppInfo);
    fca->_displayCatType = RuleConst::NET_SUBMIT_FARE;
    ptf->fareClassAppInfo() = fca;
    ptf->paxType() = createPaxType(ADULT);
    ptf->nucFareAmount() = 100;
    return *ptf;
  }

  enum
  { pu_sameNationOJ_no = 0,
    pu_sameNationOJ_yes = 1,
    pu_sameNationOrigSurfaceOJ_no = 0,
    pu_sameNationOrigSurfaceOJ_yes = 1 };
  enum
  { usePrevFareUsage_no = 0,
    usePrevFareUsage_yes = 1,
    buildFareUsage_no = 0,
    buildFareUsage_yes = 1,
    isPricingUnitValid_no = 0,
    isPricingUnitValid_yes = 1 };

  void testCheckOJRec2Cat10Indicator_SamePointTableApplicable()
  {
    createMockFactory();

    std::string failReason;
    CPPUNIT_ASSERT(_factory->checkOJRec2Cat10Indicator(
        100, createCombinabilityRuleInfo(), *_paxTypeFare1, failReason));

    CPPUNIT_ASSERT_EQUAL(std::string("SAME POINT TABLE APPLICABLE\n"), failReason);
  }

  void testCheckOJRec2Cat10Indicator_NotSameCxr()
  {
    createMockFactory();
    _factory->_pu->puGovCarrier() = PU::MULTI_CARRIER;

    std::string failReason;
    CPPUNIT_ASSERT(!_factory->checkOJRec2Cat10Indicator(
        0, createCombinabilityRuleInfo(), createPaxTypeFare(), failReason));

    CPPUNIT_ASSERT_EQUAL(std::string("REC2 SCORBOARD CHECK: NOT SAME CXR OJ-PU\n"), failReason);
  }

  void testCheckOJRec2Cat10Indicator_DestOpenJawReq()
  {
    createMockFactory();

    std::string failReason;
    CPPUNIT_ASSERT(!_factory->checkOJRec2Cat10Indicator(
        0, createCombinabilityRuleInfo('D'), createPaxTypeFare(), failReason));

    CPPUNIT_ASSERT_EQUAL(std::string("REC2 SCORBOARD CHECK: DEST OPEN JAW REQ\n"), failReason);
  }

  void testCheckOJRec2Cat10Indicator_OriginOpenJawReq()
  {
    createMockFactory();

    std::string failReason;
    CPPUNIT_ASSERT(!_factory->checkOJRec2Cat10Indicator(
        0, createCombinabilityRuleInfo('O'), createPaxTypeFare(), failReason));

    CPPUNIT_ASSERT_EQUAL(std::string("REC2 SCORBOARD CHECK: ORIGIN OPEN JAW REQ\n"), failReason);
  }

  void testCheckOJRec2Cat10Indicator_DiffCountryDOJRequired()
  {
    createMockFactory(PricingUnit::DOUBLE_OPENJAW, pu_sameNationOJ_yes);

    std::string failReason;
    CPPUNIT_ASSERT(!_factory->checkOJRec2Cat10Indicator(
        0, createCombinabilityRuleInfo(' ', 'U'), createPaxTypeFare(), failReason));

    CPPUNIT_ASSERT_EQUAL(std::string("REC2 SCORBOARD CHECK: DIFF COUNTRY DOJ REQUIRED\n"),
                         failReason);
  }

  void testCheckOJRec2Cat10Indicator_DiffCountrySOJRequired()
  {
    createMockFactory(PricingUnit::ORIG_OPENJAW, pu_sameNationOJ_yes);

    std::string failReason;
    CPPUNIT_ASSERT(!_factory->checkOJRec2Cat10Indicator(
        0, createCombinabilityRuleInfo(' ', ' ', 'V'), createPaxTypeFare(), failReason));

    CPPUNIT_ASSERT_EQUAL(std::string("REC2 SCORBOARD CHECK: DIFF COUNTRY SOJ REQUIRED\n"),
                         failReason);
  }

  void testCheckOJRec2Cat10Indicator_SameCountrySOJReq()
  {
    createMockFactory();

    std::string failReason;
    CPPUNIT_ASSERT(!_factory->checkOJRec2Cat10Indicator(
        0, createCombinabilityRuleInfo(' ', ' ', 'T'), createPaxTypeFare(), failReason));

    CPPUNIT_ASSERT_EQUAL(std::string("REC2 SCORBOARD CHECK: SAME COUNTRY SOJ REQ\n"), failReason);
  }

  void testCheckOJRec2Cat10Indicator_SameCountrySOJRequired()
  {
    createMockFactory(PricingUnit::ORIG_OPENJAW);

    std::string failReason;
    CPPUNIT_ASSERT(!_factory->checkOJRec2Cat10Indicator(
        0, createCombinabilityRuleInfo(' ', ' ', 'W'), createPaxTypeFare(), failReason));

    CPPUNIT_ASSERT_EQUAL(std::string("REC2 SCORBOARD CHECK: SAME COUNTRY SOJ REQUIRED\n"),
                         failReason);
  }

  void testCheckOJRec2Cat10Indicator_SameCountryDOJRequired()
  {
    createMockFactory(
        PricingUnit::DOUBLE_OPENJAW, pu_sameNationOJ_no, pu_sameNationOrigSurfaceOJ_no);

    std::string failReason;
    CPPUNIT_ASSERT(!_factory->checkOJRec2Cat10Indicator(
        0, createCombinabilityRuleInfo(' ', 'W'), createPaxTypeFare(), failReason));

    CPPUNIT_ASSERT_EQUAL(std::string("REC2 SCORBOARD CHECK: SAME COUNTRY DOJ REQUIRED\n"),
                         failReason);
  }

  void testCheckOJRec2Cat10Indicator_NotValidForDOJ()
  {
    createMockFactory();

    std::string failReason;
    CPPUNIT_ASSERT(!_factory->checkOJRec2Cat10Indicator(
        0, createCombinabilityRuleInfo(' ', 'N'), createPaxTypeFare(), failReason));

    CPPUNIT_ASSERT_EQUAL(std::string("REC2 SCORBOARD CHECK: NOT VALID FOR DOJ\n"), failReason);
  }

  void testCheckOJRec2Cat10Indicator_NotValidForSOJ()
  {
    createMockFactory(PricingUnit::ORIG_OPENJAW);

    std::string failReason;
    CPPUNIT_ASSERT(!_factory->checkOJRec2Cat10Indicator(
        0, createCombinabilityRuleInfo(' ', ' ', 'N'), createPaxTypeFare(), failReason));

    CPPUNIT_ASSERT_EQUAL(std::string("REC2 SCORBOARD CHECK: NOT VALID FOR SOJ\n"), failReason);
  }

  PricingUnit&
  createPricingUnit(PricingUnit::PUFareType puFareType, PricingUnit::PUSubType puSubType)
  {
    PricingUnit* pu = _memH(new PricingUnit);
    pu->puFareType() = puFareType;
    pu->puSubType() = puSubType;
    return *pu;
  }

  DiagCollector* createDiag605()
  {
    Diag605Collector* d = _memH(new Diag605Collector());
    d->activate();
    return d;
  }

  void testCheckOJSurfaceRestriction_DoubleOJ_newLogic()
  {
    _factory->_pu->sameNationOJ() = pu_sameNationOJ_no;
    _factory->_pu->sameNationOrigSurfaceOJ() = pu_sameNationOrigSurfaceOJ_no;
    _factory->_pu->allowNOJInZone210() = false;

    CPPUNIT_ASSERT(!_factory->checkOJSurfaceRestriction(
        createPricingUnit(PricingUnit::NL, PricingUnit::DOUBLE_OPENJAW), *_diag));
    CPPUNIT_ASSERT_EQUAL(std::string(" INVALID PU: NOT SP-PU\n"), _diag->str());
  }

  void testCheckOJSurfaceRestriction_OrigOJ_newLogic()
  {
    _factory->_pu->sameNationOJ() = pu_sameNationOJ_no;
    _factory->_pu->inDiffCntrySameSubareaForOOJ() = false;
    _factory->_pu->allowNOJInZone210() = false;

    CPPUNIT_ASSERT(!_factory->checkOJSurfaceRestriction(
        createPricingUnit(PricingUnit::NL, PricingUnit::ORIG_OPENJAW), *_diag));
    CPPUNIT_ASSERT_EQUAL(std::string(" INVALID PU: NOT SP-PU\n"), _diag->str());
  }

  void testCheckOJSurfaceRestriction_DoubleOJ_oldLogic()
  {
    _factory->_pu->sameNationOrigSurfaceOJ() = pu_sameNationOrigSurfaceOJ_no;
    _factory->_pu->allowNOJInZone210() = false;

    CPPUNIT_ASSERT(!_factory->checkOJSurfaceRestriction(
        createPricingUnit(PricingUnit::NL, PricingUnit::DOUBLE_OPENJAW), *_diag));
    CPPUNIT_ASSERT_EQUAL(std::string(" INVALID PU: NOT SP-PU\n"), _diag->str());
  }

  void testCheckOJSurfaceRestriction_OrigOJ_oldLogic()
  {
    _factory->_pu->allowNOJInZone210() = false;

    CPPUNIT_ASSERT(!_factory->checkOJSurfaceRestriction(
        createPricingUnit(PricingUnit::NL, PricingUnit::ORIG_OPENJAW), *_diag));
    CPPUNIT_ASSERT_EQUAL(std::string(" INVALID PU: NOT SP-PU\n"), _diag->str());
  }

  void testCheckSingleCurrency_AltCurrencyRequest_Fail()
  {
    _factory->itin() = createItin(GeoTravelType::International);
    PaxTypeFare& ptf = createPaxTypeFare();

    _factory->_altCurrencyRequest = true;
    _factory->_altCurrencyCode = "PLN";
    CPPUNIT_ASSERT(!_factory->checkCurrency(
        ptf, ptf.fareMarket()->paxTypeCortege()[0], *ptf.fareMarket(), BOTH, false));
  }

  void testCheckSingleCurrency_AltCurrencyRequest_Pass()
  {
    _factory->itin() = createItin(GeoTravelType::International);
    PaxTypeFare& ptf = createPaxTypeFare();

    _factory->_altCurrencyRequest = true;
    _factory->_altCurrencyCode = "NUC";
    CPPUNIT_ASSERT(_factory->checkCurrency(
        ptf, ptf.fareMarket()->paxTypeCortege()[0], *ptf.fareMarket(), BOTH, false));
  }

  Itin* createItin(GeoTravelType type = GeoTravelType::Transborder)
  {
    Itin* i = _memH(new Itin);
    i->geoTravelType() = type;
    i->calculationCurrency() = NUC;
    return i;
  }

  PaxType* createPaxType(PaxTypeCode code)
  {
    PaxType* pt = _memH(new PaxType);
    pt->paxType() = code;
    pt->paxTypeInfo() = _memH(new PaxTypeInfo);
    return pt;
  }

  void testCheckSingleCurrency_RequestedPaxTypeAlphaAlphaNumeric()
  {
    _factory->itin() = createItin(GeoTravelType::International);
    _factory->_paxType = createPaxType("GV1");
    PaxTypeFare& ptf = createPaxTypeFare();

    CPPUNIT_ASSERT(_factory->checkCurrency(
        ptf, ptf.fareMarket()->paxTypeCortege()[0], *ptf.fareMarket(), BOTH, false));
  }

  PricingTrx* createPricingTrx() { return _memH(new PricingTrx); }

  void testCheckSingleCurrency_PrimaryPaxTypeADULT_RequestedPaxTypeAlphaNumericNumeric()
  {
    _factory->_trx = createPricingTrx();
    _factory->_trx->paxType() += createPaxType(ADULT), createPaxType(MIL);
    _factory->itin() = createItin(GeoTravelType::International);
    _factory->_paxType = createPaxType("C10");
    PaxTypeFare& ptf = createPaxTypeFare();

    CPPUNIT_ASSERT(_factory->checkCurrency(
        ptf, ptf.fareMarket()->paxTypeCortege()[0], *ptf.fareMarket(), BOTH, false));
  }

  void testCheckSingleCurrency_PrimaryPaxTypeADULT_RequestedPaxTypeCHILD()
  {
    _factory->_trx = createPricingTrx();
    _factory->_trx->paxType() += createPaxType(ADULT), createPaxType(MIL);
    _factory->itin() = createItin(GeoTravelType::International);
    _factory->_paxType = createPaxType("CNN");
    PaxTypeFare& ptf = createPaxTypeFare();

    CPPUNIT_ASSERT(_factory->checkCurrency(
        ptf, ptf.fareMarket()->paxTypeCortege()[0], *ptf.fareMarket(), BOTH, false));
  }

  void testCheckSingleCurrency_OutboundCurrency()
  {
    _factory->itin() = createItin(GeoTravelType::Transborder);
    PaxTypeFare& ptf = createPaxTypeFare();

    PaxTypeBucket& cortege = ptf.fareMarket()->paxTypeCortege()[0];
    cortege.outboundCurrency() = NUC;

    CPPUNIT_ASSERT(_factory->checkCurrency(ptf, cortege, *ptf.fareMarket(), FROM, false));
  }

  void testCheckSingleCurrency_OutboundAseanCurrencies()
  {
    _factory->itin() = createItin(GeoTravelType::Transborder);
    PaxTypeFare& ptf = createPaxTypeFare();

    ptf.fareMarket()->outBoundAseanCurrencies() += "PLN", NUC, "USD";

    CPPUNIT_ASSERT(_factory->checkCurrency(
        ptf, ptf.fareMarket()->paxTypeCortege()[0], *ptf.fareMarket(), FROM, false));
  }

  void testCheckSingleCurrency_InboundCurrency()
  {
    _factory->itin() = createItin(GeoTravelType::Transborder);
    PaxTypeFare& ptf = createPaxTypeFare();

    PaxTypeBucket& cortege = ptf.fareMarket()->paxTypeCortege()[0];
    cortege.inboundCurrency() = NUC;

    CPPUNIT_ASSERT(_factory->checkCurrency(ptf, cortege, *ptf.fareMarket(), TO, false));
  }

  void testCheckSingleCurrency_InboundAseanCurrencies()
  {
    _factory->itin() = createItin(GeoTravelType::Transborder);
    PaxTypeFare& ptf = createPaxTypeFare();

    ptf.fareMarket()->inBoundAseanCurrencies() += "PLN", NUC, "USD";

    CPPUNIT_ASSERT(_factory->checkCurrency(
        ptf, ptf.fareMarket()->paxTypeCortege()[0], *ptf.fareMarket(), TO, false));
  }

  void testCheckSingleCurrency_Fail()
  {
    _factory->itin() = createItin(GeoTravelType::Transborder);
    PaxTypeFare& ptf = createPaxTypeFare();

    CPPUNIT_ASSERT(!_factory->checkCurrency(
        ptf, ptf.fareMarket()->paxTypeCortege()[0], *ptf.fareMarket(), TO, false));
  }

  void testReadTaxAndConvertCurrency_taxResponseEmpty()
  {
    PricingUnit prU;
    bool noBookingCodeTax = false;
    BookingCode bkCode1, bkCode2, bkCode3;

    MoneyAmount result = PricingUtil::readTaxAndConvertCurrency(*_trx,
                                                                0,
                                                                *createItin(),
                                                                createPaxTypeFare(),
                                                                prU,
                                                                noBookingCodeTax,
                                                                bkCode1,
                                                                bkCode2,
                                                                bkCode3,
                                                                *_diag);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, result, EPSILON);
    CPPUNIT_ASSERT(noBookingCodeTax);
  }

  void testReadTaxAndConvertCurrency_taxResponseNotSet()
  {
    PricingUnit prU;
    bool noBookingCodeTax = false;
    BookingCode bkCode1, bkCode2, bkCode3;

    MoneyAmount result = PricingUtil::readTaxAndConvertCurrency(
        *_trx, 0, *createItin(), createPaxTypeFare(), prU, noBookingCodeTax, bkCode1, bkCode2, bkCode3, *_diag);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, result, EPSILON);
    CPPUNIT_ASSERT(noBookingCodeTax);
  }

  TaxItem* createTaxItem(uint16_t segStart, uint16_t segEnd, TaxCode code, Indicator feeApplInd)
  {
    TaxItem* it = _memH(new TaxItem);

    it->_taxAmount = 100;
    it->_paymentCurrency = NUC;
    it->_segmentOrderStart = segStart;
    it->_segmentOrderEnd = segEnd;

    // it->taxCodeReg() = _memH(new TaxCodeReg);
    // it->taxCodeReg()->taxCode() = code;
    // it->taxCodeReg()->feeApplInd() = feeApplInd;
    // it->taxCodeReg()->bookingCode1() = "Y";
    // it->taxCodeReg()->seqNo() = segStart*1000;
    it->taxCode() = code;
    it->feeApplInd() = feeApplInd;
    it->bookingCode1() = "Y";
    it->seqNo() = segStart * 1000;

    return it;
  }

  TaxResponse* createTaxResponse(TaxCode code, Indicator feeApplInd = ' ')
  {
    TaxResponse* res = _memH(new TaxResponse);

    if (code == "CA1")
      res->taxItemVector() += createTaxItem(1, 2, code, feeApplInd);
    else
      res->taxItemVector() += createTaxItem(1, 1, code, feeApplInd),
          createTaxItem(2, 2, code, feeApplInd);

    return res;
  }

  AirSeg* createAirSeg()
  {
    AirSeg* seg = _memH(new AirSeg);
    seg->setBookingCode("Y");
    return seg;
  }

  void testReadTaxAndConvertCurrency_OutBoundYQFTaxCharged()
  {
    bool noBookingCodeTax = true;
    BookingCode bkCode1("Q"), bkCode2, bkCode3;

    PricingUnit prU;
    prU.isOutBoundYQFTaxCharged() = false;

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg() += createAirSeg(), createAirSeg();
    ptf.fareMarket()->direction() = FMDirection::OUTBOUND;

    TaxResponse *taxResponse(createTaxResponse("YQF", '1'));
    _factory->_itin = createItin();
    _factory->_itin->mutableTaxResponses() += taxResponse;
    _factory->_itin->travelSeg() = ptf.fareMarket()->travelSeg();

    MoneyAmount result = PricingUtil::readTaxAndConvertCurrency(
        *_trx, taxResponse, *_factory->_itin, ptf, prU, noBookingCodeTax, bkCode1, bkCode2, bkCode3, *_diag);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, result, EPSILON);
    CPPUNIT_ASSERT(prU.isOutBoundYQFTaxCharged());
    CPPUNIT_ASSERT_EQUAL(BookingCode("Y"), bkCode1);
    CPPUNIT_ASSERT_EQUAL(std::string("YQF 1-1 100 NUC 1000\n"), _diag->str());
    CPPUNIT_ASSERT(!noBookingCodeTax);
  }

  void testReadTaxAndConvertCurrency_InBoundYQFTaxCharged()
  {
    bool noBookingCodeTax = true;
    BookingCode bkCode1("Q"), bkCode2, bkCode3;

    PricingUnit prU;
    prU.isInBoundYQFTaxCharged() = false;

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg() += createAirSeg(), createAirSeg();
    ptf.fareMarket()->direction() = FMDirection::INBOUND;

    TaxResponse *taxResponse(createTaxResponse("YQF", '1'));
    _factory->_itin = createItin();
    _factory->_itin->mutableTaxResponses() += taxResponse;
    _factory->_itin->travelSeg() = ptf.fareMarket()->travelSeg();

    MoneyAmount result = PricingUtil::readTaxAndConvertCurrency(
        *_trx, taxResponse, *_factory->_itin, ptf, prU, noBookingCodeTax, bkCode1, bkCode2, bkCode3, *_diag);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, result, EPSILON);
    CPPUNIT_ASSERT(prU.isInBoundYQFTaxCharged());
    CPPUNIT_ASSERT_EQUAL(BookingCode("Y"), bkCode1);
    CPPUNIT_ASSERT_EQUAL(std::string("YQF 1-1 100 NUC 1000\n"), _diag->str());
    CPPUNIT_ASSERT(!noBookingCodeTax);
  }

  void testReadTaxAndConvertCurrency_OutBoundYRITaxCharged()
  {
    bool noBookingCodeTax = true;
    BookingCode bkCode1("Q"), bkCode2, bkCode3;

    PricingUnit prU;
    prU.isOutBoundYQFTaxCharged() = false;

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg() += createAirSeg(), createAirSeg();
    ptf.fareMarket()->direction() = FMDirection::OUTBOUND;

    TaxResponse *taxResponse(createTaxResponse("YRI", '1'));
    _factory->_itin = createItin();
    _factory->_itin->mutableTaxResponses() += taxResponse;
    _factory->_itin->travelSeg() = ptf.fareMarket()->travelSeg();

    MoneyAmount result = PricingUtil::readTaxAndConvertCurrency(
        *_trx, taxResponse, *_factory->_itin, ptf, prU, noBookingCodeTax, bkCode1, bkCode2, bkCode3, *_diag);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0, result, EPSILON);
    CPPUNIT_ASSERT(!prU.isOutBoundYQFTaxCharged());
    CPPUNIT_ASSERT_EQUAL(BookingCode("Y"), bkCode1);
    CPPUNIT_ASSERT_EQUAL(std::string("YRI 1-1 100 NUC 1000\n"
                                     "YRI 2-2 100 NUC 2000\n"),
                         _diag->str());
    CPPUNIT_ASSERT(!noBookingCodeTax);
  }

  void testReadTaxAndConvertCurrency_InBoundYRITaxCharged()
  {
    bool noBookingCodeTax = true;
    BookingCode bkCode1("Q"), bkCode2, bkCode3;

    PricingUnit prU;
    prU.isInBoundYQFTaxCharged() = false;

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg() += createAirSeg(), createAirSeg();
    ptf.fareMarket()->direction() = FMDirection::INBOUND;

    TaxResponse *taxResponse(createTaxResponse("YRI", '1'));
    _factory->_itin = createItin();
    _factory->_itin->mutableTaxResponses() += taxResponse;
    _factory->_itin->travelSeg() = ptf.fareMarket()->travelSeg();

    MoneyAmount result = PricingUtil::readTaxAndConvertCurrency(
        *_trx, taxResponse, *_factory->_itin, ptf, prU, noBookingCodeTax, bkCode1, bkCode2, bkCode3, *_diag);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0, result, EPSILON);
    CPPUNIT_ASSERT(!prU.isInBoundYQFTaxCharged());
    CPPUNIT_ASSERT_EQUAL(BookingCode("Y"), bkCode1);
    CPPUNIT_ASSERT_EQUAL(std::string("YRI 1-1 100 NUC 1000\n"
                                     "YRI 2-2 100 NUC 2000\n"),
                         _diag->str());
    CPPUNIT_ASSERT(!noBookingCodeTax);
  }

  void testReadTaxAndConvertCurrency_OutBoundYQITaxCharged()
  {
    bool noBookingCodeTax = true;
    BookingCode bkCode1("Q"), bkCode2, bkCode3;

    PricingUnit prU;

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg() += createAirSeg(), createAirSeg();
    ptf.fareMarket()->direction() = FMDirection::OUTBOUND;

    TaxResponse *taxResponse(createTaxResponse("YQI", '1'));
    _factory->_itin = createItin();
    _factory->_itin->mutableTaxResponses() += taxResponse;
    _factory->_itin->getTaxResponses().front()->taxItemVector() += createTaxItem(1, 2, "YQF", '1');
    _factory->_itin->getTaxResponses().front()->taxItemVector() += createTaxItem(1, 2, "yqI", '1');
    _factory->_itin->travelSeg() = ptf.fareMarket()->travelSeg();

    MoneyAmount result = PricingUtil::readTaxAndConvertCurrency(
        *_trx, taxResponse, *_factory->_itin, ptf, prU, noBookingCodeTax, bkCode1, bkCode2, bkCode3, *_diag);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0, result, EPSILON);
    CPPUNIT_ASSERT(prU.isOutBoundYQFTaxCharged());
    CPPUNIT_ASSERT(prU.isOutBoundYQITaxCharged());
    CPPUNIT_ASSERT(!prU.isInBoundYQFTaxCharged());
    CPPUNIT_ASSERT(!prU.isInBoundYQITaxCharged());
    CPPUNIT_ASSERT_EQUAL(BookingCode("Y"), bkCode1);
    CPPUNIT_ASSERT_EQUAL(std::string("YQI 1-1 100 NUC 1000\nYQF 1-2 100 NUC 1000\n"), _diag->str());
    CPPUNIT_ASSERT(!noBookingCodeTax);
  }

  void testReadTaxAndConvertCurrency_InBoundYQITaxCharged()
  {
    bool noBookingCodeTax = true;
    BookingCode bkCode1("Q"), bkCode2, bkCode3;

    PricingUnit prU;

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg() += createAirSeg(), createAirSeg();
    ptf.fareMarket()->direction() = FMDirection::INBOUND;

    TaxResponse *taxResponse(createTaxResponse("YQF", '1'));
    _factory->_itin = createItin();
    _factory->_itin->mutableTaxResponses() += taxResponse;
    _factory->_itin->getTaxResponses().front()->taxItemVector() += createTaxItem(1, 2, "YQI", '1');
    _factory->_itin->getTaxResponses().front()->taxItemVector() += createTaxItem(1, 2, "yqI", '1');
    _factory->_itin->travelSeg() = ptf.fareMarket()->travelSeg();

    MoneyAmount result = PricingUtil::readTaxAndConvertCurrency(
        *_trx, taxResponse, *_factory->_itin, ptf, prU, noBookingCodeTax, bkCode1, bkCode2, bkCode3, *_diag);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0, result, EPSILON);
    CPPUNIT_ASSERT(!prU.isOutBoundYQFTaxCharged());
    CPPUNIT_ASSERT(!prU.isOutBoundYQITaxCharged());
    CPPUNIT_ASSERT(prU.isInBoundYQFTaxCharged());
    CPPUNIT_ASSERT(prU.isInBoundYQITaxCharged());
    CPPUNIT_ASSERT_EQUAL(BookingCode("Y"), bkCode1);
    CPPUNIT_ASSERT_EQUAL(std::string("YQF 1-1 100 NUC 1000\nYQI 1-2 100 NUC 1000\n"), _diag->str());
    CPPUNIT_ASSERT(!noBookingCodeTax);
  }

  void testReadTaxAndConvertCurrency_NoFeeAppl()
  {
    bool noBookingCodeTax = true;
    BookingCode bkCode1("Q"), bkCode2, bkCode3;

    PricingUnit prU;

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg() += createAirSeg(), createAirSeg();

    TaxResponse *taxResponse(createTaxResponse("CA1"));
    _factory->_itin = createItin();
    _factory->_itin->mutableTaxResponses() += taxResponse;
    _factory->_itin->travelSeg() = ptf.fareMarket()->travelSeg();

    MoneyAmount result = PricingUtil::readTaxAndConvertCurrency(
        *_trx, taxResponse, *_factory->_itin, ptf, prU, noBookingCodeTax, bkCode1, bkCode2, bkCode3, *_diag);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, result, EPSILON);
    CPPUNIT_ASSERT_EQUAL(BookingCode("Y"), bkCode1);
    CPPUNIT_ASSERT_EQUAL(std::string("CA1 1-2 100 NUC 1000\n"), _diag->str());
    CPPUNIT_ASSERT(!noBookingCodeTax);
  }

  void testReadTaxAndConvertCurrency_RevertOrder()
  {
    bool noBookingCodeTax = true;
    BookingCode bkCode1("Q"), bkCode2, bkCode3;

    PricingUnit prU;

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg() += createAirSeg(), createAirSeg();

    TaxResponse *taxResponse(createTaxResponse("CH1"));
    _factory->_itin = createItin();
    _factory->_itin->mutableTaxResponses() += taxResponse;
    _factory->_itin->travelSeg() += ptf.fareMarket()->travelSeg()[1], createAirSeg(),
        createAirSeg(), ptf.fareMarket()->travelSeg()[0];

    MoneyAmount result = PricingUtil::readTaxAndConvertCurrency(
        *_trx, taxResponse, *_factory->_itin, ptf, prU, noBookingCodeTax, bkCode1, bkCode2, bkCode3, *_diag);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(50.0, result, EPSILON);
    CPPUNIT_ASSERT_EQUAL(BookingCode("Y"), bkCode1);
    CPPUNIT_ASSERT_EQUAL(std::string("CH1 1-1 100 NUC 1000\n"
                                     "CH1 2-2 100 NUC 2000\n"),
                         _diag->str());
    CPPUNIT_ASSERT(!noBookingCodeTax);
  }

  void testIsValidFareForCxrSpecificOpenJawTrueWhenOJNotActive()
  {
    setSpecialEuropeanDoubleOJ();

    CPPUNIT_ASSERT(_factory->isValidFareForCxrSpecificOpenJaw(*_paxTypeFare1));
  }

  void testIsValidFareForCxrSpecificOpenJawFalseNLfareWhenDOJActive()
  {
    _fca1._pricingCatType = 'N';
    setSpecialEuropeanDoubleOJ();

    CPPUNIT_ASSERT(!_factory->isValidFareForCxrSpecificOpenJaw(*_paxTypeFare1));
  }

  void testIsValidFareForCxrSpecificOpenJawTrueSPfareWhenDOJActive()
  {
    _fca1._pricingCatType = 'S';
    setSpecialEuropeanDoubleOJ();

    CPPUNIT_ASSERT(_factory->isValidFareForCxrSpecificOpenJaw(*_paxTypeFare1));
  }

  void testIsValidFareForCxrSpecificOpenJawFalseYYfareWhenTOJ()
  {
    setDestOJToCheckInvalidateYY();

    PaxType paxType;
    paxType.paxType() = "ADT";
    FareMarket fareMarket;

    FareInfo fareInfo;
    fareInfo._carrier = "YY";
    TariffCrossRefInfo tariffRefInfo;
    Fare fare;
    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);
    PaxTypeFare ptf;
    ptf.initialize(&fare, &paxType, &fareMarket);

    CPPUNIT_ASSERT(!_factory->isValidFareForCxrSpecificOpenJaw(ptf));
  }

  void testIsValidFareForCxrSpecificOpenJawTrueCxrfareWhenTOJ()
  {
    setDestOJToCheckInvalidateYY();
    _fareInfo1->_carrier = "CO";
    createPaxTypeFare(_paxTypeFare1, _fare1, _fareInfo1);
    _paxTypeFare1->fareMarket() = createFareMarket();

    CPPUNIT_ASSERT(_factory->isValidFareForCxrSpecificOpenJaw(*_paxTypeFare1));
  }

  void testIsValidFareForCxrSpecificOpenJawFalseMultiGovCxrfareWhenTOJ()
  {
    setDestOJToCheckInvalidateYY();
    _pu->invalidCxrForOJ().push_back("CO");
    _fareInfo1->_carrier = "CO";
    createPaxTypeFare(_paxTypeFare1, _fare1, _fareInfo1);

    CPPUNIT_ASSERT(!_factory->isValidFareForCxrSpecificOpenJaw(*_paxTypeFare1));
  }

  void testIsValidFareForCxrSpecificOpenJawFalseNLYYfareWhenOOJ()
  {
    setOriginOJInDiffCountrySameSubarea();
    _fareInfo1->_carrier = "YY";
    createPaxTypeFare(_paxTypeFare1, _fare1, _fareInfo1);

    _paxTypeFare1->fareMarket() = createFareMarket();
    _fca1._pricingCatType = 'N';

    CarrierPreference cxrPref;
    cxrPref.applyNormalFareOJInDiffCntrys() = 'N';
    _paxTypeFare1->fareMarket()->governingCarrierPref() = &cxrPref;

    CPPUNIT_ASSERT(!_factory->isValidFareForCxrSpecificOpenJaw(*_paxTypeFare1));
  }

  void testIsValidFareForCxrSpecificOpenJawFalseNotAllowNLCxrfareWhenOOJ()
  {
    setOriginOJInDiffCountrySameSubarea();
    _fareInfo1->_carrier = "CO";
    createPaxTypeFare(_paxTypeFare1, _fare1, _fareInfo1);

    _paxTypeFare1->fareMarket() = createFareMarket();
    _fca1._pricingCatType = 'N';

    CarrierPreference cxrPref;
    cxrPref.applyNormalFareOJInDiffCntrys() = 'N';
    _paxTypeFare1->fareMarket()->governingCarrierPref() = &cxrPref;

    CPPUNIT_ASSERT(!_factory->isValidFareForCxrSpecificOpenJaw(*_paxTypeFare1));
  }

  void testIsValidFareForCxrSpecificOpenJawTrueAllowNLCxrfareWhenOOJ()
  {
    setOriginOJInDiffCountrySameSubarea();
    createPaxTypeFare(_paxTypeFare1, _fare1, _fareInfo1);

    _paxTypeFare1->fareMarket() = createFareMarket();

    _fca1._pricingCatType = 'N';

    CarrierPreference cxrPref;
    cxrPref.applyNormalFareOJInDiffCntrys() = 'N';
    _paxTypeFare1->fareMarket()->governingCarrierPref() = &cxrPref;

    CPPUNIT_ASSERT(!_factory->isValidFareForCxrSpecificOpenJaw(*_paxTypeFare1));
  }

  void testIsValidFareForCxrSpecificOpenJawTrueSPfareWhenOOJ()
  {
    setOriginOJInDiffCountrySameSubarea();
    _pu->invalidCxrForOJ().push_back("CO");
    _fca1._pricingCatType = 'S';

    CPPUNIT_ASSERT(_factory->isValidFareForCxrSpecificOpenJaw(*_paxTypeFare1));
  }

  FareUsage* createFareUsage(PaxTypeFare* ptf)
  {
    FareUsage* fu = _memH(new FareUsage);
    fu->paxTypeFare() = ptf;
    return fu;
  }

  PricingUnit* createPricingUnit() { return _memH(new PricingUnit); }

  PricingUnit* createPricingUnit(PaxTypeFare* ptf)
  {
    PricingUnit* pu = createPricingUnit();
    pu->fareUsage() += createFareUsage(ptf);
    return pu;
  }

  void testBuildCxrFarePricingUnit_UsePrevFareUsage_Pass()
  {
    MockPricingUnitFactory* mock = createMockFactory();
    mock->setReturnValues(usePrevFareUsage_yes, buildFareUsage_no);

    _factory->_pu->fareMarket().resize(1);
    PricingUnitFactory::PUPQ cxrFarePUPQ;
    bool initStage = false;

    PUPQItem pqitem;
    pqitem.pricingUnit() = createPricingUnit(&createPaxTypeFare());

    ArrayVector<uint16_t> fareIndices(1, 0);
    std::deque<bool> cxrFareRest(1, false);
    uint16_t xPoint = 10;

    CPPUNIT_ASSERT(_factory->buildCxrFarePricingUnit(
        cxrFarePUPQ, initStage, pqitem, fareIndices, cxrFareRest, xPoint, *_diag));

    CPPUNIT_ASSERT(mock->usePrevFareUsageExecuted());
    CPPUNIT_ASSERT(!mock->buildFareUsageExecuted());

    CPPUNIT_ASSERT_EQUAL(size_t(1), cxrFarePUPQ.size());
    CPPUNIT_ASSERT_EQUAL(xPoint, cxrFarePUPQ.top()->xPoint());
    CPPUNIT_ASSERT_EQUAL(std::string(""), _diag->str());
  }

  void testBuildCxrFarePricingUnit_UsePrevFareUsage_Fail()
  {
    MockPricingUnitFactory* mock = createMockFactory();
    mock->setReturnValues(usePrevFareUsage_no, buildFareUsage_no);
    mock->setExpectArgsForBuildFareUsage(0, 0, false, "");

    _factory->_pu->fareMarket().resize(1);
    PricingUnitFactory::PUPQ cxrFarePUPQ;
    bool initStage = false;

    PUPQItem pqitem;
    pqitem.pricingUnit() = createPricingUnit(&createPaxTypeFare());

    ArrayVector<uint16_t> fareIndices(1, 0);
    std::deque<bool> cxrFareRest(1, false);
    uint16_t xPoint = 10;

    CPPUNIT_ASSERT(!_factory->buildCxrFarePricingUnit(
        cxrFarePUPQ, initStage, pqitem, fareIndices, cxrFareRest, xPoint, *_diag));

    CPPUNIT_ASSERT(mock->usePrevFareUsageExecuted());
    CPPUNIT_ASSERT(mock->buildFareUsageExecuted());

    CPPUNIT_ASSERT_EQUAL(size_t(0), cxrFarePUPQ.size());
    CPPUNIT_ASSERT_EQUAL(std::string(" NO MORE VALID FARE FOUND IN THIS MARKET FOR THIS PU\n"),
                         _diag->str());
  }

  void testBuildCxrFarePricingUnit_BuildFareUsage_Pass()
  {
    MockPricingUnitFactory* mock = createMockFactory();
    mock->setReturnValues(usePrevFareUsage_no, buildFareUsage_yes);
    mock->setExpectArgsForBuildFareUsage(0, 0, false, "");

    _factory->_pu->fareMarket().resize(1);
    PricingUnitFactory::PUPQ cxrFarePUPQ;
    bool initStage = false;

    PUPQItem pqitem;
    pqitem.pricingUnit() = createPricingUnit(&createPaxTypeFare());

    ArrayVector<uint16_t> fareIndices(1, 0);
    std::deque<bool> cxrFareRest(1, false);
    uint16_t xPoint = 10;

    CPPUNIT_ASSERT(_factory->buildCxrFarePricingUnit(
        cxrFarePUPQ, initStage, pqitem, fareIndices, cxrFareRest, xPoint, *_diag));

    CPPUNIT_ASSERT(mock->usePrevFareUsageExecuted());
    CPPUNIT_ASSERT(mock->buildFareUsageExecuted());

    CPPUNIT_ASSERT_EQUAL(size_t(1), cxrFarePUPQ.size());
    CPPUNIT_ASSERT_EQUAL(xPoint, cxrFarePUPQ.top()->xPoint());
    CPPUNIT_ASSERT_EQUAL(std::string(""), _diag->str());
  }

  void testBuildCxrFarePricingUnit_IndustryCarrier()
  {
    MockPricingUnitFactory* mock = createMockFactory();
    mock->setReturnValues(usePrevFareUsage_no, buildFareUsage_yes);
    mock->setExpectArgsForBuildFareUsage(0, 0, false, "");

    _factory->_pu->fareMarket().resize(1);
    PricingUnitFactory::PUPQ cxrFarePUPQ;
    bool initStage = false;

    PUPQItem pqitem;
    pqitem.pricingUnit() = createPricingUnit(&createPaxTypeFare("YY"));

    ArrayVector<uint16_t> fareIndices(1, 0);
    std::deque<bool> cxrFareRest(1, true);
    uint16_t xPoint = 10;

    CPPUNIT_ASSERT(_factory->buildCxrFarePricingUnit(
        cxrFarePUPQ, initStage, pqitem, fareIndices, cxrFareRest, xPoint, *_diag));

    CPPUNIT_ASSERT(!mock->usePrevFareUsageExecuted());
    CPPUNIT_ASSERT(mock->buildFareUsageExecuted());

    CPPUNIT_ASSERT_EQUAL(size_t(1), cxrFarePUPQ.size());
    CPPUNIT_ASSERT_EQUAL(xPoint, cxrFarePUPQ.top()->xPoint());
    CPPUNIT_ASSERT_EQUAL(std::string(""), _diag->str());
  }

  void testBuildCxrFarePricingUnit_xPoint()
  {
    MockPricingUnitFactory* mock = createMockFactory();
    mock->setReturnValues(usePrevFareUsage_no, buildFareUsage_yes);
    mock->setExpectArgsForBuildFareUsage(0, 1, false, "");

    _factory->_pu->fareMarket().resize(1);
    PricingUnitFactory::PUPQ cxrFarePUPQ;
    bool initStage = false;

    PUPQItem pqitem;
    pqitem.pricingUnit() = createPricingUnit(&createPaxTypeFare());

    ArrayVector<uint16_t> fareIndices(1, 0);
    std::deque<bool> cxrFareRest(1, true);
    uint16_t xPoint = 0;

    CPPUNIT_ASSERT(_factory->buildCxrFarePricingUnit(
        cxrFarePUPQ, initStage, pqitem, fareIndices, cxrFareRest, xPoint, *_diag));

    CPPUNIT_ASSERT(!mock->usePrevFareUsageExecuted());
    CPPUNIT_ASSERT(mock->buildFareUsageExecuted());

    CPPUNIT_ASSERT_EQUAL(size_t(1), cxrFarePUPQ.size());
    CPPUNIT_ASSERT_EQUAL(xPoint, cxrFarePUPQ.top()->xPoint());
    CPPUNIT_ASSERT_EQUAL(std::string(""), _diag->str());
  }

  void testBuildNextLevelPricinUnit_PqEmpty()
  {
    CPPUNIT_ASSERT(!_factory->buildNextLevelPricinUnit(*_diag));
    CPPUNIT_ASSERT(_factory->_done);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _diag->str());
  }

  PUPQItem* createPUPQItem(bool valid = true, MoneyAmount amt = 0.0, PRIORITY pr = DEFAULT_PRIORITY)
  {
    PUPQItem* it = _factory->_pupqPool.construct();
    it->pricingUnit() = _factory->_puPool.construct();
    it->isValid() = valid;
    it->pricingUnit()->setTotalPuNucAmount(amt);
    it->mutablePriorityStatus().setFarePriority(pr);
    return it;
  }

  void testBuildNextLevelPricinUnit_InvalidItem()
  {
    _factory->pqPush(createPUPQItem(false));

    CPPUNIT_ASSERT(_factory->buildNextLevelPricinUnit(*_diag));
    CPPUNIT_ASSERT(_factory->_done);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), _factory->_puCombTried);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), _factory->_curCombTried);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" REQUESTED PAXTYPE:     AMOUNT: 0\n\n"
                    "***************************************************************\n"),
        _diag->str());
  }

  void testBuildNextLevelPricinUnit_ValidItem()
  {
    createMockFactory()->setReturnValues(isPricingUnitValid_yes);

    _factory->pqPush(createPUPQItem());
    _factory->pqPush(createPUPQItem());

    CPPUNIT_ASSERT(_factory->buildNextLevelPricinUnit(*_diag));
    CPPUNIT_ASSERT(!_factory->_done);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), _factory->_puCombTried);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), _factory->_curCombTried);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _diag->str());
  }

  void testBuildNextLevelPricinUnit_ShutdownFactory()
  {
    createMockFactory()->setReturnValues(isPricingUnitValid_no);

    _factory->pqPush(createPUPQItem());
    _factory->_shutdownFactory = true;

    CPPUNIT_ASSERT(_factory->buildNextLevelPricinUnit(*_diag));
    CPPUNIT_ASSERT(!_factory->_done);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), _factory->_puCombTried);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), _factory->_curCombTried);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _diag->str());
  }

  void testBuildNextLevelPricinUnit_RequestPUCount()
  {
    createMockFactory()->setReturnValues(isPricingUnitValid_yes);

    _factory->pqPush(createPUPQItem());
    _factory->pqPush(createPUPQItem());
    _factory->pqPush(createPUPQItem());
    _factory->_reqPUCount = 2;

    CPPUNIT_ASSERT(_factory->buildNextLevelPricinUnit(*_diag));
    CPPUNIT_ASSERT(!_factory->_done);
    CPPUNIT_ASSERT_EQUAL(uint32_t(2), _factory->_puCombTried);
    CPPUNIT_ASSERT_EQUAL(uint32_t(2), _factory->_curCombTried);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _diag->str());
  }

  void testBuildPricingUnit_UsePrevFareUsage_Pass()
  {
    MockPricingUnitFactory* mock = createMockFactory();
    mock->setReturnValues(usePrevFareUsage_yes, buildFareUsage_no);

    _factory->_pu->fareMarket().resize(1);

    PUPQItem pqitem;
    pqitem.pricingUnit() = createPricingUnit(&createPaxTypeFare());

    ArrayVector<uint16_t> fareIndices(1, 0);
    uint16_t xPoint = 10;

    CPPUNIT_ASSERT(_factory->buildPricingUnit(&pqitem, fareIndices, xPoint, *_diag));

    CPPUNIT_ASSERT(mock->usePrevFareUsageExecuted());
    CPPUNIT_ASSERT(!mock->buildFareUsageExecuted());

    CPPUNIT_ASSERT_EQUAL(size_t(1), _factory->puPQ().size());
    CPPUNIT_ASSERT_EQUAL(xPoint, _factory->puPQ().top()->xPoint());
    CPPUNIT_ASSERT_EQUAL(std::string(""), _diag->str());
  }

  void testBuildPricingUnit_UsePrevFareUsage_Fail()
  {
    MockPricingUnitFactory* mock = createMockFactory();
    mock->setReturnValues(usePrevFareUsage_no, buildFareUsage_no);
    mock->setExpectArgsForBuildFareUsage(0, 0, false, "");

    _factory->_pu->fareMarket().resize(1);

    PUPQItem pqitem;
    pqitem.pricingUnit() = createPricingUnit(&createPaxTypeFare());

    ArrayVector<uint16_t> fareIndices(1, 0);
    uint16_t xPoint = 10;

    CPPUNIT_ASSERT(_factory->buildPricingUnit(&pqitem, fareIndices, xPoint, *_diag));

    CPPUNIT_ASSERT(mock->usePrevFareUsageExecuted());
    CPPUNIT_ASSERT(mock->buildFareUsageExecuted());

    CPPUNIT_ASSERT_EQUAL(size_t(0), _factory->puPQ().size());
    CPPUNIT_ASSERT_EQUAL(std::string(" NO MORE VALID FARE FOUND IN THIS MARKET FOR THIS PU\n"),
                         _diag->str());
  }

  void testBuildPricingUnit_BuildFareUsage_Pass()
  {
    MockPricingUnitFactory* mock = createMockFactory();
    mock->setReturnValues(usePrevFareUsage_no, buildFareUsage_yes);
    mock->setExpectArgsForBuildFareUsage(0, 0, false, "");

    _factory->_pu->fareMarket().resize(1);

    PUPQItem pqitem;
    pqitem.pricingUnit() = createPricingUnit(&createPaxTypeFare());

    ArrayVector<uint16_t> fareIndices(1, 0);
    uint16_t xPoint = 10;

    CPPUNIT_ASSERT(_factory->buildPricingUnit(&pqitem, fareIndices, xPoint, *_diag));

    CPPUNIT_ASSERT(mock->usePrevFareUsageExecuted());
    CPPUNIT_ASSERT(mock->buildFareUsageExecuted());

    CPPUNIT_ASSERT_EQUAL(size_t(1), _factory->puPQ().size());
    CPPUNIT_ASSERT_EQUAL(xPoint, _factory->puPQ().top()->xPoint());
    CPPUNIT_ASSERT_EQUAL(std::string(""), _diag->str());
  }

  void testBuildPricingUnit_xPoint()
  {
    MockPricingUnitFactory* mock = createMockFactory();
    mock->setReturnValues(usePrevFareUsage_no, buildFareUsage_yes);
    mock->setExpectArgsForBuildFareUsage(0, 1, false, "");

    _factory->_pu->fareMarket().resize(1);

    PUPQItem pqitem;
    pqitem.pricingUnit() = createPricingUnit(&createPaxTypeFare());

    ArrayVector<uint16_t> fareIndices(1, 0);
    uint16_t xPoint = 0;

    CPPUNIT_ASSERT(_factory->buildPricingUnit(&pqitem, fareIndices, xPoint, *_diag));

    CPPUNIT_ASSERT(!mock->usePrevFareUsageExecuted());
    CPPUNIT_ASSERT(mock->buildFareUsageExecuted());

    CPPUNIT_ASSERT_EQUAL(size_t(1), _factory->puPQ().size());
    CPPUNIT_ASSERT_EQUAL(xPoint, _factory->puPQ().top()->xPoint());
    CPPUNIT_ASSERT_EQUAL(std::string(""), _diag->str());
  }

  void testSameBookingCodes_FailOnSize()
  {
    PaxTypeFare::SegmentStatus ss1, ss2;
    PaxTypeFare::SegmentStatusVec v1, v2;

    ss1._bkgCodeReBook = "Y";
    ss2._bkgCodeReBook = "Y";

    v1.push_back(ss1);
    v1.push_back(ss2);
    v2.push_back(ss1);

    CPPUNIT_ASSERT(!_factory->sameBookingCodes(v1, v2));
  }

  void testSameBookingCodes_FailOnValue()
  {
    PaxTypeFare::SegmentStatus ss1, ss2;
    PaxTypeFare::SegmentStatusVec v1, v2;

    ss1._bkgCodeReBook = "Y";
    ss2._bkgCodeReBook = "N";

    v1.push_back(ss1);
    v2.push_back(ss2);

    CPPUNIT_ASSERT(!_factory->sameBookingCodes(v1, v2));
  }

  void testSameBookingCodes_Pass()
  {
    PaxTypeFare::SegmentStatus ss1, ss2;
    PaxTypeFare::SegmentStatusVec v1, v2;

    ss1._bkgCodeReBook = "Y";
    ss2._bkgCodeReBook = "Y";

    v1.push_back(ss1);
    v2.push_back(ss2);

    CPPUNIT_ASSERT(_factory->sameBookingCodes(v1, v2));
  }

  static const std::string _fType;

  void testGetNextCxrFarePUPQItemImpl_PUPQItem_LowPriority()
  {
    createMockFactory()->setReturnValues(isPricingUnitValid_no);

    PUPQItem pqitem, *expect = 0;
    std::deque<bool> cxrFareRest;
    CarrierCode valCxr;

    _factory->_cxrFareComboMap[_fType] = PricingUnitFactory::CXRFareCombo();
    _factory->_cxrFareComboMap[_fType].cxrFarePUPQ().push(createPUPQItem(true, 10.0, PRIORITY_LOW));
    _factory->_cxrFareComboMap[_fType].cxrFarePUPQ().push(createPUPQItem(true, 20.0, PRIORITY_LOW));

    PUPQItem* result =
        _factory->getNextCxrFarePUPQItemImpl(pqitem, true, _fType, valCxr, cxrFareRest, *_diag);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(size_t(1), _factory->_cxrFareComboMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _factory->_cxrFareComboMap[_fType].validCxrPUPQItem().size());
    CPPUNIT_ASSERT(!_factory->_cxrFareComboMap[_fType].done());
  }

  void testGetNextCxrFarePUPQItemImpl_PUPQItem_DefaultPriority()
  {
    createMockFactory()->setReturnValues(isPricingUnitValid_no);

    PUPQItem pqitem, *expect = createPUPQItem(true, 10.0);
    std::deque<bool> cxrFareRest;
    CarrierCode valCxr;

    _factory->_cxrFareComboMap[_fType] = PricingUnitFactory::CXRFareCombo();
    _factory->_cxrFareComboMap[_fType].cxrFarePUPQ().push(createPUPQItem(true, 20.0));
    _factory->_cxrFareComboMap[_fType].cxrFarePUPQ().push(expect);

    PUPQItem* result =
        _factory->getNextCxrFarePUPQItemImpl(pqitem, true, _fType, valCxr, cxrFareRest, *_diag);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(0, result->cxrFareComboIdx());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _factory->_cxrFareComboMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _factory->_cxrFareComboMap[_fType].validCxrPUPQItem().size());
    CPPUNIT_ASSERT(!_factory->_cxrFareComboMap[_fType].done());
  }

  void testGetNextCxrFarePUPQItemImpl_NoPUPQItem()
  {
    createMockFactory()->setReturnValues(isPricingUnitValid_no);

    PUPQItem pqitem, *expect = 0;
    std::deque<bool> cxrFareRest;
    CarrierCode valCxr;

    PUPQItem* result =
        _factory->getNextCxrFarePUPQItemImpl(pqitem, true, _fType, valCxr, cxrFareRest, *_diag);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(size_t(1), _factory->_cxrFareComboMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _factory->_cxrFareComboMap[_fType].validCxrPUPQItem().size());
    CPPUNIT_ASSERT(_factory->_cxrFareComboMap[_fType].done());
  }

  void testGetNextCxrFarePUPQItemImpl_ValidCxrPUPQItem_FarePriority()
  {
    createMockFactory()->setReturnValues(isPricingUnitValid_no);

    PUPQItem pqitem, expect;
    std::deque<bool> cxrFareRest;
    CarrierCode valCxr;

    pqitem.mutablePriorityStatus().setFarePriority(PRIORITY_LOW);
    _factory->_cxrFareComboMap[_fType] = PricingUnitFactory::CXRFareCombo();
    _factory->_cxrFareComboMap[_fType].validCxrPUPQItem() += &expect;

    PUPQItem* result =
        _factory->getNextCxrFarePUPQItemImpl(pqitem, true, _fType, valCxr, cxrFareRest, *_diag);

    CPPUNIT_ASSERT_EQUAL(&expect, result);
    CPPUNIT_ASSERT_EQUAL(size_t(1), _factory->_cxrFareComboMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _factory->_cxrFareComboMap[_fType].validCxrPUPQItem().size());
    CPPUNIT_ASSERT(!_factory->_cxrFareComboMap[_fType].done());
  }

  void testGetNextCxrFarePUPQItemImpl_ValidCxrPUPQItem_CxrFareComboIdx()
  {
    createMockFactory()->setReturnValues(isPricingUnitValid_no);

    PUPQItem pqitem, expect, other;
    std::deque<bool> cxrFareRest;

    pqitem.mutablePriorityStatus().setFarePriority(DEFAULT_PRIORITY);
    pqitem.cxrFareComboIdx() = 1;
    _factory->_cxrFareComboMap[_fType] = PricingUnitFactory::CXRFareCombo();
    _factory->_cxrFareComboMap[_fType].validCxrPUPQItem() += &other, &other, &expect;
    CarrierCode valCxr;

    PUPQItem* result =
        _factory->getNextCxrFarePUPQItemImpl(pqitem, true, _fType, valCxr, cxrFareRest, *_diag);

    CPPUNIT_ASSERT_EQUAL(&expect, result);
    CPPUNIT_ASSERT_EQUAL(size_t(1), _factory->_cxrFareComboMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _factory->_cxrFareComboMap[_fType].validCxrPUPQItem().size());
    CPPUNIT_ASSERT(!_factory->_cxrFareComboMap[_fType].done());
  }

  void testGetNextCxrFarePUPQItemImpl_ValidCxrPUPQItem_CxrFareComboIdxWithXPoint()
  {
    createMockFactory()->setReturnValues(isPricingUnitValid_no);

    PUPQItem pqitem, expect, other;
    std::deque<bool> cxrFareRest;
    CarrierCode valCxr;

    pqitem.mutablePriorityStatus().setFarePriority(DEFAULT_PRIORITY);
    pqitem.cxrFareComboIdx() = 1;
    _factory->_cxrFareComboMap[_fType] = PricingUnitFactory::CXRFareCombo();
    _factory->_cxrFareComboMap[_fType].validCxrPUPQItem() += &other, &other, &expect;

    PUPQItem* result =
        _factory->getNextCxrFarePUPQItemImpl(pqitem, false, _fType, valCxr, cxrFareRest, *_diag);

    CPPUNIT_ASSERT_EQUAL(&other, result);
    CPPUNIT_ASSERT_EQUAL(size_t(1), _factory->_cxrFareComboMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _factory->_cxrFareComboMap[_fType].validCxrPUPQItem().size());
    CPPUNIT_ASSERT(!_factory->_cxrFareComboMap[_fType].done());
  }

  void testGetNextCxrFarePUPQItemImpl_CxrFareCombo_Done()
  {
    createMockFactory()->setReturnValues(isPricingUnitValid_no);

    PUPQItem pqitem, *expect = 0;
    std::deque<bool> cxrFareRest;
    CarrierCode valCxr;

    _factory->_cxrFareComboMap[_fType] = PricingUnitFactory::CXRFareCombo();
    _factory->_cxrFareComboMap[_fType].done() = 0;

    PUPQItem* result =
        _factory->getNextCxrFarePUPQItemImpl(pqitem, true, _fType, valCxr, cxrFareRest, *_diag);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(size_t(1), _factory->_cxrFareComboMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _factory->_cxrFareComboMap[_fType].validCxrPUPQItem().size());
    CPPUNIT_ASSERT(_factory->_cxrFareComboMap[_fType].done());
  }

  void testCheckRTCTIATAExceptions_DifferentMilagePercentage()
  {
    PaxTypeFare& ptf1 = createPaxTypeFare();
    ptf1.mileageSurchargePctg() = 1;
    PricingUnit* pu = createPricingUnit(&ptf1);

    PaxTypeFare& ptf2 = createPaxTypeFare();
    ptf2.mileageSurchargePctg() = 2;
    pu->fareUsage() += createFareUsage(&ptf2);

    CPPUNIT_ASSERT(_factory->processRTCTIATAExceptions(*pu, *_diag));
  }

  void testCheckRTCTIATAExceptions_TravelWithinUsca()
  {
    PaxTypeFare& ptf1 = createPaxTypeFare();
    ptf1.fareMarket()->travelBoundary().set(FMTravelBoundary::TravelWithinUSCA);
    PricingUnit* pu = createPricingUnit(&ptf1);

    PaxTypeFare& ptf2 = createPaxTypeFare();
    ptf2.fareMarket()->travelBoundary().set(FMTravelBoundary::TravelWithinUSCA);
    pu->fareUsage() += createFareUsage(&ptf2);

    CPPUNIT_ASSERT(!_factory->processRTCTIATAExceptions(*pu, *_diag));
  }

  void testCheckRTCTIATAExceptions_SameNucFareAmount()
  {
    PaxTypeFare& ptf1 = createPaxTypeFare();
    ptf1.nucFareAmount() = 1;
    PricingUnit* pu = createPricingUnit(&ptf1);

    PaxTypeFare& ptf2 = createPaxTypeFare();
    ptf2.nucFareAmount() = 1;
    pu->fareUsage() += createFareUsage(&ptf2);

    CPPUNIT_ASSERT(!_factory->processRTCTIATAExceptions(*pu, *_diag));
  }

  void testCheckRTCTIATAExceptions_CarriersApplyDifferentNuc()
  {
    PaxTypeFare& ptf1 = createPaxTypeFare();
    ptf1.nucFareAmount() = 0;
    const_cast<CarrierPreference*>(ptf1.fareMarket()->governingCarrierPref())->applysamenuctort() =
        'N';
    PricingUnit* pu = createPricingUnit(&ptf1);

    PaxTypeFare& ptf2 = createPaxTypeFare();
    ptf2.nucFareAmount() = EPSILON + 1;
    pu->fareUsage() += createFareUsage(&ptf2);

    CPPUNIT_ASSERT(!_factory->processRTCTIATAExceptions(*pu, *_diag));
  }

  void createPaxTypeFare(PaxTypeFare* paxTypeFare)
  {
    paxTypeFare->nucFareAmount() = 0;
    paxTypeFare->cabin().setFirstClass();
    CarrierPreference* cp =
        const_cast<CarrierPreference*>(paxTypeFare->fareMarket()->governingCarrierPref());
    cp->applysamenuctort() = 'Y';
    cp->carrier() = "AA";
  }

  void testCheckRTCTIATAExceptions_SpecialNormalFaresCombo()
  {
    PaxTypeFare& ptf1 = createPaxTypeFare();
    createPaxTypeFare(&ptf1);
    ptf1.fareTypeApplication() = 'S';
    PricingUnit* pu = createPricingUnit(&ptf1);

    PaxTypeFare& ptf2 = createPaxTypeFare();
    ptf2.nucFareAmount() = EPSILON + 1;
    ptf2.fareTypeApplication() = 'N';
    pu->fareUsage() += createFareUsage(&ptf2);

    CPPUNIT_ASSERT(_factory->processRTCTIATAExceptions(*pu, *_diag));
  }

  void testCheckRTCTIATAExceptions_DifferentCarrier()
  {
    PaxTypeFare& ptf1 = createPaxTypeFare();
    createPaxTypeFare(&ptf1);
    PricingUnit* pu = createPricingUnit(&ptf1);

    PaxTypeFare& ptf2 = createPaxTypeFare();
    ptf2.nucFareAmount() = EPSILON + 1;
    const_cast<CarrierPreference*>(ptf2.fareMarket()->governingCarrierPref())->carrier() = "UA";
    pu->fareUsage() += createFareUsage(&ptf2);

    CPPUNIT_ASSERT(!_factory->processRTCTIATAExceptions(*pu, *_diag));
  }

  void testCheckRTCTIATAExceptions_DifferentCabin()
  {
    PaxTypeFare& ptf1 = createPaxTypeFare();
    createPaxTypeFare(&ptf1);
    ptf1.cabin().setFirstClass();
    PricingUnit* pu = createPricingUnit(&ptf1);

    PaxTypeFare& ptf2 = createPaxTypeFare();
    createPaxTypeFare(&ptf2);
    ptf2.nucFareAmount() = EPSILON + 1;
    ptf2.cabin().setEconomyClass();
    pu->fareUsage() += createFareUsage(&ptf2);

    CPPUNIT_ASSERT(!_factory->processRTCTIATAExceptions(*pu, *_diag));
  }

  void testCheckRTCTIATAExceptions_Cat25OrCat35()
  {
    PaxTypeFare& ptf1 = createPaxTypeFare();
    createPaxTypeFare(&ptf1);
    ptf1.status().set(PaxTypeFare::PTF_Negotiated);
    PricingUnit* pu = createPricingUnit(&ptf1);

    PaxTypeFare& ptf2 = createPaxTypeFare();
    createPaxTypeFare(&ptf2);
    ptf2.nucFareAmount() = EPSILON + 1;
    pu->fareUsage() += createFareUsage(&ptf2);

    CPPUNIT_ASSERT(_factory->processRTCTIATAExceptions(*pu, *_diag));
  }

  void testCheckRTCTIATAExceptions_DifferentCat2()
  {
    PaxTypeFare& ptf1 = createPaxTypeFare();
    createPaxTypeFare(&ptf1);
    const_cast<FareClassAppInfo*>(ptf1.fareClassAppInfo())->_dowType = 'W';
    PricingUnit* pu = createPricingUnit(&ptf1);

    PaxTypeFare& ptf2 = createPaxTypeFare();
    createPaxTypeFare(&ptf2);
    ptf2.nucFareAmount() = EPSILON + 1;
    const_cast<FareClassAppInfo*>(ptf2.fareClassAppInfo())->_dowType = 'X';
    pu->fareUsage() += createFareUsage(&ptf2);

    CPPUNIT_ASSERT(!_factory->processRTCTIATAExceptions(*pu, *_diag));
  }

  void testCheckRTCTIATAExceptions_SameCat2()
  {
    PaxTypeFare& ptf1 = createPaxTypeFare();
    createPaxTypeFare(&ptf1);
    const_cast<FareClassAppInfo*>(ptf1.fareClassAppInfo())->_dowType = 'X';
    PricingUnit* pu = createPricingUnit(&ptf1);

    PaxTypeFare& ptf2 = createPaxTypeFare();
    createPaxTypeFare(&ptf2);
    ptf2.nucFareAmount() = EPSILON + 1;
    const_cast<FareClassAppInfo*>(ptf2.fareClassAppInfo())->_dowType = 'X';
    pu->fareUsage() += createFareUsage(&ptf2);

    if (!_factory->_trx->getRequest())
      _factory->_trx->setRequest(_memH(new PricingRequest));

    CPPUNIT_ASSERT(_factory->processRTCTIATAExceptions(*pu, *_diag));
  }

  typedef std::vector<PaxTypeFare::SegmentStatus> SegmentStatusVec;

  PaxTypeFare& createPaxTypeFare(const SegmentStatusVec& segStat)
  {
    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg() += createAirSeg();
    ptf.segmentStatus() = segStat;
    return ptf;
  }

  void populateTJRAgent()
  {
    PricingRequest* rq = _memH(new PricingRequest);
    _factory->_trx->setRequest(rq);
    rq->ticketingAgent() = _memH(new Agent);
    Customer*& c = rq->ticketingAgent()->agentTJR() = _memH(new Customer);
    c->crsCarrier() = "1J";
    c->hostName() = "AXES";
  }

  PricingUnitFactory* setupIsFareValidForBuildFareUsage()
  {
    MockPricingUnitFactory* mock = createMockFactory();
    createPaxTypeFare(_paxTypeFare1, _fare1, _fareInfo1);
    _fare1->setTariffCrossRefInfo(_memH(new TariffCrossRefInfo));
    _paxTypeFare1->fareMarket() = _fareMarket1;
    _fareInfo1->currency() = NUC;
    mock->_pu->fareDirectionality().push_back(_paxTypeFare1->directionality());
    mock->_altCurrencyRequest = true;
    mock->_altCurrencyCode = NUC;
    populateTJRAgent();
    _factory->_itin = createItin();

    return mock;
  }

  void setMergedFareMarketDirection()
  {
    _mfm->mergedFareMarket().push_back(_fareMarket1);
    _mfm->direction() = FMDirection::OUTBOUND;
    _factory->_pu->fareMarket().push_back(_mfm);
  }

  PricingUnitFactory* setupGetNextCxrFarePricinUnit(bool isValid)
  {
    MockPricingUnitFactory* mock = createMockFactory();

    mock->setNextCxrFarePricinUnitMock(false);
    mock->setReturnValues(isValid);

    return mock;
  }

  void testGetNextCxrFarePricinUnit_Fail_Empty()
  {
    PricingUnitFactory* mock = setupGetNextCxrFarePricinUnit(true);
    PricingUnitFactory::PUPQ pq;
    std::deque<bool> cxrFareRest;

    CPPUNIT_ASSERT_EQUAL((PUPQItem*)NULL, mock->getNextCxrFarePricinUnit(pq, cxrFareRest, *createDiag605()));
  }

  void testGetNextCxrFarePricinUnit_FailNotValid()
  {
    PricingUnitFactory* mock = setupGetNextCxrFarePricinUnit(false);
    PricingUnitFactory::PUPQ pq;
    pq.push(createPUPQItem());
    std::deque<bool> cxrFareRest;

    CPPUNIT_ASSERT_EQUAL((PUPQItem*)NULL, mock->getNextCxrFarePricinUnit(pq, cxrFareRest, *createDiag605()));
  }

  void testGetNextCxrFarePricinUnit_Pass()
  {
    PricingUnitFactory* mock = setupGetNextCxrFarePricinUnit(true);
    PricingUnitFactory::PUPQ pq;
    PUPQItem* pqItem = createPUPQItem();
    pq.push(pqItem);
    std::deque<bool> cxrFareRest;

    CPPUNIT_ASSERT_EQUAL(pqItem, mock->getNextCxrFarePricinUnit(pq, cxrFareRest, *createDiag605()));
  }

  PricingUnitFactory* setupIsPricingUnitValid(bool pULevel)
  {
    MockPricingUnitFactory* mock = createMockFactory();
    mock->setCheckPULevelCombinabilityReturnValue(pULevel);
    mock->setPerformPULevelRuleValidationReturnValue(pULevel);
    populateTJRAgent();
    mock->_paxType = createPaxType(ADULT);

    mock->setIsPricingUnitValidMock(false);

    return mock;
  }

  void testIsPricingUnitValid_Pass()
  {
    PricingUnitFactory* mock = setupIsPricingUnitValid(true);

    CPPUNIT_ASSERT(mock->isPricingUnitValid(*createPUPQItem(), false, *createDiag605()));
  }

  void setRecord1Indicator(PaxTypeFare* p1, PaxTypeFare* p2, char v1, char v2, uint16_t catNum)
  {
    FareClassAppInfo* f1 = const_cast<FareClassAppInfo*>(p1->fareClassAppInfo());
    FareClassAppInfo* f2 = const_cast<FareClassAppInfo*>(p2->fareClassAppInfo());
    if (RuleConst::DAY_TIME_RULE == catNum)
    {
      f1->_dowType = v1;
      f2->_dowType = v2;
    }
    else if (RuleConst::SEASONAL_RULE == catNum)
    {
      f1->_seasonType = v1;
      f2->_seasonType = v2;
    }
  }

  void setGeneralFareRuleInfo(const GeneralFareRuleInfo* gr, char v, uint16_t catNum)
  {
    if (gr)
    {
      if (RuleConst::DAY_TIME_RULE == catNum)
        const_cast<GeneralFareRuleInfo*>(gr)->dowType() = v;
      else if (RuleConst::SEASONAL_RULE == catNum)
        const_cast<GeneralFareRuleInfo*>(gr)->seasonType() = v;
    }
  }

  void setRecord2Indicator(PaxTypeFare& ptf, char v, uint16_t catNum)
  {
    PaxTypeFareRuleData* prd = ptf.paxTypeFareRuleData(catNum);
    if (!prd)
    {
      setPaxTypeFareRuleData(ptf, catNum);
      prd = ptf.paxTypeFareRuleData(catNum);
    }

    setGeneralFareRuleInfo(
        dynamic_cast<const GeneralFareRuleInfo*>(prd->categoryRuleInfo()), v, catNum);
  }

  void setPaxTypeFareRuleData(PaxTypeFare& ptf, uint16_t catNum)
  {
    PaxTypeFareRuleData* prd(_memH(new PaxTypeFareRuleData));

    // It seem _memH is taking care of heap on vector.
    CategoryRuleItemInfoSet* criiSet = new CategoryRuleItemInfoSet;
    criiSet->push_back(CategoryRuleItemInfo());

    GeneralFareRuleInfo* ruleInfo = _memH(new GeneralFareRuleInfo);
    ruleInfo->addItemInfoSetNosync(criiSet);

    prd->categoryRuleInfo() = ruleInfo;
    prd->categoryRuleItemInfo() = _memH(new CategoryRuleItemInfo);
    ptf.setRuleData(catNum, _trx->dataHandle(), prd);
  }

  // CategoryRuleInfo -> CategoryRuleItemInfoSet -> CategoryRuleItemInfo
  // CategoryRuleInfo -> vector<CategoryRuleItemInfoSet*>
  // CategoryRuleItemInfoSet -> vector<CategoryRuleItemInfo*>
  /* CategoryRuleItemInfo:
   *  itemcat=15
   *  orderNo=1
   *  relationalInd=1
   *  inOutInd=" "
   *  directionality=" "
   *  itemNo=1346120
   */
  void setCategoryRuleItemInfoSet(PaxTypeFare& ptf,
                                  uint16_t catNum,
                                  uint32_t itemNo,
                                  uint32_t relationalInd = 2,
                                  uint32_t baseItemNo = 12345)
  {
    PaxTypeFareRuleData* prd = ptf.paxTypeFareRuleData(catNum);
    if (!prd)
    {
      setPaxTypeFareRuleData(ptf, catNum);
      prd = ptf.paxTypeFareRuleData(catNum);
    }
    const_cast<CategoryRuleItemInfo*>(prd->categoryRuleItemInfo())->setItemNo(baseItemNo);

    CategoryRuleInfo* ruleInfo = const_cast<CategoryRuleInfo*>(prd->categoryRuleInfo());
    if (ruleInfo)
    {
      tools::non_const(ruleInfo->categoryRuleItemInfoSet()).clear();
      CategoryRuleItemInfo crii;
      crii.setItemcat(catNum);
      crii.setItemNo(itemNo);
      crii.setRelationalInd(static_cast<CategoryRuleItemInfo::LogicalOperators>(relationalInd));
      CategoryRuleItemInfoSet* criiSet = new CategoryRuleItemInfoSet;
      criiSet->push_back(crii);
      ruleInfo->addItemInfoSetNosync(criiSet);
    }
  }

  void testCheckRec1Indicator_Different_DowType()
  {
    setRecord1Indicator(_paxTypeFare1, _paxTypeFare2, 'X', 'W', RuleConst::DAY_TIME_RULE);
    CPPUNIT_ASSERT(RoundTripCheck::DIFFERENT_IND ==
                   RoundTripCheck::checkRec1Indicator(
                       *_paxTypeFare1, *_paxTypeFare2, RuleConst::DAY_TIME_RULE));
  }

  void testCheckRec2Indicator_Different_DowType()
  {
    setRecord2Indicator(*_paxTypeFare1, 'X', RuleConst::DAY_TIME_RULE);
    setRecord2Indicator(*_paxTypeFare2, 'W', RuleConst::DAY_TIME_RULE);
    CPPUNIT_ASSERT(RoundTripCheck::DIFFERENT_IND ==
                   RoundTripCheck::checkRec2Indicator(
                       *_paxTypeFare1, *_paxTypeFare2, RuleConst::DAY_TIME_RULE));
  }

  void testCheckRec1Indicator_Same_DowType()
  {
    setRecord1Indicator(_paxTypeFare1, _paxTypeFare2, 'X', 'X', RuleConst::DAY_TIME_RULE);
    CPPUNIT_ASSERT(RoundTripCheck::SAME_IND ==
                   RoundTripCheck::checkRec1Indicator(
                       *_paxTypeFare1, *_paxTypeFare2, RuleConst::DAY_TIME_RULE));
  }

  void testCheckRec2Indicator_Same_DowType()
  {
    setRecord2Indicator(*_paxTypeFare1, 'X', RuleConst::DAY_TIME_RULE);
    setRecord2Indicator(*_paxTypeFare2, 'X', RuleConst::DAY_TIME_RULE);
    CPPUNIT_ASSERT(RoundTripCheck::SAME_IND ==
                   RoundTripCheck::checkRec2Indicator(
                       *_paxTypeFare1, *_paxTypeFare2, RuleConst::DAY_TIME_RULE));
  }

  void testCheckRec1Indicator_Blank_DowType()
  {
    setRecord1Indicator(_paxTypeFare1, _paxTypeFare2, ' ', ' ', RuleConst::DAY_TIME_RULE);
    CPPUNIT_ASSERT(RoundTripCheck::BLANK_IND ==
                   RoundTripCheck::checkRec1Indicator(
                       *_paxTypeFare1, *_paxTypeFare2, RuleConst::DAY_TIME_RULE));
  }

  void testCheckRec2Indicator_Blank_DowType()
  {
    setRecord2Indicator(*_paxTypeFare1, ' ', RuleConst::DAY_TIME_RULE);
    setRecord2Indicator(*_paxTypeFare2, ' ', RuleConst::DAY_TIME_RULE);
    CPPUNIT_ASSERT(RoundTripCheck::BLANK_IND ==
                   RoundTripCheck::checkRec2Indicator(
                       *_paxTypeFare1, *_paxTypeFare2, RuleConst::DAY_TIME_RULE));
  }

  void testCheckRec1Indicator_Different_SeasonType()
  {
    setRecord1Indicator(_paxTypeFare1, _paxTypeFare2, 'H', 'K', RuleConst::SEASONAL_RULE);
    CPPUNIT_ASSERT(RoundTripCheck::DIFFERENT_IND ==
                   RoundTripCheck::checkRec1Indicator(
                       *_paxTypeFare1, *_paxTypeFare2, RuleConst::SEASONAL_RULE));
  }

  void testCheckRec2Indicator_Different_SeasonType()
  {
    setRecord2Indicator(*_paxTypeFare1, 'H', RuleConst::SEASONAL_RULE);
    setRecord2Indicator(*_paxTypeFare2, 'K', RuleConst::SEASONAL_RULE);
    CPPUNIT_ASSERT(RoundTripCheck::DIFFERENT_IND ==
                   RoundTripCheck::checkRec2Indicator(
                       *_paxTypeFare1, *_paxTypeFare2, RuleConst::SEASONAL_RULE));
  }

  void testCheckRec1Indicator_Same_SeasonType()
  {
    setRecord1Indicator(_paxTypeFare1, _paxTypeFare2, 'H', 'H', RuleConst::SEASONAL_RULE);
    CPPUNIT_ASSERT(RoundTripCheck::SAME_IND ==
                   RoundTripCheck::checkRec1Indicator(
                       *_paxTypeFare1, *_paxTypeFare2, RuleConst::SEASONAL_RULE));
  }

  void testCheckRec2Indicator_Same_SeasonType()
  {
    setRecord2Indicator(*_paxTypeFare1, 'H', RuleConst::SEASONAL_RULE);
    setRecord2Indicator(*_paxTypeFare2, 'H', RuleConst::SEASONAL_RULE);
    CPPUNIT_ASSERT(RoundTripCheck::SAME_IND ==
                   RoundTripCheck::checkRec2Indicator(
                       *_paxTypeFare1, *_paxTypeFare2, RuleConst::SEASONAL_RULE));
  }

  void testCheckRec1Indicator_Blank_SeasonType()
  {
    setRecord1Indicator(_paxTypeFare1, _paxTypeFare2, ' ', ' ', RuleConst::SEASONAL_RULE);
    CPPUNIT_ASSERT(RoundTripCheck::BLANK_IND ==
                   RoundTripCheck::checkRec1Indicator(
                       *_paxTypeFare1, *_paxTypeFare2, RuleConst::SEASONAL_RULE));
  }

  void testCheckRec2Indicator_Blank_SeasonType()
  {
    setRecord2Indicator(*_paxTypeFare1, ' ', RuleConst::SEASONAL_RULE);
    setRecord2Indicator(*_paxTypeFare2, ' ', RuleConst::SEASONAL_RULE);
    CPPUNIT_ASSERT(RoundTripCheck::BLANK_IND ==
                   RoundTripCheck::checkRec2Indicator(
                       *_paxTypeFare1, *_paxTypeFare2, RuleConst::SEASONAL_RULE));
  }

  void testCheckRec3Indicator_Cat2_Different_ItemNo()
  {
    setCategoryRuleItemInfoSet(*_paxTypeFare1, RuleConst::DAY_TIME_RULE, 116221);
    setCategoryRuleItemInfoSet(*_paxTypeFare2, RuleConst::DAY_TIME_RULE, 226221);

    CPPUNIT_ASSERT(
        RoundTripCheck::DIFFERENT_IND ==
        RoundTripCheck::checkRec3ItemNo(*_paxTypeFare1, *_paxTypeFare2, RuleConst::DAY_TIME_RULE));
  }

  void testCheckRec3Indicator_Cat2_Same_ItemNo()
  {
    setCategoryRuleItemInfoSet(*_paxTypeFare1, RuleConst::DAY_TIME_RULE, 116221);
    setCategoryRuleItemInfoSet(*_paxTypeFare2, RuleConst::DAY_TIME_RULE, 116221);
    CPPUNIT_ASSERT(
        RoundTripCheck::SAME_IND ==
        RoundTripCheck::checkRec3ItemNo(*_paxTypeFare1, *_paxTypeFare2, RuleConst::DAY_TIME_RULE));
  }

  void testCheckRec3Indicator_Cat2_Different_Base_ItemNo()
  {
    setCategoryRuleItemInfoSet(*_paxTypeFare1, RuleConst::DAY_TIME_RULE, 116221, 2, 222222);
    setCategoryRuleItemInfoSet(*_paxTypeFare2, RuleConst::DAY_TIME_RULE, 116221, 2, 111111);
    CPPUNIT_ASSERT(
        RoundTripCheck::DIFFERENT_IND ==
        RoundTripCheck::checkRec3ItemNo(*_paxTypeFare1, *_paxTypeFare2, RuleConst::DAY_TIME_RULE));
  }

  void testCheckRec3Indicator_Cat2_Different_RelationalInd()
  {
    setCategoryRuleItemInfoSet(*_paxTypeFare1, RuleConst::DAY_TIME_RULE, 116221, 2);
    setCategoryRuleItemInfoSet(*_paxTypeFare2, RuleConst::DAY_TIME_RULE, 116221, 1);
    CPPUNIT_ASSERT(
        RoundTripCheck::DIFFERENT_IND ==
        RoundTripCheck::checkRec3ItemNo(*_paxTypeFare1, *_paxTypeFare2, RuleConst::DAY_TIME_RULE));
  }

  void testCheckRec3Indicator_Cat3_Different_RelationalInd()
  {
    setCategoryRuleItemInfoSet(*_paxTypeFare1, RuleConst::SEASONAL_RULE, 116221, 2);
    setCategoryRuleItemInfoSet(*_paxTypeFare2, RuleConst::SEASONAL_RULE, 116221, 1);
    CPPUNIT_ASSERT(
        RoundTripCheck::DIFFERENT_IND ==
        RoundTripCheck::checkRec3ItemNo(*_paxTypeFare1, *_paxTypeFare2, RuleConst::SEASONAL_RULE));
  }

  void testCheckRec2IndicatorForGfr_Different_DowType()
  {
    uint16_t catNum = RuleConst::DAY_TIME_RULE;
    setRecord2Indicator(*_paxTypeFare1, 'X', catNum);
    setRecord2Indicator(*_paxTypeFare2, 'W', catNum);

    const GeneralFareRuleInfo* gfrInfo1 = 0;
    const GeneralFareRuleInfo* gfrInfo2 = 0;

    PaxTypeFareRuleData* prd1 = _paxTypeFare1->paxTypeFareRuleData(catNum);
    if (prd1)
      gfrInfo1 = dynamic_cast<const GeneralFareRuleInfo*>(prd1->categoryRuleInfo());

    PaxTypeFareRuleData* prd2 = _paxTypeFare2->paxTypeFareRuleData(catNum);
    if (prd2)
      gfrInfo2 = dynamic_cast<const GeneralFareRuleInfo*>(prd2->categoryRuleInfo());

    CPPUNIT_ASSERT(RoundTripCheck::DIFFERENT_IND ==
                   RoundTripCheck::checkRec2Indicator(gfrInfo1, gfrInfo2, catNum));
  }

  void testCheckRec2IndicatorForGfr_Same_ItemInfo()
  {
    uint16_t catNum = RuleConst::SEASONAL_RULE;
    setCategoryRuleItemInfoSet(*_paxTypeFare1, catNum, 116221, 1);
    setCategoryRuleItemInfoSet(*_paxTypeFare2, catNum, 116221, 1);

    const GeneralFareRuleInfo* gfrInfo1 = 0;
    const GeneralFareRuleInfo* gfrInfo2 = 0;

    PaxTypeFareRuleData* prd1 = _paxTypeFare1->paxTypeFareRuleData(catNum);
    if (prd1)
      gfrInfo1 = dynamic_cast<const GeneralFareRuleInfo*>(prd1->categoryRuleInfo());

    PaxTypeFareRuleData* prd2 = _paxTypeFare2->paxTypeFareRuleData(catNum);
    if (prd2)
      gfrInfo2 = dynamic_cast<const GeneralFareRuleInfo*>(prd2->categoryRuleInfo());

    CPPUNIT_ASSERT(RoundTripCheck::SAME_IND ==
                   RoundTripCheck::checkRec2Indicator(gfrInfo1, gfrInfo2, catNum));
  }

  void testCheckRec2IndicatorForGfr_Different_RelationalInd()
  {
    uint16_t catNum = RuleConst::SEASONAL_RULE;
    setCategoryRuleItemInfoSet(*_paxTypeFare1, catNum, 116221, 2);
    setCategoryRuleItemInfoSet(*_paxTypeFare2, catNum, 116221, 1);

    const GeneralFareRuleInfo* gfrInfo1 = 0;
    const GeneralFareRuleInfo* gfrInfo2 = 0;

    PaxTypeFareRuleData* prd1 = _paxTypeFare1->paxTypeFareRuleData(catNum);
    if (prd1)
      gfrInfo1 = dynamic_cast<const GeneralFareRuleInfo*>(prd1->categoryRuleInfo());

    PaxTypeFareRuleData* prd2 = _paxTypeFare2->paxTypeFareRuleData(catNum);
    if (prd2)
      gfrInfo2 = dynamic_cast<const GeneralFareRuleInfo*>(prd2->categoryRuleInfo());

    CPPUNIT_ASSERT(RoundTripCheck::DIFFERENT_IND ==
                   RoundTripCheck::checkRec2Indicator(gfrInfo1, gfrInfo2, catNum));
  }

  void testCheckRec2IndicatorForGfr_Different_ItemNo()
  {
    uint16_t catNum = RuleConst::SEASONAL_RULE;
    setCategoryRuleItemInfoSet(*_paxTypeFare1, catNum, 116221, 1);
    setCategoryRuleItemInfoSet(*_paxTypeFare2, catNum, 116222, 1);

    const GeneralFareRuleInfo* gfrInfo1 = 0;
    const GeneralFareRuleInfo* gfrInfo2 = 0;

    PaxTypeFareRuleData* prd1 = _paxTypeFare1->paxTypeFareRuleData(catNum);
    if (prd1)
      gfrInfo1 = dynamic_cast<const GeneralFareRuleInfo*>(prd1->categoryRuleInfo());

    PaxTypeFareRuleData* prd2 = _paxTypeFare2->paxTypeFareRuleData(catNum);
    if (prd2)
      gfrInfo2 = dynamic_cast<const GeneralFareRuleInfo*>(prd2->categoryRuleInfo());

    CPPUNIT_ASSERT(RoundTripCheck::DIFFERENT_IND ==
                   RoundTripCheck::checkRec2Indicator(gfrInfo1, gfrInfo2, catNum));
  }

  void testIsPricingUnitValid_Fail_PULevelCombinability()
  {
    PricingUnitFactory* mock = setupIsPricingUnitValid(false);

    CPPUNIT_ASSERT(!mock->isPricingUnitValid(*createPUPQItem(), false, *createDiag605()));
  }

  PaxTypeFare* createFare(CurrencyCode curr, Directionality dir)
  {
    FareInfo* fi = _memH.create<FareInfo>();
    fi->currency() = curr;
    Fare* f = _memH.create<Fare>();
    f->setFareInfo(fi);
    PaxTypeFare* ptf = _memH.create<PaxTypeFare>();
    ptf->setFare(f);
    return ptf;
  }

  void testCheckNonDirectionalFare_MixedDirFareMarket_OneCurency_Pass()
  {
    PaxTypeBucket ptc;
    ptc.paxTypeFare() += createFare("USD", FROM), createFare("USD", BOTH);
    // ptc.setMarketCurrencyPresent(_factory->_trx, *_fareMarket1);

    // CPPUNIT_ASSERT( _factory->checkNonDirectionalFare(*ptc.paxTypeFare()[1], ptc, "USD") );
  }

  void testCheckNonDirectionalFare_MixedDirFareMarket_OneCurency_Fail()
  {
    PaxTypeBucket ptc;
    ptc.paxTypeFare() += createFare("USD", FROM), createFare("USD", BOTH);
    // ptc.setMarketCurrencyPresent(_factory->_trx, *_fareMarket1);

    // CPPUNIT_ASSERT( !_factory->checkNonDirectionalFare(*ptc.paxTypeFare()[1], ptc, "CAD") );
  }

  void testCheckNonDirectionalFare_MixedDirFareMarket_MixedCurency()
  {
    PaxTypeBucket ptc;
    ptc.paxTypeFare() += createFare("CAD", FROM), createFare("USD", BOTH);
    // ptc.setMarketCurrencyPresent(_factory->_trx, *_fareMarket1);

    // CPPUNIT_ASSERT( _factory->checkNonDirectionalFare(*ptc.paxTypeFare()[1], ptc, "USD") );
  }

  void testCheckNonDirectionalFare_OneDirFareMarket_OneCurrency_Pass()
  {
    PaxTypeBucket ptc;
    ptc.paxTypeFare() += createFare("USD", BOTH), createFare("USD", BOTH);
    // ptc.setMarketCurrencyPresent(_factory->_trx, *_fareMarket1);

    // CPPUNIT_ASSERT( _factory->checkNonDirectionalFare(*ptc.paxTypeFare()[0], ptc, "USD") );
    // CPPUNIT_ASSERT( _factory->checkNonDirectionalFare(*ptc.paxTypeFare()[1], ptc, "USD") );
  }

  void testCheckNonDirectionalFare_OneDirFareMarket_OneCurrency_Fail()
  {
    PaxTypeBucket ptc;
    ptc.paxTypeFare() += createFare("USD", BOTH), createFare("USD", BOTH);
    // ptc.setMarketCurrencyPresent(_factory->_trx, *_fareMarket1);

    // CPPUNIT_ASSERT( !_factory->checkNonDirectionalFare(*ptc.paxTypeFare()[0], ptc, "CAD") );
    // CPPUNIT_ASSERT( !_factory->checkNonDirectionalFare(*ptc.paxTypeFare()[1], ptc, "CAD") );
  }

  void testCheckNonDirectionalFare_OneDirFareMarket_MixedCurency()
  {
    PaxTypeBucket ptc;
    ptc.paxTypeFare() += createFare("CAD", BOTH), createFare("USD", BOTH);
    // ptc.setMarketCurrencyPresent(_factory->_trx, *_fareMarket1);

    // CPPUNIT_ASSERT( _factory->checkNonDirectionalFare(*ptc.paxTypeFare()[0], ptc, "CAD") );
    // CPPUNIT_ASSERT( !_factory->checkNonDirectionalFare(*ptc.paxTypeFare()[1], ptc, "CAD") );
  }

  void commonDisplayPricingUnit()
  {
    _factory->_itin = createItin();
    _factory->_itin->itinNum() = 1;
    _factory->_paxType->paxType() = "ADT";

    _trx->itin().push_back(_factory->_itin);
    _trx->diagnostic().diagnosticType() = Diagnostic605;
  }

  void testDisplayPricingUnit()
  {
    commonDisplayPricingUnit();

    _factory->displayPricingUnit(*_pricingUnit, *_diag);

    CPPUNIT_ASSERT_EQUAL(std::string(" REQUESTED PAXTYPE: ADT    AMOUNT: 0\n\n"), _diag->str());
  }

  void testDisplayPricingUnitMip()
  {
    commonDisplayPricingUnit();
    _trx->itin().push_back(_factory->_itin);

    _factory->displayPricingUnit(*_pricingUnit, *_diag);

    CPPUNIT_ASSERT_EQUAL(std::string(" REQUESTED PAXTYPE: ADT    AMOUNT: 0\n\n"), _diag->str());
  }

  void testIsValidPUForValidatingCxr_NoValidatingCxr()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    _fareUsage1->paxTypeFare() = _paxTypeFare1;
    _fareUsage2->paxTypeFare() = _paxTypeFare2;
    _fareUsage3->paxTypeFare() = _paxTypeFare3;

    _pricingUnit->fareUsage() += _fareUsage1, _fareUsage2, _fareUsage3;
    CPPUNIT_ASSERT(_pricingUnit->fareUsage().size() == 3);

    _factory->_itin = createItin();
    ValidatingCxrGSAData v;
    ValidatingCxrDataMap vcm;
    vcx::ValidatingCxrData vcd;

    vcm["AA"] = vcd;
    vcm["EK"] = vcd;
    vcm["BA"] = vcd;
    v.validatingCarriersData() = vcm;
    _factory->_itin->validatingCxrGsaData() = &v;

    // Validation should return true if PaxTypeFares have empty Validation Carrier vector
    CPPUNIT_ASSERT(_factory->isValidPUForValidatingCxr(*_pricingUnit, *_diag) == true);
  }

  void testIsValidPUForValidatingCxr_NoValidatingCxrForMultiSp()
  {
    _fareUsage1->paxTypeFare() = _paxTypeFare1;
    _fareUsage2->paxTypeFare() = _paxTypeFare2;
    _fareUsage3->paxTypeFare() = _paxTypeFare3;

    _pricingUnit->fareUsage() += _fareUsage1, _fareUsage2, _fareUsage3;
    CPPUNIT_ASSERT(_pricingUnit->fareUsage().size() == 3);

    _factory->_itin = createItin();
    ValidatingCxrGSAData v;
    ValidatingCxrDataMap vcm;
    vcx::ValidatingCxrData vcd;

    vcm["AA"] = vcd;
    vcm["EK"] = vcd;
    vcm["BA"] = vcd;
    v.validatingCarriersData() = vcm;

    SpValidatingCxrGSADataMap spGsaDataMap;
    spGsaDataMap["BSP"] = &v;

    _factory->_itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

    // Validation should return true if PaxTypeFares have empty Validation Carrier vector
    CPPUNIT_ASSERT(_factory->isValidPUForValidatingCxr(*_pricingUnit, *_diag) == true);
  }

  void testIsValidPUForValidatingCxr_ValidCxrList()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    CarrierCode set1[] = { "AA", "AB", "BA" };
    CarrierCode set2[] = { "AB", "AA", "AF" };
    CarrierCode set3[] = { "TM", "AA", "AF", "DL" };

    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 3);
    std::vector<CarrierCode> list2;
    list2.insert(list2.begin(), set2, set2 + 3);
    std::vector<CarrierCode> list3;
    list3.insert(list3.begin(), set3, set3 + 4);

    _paxTypeFare1->validatingCarriers() = list1;
    _paxTypeFare2->validatingCarriers() = list2;
    _paxTypeFare3->validatingCarriers() = list3;

    _fareUsage1->paxTypeFare() = _paxTypeFare1;
    _fareUsage2->paxTypeFare() = _paxTypeFare2;
    _fareUsage3->paxTypeFare() = _paxTypeFare3;

    _pricingUnit->fareUsage() += _fareUsage1, _fareUsage2, _fareUsage3;
    CPPUNIT_ASSERT(_pricingUnit->fareUsage().size() == 3);

    _factory->_itin = createItin();
    ValidatingCxrGSAData v;
    ValidatingCxrDataMap vcm;
    vcx::ValidatingCxrData vcd;

    vcm["AA"] = vcd;
    vcm["AF"] = vcd;
    vcm["BA"] = vcd;
    v.validatingCarriersData() = vcm;
    _factory->_itin->validatingCxrGsaData() = &v;

    // Validation should return true if PaxTypeFares have empty Validation Carrier vector
    CPPUNIT_ASSERT(_factory->isValidPUForValidatingCxr(*_pricingUnit, *_diag) == true);

    // We should have one Validating Cxr for this PU: AA
    CPPUNIT_ASSERT(_pricingUnit->validatingCarriers().size() == 1);
  }

  void testIsValidPUForValidatingCxr_ValidCxrListMultiSp()
  {
    CarrierCode set1[] = { "AA", "AB", "BA" };
    CarrierCode set2[] = { "AB", "AA", "AF" };
    CarrierCode set3[] = { "TM", "AA", "AF", "DL" };

    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 3);
    std::vector<CarrierCode> list2;
    list2.insert(list2.begin(), set2, set2 + 3);
    std::vector<CarrierCode> list3;
    list3.insert(list3.begin(), set3, set3 + 4);

    _paxTypeFare1->validatingCarriers() = list1;
    _paxTypeFare2->validatingCarriers() = list2;
    _paxTypeFare3->validatingCarriers() = list3;

    _fareUsage1->paxTypeFare() = _paxTypeFare1;
    _fareUsage2->paxTypeFare() = _paxTypeFare2;
    _fareUsage3->paxTypeFare() = _paxTypeFare3;

    _pricingUnit->fareUsage() += _fareUsage1, _fareUsage2, _fareUsage3;
    CPPUNIT_ASSERT(_pricingUnit->fareUsage().size() == 3);

    _factory->_itin = createItin();
    ValidatingCxrGSAData v;
    ValidatingCxrDataMap vcm;
    vcx::ValidatingCxrData vcd;

    vcm["AA"] = vcd;
    vcm["AF"] = vcd;
    vcm["BA"] = vcd;
    v.validatingCarriersData() = vcm;

    SpValidatingCxrGSADataMap spGsaDataMap;
    spGsaDataMap["BSP"] = &v;
    _factory->_itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

    // Validation should return true if PaxTypeFares have empty Validation Carrier vector
    CPPUNIT_ASSERT(_factory->isValidPUForValidatingCxr(*_pricingUnit, *_diag) == true);

    // We should have one Validating Cxr for this PU: AA
    CPPUNIT_ASSERT(_pricingUnit->validatingCarriers().size() == 1);
  }

  void testIsValidPUForValidatingCxr_Failed()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    CarrierCode set1[] = { "AA", "AB", "BA" };
    CarrierCode set2[] = { "AB", "AA", "AF" };
    CarrierCode set3[] = { "TM", "EK", "AF", "DL" };

    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 3);
    std::vector<CarrierCode> list2;
    list2.insert(list2.begin(), set2, set2 + 3);
    std::vector<CarrierCode> list3;
    list3.insert(list3.begin(), set3, set3 + 4);

    _paxTypeFare1->validatingCarriers() = list1;
    _paxTypeFare2->validatingCarriers() = list2;
    _paxTypeFare3->validatingCarriers() = list3;

    _fareUsage1->paxTypeFare() = _paxTypeFare1;
    _fareUsage2->paxTypeFare() = _paxTypeFare2;
    _fareUsage3->paxTypeFare() = _paxTypeFare3;

    _pricingUnit->fareUsage() += _fareUsage1, _fareUsage2, _fareUsage3;
    CPPUNIT_ASSERT(_pricingUnit->fareUsage().size() == 3);

    _factory->_itin = createItin();
    ValidatingCxrGSAData v;
    ValidatingCxrDataMap vcm;
    vcx::ValidatingCxrData vcd;

    vcm["AA"] = vcd;
    vcm["EK"] = vcd;
    vcm["BA"] = vcd;
    v.validatingCarriersData() = vcm;
    _factory->_itin->validatingCxrGsaData() = &v;

    // Validation should return true if PaxTypeFares have empty Validation Carrier vector
    CPPUNIT_ASSERT(_factory->isValidPUForValidatingCxr(*_pricingUnit, *_diag) == false);
    // Make sure PU level list is empty
    CPPUNIT_ASSERT(_pricingUnit->validatingCarriers().size() == 0);
  }

  void testIsValidPUForValidatingCxr_FailedMultiSp()
  {
    CarrierCode set1[] = { "AA", "AB", "BA" };
    CarrierCode set2[] = { "AB", "AA", "AF" };
    CarrierCode set3[] = { "TM", "EK", "AF", "DL" };

    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 3);
    std::vector<CarrierCode> list2;
    list2.insert(list2.begin(), set2, set2 + 3);
    std::vector<CarrierCode> list3;
    list3.insert(list3.begin(), set3, set3 + 4);

    _paxTypeFare1->validatingCarriers() = list1;
    _paxTypeFare2->validatingCarriers() = list2;
    _paxTypeFare3->validatingCarriers() = list3;

    _fareUsage1->paxTypeFare() = _paxTypeFare1;
    _fareUsage2->paxTypeFare() = _paxTypeFare2;
    _fareUsage3->paxTypeFare() = _paxTypeFare3;

    _pricingUnit->fareUsage() += _fareUsage1, _fareUsage2, _fareUsage3;
    CPPUNIT_ASSERT(_pricingUnit->fareUsage().size() == 3);

    _factory->_itin = createItin();
    ValidatingCxrGSAData v;
    ValidatingCxrDataMap vcm;
    vcx::ValidatingCxrData vcd;

    vcm["AA"] = vcd;
    vcm["EK"] = vcd;
    vcm["BA"] = vcd;
    v.validatingCarriersData() = vcm;

    SpValidatingCxrGSADataMap spGsaDataMap;
    spGsaDataMap["BSP"] = &v;
    _factory->_itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

    // Validation should return true if PaxTypeFares have empty Validation Carrier vector
    CPPUNIT_ASSERT(_factory->isValidPUForValidatingCxr(*_pricingUnit, *_diag) == false);
    // Make sure PU level list is empty
    CPPUNIT_ASSERT(_pricingUnit->validatingCarriers().size() == 0);
  }

  void testCheckSOJFareRestrictionNLNL()
  {
    PaxTypeFare& pf1 = createPaxTypeFare();
    PaxTypeFare& pf2 = createPaxTypeFare();
    FareUsage fu1, fu2;
    PricingUnit pu;

    fu1.paxTypeFare() = &pf1;
    fu2.paxTypeFare() = &pf2;
    pf1.fareTypeApplication() = 'N'; // PaxTypeFare::PRICING_CATTYPE_NORMAL;
    pf2.fareTypeApplication() = 'N'; // PaxTypeFare::PRICING_CATTYPE_NORMAL;

    pu.fareUsage() += &fu1, &fu2;

    CPPUNIT_ASSERT_EQUAL(false, _factory->checkSOJFareRestriction(pu));
  }

  void testCheckSOJFareRestrictionSPNL()
  {
    PaxTypeFare& pf1 = createPaxTypeFare();
    PaxTypeFare& pf2 = createPaxTypeFare();
    FareUsage fu1, fu2;
    PricingUnit pu;

    fu1.paxTypeFare() = &pf1;
    fu2.paxTypeFare() = &pf2;
    pf1.fareTypeApplication() = 'S'; // PaxTypeFare::PRICING_CATTYPE_SPECIAL;
    pf2.fareTypeApplication() = 'N'; // PaxTypeFare::PRICING_CATTYPE_NORMAL;

    pu.fareUsage() += &fu1, &fu2;

    CPPUNIT_ASSERT_EQUAL(true, _factory->checkSOJFareRestriction(pu));
  }

  void testCheckSOJFareRestrictionNLSP()
  {
    PaxTypeFare& pf1 = createPaxTypeFare();
    PaxTypeFare& pf2 = createPaxTypeFare();
    FareUsage fu1, fu2;
    PricingUnit pu;

    fu1.paxTypeFare() = &pf1;
    fu2.paxTypeFare() = &pf2;
    pf1.fareTypeApplication() = 'N'; // PaxTypeFare::PRICING_CATTYPE_NORMAL;
    pf2.fareTypeApplication() = 'S'; // PaxTypeFare::PRICING_CATTYPE_SPECIAL;

    pu.fareUsage() += &fu1, &fu2;

    CPPUNIT_ASSERT_EQUAL(true, _factory->checkSOJFareRestriction(pu));
  }

  void testCheckSOJFareRestrictionSPSP()
  {
    PaxTypeFare& pf1 = createPaxTypeFare();
    PaxTypeFare& pf2 = createPaxTypeFare();
    FareUsage fu1, fu2;
    PricingUnit pu;

    fu1.paxTypeFare() = &pf1;
    fu2.paxTypeFare() = &pf2;
    pf1.fareTypeApplication() = 'S'; // PaxTypeFare::PRICING_CATTYPE_SPECIAL;
    pf2.fareTypeApplication() = 'S'; // PaxTypeFare::PRICING_CATTYPE_SPECIAL;

    pu.fareUsage() += &fu1, &fu2;

    CPPUNIT_ASSERT_EQUAL(true, _factory->checkSOJFareRestriction(pu));
  }

  void testIsFareValidForTraditionalVC_singleTraditionalVC_PASS()
  {
    CarrierCode set1[] = { "AA", "AB", "BA" };
    std::set<CarrierCode> vcs;
    vcs.insert("BA");

    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 3);

    _paxTypeFare1->validatingCarriers() = list1;

    // Validation should return true if PaxTypeFares have empty Validation Carrier vector
    CPPUNIT_ASSERT(_factory->isFareValidForTraditionalVC(*_paxTypeFare1, vcs));
  }

  void testIsFareValidForTraditionalVC_singleTraditionalVC_FAIL()
  {
    CarrierCode set1[] = { "AA", "AB", "BA" };
    std::set<CarrierCode> vcs;
    vcs.insert("BB");

    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 3);

    _paxTypeFare1->validatingCarriers() = list1;

    // Validation should return true if PaxTypeFares have empty Validation Carrier vector
    CPPUNIT_ASSERT(!_factory->isFareValidForTraditionalVC(*_paxTypeFare1, vcs));
  }

  void testIsFareValidForTraditionalVC_multiTraditionalVC_FAIL()
  {
    CarrierCode set1[] = { "AA", "AB", "BA" };
    CarrierCode set2[] = { "HK", "LA", "JJ" };
    std::set<CarrierCode> vcs;

    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 3);
    vcs.insert(set2, set2 + 3);

    _paxTypeFare1->validatingCarriers() = list1;

    // Validation should return true if PaxTypeFares have empty Validation Carrier vector
    CPPUNIT_ASSERT(!_factory->isFareValidForTraditionalVC(*_paxTypeFare1, vcs));
  }

  void testIsFareValidForTraditionalVC_multiTraditionalVC_PASS()
  {
    CarrierCode set1[] = { "AA", "AB", "BA" };
    CarrierCode set2[] = { "AB", "LA", "JJ" };
    std::set<CarrierCode> vcs;

    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 3);
    vcs.insert(set2, set2 + 3);

    _paxTypeFare1->validatingCarriers() = list1;

    // Validation should return true if PaxTypeFares have empty Validation Carrier vector
    CPPUNIT_ASSERT(_factory->isFareValidForTraditionalVC(*_paxTypeFare1, vcs));
  }

  class MockPricingUnitFactory : public PricingUnitFactory
  {
  public:
    MockPricingUnitFactory()
      : _usePrevFareUsage(true),
        _buildFareUsage(false),
        _isPricingUnitValid(false),
        _mktIdx(0),
        _fareIdx(0),
        _fareFound(false),
        _fareType(""),
        _usePrevFareUsageExe(false),
        _buildFareUsageExe(false),
        _fmMiles(0),
        _lcMiles(0),
        _getFmMileageSuccess(false),
        _getLcMileageSuccess(false),
        _checkPULevelCombinability(false),
        _performPULevelRuleValidation(false),
        _getNextCxrFarePricinUnitMock(true),
        _isPricingUnitValidMock(true)
    {
      _pu = &pu;
    }

    void setReturnValues(bool usePrevFareUsage, bool buildFareUsage)
    {
      _usePrevFareUsage = usePrevFareUsage;
      _buildFareUsage = buildFareUsage;
    }

    void setReturnValues(bool isPricingUnitValid) { _isPricingUnitValid = isPricingUnitValid; }

    void setNextCxrFarePricinUnitMock(bool mock) { _getNextCxrFarePricinUnitMock = mock; }

    void setIsPricingUnitValidMock(bool mock) { _isPricingUnitValidMock = mock; }

    void setMileageReturnValues(uint32_t fmMiles,
                                bool getFmMileageSuccess,
                                uint32_t lcMiles = 0,
                                bool getLcMileageSuccess = false)
    {
      _fmMiles = fmMiles;
      _lcMiles = lcMiles;
      _getFmMileageSuccess = getFmMileageSuccess;
      _getLcMileageSuccess = getLcMileageSuccess;
    }

    void setCheckPULevelCombinabilityReturnValue(bool flag) { _checkPULevelCombinability = flag; }

    void setPerformPULevelRuleValidationReturnValue(bool flag)
    {
      _performPULevelRuleValidation = flag;
    }

    void setExpectArgsForBuildFareUsage(uint16_t mktIdx,
                                        uint16_t fareIdx,
                                        bool fareFound,
                                        const FareType& fareType)
    {
      _mktIdx = mktIdx;
      _fareIdx = fareIdx;
      _fareFound = fareFound;
      _fareType = fareType;
    }

    bool usePrevFareUsageExecuted() { return _usePrevFareUsageExe; }
    bool buildFareUsageExecuted() { return _buildFareUsageExe; }

  protected:
    virtual bool isSamePointApplicable(const uint16_t mktIdx,
                                       const CombinabilityRuleInfo& rec2Cat10,
                                       const DateTime& travelDate) const
    {
      return mktIdx > 99;
    }

    virtual bool buildFareUsage(const PaxTypeFare* primaryPaxTypeFare,
                                const uint16_t mktIdx,
                                uint16_t fareIdx,
                                bool& fareFound,
                                PUPQItem& pupqItem,
                                const PaxTypeFare::SegmentStatusVec* segStatus,
                                const FareType& fareType,
                                DiagCollector& diag,
                                bool fxCnException = false,
                                bool rexCheckSameFareDate = false)
    {
      CPPUNIT_ASSERT_EQUAL(_mktIdx, mktIdx);
      CPPUNIT_ASSERT_EQUAL(_fareIdx, fareIdx);
      CPPUNIT_ASSERT_EQUAL(_fareFound, fareFound);
      CPPUNIT_ASSERT_EQUAL(_fareType, fareType);

      _buildFareUsageExe = true;
      return _buildFareUsage;
    }

    virtual bool usePrevFareUsage(const uint16_t mktIdx,
                                  const PUPQItem& prevPUPQItem,
                                  PUPQItem& pupqItem,
                                  DiagCollector& diag,
				  bool& prevFailedFareUsage,
                                  bool rexCheckSameFareDate = false)
    {
      _usePrevFareUsageExe = true;
      return _usePrevFareUsage;
    }

    virtual bool
    isPricingUnitValid(PUPQItem& pupqItem, const bool allowDelayedValidation, DiagCollector& diag)
    {
      if (_isPricingUnitValidMock)
      {
        return _isPricingUnitValid;
      }
      return PricingUnitFactory::isPricingUnitValid(pupqItem, allowDelayedValidation, diag);
    }

    virtual PUPQItem* getNextCxrFarePricinUnit(PUPQ& cxrFarePUPQ, DiagCollector& diag)
    {
      if (_getNextCxrFarePricinUnitMock)
      {
        cxrFarePUPQ.pop();
        return cxrFarePUPQ.empty() ? 0 : cxrFarePUPQ.top();
      }
      std::deque<bool> cxrFareRest;
      return PricingUnitFactory::getNextCxrFarePricinUnit(cxrFarePUPQ, cxrFareRest, diag);
    }

    virtual bool getMileage(const FareMarket& fm,
                            const DateTime& travelDate,
                            uint32_t& miles,
                            DiagCollector& diag)
    {
      miles = _fmMiles;
      return _getFmMileageSuccess;
    }

    virtual bool getMileage(const LocCode& city1,
                            const LocCode& city2,
                            GlobalDirection& gd,
                            const DateTime& travelDate,
                            uint32_t& miles,
                            DiagCollector& diag)
    {
      miles = _lcMiles;
      return _getLcMileageSuccess;
    }

    bool
    getGlobalDirection(DateTime travelDate, TravelSeg& tvlSeg, GlobalDirection& globalDir) const
        override
    {
      // set universal direction
      globalDir = GlobalDirection::XX;
      return true;
    }

    virtual bool checkPULevelCombinability(PricingUnit& prU, DiagCollector& diag)
    {
      return _checkPULevelCombinability;
    }

    virtual bool performPULevelRuleValidation(PricingUnit& prU, DiagCollector& diag)
    {
      return _performPULevelRuleValidation;
    }

    PU pu;
    bool _usePrevFareUsage;
    bool _buildFareUsage;
    bool _isPricingUnitValid;

    uint16_t _mktIdx;
    uint16_t _fareIdx;
    bool _fareFound;
    FareType _fareType;

    bool _usePrevFareUsageExe;
    bool _buildFareUsageExe;

    uint32_t _fmMiles;
    uint32_t _lcMiles;
    bool _getFmMileageSuccess;
    bool _getLcMileageSuccess;
    bool _checkPULevelCombinability;
    bool _performPULevelRuleValidation;
    bool _getNextCxrFarePricinUnitMock;
    bool _isPricingUnitValidMock;
  };

  MockPricingUnitFactory*
  createMockFactory(PricingUnit::PUSubType puSubType = PricingUnit::DOUBLE_OPENJAW,
                    bool sameNationOJ = pu_sameNationOJ_no,
                    bool sameNationOrigSurfaceOJ = pu_sameNationOrigSurfaceOJ_yes)
  {
    MockPricingUnitFactory* mock = _memH(new MockPricingUnitFactory);
    _factory = mock;
    _factory->pu()->sameNationOJ() = sameNationOJ;
    _factory->pu()->puSubType() = puSubType;
    _factory->pu()->sameNationOrigSurfaceOJ() = sameNationOrigSurfaceOJ;
    _factory->_trx = _memH(new PricingTrx);
    _factory->_trx->setOptions(_memH(new PricingOptions));
    _factory->_trx->setRequest(_memH(new PricingRequest));
    _factory->_trx->getRequest()->ticketingDT() = DateTime::localTime();
    _factory->_combinations = _memH(new Combinations);

    return mock;
  }

  class MyDataHandle : public DataHandleMock
  {
  public:
    const std::vector<GeneralFareRuleInfo*>& getGeneralFareRule(const VendorCode& vendor,
                                                                const CarrierCode& carrier,
                                                                const TariffNumber& ruleTariff,
                                                                const RuleNumber& rule,
                                                                const CatNumber& category,
                                                                const DateTime& date,
                                                                const DateTime& applDate)
    {
      static std::vector<GeneralFareRuleInfo*> ret;
      return ret;
    }
  };
};

const std::string
PricingUnitFactoryTest::_fType("ABC");

CPPUNIT_TEST_SUITE_REGISTRATION(PricingUnitFactoryTest);
}
