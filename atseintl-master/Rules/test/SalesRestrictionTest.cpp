#include "Common/Config/DynamicConfigLoader.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/Trx.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/Nation.h"
#include "DBAccess/SalesRestriction.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/SalesRestrictionRule.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestFactoryManager.h"
#include "test/testdata/TestLocFactory.h"
#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include <iostream>
#include <set>
#include <time.h>
#include <vector>

using namespace boost::assign;

namespace
{
class PaxTypeFareMock : public tse::PaxTypeFare
{
public:
  PaxTypeFareMock() : PaxTypeFare() {}

  void setCarrier(const tse::CarrierCode& cxr) { _carrier = cxr; }
};
}

namespace tse
{
FALLBACKVALUE_DECL(fallbackValidatingCxrMultiSp);

const std::string LOC_DFW = "/vobs/atseintl/test/testdata/data/LocDFW.xml";
const std::string LOC_DEN = "/vobs/atseintl/test/testdata/data/LocDEN.xml";
const std::string LOC_CHI = "/vobs/atseintl/test/testdata/data/LocCHI.xml";
const std::string LOC_NYC1 = "/vobs/atseintl/test/testdata/data/LocNYC.xml";
const std::string LOC_LON = "/vobs/atseintl/test/testdata/data/LocLON.xml";
const std::string LOC_FRA = "/vobs/atseintl/test/testdata/data/LocFRA.xml";
const std::string LOC_PAR = "/vobs/atseintl/test/testdata/data/LocPAR.xml";

const std::string carrier_AA = "AA";
const std::string carrier_UA = "UA";
const std::string carrier_LH = "LH";
const std::string carrier_BA = "BA";

const std::string city_FRA = "FRA";
const std::string city_LON = "LON";
const std::string city_NYC = "NYC";
const std::string city_KRK = "KRK";

const std::string agency_B4T0 = "B4T0";
const CurrencyCode GBP = "GBP";
const CurrencyCode EUR = "EUR";

class SalesRestrictionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SalesRestrictionTest);
  CPPUNIT_TEST(testValidateSelectedNetRemitFareFailWhenSaleRestrictionDataIsUnAvailable);
  CPPUNIT_TEST(testValidateSelectedNetRemitFareSkipWhenSaleRestrictedToTextOnly);
  CPPUNIT_TEST(testValidateSelectedNetRemitFarePassWhenNoUnavailTagRestrictions);
  CPPUNIT_TEST(testValidateFailWhenSalesRestrictionDataIsUnavailable);
  CPPUNIT_TEST(testValidateFailWhenSalesRestrictedToTextOnly);
  CPPUNIT_TEST(testValidateSkipWhenMustBeSoldViaCRSAndIsQualified);
  CPPUNIT_TEST(testValidateSkipWhenMustBeSoldViaCRSAndIsNotQualified);
  CPPUNIT_TEST(testCheckCountryRestPURetrunTrueWhenAgentCountryEqualsOriginCountry);
  CPPUNIT_TEST(testCheckCountryRestPUFailWhenAgentLocationNotEqualDestintionCountry);
  CPPUNIT_TEST(testCheckCountryRestPUFailWhenSalePointOverrideNotEqualOriginCountry);
  CPPUNIT_TEST(testCheckSecurityRestrictionFailWhenTACantSell);
  CPPUNIT_TEST(testIsWebFareFaileWhenNonCheckedAgencyAndTravelocity);
  CPPUNIT_TEST(testIsWebFarePassWhenLoc1HomeTAAndTravelocity);
  CPPUNIT_TEST(testIsWebFarePassWhenLoc2HomeTAAndTravelocity);
  CPPUNIT_TEST(testCheckCountryRestrictionPassWhenNoRestriction);
  CPPUNIT_TEST(testCheckCountryRestrictionFailWhenNoOverrideOfRestriction);
  CPPUNIT_TEST(testCheckCountryRestrictionPassWhenOverrideOfRestriction);
  CPPUNIT_TEST(testCheckCountryRestrictionFailWhenIncorrectOverrideOfRestriction);
  CPPUNIT_TEST(testCheckFormOfPaymentPassIfNoRestrictions);
  CPPUNIT_TEST(testCheckFormOfPaymentAlwaysPassIfNotTicketEntry);
  CPPUNIT_TEST(testCheckFormOfPaymentDoesntAllowPaymentWhenRestricted);
  CPPUNIT_TEST(testCheckCurrencyRestrictionPassNoRestrictionsForGivenCurrency);
  CPPUNIT_TEST(testCheckCurrencyRestrictionFailWhenAgentCurrencyNotSameAsRestriction);
  CPPUNIT_TEST(testCheckCurrencyRestrictionPassIfCurrencyOverrideIsValid);
  CPPUNIT_TEST(testCheckCurrencyRestPUFailIfInboundLocationCurrencyDoesntMatchOverride);
  CPPUNIT_TEST(testCheckTicketRestrictionFailWhenETicketNotAllowedAndIsTicketEntry);
  CPPUNIT_TEST(testCheckTicketRestrictionPassWhenETicketAllowedAndIsTicketEntry);
  CPPUNIT_TEST(testCheckSoldTktRestrictionPassWhenSegOriginEqualsAgentNation);
  CPPUNIT_TEST(testCheckSoldTktRestrictionFailWhenIssSitiAndSoldInsideTktInside);
  CPPUNIT_TEST(testCheckSoldTktRestrictionFailWhenIssSotoAndSoldInsideTktInside);
  CPPUNIT_TEST(testHasTvlyLocationFailIfNoAgent);
  CPPUNIT_TEST(testHasTvlyLocationFailIfNotTJRAgent);
  CPPUNIT_TEST(testHasTvlyLocationPassIfTvlyLocation);
  CPPUNIT_TEST(testCheckTicketElectronicWithTicketETicketRequired);
  CPPUNIT_TEST(testCheckTicketElectronicWithTicketETicketNotRequired);
  CPPUNIT_TEST(testCheckCarrierRestrictionPassIfNoRestrictions);
  CPPUNIT_TEST(testCheckCarrierRestrictionPassIfMipTrxAndNoFareUsage);
  CPPUNIT_TEST(testCheckCarrierRestrictionPassIfTrxTypeEqualsMIP_TRX);
  CPPUNIT_TEST(testCheckCarrierRestrictionPassWhenValidateCarrierMatchesRuleOtherCarrier);
  CPPUNIT_TEST(testCheckCarrierRestrictionFailIfNotJointCarrier);
  CPPUNIT_TEST(testCheckSaleSecurityFailWhenFareMayNotBeSoldToTA);
  CPPUNIT_TEST(testCheckSaleSecurityFailWhenFareMustBeSoldByCarrier);
  CPPUNIT_TEST(testCheckSaleSecurityPassWhenPrivateTariffAndJointCarrier);
  CPPUNIT_TEST(testCheckSaleSecurityPassWhenPrivateTariffAndNotJointCarrierPartitionIdEmpty);
  CPPUNIT_TEST(
      testCheckSaleSecurityFailWhenPrivateTariffAndNotJointCarrierPartitionIdNotMatchCarrierNotMatchOtherCarrierNotMatchGoverningCarrier);
  CPPUNIT_TEST(
      testCheckSaleSecurityFailWhenPrivateTariffAndNotJointCarrierPartitionIdNotMatchCarrierNotMatchOtherCarrierFareIsNotIndustry);
  CPPUNIT_TEST(
      testCheckSaleSecurityPassWhenPrivateTariffAndNotJointCarrierPartitionIdNotMatchCarrierNotMatchOtherCarrierGoverningCarrierNotMatchFareIsIndustry);
  CPPUNIT_TEST(
      testCheckSaleSecurityPassWhenNotPrivateTariffAndPartitionIdNotMatchCarrierAndMustBeSoldViaCarrierAndJointCarrier);
  CPPUNIT_TEST(
      testCheckSaleSecurityFailWhenNotPrivateTariffAndPartitionIdNotMatchCarrierAndMustBeSoldViaCarrierAndNotJointCarrier);
  CPPUNIT_TEST(
      testCheckSaleSecurityFailWhenNotPrivateTariffAndPartitionIdNotMatchCarrierAndNotMustBeSoldViaCarrierAndMayOnlyBeSoldByTa);
  CPPUNIT_TEST(testCheckSaleSecurityFailWhenNotPrivateTariffAnd);
  CPPUNIT_TEST(testCheckSelectSecurityPassNoRuleSet);
  CPPUNIT_TEST(testCheckSelectSecurityPassIfPrivateTarrifAndOnlyOverrideDateTblItemNoIsSet);
  CPPUNIT_TEST(testCheckSaleSecurityPassRTWPrivate);
  CPPUNIT_TEST(testCheckSaleSecurityPassRTWPrivateExact);
  CPPUNIT_TEST(testCheckSaleSecurityPassRTWPrivatePTF);
  CPPUNIT_TEST(testCheckSaleSecurityFailRTWPrivate);
  CPPUNIT_TEST(testCheckSaleSecurityPassRTWPublic);
  CPPUNIT_TEST(testCheckSaleSecurityPassRTWPublicExact);
  CPPUNIT_TEST(testCheckSaleSecurityPassRTWPublicPTF);
  CPPUNIT_TEST(testCheckSaleSecurityFailRTWPublic);
  CPPUNIT_TEST(testCheckSaleSecurityPassRTWValidatingCxr);
  CPPUNIT_TEST(testCheckSaleSecurityPassRTWValidatingCxrExact);
  CPPUNIT_TEST(testCheckSaleSecurityFailRTWValidatingCxr);
  CPPUNIT_TEST(testCheckSaleSecurityPassRTWValidatingCxrBlank);
  CPPUNIT_TEST(testCheckSaleSecurityPassRTWValidatingCxrExactBlank);
  CPPUNIT_TEST(testCheckSaleSecurityFailRTWValidatingCxrBlank);
  CPPUNIT_TEST(testCheckLocaleItemsFailWhenTktNotSoldMatchinPCCButNotOptIn);
  CPPUNIT_TEST(testCheckLocaleItemsFailWhenTktMayBeSoldMatchinPCCButNotOptIn);
  CPPUNIT_TEST(testCheckLocaleItemsFailWhenPCCNotMatchAndNoOptIn);
  CPPUNIT_TEST(testCheckLocaleItemsFail_NBET0_N88D7_YNUS);
  CPPUNIT_TEST(testCheckLocaleItemsPassWhenOptInAndPCCMatchAndPrivateTarrif);
  CPPUNIT_TEST(testCheckLocaleItemsPass_N88D7_YB4T0);
  CPPUNIT_TEST(testCheckLocaleItemsPass_N88D7_YB4T0_OptISetToNo);
  CPPUNIT_TEST(testCheckLocaleItemsPass_N88D7_YNUS);
  CPPUNIT_TEST(testCheckLocaleItemsFail_N88D7_MatchPCCFailOptIn);
  CPPUNIT_TEST(testCheckLocaleItemsPass_N88D7);
  CPPUNIT_TEST(testCheckLocaleItemsPass_YNFR);
  CPPUNIT_TEST(testCheckLocaleItemsPassWhenLocalesIsEmpty);
  CPPUNIT_TEST(testDiag355NotActive);
  CPPUNIT_TEST(testDeterminePricingCurrencyFail);
  CPPUNIT_TEST(testDeterminePricingCurrencyPassPrimeCurr);
  CPPUNIT_TEST(testDeterminePricingCurrencyPassAlternateCurr);
  CPPUNIT_TEST(testDeterminePricingCurrencyPassConversionCurr);
  CPPUNIT_TEST(testValidateCarrierWqWarnigSet);
  CPPUNIT_TEST(testValidateCarrierWqWarnigNotSet);
  CPPUNIT_TEST(testValidateCarrierPassValidatingCarrierEqualToOtherCarrier);
  CPPUNIT_TEST(testValidateCarrierPassCarrierNotJointCarrier);
  CPPUNIT_TEST(testValidateCarrierPassAllianceMatchRTW);
  CPPUNIT_TEST(testValidateCarrierFailCarrierNotJointCarrier);
  CPPUNIT_TEST(testValidateCarrierFailJointCarrier);
  CPPUNIT_TEST(testValidateCarrierPassJointCarrier);
  CPPUNIT_TEST(testValidateCarrierRestrictionSegmentWqWarningNotSet);
  CPPUNIT_TEST(testValidateCarrierRestrictionSegmentWqWarningSet);
  CPPUNIT_TEST(testValidateCarrierRestrictionSegmentPaxTypeAdtPaxTypeFareCarrierNotJointPass);
  CPPUNIT_TEST(testValidateCarrierRestrictionSegmentPaxTypeNegMatchPass);
  CPPUNIT_TEST(testValidateCarrierRestrictionSegmentIsTicketEntryPass);
  CPPUNIT_TEST(testValidateCarrierRestrictionSegmentIsTicketEntryFail);
  CPPUNIT_TEST(testValidateCarrierRestrictionSegmentPaxTypeFareJointCarrierMatchFoundPass);
  CPPUNIT_TEST(testValidateCarrierRestrictionSegmentPaxTypeFareJointCarrierMatchNotFoundPass);
  CPPUNIT_TEST(testValidateCarrierRestrictionSegmentPaxTypeFareJointCarrierMatchNotFoundFail);
  CPPUNIT_TEST(testValidateCarrierRestrictionFareWqWarningSet);
  CPPUNIT_TEST(testValidateCarrierRestrictionFareWqWarningNotSet);
  CPPUNIT_TEST(testValidateCarrierRestrictionFarePassJointCarrier);
  CPPUNIT_TEST(testValidateCarrierRestrictionFarePassNoTravelSegments);
  CPPUNIT_TEST(testValidateCarrierRestrictionFareFail);
  CPPUNIT_TEST(testDetermineCountryLocNoOverride);
  CPPUNIT_TEST(testChackCarrierMatchForJointCarrierPassWhenNoTravelSegments);
  CPPUNIT_TEST(testChackCarrierMatchForJointCarrierPassWhenCarrierMatch);
  CPPUNIT_TEST(testChackCarrierMatchForJointCarrierPassWhenCarrierNoMatchAndIsNotTicketEntry);
  CPPUNIT_TEST(testChackCarrierMatchForJointCarrierPassWhenCarrierNoMatchAndIsNotTicketEntryRTW);
  CPPUNIT_TEST(testChackCarrierMatchForJointCarrierFailWhenCarrierNoMatchAndIsTicketEntry);

  CPPUNIT_TEST(testSetFrInCat15NothingSetWhenNotCwtUser);
  CPPUNIT_TEST(testSetFrInCat15NothingSetWhenNotPrivateTariff);
  CPPUNIT_TEST(testSetFrInCat15NothingSetWhenNoMatchRestriction);
  CPPUNIT_TEST(testSetFrInCat15NothingSetWhenLocTypeNotNation);
  CPPUNIT_TEST(testSetFrInCat15SetFrWhenCwtUserPrivateTariffmatchSaleRestrloc1FrCategory15);
  CPPUNIT_TEST(testSetFrInCat15SetFrFnWhenTypeIsFootNote);
  CPPUNIT_TEST(testSetFrInCat15SetGrFnWhenTypeIsFootNote);

  CPPUNIT_TEST(testfindCorpIdMatchFailWhenLocalesEmpty);
  CPPUNIT_TEST(testfindCorpIdMatchFailWhenLocTypeIsNotTravelAgencyAndNotHomeAgency);
  CPPUNIT_TEST(testfindCorpIdMatchFailWhenLocTypeIsTravelAgencyLocNotMatchPcc);
  CPPUNIT_TEST(testfindCorpIdMatchFailWhenLocTypeIsHomeTravelAgencyLocNotMatchPcc);
  CPPUNIT_TEST(testfindCorpIdMatchPassWhenMatchedCorpIdAndCorpIdNotEmptyForLoc1);
  CPPUNIT_TEST(testfindCorpIdMatchPassWhenNoMatchedCorpIdAndCorpIdEmptyForLoc1);
  CPPUNIT_TEST(testfindCorpIdMatchPassWhenMatchedCorpIdAndCorpIdNotEmptyForLoc2);
  CPPUNIT_TEST(testfindCorpIdMatchPassWhenNoMatchedCorpIdAndCorpIdEmptyForLoc2);

  CPPUNIT_TEST(testSetFlagsPassWhenLocalesEmpty);
  CPPUNIT_TEST(testSetFlagsPassWhenMatchSaleRestrAndLocApplIsTktsMayOnlyBeSold);
  CPPUNIT_TEST(testSetFlagsPassWhenMatchSaleRestrAndLocApplIsTktsMayNotBeSold);
  CPPUNIT_TEST(testSetFlagsPassWhenMatchTktRestrAndLocApplIsTktsMayOnlyBeIssued);
  CPPUNIT_TEST(testSetFlagsPassWhenMatchTktRestrAndLocApplIsTktsMayNotBeIssued);
  CPPUNIT_TEST(testSetFlagsWqWarningSet);
  CPPUNIT_TEST(testSetFlagsMustBeSoldWhenLocApplIsTktsMayOnlyBeSoldAndNoMatchTktRestrictions);
  CPPUNIT_TEST(testSetFlagsNotMustBeSoldWhenLocApplIsTktsMayOnlyBeIssuedAndNoMatchTktRestrictions);
  CPPUNIT_TEST(testSetFlagsMustBeTktWhenLocApplIsTktsMayOnlyBeIssuedAndNoMatchTktRestrictions);
  CPPUNIT_TEST(testSetFlagsNotMustBeTktWhenLocApplIsTktsMayOnlyBeSoldAndNoMatchTktRestrictions);

  CPPUNIT_TEST(testValidateCarrierRestrictionPassWhenCarrierMatch);
  CPPUNIT_TEST(testValidateCarrierRestrictionPassWhenOtherCarrierMatch);
  CPPUNIT_TEST(testValidateCarrierRestrictionPassWhenOtherCarrierAllianceMatchRTW);
  CPPUNIT_TEST(testValidateCarrierRestrictionFailWhenNeitherCarrierMatch);

  CPPUNIT_TEST(testCheckCarrierPassWhenMatchForJointCarrier);
  CPPUNIT_TEST(testCheckCarrierFailWhenMatchForJointCarrier);
  CPPUNIT_TEST(testCheckCarrierPassWhenMatchForNotJointCarrier);

  CPPUNIT_TEST(testSetTrailerMsg);
  CPPUNIT_TEST(testSetTrailerMsg_FareUsage);
  CPPUNIT_TEST(testCheckPreconditionsPass);
  CPPUNIT_TEST(testCheckPreconditionsFailWhenDataUnavailable);
  CPPUNIT_TEST(testCheckPreconditionsFailReasonSetToDataUnavailableWhenDataUnavailable);
  CPPUNIT_TEST(testCheckPreconditionsFailWhenTextOnly);
  CPPUNIT_TEST(testCheckPreconditionsFailReasonSetToTextOnlyWhenTextOnly);
  CPPUNIT_TEST(testCheckPreconditionsFailWhenNoSkipCat15Security);
  CPPUNIT_TEST(testCheckPreconditionsFailReasonCrsOtherCarrierWhenSkipCat15Security);
  CPPUNIT_TEST(testCheckSecurityDataInMainCat15SetCat15HasSecurity);
  CPPUNIT_TEST(
      testCheckSecurityDataInMainCat15DontSetCat15HasSecurityWhenLocalesIsEmptyAndCarrierIndAndTvlIndAreBlank);
  CPPUNIT_TEST(testCheckSecurityDataInMainCat15DontSetCat15HasSecurityWhenIsQualifiedCategory);
  CPPUNIT_TEST(
      testCheckSecurityDataInMainCat15SetSecurityInCat15FnWhenCategoryRuleInfoIsFootNoteCtrlInfo);
  CPPUNIT_TEST(
      testCheckSecurityDataInMainCat15SetCat15GeneralRuleProcessHasSecurityWhenIsQualifiedCategory);
  CPPUNIT_TEST(testCheckSecurityDataInMainCat15SetCat15FRWhenIsNotFootNoteAndNotGeneralRule);
  CPPUNIT_TEST(testCheckPrivateTariffCarrierRestrictionsPassWhenNotPrivateTariff);
  CPPUNIT_TEST(testCheckPrivateTariffCarrierRestrictionsFailWhenPrivateTariff);
  CPPUNIT_TEST(
      testCheckPrivateTariffCarrierRestrictionsSetFailReasonToPrivateSecurityWhenPrivateTariff);
  CPPUNIT_TEST(testCheckSecurityPassWhenSkipCat15SecurityCheck);
  CPPUNIT_TEST(testCheckSecuritylocaleFailedWhenCheckLocaleItemsFailed);
  CPPUNIT_TEST(testCheckSecurityFailReasonSecurityWhenSecurityRestrictionFail);
  CPPUNIT_TEST(testCheckOtherRestrictionsFailReasonCountryWhenCountryRestrictionFailed);
  CPPUNIT_TEST(testCheckOtherRestrictionsFailReasonSalesDateWhenSalesDateFail);
  CPPUNIT_TEST(testCheckOtherRestrictionsFailReasonFormOfPaymentWhenFormOfPaymentFail);
  CPPUNIT_TEST(testCheckOtherRestrictionsFailReasonCurrencyWhenCurrencyRestrictionFail);
  CPPUNIT_TEST(testCheckOtherRestrictionsFailReasonTicketWhenTicketRestrictionFail);
  CPPUNIT_TEST(testCheckOtherRestrictionsFailReasonSoldTicketWhenSoldTicketRestrictionFail);
  CPPUNIT_TEST(testDetermineNotValidReturnCodeSkipWhenTextOnly);
  CPPUNIT_TEST(testDetermineNotValidReturnCodeSkipWhenDateOvverride);
  CPPUNIT_TEST(testDetermineNotValidReturnCodeSkipWhenCRSOtherCarrierAndIsCat15GeneralRuleProcess);
  CPPUNIT_TEST(testDetermineNotValidReturnCodeFailWhenDataUnavailable);
  CPPUNIT_TEST(
      testDetermineNotValidReturnCodeFailWhenCRSOtherCarrierAndIsNotCat15GeneralRuleProcess);

  CPPUNIT_TEST(testPricingGetAgent);
  CPPUNIT_TEST(testRefundGetAgent);

  CPPUNIT_TEST(testIsPropperGDSvalidGds1S);
  CPPUNIT_TEST(testIsPropperGDSvalidGds1W);
  CPPUNIT_TEST(testIsPropperGDSvalidGds1B);
  CPPUNIT_TEST(testIsPropperGDSvalidGds1J);
  CPPUNIT_TEST(testIsPropperGDSvalidGds1F);
  CPPUNIT_TEST(testIsPropperGDSinvalidGds);
  CPPUNIT_TEST(testIsPropperGDSCrsIndNotSet);

  CPPUNIT_TEST(testMatchDeptCode3CharOACSuccess);
  CPPUNIT_TEST(testMatchDeptCode3CharOACFail);
  CPPUNIT_TEST(testMatchDeptCode5CharOACSuccess);
  CPPUNIT_TEST(testMatchDeptCode5CharOACFail);

  CPPUNIT_TEST(testValidateAllCarriersIS);
  CPPUNIT_TEST(testValidateAllCarriersISForMultiSp);
  CPPUNIT_TEST(testValidateCarriers);

  CPPUNIT_TEST(testUpdatePaxTypeFareWhenMainCategory_Cat15SoftPass_True);
  CPPUNIT_TEST(testUpdatePaxTypeFareWhenMainCategory_Cat15SoftPass_False);
  CPPUNIT_TEST(testUpdatePaxTypeFareWhenMainCategory_Cat15SecurityFail_True);
  CPPUNIT_TEST(testUpdatePaxTypeFareWhenMainCategory_Cat15SecurityFail_False);

  CPPUNIT_TEST(testValidateCarrierExcludePublishingFailPublishingCrxMatch);
  CPPUNIT_TEST(testValidateCarrierExcludePublishingFailAllianceNotMatch);
  CPPUNIT_TEST(testValidateCarrierExcludePublishingPassAllianceMatch);
  CPPUNIT_TEST(testValidateCarrierExcludePublishingPassOtherCarrierMatch);
 CPPUNIT_TEST(testValidateCarrierExcludePublishingFailOtherCarrierNotMatch);

  CPPUNIT_TEST_SUITE_END();

