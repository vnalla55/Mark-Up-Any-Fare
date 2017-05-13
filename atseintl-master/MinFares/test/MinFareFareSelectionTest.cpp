//----------------------------------------------------------------------------
//  Copyright Sabre 2007, 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "MinFares/test/MinFareDataHandleTest.h"

#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "MinFares/MinFareFareSelection.h"
#include "Rules/RuleConst.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestDiscountInfoFactory.h"
#include "test/testdata/TestPaxTypeFareFactory.h"

namespace tse
{
class MinFareFareSelectionForTest : public MinFareFareSelection
{
public:
  friend class MinFareFareSelectionTest;

  MinFareFareSelectionForTest(MinimumFareModule module,
                              EligibleFare eligibleFare,
                              FareDirectionChoice fareDirection,
                              PricingTrx& trx,
                              const Itin& itin,
                              const std::vector<TravelSeg*>& travelSegs,
                              const std::vector<PricingUnit*>& pricingUnits,
                              const PaxType* paxType,
                              DateTime travelDate,
                              const FarePath* farePath = 0,
                              const PaxTypeFare* thruFare = 0,
                              const MinFareAppl* minFareAppl = 0,
                              const MinFareDefaultLogic* minFareDefaultLogic = 0,
                              const RepricingTrx* repricingTrx = 0,
                              const PaxTypeCode& actualPaxType = "")
    : MinFareFareSelection(module,
                           eligibleFare,
                           fareDirection,
                           trx,
                           itin,
                           travelSegs,
                           pricingUnits,
                           paxType,
                           travelDate,
                           farePath,
                           thruFare,
                           minFareAppl,
                           minFareDefaultLogic,
                           repricingTrx,
                           actualPaxType)
  {
    _validFare.setRoutingMapValid(true);
    _notValidFareForFlightApplicationRule.setRoutingProcessed(false);
    _notValidFareForSeasonalRule.setRoutingProcessed(false);
    _notValidFareForDayTimeRule.setRoutingProcessed(false);
  }

protected:
  bool ruleValidated(const std::vector<uint16_t>& categories,
                     const PaxTypeFare& paxTypeFare,
                     bool puScope)
  {
    std::vector<uint16_t>::const_iterator iter;
    uint16_t category;

    if (puScope)
    {
      for (iter = categories.begin(); iter != categories.end(); ++iter)
      {
        category = *iter;
        if (RuleConst::SEASONAL_RULE == category && &_notValidFareForSeasonalRule == &paxTypeFare)
          return false;
        else if (RuleConst::DAY_TIME_RULE == category &&
                 &_notValidFareForDayTimeRule == &paxTypeFare)
          return false;
      }
    }
    else
    {
      for (iter = categories.begin(); iter != categories.end(); ++iter)
      {
        category = *iter;
        if (RuleConst::FLIGHT_APPLICATION_RULE == category &&
            &_notValidFareForFlightApplicationRule == &paxTypeFare)
          return false;
      }
    }
    return true;
  }

  void displayMinFareApplLogic() {}
  void getDefaultLogic() {}
  void getOverrideLogic() {}

  const PaxTypeFare*
  selectNormalFareForConstruction(const std::vector<TravelSeg*>& travelSegs,
                                  PaxTypeStatus selPaxTypeStatus = PAX_TYPE_STATUS_UNKNOWN,
                                  bool selectNextCabin = false)
  {
    return 0;
  }

