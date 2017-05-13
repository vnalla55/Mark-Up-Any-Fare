#include <algorithm>
#include <functional>

#include "DataModel/FareCompInfo.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingTrx.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleController.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class MockRexPricingTrx : public RexPricingTrx
{
  friend class RuleControllerTest;
};

class TestRuleController : public RuleController
{
  /* virtual */
  Record3ReturnTypes revalidateC15BaseFareForDisc(uint16_t category,
                                                  bool& checkFare,
                                                  PaxTypeFare* ptf,
                                                  RuleControllerDataAccess& da)
  {
    return NOTPROCESSED;
  }

  /* virtual */
  Record3ReturnTypes doCategoryPostProcessing(PricingTrx& trx,
                                              RuleControllerDataAccess& da,
                                              const uint16_t category,
                                              RuleProcessingData& rpData,
                                              const Record3ReturnTypes preResult)
  {
    return NOTPROCESSED;
  }

  /* virtual */
  Record3ReturnTypes validateBaseFare(uint16_t category,
                                      const FareByRuleItemInfo* fbrItemInfo,
                                      bool& checkFare,
                                      PaxTypeFare* fbrBaseFare,
                                      RuleControllerDataAccess& da)
  {
    return NOTPROCESSED;
  }

  /* virtual */
  Record3ReturnTypes callCategoryRuleItemSet(CategoryRuleItemSet& catRuleIS,
                                             const CategoryRuleInfo&,
                                             const std::vector<CategoryRuleItemInfoSet*>&,
                                             RuleControllerDataAccess& da,
                                             RuleProcessingData& rpData,
                                             bool isLocationSwapped,
                                             bool isFareRule,
                                             bool skipCat15Security)
  {
    return NOTPROCESSED;
  }

  /* virtual */
  void applySurchargeGenRuleForFMS(PricingTrx& trx,
                                   RuleControllerDataAccess& da,
                                   uint16_t categoryNumber,
                                   RuleControllerParam& rcParam,
                                   bool skipCat15Security)
  {
  }

  /* virtual */
  
public:
  bool
  skipCategoryProcessing(uint16_t category, const PaxTypeFare& paxTypeFare, const PricingTrx& trx)
      const
  {
    return RuleController::skipCategoryProcessing(category, paxTypeFare, trx);
  }

  void storeResultsFr(PricingTrx& trx,
                      PaxTypeFare& paxTypeFare,
                      uint16_t categoryNumber,
                      const FareMarket::FareMarketSavedGfrResult::Result* savedRet,
                      RuleControllerParam& rcParam)
  {
     RuleController::storeResultsFr(trx, paxTypeFare, categoryNumber, savedRet, rcParam);
   }

  void storeSalesRestrictionValuesGr(PricingTrx& trx,
                                     PaxTypeFare& paxTypeFare,
                                     uint16_t categoryNumber,
                                     const FareMarket::FareMarketSavedGfrResult::Result* savedRet)
  {
    RuleController::storeSalesRestrictionValuesGr(trx, paxTypeFare, categoryNumber, savedRet);
  }
  void storeSalesRestrictionValuesFn(PricingTrx& trx,
                                      PaxTypeFare& paxTypeFare,
                                      uint16_t categoryNumber,
                                      const FareMarket::FareMarketSavedFnResult::Result* savedRet)
   {
     RuleController::storeSalesRestrictionValuesFn(trx, paxTypeFare, categoryNumber, savedRet);
   }
  bool
  skipCategoryFCORuleValidation(PricingTrx& trx, uint16_t category, PaxTypeFare& paxTypeFare) const
  {
    return RuleController::skipCategoryFCORuleValidation(trx, category, paxTypeFare);
  }

  bool skipRuleDisplayValidation(uint16_t category, const PaxTypeFare& paxTypeFare) const
  {
    return RuleController::skipRuleDisplayValidation(category, paxTypeFare);
  }

  bool skipCalculatedFBR(const PaxTypeFare& paxTypeFare, bool fbrCalcFare, uint16_t category) const
  {
    return RuleController::skipCalculatedFBR(paxTypeFare, fbrCalcFare, category);
  }

  bool skipCat15ForSellingNetFare(Indicator fcaDisplayCatType) const
  {
    return RuleController::skipCat15ForSellingNetFare(fcaDisplayCatType);
  }

  void updateBaseFareRuleStatDiscounted(PaxTypeFare* ptf, PricingTrx& trx, uint16_t category) const
  {
    RuleController::updateBaseFareRuleStatDiscounted(*ptf, trx, category);
  }

  CategoryPhase& categoryFase() { return _categoryPhase; }

  // use the most bare ctor possible
  TestRuleController() : RuleController(NormalValidation, std::vector<uint16_t>()) {}
};

class FakeRuleControllerDataAccess : public RuleControllerDataAccess
{
  friend class RuleControllerTest;

private:
  FakeRuleControllerDataAccess() {}

public:
  FakeRuleControllerDataAccess(PaxTypeFare* ptf, PricingTrx* trx) : _ptf(ptf), _trx(trx)
  {
    _retrieveBaseFare = false;
  }

  Itin* itin() { return &_itin; }

  PaxTypeFare& paxTypeFare() const { return *_ptf; }

  PricingTrx& trx() { return *_trx; }

  // data members
  Itin _itin;
  PaxTypeFare* _ptf;
  PricingTrx* _trx;
};

class RuleControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RuleControllerTest);

  CPPUNIT_TEST(testSkipRexSecurityCheck_Reissue);
  CPPUNIT_TEST(testSkipRexSecurityCheck_forVCTR);
  CPPUNIT_TEST(testSkipRexSecurityCheck_Refund);

  // Test RuleController::mayBeSkippedByCmdPricing
  CPPUNIT_TEST(testMayBeSkippedByCmdPricing_Pass_Eligibility);
  CPPUNIT_TEST(testMayBeSkippedByCmdPricing_Fail_FareByRule);
  CPPUNIT_TEST(testMayBeSkippedByCmdPricing_Pass_AgentsDiscount);
  CPPUNIT_TEST(testMayBeSkippedByCmdPricing_Fail_TravelRestrictions);
  CPPUNIT_TEST(testMayBeSkippedByCmdPricing_Fail_MiscFareTag);
  CPPUNIT_TEST(testMayBeSkippedByCmdPricing_Fail_Negotiated);
  CPPUNIT_TEST(testMayBeSkippedByCmdPricing_Fail_SaleRestrictions);
  CPPUNIT_TEST(testMayBeSkippedByCmdPricing_Pass_SaleRestrictions);

  CPPUNIT_TEST(testDontReuseSaleRestrictionRule);

  // Tests for RuleController::skipCategoryProcessing()
  CPPUNIT_TEST(testSkipCategoryProcessingForFareByRuleTrue);
  CPPUNIT_TEST(testSkipCategoryProcessingForFareByRuleFalse);
  CPPUNIT_TEST(testSkipCategoryProcessingForNegotiategRuleTrue);
  CPPUNIT_TEST(testSkipCategoryProcessingForNegotiategRuleFalse);
  CPPUNIT_TEST(testSkipCategoryProcessingForDiscounts19True);
  CPPUNIT_TEST(testSkipCategoryProcessingForDiscounts19False);
  CPPUNIT_TEST(testSkipCategoryProcessingForDiscounts20True);
  CPPUNIT_TEST(testSkipCategoryProcessingForDiscounts20False);
  CPPUNIT_TEST(testSkipCategoryProcessingForDiscounts21True);
  CPPUNIT_TEST(testSkipCategoryProcessingForDiscounts21False);
  CPPUNIT_TEST(testSkipCategoryProcessingForDiscounts22True);
  CPPUNIT_TEST(testSkipCategoryProcessingForDiscounts22False);
  CPPUNIT_TEST(testSkipCategoryProcessingForEligibilityTrue);
  CPPUNIT_TEST(testSkipCategoryProcessingForEligibilityFalse);
  CPPUNIT_TEST(testSkipCategoryProcessingForAccompaniedTrue);
  CPPUNIT_TEST(testSkipCategoryProcessingForAccompaniedFalse);
  CPPUNIT_TEST(testSkipCategoryProcessingForVoluntaryExchangeTrue);
  CPPUNIT_TEST(testSkipCategoryProcessingForVoluntaryExchangeFalse);
  CPPUNIT_TEST(testSkipCategoryProcessingForVoluntaryRefundTrue);
  CPPUNIT_TEST(testSkipCategoryProcessingForVoluntaryRefundFalse);

  CPPUNIT_TEST(testSkipCategoryNormalValidationProcessedNotEndorsmentTrue);
  CPPUNIT_TEST(testSkipCategoryNormalValidationProcessedNotEndorsmentFalse);
  CPPUNIT_TEST(testSkipCategoryNormalValidationFBRTrue);
  CPPUNIT_TEST(testSkipCategoryNormalValidationFBRFalse);
  CPPUNIT_TEST(testSkipCategoryNormalValidationNEGTrue);
  CPPUNIT_TEST(testSkipCategoryNormalValidationNEGFalse);
  CPPUNIT_TEST(testSkipCategoryNormalValidationDiscountedTrue);
  CPPUNIT_TEST(testSkipCategoryNormalValidationDiscountedEndorsmentFalse);
  CPPUNIT_TEST(testSkipCategoryNormalValidationDiscountedChildrenFalse);
  CPPUNIT_TEST(testSkipCategoryNormalValidationDiscountedFareRuleStatFalse);
  CPPUNIT_TEST(testSkipCategoryNormalValidationMinimumStayTrue);
  CPPUNIT_TEST(testSkipCategoryNormalValidationMinimumStayNotMayMayNotBeDoubledFalse);
  CPPUNIT_TEST(testSkipCategoryNormalValidationMinimumStayRTFalse);

  CPPUNIT_TEST(testProcessCategoryNormalValidationProcessedNotEndorsmentSkip);
  CPPUNIT_TEST(testProcessCategoryNormalValidationProcessedNotEndorsmentFail);
  CPPUNIT_TEST(testProcessCategoryNormalValidationFBRSkip);
  CPPUNIT_TEST(testProcessCategoryNormalValidationFBRPass);
  CPPUNIT_TEST(testProcessCategoryNormalValidationNEGSkip);
  CPPUNIT_TEST(testProcessCategoryNormalValidationNEGPass);
  CPPUNIT_TEST(testProcessCategoryNormalValidationDiscountedSkip);
  CPPUNIT_TEST(testProcessCategoryNormalValidationDiscountedEndorsmentPass);
  CPPUNIT_TEST(testProcessCategoryNormalValidationDiscountedChildrenPass);
  CPPUNIT_TEST(testProcessCategoryNormalValidationDiscountedFareRuleStatPass);
  CPPUNIT_TEST(testProcessCategoryNormalValidationMinimumStaySkip);
  CPPUNIT_TEST(testProcessCategoryNormalValidationMinimumStayNotMayMayNotBeDoubledPass);
  CPPUNIT_TEST(testProcessCategoryNormalValidationMinimumStayRTSkip);

  // Tests for RuleController::skipCategoryFCORuleValidation()
  CPPUNIT_TEST(testSkipCategoryFCORuleValidationCatProcessed);
  CPPUNIT_TEST(testSkipCategoryFCORuleValidationFBRTrue);
  CPPUNIT_TEST(testSkipCategoryFCORuleValidationFBRFalse);
  CPPUNIT_TEST(testSkipCategoryFCORuleValidationNEGTrue);
  CPPUNIT_TEST(testSkipCategoryFCORuleValidationNEGFalse);
  CPPUNIT_TEST(testSkipCategoryFCORuleValidationDiscountTrue);
  CPPUNIT_TEST(testSkipCategoryFCORuleValidationDiscountFalse);

  // Tests for RuleController::skipRuleDisplayValidation()
  CPPUNIT_SKIP_TEST(testSkipRuleDisplayValidationCat15TuningEnabledTrue);
  CPPUNIT_TEST(testSkipRuleDisplayValidationCat15TuningEnabledFalse);
  CPPUNIT_TEST(testSkipRuleDisplayValidationCat15TuningDisabledTrue);
  CPPUNIT_TEST(testSkipRuleDisplayValidationCat15TuningDisabledFalse);
  CPPUNIT_TEST(testSkipRuleDisplayValidationFBRTrue);
  CPPUNIT_TEST(testSkipRuleDisplayValidationFBRFalse);
  CPPUNIT_TEST(testSkipRuleDisplayValidationNEGTrue);
  CPPUNIT_TEST(testSkipRuleDisplayValidationNEGFalse);

  // Tests for RuleController::skipCalculatedFBR()
  CPPUNIT_TEST(testSkipCalculatedFBRCalcFareTrue);
  CPPUNIT_TEST(testSkipCalculatedFBRCalcFareFalse);
  CPPUNIT_TEST(testSkipCalculatedFBRPURuleValTrue);
  CPPUNIT_TEST(testSkipCalculatedFBRPURuleValProcessedFalse);
  CPPUNIT_TEST(testSkipCalculatedFBRPURuleValSoftPassFalse);
  CPPUNIT_TEST(testSkipCalculatedFBRPURuleValTSTrue);
  CPPUNIT_TEST(testSkipCalculatedFBRPURuleValTSProcessedFalse);
  CPPUNIT_TEST(testSkipCalculatedFBRPURuleValTSSoftPassFalse);
  CPPUNIT_TEST(testSkipCalculatedFBRFPRuleValTrue);
  CPPUNIT_TEST(testSkipCalculatedFBRFPRuleValProcessedFalse);
  CPPUNIT_TEST(testSkipCalculatedFBRFPRuleValISALTTrue);
  CPPUNIT_TEST(testSkipCalculatedFBRFPRuleValISALTProcessedFalse);
  CPPUNIT_TEST(testSkipCalculatedFBRFPRuleValISALTSoftPassFalse);

  // Tests for RuleController::skipCat15ForSellingNetFare
  CPPUNIT_TEST(testSkipCat15ForSellingNetFareSelingFareTrue);
  CPPUNIT_TEST(testSkipCat15ForSellingNetFareNetFareTrue);
  CPPUNIT_TEST(testSkipCat15ForSellingNetFareNetFareUPDTrue);
  CPPUNIT_TEST(testSkipCat15ForSellingNetFareFalse);

  // Tests for RuleController::isBasicValidationPhase
  CPPUNIT_TEST(testIsBasicValidationPhase_Pass_Normal);
  CPPUNIT_TEST(testIsBasicValidationPhase_Pass_FareDisplay);
  CPPUNIT_TEST(testIsBasicValidationPhase_Pass_RuleDisplay);
  CPPUNIT_TEST(testIsBasicValidationPhase_Pass_FCORule);
  CPPUNIT_TEST(testIsBasicValidationPhase_Pass_PreValidation);
  CPPUNIT_TEST(testIsBasicValidationPhase_Pass_ShoppingComponent);
  CPPUNIT_TEST(testIsBasicValidationPhase_Pass_ShoppingAcrossStopOverComponent);
  CPPUNIT_TEST(testIsBasicValidationPhase_Pass_ShoppingComponentValidateQualifiedCat4);
  CPPUNIT_TEST(testIsBasicValidationPhase_Fail_ShoppingASOComponentWithFlights);
  CPPUNIT_TEST(testIsBasicValidationPhase_Fail_ShoppingComponentWithFlightsValidation);

  // Tests for RuleController::ifSkipCat15Security
  CPPUNIT_TEST(testIfSkipCat15Security_Pass_FD);
  CPPUNIT_TEST(testIfSkipCat15Security_Fail_FD);
  CPPUNIT_TEST(testIfSkipCat15Security_Pass_Negotiated);
  CPPUNIT_TEST(testIfSkipCat15Security_Pass_SkipCat15);
  CPPUNIT_TEST(testIfSkipCat15Security_Fail_NotFD);

  // Tests for RuleController::processCategoryPurFprShopping
  CPPUNIT_TEST(testProcessCategoryPurFprShopping_Skip_Eligibility);
  CPPUNIT_TEST(testProcessCategoryPurFprShopping_Skip_FareByRule);
  CPPUNIT_TEST(testProcessCategoryPurFprShopping_Skip_Negotiated);

  // Test other methods
  CPPUNIT_TEST(testReValidDiscQualifiers_Pass);
  CPPUNIT_TEST(testValidate_Pass);

  // Test RuleControllerDataAccess class
  CPPUNIT_TEST(testGetBaseOrPaxTypeFare_NotRetrieve);
  CPPUNIT_TEST(testGetBaseOrPaxTypeFare_NotFareByRule);
  CPPUNIT_TEST(testGetBaseOrPaxTypeFare_NoFBRData);
  CPPUNIT_TEST(testCurrentPU);
  CPPUNIT_TEST(testCurrentFU);
  CPPUNIT_TEST(testProcessRuleResult);
  CPPUNIT_TEST(testDoNotReuseGrFareRuleResult_True_Cat1_Web);
  CPPUNIT_TEST(testDoNotReuseGrFareRuleResult_True_Cat15_Skip_Secure);
  CPPUNIT_TEST(testDoNotReuseGrFareRuleResult_True_Cat18);
  CPPUNIT_TEST(testDoNotReuseGrFareRuleResult_False);

  // Tests for RuleController::updateBaseFareRuleStatDiscounted
  CPPUNIT_TEST(testUpdateBaseFareRuleStatDiscounted_NotUpdated_NotValid_ForFBR);
  CPPUNIT_TEST(testUpdateBaseFareRuleStatDiscounted_NotUpdated_Valid_ForFBR);
  CPPUNIT_TEST(testUpdateBaseFareRuleStatDiscounted_NotUpdated_NotValid_ForCat35);
  CPPUNIT_TEST(testUpdateBaseFareRuleStatDiscounted_NotUpdated_Valid_ForCat35);

  // Tests for RuleController::ApplySystemDefaultAssumption
  CPPUNIT_TEST(testApplySystemDefaultAssumptionCat15_Populate_Validating_Carriers);

  CPPUNIT_TEST(testStoreSalesRestrictionValuesGr);
  CPPUNIT_TEST(testStoreResultsFr);
  CPPUNIT_TEST(testStoreSalesRestrictionValuesFn);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  TestRuleController* _testRuleController;
  std::vector<uint16_t>* _categories;
  MockRexPricingTrx* _rexBaseTrx;
  PaxTypeFare* _paxTypeFare;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _testRuleController = _memHandle.create<TestRuleController>();
    _rexBaseTrx = _memHandle.create<MockRexPricingTrx>();
  }

  void tearDown() { _memHandle.clear(); }

  void createDiscountedRuleData(PaxTypeFare* ptf)
  {
    PaxTypeFareRuleData* ruleData = _memHandle.create<PaxTypeFareRuleData>();
    ruleData->baseFare() = createPaxTypeFare();

    PaxTypeFare::PaxTypeFareAllRuleData* allRules =
        _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
    allRules->chkedRuleData = true;
    allRules->chkedGfrData = false;
    allRules->fareRuleData = ruleData;
    allRules->gfrRuleData = 0;
    (*ptf->paxTypeFareRuleDataMap())[RuleConst::CHILDREN_DISCOUNT_RULE] = allRules;
  }

  void setInfoForBaseFare(PaxTypeFare* ptf, uint16_t cat, bool condition)
  {
    PaxTypeFare* baseFare = ptf->baseFare(RuleConst::CHILDREN_DISCOUNT_RULE);
    if (baseFare)
    {
      baseFare->setCategoryValid(cat, condition);
      baseFare->setCategorySoftPassed(cat, true);
      baseFare->setCategoryProcessed(cat);
    }
  }

  void setInfoForDiscountedFare(PaxTypeFare* ptf, uint16_t cat, bool condition)
  {
    ptf->setCategoryValid(cat, condition);
    ptf->setCategorySoftPassed(cat, true);
    ptf->setCategoryProcessed(cat);
  }

  PaxTypeFare* createPaxTypeFare()
  {
    PaxTypeFare* paxTypeFare = _memHandle(new PaxTypeFare);
    paxTypeFare->setFare(_memHandle(new Fare));
    paxTypeFare->fare()->setFareInfo(_memHandle(new FareInfo));

    return paxTypeFare;
  }

  PaxTypeFare* createPaxTypeFare(Indicator fcaDisplayCatType)
  {
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    FareClassAppInfo* fca = _memHandle(new FareClassAppInfo);
    fca->_displayCatType = fcaDisplayCatType;
    paxTypeFare->fareClassAppInfo() = fca;

    return paxTypeFare;
  }

  PaxTypeFare* createPaxTypeFare(PaxTypeFare::PaxTypeFareState state, bool set = true)
  {
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    paxTypeFare->status().set(state, set);
    return paxTypeFare;
  }

  PaxTypeFare* createPaxTypeFareSoftPassed(uint16_t category, bool set = true)
  {
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    paxTypeFare->setCategorySoftPassed(category, set);
    return paxTypeFare;
  }

  PricingTrx* createPricingTrx()
  {
    PricingTrx* trx = _memHandle(new PricingTrx);
    PricingOptions* popt = _memHandle(new PricingOptions);
    PricingRequest* pricingReqest = _memHandle(new PricingRequest);
    trx->setOptions(popt);
    trx->setRequest(pricingReqest);
    return trx;
    //     return _memHandle(new PricingTrx);
  }

  FakeRuleControllerDataAccess* createRuleControllerDataAccess(PaxTypeFare* ptf)
  {
    return _memHandle(new FakeRuleControllerDataAccess(ptf, createPricingTrx()));
  }

  const std::vector<uint16_t>& setUpSkipRexSecurityCheck()
  {
    _rexBaseTrx->setOptions(_memHandle(new RexPricingOptions));
    _rexBaseTrx->setAnalyzingExcItin(true);
    _rexBaseTrx->_skipSecurityForExcItin = true;

    _paxTypeFare = _memHandle(new PaxTypeFare);
    _paxTypeFare->setFare(_memHandle(new Fare));
    _paxTypeFare->setCategoryValid(RuleConst::ELIGIBILITY_RULE, false);
    _paxTypeFare->setCategoryValid(RuleConst::SALE_RESTRICTIONS_RULE, false);
    _paxTypeFare->setCategoryProcessed(RuleConst::ELIGIBILITY_RULE, false);
    _paxTypeFare->setCategoryProcessed(RuleConst::SALE_RESTRICTIONS_RULE, false);
    assertPTFCategories1and15(false);

    uint16_t categories[5] = { 8, 1, 6, 15, 4 };
    _categories = _memHandle(new std::vector<uint16_t>(categories, categories + 5));

    std::vector<uint16_t>* veryficationCategories =
        _memHandle(new std::vector<uint16_t>(*_categories));
    veryficationCategories->erase(veryficationCategories->begin() + 3); // remove 15
    veryficationCategories->erase(veryficationCategories->begin() + 1); // remove 1
    return *veryficationCategories;
  }

  void assertPTFCategories1and15(bool valid)
  {
    CPPUNIT_ASSERT_EQUAL(valid, _paxTypeFare->isCategoryValid(RuleConst::ELIGIBILITY_RULE));
    CPPUNIT_ASSERT_EQUAL(valid, _paxTypeFare->isCategoryValid(RuleConst::SALE_RESTRICTIONS_RULE));
    CPPUNIT_ASSERT_EQUAL(valid, _paxTypeFare->isCategoryProcessed(RuleConst::ELIGIBILITY_RULE));
    CPPUNIT_ASSERT_EQUAL(valid,
                         _paxTypeFare->isCategoryProcessed(RuleConst::SALE_RESTRICTIONS_RULE));
  }

  void setUpSkipRexSecurityCheck_forVCTR()
  {
    _rexBaseTrx->_skipSecurityForExcItin = false;
    _paxTypeFare->fareMarket() = _memHandle(new FareMarket);
    _paxTypeFare->fareMarket()->fareCompInfo() = _memHandle(new FareCompInfo);
    _paxTypeFare->fareMarket()->fareCompInfo()->hasVCTR() = true;
  }

  // Tests

  void testSkipRexSecurityCheck_Reissue()
  {
    const std::vector<uint16_t>& veryficationCategories = setUpSkipRexSecurityCheck();
    _testRuleController->skipRexSecurityCheck(*_rexBaseTrx, *_paxTypeFare, *_categories);

    assertPTFCategories1and15(true);
    CPPUNIT_ASSERT(veryficationCategories == *_categories);
  }

  void testSkipRexSecurityCheck_forVCTR()
  {
    const std::vector<uint16_t>& veryficationCategories = setUpSkipRexSecurityCheck();
    setUpSkipRexSecurityCheck_forVCTR();
    _testRuleController->skipRexSecurityCheck(*_rexBaseTrx, *_paxTypeFare, *_categories);

    assertPTFCategories1and15(true);
    CPPUNIT_ASSERT(veryficationCategories == *_categories);
  }

  void testSkipRexSecurityCheck_Refund()
  {
    const std::vector<uint16_t>& veryficationCategories = setUpSkipRexSecurityCheck();
    _rexBaseTrx->setExcTrxType(PricingTrx::AF_EXC_TRX);
    _testRuleController->skipRexSecurityCheck(*_rexBaseTrx, *_paxTypeFare, *_categories);

    assertPTFCategories1and15(true);
    CPPUNIT_ASSERT(veryficationCategories == *_categories);
  }

  void testMayBeSkippedByCmdPricing_Pass_Eligibility()
  {
    CPPUNIT_ASSERT(RuleController::mayBeSkippedByCmdPricing(
        *createPricingTrx(), *createPaxTypeFare(), RuleConst::ELIGIBILITY_RULE));
  }

  void testMayBeSkippedByCmdPricing_Fail_FareByRule()
  {
    CPPUNIT_ASSERT(!RuleController::mayBeSkippedByCmdPricing(
                       *createPricingTrx(), *createPaxTypeFare(), RuleConst::FARE_BY_RULE));
  }

  void testMayBeSkippedByCmdPricing_Pass_AgentsDiscount()
  {
    CPPUNIT_ASSERT(RuleController::mayBeSkippedByCmdPricing(
        *createPricingTrx(), *createPaxTypeFare(), RuleConst::AGENTS_DISCOUNT_RULE));
  }

  void testMayBeSkippedByCmdPricing_Fail_TravelRestrictions()
  {
    CPPUNIT_ASSERT(!RuleController::mayBeSkippedByCmdPricing(*createPricingTrx(),
                                                             *createPaxTypeFare(),
                                                             RuleConst::TRAVEL_RESTRICTIONS_RULE));
  }

  void testMayBeSkippedByCmdPricing_Fail_MiscFareTag()
  {
    CPPUNIT_ASSERT(!RuleController::mayBeSkippedByCmdPricing(
                       *createPricingTrx(), *createPaxTypeFare(), RuleConst::MISC_FARE_TAG));
  }

  void testMayBeSkippedByCmdPricing_Fail_Negotiated()
  {
    CPPUNIT_ASSERT(!RuleController::mayBeSkippedByCmdPricing(
                       *createPricingTrx(), *createPaxTypeFare(), RuleConst::NEGOTIATED_RULE));
  }

  void testMayBeSkippedByCmdPricing_Fail_SaleRestrictions()
  {
    CPPUNIT_ASSERT(!RuleController::mayBeSkippedByCmdPricing(*createPricingTrx(),
                                                             *createPaxTypeFare(),
                                                             RuleConst::SALE_RESTRICTIONS_RULE));
  }

  void testMayBeSkippedByCmdPricing_Pass_SaleRestrictions()
  {
    PaxTypeFare* ptf = createPaxTypeFare();
    ptf->setCat15SoftPass(true);
    CPPUNIT_ASSERT(RuleController::mayBeSkippedByCmdPricing(
        *createPricingTrx(), *ptf, RuleConst::SALE_RESTRICTIONS_RULE));
  }

  void testDontReuseSaleRestrictionRule()
  {
    FareMarket::FareMarketSavedGfrResult::Result savedPublFail, savedPublPass;
    FareMarket::FareMarketSavedGfrResult::Result savedPrivFail, savedPrivPass;
    PaxTypeFare privPtf, publPtf;
    Fare privFare, publFare;
    TariffCrossRefInfo publTcr, privTcr;

    publTcr.tariffCat() = (TariffCategory)0;
    privTcr.tariffCat() = RuleConst::PRIVATE_TARIFF;

    privFare.setTariffCrossRefInfo(&privTcr);
    publFare.setTariffCrossRefInfo(&publTcr);

    privPtf.setFare(&privFare);
    publPtf.setFare(&publFare);

    savedPublFail._ret = FAIL;
    savedPrivFail._ret = FAIL;
    savedPublPass._ret = PASS;
    savedPrivPass._ret = PASS;

    savedPublFail._ptfResultFrom = &publPtf;
    savedPublPass._ptfResultFrom = &publPtf;
    savedPrivFail._ptfResultFrom = &privPtf;
    savedPrivPass._ptfResultFrom = &privPtf;

    CPPUNIT_ASSERT(RuleController::dontReuseSaleRestrictionRule(savedPrivFail, publPtf));
    CPPUNIT_ASSERT(RuleController::dontReuseSaleRestrictionRule(savedPrivFail, privPtf));
    CPPUNIT_ASSERT(RuleController::dontReuseSaleRestrictionRule(savedPrivPass, publPtf));
    CPPUNIT_ASSERT(RuleController::dontReuseSaleRestrictionRule(savedPublFail, privPtf));
    CPPUNIT_ASSERT(RuleController::dontReuseSaleRestrictionRule(savedPublPass, privPtf));

    CPPUNIT_ASSERT(!RuleController::dontReuseSaleRestrictionRule(savedPrivPass, privPtf));
    CPPUNIT_ASSERT(!RuleController::dontReuseSaleRestrictionRule(savedPublFail, publPtf));
    CPPUNIT_ASSERT(!RuleController::dontReuseSaleRestrictionRule(savedPublPass, publPtf));
  }

  void testSkipCategoryProcessingForFareByRuleTrue()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.skipCategoryProcessing(
        RuleConst::FARE_BY_RULE,
        *createPaxTypeFare(PaxTypeFare::PTF_FareByRule, false),
        *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForFareByRuleFalse()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(!testRuleController.skipCategoryProcessing(
        RuleConst::FARE_BY_RULE, *createPaxTypeFare(PaxTypeFare::PTF_FareByRule), *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForNegotiategRuleTrue()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.skipCategoryProcessing(
        RuleConst::NEGOTIATED_RULE,
        *createPaxTypeFare(PaxTypeFare::PTF_Negotiated, false),
        *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForNegotiategRuleFalse()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(!testRuleController.skipCategoryProcessing(
        RuleConst::NEGOTIATED_RULE, *createPaxTypeFare(PaxTypeFare::PTF_Negotiated), *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForDiscounts19True()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.skipCategoryProcessing(
        RuleConst::CHILDREN_DISCOUNT_RULE,
        *createPaxTypeFare(PaxTypeFare::PTF_Discounted, false),
        *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForDiscounts19False()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(
        !testRuleController.skipCategoryProcessing(RuleConst::CHILDREN_DISCOUNT_RULE,
                                                   *createPaxTypeFare(PaxTypeFare::PTF_Discounted),
                                                   *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForDiscounts20True()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.skipCategoryProcessing(
        RuleConst::TOUR_DISCOUNT_RULE,
        *createPaxTypeFare(PaxTypeFare::PTF_Discounted, false),
        *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForDiscounts20False()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(
        !testRuleController.skipCategoryProcessing(RuleConst::TOUR_DISCOUNT_RULE,
                                                   *createPaxTypeFare(PaxTypeFare::PTF_Discounted),
                                                   *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForDiscounts21True()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.skipCategoryProcessing(
        RuleConst::AGENTS_DISCOUNT_RULE,
        *createPaxTypeFare(PaxTypeFare::PTF_Discounted, false),
        *_rexBaseTrx));
  }
  void testSkipCategoryProcessingForDiscounts21False()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(
        !testRuleController.skipCategoryProcessing(RuleConst::AGENTS_DISCOUNT_RULE,
                                                   *createPaxTypeFare(PaxTypeFare::PTF_Discounted),
                                                   *_rexBaseTrx));
  }
  void testSkipCategoryProcessingForDiscounts22True()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.skipCategoryProcessing(
        RuleConst::OTHER_DISCOUNT_RULE,
        *createPaxTypeFare(PaxTypeFare::PTF_Discounted, false),
        *_rexBaseTrx));
  }
  void testSkipCategoryProcessingForDiscounts22False()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(
        !testRuleController.skipCategoryProcessing(RuleConst::OTHER_DISCOUNT_RULE,
                                                   *createPaxTypeFare(PaxTypeFare::PTF_Discounted),
                                                   *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForEligibilityTrue()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(
        testRuleController.skipCategoryProcessing(RuleConst::ELIGIBILITY_RULE,
                                                  *createPaxTypeFare(PaxTypeFare::PTF_Discounted),
                                                  *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForEligibilityFalse()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(!testRuleController.skipCategoryProcessing(
        RuleConst::ELIGIBILITY_RULE,
        *createPaxTypeFare(PaxTypeFare::PTF_Discounted, false),
        *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForAccompaniedTrue()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(
        testRuleController.skipCategoryProcessing(RuleConst::ACCOMPANIED_PSG_RULE,
                                                  *createPaxTypeFare(PaxTypeFare::PTF_Discounted),
                                                  *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForAccompaniedFalse()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(!testRuleController.skipCategoryProcessing(
        RuleConst::ACCOMPANIED_PSG_RULE,
        *createPaxTypeFare(PaxTypeFare::PTF_Discounted, false),
        *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForVoluntaryExchangeTrue()
  {
    TestRuleController testRuleController;
    _rexBaseTrx->setExcTrxType(PricingTrx::AF_EXC_TRX);

    CPPUNIT_ASSERT(testRuleController.skipCategoryProcessing(
        RuleConst::VOLUNTARY_EXCHANGE_RULE, *createPaxTypeFare(), *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForVoluntaryExchangeFalse()
  {
    TestRuleController testRuleController;
    _rexBaseTrx->setExcTrxType(PricingTrx::AR_EXC_TRX);

    CPPUNIT_ASSERT(!testRuleController.skipCategoryProcessing(
        RuleConst::VOLUNTARY_EXCHANGE_RULE, *createPaxTypeFare(), *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForVoluntaryRefundTrue()
  {
    TestRuleController testRuleController;
    _rexBaseTrx->setExcTrxType(PricingTrx::AR_EXC_TRX);

    CPPUNIT_ASSERT(testRuleController.skipCategoryProcessing(
        RuleConst::VOLUNTARY_REFUNDS_RULE, *createPaxTypeFare(), *_rexBaseTrx));
  }

  void testSkipCategoryProcessingForVoluntaryRefundFalse()
  {
    TestRuleController testRuleController;
    _rexBaseTrx->setExcTrxType(PricingTrx::AF_EXC_TRX);

    CPPUNIT_ASSERT(!testRuleController.skipCategoryProcessing(
        RuleConst::VOLUNTARY_REFUNDS_RULE, *createPaxTypeFare(), *_rexBaseTrx));
  }

  void testProcessCategoryNormalValidationProcessedNotEndorsmentSkip()
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = 0;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;

    paxTypeFare.setCategoryProcessed(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
    CPPUNIT_ASSERT_EQUAL(SKIP,
        testRuleController.processCategoryNormalValidation(*createPricingTrx(),
														RuleConst::VOLUNTARY_EXCHANGE_RULE,
														paxTypeFare,
														fbrCalcFare,
														fbrPaxTypeFare));
  }

  void testSkipCategoryNormalValidationProcessedNotEndorsmentTrue()
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = 0;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;

    paxTypeFare.setCategoryProcessed(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
    CPPUNIT_ASSERT(
        testRuleController.skipCategoryNormalValidation(*createPricingTrx(),
														RuleConst::VOLUNTARY_EXCHANGE_RULE,
														paxTypeFare,
														fbrCalcFare,
														fbrPaxTypeFare));
  }

  void testProcessCategoryNormalValidationProcessedNotEndorsmentFail()
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = 0;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;

    paxTypeFare.setCategoryProcessed(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
    paxTypeFare.setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, false);
    CPPUNIT_ASSERT_EQUAL(FAIL,
        testRuleController.processCategoryNormalValidation(*createPricingTrx(),
                                                        RuleConst::VOLUNTARY_EXCHANGE_RULE,
                                                        paxTypeFare,
                                                        fbrCalcFare,
                                                        fbrPaxTypeFare));
  }

  void testSkipCategoryNormalValidationProcessedNotEndorsmentFalse()
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = 0;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;

    paxTypeFare.setCategoryProcessed(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
    CPPUNIT_ASSERT(
        testRuleController.skipCategoryNormalValidation(*createPricingTrx(),
                                                        RuleConst::VOLUNTARY_EXCHANGE_RULE,
                                                        paxTypeFare,
                                                        fbrCalcFare,
                                                        fbrPaxTypeFare));
  }

  void testProcessCategoryNormalValidationFBRSkip()
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = 0;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;

    paxTypeFare.setCategorySoftPassed(RuleConst::FARE_BY_RULE, false);
    CPPUNIT_ASSERT_EQUAL(SKIP, testRuleController.processCategoryNormalValidation(
        *createPricingTrx(), RuleConst::FARE_BY_RULE, paxTypeFare, fbrCalcFare, fbrPaxTypeFare));
  }

  void testSkipCategoryNormalValidationFBRTrue()
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = 0;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;

    paxTypeFare.setCategorySoftPassed(RuleConst::FARE_BY_RULE, false);
    CPPUNIT_ASSERT(testRuleController.skipCategoryNormalValidation(
        *createPricingTrx(), RuleConst::FARE_BY_RULE, paxTypeFare, fbrCalcFare, fbrPaxTypeFare));
  }

  void testProcessCategoryNormalValidationFBRPass()
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = 0;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;

    paxTypeFare.setCategorySoftPassed(RuleConst::FARE_BY_RULE, true);
    CPPUNIT_ASSERT_EQUAL(PASS, testRuleController.processCategoryNormalValidation(
        *createPricingTrx(), RuleConst::FARE_BY_RULE, paxTypeFare, fbrCalcFare, fbrPaxTypeFare));
  }

  void testSkipCategoryNormalValidationFBRFalse()
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = 0;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;

    paxTypeFare.setCategorySoftPassed(RuleConst::FARE_BY_RULE, true);
    CPPUNIT_ASSERT(!testRuleController.skipCategoryNormalValidation(
        *createPricingTrx(), RuleConst::FARE_BY_RULE, paxTypeFare, fbrCalcFare, fbrPaxTypeFare));
  }

  void testProcessCategoryNormalValidationNEGSkip()
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = 0;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;

    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, false);
    CPPUNIT_ASSERT_EQUAL(SKIP, testRuleController.processCategoryNormalValidation(
        *createPricingTrx(), RuleConst::NEGOTIATED_RULE, paxTypeFare, fbrCalcFare, fbrPaxTypeFare));
  }

  void testSkipCategoryNormalValidationNEGTrue()
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = 0;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;

    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, false);
    CPPUNIT_ASSERT(testRuleController.skipCategoryNormalValidation(
        *createPricingTrx(), RuleConst::NEGOTIATED_RULE, paxTypeFare, fbrCalcFare, fbrPaxTypeFare));
  }

  void testProcessCategoryNormalValidationNEGPass()
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = 0;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;

    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, true);
    CPPUNIT_ASSERT_EQUAL(PASS, testRuleController.processCategoryNormalValidation(
        *createPricingTrx(), RuleConst::NEGOTIATED_RULE, paxTypeFare, fbrCalcFare, fbrPaxTypeFare));
  }

  void testSkipCategoryNormalValidationNEGFalse()
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = 0;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;

    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, true);
    CPPUNIT_ASSERT(!testRuleController.skipCategoryNormalValidation(
        *createPricingTrx(), RuleConst::NEGOTIATED_RULE, paxTypeFare, fbrCalcFare, fbrPaxTypeFare));
  }

  void testProcessCategoryNormalValidationDiscountedSkip()
  {
    DataHandle dataHandle;
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;
    paxTypeFare.status().set(PaxTypeFare::PTF_Discounted);

    fbrPaxTypeFare.baseFare() = createPaxTypeFare();
    paxTypeFare.setRuleData(RuleConst::CHILDREN_DISCOUNT_RULE, dataHandle, &fbrPaxTypeFare, true);
    paxTypeFare.setCategoryProcessed(RuleConst::HIP_RULE, false);

    CPPUNIT_ASSERT_EQUAL(SKIP, testRuleController.processCategoryNormalValidation(
        *createPricingTrx(), RuleConst::HIP_RULE, paxTypeFare, fbrCalcFare, 0));
  }

  void testSkipCategoryNormalValidationDiscountedTrue()
  {
    DataHandle dataHandle;
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;
    paxTypeFare.status().set(PaxTypeFare::PTF_Discounted);

    fbrPaxTypeFare.baseFare() = createPaxTypeFare();
    paxTypeFare.setRuleData(RuleConst::CHILDREN_DISCOUNT_RULE, dataHandle, &fbrPaxTypeFare, true);
    paxTypeFare.setCategoryProcessed(RuleConst::HIP_RULE, false);

    CPPUNIT_ASSERT(testRuleController.skipCategoryNormalValidation(
        *createPricingTrx(), RuleConst::HIP_RULE, paxTypeFare, fbrCalcFare, 0));
  }

  void testProcessCategoryNormalValidationDiscountedEndorsmentPass()
  {
    DataHandle dataHandle;
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;
    paxTypeFare.status().set(PaxTypeFare::PTF_Discounted);

    paxTypeFare.setRuleData(RuleConst::CHILDREN_DISCOUNT_RULE, dataHandle, &fbrPaxTypeFare, true);
    paxTypeFare.setCategoryProcessed(RuleConst::TICKET_ENDORSMENT_RULE, false);

    CPPUNIT_ASSERT_EQUAL(PASS, testRuleController.processCategoryNormalValidation(
        *createPricingTrx(), RuleConst::TICKET_ENDORSMENT_RULE, paxTypeFare, fbrCalcFare, 0));
  }

  void testSkipCategoryNormalValidationDiscountedEndorsmentFalse()
  {
    DataHandle dataHandle;
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;
    paxTypeFare.status().set(PaxTypeFare::PTF_Discounted);

    paxTypeFare.setRuleData(RuleConst::CHILDREN_DISCOUNT_RULE, dataHandle, &fbrPaxTypeFare, true);
    paxTypeFare.setCategoryProcessed(RuleConst::TICKET_ENDORSMENT_RULE, false);

    CPPUNIT_ASSERT(!testRuleController.skipCategoryNormalValidation(
        *createPricingTrx(), RuleConst::TICKET_ENDORSMENT_RULE, paxTypeFare, fbrCalcFare, 0));
  }

  void testProcessCategoryNormalValidationDiscountedChildrenPass()
  {
    DataHandle dataHandle;
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;
    paxTypeFare.status().set(PaxTypeFare::PTF_Discounted, true);

    fbrPaxTypeFare.baseFare() = createPaxTypeFare();
    paxTypeFare.setRuleData(RuleConst::TICKET_ENDORSMENT_RULE, dataHandle, &fbrPaxTypeFare, true);
    paxTypeFare.setCategoryProcessed(RuleConst::NEGOTIATED_RULE, false);
    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, true);
    CPPUNIT_ASSERT_EQUAL(PASS, testRuleController.processCategoryNormalValidation(
        *createPricingTrx(), RuleConst::NEGOTIATED_RULE, paxTypeFare, fbrCalcFare, 0));
  }

  void testSkipCategoryNormalValidationDiscountedChildrenFalse()
  {
    DataHandle dataHandle;
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;
    paxTypeFare.status().set(PaxTypeFare::PTF_Discounted, true);

    fbrPaxTypeFare.baseFare() = createPaxTypeFare();
    paxTypeFare.setRuleData(RuleConst::TICKET_ENDORSMENT_RULE, dataHandle, &fbrPaxTypeFare, true);
    paxTypeFare.setCategoryProcessed(RuleConst::NEGOTIATED_RULE, false);
    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, true);
    CPPUNIT_ASSERT(!testRuleController.skipCategoryNormalValidation(
        *createPricingTrx(), RuleConst::NEGOTIATED_RULE, paxTypeFare, fbrCalcFare, 0));
  }

  void testProcessCategoryNormalValidationDiscountedFareRuleStatPass()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;
    paxTypeFare.status().set(PaxTypeFare::PTF_Discounted, true);

    paxTypeFare.setCategoryProcessed(RuleConst::PENALTIES_RULE, false);
    paxTypeFare.setCategorySoftPassed(RuleConst::PENALTIES_RULE, true);
    CPPUNIT_ASSERT_EQUAL(PASS, testRuleController.processCategoryNormalValidation(
        *createPricingTrx(), RuleConst::PENALTIES_RULE, paxTypeFare, fbrCalcFare, 0));
  }

  void testSkipCategoryNormalValidationDiscountedFareRuleStatFalse()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    TestRuleController testRuleController;
    paxTypeFare.status().set(PaxTypeFare::PTF_Discounted, true);

    paxTypeFare.setCategoryProcessed(RuleConst::PENALTIES_RULE, false);
    paxTypeFare.setCategorySoftPassed(RuleConst::PENALTIES_RULE, true);
    CPPUNIT_ASSERT(!testRuleController.skipCategoryNormalValidation(
        *createPricingTrx(), RuleConst::PENALTIES_RULE, paxTypeFare, fbrCalcFare, 0));
  }

  void testProcessCategoryNormalValidationMinimumStaySkip()
  {
    DataHandle dataHandle;
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    bool fbrCalcFare = true;

    TestRuleController testRuleController;
    fbrPaxTypeFare.baseFare() = createPaxTypeFare();
    const_cast<FareInfo*>(fbrPaxTypeFare.baseFare()->fare()->fareInfo())->owrt() =
        ROUND_TRIP_MAYNOT_BE_HALVED;
    const_cast<FareInfo*>(paxTypeFare->fare()->fareInfo())->owrt() = ONE_WAY_MAY_BE_DOUBLED;
    paxTypeFare->setRuleData(RuleConst::CHILDREN_DISCOUNT_RULE, dataHandle, &fbrPaxTypeFare, true);

    paxTypeFare->setCategoryProcessed(RuleConst::MINIMUM_STAY_RULE, false);
    CPPUNIT_ASSERT_EQUAL(SKIP, testRuleController.processCategoryNormalValidation(*createPricingTrx(),
                                                                   RuleConst::MINIMUM_STAY_RULE,
                                                                   *paxTypeFare,
                                                                   fbrCalcFare,
                                                                   &fbrPaxTypeFare));
  }

  void testSkipCategoryNormalValidationMinimumStayTrue()
  {
    DataHandle dataHandle;
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    bool fbrCalcFare = true;

    TestRuleController testRuleController;
    fbrPaxTypeFare.baseFare() = createPaxTypeFare();
    const_cast<FareInfo*>(fbrPaxTypeFare.baseFare()->fare()->fareInfo())->owrt() =
        ROUND_TRIP_MAYNOT_BE_HALVED;
    const_cast<FareInfo*>(paxTypeFare->fare()->fareInfo())->owrt() = ONE_WAY_MAY_BE_DOUBLED;
    paxTypeFare->setRuleData(RuleConst::CHILDREN_DISCOUNT_RULE, dataHandle, &fbrPaxTypeFare, true);

    paxTypeFare->setCategoryProcessed(RuleConst::MINIMUM_STAY_RULE, false);
    CPPUNIT_ASSERT(testRuleController.skipCategoryNormalValidation(*createPricingTrx(),
                                                                   RuleConst::MINIMUM_STAY_RULE,
                                                                   *paxTypeFare,
                                                                   fbrCalcFare,
                                                                   &fbrPaxTypeFare));
  }

  void testProcessCategoryNormalValidationMinimumStayNotMayMayNotBeDoubledPass()
  {
    DataHandle dataHandle;
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    bool fbrCalcFare = true;

    TestRuleController testRuleController;
    fbrPaxTypeFare.baseFare() = createPaxTypeFare();
    const_cast<FareInfo*>(fbrPaxTypeFare.baseFare()->fare()->fareInfo())->owrt() =
        ROUND_TRIP_MAYNOT_BE_HALVED;
    const_cast<FareInfo*>(paxTypeFare->fare()->fareInfo())->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    paxTypeFare->setRuleData(RuleConst::CHILDREN_DISCOUNT_RULE, dataHandle, &fbrPaxTypeFare, true);

    paxTypeFare->setCategoryProcessed(RuleConst::MINIMUM_STAY_RULE, false);
    CPPUNIT_ASSERT_EQUAL(PASS, testRuleController.processCategoryNormalValidation(*createPricingTrx(),
                                                                    RuleConst::MINIMUM_STAY_RULE,
                                                                    *paxTypeFare,
                                                                    fbrCalcFare,
                                                                    &fbrPaxTypeFare));
  }

  void testSkipCategoryNormalValidationMinimumStayNotMayMayNotBeDoubledFalse()
  {
    DataHandle dataHandle;
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    bool fbrCalcFare = true;

    TestRuleController testRuleController;
    fbrPaxTypeFare.baseFare() = createPaxTypeFare();
    const_cast<FareInfo*>(fbrPaxTypeFare.baseFare()->fare()->fareInfo())->owrt() =
        ROUND_TRIP_MAYNOT_BE_HALVED;
    const_cast<FareInfo*>(paxTypeFare->fare()->fareInfo())->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    paxTypeFare->setRuleData(RuleConst::CHILDREN_DISCOUNT_RULE, dataHandle, &fbrPaxTypeFare, true);

    paxTypeFare->setCategoryProcessed(RuleConst::MINIMUM_STAY_RULE, false);
    CPPUNIT_ASSERT(!testRuleController.skipCategoryNormalValidation(*createPricingTrx(),
                                                                    RuleConst::MINIMUM_STAY_RULE,
                                                                    *paxTypeFare,
                                                                    fbrCalcFare,
                                                                    &fbrPaxTypeFare));
  }

  void testProcessCategoryNormalValidationMinimumStayRTSkip()
  {
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    bool fbrCalcFare = true;

    TestRuleController testRuleController;
    fbrPaxTypeFare.baseFare() = createPaxTypeFare();
    const_cast<FareInfo*>(fbrPaxTypeFare.baseFare()->fare()->fareInfo())->owrt() =
        ROUND_TRIP_MAYNOT_BE_HALVED;
    const_cast<FareInfo*>(paxTypeFare->fare()->fareInfo())->owrt() = ONE_WAY_MAY_BE_DOUBLED;

    paxTypeFare->setCategoryProcessed(RuleConst::MINIMUM_STAY_RULE, false);
    // Should be skipped for no NEG fare.
    CPPUNIT_ASSERT_EQUAL(SKIP, testRuleController.processCategoryNormalValidation(*createPricingTrx(),
                                                                   RuleConst::MINIMUM_STAY_RULE,
                                                                   *paxTypeFare,
                                                                   fbrCalcFare,
                                                                   &fbrPaxTypeFare));
  }

  void testSkipCategoryNormalValidationMinimumStayRTFalse()
  {
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    bool fbrCalcFare = true;

    TestRuleController testRuleController;
    fbrPaxTypeFare.baseFare() = createPaxTypeFare();
    const_cast<FareInfo*>(fbrPaxTypeFare.baseFare()->fare()->fareInfo())->owrt() =
        ROUND_TRIP_MAYNOT_BE_HALVED;
    const_cast<FareInfo*>(paxTypeFare->fare()->fareInfo())->owrt() = ONE_WAY_MAY_BE_DOUBLED;

    paxTypeFare->setCategoryProcessed(RuleConst::MINIMUM_STAY_RULE, false);
    // Should be skipped for no NEG fare.
    CPPUNIT_ASSERT(testRuleController.skipCategoryNormalValidation(*createPricingTrx(),
                                                                   RuleConst::MINIMUM_STAY_RULE,
                                                                   *paxTypeFare,
                                                                   fbrCalcFare,
                                                                   &fbrPaxTypeFare));
  }

  void testProcessCategoryNormalValidationMaximumStaySkip()
  {
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    bool fbrCalcFare = true;

    TestRuleController testRuleController;
    fbrPaxTypeFare.baseFare() = createPaxTypeFare();
    const_cast<FareInfo*>(fbrPaxTypeFare.baseFare()->fare()->fareInfo())->owrt() =
        ROUND_TRIP_MAYNOT_BE_HALVED;
    const_cast<FareInfo*>(paxTypeFare->fare()->fareInfo())->owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;

    paxTypeFare->setCategoryProcessed(RuleConst::MINIMUM_STAY_RULE, false);
    CPPUNIT_ASSERT_EQUAL(SKIP, testRuleController.processCategoryNormalValidation(*createPricingTrx(),
                                                                   RuleConst::MINIMUM_STAY_RULE,
                                                                   *paxTypeFare,
                                                                   fbrCalcFare,
                                                                   &fbrPaxTypeFare));
  }

  void testSkipCategoryNormalValidationMaximumStayTrue()
  {
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    bool fbrCalcFare = true;

    TestRuleController testRuleController;
    fbrPaxTypeFare.baseFare() = createPaxTypeFare();
    const_cast<FareInfo*>(fbrPaxTypeFare.baseFare()->fare()->fareInfo())->owrt() =
        ROUND_TRIP_MAYNOT_BE_HALVED;
    const_cast<FareInfo*>(paxTypeFare->fare()->fareInfo())->owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;

    paxTypeFare->setCategoryProcessed(RuleConst::MINIMUM_STAY_RULE, false);
    CPPUNIT_ASSERT(testRuleController.skipCategoryNormalValidation(*createPricingTrx(),
                                                                   RuleConst::MINIMUM_STAY_RULE,
                                                                   *paxTypeFare,
                                                                   fbrCalcFare,
                                                                   &fbrPaxTypeFare));
  }

  void testProcessCategoryNormalValidationMaximumStayNotMayMayNotBeDoubledPass()
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = 0;
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    bool fbrCalcFare = true;

    TestRuleController testRuleController;
    const_cast<FareInfo*>(paxTypeFare->fare()->fareInfo())->owrt() = ALL_WAYS;

    CPPUNIT_ASSERT_EQUAL(PASS, testRuleController.processCategoryNormalValidation(*createPricingTrx(),
                                                                    RuleConst::MAXIMUM_STAY_RULE,
                                                                    *paxTypeFare,
                                                                    fbrCalcFare,
                                                                    fbrPaxTypeFare));
  }

  void testSkipCategoryNormalValidationMaximumStayNotMayMayNotBeDoubledFalse()
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = 0;
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    bool fbrCalcFare = true;

    TestRuleController testRuleController;
    const_cast<FareInfo*>(paxTypeFare->fare()->fareInfo())->owrt() = ALL_WAYS;

    CPPUNIT_ASSERT(!testRuleController.skipCategoryNormalValidation(*createPricingTrx(),
                                                                    RuleConst::MAXIMUM_STAY_RULE,
                                                                    *paxTypeFare,
                                                                    fbrCalcFare,
                                                                    fbrPaxTypeFare));
  }

  void testProcessCategoryNormalValidationMaximumStayRTPass()
  {
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    bool fbrCalcFare = true;

    TestRuleController testRuleController;
    fbrPaxTypeFare.baseFare() = createPaxTypeFare();
    const_cast<FareInfo*>(fbrPaxTypeFare.baseFare()->fare()->fareInfo())->owrt() =
        ONE_WAY_MAYNOT_BE_DOUBLED;
    const_cast<FareInfo*>(paxTypeFare->fare()->fareInfo())->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;

    paxTypeFare->setCategoryProcessed(RuleConst::MINIMUM_STAY_RULE, false);
    CPPUNIT_ASSERT_EQUAL(PASS, testRuleController.processCategoryNormalValidation(*createPricingTrx(),
                                                                   RuleConst::MAXIMUM_STAY_RULE,
                                                                   *paxTypeFare,
                                                                   fbrCalcFare,
                                                                   &fbrPaxTypeFare));
  }

  void testSkipCategoryNormalValidationMaximumStayRTFalse()
  {
    FBRPaxTypeFareRuleData fbrPaxTypeFare;
    PaxTypeFare* paxTypeFare = createPaxTypeFare();
    bool fbrCalcFare = true;

    TestRuleController testRuleController;
    fbrPaxTypeFare.baseFare() = createPaxTypeFare();
    const_cast<FareInfo*>(fbrPaxTypeFare.baseFare()->fare()->fareInfo())->owrt() =
        ONE_WAY_MAYNOT_BE_DOUBLED;
    const_cast<FareInfo*>(paxTypeFare->fare()->fareInfo())->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;

    paxTypeFare->setCategoryProcessed(RuleConst::MINIMUM_STAY_RULE, false);
    CPPUNIT_ASSERT(testRuleController.skipCategoryNormalValidation(*createPricingTrx(),
                                                                   RuleConst::MAXIMUM_STAY_RULE,
                                                                   *paxTypeFare,
                                                                   fbrCalcFare,
                                                                   &fbrPaxTypeFare));
  }

  // Tests for RuleController::skipCategoryFCORuleValidation()
  void testSkipCategoryFCORuleValidationCatProcessed()
  {
    TestRuleController testRuleController;
    PaxTypeFare* ptf = createPaxTypeFare();
    ptf->setCategoryProcessed(RuleConst::ELIGIBILITY_RULE, true);
    CPPUNIT_ASSERT(testRuleController.skipCategoryFCORuleValidation(
        *createPricingTrx(), RuleConst::ELIGIBILITY_RULE, *ptf));
  }

  void testSkipCategoryFCORuleValidationFBRTrue()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.skipCategoryFCORuleValidation(
        *createPricingTrx(),
        RuleConst::FARE_BY_RULE,
        *createPaxTypeFareSoftPassed(RuleConst::FARE_BY_RULE, false)));
  }

  void testSkipCategoryFCORuleValidationFBRFalse()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(!testRuleController.skipCategoryFCORuleValidation(
        *createPricingTrx(),
        RuleConst::FARE_BY_RULE,
        *createPaxTypeFareSoftPassed(RuleConst::FARE_BY_RULE, true)));
  }

  void testSkipCategoryFCORuleValidationNEGTrue()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.skipCategoryFCORuleValidation(
        *createPricingTrx(),
        RuleConst::NEGOTIATED_RULE,
        *createPaxTypeFareSoftPassed(RuleConst::NEGOTIATED_RULE, false)));
  }

  void testSkipCategoryFCORuleValidationNEGFalse()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(!testRuleController.skipCategoryFCORuleValidation(
        *createPricingTrx(),
        RuleConst::NEGOTIATED_RULE,
        *createPaxTypeFareSoftPassed(RuleConst::NEGOTIATED_RULE, true)));
  }

  void testSkipCategoryFCORuleValidationDiscountTrue()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(!testRuleController.skipCategoryFCORuleValidation(
        *createPricingTrx(),
        RuleConst::ELIGIBILITY_RULE,
        *createPaxTypeFare(PaxTypeFare::PTF_Discounted, true)));
  }

  void testSkipCategoryFCORuleValidationDiscountFalse()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(!testRuleController.skipCategoryFCORuleValidation(
        *createPricingTrx(),
        RuleConst::ELIGIBILITY_RULE,
        *createPaxTypeFare(PaxTypeFare::PTF_Discounted, false)));
  }

  // Tests for RuleController::skipCategoryFCORuleValidationCatProcessed()
  void testSkipRuleDisplayValidationCat15TuningEnabledTrue()
  {
    PaxTypeFare paxTypeFare;

    paxTypeFare.setCategoryProcessed(RuleConst::SALE_RESTRICTIONS_RULE);

    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.skipRuleDisplayValidation(RuleConst::SALE_RESTRICTIONS_RULE,
                                                                paxTypeFare));
  }

  void testSkipRuleDisplayValidationCat15TuningEnabledFalse()
  {
    PaxTypeFare paxTypeFare;

    paxTypeFare.setCategoryProcessed(RuleConst::SALE_RESTRICTIONS_RULE);

    TestRuleController testRuleController;
    CPPUNIT_ASSERT(!testRuleController.skipRuleDisplayValidation(RuleConst::SALE_RESTRICTIONS_RULE,
                                                                 paxTypeFare));
  }

  void testSkipRuleDisplayValidationCat15TuningDisabledTrue()
  {
    PaxTypeFare paxTypeFare;

    paxTypeFare.setCategoryProcessed(RuleConst::ELIGIBILITY_RULE);

    TestRuleController testRuleController;
    CPPUNIT_ASSERT(
        testRuleController.skipRuleDisplayValidation(RuleConst::ELIGIBILITY_RULE, paxTypeFare));
  }

  void testSkipRuleDisplayValidationCat15TuningDisabledFalse()
  {
    PaxTypeFare paxTypeFare;

    paxTypeFare.setCategoryProcessed(RuleConst::ELIGIBILITY_RULE, false);

    TestRuleController testRuleController;
    CPPUNIT_ASSERT(
        !testRuleController.skipRuleDisplayValidation(RuleConst::ELIGIBILITY_RULE, paxTypeFare));
  }

  void testSkipRuleDisplayValidationFBRTrue()
  {
    PaxTypeFare paxTypeFare;

    paxTypeFare.setCategorySoftPassed(RuleConst::FARE_BY_RULE, false);

    TestRuleController testRuleController;
    CPPUNIT_ASSERT(
        testRuleController.skipRuleDisplayValidation(RuleConst::FARE_BY_RULE, paxTypeFare));
  }

  void testSkipRuleDisplayValidationFBRFalse()
  {
    PaxTypeFare paxTypeFare;

    paxTypeFare.setCategorySoftPassed(RuleConst::FARE_BY_RULE, false);

    TestRuleController testRuleController;
    CPPUNIT_ASSERT(
        testRuleController.skipRuleDisplayValidation(RuleConst::FARE_BY_RULE, paxTypeFare));
  }

  void testSkipRuleDisplayValidationNEGTrue()
  {
    PaxTypeFare paxTypeFare;

    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, false);

    TestRuleController testRuleController;
    CPPUNIT_ASSERT(
        testRuleController.skipRuleDisplayValidation(RuleConst::NEGOTIATED_RULE, paxTypeFare));
  }

  void testSkipRuleDisplayValidationNEGFalse()
  {
    PaxTypeFare paxTypeFare;

    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, false);

    TestRuleController testRuleController;
    CPPUNIT_ASSERT(
        testRuleController.skipRuleDisplayValidation(RuleConst::NEGOTIATED_RULE, paxTypeFare));
  }

  void testSkipCalculatedFBRCalcFareTrue()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = true;

    paxTypeFare.setCategoryProcessed(RuleConst::NEGOTIATED_RULE);
    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, false);

    TestRuleController testRuleController;
    testRuleController.categoryFase() = PURuleValidation;
    CPPUNIT_ASSERT(
        testRuleController.skipCalculatedFBR(paxTypeFare, fbrCalcFare, RuleConst::NEGOTIATED_RULE));
  }

  void testSkipCalculatedFBRCalcFareFalse()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = false;

    paxTypeFare.setCategoryProcessed(RuleConst::NEGOTIATED_RULE);
    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, false);

    TestRuleController testRuleController;
    testRuleController.categoryFase() = PURuleValidation;
    CPPUNIT_ASSERT(!testRuleController.skipCalculatedFBR(
        paxTypeFare, fbrCalcFare, RuleConst::NEGOTIATED_RULE));
  }

  void testSkipCalculatedFBRPURuleValTrue()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = true;

    paxTypeFare.setCategoryProcessed(RuleConst::NEGOTIATED_RULE);
    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, false);

    TestRuleController testRuleController;
    testRuleController.categoryFase() = PURuleValidation;
    CPPUNIT_ASSERT(
        testRuleController.skipCalculatedFBR(paxTypeFare, fbrCalcFare, RuleConst::NEGOTIATED_RULE));
  }

  void testSkipCalculatedFBRPURuleValProcessedFalse()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = true;

    paxTypeFare.setCategoryProcessed(RuleConst::NEGOTIATED_RULE, false);
    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, false);

    TestRuleController testRuleController;
    testRuleController.categoryFase() = PURuleValidation;
    CPPUNIT_ASSERT(!testRuleController.skipCalculatedFBR(
        paxTypeFare, fbrCalcFare, RuleConst::NEGOTIATED_RULE));
  }

  void testSkipCalculatedFBRPURuleValSoftPassFalse()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = true;

    paxTypeFare.setCategoryProcessed(RuleConst::NEGOTIATED_RULE);
    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE);

    TestRuleController testRuleController;
    testRuleController.categoryFase() = PURuleValidation;
    CPPUNIT_ASSERT(!testRuleController.skipCalculatedFBR(
        paxTypeFare, fbrCalcFare, RuleConst::NEGOTIATED_RULE));
  }

  void testSkipCalculatedFBRPURuleValTSTrue()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = true;

    paxTypeFare.setCategoryProcessed(RuleConst::NEGOTIATED_RULE);
    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, false);

    TestRuleController testRuleController;
    testRuleController.categoryFase() = PURuleValidationIS;
    CPPUNIT_ASSERT(
        testRuleController.skipCalculatedFBR(paxTypeFare, fbrCalcFare, RuleConst::NEGOTIATED_RULE));
  }

  void testSkipCalculatedFBRPURuleValTSProcessedFalse()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = true;

    paxTypeFare.setCategoryProcessed(RuleConst::NEGOTIATED_RULE, false);
    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, false);

    TestRuleController testRuleController;
    testRuleController.categoryFase() = PURuleValidationIS;
    CPPUNIT_ASSERT(!testRuleController.skipCalculatedFBR(
        paxTypeFare, fbrCalcFare, RuleConst::NEGOTIATED_RULE));
  }

  void testSkipCalculatedFBRPURuleValTSSoftPassFalse()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = true;

    paxTypeFare.setCategoryProcessed(RuleConst::NEGOTIATED_RULE);
    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE);

    TestRuleController testRuleController;
    testRuleController.categoryFase() = PURuleValidationIS;
    CPPUNIT_ASSERT(!testRuleController.skipCalculatedFBR(
        paxTypeFare, fbrCalcFare, RuleConst::NEGOTIATED_RULE));
  }

  void testSkipCalculatedFBRFPRuleValTrue()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = true;

    paxTypeFare.setCategoryProcessed(RuleConst::NEGOTIATED_RULE);
    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, false);

    TestRuleController testRuleController;
    testRuleController.categoryFase() = FPRuleValidation;
    CPPUNIT_ASSERT(
        testRuleController.skipCalculatedFBR(paxTypeFare, fbrCalcFare, RuleConst::NEGOTIATED_RULE));
  }

  void testSkipCalculatedFBRFPRuleValProcessedFalse()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = true;

    TestRuleController testRuleController;

    CPPUNIT_ASSERT(!testRuleController.skipCalculatedFBR(
        paxTypeFare, fbrCalcFare, RuleConst::NEGOTIATED_RULE));
  }

  void testSkipCalculatedFBRFPRuleValISALTTrue()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = true;

    paxTypeFare.setCategoryProcessed(RuleConst::NEGOTIATED_RULE);
    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, false);

    TestRuleController testRuleController;
    testRuleController.categoryFase() = FPRuleValidationISALT;
    CPPUNIT_ASSERT(
        testRuleController.skipCalculatedFBR(paxTypeFare, fbrCalcFare, RuleConst::NEGOTIATED_RULE));
  }

  void testSkipCalculatedFBRFPRuleValISALTProcessedFalse()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = true;

    paxTypeFare.setCategoryProcessed(RuleConst::NEGOTIATED_RULE, false);
    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, false);

    TestRuleController testRuleController;
    testRuleController.categoryFase() = FPRuleValidationISALT;
    CPPUNIT_ASSERT(!testRuleController.skipCalculatedFBR(
        paxTypeFare, fbrCalcFare, RuleConst::NEGOTIATED_RULE));
  }

  void testSkipCalculatedFBRFPRuleValISALTSoftPassFalse()
  {
    PaxTypeFare paxTypeFare;
    bool fbrCalcFare = true;

    paxTypeFare.setCategoryProcessed(RuleConst::NEGOTIATED_RULE);
    paxTypeFare.setCategorySoftPassed(RuleConst::NEGOTIATED_RULE);

    TestRuleController testRuleController;
    testRuleController.categoryFase() = FPRuleValidationISALT;
    CPPUNIT_ASSERT(!testRuleController.skipCalculatedFBR(
        paxTypeFare, fbrCalcFare, RuleConst::NEGOTIATED_RULE));
  }

  // Tests for RuleController::skipCat15ForSellingNetFare
  void testSkipCat15ForSellingNetFareSelingFareTrue()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.skipCat15ForSellingNetFare(RuleConst::SELLING_FARE));
  }

  void testSkipCat15ForSellingNetFareNetFareTrue()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.skipCat15ForSellingNetFare(RuleConst::NET_SUBMIT_FARE));
  }

  void testSkipCat15ForSellingNetFareNetFareUPDTrue()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.skipCat15ForSellingNetFare(RuleConst::NET_SUBMIT_FARE_UPD));
  }

  void testSkipCat15ForSellingNetFareFalse()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(
        !testRuleController.skipCat15ForSellingNetFare(RuleConst::SELLING_FARE_NOT_FOR_SEC));
  }

  void testIsBasicValidationPhase_Pass_Normal()
  {
    TestRuleController testRuleController;
    testRuleController._categoryPhase = NormalValidation;
    CPPUNIT_ASSERT(testRuleController.isBasicValidationPhase());
  }

  void testIsBasicValidationPhase_Pass_FareDisplay()
  {
    TestRuleController testRuleController;
    testRuleController._categoryPhase = FareDisplayValidation;
    CPPUNIT_ASSERT(testRuleController.isBasicValidationPhase());
  }

  void testIsBasicValidationPhase_Pass_RuleDisplay()
  {
    TestRuleController testRuleController;
    testRuleController._categoryPhase = RuleDisplayValidation;
    CPPUNIT_ASSERT(testRuleController.isBasicValidationPhase());
  }

  void testIsBasicValidationPhase_Pass_FCORule()
  {
    TestRuleController testRuleController;
    testRuleController._categoryPhase = FCORuleValidation;
    CPPUNIT_ASSERT(testRuleController.isBasicValidationPhase());
  }

  void testIsBasicValidationPhase_Pass_PreValidation()
  {
    TestRuleController testRuleController;
    testRuleController._categoryPhase = PreValidation;
    CPPUNIT_ASSERT(testRuleController.isBasicValidationPhase());
  }

  void testIsBasicValidationPhase_Pass_ShoppingComponent()
  {
    TestRuleController testRuleController;
    testRuleController._categoryPhase = ShoppingComponentValidation;
    CPPUNIT_ASSERT(testRuleController.isBasicValidationPhase());
  }

  void testIsBasicValidationPhase_Pass_ShoppingAcrossStopOverComponent()
  {
    TestRuleController testRuleController;
    testRuleController._categoryPhase = ShoppingAcrossStopOverComponentValidation;
    CPPUNIT_ASSERT(testRuleController.isBasicValidationPhase());
  }

  void testIsBasicValidationPhase_Pass_ShoppingComponentValidateQualifiedCat4()
  {
    TestRuleController testRuleController;
    testRuleController._categoryPhase = ShoppingComponentValidateQualifiedCat4;
    CPPUNIT_ASSERT(testRuleController.isBasicValidationPhase());
  }

  void testIsBasicValidationPhase_Fail_ShoppingASOComponentWithFlights()
  {
    TestRuleController testRuleController;
    testRuleController._categoryPhase = ShoppingASOComponentWithFlightsValidation;
    CPPUNIT_ASSERT(!testRuleController.isBasicValidationPhase());
  }

  void testIsBasicValidationPhase_Fail_ShoppingComponentWithFlightsValidation()
  {
    TestRuleController testRuleController;
    testRuleController._categoryPhase = ShoppingComponentWithFlightsValidation;
    CPPUNIT_ASSERT(!testRuleController.isBasicValidationPhase());
  }

  void testIfSkipCat15Security_Pass_FD()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.ifSkipCat15Security(
        *createPaxTypeFare(RuleConst::SELLING_FARE), 0, true));
  }

  void testIfSkipCat15Security_Fail_FD()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(!testRuleController.ifSkipCat15Security(
        *createPaxTypeFare(RuleConst::SELLING_FARE_NOT_FOR_SEC), 0, true));
  }

  void testIfSkipCat15Security_Pass_Negotiated()
  {
    TestRuleController testRuleController;
    PaxTypeFare* ptf = createPaxTypeFare(RuleConst::SELLING_FARE);
    ptf->status().set(PaxTypeFare::PTF_Negotiated);
    CPPUNIT_ASSERT(
        testRuleController.ifSkipCat15Security(*ptf, RuleConst::SALE_RESTRICTIONS_RULE, false));
  }

  void testIfSkipCat15Security_Pass_SkipCat15()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.ifSkipCat15Security(
        *createPaxTypeFare(RuleConst::SELLING_FARE), 0, false));
  }

  void testIfSkipCat15Security_Fail_NotFD()
  {
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(!testRuleController.ifSkipCat15Security(
        *createPaxTypeFare(RuleConst::SELLING_FARE_NOT_FOR_SEC), 0, false));
  }

  void testProcessCategoryPurFprShopping_Skip_Eligibility()
  {
    FakeRuleControllerDataAccess* da = createRuleControllerDataAccess(createPaxTypeFare());
    TestRuleController testRuleController;
    da->_ptf->ticketedFareForAxess() = true;
    CPPUNIT_ASSERT_EQUAL(SKIP,
                         testRuleController.processCategoryPurFprShopping(
                             *da->_trx, *da, *da->_ptf, RuleConst::ELIGIBILITY_RULE, false));
  }

  void testProcessCategoryPurFprShopping_Skip_NoRuleDataDiscounted()
  {
    FakeRuleControllerDataAccess* da =
        createRuleControllerDataAccess(createPaxTypeFare(PaxTypeFare::PTF_Discounted));
    TestRuleController testRuleController;
    CPPUNIT_ASSERT_EQUAL(SKIP,
                         testRuleController.processCategoryPurFprShopping(
                             *da->_trx, *da, *da->_ptf, RuleConst::CHILDREN_DISCOUNT_RULE, false));
  }

  void testProcessCategoryPurFprShopping_Skip_FareByRule()
  {
    FakeRuleControllerDataAccess* da =
        createRuleControllerDataAccess(createPaxTypeFareSoftPassed(RuleConst::FARE_BY_RULE, false));
    TestRuleController testRuleController;
    CPPUNIT_ASSERT_EQUAL(SKIP,
                         testRuleController.processCategoryPurFprShopping(
                             *da->_trx, *da, *da->_ptf, RuleConst::FARE_BY_RULE, false));
  }

  void testProcessCategoryPurFprShopping_Skip_Negotiated()
  {
    FakeRuleControllerDataAccess* da = createRuleControllerDataAccess(
        createPaxTypeFareSoftPassed(RuleConst::NEGOTIATED_RULE, false));
    TestRuleController testRuleController;
    CPPUNIT_ASSERT_EQUAL(SKIP,
                         testRuleController.processCategoryPurFprShopping(
                             *da->_trx, *da, *da->_ptf, RuleConst::NEGOTIATED_RULE, false));
  }

  void testReValidDiscQualifiers_Pass()
  {
    FakeRuleControllerDataAccess* da = createRuleControllerDataAccess(createPaxTypeFare());
    TestRuleController testRuleController;
    CPPUNIT_ASSERT_EQUAL(PASS, testRuleController.reValidDiscQualifiers(*da->_ptf, 0, *da));
  }

  void testValidate_Pass()
  {
    FakeRuleControllerDataAccess* da = createRuleControllerDataAccess(createPaxTypeFare());
    TestRuleController testRuleController;
    CPPUNIT_ASSERT(testRuleController.validate(*da->_trx, da->_itin, *da->_ptf));
  }

  void testGetBaseOrPaxTypeFare_NotRetrieve()
  {
    FakeRuleControllerDataAccess* da = createRuleControllerDataAccess(createPaxTypeFare());
    PaxTypeFare* ptf = createPaxTypeFare();
    CPPUNIT_ASSERT_EQUAL(ptf, &(da->getBaseOrPaxTypeFare(*ptf)));
  }

  void testGetBaseOrPaxTypeFare_NotFareByRule()
  {
    FakeRuleControllerDataAccess* da = createRuleControllerDataAccess(createPaxTypeFare());
    da->retrieveBaseFare(true);
    PaxTypeFare* ptf = createPaxTypeFare();
    CPPUNIT_ASSERT_EQUAL(ptf, &(da->getBaseOrPaxTypeFare(*ptf)));
  }

  void testGetBaseOrPaxTypeFare_NoFBRData()
  {
    FakeRuleControllerDataAccess* da = createRuleControllerDataAccess(createPaxTypeFare());
    da->retrieveBaseFare(true);
    PaxTypeFare* ptf = createPaxTypeFare(PaxTypeFare::PTF_FareByRule);
    CPPUNIT_ASSERT_EQUAL(ptf, &(da->getBaseOrPaxTypeFare(*ptf)));
  }

  void testCurrentPU()
  {
    RuleControllerDataAccess* da = createRuleControllerDataAccess(createPaxTypeFare());
    CPPUNIT_ASSERT_EQUAL((PricingUnit*)NULL, da->currentPU());
  }

  void testCurrentFU()
  {
    RuleControllerDataAccess* da = createRuleControllerDataAccess(createPaxTypeFare());
    da->cloneFU(0);
    CPPUNIT_ASSERT_EQUAL((FareUsage*)NULL, da->currentFU());
  }

  void testProcessRuleResult()
  {
    RuleControllerDataAccess* da = createRuleControllerDataAccess(createPaxTypeFare());
    Record3ReturnTypes ruleRet;
    unsigned int ruleIndex;
    CPPUNIT_ASSERT(!da->processRuleResult(ruleRet, ruleIndex));
  }

  void testFootNoteTbl()
  {
    RuleControllerDataAccess* da = createRuleControllerDataAccess(createPaxTypeFare());
    da->footNoteTbl().push_back(*_memHandle(new FootnoteTable));
    CPPUNIT_ASSERT_EQUAL((size_t)1, da->footNoteTbl().size());
  }

  void testDoNotReuseGrFareRuleResult_True_Cat1_Web()
  {
    bool chkPsgEligibility = true;
    bool skipCat15Security = false;
    uint16_t categoryNumber = 0;

    PricingTrx trx;
    PricingOptions opt;
    opt.web() = 'Y';
    trx.setOptions(&opt);
    CPPUNIT_ASSERT(RuleController::doNotReuseGrFareRuleResult(
        trx, categoryNumber, chkPsgEligibility, skipCat15Security));
  }

  void testDoNotReuseGrFareRuleResult_True_Cat15_Skip_Secure()
  {
    bool chkPsgEligibility = false;
    bool skipCat15Security = true;
    uint16_t categoryNumber = RuleConst::SALE_RESTRICTIONS_RULE;

    PricingTrx trx;
    PricingOptions opt;
    trx.setOptions(&opt);
    CPPUNIT_ASSERT(RuleController::doNotReuseGrFareRuleResult(
        trx, categoryNumber, chkPsgEligibility, skipCat15Security));
  }

  void testDoNotReuseGrFareRuleResult_True_Cat18()
  {
    bool chkPsgEligibility = false;
    bool skipCat15Security = false;
    uint16_t categoryNumber = RuleConst::TICKET_ENDORSMENT_RULE;

    PricingTrx trx;
    PricingOptions opt;
    trx.setOptions(&opt);
    CPPUNIT_ASSERT(RuleController::doNotReuseGrFareRuleResult(
        trx, categoryNumber, chkPsgEligibility, skipCat15Security));
  }

  void testDoNotReuseGrFareRuleResult_False()
  {
    bool chkPsgEligibility = false;
    bool skipCat15Security = false;
    uint16_t categoryNumber = 0;

    PricingTrx trx;
    PricingOptions opt;
    trx.setOptions(&opt);
    CPPUNIT_ASSERT(!RuleController::doNotReuseGrFareRuleResult(
                       trx, categoryNumber, chkPsgEligibility, skipCat15Security));
  }

  void testUpdateBaseFareRuleStatDiscounted_NotUpdated_NotValid_ForFBR()
  {
    PricingTrx* trx = createPricingTrx();
    uint16_t cat = RuleConst::FARE_BY_RULE;
    PaxTypeFare* ptf = createPaxTypeFare(PaxTypeFare::PTF_Discounted, true);
    createDiscountedRuleData(ptf);
    ptf->status().set(PaxTypeFare::PTF_FareByRule);

    setInfoForBaseFare(ptf, cat, false);
    setInfoForDiscountedFare(ptf, cat, true);
    TestRuleController testRC;
    testRC.updateBaseFareRuleStatDiscounted(ptf, *trx, cat);
    PaxTypeFare* baseF = ptf->baseFare(RuleConst::CHILDREN_DISCOUNT_RULE);
    CPPUNIT_ASSERT(!baseF->isCategoryValid(cat));
  }

  void testUpdateBaseFareRuleStatDiscounted_NotUpdated_Valid_ForFBR()
  {
    //   PricingTrx trx;
    PricingTrx* trx = createPricingTrx();
    uint16_t cat = RuleConst::FARE_BY_RULE;
    PaxTypeFare* ptf = createPaxTypeFare(PaxTypeFare::PTF_Discounted, true);
    createDiscountedRuleData(ptf);
    ptf->status().set(PaxTypeFare::PTF_FareByRule);

    setInfoForBaseFare(ptf, cat, true);
    setInfoForDiscountedFare(ptf, cat, false);
    TestRuleController testRC;
    testRC.updateBaseFareRuleStatDiscounted(ptf, *trx, cat);
    PaxTypeFare* baseFr = ptf->baseFare(RuleConst::CHILDREN_DISCOUNT_RULE);
    CPPUNIT_ASSERT(baseFr->isCategoryValid(cat));
  }

  void testUpdateBaseFareRuleStatDiscounted_NotUpdated_NotValid_ForCat35()
  {
    PricingTrx* trx = createPricingTrx();
    uint16_t cat = RuleConst::NEGOTIATED_RULE;
    PaxTypeFare* ptf = createPaxTypeFare(PaxTypeFare::PTF_Discounted, true);
    createDiscountedRuleData(ptf);
    ptf->status().set(PaxTypeFare::PTF_Negotiated);

    setInfoForBaseFare(ptf, cat, false);
    setInfoForDiscountedFare(ptf, cat, true);
    TestRuleController testRC;
    testRC.updateBaseFareRuleStatDiscounted(ptf, *trx, cat);
    PaxTypeFare* baseF = ptf->baseFare(RuleConst::CHILDREN_DISCOUNT_RULE);
    CPPUNIT_ASSERT(!baseF->isCategoryValid(cat));
  }

  void testUpdateBaseFareRuleStatDiscounted_NotUpdated_Valid_ForCat35()
  {
    PricingTrx* trx = createPricingTrx();
    uint16_t cat = RuleConst::NEGOTIATED_RULE;
    PaxTypeFare* ptf = createPaxTypeFare(PaxTypeFare::PTF_Discounted, true);
    createDiscountedRuleData(ptf);
    ptf->status().set(PaxTypeFare::PTF_Negotiated);

    setInfoForBaseFare(ptf, cat, true);
    setInfoForDiscountedFare(ptf, cat, false);
    TestRuleController testRC;
    testRC.updateBaseFareRuleStatDiscounted(ptf, *trx, cat);
    PaxTypeFare* baseFr = ptf->baseFare(RuleConst::CHILDREN_DISCOUNT_RULE);
    CPPUNIT_ASSERT(baseFr->isCategoryValid(cat));
  }
  void testApplySystemDefaultAssumptionCat15_Populate_Validating_Carriers()
  {
    FakeRuleControllerDataAccess* da = createRuleControllerDataAccess(createPaxTypeFare());
    TestRuleController testRuleController;
    uint16_t cat = RuleConst::SALE_RESTRICTIONS_RULE;
    PricingTrx* trx = createPricingTrx();
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->validatingCarriers().push_back("AA");
    fm->validatingCarriers().push_back("DL");
    da->_ptf->validatingCarriers().push_back("AA");
    da->_ptf->fareMarket() = fm;
    trx->setValidatingCxrGsaApplicable(true);
    bool displayDiag = false;
    CPPUNIT_ASSERT_EQUAL(SKIP, testRuleController.applySystemDefaultAssumption(*trx, *da, cat , displayDiag));
    CPPUNIT_ASSERT(da->_ptf->validatingCarriers()[0] == "AA");
  }
  void testStoreSalesRestrictionValuesFn()
  {
    PaxTypeFare* ptfOrigin = createPaxTypeFare();
    ptfOrigin->setCat15HasT996FT(true);
    PaxTypeFare* ptf = createPaxTypeFare();
    FareMarket::FareMarketSavedFnResult::Result savedRet;
    TestRuleController testRuleController;
    savedRet._latestTktDT = DateTime(2014, 12, 18, 23, 59, 0);
    savedRet._ptfResultFrom = ptfOrigin;
    testRuleController.storeSalesRestrictionValuesFn_deprecated(*createPricingTrx(), *ptf, RuleConst::SALE_RESTRICTIONS_RULE, &savedRet);
    CPPUNIT_ASSERT(ptf->isCat15HasT996FT());
   }
  void testStoreSalesRestrictionValuesGr()
  {
     PaxTypeFare* ptfOrigin = createPaxTypeFare();
     ptfOrigin->setCat15HasT996GR(true);
     PaxTypeFare* ptf = createPaxTypeFare();
     FareMarket::FareMarketSavedGfrResult::Result savedRet;
     TestRuleController testRuleController;
     savedRet._latestTktDT = DateTime(2014, 12, 18, 23, 59, 0);
     savedRet._ptfResultFrom = ptfOrigin;
     testRuleController.storeSalesRestrictionValuesGr(*createPricingTrx(), *ptf, RuleConst::SALE_RESTRICTIONS_RULE, &savedRet);
     CPPUNIT_ASSERT(ptf->isCat15HasT996GR());
  }
  void testStoreResultsFr()
  {
     PaxTypeFare* ptfOrigin = createPaxTypeFare();
     RuleControllerParam rcParam;
     ptfOrigin->setCat15HasT996FR(true);
     PaxTypeFare* ptf = createPaxTypeFare();
     FareMarket::FareMarketSavedGfrResult::Result savedRet;
     TestRuleController testRuleController;
     savedRet._latestTktDT = DateTime(2014, 12, 18, 23, 59, 0);
     savedRet._ptfResultFrom = ptfOrigin;
     testRuleController.storeResultsFr(*createPricingTrx(), *ptf, RuleConst::SALE_RESTRICTIONS_RULE, &savedRet, rcParam);
     CPPUNIT_ASSERT(ptf->isCat15HasT996FR());
  }
};
// class
CPPUNIT_TEST_SUITE_REGISTRATION(RuleControllerTest);
} // namespace