public:
  SalesRestrictionTest() {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    CppUnit::TestFixture::setUp();
    _memHandle.create<SalesRestrictionDataHandle>(_memHandle);
    _usAgentLoc = _memHandle.create<Loc>();
    _usAgentLoc->nation() = NATION_US;

    _frAgentLoc = _memHandle.create<Loc>();
    _frAgentLoc->nation() = NATION_FRANCE;

    _sr = _memHandle.create<SalesRestriction>();
    _srr = _memHandle.create<SalesRestrictionRuleCat15FallBack>();
    _trx = createGenericPricingTrx();
    _paxTypeFare = _memHandle.create<PaxTypeFareMock>();
  }

  void tearDown()
  {
    // Cleanup all objects between tests
    _memHandle.clear();
    TestFactoryManager::instance()->destroyAll();
    CppUnit::TestFixture::tearDown();
  }

  void testValidateSelectedNetRemitFareFailWhenSaleRestrictionDataIsUnAvailable()
  {
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    createBasicPaxTypeFareWithFareMarket();
    _sr->unavailTag() = RuleConst::DATA_UNAVAILABLE; // 'X';

    Record3ReturnTypes ret =
        _srr->validateSelectedNetRemitFare(*_trx, cri, &rule, *_paxTypeFare, _sr);

    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidateSelectedNetRemitFareSkipWhenSaleRestrictedToTextOnly()
  {
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    createBasicPaxTypeFareWithFareMarket();
    _sr->unavailTag() = RuleConst::TEXT_ONLY;

    Record3ReturnTypes ret =
        _srr->validateSelectedNetRemitFare(*_trx, cri, &rule, *_paxTypeFare, _sr);

    CPPUNIT_ASSERT_EQUAL(SKIP, ret);
  }

  void testValidateSelectedNetRemitFarePassWhenNoUnavailTagRestrictions()
  {
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    createBasicPaxTypeFareWithFareMarket();

    _sr->unavailTag() = ' ';
    _sr->overrideDateTblItemNo() = 0;

    Record3ReturnTypes ret =
        _srr->validateSelectedNetRemitFare(*_trx, cri, &rule, *_paxTypeFare, _sr);

    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testValidateFailWhenSalesRestrictionDataIsUnavailable()
  {
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    Itin itin;
    FareUsage fu;
    createBasicPaxTypeFareWithFareMarket();
    fu.paxTypeFare() = _paxTypeFare;

    bool isQualified = false;
    bool isCat15Security = false;
    bool skipCat15Security = false;

    _sr->unavailTag() = RuleConst::DATA_UNAVAILABLE; // 'X';

    Record3ReturnTypes ret = _srr->validate(*_trx,
                                            itin,
                                            &fu,
                                            *_paxTypeFare,
                                            cri,
                                            &rule,
                                            _sr,
                                            isQualified,
                                            isCat15Security,
                                            skipCat15Security);

    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidateFailWhenSalesRestrictedToTextOnly()
  {
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    Itin itin;

    FareUsage fu;
    createBasicPaxTypeFareWithFareMarket();
    fu.paxTypeFare() = _paxTypeFare;

    _sr->unavailTag() = RuleConst::TEXT_ONLY; // 'Y';

    bool isQualified = false;
    bool isCat15Security = false;
    bool skipCat15Security = false;
    Record3ReturnTypes ret = _srr->validate(*_trx,
                                            itin,
                                            &fu,
                                            *_paxTypeFare,
                                            cri,
                                            &rule,
                                            _sr,
                                            isQualified,
                                            isCat15Security,
                                            skipCat15Security);

    CPPUNIT_ASSERT_EQUAL(SKIP, ret);
  }

  void testValidateSkipWhenMustBeSoldViaCRSAndIsQualified()
  {
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    Itin itin;
    FareUsage fu;
    createBasicPaxTypeFareWithFareMarket();
    fu.paxTypeFare() = _paxTypeFare;

    _sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS; //'C';
    _sr->otherCarrier() = carrier_AA;

    bool isQualified = true;
    bool isCat15Security = false;
    bool skipCat15Security = false;
    Record3ReturnTypes ret = _srr->validate(*_trx,
                                            itin,
                                            &fu,
                                            *_paxTypeFare,
                                            cri,
                                            &rule,
                                            _sr,
                                            isQualified,
                                            isCat15Security,
                                            skipCat15Security);

    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidateSkipWhenMustBeSoldViaCRSAndIsNotQualified()
  {
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    Itin itin;

    FareUsage fu;
    createBasicPaxTypeFareWithFareMarket();
    fu.paxTypeFare() = _paxTypeFare;

    _sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS; //'C';
    _sr->otherCarrier() = carrier_AA;

    bool isQualified = false;
    bool isCat15Security = false;
    bool skipCat15Security = false;
    Record3ReturnTypes ret = _srr->validate(*_trx,
                                            itin,
                                            &fu,
                                            *_paxTypeFare,
                                            cri,
                                            &rule,
                                            _sr,
                                            isQualified,
                                            isCat15Security,
                                            skipCat15Security);

    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testCheckCountryRestPURetrunTrueWhenAgentCountryEqualsOriginCountry()
  {
    // Create Fare Usage
    FareUsage* fu = createBasicFareUsage();

    // Populate Request
    _trx->getRequest()->ticketingDT() = DateTime::localTime();

    // Set up Cat15 Rec 3 Data

    // 1 agent location = country of origing;
    _sr->countryRest() = RuleConst::COUNTRY_OF_ORIGIN;

    SalesRestrictionRuleMock checker;
    checker._countryLoc = TestLocFactory::create(LOC_PAR);
    CPPUNIT_ASSERT(checker.checkCountryRestPU(*_trx, fu, _sr));
  }

  void testCheckCountryRestPUFailWhenAgentLocationNotEqualDestintionCountry()
  {
    // Create Fare Usage
    FareUsage* fu = createBasicFareUsage();

    // Populate Request
    _trx->getRequest()->ticketingDT() = DateTime::localTime();

    // Set up Cat15 Rec 3 Data

    // 2 agent location != country of destination;
    _sr->countryRest() = RuleConst::COUNTRY_OF_DESTINATION;
    SalesRestrictionRuleMock checker;
    CPPUNIT_ASSERT(!checker.checkCountryRestPU(*_trx, fu, _sr));
  }

  void testCheckCountryRestPUFailWhenSalePointOverrideNotEqualOriginCountry()
  {

    // Create Fare Usage
    FareUsage* fu = createBasicFareUsage();

    // Populate Request
    _trx->getRequest()->ticketingDT() = DateTime::localTime();

    // Set up Cat15 Rec 3 Data

    // 3  salePoint ovveride (NYC) != country of origin;
    _trx->getRequest()->salePointOverride() = city_NYC;
    _sr->countryRest() = RuleConst::COUNTRY_OF_ORIGIN;

    SalesRestrictionRuleMock checker;
    checker._location = TestLocFactory::create(LOC_NYC1);
    CPPUNIT_ASSERT(!checker.checkCountryRestPU(*_trx, fu, _sr));
  }

  void testCheckSecurityRestrictionFailWhenTACantSell()
  {
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;

    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = agency_B4T0;
    _sr->tvlAgentSaleInd() = RuleConst::MAY_NOT_BE_SOLD_BY_TA;

    bool failedSabre = true;
    bool isFareDisplay = false;
    bool checkLocale = false;
    CPPUNIT_ASSERT(!tse::checkSecurityRestriction(*_srr,
        *_trx, 0, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, checkLocale));
  }

  void testIsWebFareFaileWhenNonCheckedAgencyAndTravelocity()
  {
    SalesRestrictionRuleForIsWebFareTest srrForTest;
    _srr = &srrForTest;

    srrForTest._hasTvlyLocation = true;
    _trx->getOptions()->web() = 'T';
    _trx->getRequest()->formOfPaymentCash() = YES;
    _sr->unavailTag() = ' ';

    // Memory will be deleted by SalesRestriction destructor
    Locale* locale = new Locale();
    LocKey lc1;
    LocKey lc2;

    LocTypeCode ltc = RuleConst::IATA_TVL_AGENCY_NO;
    LocCode lc;

    lc2.locType() = ltc;
    lc2.loc() = lc;

    locale->loc1() = lc1;
    locale->loc2() = lc2;

    _sr->locales() += locale;

    CPPUNIT_ASSERT(!_srr->isWebFare(*_trx, _sr));
  }

  void testIsWebFarePassWhenLoc1HomeTAAndTravelocity()
  {
    SalesRestrictionRuleForIsWebFareTest srrForTest;
    _srr = &srrForTest;

    srrForTest._hasTvlyLocation = true;
    _trx->getOptions()->web() = 'T';
    _trx->getRequest()->formOfPaymentCash() = YES;

    // Memory will be deleted by SalesRestriction destructor
    Locale* locale = new Locale();
    LocKey lc1;
    LocKey lc2;

    LocTypeCode ltc = RuleConst::HOME_TRAVEL_AGENCY;
    LocCode lc = RuleConst::WEB_FARE_PCC;

    lc2.locType() = ltc;
    lc2.loc() = lc;

    locale->loc1() = lc1;
    locale->loc2() = lc2;

    _sr->locales() += locale;

    bool ret = _srr->isWebFare(*_trx, _sr);

    CPPUNIT_ASSERT(ret);
  }

  void testIsWebFarePassWhenLoc2HomeTAAndTravelocity()
  {
    SalesRestrictionRuleForIsWebFareTest srrForTest;
    _srr = &srrForTest;

    srrForTest._hasTvlyLocation = true;
    _trx->getOptions()->web() = 'T';
    _trx->getRequest()->formOfPaymentCash() = YES;
    _sr->unavailTag() = ' ';

    // Memory will be deleted by SalesRestriction destructor
    Locale* locale = new Locale();
    LocKey lc1;
    LocKey lc2;

    LocTypeCode ltc = RuleConst::HOME_TRAVEL_AGENCY;
    LocCode lc = RuleConst::WEB_FARE_PCC;

    lc1.locType() = ltc;
    lc1.loc() = lc;

    locale->loc1() = lc1;
    locale->loc2() = lc2;

    _sr->locales() += locale;

    CPPUNIT_ASSERT(_srr->isWebFare(*_trx, _sr));
  }

  void testCheckCountryRestrictionPassWhenNoRestriction()
  {
    createBasicPaxTypeFareWithFareMarketParToLon();
    FareUsage* fu = 0;

    _trx->getRequest()->ticketingAgent()->agentLocation() = _usAgentLoc;
    _trx->getRequest()->ticketingDT() = DateTime::localTime();
    _trx->getRequest()->formOfPaymentCash() = YES;

    _sr->countryRest() = ' '; // RuleConst::BLANK;

    bool ret = _srr->checkCountryRestriction(*_trx, fu, *_paxTypeFare, _sr);
    CPPUNIT_ASSERT(ret);
  }

  void testCheckCountryRestrictionFailWhenNoOverrideOfRestriction()
  {
    createBasicPaxTypeFareWithFareMarketParToLon();
    FareUsage* fu = 0;

    _trx->getRequest()->ticketingAgent()->agentLocation() = _usAgentLoc;
    _trx->getRequest()->ticketingDT() = DateTime::localTime();
    _trx->getRequest()->formOfPaymentCash() = YES;

    // no FPoint
    _sr->countryRest() = RuleConst::COUNTRY_OF_DESTINATION;

    SalesRestrictionRuleMock checker;
    bool ret = checker.checkCountryRestriction(*_trx, fu, *_paxTypeFare, _sr);

    CPPUNIT_ASSERT(!ret);
  }

  void testCheckCountryRestrictionPassWhenOverrideOfRestriction()
  {
    FareUsage* fu = 0;
    createBasicPaxTypeFareWithFareMarketParToLon();

    _trx->getRequest()->ticketingAgent()->agentLocation() = _usAgentLoc;
    _trx->getRequest()->ticketingDT() = DateTime::localTime();
    _trx->getRequest()->formOfPaymentCash() = YES;

    // no FPoint
    _sr->countryRest() = RuleConst::COUNTRY_OF_DESTINATION;
    _trx->getRequest()->salePointOverride() = city_LON;

    SalesRestrictionRuleMock checker;
    checker._location = TestLocFactory::create(LOC_LON);
    bool ret = checker.checkCountryRestriction(*_trx, fu, *_paxTypeFare, _sr);
    CPPUNIT_ASSERT(ret);
  }

  void testCheckCountryRestrictionFailWhenIncorrectOverrideOfRestriction()
  {
    FareUsage* fu = 0;
    createBasicPaxTypeFareWithFareMarketParToLon();

    _trx->getRequest()->ticketingAgent()->agentLocation() = _usAgentLoc;
    _trx->getRequest()->ticketingDT() = DateTime::localTime();
    _trx->getRequest()->formOfPaymentCash() = YES;

    _sr->countryRest() = RuleConst::COUNTRY_OF_ORIGIN;
    _trx->getRequest()->salePointOverride() = city_FRA;

    SalesRestrictionRuleMock checker;
    bool ret = checker.checkCountryRestriction(*_trx, fu, *_paxTypeFare, _sr);
    CPPUNIT_ASSERT(!ret);
  }

  void testCheckFormOfPaymentPassIfNoRestrictions()
  {

    _sr->fopCashInd() = RuleConst::BLANK;
    _sr->fopCheckInd() = RuleConst::BLANK;
    _sr->fopCreditInd() = RuleConst::BLANK;
    _sr->fopGtrInd() = RuleConst::BLANK;

    // Pay by Cash
    _trx->getRequest()->formOfPaymentCash() = YES;
    _trx->getRequest()->formOfPaymentCheck() = '\0';
    _trx->getRequest()->formOfPaymentCard() = '\0';

    bool ret = _srr->checkFormOfPayment(*_trx, *_paxTypeFare, _sr, 0);
    CPPUNIT_ASSERT(ret);

    // Pay by Check
    _trx->getRequest()->formOfPaymentCheck() = YES;
    _trx->getRequest()->formOfPaymentCash() = '\0';
    ret = _srr->checkFormOfPayment(*_trx, *_paxTypeFare, _sr, 0);
    CPPUNIT_ASSERT(ret);

    // Pay by Payment Card
    _trx->getRequest()->formOfPaymentCard() = YES;
    _trx->getRequest()->formOfPaymentCheck() = '\0';
    ret = _srr->checkFormOfPayment(*_trx, *_paxTypeFare, _sr, 0);
    CPPUNIT_ASSERT(ret);
  }

  void testCheckFormOfPaymentAlwaysPassIfNotTicketEntry()
  {

    createBasicPaxTypeFareWithFare();

    _trx->getRequest()->formOfPaymentCash() = '\0';
    _trx->getRequest()->formOfPaymentCheck() = '\0';
    _trx->getRequest()->formOfPaymentCard() = '\0';
    Indicator i = 'V';

    _sr->fopCashInd() = i;
    _sr->fopCheckInd() = i;
    _sr->fopCreditInd() = i;
    _sr->fopGtrInd() = i;

    bool ret = _srr->checkFormOfPayment(*_trx, *_paxTypeFare, _sr, 0);

    CPPUNIT_ASSERT(ret);
  }

  void testCheckFormOfPaymentDoesntAllowPaymentWhenRestricted()
  {
    createBasicPaxTypeFareWithFare();

    _trx->getRequest()->formOfPaymentCash() = '\0';
    _trx->getRequest()->formOfPaymentCheck() = '\0';
    _trx->getRequest()->formOfPaymentCard() = '\0';
    Indicator i = 'V';

    _sr->fopCashInd() = i;
    _sr->fopCheckInd() = i;
    _sr->fopCreditInd() = i;

    // Test cash payment when restricted
    _trx->getRequest()->formOfPaymentCash() = YES;
    _sr->fopCashInd() = 'M';

    bool ret = _srr->checkFormOfPayment(*_trx, *_paxTypeFare, _sr, 0);

    // Test check payment when restricted
    _trx->getRequest()->formOfPaymentCash() = '\0';
    _trx->getRequest()->formOfPaymentCheck() = YES;
    _sr->fopCashInd() = i;
    _sr->fopCheckInd() = 'M';

    ret = _srr->checkFormOfPayment(*_trx, *_paxTypeFare, _sr, 0);

    // Test card payment when restricted
    _trx->getRequest()->formOfPaymentCheck() = '\0';
    _trx->getRequest()->formOfPaymentCard() = YES;
    _sr->fopCheckInd() = i;
    _sr->fopCreditInd() = 'M';

    ret = _srr->checkFormOfPayment(*_trx, *_paxTypeFare, _sr, 0);
    CPPUNIT_ASSERT(!ret);
  }

  void testCheckCurrencyRestrictionPassNoRestrictionsForGivenCurrency()
  {
    createBasicPaxTypeFareWithFareMarketParToLon();
    FareUsage* fu = 0;

    _trx->getRequest()->ticketingAgent()->agentLocation() = _usAgentLoc;
    _trx->getRequest()->ticketingAgent()->currencyCodeAgent() = GBP;
    _trx->getRequest()->formOfPaymentCash() = YES;
    _trx->getRequest()->ticketingDT() = DateTime::localTime();

    AirSeg* seg = createAirSegParToLon();
    _trx->travelSeg() += seg;

    _sr->curr() = GBP;

    bool ret = _srr->checkCurrencyRestriction(*_trx, fu, *_paxTypeFare, _sr);

    CPPUNIT_ASSERT(ret);
  }

  void testCheckCurrencyRestrictionFailWhenAgentCurrencyNotSameAsRestriction()
  {
    createBasicPaxTypeFareWithFareMarketParToLon();
    FareUsage* fu = 0;

    _trx->getRequest()->ticketingAgent()->agentLocation() = _usAgentLoc;
    _trx->getRequest()->ticketingAgent()->currencyCodeAgent() = "ABC";
    _trx->getRequest()->formOfPaymentCash() = YES;
    _trx->getRequest()->ticketingDT() = DateTime::localTime();

    AirSeg* seg = createAirSegParToLon();
    _trx->travelSeg() += seg;

    _sr->curr() = GBP;

    SalesRestrictionRuleMock checker;
    CPPUNIT_ASSERT(!checker.checkCurrencyRestriction(*_trx, fu, *_paxTypeFare, _sr));
  }

  void testCheckCurrencyRestrictionPassIfCurrencyOverrideIsValid()
  {
    createBasicPaxTypeFareWithFareMarketParToLon();
    FareUsage* fu = 0;

    _trx->getRequest()->ticketingAgent()->agentLocation() = _usAgentLoc;
    _trx->getRequest()->ticketingAgent()->currencyCodeAgent() = USD;
    _trx->getRequest()->formOfPaymentCash() = YES;
    _trx->getRequest()->ticketingDT() = DateTime::localTime();

    AirSeg* seg = createAirSegParToLon();
    _trx->travelSeg() += seg;

    _sr->curr() = GBP;

    _trx->getOptions()->currencyOverride() = GBP;

    CPPUNIT_ASSERT(_srr->checkCurrencyRestriction(*_trx, fu, *_paxTypeFare, _sr));
  }

  void testCheckCurrencyRestPUFailIfInboundLocationCurrencyDoesntMatchOverride()
  {
    FareUsage* fu = createBasicFareUsage();

    // Populate Request
    _trx->getRequest()->ticketingDT() = DateTime::localTime();
    _trx->getRequest()->formOfPaymentCash() = YES;

    _sr->curr() = city_FRA;
    _sr->currCntryInd() = RuleConst::COUNTRY_OF_DESTINATION;

    _trx->getOptions()->currencyOverride() = GBP;
    fu->paxTypeFare()->fareMarket()->travelDate() = DateTime::localTime();
    fu->inbound() = true;

    SalesRestrictionRuleMock checker;
    CPPUNIT_ASSERT(!checker.checkCurrencyRestPU(*_trx, fu, _sr->currCntryInd(), _sr->curr()));
  }

  void testCheckSelectSecurityPassNoRuleSet()
  {
    // Create PaxType Fare
    createBasicPaxTypeFareWithFareAndTariffXRef();

    _sr->tvlAgentSelectedInd() = RuleConst::BLANK;

    bool ret = _srr->checkSelectSecurity(*_paxTypeFare, _sr);
    CPPUNIT_ASSERT(ret);
  }

  void testCheckSelectSecurityPassIfPrivateTarrifAndOnlyOverrideDateTblItemNoIsSet()
  {
    // Create PaxType Fare
    createBasicPaxTypeFareWithFareAndTariffXRef();

    // Set up Cat15 Rec 3 Data

    _sr->tvlAgentSelectedInd() = RuleConst::AIRPORT;
    _sr->overrideDateTblItemNo() = 21345;
    _sr->countryRest() = RuleConst::BLANK;
    _sr->residentRest() = RuleConst::BLANK;
    _sr->carrierCrsInd() = RuleConst::BLANK;
    _sr->validationInd() = RuleConst::BLANK;
    _sr->carrierSegInd() = RuleConst::BLANK;
    _sr->tvlAgentSaleInd() = RuleConst::BLANK;
    _sr->fopCashInd() = RuleConst::BLANK;
    _sr->fopCheckInd() = RuleConst::BLANK;
    _sr->fopCreditInd() = RuleConst::BLANK;
    _sr->fopGtrInd() = RuleConst::BLANK;

    _sr->currCntryInd() = RuleConst::BLANK;
    _sr->curr().clear();
    _sr->tktIssMail() = RuleConst::BLANK;
    _sr->tktIssPta() = RuleConst::BLANK;
    _sr->tktIssMech() = RuleConst::BLANK;
    _sr->tktIssSelf() = RuleConst::BLANK;
    _sr->tktIssPtaTkt() = RuleConst::BLANK;
    _sr->tktIssAuto() = RuleConst::BLANK;
    _sr->tktIssSat() = RuleConst::BLANK;
    _sr->tktIssSatOcAto() = RuleConst::BLANK;

    _sr->tktIssElectronic() = RuleConst::BLANK;
    _sr->tktIssSiti() = RuleConst::BLANK;
    _sr->tktIssSoto() = RuleConst::BLANK;
    _sr->tktIssSito() = RuleConst::BLANK;
    _sr->tktIssSoti() = RuleConst::BLANK;
    _sr->familyGrpInd() = RuleConst::BLANK;
    _sr->extendInd() = RuleConst::BLANK;

    CPPUNIT_ASSERT(!_srr->checkSelectSecurity(*_paxTypeFare, _sr));
  }

  void testCheckTicketRestrictionFailWhenETicketNotAllowedAndIsTicketEntry()
  {
    CategoryRuleInfo cri;

    _trx->getRequest()->ticketingAgent()->agentLocation() = _usAgentLoc;
    _trx->getRequest()->ticketEntry() = 'W';
    _trx->getRequest()->electronicTicket() = 'T';
    _trx->getRequest()->formOfPaymentCash() = YES;

    _sr->tktIssElectronic() = NO; //'N'  not electronic ticket

    CPPUNIT_ASSERT(!tse::checkTicketRestriction(*_srr, *_trx, 0, *_paxTypeFare, _sr, cri));
  }

  void testCheckTicketRestrictionPassWhenETicketAllowedAndIsTicketEntry()
  {
    CategoryRuleInfo cri;

    _trx->getRequest()->ticketingAgent()->agentLocation() = _usAgentLoc;
    _trx->getRequest()->ticketEntry() = 'W';
    _trx->getRequest()->electronicTicket() = 'T';
    _trx->getRequest()->formOfPaymentCash() = YES;

    _sr->tktIssElectronic() = YES; //'Y'   allowed

    CPPUNIT_ASSERT(tse::checkTicketRestriction(*_srr, *_trx, 0, *_paxTypeFare, _sr, cri));
  }

  void testCheckSoldTktRestrictionPassWhenSegOriginEqualsAgentNation()
  {
    FareMarket fm;
    Itin itin;
    itin.fareMarket() += &fm;

    AirSeg* seg = createAirSegParToLon();
    itin.travelSeg() += seg;

    _sr->tktIssSiti() = RuleConst::REQUIRED;

    bool ret = _srr->checkSoldTktRestriction(*_trx, itin, *_paxTypeFare, _sr);
    CPPUNIT_ASSERT(ret);
  }

  void testCheckSoldTktRestrictionFailWhenIssSitiAndSoldInsideTktInside()
  {
    FareMarket fm;
    Itin itin;
    itin.fareMarket() += &fm;
    _sr->tktIssSoti() = RuleConst::REQUIRED;

    AirSeg* seg = createAirSegParToLon();
    itin.travelSeg() += seg;

    _sr->tktIssSiti() = RuleConst::NOT_ALLOWED;
    bool ret = _srr->checkSoldTktRestriction(*_trx, itin, *_paxTypeFare, _sr);
    CPPUNIT_ASSERT(!ret);

    // Also test with value BLANK
    _sr->tktIssSiti() = RuleConst::BLANK;
    ret = _srr->checkSoldTktRestriction(*_trx, itin, *_paxTypeFare, _sr);
    CPPUNIT_ASSERT(!ret);
  }

  void testCheckSoldTktRestrictionFailWhenIssSotoAndSoldInsideTktInside()
  {
    FareMarket fm;
    Itin itin;
    itin.fareMarket() += &fm;
    _sr->tktIssSoti() = RuleConst::REQUIRED;

    AirSeg* seg = createAirSegLonToPar();
    itin.travelSeg() += seg;
    ;

    _sr->tktIssSoto() = RuleConst::NOT_ALLOWED;
    bool ret = _srr->checkSoldTktRestriction(*_trx, itin, *_paxTypeFare, _sr);
    CPPUNIT_ASSERT(!ret);

    // Also test with value BLANK
    _sr->tktIssSoto() = RuleConst::BLANK;
    ret = _srr->checkSoldTktRestriction(*_trx, itin, *_paxTypeFare, _sr);
    CPPUNIT_ASSERT(!ret);
  }

  void testHasTvlyLocationFailIfNoAgent()
  {
    bool ret = true;

    // Ensure Agent is null
    _trx->getRequest()->ticketingAgent() = 0;

    ret = _srr->hasTvlyLocation(*_trx);
    CPPUNIT_ASSERT(!ret);
  }

  void testHasTvlyLocationFailIfNotTJRAgent()
  {
    bool ret = true;

    ret = _srr->hasTvlyLocation(*_trx);
    CPPUNIT_ASSERT(!ret);
  }

  void testHasTvlyLocationPassIfTvlyLocation()
  {
    _trx->getRequest()->ticketingAgent()->agentTJR() = _memHandle.create<Customer>();
    _trx->getRequest()->ticketingAgent()->agentTJR()->tvlyLocation() = YES;
    bool ret = _srr->hasTvlyLocation(*_trx);
    CPPUNIT_ASSERT(ret);
  }

  void testCheckTicketElectronicWithTicketETicketRequired()
  {
    bool reqEtkt = true;
    Indicator canEtkt = RuleConst::REQUIRED;

    bool ret = _srr->checkTicketElectronic(reqEtkt, canEtkt);
    CPPUNIT_ASSERT(ret);

    canEtkt = RuleConst::NOT_ALLOWED;
    ret = _srr->checkTicketElectronic(reqEtkt, canEtkt);
    CPPUNIT_ASSERT(!ret);
  }

  void testCheckTicketElectronicWithTicketETicketNotRequired()
  {
    bool reqEtkt = false;
    Indicator canEtkt = RuleConst::REQUIRED;

    bool ret = _srr->checkTicketElectronic(reqEtkt, canEtkt);
    CPPUNIT_ASSERT(!ret);

    canEtkt = RuleConst::NOT_ALLOWED;
    ret = _srr->checkTicketElectronic(reqEtkt, canEtkt);
    CPPUNIT_ASSERT(ret);
  }

  void testCheckCarrierRestrictionPassIfNoRestrictions()
  {
    Itin itin;
    FareUsage* fu = 0;
    FareMarket fm;
    _paxTypeFare->fareMarket() = &fm;

    // Must force fields to be blank .. defaults to \0
    _sr->validationInd() = RuleConst::BLANK;
    _sr->carrierSegInd() = RuleConst::BLANK;
    bool ret = _srr->checkCarrierRestriction(*_trx, itin, *_paxTypeFare, _sr, fu);
    CPPUNIT_ASSERT(ret);
  }

  void testCheckCarrierRestrictionPassIfMipTrxAndNoFareUsage()
  {
    Itin itin;
    FareUsage* fu = 0;
    itin.validatingCarrier() = carrier_AA;
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_AA);
    itin.fareMarket() += _paxTypeFare->fareMarket();
    _trx->itin() += &itin;

    _sr->validationInd() = 'S';
    _sr->carrierSegInd() = RuleConst::BLANK;

    _trx->setTrxType(PricingTrx::MIP_TRX);
    bool ret = _srr->checkCarrierRestriction(*_trx, itin, *_paxTypeFare, _sr, fu);
    CPPUNIT_ASSERT(ret);
  }

  void testCheckCarrierRestrictionPassIfTrxTypeEqualsMIP_TRX()
  {
    Itin itin;
    FareUsage fu;
    FareMarket fm;
    _paxTypeFare->fareMarket() = &fm;
    _sr->validationInd() = 'S';
    _sr->carrierSegInd() = RuleConst::BLANK;

    _trx->setTrxType(PricingTrx::MIP_TRX);
    bool ret = _srr->checkCarrierRestriction(*_trx, itin, *_paxTypeFare, _sr, &fu);
    CPPUNIT_ASSERT(ret);
  }

  // This test assumes the following:
  //         SalesRestriction.carrierSegInd != RuleConst::NO_SEGMENT_OF_TICKET
  //         SalesRestriction.carrierSegInd != RuleConst::NO_SEGMENT_AT_THIS_FARE
  void testCheckCarrierRestrictionPassWhenValidateCarrierMatchesRuleOtherCarrier()
  {
    Itin itin;
    FareUsage fu;
    FareMarket fm;
    _paxTypeFare->fareMarket() = &fm;
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    itin.validatingCarrier() = carrier_AA;
    _sr->validationInd() = 'S';
    _sr->otherCarrier() = carrier_AA;
    _sr->carrierSegInd() = RuleConst::BLANK;
    bool ret = _srr->checkCarrierRestriction(*_trx, itin, *_paxTypeFare, _sr, &fu);
    CPPUNIT_ASSERT(ret);
  }

  void testCheckCarrierRestrictionFailIfNotJointCarrier()
  {
    Itin itin;
    FareUsage fu;
    FareMarket fm;
    fm.validatingCarriers().push_back(carrier_UA);
    fm.validatingCarriers().push_back(carrier_AA);
    _paxTypeFare->fareMarket() = &fm;
    _paxTypeFare->validatingCarriers().push_back(carrier_UA);
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    _trx->setValidatingCxrGsaApplicable(true);
    itin.validatingCarrier() = carrier_AA;
    _sr->validationInd() = RuleConst::BLANK;
    _sr->otherCarrier() = carrier_UA;
    _sr->carrierSegInd() = RuleConst::BLANK;

    createBasicPaxTypeFareWithFareAndFareInfo(carrier_UA);

    bool ret = _srr->checkCarrierRestriction(*_trx, itin, *_paxTypeFare, _sr, &fu);
    CPPUNIT_ASSERT(_paxTypeFare->validatingCarriers()[0] == "UA");
    CPPUNIT_ASSERT(ret);
  }

  void testCheckSaleSecurityFailWhenFareMayNotBeSoldToTA()
  {
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;

    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = agency_B4T0;
    _sr->tvlAgentSaleInd() = RuleConst::MAY_NOT_BE_SOLD_BY_TA;

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(!tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));

  }

  void testCheckSaleSecurityFailWhenFareMustBeSoldByCarrier()
  {
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;

    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = agency_B4T0;
    _sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CARRIER;

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(!tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityPassWhenPrivateTariffAndJointCarrier()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(RuleConst::PRIVATE_TARIFF,
                                                           RuleConst::JOINT_CARRIER);
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = EMPTY_STRING();
    _trx->getRequest()->ticketingAgent()->tvlAgencyIATA() = EMPTY_STRING();

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityPassWhenPrivateTariffAndNotJointCarrierPartitionIdEmpty()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(RuleConst::PRIVATE_TARIFF,
                                                           RuleConst::SABRE1W);
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    _trx->billing() = _memHandle.create<Billing>();
    _trx->billing()->partitionID() = EMPTY_STRING();
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = EMPTY_STRING();
    _trx->getRequest()->ticketingAgent()->tvlAgencyIATA() = EMPTY_STRING();

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void
  testCheckSaleSecurityFailWhenPrivateTariffAndNotJointCarrierPartitionIdNotMatchCarrierNotMatchOtherCarrierNotMatchGoverningCarrier()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(RuleConst::PRIVATE_TARIFF,
                                                           RuleConst::SABRE1W);
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    _trx->billing() = _memHandle.create<Billing>();
    _trx->billing()->partitionID() = RuleConst::SABRE1S;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);
    _sr->otherCarrier() = RuleConst::SABRE1W;
    _paxTypeFare->fareMarket() = _memHandle.create<FareMarket>();
    _paxTypeFare->fareMarket()->governingCarrier() = RuleConst::SABRE1W;

    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = EMPTY_STRING();
    _trx->getRequest()->ticketingAgent()->tvlAgencyIATA() = EMPTY_STRING();

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(!tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void
  testCheckSaleSecurityFailWhenPrivateTariffAndNotJointCarrierPartitionIdNotMatchCarrierNotMatchOtherCarrierFareIsNotIndustry()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(RuleConst::PRIVATE_TARIFF,
                                                           RuleConst::SABRE1W);
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    _trx->billing() = _memHandle.create<Billing>();
    _trx->billing()->partitionID() = RuleConst::SABRE1S;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);
    _sr->otherCarrier() = RuleConst::SABRE1W;
    _paxTypeFare->fareMarket() = _memHandle.create<FareMarket>();
    _paxTypeFare->fareMarket()->governingCarrier() = RuleConst::SABRE1S;
    _paxTypeFare->fare()->status().set(Fare::FS_IsRouting); // other than industry

    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = EMPTY_STRING();
    _trx->getRequest()->ticketingAgent()->tvlAgencyIATA() = EMPTY_STRING();

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(!tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void
  testCheckSaleSecurityPassWhenPrivateTariffAndNotJointCarrierPartitionIdNotMatchCarrierNotMatchOtherCarrierGoverningCarrierNotMatchFareIsIndustry()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(RuleConst::PRIVATE_TARIFF,
                                                           RuleConst::SABRE1W);
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    _trx->billing() = _memHandle.create<Billing>();
    _trx->billing()->partitionID() = RuleConst::SABRE1S;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);
    _sr->otherCarrier() = RuleConst::SABRE1W;
    _paxTypeFare->fareMarket() = _memHandle.create<FareMarket>();
    _paxTypeFare->fareMarket()->governingCarrier() = RuleConst::SABRE1S;
    _paxTypeFare->fare()->status().set(Fare::FS_IndustryFare);

    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = EMPTY_STRING();
    _trx->getRequest()->ticketingAgent()->tvlAgencyIATA() = EMPTY_STRING();

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void
  testCheckSaleSecurityPassWhenNotPrivateTariffAndPartitionIdNotMatchCarrierAndMustBeSoldViaCarrierAndJointCarrier()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(0, RuleConst::JOINT_CARRIER);
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    _trx->billing() = _memHandle.create<Billing>();
    _trx->billing()->partitionID() = RuleConst::SABRE1S;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);
    _sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CARRIER;
    _sr->otherCarrier() = RuleConst::JOINT_CARRIER;
    _paxTypeFare->fareMarket() = _memHandle.create<FareMarket>();

    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = EMPTY_STRING();
    _trx->getRequest()->ticketingAgent()->tvlAgencyIATA() = EMPTY_STRING();

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void
  testCheckSaleSecurityFailWhenNotPrivateTariffAndPartitionIdNotMatchCarrierAndMustBeSoldViaCarrierAndNotJointCarrier()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(0, RuleConst::SABRE1W);
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    _trx->billing() = _memHandle.create<Billing>();
    _trx->billing()->partitionID() = RuleConst::SABRE1S;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);
    _sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CARRIER;

    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = EMPTY_STRING();
    _trx->getRequest()->ticketingAgent()->tvlAgencyIATA() = EMPTY_STRING();

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(!tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void
  testCheckSaleSecurityFailWhenNotPrivateTariffAndPartitionIdNotMatchCarrierAndNotMustBeSoldViaCarrierAndMayOnlyBeSoldByTa()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(0, RuleConst::SABRE1W);
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    _trx->billing() = _memHandle.create<Billing>();
    _trx->billing()->partitionID() = RuleConst::SABRE1S;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);
    _sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    _sr->tvlAgentSaleInd() = RuleConst::MAY_ONLY_BE_SOLD_BY_TA;

    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = EMPTY_STRING();
    _trx->getRequest()->ticketingAgent()->tvlAgencyIATA() = EMPTY_STRING();

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(!tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityFailWhenNotPrivateTariffAnd()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(0, RuleConst::SABRE1W);
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    _trx->billing() = _memHandle.create<Billing>();
    _trx->billing()->partitionID() = RuleConst::SABRE1S;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);
    _sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    _sr->tvlAgentSaleInd() = RuleConst::BLANK;

    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = EMPTY_STRING();
    _trx->getRequest()->ticketingAgent()->tvlAgencyIATA() = EMPTY_STRING();

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(!tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void commonRtwPreparationSaleSecurity(const CarrierCode& cxr,
                                        const CarrierCode& valCxr,
                                        const Indicator& carrierCrsInd,
                                        const Indicator& validationInd,
                                        const CarrierCode& otherCarrier,
                                        const CarrierCode& partitionID)
  {
    _trx->billing() = _memHandle.create<Billing>();
    _trx->billing()->partitionID() = partitionID;
    _trx->getOptions()->setRtw(true);

    _sr->otherCarrier() = otherCarrier;
    _sr->validationInd() = validationInd;
    _sr->carrierCrsInd() = carrierCrsInd;
    _paxTypeFare->fareMarket() = _memHandle.create<FareMarket>();
    _paxTypeFare->fareMarket()->governingCarrier() = RuleConst::SABRE1S;
    Fare* fare = _paxTypeFare->fare();
    FareInfo* fi = const_cast<FareInfo*>(fare->fareInfo());
    fi->carrier() = cxr;
    PaxTypeFareMock* ptfm = dynamic_cast<PaxTypeFareMock*>(_paxTypeFare);
    CPPUNIT_ASSERT(ptfm != 0);
    ptfm->setCarrier(cxr);
    _paxTypeFare->setFare(fare);
    _paxTypeFare->fare()->status().set(Fare::FS_IndustryFare);

    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = EMPTY_STRING();
    _trx->getRequest()->ticketingAgent()->tvlAgencyIATA() = EMPTY_STRING();

    _itin = _memHandle.create<Itin>();
    _itin->validatingCarrier() = valCxr;
    AirSeg* seg = createAirSegParToLon(); // carrier AA
    AirSeg* seg2 = createAirSegLonToPar(); // carrier AA
    _itin->travelSeg() += seg;
    _itin->travelSeg() += seg2;
    _trx->itin().push_back(_itin);
  }

  void testCheckSaleSecurityPassRTWPrivate()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(RuleConst::PRIVATE_TARIFF);
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    commonRtwPreparationSaleSecurity(
        "LH", "LH", RuleConst::MUST_BE_SOLD_VIA_CARRIER, RuleConst::BLANK, STAR_ALLIANCE, "LH");

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityPassRTWPrivateExact()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(RuleConst::PRIVATE_TARIFF);
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    commonRtwPreparationSaleSecurity(
        "LH", "LH", RuleConst::MUST_BE_SOLD_VIA_CARRIER, RuleConst::BLANK, "LH", "LH");

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityPassRTWPrivatePTF()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(RuleConst::PRIVATE_TARIFF);
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    commonRtwPreparationSaleSecurity(
        "AA", "LH", RuleConst::MUST_BE_SOLD_VIA_CARRIER, RuleConst::BLANK, "LH", "AA");

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityFailRTWPrivate()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(RuleConst::PRIVATE_TARIFF);
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    commonRtwPreparationSaleSecurity(
        "LH", "LH", RuleConst::MUST_BE_SOLD_VIA_CARRIER, RuleConst::BLANK, STAR_ALLIANCE, "AA");

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(!tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityPassRTWPublic()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(2); // PUBLIC TARIFF
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    commonRtwPreparationSaleSecurity(
        "LH", "LH", RuleConst::MUST_BE_SOLD_VIA_CARRIER, RuleConst::BLANK, STAR_ALLIANCE, "LH");

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityPassRTWPublicExact()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(2); // PUBLIC TARIFF
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    commonRtwPreparationSaleSecurity(
        "LH", "LH", RuleConst::MUST_BE_SOLD_VIA_CARRIER, RuleConst::BLANK, "LH", "LH");

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityPassRTWPublicPTF()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(2); // PUBLIC TARIFF
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    commonRtwPreparationSaleSecurity(
        "AA", "LH", RuleConst::MUST_BE_SOLD_VIA_CARRIER, RuleConst::BLANK, "LH", "AA");

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityFailRTWPublic()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(2); // PUBLIC TARIFF
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    commonRtwPreparationSaleSecurity(
        "LH", "AA", RuleConst::MUST_BE_SOLD_VIA_CARRIER, RuleConst::BLANK, STAR_ALLIANCE, "AA");

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(!tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityPassRTWValidatingCxr()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(2); // PUBLIC TARIFF
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    commonRtwPreparationSaleSecurity("LH",
                                     "LH",
                                     RuleConst::MUST_BE_SOLD_VIA_CARRIER,
                                     RuleConst::VALIDATIING_CXR_RESTR_SALE_BASED,
                                     STAR_ALLIANCE,
                                     "LH");

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityPassRTWValidatingCxrExact()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(2); // PUBLIC TARIFF
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    commonRtwPreparationSaleSecurity("LH",
                                     "LH",
                                     RuleConst::MUST_BE_SOLD_VIA_CARRIER,
                                     RuleConst::VALIDATIING_CXR_RESTR_SALE_BASED,
                                     "LH",
                                     "LH");

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityFailRTWValidatingCxr()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(2); // PUBLIC TARIFF
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    commonRtwPreparationSaleSecurity("LH",
                                     "AA",
                                     RuleConst::MUST_BE_SOLD_VIA_CARRIER,
                                     RuleConst::VALIDATIING_CXR_RESTR_SALE_BASED,
                                     STAR_ALLIANCE,
                                     "LH");

  }

  void testCheckSaleSecurityPassRTWValidatingCxrBlank()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(2); // PUBLIC TARIFF
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    commonRtwPreparationSaleSecurity("AA",
                                     "LH",
                                     RuleConst::BLANK,
                                     RuleConst::VALIDATIING_CXR_RESTR_SALE_BASED,
                                     STAR_ALLIANCE,
                                     "LH");

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityPassRTWValidatingCxrExactBlank()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(2); // PUBLIC TARIFF
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    commonRtwPreparationSaleSecurity("LH",
                                     "LH",
                                     RuleConst::BLANK,
                                     RuleConst::VALIDATIING_CXR_RESTR_SALE_BASED,
                                     ONE_WORLD_ALLIANCE,
                                     "LH");

    bool failedSabre = true;
    bool isFareDisplay = false;
    CPPUNIT_ASSERT(tse::checkSaleSecurity(*_srr,
        *_trx, *_paxTypeFare, cri, &rule, _sr, failedSabre, isFareDisplay, false));
  }

  void testCheckSaleSecurityFailRTWValidatingCxrBlank()
  {
    createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(2); // PUBLIC TARIFF
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);

    commonRtwPreparationSaleSecurity("LH",
                                     "AA",
                                     RuleConst::BLANK,
                                     RuleConst::VALIDATIING_CXR_RESTR_SALE_BASED,
                                     STAR_ALLIANCE,
                                     "LH");


  }

  // N B4T0 88D7
  // Y N US
  void testCheckLocaleItemsFailWhenTktNotSoldMatchinPCCButNotOptIn()
  {
    _trx = createPricingTrxForCheckLocalItems();
    createBasicPaxTypeFareWithFareAndTariffXRef();
    CategoryRuleInfo cri;

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->locType() = RuleConst::TRAVEL_AGENCY;
    loc1->loc() = agency_B4T0;

    LocKey* loc2 = _memHandle.create<LocKey>();
    loc2->locType() = RuleConst::HOME_TRAVEL_AGENCY;
    loc2->loc() = SalesRestrictionRule::OPT_IN_AGENCY;

    LocKey* loc3 = _memHandle.create<LocKey>();
    loc3->locType() = LOCTYPE_NATION; //'N';
    loc3->loc() = NATION_US;

    // Test Case #1 (FAIL)
    // N B4T0 88D7
    // Y N US

    Locale* locale1 = new Locale();
    locale1->locAppl() = RuleConst::TKTS_MAY_NOT_BE_SOLD;
    locale1->loc1() = *loc1;
    locale1->loc2() = *loc2;

    Locale* locale2 = new Locale();
    locale2->locAppl() = RuleConst::TKTS_MAY_ONLY_BE_SOLD;
    locale2->loc1() = *loc3;

    _sr->locales() += locale1, locale2;
    _sr->vendor() = ATPCO_VENDOR_CODE;

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(ret = tse::checkLocaleItems(*_srr, *_trx, 0, *_paxTypeFare, cri, _sr, false));
    CPPUNIT_ASSERT(!ret);
  }

  void testCheckLocaleItemsFailWhenTktMayBeSoldMatchinPCCButNotOptIn()
  {
    _trx = createPricingTrxForCheckLocalItems();
    createBasicPaxTypeFareWithFareAndTariffXRef();
    CategoryRuleInfo cri;

    // FAIL if not OptIn
    _trx->getRequest()->ticketingAgent()->agentTJR()->optInAgency() = 'N';

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->locType() = RuleConst::TRAVEL_AGENCY;
    loc1->loc() = agency_B4T0;

    LocKey* loc2 = _memHandle.create<LocKey>();
    loc2->locType() = RuleConst::HOME_TRAVEL_AGENCY;
    loc2->loc() = SalesRestrictionRule::OPT_IN_AGENCY;

    LocKey* loc3 = _memHandle.create<LocKey>();
    loc3->locType() = LOCTYPE_NATION; //'N';
    loc3->loc() = NATION_US;

    _sr->vendor() = ATPCO_VENDOR_CODE;

    // Test Case #1 (FAIL)
    // N B4T0 88D7
    // Y N US

    Locale* locale1 = new Locale();
    locale1->locAppl() = RuleConst::TKTS_MAY_NOT_BE_SOLD;
    locale1->loc1() = *loc1;
    locale1->loc2() = *loc2;

    Locale* locale2 = new Locale();
    locale2->locAppl() = RuleConst::TKTS_MAY_ONLY_BE_SOLD;
    locale2->loc1() = *loc3;

    _sr->locales() += locale1, locale2;

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(ret = tse::checkLocaleItems(*_srr, *_trx, 0, *_paxTypeFare, cri, _sr, false));
    CPPUNIT_ASSERT(!ret);
  }

  void testCheckLocaleItemsFailWhenPCCNotMatchAndNoOptIn()
  {
    _trx = createPricingTrxForCheckLocalItems();
    createBasicPaxTypeFareWithFareAndTariffXRef();
    CategoryRuleInfo cri;

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->locType() = RuleConst::TRAVEL_AGENCY;
    loc1->loc() = agency_B4T0;

    LocKey* loc2 = _memHandle.create<LocKey>();
    loc2->locType() = RuleConst::HOME_TRAVEL_AGENCY;
    loc2->loc() = SalesRestrictionRule::OPT_IN_AGENCY;

    LocKey* loc3 = _memHandle.create<LocKey>();
    loc3->locType() = LOCTYPE_NATION; //'N';
    loc3->loc() = NATION_US;

    _sr->vendor() = ATPCO_VENDOR_CODE;

    // Test Case #2 (FAIL)
    // N BET0
    // N 88D7
    // Y N US
    Locale* locale1 = new Locale();
    locale1->locAppl() = RuleConst::TKTS_MAY_NOT_BE_SOLD;
    locale1->loc1() = *loc1;

    Locale* locale2 = new Locale();
    locale2->locAppl() = RuleConst::TKTS_MAY_NOT_BE_SOLD;
    locale2->loc1() = *loc2;

    Locale* locale3 = new Locale();
    locale3->locAppl() = RuleConst::TKTS_MAY_ONLY_BE_SOLD;
    locale3->loc1() = *loc3;

    _sr->locales() += locale1, locale2, locale3;

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(ret = tse::checkLocaleItems(*_srr, *_trx, 0, *_paxTypeFare, cri, _sr, false));
    CPPUNIT_ASSERT(!ret);
  }

  // N BET0
  // N 88D7
  // Y N US
  void testCheckLocaleItemsFail_NBET0_N88D7_YNUS()
  {
    _trx = createPricingTrxForCheckLocalItems();
    createBasicPaxTypeFareWithFareAndTariffXRef();
    CategoryRuleInfo cri;

    // FAIL if not OptIn
    _trx->getRequest()->ticketingAgent()->agentTJR()->optInAgency() = 'N';

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->locType() = RuleConst::TRAVEL_AGENCY;
    loc1->loc() = agency_B4T0;

    LocKey* loc2 = _memHandle.create<LocKey>();
    loc2->locType() = RuleConst::HOME_TRAVEL_AGENCY;
    loc2->loc() = SalesRestrictionRule::OPT_IN_AGENCY;

    LocKey* loc3 = _memHandle.create<LocKey>();
    loc3->locType() = LOCTYPE_NATION; //'N';
    loc3->loc() = NATION_US;

    // 3. Test Case #2 (FAIL)
    // N BET0
    // N 88D7
    // Y N US
    // Locales will be deleted by the SalseRestriction
    Locale* locale1 = new Locale();
    locale1->locAppl() = RuleConst::TKTS_MAY_NOT_BE_SOLD;
    locale1->loc1() = *loc1;

    Locale* locale2 = new Locale();
    locale2->locAppl() = RuleConst::TKTS_MAY_NOT_BE_SOLD;
    locale2->loc1() = *loc2;

    Locale* locale3 = new Locale();
    locale3->locAppl() = RuleConst::TKTS_MAY_ONLY_BE_SOLD;
    locale3->loc1() = *loc3;

    _sr->locales() += locale1, locale2, locale3;
    _sr->vendor() = ATPCO_VENDOR_CODE;

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(ret = tse::checkLocaleItems(*_srr, *_trx, 0, *_paxTypeFare, cri, _sr, false));
    CPPUNIT_ASSERT(!ret);
  }

  void testCheckLocaleItemsPassWhenOptInAndPCCMatchAndPrivateTarrif()
  {
    _trx = createPricingTrxForCheckLocalItems();
    createBasicPaxTypeFareWithFareAndTariffXRef();
    CategoryRuleInfo cri;

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->locType() = RuleConst::TRAVEL_AGENCY;
    loc1->loc() = agency_B4T0;

    LocKey* loc2 = _memHandle.create<LocKey>();
    loc2->locType() = RuleConst::HOME_TRAVEL_AGENCY;
    loc2->loc() = SalesRestrictionRule::OPT_IN_AGENCY;

    _sr->vendor() = ATPCO_VENDOR_CODE;

    // Test Case #3 (PASS)
    // N B4T0
    // Y 88D7
    // Locales will be deleted by the SalesRestriction
    Locale* locale1 = new Locale();
    locale1->locAppl() = RuleConst::TKTS_MAY_NOT_BE_SOLD;
    locale1->loc1() = *loc1;

    Locale* locale2 = new Locale();
    locale2->locAppl() = RuleConst::TKTS_MAY_ONLY_BE_SOLD;
    locale2->loc1() = *loc2;

    _sr->locales() += locale1, locale2;

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(ret = tse::checkLocaleItems(*_srr, *_trx, 0, *_paxTypeFare, cri, _sr, false));
    CPPUNIT_ASSERT(ret);
  }

  // N 88D7
  // Y B4T0
  void testCheckLocaleItemsPass_N88D7_YB4T0()
  {
    _trx = createPricingTrxForCheckLocalItems();
    createBasicPaxTypeFareWithFareAndTariffXRef();
    CategoryRuleInfo cri;

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->locType() = RuleConst::TRAVEL_AGENCY;
    loc1->loc() = agency_B4T0;

    LocKey* loc2 = _memHandle.create<LocKey>();
    loc2->locType() = RuleConst::HOME_TRAVEL_AGENCY;
    loc2->loc() = SalesRestrictionRule::OPT_IN_AGENCY;

    _sr->vendor() = ATPCO_VENDOR_CODE;

    // 7. Test Case #4 (PASS)
    // N 88D7
    // Y B4T0
    Locale* locale1 = new Locale();
    locale1->locAppl() = RuleConst::TKTS_MAY_NOT_BE_SOLD;
    locale1->loc1() = *loc2;

    Locale* locale2 = new Locale();
    locale2->locAppl() = RuleConst::TKTS_MAY_ONLY_BE_SOLD;
    locale2->loc1() = *loc1;

    _sr->locales() += locale1, locale2;

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(ret = tse::checkLocaleItems(*_srr, *_trx, 0, *_paxTypeFare, cri, _sr, false));
    CPPUNIT_ASSERT(ret);
  }

  // N 88D7
  // Y B4T0
  void testCheckLocaleItemsPass_N88D7_YB4T0_OptISetToNo()
  {
    _trx = createPricingTrxForCheckLocalItems();
    createBasicPaxTypeFareWithFareAndTariffXRef();
    CategoryRuleInfo cri;

    // PASS if not OptIn
    _trx->getRequest()->ticketingAgent()->agentTJR()->optInAgency() = 'N';

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->locType() = RuleConst::TRAVEL_AGENCY;
    loc1->loc() = agency_B4T0;

    LocKey* loc2 = _memHandle.create<LocKey>();
    loc2->locType() = RuleConst::HOME_TRAVEL_AGENCY;
    loc2->loc() = SalesRestrictionRule::OPT_IN_AGENCY;

    _sr->vendor() = ATPCO_VENDOR_CODE;

    // Test Case #4 (PASS)
    // N 88D7
    // Y B4T0
    Locale* locale1 = new Locale();
    locale1->locAppl() = RuleConst::TKTS_MAY_NOT_BE_SOLD;
    locale1->loc1() = *loc2;

    Locale* locale2 = new Locale();
    locale2->locAppl() = RuleConst::TKTS_MAY_ONLY_BE_SOLD;
    locale2->loc1() = *loc1;

    _sr->locales() += locale1, locale2;

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(ret = tse::checkLocaleItems(*_srr, *_trx, 0, *_paxTypeFare, cri, _sr, false));
    CPPUNIT_ASSERT(ret);
  }

  // N 88D7
  // Y N US
  void testCheckLocaleItemsPass_N88D7_YNUS()
  {
    _trx = createPricingTrxForCheckLocalItems();
    createBasicPaxTypeFareWithFareAndTariffXRef();
    CategoryRuleInfo cri;

    // PASS if not OptIn
    _trx->getRequest()->ticketingAgent()->agentTJR()->optInAgency() = 'N';

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->locType() = RuleConst::HOME_TRAVEL_AGENCY;
    loc1->loc() = SalesRestrictionRule::OPT_IN_AGENCY;

    LocKey* loc2 = _memHandle.create<LocKey>();
    loc2->locType() = LOCTYPE_NATION; //'N';
    loc2->loc() = NATION_US;

    // Test Case #5 (PASS)
    // N 88D7
    // Y N US
    Locale* locale1 = new Locale();
    locale1->locAppl() = RuleConst::TKTS_MAY_NOT_BE_SOLD;
    locale1->loc1() = *loc1;

    Locale* locale2 = new Locale();
    locale2->locAppl() = RuleConst::TKTS_MAY_ONLY_BE_SOLD;
    locale2->loc1() = *loc2;

    _sr->locales() += locale1, locale2;
    _sr->vendor() = ATPCO_VENDOR_CODE;

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(ret = tse::checkLocaleItems(*_srr, *_trx, 0, *_paxTypeFare, cri, _sr, false));
    CPPUNIT_ASSERT(ret);
  }

  // N 88D7
  void testCheckLocaleItemsFail_N88D7_MatchPCCFailOptIn()
  {
    _trx = createPricingTrxForCheckLocalItems();
    createBasicPaxTypeFareWithFareAndTariffXRef();
    CategoryRuleInfo cri;

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->locType() = RuleConst::HOME_TRAVEL_AGENCY;
    loc1->loc() = SalesRestrictionRule::OPT_IN_AGENCY;

    // Test Case #6 (FAIL)
    // N 88D7
    Locale* locale1 = new Locale();
    locale1->locAppl() = RuleConst::TKTS_MAY_NOT_BE_SOLD;
    locale1->loc1() = *loc1;

    _sr->locales() += locale1;
    _sr->vendor() = ATPCO_VENDOR_CODE;

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(ret = tse::checkLocaleItems(*_srr, *_trx, 0, *_paxTypeFare, cri, _sr, false));
    CPPUNIT_ASSERT(!ret);
  }

  // N 88D7
  void testCheckLocaleItemsPass_N88D7()
  {
    _trx = createPricingTrxForCheckLocalItems();
    createBasicPaxTypeFareWithFareAndTariffXRef();
    CategoryRuleInfo cri;

    // PASS if not OptIn
    _trx->getRequest()->ticketingAgent()->agentTJR()->optInAgency() = 'N';

    // Test Case #6 (FAIL?)
    // N 88D7

    _sr->vendor() = ATPCO_VENDOR_CODE;

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(ret = tse::checkLocaleItems(*_srr, *_trx, 0, *_paxTypeFare, cri, _sr, false));
    CPPUNIT_ASSERT(ret);
  }

  void testCheckLocaleItemsPass_YNFR()
  {
    createBasicPaxTypeFareWithFareAndTariffXRef();
    CategoryRuleInfo cri;

    // CWT
    createAgentTjrWithSsgGroup(Agent::CWT_GROUP_NUMBER);
    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = "F8RA";
    _trx->getRequest()->ticketingAgent()->agentLocation() = _frAgentLoc;

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->locType() = LOCTYPE_NATION;
    loc1->loc() = NATION_FRANCE;

    Locale* locale1 = new Locale();
    locale1->locAppl() = RuleConst::TKTS_MAY_ONLY_BE_SOLD;
    locale1->loc1() = *loc1;

    _sr->locales() += locale1;
    _sr->vendor() = ATPCO_VENDOR_CODE;

    cri.categoryNumber() = 15;

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(ret = tse::checkLocaleItems(*_srr, *_trx, 0, *_paxTypeFare, cri, _sr, false));
    CPPUNIT_ASSERT(ret);

    _sr->locales().clear();
  }

  void testCheckLocaleItemsPassWhenLocalesIsEmpty()
  {
    CategoryRuleInfo cri;
    CPPUNIT_ASSERT(tse::checkLocaleItems(*_srr, *_trx, 0, *_paxTypeFare, cri, _sr, false));
  }

  void testDiag355NotActive()
  {
    createBasicPaxTypeFareWithFareMarketParToLon();
    CategoryRuleItemInfo rule;
    SalesRestrictionRule::Diag355 diag355(*_trx, _sr, &rule, *_paxTypeFare);
    CPPUNIT_ASSERT(!diag355._diagEnabled);
  }

  void testDeterminePricingCurrencyFail()
  {
    CurrencyCode pricingCurrency;
    Nation nation;

    nation.primeCur() = EMPTY_STRING();
    nation.alternateCur() = EMPTY_STRING();
    nation.conversionCur() = EMPTY_STRING();

    CPPUNIT_ASSERT(!_srr->determinePricingCurrency(nation, pricingCurrency));
    CPPUNIT_ASSERT(pricingCurrency.empty());
  }

  void testDeterminePricingCurrencyPassPrimeCurr()
  {
    CurrencyCode pricingCurrency;
    Nation nation;

    nation.primeCur() = EUR;
    nation.alternateCur() = USD;
    nation.conversionCur() = GBP;

    CPPUNIT_ASSERT(_srr->determinePricingCurrency(nation, pricingCurrency));
    CPPUNIT_ASSERT_EQUAL(pricingCurrency, nation.primeCur());
  }

  void testDeterminePricingCurrencyPassAlternateCurr()
  {
    CurrencyCode pricingCurrency;
    Nation nation;

    nation.primeCur() = EMPTY_STRING();
    nation.alternateCur() = USD;
    nation.conversionCur() = GBP;

    CPPUNIT_ASSERT(_srr->determinePricingCurrency(nation, pricingCurrency));
    CPPUNIT_ASSERT_EQUAL(pricingCurrency, nation.alternateCur());
  }

  void testDeterminePricingCurrencyPassConversionCurr()
  {
    CurrencyCode pricingCurrency;
    Nation nation;

    nation.primeCur() = EMPTY_STRING();
    nation.alternateCur() = EMPTY_STRING();
    nation.conversionCur() = GBP;

    CPPUNIT_ASSERT(_srr->determinePricingCurrency(nation, pricingCurrency));
    CPPUNIT_ASSERT_EQUAL(pricingCurrency, nation.conversionCur());
  }

  void testValidateCarrierWqWarnigSet()
  {
    Itin itin;
    bool isWqTrx = true;
    _paxTypeFare->warningMap().set(WarningMap::cat15_warning_1, false);
    _srr->validateCarrier(*_trx, itin.validatingCarrier(), *_paxTypeFare, _sr, isWqTrx);
    CPPUNIT_ASSERT(_paxTypeFare->warningMap().isSet(WarningMap::cat15_warning_1));
  }

  void testValidateCarrierWqWarnigNotSet()
  {
    Itin itin;
    bool isWqTrx = false;
    _paxTypeFare->warningMap().set(WarningMap::cat15_warning_1, false);
    _srr->validateCarrier(*_trx, itin.validatingCarrier(), *_paxTypeFare, _sr, isWqTrx);
    CPPUNIT_ASSERT(!_paxTypeFare->warningMap().isSet(WarningMap::cat15_warning_1));
  }

  void testValidateCarrierPassValidatingCarrierEqualToOtherCarrier()
  {
    Itin itin;
    itin.validatingCarrier() = carrier_AA;
    _sr->otherCarrier() = carrier_AA;
    CPPUNIT_ASSERT(
        _srr->validateCarrier(*_trx, itin.validatingCarrier(), *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierPassCarrierNotJointCarrier()
  {
    Itin itin;
    itin.validatingCarrier() = carrier_AA;
    _sr->otherCarrier() = carrier_LH;

    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_AA);

    CPPUNIT_ASSERT(
        _srr->validateCarrier(*_trx, itin.validatingCarrier(), *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierPassAllianceMatchRTW()
  {
    _trx->getOptions()->setRtw(true);
    Itin itin;
    itin.validatingCarrier() = carrier_AA;
    _sr->otherCarrier() = ONE_WORLD_ALLIANCE;

    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_AA);

    CPPUNIT_ASSERT(
        _srr->validateCarrier(*_trx, itin.validatingCarrier(), *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierFailCarrierNotJointCarrier()
  {
    Itin itin;
    itin.validatingCarrier() = carrier_AA;
    _sr->otherCarrier() = carrier_LH;
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_UA);
    CPPUNIT_ASSERT(
        !_srr->validateCarrier(*_trx, itin.validatingCarrier(), *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierFailJointCarrier()
  {
    Itin itin;
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(JOINT_CARRIER);
    itin.validatingCarrier() = carrier_AA;
    _sr->otherCarrier() = carrier_LH;
    CPPUNIT_ASSERT(
        !_srr->validateCarrier(*_trx, itin.validatingCarrier(), *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierPassJointCarrier()
  {
    Itin itin;
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(JOINT_CARRIER);
    itin.validatingCarrier() = carrier_AA;
    _sr->otherCarrier() = carrier_LH;
    AirSeg* seg = createAirSegParToLon(); // carrier AA
    _paxTypeFare->fareMarket()->travelSeg() += seg;
    ;
    CPPUNIT_ASSERT(
        _srr->validateCarrier(*_trx, itin.validatingCarrier(), *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierRestrictionSegmentWqWarningSet()
  {
    Itin itin;
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_UA);
    _paxTypeFare->actualPaxType() = _memHandle.create<PaxType>();
    _paxTypeFare->actualPaxType()->paxType() = ADULT;
    bool isWqTrx = true;
    _paxTypeFare->warningMap().set(WarningMap::cat15_warning_1, false);
    _srr->validateCarrierRestrictionSegment(*_trx, itin, *_paxTypeFare, _sr, isWqTrx);
    CPPUNIT_ASSERT(_paxTypeFare->warningMap().isSet(WarningMap::cat15_warning_1));
  }

  void testValidateCarrierRestrictionSegmentWqWarningNotSet()
  {
    Itin itin;
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_UA);
    _paxTypeFare->actualPaxType() = _memHandle.create<PaxType>();
    _paxTypeFare->actualPaxType()->paxType() = ADULT;
    bool isWqTrx = false;
    _paxTypeFare->warningMap().set(WarningMap::cat15_warning_1, false);
    _srr->validateCarrierRestrictionSegment(*_trx, itin, *_paxTypeFare, _sr, isWqTrx);
    CPPUNIT_ASSERT(!_paxTypeFare->warningMap().isSet(WarningMap::cat15_warning_1));
  }

  void testValidateCarrierRestrictionSegmentPaxTypeAdtPaxTypeFareCarrierNotJointPass()
  {
    Itin itin;
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_UA);
    _paxTypeFare->actualPaxType() = _memHandle.create<PaxType>();
    _paxTypeFare->actualPaxType()->paxType() = ADULT;
    CPPUNIT_ASSERT(_srr->validateCarrierRestrictionSegment(*_trx, itin, *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierRestrictionSegmentPaxTypeNegMatchPass()
  {
    Itin itin;
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_UA);
    _paxTypeFare->actualPaxType() = _memHandle.create<PaxType>();
    _paxTypeFare->actualPaxType()->paxType() = RuleConst::MATCH_NEG_PAX_TYPE;
    CPPUNIT_ASSERT(_srr->validateCarrierRestrictionSegment(*_trx, itin, *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierRestrictionSegmentIsTicketEntryPass()
  {
    _trx->getRequest()->ticketEntry() = NO;
    Itin itin;
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_UA);
    _paxTypeFare->actualPaxType() = _memHandle.create<PaxType>();
    _paxTypeFare->actualPaxType()->paxType() = RuleConst::MATCH_NEG_PAX_TYPE;
    _sr->otherCarrier() = EMPTY_STRING();
    AirSeg* seg = createAirSegParToLon(); // carrier AA
    AirSeg* seg2 = createAirSegLonToPar(); // carrier AA
    seg->carrier() = carrier_LH;
    itin.travelSeg() += seg, seg2;
    ;
    CPPUNIT_ASSERT(_srr->validateCarrierRestrictionSegment(*_trx, itin, *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierRestrictionSegmentIsTicketEntryFail()
  {
    _trx->getRequest()->ticketEntry() = YES;
    Itin itin;
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_UA);
    _paxTypeFare->actualPaxType() = _memHandle.create<PaxType>();
    _paxTypeFare->actualPaxType()->paxType() = RuleConst::MATCH_NEG_PAX_TYPE;
    _sr->otherCarrier() = EMPTY_STRING();
    AirSeg* seg = createAirSegParToLon(); // carrier AA
    AirSeg* seg2 = createAirSegLonToPar(); // carrier AA
    seg->carrier() = carrier_LH;
    itin.travelSeg() += seg, seg2;
    ;
    CPPUNIT_ASSERT(
        !_srr->validateCarrierRestrictionSegment(*_trx, itin, *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierRestrictionSegmentPaxTypeFareJointCarrierMatchFoundPass()
  {
    _trx->getRequest()->ticketEntry() = NO;
    Itin itin;
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(JOINT_CARRIER);
    _paxTypeFare->actualPaxType() = _memHandle.create<PaxType>();
    _paxTypeFare->actualPaxType()->paxType() = ADULT; //= RuleConst::MATCH_NEG_PAX_TYPE;

    AirSeg* airSeg = createAirSegParToLon(); // carrier AA
    _paxTypeFare->fareMarket()->travelSeg() += airSeg;
    AirSeg* seg = createAirSegParToLon(); // carrier AA
    itin.travelSeg() += seg;
    ;
    CPPUNIT_ASSERT(_srr->validateCarrierRestrictionSegment(*_trx, itin, *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierRestrictionSegmentPaxTypeFareJointCarrierMatchNotFoundPass()
  {
    _trx->getRequest()->ticketEntry() = NO;
    Itin itin;
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(JOINT_CARRIER);
    _paxTypeFare->actualPaxType() = _memHandle.create<PaxType>();
    _paxTypeFare->actualPaxType()->paxType() = RuleConst::MATCH_NEG_PAX_TYPE;
    AirSeg* seg = createAirSegParToLon(); // carrier AA
    AirSeg* seg2 = createAirSegLonToPar(); // carrier AA
    seg->carrier() = carrier_LH;
    itin.travelSeg() += seg, seg2;
    CPPUNIT_ASSERT(_srr->validateCarrierRestrictionSegment(*_trx, itin, *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierRestrictionSegmentPaxTypeFareJointCarrierMatchNotFoundFail()
  {
    Itin itin;
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(JOINT_CARRIER);
    _paxTypeFare->actualPaxType() = _memHandle.create<PaxType>();
    _paxTypeFare->actualPaxType()->paxType() = ADULT;
    AirSeg* seg = createAirSegParToLon(); // carrier AA
    AirSeg* seg2 = createAirSegLonToPar(); // carrier AA
    seg->carrier() = carrier_LH;
    itin.travelSeg() += seg, seg2;
    CPPUNIT_ASSERT(
        !_srr->validateCarrierRestrictionSegment(*_trx, itin, *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierRestrictionFareWqWarningSet()
  {
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(JOINT_CARRIER);
    bool isWqTrx = true;
    _paxTypeFare->warningMap().set(WarningMap::cat15_warning_1, false);
    _srr->validateCarrierRestrictionFare(*_trx, *_paxTypeFare, _sr, isWqTrx);
    CPPUNIT_ASSERT(_paxTypeFare->warningMap().isSet(WarningMap::cat15_warning_1));
  }

  void testValidateCarrierRestrictionFareWqWarningNotSet()
  {
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(JOINT_CARRIER);
    bool isWqTrx = false;
    _paxTypeFare->warningMap().set(WarningMap::cat15_warning_1, false);
    _srr->validateCarrierRestrictionFare(*_trx, *_paxTypeFare, _sr, isWqTrx);
    CPPUNIT_ASSERT(!_paxTypeFare->warningMap().isSet(WarningMap::cat15_warning_1));
  }

  void testValidateCarrierRestrictionFarePassJointCarrier()
  {
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(JOINT_CARRIER);
    CPPUNIT_ASSERT(_srr->validateCarrierRestrictionFare(*_trx, *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierRestrictionFarePassNoTravelSegments()
  {
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_AA);
    CPPUNIT_ASSERT(_srr->validateCarrierRestrictionFare(*_trx, *_paxTypeFare, _sr, false));
  }

  void testValidateCarrierRestrictionFareFail()
  {
    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_UA);
    AirSeg* airSeg = createAirSegParToLon(); // carrier AA
    _paxTypeFare->fareMarket()->travelSeg() += airSeg;
    CPPUNIT_ASSERT(!_srr->validateCarrierRestrictionFare(*_trx, *_paxTypeFare, _sr, false));
  }

  void testDetermineCountryLocNoOverride()
  {
    _trx->getRequest()->salePointOverride() = EMPTY_STRING();
    _trx->getRequest()->ticketPointOverride() = EMPTY_STRING();
    _trx->getRequest()->ticketingAgent()->agentLocation() = createTicketingAgentKrk();
    const Loc* loc2 = _srr->determineCountryLoc(*_trx);
    CPPUNIT_ASSERT_EQUAL(_trx->getRequest()->ticketingAgent()->agentLocation()->loc(), loc2->loc());
  }

  void testChackCarrierMatchForJointCarrierPassWhenNoTravelSegments()
  {
    Itin itin;
    CPPUNIT_ASSERT(_srr->checkCarrierMatchForJointCarrier(*_trx, itin, *_paxTypeFare, _sr));
  }

  void testChackCarrierMatchForJointCarrierPassWhenCarrierMatch()
  {
    Itin itin;
    addTwoAirSegmentsToItin(itin);
    addTwoAirSegmentsToFareMarket();
    CPPUNIT_ASSERT(_srr->checkCarrierMatchForJointCarrier(*_trx, itin, *_paxTypeFare, _sr));
  }

  void testChackCarrierMatchForJointCarrierPassWhenCarrierNoMatchAndIsNotTicketEntry()
  {
    _trx->getRequest()->ticketEntry() = NO;
    Itin itin;
    addOneAirSegmentToItinWtihCarrierAA(itin);
    addOneAirSegmentToFareMarketWithCarrierBA();
    _paxTypeFare->actualPaxType() = _memHandle.create<PaxType>();
    _paxTypeFare->actualPaxType()->paxType() = RuleConst::MATCH_NEG_PAX_TYPE;
    CPPUNIT_ASSERT(_srr->checkCarrierMatchForJointCarrier(*_trx, itin, *_paxTypeFare, _sr));
  }

  void testChackCarrierMatchForJointCarrierPassWhenCarrierNoMatchAndIsNotTicketEntryRTW()
  {
    _trx->getRequest()->ticketEntry() = NO;
    _trx->getOptions()->setRtw(true);
    Itin itin;
    addOneAirSegmentToItinWtihCarrierAA(itin);
    addOneAirSegmentToFareMarketWithCarrierBA();
    _paxTypeFare->actualPaxType() = _memHandle.create<PaxType>();
    _paxTypeFare->actualPaxType()->paxType() = RuleConst::MATCH_NEG_PAX_TYPE;
    _sr->otherCarrier() = ONE_WORLD_ALLIANCE;
    CPPUNIT_ASSERT(_srr->checkCarrierMatchForJointCarrier(*_trx, itin, *_paxTypeFare, _sr));
  }

  void testChackCarrierMatchForJointCarrierFailWhenCarrierNoMatchAndIsTicketEntry()
  {
    _trx->getRequest()->ticketEntry() = YES;
    Itin itin;
    addOneAirSegmentToItinWtihCarrierAA(itin);
    addOneAirSegmentToFareMarketWithCarrierBA();
    _paxTypeFare->actualPaxType() = _memHandle.create<PaxType>();
    _paxTypeFare->actualPaxType()->paxType() = RuleConst::MATCH_NEG_PAX_TYPE;
    CPPUNIT_ASSERT(!_srr->checkCarrierMatchForJointCarrier(*_trx, itin, *_paxTypeFare, _sr));
  }

  void testSetFrInCat15NothingSetWhenNotCwtUser()
  {
    LocKey locKey1, locKey2;
    setNationFrance(locKey1, locKey2);
    CategoryRuleInfo cri;
    cri.categoryNumber() = RuleConst::SALE_RESTRICTIONS_RULE;
    createAgentTjrWithSsgGroup(Agent::CWT_GROUP_NUMBER + 1);
    createBasicPaxTypeFareWithFareAndTariffXRef();

    _srr->setFrInCat15(
        locKey1, locKey2, true, true, cri, *_trx->getRequest()->ticketingAgent(), *_paxTypeFare);

    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Gr());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fr());
  }

  void testSetFrInCat15NothingSetWhenNotPrivateTariff()
  {
    LocKey locKey1, locKey2;
    setNationFrance(locKey1, locKey2);
    CategoryRuleInfo cri;
    cri.categoryNumber() = RuleConst::SALE_RESTRICTIONS_RULE;
    createAgentTjrWithSsgGroup(Agent::CWT_GROUP_NUMBER);
    createBasicPaxTypeFareWithFareAndTariffXRef(0);

    _srr->setFrInCat15(
        locKey1, locKey2, true, true, cri, *_trx->getRequest()->ticketingAgent(), *_paxTypeFare);

    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Gr());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fr());
  }

  void testSetFrInCat15NothingSetWhenNoMatchRestriction()
  {
    LocKey locKey1, locKey2;
    setNationFrance(locKey1, locKey2);
    CategoryRuleInfo cri;
    cri.categoryNumber() = RuleConst::SALE_RESTRICTIONS_RULE;
    createAgentTjrWithSsgGroup(Agent::CWT_GROUP_NUMBER);
    createBasicPaxTypeFareWithFareAndTariffXRef();

    _srr->setFrInCat15(
        locKey1, locKey2, false, false, cri, *_trx->getRequest()->ticketingAgent(), *_paxTypeFare);

    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Gr());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fr());
  }

  void testSetFrInCat15NothingSetWhenLocTypeNotNation()
  {
    LocKey locKey1, locKey2;
    setNationFrance(locKey1, locKey2);
    locKey1.locType() = LOCTYPE_CITY;
    locKey2.locType() = LOCTYPE_STATE;
    CategoryRuleInfo cri;
    cri.categoryNumber() = RuleConst::SALE_RESTRICTIONS_RULE;
    createAgentTjrWithSsgGroup(Agent::CWT_GROUP_NUMBER);
    createBasicPaxTypeFareWithFareAndTariffXRef();

    _srr->setFrInCat15(
        locKey1, locKey2, true, true, cri, *_trx->getRequest()->ticketingAgent(), *_paxTypeFare);

    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Gr());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fr());
  }

  void testSetFrInCat15SetFrWhenCwtUserPrivateTariffmatchSaleRestrloc1FrCategory15()
  {
    LocKey locKey1, locKey2;
    setNationFrance(locKey1, locKey2);
    CategoryRuleInfo cri;
    cri.categoryNumber() = RuleConst::SALE_RESTRICTIONS_RULE;
    createAgentTjrWithSsgGroup(Agent::CWT_GROUP_NUMBER);
    createBasicPaxTypeFareWithFareAndTariffXRef();

    _srr->setFrInCat15(
        locKey1, locKey2, true, true, cri, *_trx->getRequest()->ticketingAgent(), *_paxTypeFare);

    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Gr());
    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15Fr());
  }

  void testSetFrInCat15SetFrFnWhenTypeIsFootNote()
  {
    LocKey locKey1, locKey2;
    setNationFrance(locKey1, locKey2);
    FootNoteCtrlInfo cri;
    cri.categoryNumber() = RuleConst::SALE_RESTRICTIONS_RULE;
    createAgentTjrWithSsgGroup(Agent::CWT_GROUP_NUMBER);
    createBasicPaxTypeFareWithFareAndTariffXRef();

    _srr->setFrInCat15(
        locKey1, locKey2, true, true, cri, *_trx->getRequest()->ticketingAgent(), *_paxTypeFare);

    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15());
    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Gr());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fr());
  }

  void testSetFrInCat15SetGrFnWhenTypeIsFootNote()
  {
    LocKey locKey1, locKey2;
    setNationFrance(locKey1, locKey2);
    CategoryRuleInfo cri;
    cri.categoryNumber() = RuleConst::SALE_RESTRICTIONS_RULE;
    createAgentTjrWithSsgGroup(Agent::CWT_GROUP_NUMBER);
    createBasicPaxTypeFareWithFareAndTariffXRef();
    _paxTypeFare->fare()->status().set(Fare::FS_Cat15GeneralRuleProcess);

    _srr->setFrInCat15(
        locKey1, locKey2, true, true, cri, *_trx->getRequest()->ticketingAgent(), *_paxTypeFare);

    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15Gr());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fr());
  }

  void testfindCorpIdMatchFailWhenLocalesEmpty()
  {
    PseudoCityCode pcc = "ABC";
    std::vector<Locale*> locales;
    PosPaxType posPaxType;
    bool matchedCorpId = false;

    CPPUNIT_ASSERT(!_srr->findCorpIdMatch(pcc, locales, &posPaxType, matchedCorpId));
  }

  void testfindCorpIdMatchFailWhenLocTypeIsNotTravelAgencyAndNotHomeAgency()
  {
    PseudoCityCode pcc = "ABC";
    std::vector<Locale*> locales;
    locales +=
        _memHandle(createLocale(RuleConst::IATA_TVL_AGENCY_NO, RuleConst::IATA_TVL_AGENCY_NO));
    PosPaxType posPaxType;
    bool matchedCorpId = false;

    CPPUNIT_ASSERT(!_srr->findCorpIdMatch(pcc, locales, &posPaxType, matchedCorpId));
  }

  void testfindCorpIdMatchFailWhenLocTypeIsTravelAgencyLocNotMatchPcc()
  {
    PseudoCityCode pcc = "CBA";
    std::vector<Locale*> locales;
    locales += _memHandle(createLocale(RuleConst::TRAVEL_AGENCY, RuleConst::TRAVEL_AGENCY));
    PosPaxType posPaxType;
    bool matchedCorpId = false;

    CPPUNIT_ASSERT(!_srr->findCorpIdMatch(pcc, locales, &posPaxType, matchedCorpId));
  }

  void testfindCorpIdMatchFailWhenLocTypeIsHomeTravelAgencyLocNotMatchPcc()
  {
    PseudoCityCode pcc = "CBA";
    std::vector<Locale*> locales;
    locales +=
        _memHandle(createLocale(RuleConst::HOME_TRAVEL_AGENCY, RuleConst::HOME_TRAVEL_AGENCY));
    PosPaxType posPaxType;
    bool matchedCorpId = false;

    CPPUNIT_ASSERT(!_srr->findCorpIdMatch(pcc, locales, &posPaxType, matchedCorpId));
  }

  void testfindCorpIdMatchPassWhenMatchedCorpIdAndCorpIdNotEmptyForLoc1()
  {
    PseudoCityCode pcc = "ABC";
    std::vector<Locale*> locales;
    locales +=
        _memHandle(createLocale(RuleConst::HOME_TRAVEL_AGENCY, RuleConst::IATA_TVL_AGENCY_NO));
    PosPaxType posPaxType;
    posPaxType.corpID() = "A";
    bool matchedCorpId = true;

    CPPUNIT_ASSERT(_srr->findCorpIdMatch(pcc, locales, &posPaxType, matchedCorpId));
  }

  void testfindCorpIdMatchPassWhenNoMatchedCorpIdAndCorpIdEmptyForLoc1()
  {
    PseudoCityCode pcc = "ABC";
    std::vector<Locale*> locales;
    locales +=
        _memHandle(createLocale(RuleConst::HOME_TRAVEL_AGENCY, RuleConst::IATA_TVL_AGENCY_NO));
    PosPaxType posPaxType;
    posPaxType.corpID() = EMPTY_STRING();
    bool matchedCorpId = false;

    CPPUNIT_ASSERT(_srr->findCorpIdMatch(pcc, locales, &posPaxType, matchedCorpId));
  }

  void testfindCorpIdMatchPassWhenMatchedCorpIdAndCorpIdNotEmptyForLoc2()
  {
    PseudoCityCode pcc = "ABC";
    std::vector<Locale*> locales;
    locales +=
        _memHandle(createLocale(RuleConst::IATA_TVL_AGENCY_NO, RuleConst::HOME_TRAVEL_AGENCY));
    PosPaxType posPaxType;
    posPaxType.corpID() = "A";
    bool matchedCorpId = true;

    CPPUNIT_ASSERT(_srr->findCorpIdMatch(pcc, locales, &posPaxType, matchedCorpId));
  }

  void testfindCorpIdMatchPassWhenNoMatchedCorpIdAndCorpIdEmptyForLoc2()
  {
    PseudoCityCode pcc = "ABC";
    std::vector<Locale*> locales;
    locales +=
        _memHandle(createLocale(RuleConst::IATA_TVL_AGENCY_NO, RuleConst::HOME_TRAVEL_AGENCY));
    PosPaxType posPaxType;
    posPaxType.corpID() = EMPTY_STRING();
    bool matchedCorpId = false;

    CPPUNIT_ASSERT(_srr->findCorpIdMatch(pcc, locales, &posPaxType, matchedCorpId));
  }

  void testSetFlagsPassWhenLocalesEmpty()
  {
    CategoryRuleInfo cri;
    bool mustBeSold = false;
    bool mustBeTicket = false;
    bool matchSaleRestriction = false;
    bool matchTktRestriction = false;
    bool negativeMatchOptIn = false;
    bool negativeMatchPCC = false;
    CPPUNIT_ASSERT(_srr->setFlags(*_trx,
                                  *_paxTypeFare,
                                  cri,
                                  _sr,
                                  mustBeSold,
                                  mustBeTicket,
                                  matchSaleRestriction,
                                  matchTktRestriction,
                                  negativeMatchOptIn,
                                  negativeMatchPCC,
                                  false));
  }

  void testSetFlagsPassWhenMatchSaleRestrAndLocApplIsTktsMayOnlyBeSold()
  {
    CategoryRuleInfo cri;
    _sr->locales() += createLocale(RuleConst::IATA_TVL_AGENCY_NO,
                                   RuleConst::HOME_TRAVEL_AGENCY,
                                   RuleConst::TKTS_MAY_ONLY_BE_SOLD);
    bool mustBeSold = false;
    bool mustBeTicket = false;
    bool matchSaleRestriction = true;
    bool matchTktRestriction = false;
    bool negativeMatchOptIn = false;
    bool negativeMatchPCC = false;
    CPPUNIT_ASSERT(_srr->setFlags(*_trx,
                                  *_paxTypeFare,
                                  cri,
                                  _sr,
                                  mustBeSold,
                                  mustBeTicket,
                                  matchSaleRestriction,
                                  matchTktRestriction,
                                  negativeMatchOptIn,
                                  negativeMatchPCC,
                                  false));
  }

  void testSetFlagsPassWhenMatchSaleRestrAndLocApplIsTktsMayNotBeSold()
  {
    CategoryRuleInfo cri;
    _sr->locales() += createLocale(RuleConst::IATA_TVL_AGENCY_NO,
                                   RuleConst::HOME_TRAVEL_AGENCY,
                                   RuleConst::TKTS_MAY_NOT_BE_SOLD);
    bool mustBeSold = false;
    bool mustBeTicket = false;
    bool matchSaleRestriction = true;
    bool matchTktRestriction = false;
    bool negativeMatchOptIn = false;
    bool negativeMatchPCC = false;
    CPPUNIT_ASSERT(_srr->setFlags(*_trx,
                                  *_paxTypeFare,
                                  cri,
                                  _sr,
                                  mustBeSold,
                                  mustBeTicket,
                                  matchSaleRestriction,
                                  matchTktRestriction,
                                  negativeMatchOptIn,
                                  negativeMatchPCC,
                                  false));
  }

  void testSetFlagsPassWhenMatchTktRestrAndLocApplIsTktsMayOnlyBeIssued()
  {
    CategoryRuleInfo cri;
    _sr->locales() += createLocale(RuleConst::IATA_TVL_AGENCY_NO,
                                   RuleConst::HOME_TRAVEL_AGENCY,
                                   RuleConst::TKTS_MAY_ONLY_BE_ISSUED);
    bool mustBeSold = false;
    bool mustBeTicket = false;
    bool matchSaleRestriction = false;
    bool matchTktRestriction = true;
    bool negativeMatchOptIn = false;
    bool negativeMatchPCC = false;
    CPPUNIT_ASSERT(_srr->setFlags(*_trx,
                                  *_paxTypeFare,
                                  cri,
                                  _sr,
                                  mustBeSold,
                                  mustBeTicket,
                                  matchSaleRestriction,
                                  matchTktRestriction,
                                  negativeMatchOptIn,
                                  negativeMatchPCC,
                                  false));
  }

  void testSetFlagsPassWhenMatchTktRestrAndLocApplIsTktsMayNotBeIssued()
  {
    CategoryRuleInfo cri;
    _sr->locales() += createLocale(RuleConst::IATA_TVL_AGENCY_NO,
                                   RuleConst::HOME_TRAVEL_AGENCY,
                                   RuleConst::TKTS_MAY_NOT_BE_ISSUED);
    bool mustBeSold = false;
    bool mustBeTicket = false;
    bool matchSaleRestriction = false;
    bool matchTktRestriction = true;
    bool negativeMatchOptIn = false;
    bool negativeMatchPCC = false;
    CPPUNIT_ASSERT(_srr->setFlags(*_trx,
                                  *_paxTypeFare,
                                  cri,
                                  _sr,
                                  mustBeSold,
                                  mustBeTicket,
                                  matchSaleRestriction,
                                  matchTktRestriction,
                                  negativeMatchOptIn,
                                  negativeMatchPCC,
                                  false));
  }

  void testSetFlagsWqWarningSet()
  {
    CategoryRuleInfo cri;
    _sr->locales() += createLocale(RuleConst::IATA_TVL_AGENCY_NO,
                                   RuleConst::HOME_TRAVEL_AGENCY,
                                   RuleConst::TKTS_MAY_NOT_BE_ISSUED);
    bool mustBeSold = false;
    bool mustBeTicket = false;
    bool matchSaleRestriction = false;
    bool matchTktRestriction = true;
    bool negativeMatchOptIn = false;
    bool negativeMatchPCC = false;
    _trx->noPNRPricing() = true;
    cri.categoryNumber() = RuleConst::SALE_RESTRICTIONS_RULE;
    _srr->setFlags(*_trx,
                   *_paxTypeFare,
                   cri,
                   _sr,
                   mustBeSold,
                   mustBeTicket,
                   matchSaleRestriction,
                   matchTktRestriction,
                   negativeMatchOptIn,
                   negativeMatchPCC,
                   false);
    CPPUNIT_ASSERT(_paxTypeFare->warningMap().isSet(WarningMap::cat15_warning_2));
  }

  void testSetFlagsMustBeSoldWhenLocApplIsTktsMayOnlyBeSoldAndNoMatchTktRestrictions()
  {
    CategoryRuleInfo cri;
    _sr->locales() += createLocale(RuleConst::IATA_TVL_AGENCY_NO,
                                   RuleConst::HOME_TRAVEL_AGENCY,
                                   RuleConst::TKTS_MAY_ONLY_BE_SOLD);
    bool mustBeSold = false;
    bool mustBeTicket = false;
    bool matchSaleRestriction = false;
    bool matchTktRestriction = false;
    bool negativeMatchOptIn = false;
    bool negativeMatchPCC = false;
    _srr->setFlags(*_trx,
                   *_paxTypeFare,
                   cri,
                   _sr,
                   mustBeSold,
                   mustBeTicket,
                   matchSaleRestriction,
                   matchTktRestriction,
                   negativeMatchOptIn,
                   negativeMatchPCC,
                   false);
    CPPUNIT_ASSERT(mustBeSold);
  }

  void testSetFlagsNotMustBeSoldWhenLocApplIsTktsMayOnlyBeIssuedAndNoMatchTktRestrictions()
  {
    CategoryRuleInfo cri;
    _sr->locales() += createLocale(RuleConst::IATA_TVL_AGENCY_NO,
                                   RuleConst::HOME_TRAVEL_AGENCY,
                                   RuleConst::TKTS_MAY_NOT_BE_ISSUED);
    bool mustBeSold = false;
    bool mustBeTicket = false;
    bool matchSaleRestriction = false;
    bool matchTktRestriction = true;
    bool negativeMatchOptIn = false;
    bool negativeMatchPCC = false;
    _srr->setFlags(*_trx,
                   *_paxTypeFare,
                   cri,
                   _sr,
                   mustBeSold,
                   mustBeTicket,
                   matchSaleRestriction,
                   matchTktRestriction,
                   negativeMatchOptIn,
                   negativeMatchPCC,
                   false);
    CPPUNIT_ASSERT(!mustBeSold);
  }

  void testSetFlagsMustBeTktWhenLocApplIsTktsMayOnlyBeIssuedAndNoMatchTktRestrictions()
  {
    CategoryRuleInfo cri;
    _sr->locales() += createLocale(RuleConst::IATA_TVL_AGENCY_NO,
                                   RuleConst::HOME_TRAVEL_AGENCY,
                                   RuleConst::TKTS_MAY_ONLY_BE_ISSUED);
    bool mustBeSold = false;
    bool mustBeTicket = false;
    bool matchSaleRestriction = false;
    bool matchTktRestriction = false;
    bool negativeMatchOptIn = false;
    bool negativeMatchPCC = false;
    _srr->setFlags(*_trx,
                   *_paxTypeFare,
                   cri,
                   _sr,
                   mustBeSold,
                   mustBeTicket,
                   matchSaleRestriction,
                   matchTktRestriction,
                   negativeMatchOptIn,
                   negativeMatchPCC,
                   false);
    CPPUNIT_ASSERT(mustBeTicket);
  }

  void testSetFlagsNotMustBeTktWhenLocApplIsTktsMayOnlyBeSoldAndNoMatchTktRestrictions()
  {
    CategoryRuleInfo cri;
    _sr->locales() += createLocale(RuleConst::IATA_TVL_AGENCY_NO,
                                   RuleConst::HOME_TRAVEL_AGENCY,
                                   RuleConst::TKTS_MAY_ONLY_BE_SOLD);
    bool mustBeSold = false;
    bool mustBeTicket = false;
    bool matchSaleRestriction = false;
    bool matchTktRestriction = false;
    bool negativeMatchOptIn = false;
    bool negativeMatchPCC = false;
    _srr->setFlags(*_trx,
                   *_paxTypeFare,
                   cri,
                   _sr,
                   mustBeSold,
                   mustBeTicket,
                   matchSaleRestriction,
                   matchTktRestriction,
                   negativeMatchOptIn,
                   negativeMatchPCC,
                   false);
    CPPUNIT_ASSERT(!mustBeTicket);
  }

  void testValidateCarrierRestrictionPassWhenCarrierMatch()
  {
    CarrierCode carrier = carrier_AA;
    CarrierCode otherCarrier = carrier_UA;
    std::vector<TravelSeg*> travelSegments;
    travelSegments += createAirSegParToLon(); // carrier AA
    CPPUNIT_ASSERT(_srr->validateCarrierRestriction(*_trx, travelSegments, carrier, otherCarrier));
  }

  void testValidateCarrierRestrictionPassWhenOtherCarrierMatch()
  {
    CarrierCode carrier = carrier_UA;
    CarrierCode otherCarrier = carrier_AA;
    std::vector<TravelSeg*> travelSegments;
    travelSegments += createAirSegParToLon(); // carrier AA
    CPPUNIT_ASSERT(_srr->validateCarrierRestriction(*_trx, travelSegments, carrier, otherCarrier));
  }

  void testValidateCarrierRestrictionPassWhenOtherCarrierAllianceMatchRTW()
  {
    _trx->getOptions()->setRtw(true);
    CarrierCode carrier = carrier_UA;
    CarrierCode otherCarrier = ONE_WORLD_ALLIANCE;
    std::vector<TravelSeg*> travelSegments;
    travelSegments += createAirSegParToLon(); // carrier AA
    CPPUNIT_ASSERT(_srr->validateCarrierRestriction(*_trx, travelSegments, carrier, otherCarrier));
  }

  void testValidateCarrierRestrictionFailWhenNeitherCarrierMatch()
  {
    CarrierCode carrier = carrier_UA;
    CarrierCode otherCarrier = carrier_UA;
    std::vector<TravelSeg*> travelSegments;
    travelSegments += createAirSegParToLon(); // carrier AA
    CPPUNIT_ASSERT(!_srr->validateCarrierRestriction(*_trx, travelSegments, carrier, otherCarrier));
  }

  void testCheckCarrierPassWhenMatchForJointCarrier()
  {
    Itin itin;
    createBasicPaxTypeFareWithFareAndFareInfo(RuleConst::JOINT_CARRIER);
    CPPUNIT_ASSERT(_srr->checkCarrier(*_trx, itin, *_paxTypeFare, _sr));
  }

  void testCheckCarrierFailWhenMatchForJointCarrier()
  {
    Itin itin;
    _trx->getRequest()->ticketEntry() = YES;
    addOneAirSegmentToItinWtihCarrierAA(itin);
    addOneAirSegmentToFareMarketWithCarrierBA();
    _paxTypeFare->actualPaxType() = _memHandle.create<PaxType>();
    _paxTypeFare->actualPaxType()->paxType() = RuleConst::MATCH_NEG_PAX_TYPE;
    createBasicPaxTypeFareWithFareAndFareInfo(RuleConst::JOINT_CARRIER);
    CPPUNIT_ASSERT(!_srr->checkCarrier(*_trx, itin, *_paxTypeFare, _sr));
  }

  void testCheckCarrierPassWhenMatchForNotJointCarrier()
  {
    Itin itin;
    createBasicPaxTypeFareWithFareAndFareInfo(carrier_AA);
    CPPUNIT_ASSERT(_srr->checkCarrier(*_trx, itin, *_paxTypeFare, _sr));
  }

  void testSetTrailerMsg()
  {
    createBasicPaxTypeFareWithFare();
    _sr->fopCashInd() = 'Y';
    _sr->fopCheckInd() = 'Y';
    _sr->fopCreditInd() = 'Y';
    _sr->fopGtrInd() = 'Y';
    _srr->getTrailerMsg(_sr);
    _srr->setTrailerMsg(*_paxTypeFare, 0);
    Fare::FopStatus forbiddenFop(
        Fare::FopState(Fare::FOP_CASH | Fare::FOP_CHECK | Fare::FOP_CREDIT | Fare::FOP_GTR));
    CPPUNIT_ASSERT_EQUAL(forbiddenFop.value(), _paxTypeFare->fare()->forbiddenFop().value());
  }

  void testSetTrailerMsg_FareUsage()
  {
    _sr->fopCashInd() = 'Y';
    _sr->fopCreditInd() = 'Y';
    FareUsage fu;
    _srr->getTrailerMsg(_sr);
    _srr->setTrailerMsg(*_paxTypeFare, &fu);
    Fare::FopStatus forbiddenFop(Fare::FopState(Fare::FOP_CASH | Fare::FOP_CREDIT));
    CPPUNIT_ASSERT_EQUAL(forbiddenFop.value(), fu.forbiddenFop().value());
    CPPUNIT_ASSERT_EQUAL(std::string("WHEN TICKETING FOP MUST NOT BE CASH/CC"),
                         fu.getFopTrailerMsg());
  }

  void testCheckPreconditionsPass()
  {
    SalesRestrictionRule::Cat15FailReasons failReason;
    _sr->unavailTag() = RuleConst::BLANK;
    CPPUNIT_ASSERT(_srr->checkPreConditions(failReason, _sr, true));
  }

  void testCheckPreconditionsFailWhenDataUnavailable()
  {
    SalesRestrictionRule::Cat15FailReasons failReason;
    _sr->unavailTag() = RuleConst::DATA_UNAVAILABLE;
    CPPUNIT_ASSERT(!_srr->checkPreConditions(failReason, _sr, true));
  }

  void testCheckPreconditionsFailReasonSetToDataUnavailableWhenDataUnavailable()
  {
    SalesRestrictionRule::Cat15FailReasons failReason;
    _sr->unavailTag() = RuleConst::DATA_UNAVAILABLE;
    _srr->checkPreConditions(failReason, _sr, true);
    CPPUNIT_ASSERT_EQUAL(SalesRestrictionRule::SALES_RESTR_DATA_UNAVAILABLE, failReason);
  }

  void testCheckPreconditionsFailWhenTextOnly()
  {
    SalesRestrictionRule::Cat15FailReasons failReason;
    _sr->unavailTag() = RuleConst::TEXT_ONLY;
    CPPUNIT_ASSERT(!_srr->checkPreConditions(failReason, _sr, true));
  }

  void testCheckPreconditionsFailReasonSetToTextOnlyWhenTextOnly()
  {
    SalesRestrictionRule::Cat15FailReasons failReason;
    _sr->unavailTag() = RuleConst::TEXT_ONLY;
    _srr->checkPreConditions(failReason, _sr, true);
    CPPUNIT_ASSERT_EQUAL(SalesRestrictionRule::SALES_RESTR_TEXT_ONLY, failReason);
  }

  void testCheckPreconditionsFailWhenNoSkipCat15Security()
  {
    SalesRestrictionRule::Cat15FailReasons failReason;
    _sr->unavailTag() = RuleConst::BLANK;
    _sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    _sr->otherCarrier() = RuleConst::JOINT_CARRIER;
    CPPUNIT_ASSERT(!_srr->checkPreConditions(failReason, _sr, false));
  }

  void testCheckPreconditionsFailReasonCrsOtherCarrierWhenSkipCat15Security()
  {
    SalesRestrictionRule::Cat15FailReasons failReason;
    _sr->unavailTag() = RuleConst::BLANK;
    _sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    _sr->otherCarrier() = RuleConst::JOINT_CARRIER;
    _srr->checkPreConditions(failReason, _sr, false);
    CPPUNIT_ASSERT_EQUAL(SalesRestrictionRule::SALES_RESTR_CRS_OTHER_CARRIER, failReason);
  }

  void testCheckSecurityDataInMainCat15SetCat15HasSecurity()
  {
    _sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    _sr->tvlAgentSaleInd() = RuleConst::BLANK;

    Locale* locale = new Locale();
    _sr->locales() += locale;

    createBasicPaxTypeFareWithFare();
    _paxTypeFare->fare()->setCat15HasSecurity(false);
    CategoryRuleInfo cri;
    _srr->checkSecurityDataInMainCat15(_sr, false, *_paxTypeFare, cri);

    CPPUNIT_ASSERT(_paxTypeFare->fare()->setCat15HasSecurity());
  }

  void
  testCheckSecurityDataInMainCat15DontSetCat15HasSecurityWhenLocalesIsEmptyAndCarrierIndAndTvlIndAreBlank()
  {
    _sr->carrierCrsInd() = RuleConst::BLANK;
    _sr->tvlAgentSaleInd() = RuleConst::BLANK;
    createBasicPaxTypeFareWithFare();
    _paxTypeFare->fare()->setCat15HasSecurity(false);
    CategoryRuleInfo cri;
    _srr->checkSecurityDataInMainCat15(_sr, false, *_paxTypeFare, cri);

    CPPUNIT_ASSERT(!_paxTypeFare->fare()->cat15HasSecurity());
  }

  void testCheckSecurityDataInMainCat15DontSetCat15HasSecurityWhenIsQualifiedCategory()
  {
    _sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    _sr->tvlAgentSaleInd() = RuleConst::BLANK;
    Locale* locale = new Locale();
    _sr->locales() += locale;

    createBasicPaxTypeFareWithFare();
    _paxTypeFare->fare()->setCat15HasSecurity(false);
    CategoryRuleInfo cri;
    _srr->checkSecurityDataInMainCat15(_sr, true, *_paxTypeFare, cri);

    CPPUNIT_ASSERT(!_paxTypeFare->fare()->cat15HasSecurity());
  }

  void testCheckSecurityDataInMainCat15SetSecurityInCat15FnWhenCategoryRuleInfoIsFootNoteCtrlInfo()
  {
    _sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    createBasicPaxTypeFareWithFare();
    FootNoteCtrlInfo cri;
    _srr->checkSecurityDataInMainCat15(_sr, false, *_paxTypeFare, cri);

    CPPUNIT_ASSERT(_paxTypeFare->fare()->isSecurityInCat15Fn());
  }

  void
  testCheckSecurityDataInMainCat15SetCat15GeneralRuleProcessHasSecurityWhenIsQualifiedCategory()
  {
    _sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    createBasicPaxTypeFareWithFare();
    _paxTypeFare->fare()->setCat15HasSecurity(false);
    CategoryRuleInfo cri;
    _paxTypeFare->fare()->setCat15GeneralRuleProcess(true);
    _srr->checkSecurityDataInMainCat15(_sr, true, *_paxTypeFare, cri);

    CPPUNIT_ASSERT(_paxTypeFare->fare()->isCat15GeneralRuleProcess());
  }

  void testCheckSecurityDataInMainCat15SetCat15FRWhenIsNotFootNoteAndNotGeneralRule()
  {
    _sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    createBasicPaxTypeFareWithFare();
    _paxTypeFare->fare()->setCat15HasSecurity(false);
    CategoryRuleInfo cri;
    _paxTypeFare->fare()->setCat15GeneralRuleProcess(false);
    _srr->checkSecurityDataInMainCat15(_sr, false, *_paxTypeFare, cri);

    CPPUNIT_ASSERT(_paxTypeFare->fare()->isSecurityInCat15Fr());
  }

  void testCheckPrivateTariffCarrierRestrictionsPassWhenNotPrivateTariff()
  {
    bool isCat15Security;
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    SalesRestrictionRule::Cat15FailReasons failReason;
    createBasicPaxTypeFareWithFareAndTariffXRef(0);

    CPPUNIT_ASSERT(_srr->checkPrivateTariffCarrierRestrictions(
        failReason, _sr, true, true, *_paxTypeFare, isCat15Security, cri, &rule));
  }

  void testCheckPrivateTariffCarrierRestrictionsFailWhenPrivateTariff()
  {
    bool isCat15Security;
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);
    SalesRestrictionRule::Cat15FailReasons failReason;
    createBasicPaxTypeFareWithFareAndTariffXRef();
    _sr->carrierCrsInd() = RuleConst::BLANK;
    _sr->tvlAgentSaleInd() = RuleConst::BLANK;
    _paxTypeFare->fare()->setCat15GeneralRuleProcess(false);

    CPPUNIT_ASSERT(!_srr->checkPrivateTariffCarrierRestrictions(
        failReason, _sr, false, false, *_paxTypeFare, isCat15Security, cri, &rule));
  }

  void testCheckPrivateTariffCarrierRestrictionsSetFailReasonToPrivateSecurityWhenPrivateTariff()
  {
    bool isCat15Security;
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    rule.setRelationalInd(CategoryRuleItemInfo::OR);
    SalesRestrictionRule::Cat15FailReasons failReason;
    createBasicPaxTypeFareWithFareAndTariffXRef();
    _sr->carrierCrsInd() = RuleConst::BLANK;
    _sr->tvlAgentSaleInd() = RuleConst::BLANK;
    _paxTypeFare->fare()->setCat15GeneralRuleProcess(false);

    _srr->checkPrivateTariffCarrierRestrictions(
        failReason, _sr, false, false, *_paxTypeFare, isCat15Security, cri, &rule);

    CPPUNIT_ASSERT_EQUAL(SalesRestrictionRule::SALES_RESTR_PRIVATE_SECURITY, failReason);
  }

  void testCheckSecurityPassWhenSkipCat15SecurityCheck()
  {
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    bool localeFailed;
    bool failedSabre;
    SalesRestrictionRule::Cat15FailReasons failReason;
    CPPUNIT_ASSERT(tse::checkSecurity(*_srr, *_trx,
                                       0,
                                       &rule,
                                       failReason,
                                       true,
                                       _sr,
                                       *_paxTypeFare,
                                       cri,
                                       true,
                                       localeFailed,
                                       failedSabre));
  }

  void testCheckSecuritylocaleFailedWhenCheckLocaleItemsFailed()
  {
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    bool localeFailed = false;
    bool failedSabre;
    SalesRestrictionRule::Cat15FailReasons failReason;
    SalesRestrictionRuleMock checker;
    checker._localeItemResult = false;
    tse::checkSecurity(checker, *_trx,
                          0,
                          &rule,
                          failReason,
                          true,
                          _sr,
                          *_paxTypeFare,
                          cri,
                          false,
                          localeFailed,
                          failedSabre);

    CPPUNIT_ASSERT(localeFailed);
  }

  void testCheckSecurityFailReasonSecurityWhenSecurityRestrictionFail()
  {
    CategoryRuleInfo cri;
    CategoryRuleItemInfo rule;
    bool localeFailed = false;
    bool failedSabre = true;
    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = agency_B4T0;
    _sr->tvlAgentSaleInd() = RuleConst::MAY_NOT_BE_SOLD_BY_TA;
    SalesRestrictionRule::Cat15FailReasons failReason;
    SalesRestrictionRuleMock checker;
    checker._localeItemResult = false;
    tse::checkSecurity(checker, *_trx,
                          0,
                          &rule,
                          failReason,
                          true,
                          _sr,
                          *_paxTypeFare,
                          cri,
                          false,
                          localeFailed,
                          failedSabre);

    CPPUNIT_ASSERT_EQUAL(failReason, SalesRestrictionRule::SALES_RESTR_SECURITY);
  }

  void testCheckOtherRestrictionsFailReasonCountryWhenCountryRestrictionFailed()
  {
    CategoryRuleInfo cri;
    Itin itin;
    SalesRestrictionRule::Cat15FailReasons failReason;
    FareUsage* fu = createBasicFareUsage();
    SalesRestrictionRuleMock2 checker;
    checker._checkCountryRestriction = false;

    CPPUNIT_ASSERT(
        !tse::checkOtherRestrictions(checker, *_trx, failReason, itin, fu, *_paxTypeFare, _sr, cri));
    CPPUNIT_ASSERT_EQUAL(failReason, SalesRestrictionRule::SALES_RESTR_COUNTRY);
  }

  void testCheckOtherRestrictionsFailReasonSalesDateWhenSalesDateFail()
  {
    CategoryRuleInfo cri;
    Itin itin;
    SalesRestrictionRule::Cat15FailReasons failReason;
    FareUsage* fu = createBasicFareUsage();
    SalesRestrictionRuleMock2 checker;
    checker._checkSalesDate = false;

    CPPUNIT_ASSERT(
        !tse::checkOtherRestrictions(checker, *_trx, failReason, itin, fu, *_paxTypeFare, _sr, cri));
    CPPUNIT_ASSERT_EQUAL(failReason, SalesRestrictionRule::SALES_RESTR_SALES_DATE);
  }

  void testCheckOtherRestrictionsFailReasonFormOfPaymentWhenFormOfPaymentFail()
  {
    CategoryRuleInfo cri;
    Itin itin;
    SalesRestrictionRule::Cat15FailReasons failReason;
    FareUsage* fu = createBasicFareUsage();
    SalesRestrictionRuleMock2 checker;
    checker._checkFormOfPayment = false;

    CPPUNIT_ASSERT(
        !tse::checkOtherRestrictions(checker, *_trx, failReason, itin, fu, *_paxTypeFare, _sr, cri));
    CPPUNIT_ASSERT_EQUAL(failReason, SalesRestrictionRule::SALES_RESTR_FORM_OF_PAYMENT);
  }

  void testCheckOtherRestrictionsFailReasonCurrencyWhenCurrencyRestrictionFail()
  {
    CategoryRuleInfo cri;
    Itin itin;
    SalesRestrictionRule::Cat15FailReasons failReason;
    FareUsage* fu = createBasicFareUsage();
    SalesRestrictionRuleMock2 checker;
    checker._checkCurrencyRestriction = false;

    CPPUNIT_ASSERT(
        !tse::checkOtherRestrictions(checker, *_trx, failReason, itin, fu, *_paxTypeFare, _sr, cri));
    CPPUNIT_ASSERT_EQUAL(failReason, SalesRestrictionRule::SALES_RESTR_CURRENCY);
  }

  void testCheckOtherRestrictionsFailReasonTicketWhenTicketRestrictionFail()
  {
    CategoryRuleInfo cri;
    Itin itin;
    SalesRestrictionRule::Cat15FailReasons failReason;
    FareUsage* fu = createBasicFareUsage();
    SalesRestrictionRuleMock2 checker;
    checker._checkTicketRestriction = false;

    CPPUNIT_ASSERT(
        !tse::checkOtherRestrictions(checker, *_trx, failReason, itin, fu, *_paxTypeFare, _sr, cri));
    CPPUNIT_ASSERT_EQUAL(failReason, SalesRestrictionRule::SALES_RESTR_TICKET);
  }

  void testCheckOtherRestrictionsFailReasonSoldTicketWhenSoldTicketRestrictionFail()
  {
    CategoryRuleInfo cri;
    Itin itin;
    SalesRestrictionRule::Cat15FailReasons failReason;
    FareUsage* fu = createBasicFareUsage();
    SalesRestrictionRuleMock2 checker;
    checker._checkSoldTktRestriction = false;

    CPPUNIT_ASSERT(
        !tse::checkOtherRestrictions(checker, *_trx, failReason, itin, fu, *_paxTypeFare, _sr, cri));
    CPPUNIT_ASSERT_EQUAL(failReason, SalesRestrictionRule::SALES_RESTR_SOLD_TICKET);
  }

  void testDetermineNotValidReturnCodeSkipWhenTextOnly()
  {
    SalesRestrictionRule::Cat15FailReasons failReason = SalesRestrictionRule::SALES_RESTR_TEXT_ONLY;
    createBasicPaxTypeFareWithFare();
    CPPUNIT_ASSERT_EQUAL(SKIP, _srr->determineNotValidReturnCode(failReason, *_paxTypeFare));
  }

  void testDetermineNotValidReturnCodeSkipWhenDateOvverride()
  {
    SalesRestrictionRule::Cat15FailReasons failReason =
        SalesRestrictionRule::SALES_RESTR_DATE_OVERRIDE;
    createBasicPaxTypeFareWithFare();
    CPPUNIT_ASSERT_EQUAL(SKIP, _srr->determineNotValidReturnCode(failReason, *_paxTypeFare));
  }

  void testDetermineNotValidReturnCodeSkipWhenCRSOtherCarrierAndIsCat15GeneralRuleProcess()
  {
    SalesRestrictionRule::Cat15FailReasons failReason = SalesRestrictionRule::SALES_RESTR_TEXT_ONLY;
    createBasicPaxTypeFareWithFare();
    _paxTypeFare->fare()->setCat15GeneralRuleProcess(true);
    CPPUNIT_ASSERT_EQUAL(SKIP, _srr->determineNotValidReturnCode(failReason, *_paxTypeFare));
  }

  void testDetermineNotValidReturnCodeFailWhenDataUnavailable()
  {
    SalesRestrictionRule::Cat15FailReasons failReason =
        SalesRestrictionRule::SALES_RESTR_DATA_UNAVAILABLE;
    createBasicPaxTypeFareWithFare();
    _paxTypeFare->fare()->setCat15GeneralRuleProcess(true);
    CPPUNIT_ASSERT_EQUAL(FAIL, _srr->determineNotValidReturnCode(failReason, *_paxTypeFare));
  }

  void testDetermineNotValidReturnCodeFailWhenCRSOtherCarrierAndIsNotCat15GeneralRuleProcess()
  {
    SalesRestrictionRule::Cat15FailReasons failReason =
        SalesRestrictionRule::SALES_RESTR_CRS_OTHER_CARRIER;
    createBasicPaxTypeFareWithFare();
    _paxTypeFare->fare()->setCat15GeneralRuleProcess(false);
    CPPUNIT_ASSERT_EQUAL(FAIL, _srr->determineNotValidReturnCode(failReason, *_paxTypeFare));
  }

  void testPricingGetAgent()
  {
    CPPUNIT_ASSERT_EQUAL(const_cast<const Agent*>(_trx->getRequest()->ticketingAgent()),
                         _srr->getAgent(*_trx, true));
  }

  void testRefundGetAgent()
  {
    RefundPricingTrx trx;
    RexBaseRequest* req = _memHandle.create<RexBaseRequest>();
    trx.setRequest(req);
    req->currentTicketingAgent() = _memHandle.create<Agent>();
    CPPUNIT_ASSERT_EQUAL(const_cast<const Agent*>(req->currentTicketingAgent()),
                         _srr->getAgent(trx, true));
  }

  void testIsPropperGDSvalidGds1S()
  {
    SalesRestriction* sr = _memHandle.create<SalesRestriction>();
    sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    sr->otherCarrier() = RuleConst::SABRE1S;
    CPPUNIT_ASSERT(_srr->isPropperGDS(sr));
  }

  void testIsPropperGDSvalidGds1B()
  {
    SalesRestriction* sr = _memHandle.create<SalesRestriction>();
    sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    sr->otherCarrier() = RuleConst::SABRE1B;
    CPPUNIT_ASSERT(_srr->isPropperGDS(sr));
  }

  void testIsPropperGDSvalidGds1J()
  {
    SalesRestriction* sr = _memHandle.create<SalesRestriction>();
    sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    sr->otherCarrier() = RuleConst::SABRE1J;
    CPPUNIT_ASSERT(_srr->isPropperGDS(sr));
  }

  void testIsPropperGDSvalidGds1F()
  {
    SalesRestriction* sr = _memHandle.create<SalesRestriction>();
    sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    sr->otherCarrier() = RuleConst::SABRE1F;
    CPPUNIT_ASSERT(_srr->isPropperGDS(sr));
  }

  void testIsPropperGDSvalidGds1W()
  {
    SalesRestriction* sr = _memHandle.create<SalesRestriction>();
    sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    sr->otherCarrier() = RuleConst::SABRE1W;
    CPPUNIT_ASSERT(_srr->isPropperGDS(sr));
  }

  void testIsPropperGDSinvalidGds()
  {
    SalesRestriction* sr = _memHandle.create<SalesRestriction>();
    sr->carrierCrsInd() = RuleConst::MUST_BE_SOLD_VIA_CRS;
    sr->otherCarrier() = "1H";
    CPPUNIT_ASSERT(!_srr->isPropperGDS(sr));
  }

  void testIsPropperGDSCrsIndNotSet()
  {
    SalesRestriction* sr = _memHandle.create<SalesRestriction>();
    sr->carrierCrsInd() = 'X';
    CPPUNIT_ASSERT(_srr->isPropperGDS(sr));
  }

  void testMatchDeptCode3CharOACSuccess()
  {
    _trx = createPricingTrxForCheckLocalItems();

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->loc() = "AAA";

    LocKey* loc2 = _memHandle.create<LocKey>();
    loc2->loc() = "BBB";

    _trx->getRequest()->ticketingAgent()->airlineDept() = "AAA";

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(
        ret = _srr->matchDeptCodeOAC(*_trx, *(_trx->getRequest()->ticketingAgent()), *loc1, *loc2));
    CPPUNIT_ASSERT(ret);
  }

  void testMatchDeptCode3CharOACFail()
  {
    _trx = createPricingTrxForCheckLocalItems();

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->loc() = "AAA";

    LocKey* loc2 = _memHandle.create<LocKey>();
    loc2->loc() = "BBB";

    _trx->getRequest()->ticketingAgent()->airlineDept() = "AAAA";

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(
        ret = _srr->matchDeptCodeOAC(*_trx, *(_trx->getRequest()->ticketingAgent()), *loc1, *loc2));
    CPPUNIT_ASSERT(!ret);
  }

  void testMatchDeptCode5CharOACSuccess()
  {
    _trx = createPricingTrxForCheckLocalItems();

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->loc() = "AAAAA";

    LocKey* loc2 = _memHandle.create<LocKey>();
    loc2->loc() = "BBB";

    _trx->getRequest()->ticketingAgent()->airlineDept() = "AAAAA";
    _trx->getRequest()->ticketingAgent()->officeDesignator() = "AAAAA";

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(
        ret = _srr->matchDeptCodeOAC(*_trx, *(_trx->getRequest()->ticketingAgent()), *loc1, *loc2));
    CPPUNIT_ASSERT(ret);
  }

  void testMatchDeptCode5CharOACFail()
  {
    _trx = createPricingTrxForCheckLocalItems();

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->loc() = "AAA";

    LocKey* loc2 = _memHandle.create<LocKey>();
    loc2->loc() = "BBB";

    _trx->getRequest()->ticketingAgent()->airlineDept() = "AAAAA";
    _trx->getRequest()->ticketingAgent()->officeDesignator() = "AAAAA";

    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(
        ret = _srr->matchDeptCodeOAC(*_trx, *(_trx->getRequest()->ticketingAgent()), *loc1, *loc2));
    CPPUNIT_ASSERT(!ret);
  }

  void testValidateCarriers()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    SalesRestrictionRuleMock2 salesRestrictionRule;

    PricingTrx trx;
    PaxTypeFare ptf;
    Itin itin;

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData vData1, vData2, vData3;

    vData1.validatingCarriersData()["F1"] = v;
    vData1.validatingCarriersData()["F2"] = v;
    itin.validatingCxrGsaData() = &vData1;

    itin.validatingCarrier() = "A1";

    trx.setTrxType(PricingTrx::IS_TRX);
    CPPUNIT_ASSERT(salesRestrictionRule.validateCarriers(trx, itin, ptf, 0, 0, 0));

    trx.setTrxType(PricingTrx::MIP_TRX);
    FareUsage fu;
    CPPUNIT_ASSERT(salesRestrictionRule.validateCarriers(trx, itin, ptf, 0, &fu, 0));

    trx.setValidatingCxrGsaApplicable(false);
    CPPUNIT_ASSERT(salesRestrictionRule.validateCarriers(trx, itin, ptf, 0, 0, 0));

    trx.setValidatingCxrGsaApplicable(true);
    CPPUNIT_ASSERT(!salesRestrictionRule.validateCarriers(trx, itin, ptf, 0, 0, 0));

    vData2.validatingCarriersData()["A1"] = v;
    itin.validatingCxrGsaData() = &vData2;
    CPPUNIT_ASSERT(salesRestrictionRule.validateCarriers(trx, itin, ptf, 0, 0, 0));

    vData2.validatingCarriersData()["F1"] = v;
    itin.validatingCxrGsaData() = &vData2;
    CPPUNIT_ASSERT(salesRestrictionRule.validateCarriers(trx, itin, ptf, 0, 0, 0));
  }

  void testValidateAllCarriersIS()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    SalesRestrictionRuleMock2 salesRestrictionRule;

    PricingTrx trx;
    PaxTypeFare ptf;
    Itin itin;

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData vData1, vData2, vData3;

    vData1.validatingCarriersData()["A1"] = v;
    itin.validatingCxrGsaData() = &vData1;
    CPPUNIT_ASSERT(salesRestrictionRule.validateAllCarriersIS(trx, ptf, itin, 0));

    vData1.validatingCarriersData()["A2"] = v;
    CPPUNIT_ASSERT(salesRestrictionRule.validateAllCarriersIS(trx, ptf, itin, 0));

    vData2.validatingCarriersData()["F1"] = v;
    itin.validatingCxrGsaData() = &vData2;
    CPPUNIT_ASSERT(!salesRestrictionRule.validateAllCarriersIS(trx, ptf, itin, 0));

    vData2.validatingCarriersData()["F2"] = v;
    CPPUNIT_ASSERT(!salesRestrictionRule.validateAllCarriersIS(trx, ptf, itin, 0));

    vData1.validatingCarriersData()["F1"] = v;
    itin.validatingCxrGsaData() = &vData1;
    CPPUNIT_ASSERT(salesRestrictionRule.validateAllCarriersIS(trx, ptf, itin, 0));

    vData1.validatingCarriersData()["F1"] = v;
    vData1.validatingCarriersData()["F2"] = v;

    CPPUNIT_ASSERT(salesRestrictionRule.validateAllCarriersIS(trx, ptf, itin, 0));
  }

  void testValidateAllCarriersISForMultiSp()
  {
    SalesRestrictionRuleMock2 salesRestrictionRule;

    PricingTrx trx;
    PaxTypeFare ptf;
    Itin itin;

    vcx::ValidatingCxrData v;
    ValidatingCxrGSAData vData1, vData2, vData3;
    SpValidatingCxrGSADataMap spGsaDataMap;

    vData1.validatingCarriersData()["A1"] = v;
    spGsaDataMap["BSP"] = &vData1;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;
    auto it1 = itin.spValidatingCxrGsaDataMap()->find("BSP");
    CPPUNIT_ASSERT(it1 != itin.spValidatingCxrGsaDataMap()->end());
    CPPUNIT_ASSERT(salesRestrictionRule.validateAllCarriersIS(trx, ptf, itin, 0));

    vData1.validatingCarriersData()["A2"] = v;
    auto it2 = itin.spValidatingCxrGsaDataMap()->find("BSP");
    CPPUNIT_ASSERT(it2 != itin.spValidatingCxrGsaDataMap()->end());
    CPPUNIT_ASSERT(salesRestrictionRule.validateAllCarriersIS(trx, ptf, itin, 0));

    vData2.validatingCarriersData()["F1"] = v;
    spGsaDataMap.erase("BSP");
    spGsaDataMap["GEN"] = &vData2;
    auto it3 = itin.spValidatingCxrGsaDataMap()->find("GEN");
    CPPUNIT_ASSERT(it3 != itin.spValidatingCxrGsaDataMap()->end());
    it3 = itin.spValidatingCxrGsaDataMap()->find("BSP");
    CPPUNIT_ASSERT(it3 == itin.spValidatingCxrGsaDataMap()->end());

    CPPUNIT_ASSERT(!salesRestrictionRule.validateAllCarriersIS(trx, ptf, itin, 0));

    vData2.validatingCarriersData()["F2"] = v;
    CPPUNIT_ASSERT(!salesRestrictionRule.validateAllCarriersIS(trx, ptf, itin, 0));

    spGsaDataMap["BSP"] = &vData1;
    CPPUNIT_ASSERT(salesRestrictionRule.validateAllCarriersIS(trx, ptf, itin, 0));
  }

  void testValidateCarrierExcludePublishingFailPublishingCrxMatch()
  {

    Itin itin;
    itin.validatingCarrier() = carrier_AA;
    _sr->otherCarrier() = ONE_WORLD_ALLIANCE;
    _sr->validationInd() = RuleConst::VALIDATING_CXR_RESTR_EXCLUDE_PUBLISHING;

    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_AA);

    CPPUNIT_ASSERT(
        !_srr->validateCarrierExcludePublishing(*_trx, itin.validatingCarrier(), *_paxTypeFare, _sr));
  }

  void testValidateCarrierExcludePublishingFailAllianceNotMatch()
  {

    Itin itin;
    itin.validatingCarrier() = carrier_LH;
    _sr->otherCarrier() = ONE_WORLD_ALLIANCE;
    _sr->validationInd() = RuleConst::VALIDATING_CXR_RESTR_EXCLUDE_PUBLISHING;

    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_AA);

    CPPUNIT_ASSERT(
        !_srr->validateCarrierExcludePublishing(*_trx, itin.validatingCarrier(), *_paxTypeFare, _sr));
  }

  void testValidateCarrierExcludePublishingPassAllianceMatch()
  {

    Itin itin;
    itin.validatingCarrier() = carrier_AA;
    _sr->otherCarrier() = ONE_WORLD_ALLIANCE;
    _sr->validationInd() = RuleConst::VALIDATING_CXR_RESTR_EXCLUDE_PUBLISHING;

    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_LH);

    CPPUNIT_ASSERT(
        _srr->validateCarrierExcludePublishing(*_trx, itin.validatingCarrier(), *_paxTypeFare, _sr));
  }

  void testValidateCarrierExcludePublishingPassOtherCarrierMatch()
  {

    Itin itin;
    itin.validatingCarrier() = carrier_AA;
    _sr->otherCarrier() = "AA";
    _sr->validationInd() = RuleConst::VALIDATING_CXR_RESTR_EXCLUDE_PUBLISHING;

    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_LH);

    CPPUNIT_ASSERT(
        _srr->validateCarrierExcludePublishing(*_trx, itin.validatingCarrier(), *_paxTypeFare, _sr));
  }

  void testValidateCarrierExcludePublishingFailOtherCarrierNotMatch()
  {

    Itin itin;
    itin.validatingCarrier() = carrier_AA;
    _sr->otherCarrier() = "BA";
    _sr->validationInd() = RuleConst::VALIDATING_CXR_RESTR_EXCLUDE_PUBLISHING;

    createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(carrier_LH);

    CPPUNIT_ASSERT(
        !_srr->validateCarrierExcludePublishing(*_trx, itin.validatingCarrier(), *_paxTypeFare, _sr));
  }

  void mockUpdatePaxTypeFareWhenMainCategory(SalesRestrictionRule::Cat15FailReasons failReason,
                                             PaxTypeFare& paxTypeFare)
  {
    SalesRestrictionRule salesRestrictionRule;
    PricingTrx trx;
    SalesRestriction salesRestriction;
    CategoryRuleInfo cri;
    FareUsage fU;

    salesRestrictionRule.updatePaxTypeFareWhenMainCategory(trx,
        &salesRestriction,
        failReason,
        false,
        paxTypeFare,
        cri,
        FAIL,
        &fU);
  }

  void testUpdatePaxTypeFareWhenMainCategory_Cat15SoftPass_True()
  {
    PaxTypeFare ptf;
    mockUpdatePaxTypeFareWhenMainCategory(SalesRestrictionRule::SALES_RESTR_NO_ERROR, ptf);

    CPPUNIT_ASSERT(ptf.cat15SoftPass());
  }

  void testUpdatePaxTypeFareWhenMainCategory_Cat15SoftPass_False()
  {
    PaxTypeFare ptf;
    mockUpdatePaxTypeFareWhenMainCategory(SalesRestrictionRule::SALES_RESTR_SECURITY, ptf);

    CPPUNIT_ASSERT(!ptf.cat15SoftPass());
  }

  void testUpdatePaxTypeFareWhenMainCategory_Cat15SecurityFail_True()
  {
    PaxTypeFare ptf;
    mockUpdatePaxTypeFareWhenMainCategory(SalesRestrictionRule::SALES_RESTR_SECURITY, ptf);

    CPPUNIT_ASSERT(ptf.cat15SecurityFail());
  }

  void testUpdatePaxTypeFareWhenMainCategory_Cat15SecurityFail_False()
  {
    PaxTypeFare ptf;
    mockUpdatePaxTypeFareWhenMainCategory(SalesRestrictionRule::SALES_RESTR_NO_ERROR, ptf);

    CPPUNIT_ASSERT(!ptf.cat15SecurityFail());
  }



  // helper methods

  Locale*
  createLocale(Indicator locType1, Indicator locType2, Indicator locAppl = RuleConst::REQUIRED)
  {
    Locale* locale = new Locale();
    locale->loc1().locType() = locType1;
    locale->loc2().locType() = locType2;
    locale->loc1().loc() = "ABC";
    locale->loc2().loc() = "ABC";
    locale->locAppl() = locAppl;
    return locale;
  }

  void createAgentTjrWithSsgGroup(int groupNo)
  {
    _trx->getRequest()->ticketingAgent()->agentTJR() = _memHandle.create<Customer>();
    _trx->getRequest()->ticketingAgent()->agentTJR()->ssgGroupNo() = groupNo;
  }

  void setNationFrance(LocKey& locKey1, LocKey& locKey2)
  {
    locKey1.locType() = LOCTYPE_NATION;
    locKey1.loc() = NATION_FRANCE;
    locKey2.locType() = LOCTYPE_NATION;
    locKey2.loc() = NATION_FRANCE;
  }
  // Trx contains the following items:
  //    PricingOptions with default values
  //    PricingRequest with formOfPaymentCash set to 'N'
  //        TicketingAgent of type Agent
  //          Agent has Local with currency FRA
  PricingTrx* createGenericPricingTrx()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    // Add options
    PricingOptions* options = _memHandle.create<PricingOptions>();
    trx->setOptions(options);

    // Add Request
    PricingRequest* request = _memHandle.create<PricingRequest>();
    trx->setRequest(request);
    trx->getRequest()->formOfPaymentCash() = NO; //'N';

    Agent* agent = _memHandle.create<Agent>();
    agent->agentLocation() = _frAgentLoc;
    agent->currencyCodeAgent() = city_FRA;
    trx->getRequest()->ticketingAgent() = agent;

    return trx;
  }

  PricingTrx* createPricingTrxForCheckLocalItems()
  {
    Customer* agentTJR = _memHandle.create<Customer>();
    agentTJR->optInAgency() = YES;

    _trx->getRequest()->ticketingAgent()->agentTJR() = agentTJR;
    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = agency_B4T0;
    _trx->getRequest()->ticketingAgent()->agentLocation() = _usAgentLoc;

    return _trx;
  }

  // Create a FareUsage with a PaxTypeFare with FareMarket from Paris to London
  FareUsage* createBasicFareUsage()
  {
    // Create Fare Usage
    FareUsage* fu = _memHandle.create<FareUsage>();
    fu->inbound() = false;
    createBasicPaxTypeFareWithFareMarketParToLon();
    fu->paxTypeFare() = _paxTypeFare;
    return fu;
  }

  void createBasicPaxTypeFareWithFare()
  {
    Fare* fare = _memHandle.create<Fare>();
    _paxTypeFare->setFare(fare);
  }

  // Create a PaxTypeFare with a PRIVATE_TARIFF an new Fare
  void
  createBasicPaxTypeFareWithFareAndTariffXRef(TariffCategory tariffCat = RuleConst::PRIVATE_TARIFF)
  {
    TariffCrossRefInfo* tariffCrossRef = _memHandle.create<TariffCrossRefInfo>();
    tariffCrossRef->tariffCat() = tariffCat;
    createBasicPaxTypeFareWithFare();
    _paxTypeFare->fare()->setTariffCrossRefInfo(tariffCrossRef);
    _paxTypeFare->setFare(_paxTypeFare->fare());
  }

  void createBasicPaxTypeFareWithFareAndTariffXRefAndFareInfo(
      TariffCategory tariffCat = RuleConst::PRIVATE_TARIFF,
      CarrierCode carrier = RuleConst::SABRE1W)
  {
    createBasicPaxTypeFareWithFare();
    TariffCrossRefInfo* tariffCrossRef = _memHandle.create<TariffCrossRefInfo>();
    tariffCrossRef->tariffCat() = tariffCat;
    _paxTypeFare->fare()->setTariffCrossRefInfo(tariffCrossRef);
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->carrier() = carrier;
    _paxTypeFare->fare()->setFareInfo(fi);
    _paxTypeFare->setFare(_paxTypeFare->fare());
  }

  void createBasicPaxTypeFareWithFareAndFareInfo(CarrierCode carrier = RuleConst::SABRE1W)
  {
    createBasicPaxTypeFareWithFare();
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->carrier() = carrier;
    _paxTypeFare->fare()->setFareInfo(fi);
  }

  // Creates a PaxTypeFare with a fareMarket that includes an orgin of paris
  // and a destination of London
  void createBasicPaxTypeFareWithFareMarketParToLon()
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    Loc* par = TestLocFactory::create(LOC_PAR);
    Loc* lon = TestLocFactory::create(LOC_LON);
    fm->origin() = par;
    fm->destination() = lon;

    // Create PaxType Fare
    _paxTypeFare->fareMarket() = fm;
  }

  void
  createBasicPaxTypeFareWithFareMarketParToLonAndFareInfo(CarrierCode carrier = RuleConst::SABRE1W)
  {
    createBasicPaxTypeFareWithFare();
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->carrier() = carrier;
    _paxTypeFare->fare()->setFareInfo(fi);
    _paxTypeFare->setFare(_paxTypeFare->fare());

    FareMarket* fm = _memHandle.create<FareMarket>();
    Loc* par = TestLocFactory::create(LOC_PAR);
    Loc* lon = TestLocFactory::create(LOC_LON);
    fm->origin() = par;
    fm->destination() = lon;

    // Create PaxType Fare
    _paxTypeFare->fareMarket() = fm;
  }

  void createBasicPaxTypeFareWithFareMarket()
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    _paxTypeFare->fareMarket() = fm;
  }

  // Creates and air segment from Paris to London
  AirSeg* createAirSegParToLon()
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->pnrSegment() = 1;
    seg->segmentOrder() = 1;
    seg->origin() = TestLocFactory::create(LOC_PAR);
    seg->destination() = TestLocFactory::create(LOC_LON);
    seg->stopOver() = false;
    seg->carrier() = carrier_AA;
    seg->departureDT() = DateTime::localTime();
    seg->arrivalDT() = DateTime::localTime();
    return seg;
  }

  // Creates and air segment from London to Paris
  AirSeg* createAirSegLonToPar()
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->pnrSegment() = 1;
    seg->segmentOrder() = 1;
    seg->origin() = TestLocFactory::create(LOC_LON);
    seg->destination() = TestLocFactory::create(LOC_PAR);
    seg->stopOver() = false;
    seg->carrier() = carrier_AA;
    seg->departureDT() = DateTime::localTime();
    seg->arrivalDT() = DateTime::localTime();
    return seg;
  }

  void addTwoAirSegmentsToItin(Itin& itin)
  {
    itin.travelSeg() += createAirSegParToLon(), createAirSegLonToPar();
  }

  void addTwoAirSegmentsToFareMarket()
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    _paxTypeFare->fareMarket() = fm;
    _paxTypeFare->fareMarket()->travelSeg() += createAirSegParToLon(), createAirSegLonToPar();
  }

  void addOneAirSegmentToItinWtihCarrierAA(Itin& itin)
  {
    itin.travelSeg() += createAirSegParToLon();
  }

  void addOneAirSegmentToFareMarketWithCarrierBA()
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    _paxTypeFare->fareMarket() = fm;
    AirSeg* airSeg = createAirSegParToLon();
    airSeg->carrier() = carrier_BA;
    _paxTypeFare->fareMarket()->travelSeg() += airSeg;
  }

  Loc* createTicketingAgentKrk()
  {
    Loc* loc = _memHandle.create<Loc>();
    loc->loc() = city_KRK;
    return loc;
  }

public:
  // mock class

  class SalesRestrictionRuleCat15FallBack : public SalesRestrictionRule
  {
  public:
    using SalesRestrictionRule::checkPreConditions;
    SalesRestrictionRuleCat15FallBack() {}
    const bool checkPreConditions(Cat15FailReasons& failReason,
                                  const SalesRestriction* salesRestriction,
                                  const bool skipCat15SecurityCheck) const
    {
      PricingTrx trx;
      return checkPreConditions(failReason, salesRestriction, skipCat15SecurityCheck, trx);
    }
  };

  class SalesRestrictionRuleForIsWebFareTest : public SalesRestrictionRuleCat15FallBack
  {
  public:
    bool _hasTvlyLocation;

  protected:
    bool hasTvlyLocation(const PricingTrx& _trx) const { return _hasTvlyLocation; }
  };

  class SalesRestrictionRuleMock : public SalesRestrictionRule
  {
  public:
    SalesRestrictionRuleMock()
      : SalesRestrictionRule(),
        _isRussianGroup(false),
        _isUsTerritory(false),
        _isScandinavia(false),
        _saleSecurity(false),
        _localeItemResult(false),
        _countryLoc(NULL),
        _location(NULL),
        _nation(NULL)
    {
    }

    bool checkRussianGroup(const Loc& loc1, const Loc& loc2) const override { return _isRussianGroup; }

    bool checkUsTerritoryRule(const Loc& loc1, const Loc& loc2) const override { return _isUsTerritory; }

    bool checkScandinavia(const Loc& loc1, const Loc& loc2) const override { return _isScandinavia; }

    const Loc* getLoc(const LocCode& loc, const PricingTrx& _trx) const override { return _location; }

    const Nation*
    getNation(const NationCode& nation, const DateTime& dt, const PricingTrx& _trx) const override
    {
      return _nation;
    }

    bool _isRussianGroup;
    bool _isUsTerritory;
    bool _isScandinavia;
    bool _saleSecurity;
    bool _localeItemResult;
    Loc* _countryLoc;
    Loc* _location;
    Nation* _nation;
  }; // SalesRestrictionRuleMock


  class SalesRestrictionRuleMock2 : public SalesRestrictionRule
  {
  public:
    SalesRestrictionRuleMock2()
      : _checkCountryRestriction(true),
        _checkSalesDate(true),
        _checkFormOfPayment(true),
        _checkCurrencyRestriction(true),
        _checkTicketRestriction(true),
        _checkSoldTktRestriction(true)
    {
    }

    bool checkCountryRestriction(PricingTrx& trx,
                                 const FareUsage* fU,
                                 PaxTypeFare& paxTypeFare,
                                 const SalesRestriction* salesRestriction) override
    {
      return _checkCountryRestriction;
    }

    bool checkSalesDate(PricingTrx& trx,
                        PaxTypeFare& paxTypeFare,
                        const SalesRestriction* salesRestriction) const override
    {
      return _checkSalesDate;
    }

    bool checkFormOfPayment(const PricingTrx&,
                            PaxTypeFare& paxTypeFare,
                            const SalesRestriction*,
                            FareUsage* fareUsage) override
    {
      return _checkFormOfPayment;
    }

    bool
    checkCurrencyRestriction(PricingTrx&, const FareUsage*, PaxTypeFare&, const SalesRestriction*) override
    {
      return _checkCurrencyRestriction;
    }

    bool checkSoldTktRestriction(PricingTrx&, Itin&, PaxTypeFare&, const SalesRestriction*) const override
    {
      return _checkSoldTktRestriction;
    }

    bool validateCarrier(const PricingTrx& trx,
                         const CarrierCode& vcr,
                         const PaxTypeFare& paxTypeFare,
                         const SalesRestriction* salesRestriction,
                         const bool isWQTrx) const override
    {
      if (vcr == "A1")
        return true;

      if (vcr == "A2")
        return true;

      if (vcr == "F1")
        return false;

      if (vcr == "F2")
        return false;

      return true;
    }

    bool _checkCountryRestriction;
    bool _checkSalesDate;
    bool _checkFormOfPayment;
    bool _checkCurrencyRestriction;
    bool _checkTicketRestriction;
    bool _checkSoldTktRestriction;
  }; // SalesRestrictionRuleMock2


  class SalesRestrictionDataHandle : public DataHandleMock
  {
    TestMemHandle& _memHandle;

  public:
    explicit SalesRestrictionDataHandle(TestMemHandle& dh) : _memHandle(dh) {}

    const std::vector<AirlineAllianceCarrierInfo*>&
    getAirlineAllianceCarrier(const CarrierCode& carrierCode)
    {
      std::vector<AirlineAllianceCarrierInfo*>* ret =
          _memHandle.create<std::vector<AirlineAllianceCarrierInfo*> >();

      if (carrierCode == "LH" || carrierCode == "LO")
      {
        AirlineAllianceCarrierInfo* cxrInfo = _memHandle.create<AirlineAllianceCarrierInfo>();
        cxrInfo->genericAllianceCode() = "*A";

        ret->push_back(cxrInfo);
        return *ret;
      }

      if (carrierCode == "AA" || carrierCode == "AB")
      {
        AirlineAllianceCarrierInfo* cxrInfo = _memHandle.create<AirlineAllianceCarrierInfo>();
        cxrInfo->genericAllianceCode() = "*O";

        ret->push_back(cxrInfo);
        return *ret;
      }

      if (carrierCode == "AF" || carrierCode == "AM")
      {
        AirlineAllianceCarrierInfo* cxrInfo = _memHandle.create<AirlineAllianceCarrierInfo>();
        cxrInfo->genericAllianceCode() = "*S";

        ret->push_back(cxrInfo);
        return *ret;
      }

      return *ret;
    }
  };

  // Global variables
  Itin* _itin;
  SalesRestriction* _sr;
  SalesRestrictionRuleCat15FallBack* _srr;
  Loc* _usAgentLoc;
  Loc* _frAgentLoc;
  PricingTrx* _trx;
  PaxTypeFare* _paxTypeFare;
  TestMemHandle _memHandle;
};

// ADL-matched for test

bool checkSaleSecurity(const SalesRestrictionTest::SalesRestrictionRuleMock& mock,
                       PricingTrx& trx,
                       const PaxTypeFare& paxTypeFare,
                       const CategoryRuleInfo& cri,
                       const CategoryRuleItemInfo* rule,
                       const SalesRestriction* salesRestriction,
                       bool& failedSabre,
                       bool isFareDisplay,
                       bool isQualifiedCategory)
{
  return mock._saleSecurity;
}

bool checkLocaleItems(const SalesRestrictionTest::SalesRestrictionRuleMock& mock,
                      const PricingTrx& trx,
                      FareUsage* fU,
                      PaxTypeFare& paxTypeFare,
                      const CategoryRuleInfo& cri,
                      const SalesRestriction* salesRestriction,
                      bool isQualifiedCategory)
{
  return mock._localeItemResult;
}

bool checkTicketRestriction(const SalesRestrictionTest::SalesRestrictionRuleMock2& mock,
                            PricingTrx&,
                            FareUsage*,
                            PaxTypeFare&,
                            const SalesRestriction*,
                            const CategoryRuleInfo&)
{
  return mock._checkTicketRestriction;
}

CPPUNIT_TEST_SUITE_REGISTRATION(SalesRestrictionTest);
}