  PaxTypeFare _validFare;
  PaxTypeFare _notValidFareForFlightApplicationRule;
  PaxTypeFare _notValidFareForSeasonalRule;
  PaxTypeFare _notValidFareForDayTimeRule;
};

class MinFareFareSelectionRtg : public MinFareFareSelection
{

public:
  MinFareFareSelectionRtg(MinimumFareModule module,
                          EligibleFare eligibleFare,
                          FareDirectionChoice fareDirection,
                          PricingTrx& trx,
                          const Itin& itin,
                          const std::vector<TravelSeg*>& travelSegs,
                          const std::vector<PricingUnit*>& pricingUnits,
                          const PaxType* paxType,
                          DateTime travelDate,
                          const FarePath* farePath = 0,
                          const PaxTypeFare* thruFare = 0,
                          const MinFareAppl* minFareAppl = 0,
                          const MinFareDefaultLogic* minFareDefaultLogic = 0,
                          const RepricingTrx* repricingTrx = 0,
                          const PaxTypeCode& actualPaxType = "")
    : MinFareFareSelection(module,
                           eligibleFare,
                           fareDirection,
                           trx,
                           itin,
                           travelSegs,
                           pricingUnits,
                           paxType,
                           travelDate,
                           farePath,
                           thruFare,
                           minFareAppl,
                           minFareDefaultLogic,
                           repricingTrx,
                           actualPaxType),
      _rtgFare()
  {
    _module = HIP;
    _rtgFare.setRoutingProcessed(true);
    _rtgFare.setRoutingValid(false);
  }

  ValidStatus validateFareAccess() { return validateFare(_rtgFare); }

  bool ruleValidated(const std::vector<uint16_t>& categories,
                     const PaxTypeFare& paxTypeFare,
                     bool puScope = false)
  {
    if (paxTypeFare.isCategoryValid(1))
      return true;
    else
      return false;
  }

  void getDefaultLogic() {};
  void getOverrideLogic() {};
  void displayMinFareApplLogic() {};

  const PaxTypeFare*
  selectNormalFareForConstruction(const std::vector<TravelSeg*>& travelSegs,
                                  PaxTypeStatus selPaxTypeStatus = PAX_TYPE_STATUS_UNKNOWN,
                                  bool selectNextCabin = false)
  {
    return 0;
  }

  PaxTypeFare _rtgFare;
};

class MinFareFareSelectionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MinFareFareSelectionTest);
  CPPUNIT_TEST(testValidateValidFare);
  CPPUNIT_TEST(testValidateNonValidFareForFlightApplicationRule);
  CPPUNIT_TEST(testValidateNonValidFareForSeasonalRule);
  CPPUNIT_TEST(testValidateNonValidFareForDayTimeRule);
  CPPUNIT_TEST(testValidFlag);
  CPPUNIT_TEST(testInvalidFlag);
  CPPUNIT_TEST(testInvalidPrevalidation);
  CPPUNIT_TEST(test_failCheckRuleLevelExclusionComparisonWhenNoPointer);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_HIP);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_HIP_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_BHC);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_BHC_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_CTM);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_CTM_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_COM);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_COM_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_DMC);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_DMC_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_COP);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_COP_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_OSC);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_OSC_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_RSC);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_RSC_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_CPM);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_CPM_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_HIP_COMPARE_NO);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_BHC_COMPARE_NO);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_CTM_COMPARE_NO);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_HIP_SameFareGroup_PERMITTED);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_BHC_SameFareGroup_PERMITTED);
  CPPUNIT_TEST(test_checkRuleLevelExclusionComparison_CTM_SameFareGroup_PERMITTED);
  CPPUNIT_TEST(test_skipCat15ValidationforDiscountedFare_HIP);
  CPPUNIT_TEST(test_skipCat15ValidationforDiscountedFare_CTM);
  CPPUNIT_TEST(test_ThroughPublicFareCheckPrivateFareForHIPAndBHC);

  CPPUNIT_TEST_SUITE_END();

public:
  PricingTrx* _trx;
  MinFareFareSelectionForTest* _fareSel;
  MinFareFareSelectionRtg* _minFareFareSelectionRtgp;
  MinFareRuleLevelExcl ruleLevelExcl;
  MatchRuleLevelExclTable* matchRule;
  PaxType paxType;
  Itin itin;
  std::vector<TravelSeg*> travelSegs;
  std::vector<PricingUnit*> pricingUnits;
  PaxTypeFare paxTypeFare;
  DateTime tvlDate;
  MinimumFareModule module;
  TestMemHandle _memHandle;

  MinFareFareSelectionTest() : _fareSel(0), _minFareFareSelectionRtgp(0) {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MinFareDataHandleTest>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _fareSel = _memHandle.insert(new MinFareFareSelectionForTest(HIP,
                                                                 MinFareFareSelection::ONE_WAY,
                                                                 MinFareFareSelection::OUTBOUND,
                                                                 *_trx,
                                                                 itin,
                                                                 travelSegs,
                                                                 pricingUnits,
                                                                 &paxType,
                                                                 DateTime::emptyDate()));

    _minFareFareSelectionRtgp =
        _memHandle.insert(new MinFareFareSelectionRtg(HIP,
                                                      MinFareFareSelection::ONE_WAY,
                                                      MinFareFareSelection::OUTBOUND,
                                                      *_trx,
                                                      itin,
                                                      travelSegs,
                                                      pricingUnits,
                                                      &paxType,
                                                      DateTime::emptyDate()));

    matchRule = _memHandle.insert(
        new MatchRuleLevelExclTable(module, *_trx, itin, paxTypeFare, tvlDate, true));
  }

  void tearDown() { _memHandle.clear(); }

  void assertValidate(const PaxTypeFare& fare, MinFareFareSelection::ValidStatus expected)
  {
    _fareSel->_module = HIP;
    CPPUNIT_ASSERT_EQUAL(expected, _fareSel->validateFare(fare));

    _fareSel->_module = BHC;
    CPPUNIT_ASSERT_EQUAL(expected, _fareSel->validateFare(fare));

    _fareSel->_module = CPM;
    CPPUNIT_ASSERT_EQUAL(expected, _fareSel->validateFare(fare));
  }

  void testValidateValidFare()
  {
    assertValidate(_fareSel->_validFare, MinFareFareSelection::VALIDATED);
  }

  void testValidateNonValidFareForFlightApplicationRule()
  {
    assertValidate(_fareSel->_notValidFareForFlightApplicationRule, MinFareFareSelection::NONE);
  }

  void testValidateNonValidFareForSeasonalRule()
  {
    assertValidate(_fareSel->_notValidFareForSeasonalRule, MinFareFareSelection::NONE);
  }

  void testValidateNonValidFareForDayTimeRule()
  {
    assertValidate(_fareSel->_notValidFareForDayTimeRule, MinFareFareSelection::NONE);
  }

  void testValidFlag()
  {
    _minFareFareSelectionRtgp->_rtgFare.setRoutingMapValid(true);

    CPPUNIT_ASSERT(_minFareFareSelectionRtgp->validateFareAccess() ==
                   MinFareFareSelection::VALIDATED);
  }

  void testInvalidFlag()
  {
    _minFareFareSelectionRtgp->_rtgFare.setRoutingMapValid(false);

    CPPUNIT_ASSERT(_minFareFareSelectionRtgp->validateFareAccess() !=
                   MinFareFareSelection::VALIDATED);
  }

  void testInvalidPrevalidation()
  {
    _minFareFareSelectionRtgp->_rtgFare.setRoutingMapValid(true);
    _minFareFareSelectionRtgp->_rtgFare.setCategoryValid(1, false);

    CPPUNIT_ASSERT(_minFareFareSelectionRtgp->validateFareAccess() !=
                   MinFareFareSelection::VALIDATED);
  }

  void test_failCheckRuleLevelExclusionComparisonWhenNoPointer()
  {
    const MinFareRuleLevelExcl* ruleLevelExcl = 0;
    CPPUNIT_ASSERT(
        !_fareSel->checkRuleLevelExclusionComparison(paxTypeFare, ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_HIP()
  {
    _fareSel->module() = HIP;
    ruleLevelExcl.hipFareCompAppl() = 'P';
    CPPUNIT_ASSERT(
        _fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_HIP_Compare_YES()
  {
    _fareSel->module() = HIP;
    ruleLevelExcl.hipFareCompAppl() = 'Y';
    CPPUNIT_ASSERT(
        !_fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_BHC()
  {
    _fareSel->module() = BHC;
    ruleLevelExcl.backhaulFareCompAppl() = 'P';
    CPPUNIT_ASSERT(
        _fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_BHC_Compare_YES()
  {
    _fareSel->module() = BHC;
    ruleLevelExcl.backhaulFareCompAppl() = 'Y';
    CPPUNIT_ASSERT(
        !_fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_CTM()
  {
    _fareSel->module() = CTM;
    ruleLevelExcl.ctmFareCompAppl() = 'P';
    CPPUNIT_ASSERT(
        _fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_CTM_Compare_YES()
  {
    _fareSel->module() = CTM;
    ruleLevelExcl.ctmFareCompAppl() = 'Y';
    CPPUNIT_ASSERT(
        !_fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_COM()
  {
    _fareSel->module() = COM;
    ruleLevelExcl.comFareCompAppl() = 'P';
    CPPUNIT_ASSERT(
        _fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_COM_Compare_YES()
  {
    _fareSel->module() = COM;
    ruleLevelExcl.comFareCompAppl() = 'Y';
    CPPUNIT_ASSERT(
        !_fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_DMC()
  {
    _fareSel->module() = DMC;
    ruleLevelExcl.dmcFareCompAppl() = 'P';
    CPPUNIT_ASSERT(
        _fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_DMC_Compare_YES()
  {
    _fareSel->module() = DMC;
    ruleLevelExcl.dmcFareCompAppl() = 'Y';
    CPPUNIT_ASSERT(
        !_fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_COP()
  {
    _fareSel->module() = COP;
    ruleLevelExcl.copFareCompAppl() = 'P';
    CPPUNIT_ASSERT(
        _fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_COP_Compare_YES()
  {
    _fareSel->module() = COP;
    ruleLevelExcl.copFareCompAppl() = 'Y';
    CPPUNIT_ASSERT(
        !_fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_OSC()
  {
    _fareSel->module() = OSC;
    ruleLevelExcl.oscFareCompAppl() = 'P';
    CPPUNIT_ASSERT(
        _fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_OSC_Compare_YES()
  {
    _fareSel->module() = OSC;
    ruleLevelExcl.oscFareCompAppl() = 'Y';
    CPPUNIT_ASSERT(
        !_fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_RSC()
  {
    _fareSel->module() = RSC;
    ruleLevelExcl.rscFareCompAppl() = 'P';
    CPPUNIT_ASSERT(
        _fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_RSC_Compare_YES()
  {
    _fareSel->module() = RSC;
    ruleLevelExcl.rscFareCompAppl() = 'Y';
    CPPUNIT_ASSERT(
        !_fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_CPM()
  {
    _fareSel->module() = CPM;
    ruleLevelExcl.cpmFareCompAppl() = 'P';
    CPPUNIT_ASSERT(
        _fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_CPM_Compare_YES()
  {
    _fareSel->module() = CPM;
    ruleLevelExcl.cpmFareCompAppl() = 'Y';
    CPPUNIT_ASSERT(
        !_fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_HIP_COMPARE_NO()
  {
    _fareSel->module() = HIP;
    ruleLevelExcl.hipFareCompAppl() = 'N';
    ruleLevelExcl.hipSameGroupAppl() = 'N'; // if N return true if P check same fare group logic
    CPPUNIT_ASSERT(
        _fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_BHC_COMPARE_NO()
  {
    _fareSel->module() = BHC;
    ruleLevelExcl.backhaulFareCompAppl() = 'N';
    ruleLevelExcl.backhaulSameGroupAppl() = 'N'; // if N return true if P check same fare group
                                                 // logic
    CPPUNIT_ASSERT(
        _fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_CTM_COMPARE_NO()
  {
    _fareSel->module() = CTM;
    ruleLevelExcl.ctmFareCompAppl() = 'N';
    ruleLevelExcl.ctmSameGroupAppl() = 'N'; // if N return true if P check same fare group logic
    CPPUNIT_ASSERT(
        _fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_HIP_SameFareGroup_PERMITTED()
  {
    _fareSel->module() = HIP;
    ruleLevelExcl.hipFareCompAppl() = 'Y';
    ruleLevelExcl.hipSameGroupAppl() = 'P'; // if P return false as Compare is Y
    CPPUNIT_ASSERT(
        !_fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_BHC_SameFareGroup_PERMITTED()
  {
    _fareSel->module() = BHC;
    ruleLevelExcl.backhaulFareCompAppl() = 'Y';
    ruleLevelExcl.backhaulSameGroupAppl() = 'P'; // if P return false as Compare is Y
    CPPUNIT_ASSERT(
        !_fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_checkRuleLevelExclusionComparison_CTM_SameFareGroup_PERMITTED()
  {
    _fareSel->module() = CTM;
    ruleLevelExcl.ctmFareCompAppl() = 'Y';
    ruleLevelExcl.ctmSameGroupAppl() = 'P'; // if P return false as Compare is Y
    CPPUNIT_ASSERT(
        !_fareSel->checkRuleLevelExclusionComparison(paxTypeFare, &ruleLevelExcl, *matchRule));
  }

  void test_skipCat15ValidationforDiscountedFare_HIP()
  {
    _fareSel->module() = HIP;
    DiscountInfo* discountInfo =
        TestDiscountInfoFactory::create("/vobs/atseintl/MinFares/test/testdata/DiscountInfo.xml");
    PaxTypeFare* fare =
        TestPaxTypeFareFactory::create("/vobs/atseintl/MinFares/test/testdata/PaxTypeFare.xml");
    fare->paxTypeFareRuleData(19)->ruleItemInfo() = discountInfo;
    CPPUNIT_ASSERT_EQUAL(MinFareFareSelection::VALIDATED, _fareSel->validateFare(*fare));
  }

  void test_ThroughPublicFareCheckPrivateFareForHIPAndBHC()
  {
    PaxTypeFare dummyThruFare;
    _fareSel->_thruFare = &dummyThruFare;
    PaxTypeFare dummyIntermediateMarketPrivateFare;
    dummyIntermediateMarketPrivateFare.setTcrTariffCatPrivate();
    CPPUNIT_ASSERT(dummyThruFare.tcrTariffCat() == RuleConst::PUBLIC_TARIFF);
    CPPUNIT_ASSERT(dummyIntermediateMarketPrivateFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF);
    PaxTypeFare dummyIntermediateMarketPublicFare;
    CPPUNIT_ASSERT(dummyIntermediateMarketPublicFare.tcrTariffCat() == RuleConst::PUBLIC_TARIFF);
    _fareSel->module() = HIP;
    CPPUNIT_ASSERT_EQUAL(MinFareFareSelection::NONE,
        _fareSel->validateFare(dummyIntermediateMarketPrivateFare));
    CPPUNIT_ASSERT_EQUAL(MinFareFareSelection::VALIDATED,
        _fareSel->validateFare(dummyIntermediateMarketPublicFare));
    _fareSel->module() = BHC;
    CPPUNIT_ASSERT_EQUAL(MinFareFareSelection::NONE,
        _fareSel->validateFare(dummyIntermediateMarketPrivateFare));
    CPPUNIT_ASSERT_EQUAL(MinFareFareSelection::VALIDATED,
        _fareSel->validateFare(dummyIntermediateMarketPublicFare));
  }

  void test_skipCat15ValidationforDiscountedFare_CTM()
  {
    _fareSel->module() = CTM;
    DiscountInfo* discountInfo =
        TestDiscountInfoFactory::create("/vobs/atseintl/MinFares/test/testdata/DiscountInfo.xml");
    PaxTypeFare* fare =
        TestPaxTypeFareFactory::create("/vobs/atseintl/MinFares/test/testdata/PaxTypeFare.xml");
    fare->paxTypeFareRuleData(19)->ruleItemInfo() = discountInfo;
    CPPUNIT_ASSERT_EQUAL(MinFareFareSelection::VALIDATED, _fareSel->validateFare(*fare));
  }

}; // test class
CPPUNIT_TEST_SUITE_REGISTRATION(MinFareFareSelectionTest);
} // namespace tse
