#include "Common/TrxUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/EligibilityInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/RuleItemInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Rules/Eligibility.h"
#include "Rules/RuleValidationChancelor.h"
#include "Rules/RuleValidationObserver.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/CppUnitHelperMacros.h"
#include <set>
#include <vector>

namespace tse
{

class EligibilityTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EligibilityTest);

  CPPUNIT_TEST(testMatchPassengerStatus_blankPsgStatus);
  CPPUNIT_TEST(testMatchPassengerStatus_blankPsgStatus_Nationality);
  CPPUNIT_TEST(testMatchPassengerStatus_blankPsgStatus_Residency);
  CPPUNIT_TEST(testMatchPassengerStatus_blankPsgStatus_Employment);
  CPPUNIT_TEST(testCheckAgeRestrictions_emptyFcasPaxType);
  CPPUNIT_TEST(testCheckAgeRestrictions_paxTypeSameWithR3);
  CPPUNIT_TEST(testCheckAgeRestrictions_paxTypeDiffFromR3);
  CPPUNIT_TEST(testCheckPTC_EmptypaxADTr3);
  CPPUNIT_TEST(testCheckPTC_EmptyEinfopaxADTr3_EnableDiag);
  CPPUNIT_TEST(testCheckPTC_ADTpaxADTr3);
  CPPUNIT_TEST(testCheckPTC_ADTpaxNEGr3);
  CPPUNIT_TEST(testValidate_ADTpax_NEGr3);
  CPPUNIT_TEST(testValidate_ADTpax_ADTr3);
  CPPUNIT_TEST(testValidate_ADTpax_C10r3);
  CPPUNIT_TEST(testValidateFromFCO_ADTpax_ADTr3);
  CPPUNIT_TEST(testValidateFromFCO_ADTpax_ADTr3_EnableDiagnostic);
  CPPUNIT_TEST(testMatchPassengerStatus_blankPsgStatus_EnableDiagnostic);
  CPPUNIT_TEST(testValidate_ADTpax_NEGr3_EnableDiagnostic);
  CPPUNIT_TEST(testcheckUnavailableAndText_Record3_dataUnavailable);
  CPPUNIT_TEST(testcheckUnavailableAndText_Record3_TextOnly);
  CPPUNIT_TEST(testValidateRuleItemInfo_ADTpax_EnableDiagnostic);
  CPPUNIT_TEST(testValidate_ADTpax_NEGr3_EnableDiagnostic_DataUnavailable);
  CPPUNIT_TEST(testValidate_ADTpax_NEGr3_EnableDiagnostic_TextOnly);
  CPPUNIT_TEST(testValidate_ADTpax_NEGr3_setNeedChkCat1R3Psg_EnableDiagnostic_TextOnly);
  CPPUNIT_TEST(testcheckAccountCode_Empty_EnableDiagnostic);
  CPPUNIT_TEST(testcheckAccountCode_withAccountCode_EnableDiagnostic);
  CPPUNIT_TEST(testcheckAccountCode_requestwithCorpId_EnableDiagnostic);
  CPPUNIT_TEST(testcheckAccountCode_requestwithAccountCode_EnableDiagnostic);
  CPPUNIT_TEST(testcheckAccountCode_requestwithAccountCode_Cat1AccountCode_EnableDiagnostic);
  CPPUNIT_TEST(testcheckAccountCode_requestwithCorpId_Cat1AccountCode_EnableDiagnostic);
  CPPUNIT_TEST(testValidate_PricingUnit_testFail);
  CPPUNIT_TEST(testValidate_PricingUnit_testdataUnavailable);
  CPPUNIT_TEST(testValidate_PricingUnit_testtextOnly);
  CPPUNIT_TEST(testValidate_PricingUnit_testPsgType);
  CPPUNIT_TEST(testValidate_PricingUnit_testPsgType_isCat15QualifyingTrue);
  CPPUNIT_TEST(testValidate_checkPassengerStatus);
  CPPUNIT_TEST(testCheckMultiAccCodeCorpId_emptyVector);
  CPPUNIT_TEST(testCheckMultiAccCodeCorpId_passAccCode);
  CPPUNIT_TEST(testCheckMultiAccCodeCorpId_failAccCode);
  CPPUNIT_TEST(test_checkFlexFareAcctCodeCorpId_fail);
  CPPUNIT_TEST(test_isSingleFlexFaresAccCodeCorpIdMatched_pass);
  CPPUNIT_TEST(test_isSingleFlexFaresAccCodeCorpIdMatched_fail);
  CPPUNIT_TEST(test_areFlexFaresAccCodesCorpIdsMatched_context_faremarket_pass);
  CPPUNIT_TEST(test_areFlexFaresAccCodesCorpIdsMatched_context_faremarket_fail);
  CPPUNIT_TEST(test_areFlexFaresAccCodesCorpIdsMatched_context_fareusage_pass);
  CPPUNIT_TEST(test_areFlexFaresAccCodesCorpIdsMatched_context_fareusage_fail);
  CPPUNIT_TEST(test_checkFlexFareAcctCodeCorpId_pass);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<PricingTrx>();
    _options = _memHandle.create<PricingOptions>();
    _trx->setOptions(_options);

    _ptADT = _memHandle.create<PaxType>();
    _fareADTpax = _memHandle.create<Fare>();
    _fu = _memHandle.create<FareUsage>();
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _fu->paxTypeFare() = _paxTypeFare;

    _factory = DCFactory::instance();
    _diagPtr = _factory->create(*_trx);
    _diagPtr->enable(Diagnostic301);

    _diagEnabled = true;
    _isQualifyingCat = true;
    _request.ticketingDT() = DateTime::localTime();
    _trx->setRequest(&_request);
  }

  void tearDown() { _memHandle.clear(); }

  void activateDiag301() { _trx->diagnostic().diagnosticType() = Diagnostic301; }

  void testMatchPassengerStatus_blankPsgStatus()
  {
    EligibilityInfo eInfo;
    PaxTypeFare ptf;

    Eligibility el;
    CPPUNIT_ASSERT_EQUAL(true, el.matchPassengerStatus(eInfo, *_trx, ptf));
  }

  void testMatchPassengerStatus_blankPsgStatus_Nationality()
  {
    EligibilityInfo eInfo;
    _options->nationality() = "US";
    PaxTypeFare ptf;

    Eligibility el;
    CPPUNIT_ASSERT_EQUAL(true, el.matchPassengerStatus(eInfo, *_trx, ptf));
    _options->nationality() = "";
  }

  void testMatchPassengerStatus_blankPsgStatus_Residency()
  {
    EligibilityInfo eInfo;
    _options->residency() = "US";
    PaxTypeFare ptf;

    Eligibility el;
    CPPUNIT_ASSERT_EQUAL(true, el.matchPassengerStatus(eInfo, *_trx, ptf));
    _options->residency() = "";
  }

  void testMatchPassengerStatus_blankPsgStatus_Employment()
  {
    EligibilityInfo eInfo;
    _options->employment() = "US";
    PaxTypeFare ptf;

    Eligibility el;
    CPPUNIT_ASSERT_EQUAL(true, el.matchPassengerStatus(eInfo, *_trx, ptf));
    _options->employment() = "";
  }

  void testCheckAgeRestrictions_emptyFcasPaxType()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = ADULT;

    PaxTypeFare ptf;
    Eligibility el;
    Record3ReturnTypes ret =
        el.checkAgeRestrictions(&eInfo, ptf, _factory, _diagPtr, _isQualifyingCat, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testCheckAgeRestrictions_paxTypeSameWithR3()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = ADULT;
    eInfo.maxAge() = 12;
    eInfo.minAge() = 2;

    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;
    pt.age() = 10;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;

    Eligibility el;
    Record3ReturnTypes ret =
        el.checkAgeRestrictions(&eInfo, ptf, _factory, _diagPtr, _isQualifyingCat, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testCheckAgeRestrictions_paxTypeDiffFromR3()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = ADULT;
    eInfo.minAge() = 18;

    FareClassAppSegInfo fcas;
    fcas._paxType = NEG;
    PaxType pt;
    pt.paxType() = NEG;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;

    Eligibility el;
    Record3ReturnTypes ret =
        el.checkAgeRestrictions(&eInfo, ptf, _factory, _diagPtr, _isQualifyingCat, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testCheckPTC_EmptypaxADTr3()
  {
    EligibilityInfo eInfo;

    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;

    Eligibility el;
    Record3ReturnTypes ret =
        el.checkPTC(&eInfo, ptf, *_trx, _factory, _diagPtr, _isQualifyingCat, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testCheckPTC_EmptyEinfopaxADTr3_EnableDiag()
  {
    EligibilityInfo eInfo;

    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;

    PaxTypeFare ptf;

    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;

    activateDiag301();
    Eligibility el;
    Record3ReturnTypes ret =
        el.checkPTC(&eInfo, ptf, *_trx, _factory, _diagPtr, _isQualifyingCat, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testCheckPTC_ADTpaxADTr3()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = ADULT;

    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;
    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret =
        el.checkPTC(&eInfo, ptf, *_trx, _factory, _diagPtr, _isQualifyingCat, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testCheckPTC_ADTpaxNEGr3()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = NEG;

    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;
    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret =
        el.checkPTC(&eInfo, ptf, *_trx, _factory, _diagPtr, _diagEnabled, _isQualifyingCat);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidate_ADTpax_NEGr3()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = NEG;

    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;
    ptf.setNeedChkCat1R3Psg(false);

    Itin itin;
    FareMarket fm;
    bool isCat15Qualifying = false;
    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret =
        el.validate(*_trx, itin, ptf, &eInfo, fm, _isQualifyingCat, isCat15Qualifying);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidate_ADTpax_ADTr3()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = ADULT;
    eInfo.maxAge() = 0;
    eInfo.minAge() = 0;

    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;
    ptf.setNeedChkCat1R3Psg(false);

    Itin itin;
    FareMarket fm;
    bool isCat15Qualifying = false;

    Eligibility el;
    Record3ReturnTypes ret =
        el.validate(*_trx, itin, ptf, &eInfo, fm, _isQualifyingCat, isCat15Qualifying);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testValidate_ADTpax_C10r3()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = "C10";
    eInfo.maxAge() = 12;
    eInfo.minAge() = 2;

    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;
    ptf.setNeedChkCat1R3Psg(false);

    Itin itin;
    FareMarket fm;
    bool isCat15Qualifying = false;

    Eligibility el;
    Record3ReturnTypes ret =
        el.validate(*_trx, itin, ptf, &eInfo, fm, _isQualifyingCat, isCat15Qualifying);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidateFromFCO_ADTpax_ADTr3()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = ADULT;
    eInfo.maxAge() = 0;
    eInfo.minAge() = 0;

    CategoryRuleInfo ruleInfo;
    ruleInfo.categoryNumber() = 35;

    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;
    ptf.setNeedChkCat1R3Psg(false);

    Itin itin;
    FareMarket fm;

    Eligibility el;
    Record3ReturnTypes ret = el.validateFromFCO(*_trx, &eInfo, itin, ptf, &ruleInfo, fm);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testValidateFromFCO_ADTpax_ADTr3_EnableDiagnostic()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = ADULT;
    eInfo.maxAge() = 0;
    eInfo.minAge() = 0;

    CategoryRuleInfo ruleInfo;
    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;
    PaxTypeFare ptf;

    Fare fare;
    FareInfo fi;
    ptf.setFare(&fare);
    fare.setFareInfo(&fi);
    fi.fareClass() = "";

    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;
    ptf.setNeedChkCat1R3Psg(false);

    Itin itin;
    FareMarket fm;
    fm.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    fm.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    ptf.fareMarket() = &fm;

    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret = el.validateFromFCO(*_trx, &eInfo, itin, ptf, &ruleInfo, fm);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testMatchPassengerStatus_blankPsgStatus_EnableDiagnostic()
  {
    EligibilityInfo eInfo;
    PaxTypeFare ptf;
    activateDiag301();
    Eligibility el;
    CPPUNIT_ASSERT_EQUAL(true, el.matchPassengerStatus(eInfo, *_trx, ptf));
  }

  void testcheckUnavailableAndText_Record3_dataUnavailable()
  {
    EligibilityInfo eInfo;
    eInfo.unavailTag() = RuleApplicationBase::dataUnavailable;
    activateDiag301();
    Eligibility el;
    Record3ReturnTypes ret = el.checkUnavailableAndText(&eInfo);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testcheckUnavailableAndText_Record3_TextOnly()
  {
    EligibilityInfo eInfo;
    eInfo.unavailTag() = RuleApplicationBase::textOnly;
    activateDiag301();
    Eligibility el;
    Record3ReturnTypes ret = el.checkUnavailableAndText(&eInfo);
    CPPUNIT_ASSERT_EQUAL(SKIP, ret);
  }

  void testValidate_ADTpax_NEGr3_EnableDiagnostic()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = NEG;

    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;
    ptf.setNeedChkCat1R3Psg(false);

    Itin itin;
    FareMarket fm;
    bool isCat15Qualifying = false;
    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret =
        el.validate(*_trx, itin, ptf, &eInfo, fm, _isQualifyingCat, isCat15Qualifying);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidateRuleItemInfo_ADTpax_EnableDiagnostic()
  {
    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;
    ptf.setNeedChkCat1R3Psg(false);

    RuleItemInfo rule;

    Itin itin;
    FareMarket fm;
    bool isCat15Qualifying = false;
    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret =
        el.validate(*_trx, itin, ptf, &rule, fm, _isQualifyingCat, isCat15Qualifying);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidate_ADTpax_NEGr3_EnableDiagnostic_DataUnavailable()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = NEG;
    eInfo.unavailTag() = RuleApplicationBase::dataUnavailable;

    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;
    ptf.setNeedChkCat1R3Psg(false);

    RuleItemInfo rule;

    Itin itin;
    FareMarket fm;
    bool isCat15Qualifying = false;
    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret =
        el.validate(*_trx, itin, ptf, &eInfo, fm, _isQualifyingCat, isCat15Qualifying);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidate_ADTpax_NEGr3_EnableDiagnostic_TextOnly()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = NEG;
    eInfo.unavailTag() = RuleApplicationBase::textOnly;

    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;
    ptf.setNeedChkCat1R3Psg(false);

    Itin itin;
    FareMarket fm;
    bool isCat15Qualifying = false;
    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret =
        el.validate(*_trx, itin, ptf, &eInfo, fm, _isQualifyingCat, isCat15Qualifying);
    CPPUNIT_ASSERT_EQUAL(SKIP, ret);
  }

  void testValidate_ADTpax_NEGr3_setNeedChkCat1R3Psg_EnableDiagnostic_TextOnly()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = ADULT;
    eInfo.unavailTag() = RuleApplicationBase::textOnly;

    FareClassAppSegInfo fcas;
    fcas._paxType = ADULT;
    PaxType pt;
    pt.paxType() = ADULT;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.paxType() = &pt;
    ptf.setNeedChkCat1R3Psg(true);

    Itin itin;
    FareMarket fm;
    bool isCat15Qualifying = false;
    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret =
        el.validate(*_trx, itin, ptf, &eInfo, fm, _isQualifyingCat, isCat15Qualifying);
    CPPUNIT_ASSERT_EQUAL(SKIP, ret);
  }

  void testcheckAccountCode_Empty_EnableDiagnostic()
  {
    EligibilityInfo eInfo;

    FareClassAppSegInfo fcas;
    PaxType pt;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.setNeedChkCat1R3Psg(true);

    FareMarket fm;
    bool isCat15Qualifying = false;
    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret = el.checkAccountCode(
        &eInfo, ptf, *_trx, _factory, isCat15Qualifying, _diagPtr, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testcheckAccountCode_withAccountCode_EnableDiagnostic()
  {
    EligibilityInfo eInfo;
    eInfo.acctCode() = "TZZ01"; // IBM01

    _request.isMultiAccCorpId() = false;
    _request.corporateID() = ""; // TST11
    _trx->setRequest(&_request);

    FareClassAppSegInfo fcas;
    PaxType pt;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.setNeedChkCat1R3Psg(true);

    FareMarket fm;
    bool isCat15Qualifying = false;
    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret = el.checkAccountCode(
        &eInfo, ptf, *_trx, _factory, isCat15Qualifying, _diagPtr, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testcheckAccountCode_requestwithCorpId_EnableDiagnostic()
  {
    EligibilityInfo eInfo;

    _request.isMultiAccCorpId() = false;
    _request.corporateID() = "TST11";
    _request.accountCode() = ""; // TZZ01
    _trx->setRequest(&_request);

    FareClassAppSegInfo fcas;
    PaxType pt;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.setNeedChkCat1R3Psg(true);

    Itin itin;
    FareMarket fm;
    bool isCat15Qualifying = false;
    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret = el.checkAccountCode(
        &eInfo, ptf, *_trx, _factory, isCat15Qualifying, _diagPtr, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testcheckAccountCode_requestwithCorpId_Cat1AccountCode_EnableDiagnostic()
  {
    EligibilityInfo eInfo;
    eInfo.acctCode() = "TZZ01"; // IBM01

    _request.isMultiAccCorpId() = false;
    _request.corporateID() = "TZZ01"; // TST11
    _request.accountCode() = ""; // TZZ01
    _trx->setRequest(&_request);

    FareClassAppSegInfo fcas;
    PaxType pt;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.setNeedChkCat1R3Psg(true);

    FareMarket fm;
    bool isCat15Qualifying = false;
    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret = el.checkAccountCode(
        &eInfo, ptf, *_trx, _factory, isCat15Qualifying, _diagPtr, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testcheckAccountCode_requestwithAccountCode_EnableDiagnostic()
  {
    EligibilityInfo eInfo;

    _request.isMultiAccCorpId() = false;
    _request.corporateID() = ""; // TST11
    _request.accountCode() = "TZZ01";

    _trx->setRequest(&_request);

    FareClassAppSegInfo fcas;
    PaxType pt;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.setNeedChkCat1R3Psg(true);

    FareMarket fm;
    bool isCat15Qualifying = false;
    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret = el.checkAccountCode(
        &eInfo, ptf, *_trx, _factory, isCat15Qualifying, _diagPtr, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testcheckAccountCode_requestwithAccountCode_Cat1AccountCode_EnableDiagnostic()
  {
    EligibilityInfo eInfo;
    eInfo.acctCode() = "TZZ01"; // IBM01

    _request.isMultiAccCorpId() = false;
    _request.corporateID() = ""; // TST11
    _request.accountCode() = "IBM01"; // TZZ01
    _trx->setRequest(&_request);

    FareClassAppSegInfo fcas;
    PaxType pt;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.setNeedChkCat1R3Psg(true);

    FareMarket fm;
    bool isCat15Qualifying = false;
    activateDiag301();

    Eligibility el;
    Record3ReturnTypes ret = el.checkAccountCode(
        &eInfo, ptf, *_trx, _factory, isCat15Qualifying, _diagPtr, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidate_PricingUnit_testFail()
  {

    RuleItemInfo rule;
    rule.itemNo() = 34567;
    activateDiag301();

    Eligibility el;
    bool isCat15Qualifying = false;
    PricingUnit pu;
    PaxType pt;
    pt.paxType() = ADULT;
    pu.paxType() = &pt;

    Record3ReturnTypes ret = el.validate(*_trx, &rule, pu, *_fu, isCat15Qualifying);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidate_PricingUnit_testdataUnavailable()
  {
    EligibilityInfo eInfo;
    eInfo.unavailTag() = RuleApplicationBase::dataUnavailable;
    activateDiag301();
    Eligibility el;
    bool isCat15Qualifying = false;
    PricingUnit pu;
    PaxType pt;
    pt.paxType() = ADULT;
    pu.paxType() = &pt;
    _trx->setRequest(&_request);
    DateTime dt;
    _trx->getRequest()->ticketingDT() = dt.localTime();

    Record3ReturnTypes ret = el.validate(*_trx, &eInfo, pu, *_fu, isCat15Qualifying);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidate_PricingUnit_testtextOnly()
  {
    EligibilityInfo eInfo;
    eInfo.unavailTag() = RuleApplicationBase::textOnly;

    Eligibility el;
    bool isCat15Qualifying = false;
    PricingUnit pu;
    PaxType pt;
    pt.paxType() = ADULT;
    pu.paxType() = &pt;
    activateDiag301();
    _trx->setRequest(&_request);
    DateTime dt;
    _trx->getRequest()->ticketingDT() = dt.localTime();

    Record3ReturnTypes ret = el.validate(*_trx, &eInfo, pu, *_fu, isCat15Qualifying);
    CPPUNIT_ASSERT_EQUAL(SKIP, ret);
  }

  void testValidate_PricingUnit_testPsgType()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = NEG;
    activateDiag301();

    Eligibility el;
    bool isCat15Qualifying = false;
    PricingUnit pu;

    PaxType pt;
    pt.paxType() = ADULT;
    pu.paxType() = &pt;
    PaxType pt1;
    pt1.paxType() = "";
    _fu->paxTypeFare() = _paxTypeFare;
    _trx->setRequest(&_request);
    DateTime dt;
    _trx->getRequest()->ticketingDT() = dt.localTime();

    Record3ReturnTypes ret = el.validate(*_trx, &eInfo, pu, *_fu, isCat15Qualifying);
    CPPUNIT_ASSERT_EQUAL(SKIP, ret);
  }

  void testValidate_PricingUnit_testPsgType_isCat15QualifyingTrue()
  {
    EligibilityInfo eInfo;
    eInfo.psgType() = NEG;
    activateDiag301();

    Eligibility el;
    bool isCat15Qualifying = true;
    PricingUnit pu;

    PaxType pt;
    pt.paxType() = ADULT;
    pu.paxType() = &pt;
    PaxType pt1;
    pt1.paxType() = "";
    _fu->paxTypeFare() = _paxTypeFare;

    _trx->setRequest(&_request);
    DateTime dt;
    _trx->getRequest()->ticketingDT() = dt.localTime();

    Record3ReturnTypes ret = el.validate(*_trx, &eInfo, pu, *_fu, isCat15Qualifying);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidate_checkPassengerStatus()
  {
    EligibilityInfo eInfo;
    eInfo.psgStatus() = BLANK;
    eInfo.psgAppl() = RuleConst::NOT_ALLOWED;
    eInfo.psgType() = NEG;
    activateDiag301();

    Eligibility el;
    FareClassAppSegInfo fcas;
    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;
    ptf.setNeedChkCat1R3Psg(true);
    Itin itin;

    Record3ReturnTypes ret =
        el.checkPassengerStatus(&eInfo, ptf, *_trx, _factory, _diagPtr, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testCheckMultiAccCodeCorpId_emptyVector()
  {
    EligibilityInfo eInfo;
    eInfo.acctCode() = "TST02";

    _request.isMultiAccCorpId() = true;

    _trx->setRequest(&_request);

    FareClassAppSegInfo fcas;
    PaxType pt;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;

    Eligibility el;

    Record3ReturnTypes ret = el.checkMultiAccCodeCorpId(&eInfo, ptf, *_trx, _diagPtr, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testCheckMultiAccCodeCorpId_passAccCode()
  {
    EligibilityInfo eInfo;
    eInfo.acctCode() = "TST02";

    _request.isMultiAccCorpId() = true;

    _trx->setRequest(&_request);

    FareClassAppSegInfo fcas;
    PaxType pt;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;

    Eligibility el;

    _request.accCodeVec().push_back("TST01");
    _request.accCodeVec().push_back("TST02");
    _request.accCodeVec().push_back("TST03");
    Record3ReturnTypes ret = el.checkMultiAccCodeCorpId(&eInfo, ptf, *_trx, _diagPtr, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void testCheckMultiAccCodeCorpId_failAccCode()
  {
    EligibilityInfo eInfo;
    eInfo.acctCode() = "TST02";

    _request.isMultiAccCorpId() = true;

    _trx->setRequest(&_request);

    FareClassAppSegInfo fcas;
    PaxType pt;

    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;

    Eligibility el;

    _request.accCodeVec().push_back("TST01");
    _request.accCodeVec().push_back("TST02");
    _request.accCodeVec().push_back("TST03");

    eInfo.acctCode() = "TST05";
    Record3ReturnTypes ret = el.checkMultiAccCodeCorpId(&eInfo, ptf, *_trx, _diagPtr, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void initFlexFaresData(Eligibility& el,
                         PaxTypeFare& ptf,
                         RuleValidationContext::ContextType contextType)
  {
    FareClassAppSegInfo fcas;
    PaxType pt;
    ptf.fareClassAppSegInfo() = &fcas;

    std::shared_ptr<RuleValidationChancelor> chancelor(new RuleValidationChancelor());
    chancelor->updateContextType(contextType);
    el.setChancelor(chancelor);

    _trx->getMutableFlexFaresTotalAttrs().addAccCode("TST01", 0);
    _trx->getMutableFlexFaresTotalAttrs().addAccCode("TST02", 1);
    _trx->getMutableFlexFaresTotalAttrs().addCorpId("TST03", 0);
    _trx->getMutableFlexFaresTotalAttrs().addCorpId("TST04", 1);

    _trx->getRequest()->getMutableFlexFaresGroupsData().addAccCode("TST01", 0);
    _trx->getRequest()->getMutableFlexFaresGroupsData().addAccCode("TST02", 1);
    _trx->getRequest()->getMutableFlexFaresGroupsData().addCorpId("TST03", 0);
    _trx->getRequest()->getMutableFlexFaresGroupsData().addCorpId("TST04", 1);
  }

  void test_isSingleFlexFaresAccCodeCorpIdMatched_fail()
  {
    EligibilityInfo eInfo;
    eInfo.acctCode() = "TST02";

    PaxTypeFare ptf;
    Eligibility el;

    initFlexFaresData(el, ptf, RuleValidationContext::FARE_MARKET);
    bool ret = el.isSingleFlexFaresAccCodeCorpIdMatched(
        &eInfo, ptf, *_trx, flexFares::ACC_CODES, "ACCODE");
    CPPUNIT_ASSERT_EQUAL(false, ret);
  }

  void test_isSingleFlexFaresAccCodeCorpIdMatched_pass()
  {
    EligibilityInfo eInfo;
    eInfo.acctCode() = "TST02";

    PaxTypeFare ptf;
    _trx->setFlexFare(true);
    ptf.initializeFlexFareValidationStatus(*_trx);
    Eligibility el;

    initFlexFaresData(el, ptf, RuleValidationContext::FARE_MARKET);
    bool ret =
        el.isSingleFlexFaresAccCodeCorpIdMatched(&eInfo, ptf, *_trx, flexFares::ACC_CODES, "TST02");
    CPPUNIT_ASSERT_EQUAL(true, ret);
  }

  void test_areFlexFaresAccCodesCorpIdsMatched_context_faremarket_pass()
  {
    EligibilityInfo eInfo;
    eInfo.acctCode() = "TST01";

    Eligibility el;
    PaxTypeFare ptf;
    _trx->setFlexFare(true);
    ptf.initializeFlexFareValidationStatus(*_trx);
    std::vector<std::string> vec;

    initFlexFaresData(el, ptf, RuleValidationContext::FARE_MARKET);

    bool ret = el.areFlexFaresAccCodesCorpIdsMatched(&eInfo, ptf, *_trx, flexFares::ACC_CODES, vec);
    CPPUNIT_ASSERT_EQUAL(true, ret);
  }

  void test_areFlexFaresAccCodesCorpIdsMatched_context_faremarket_fail()
  {
    EligibilityInfo eInfo;
    eInfo.acctCode() = "TST00";

    Eligibility el;
    PaxTypeFare ptf;
    std::vector<std::string> vec;

    initFlexFaresData(el, ptf, RuleValidationContext::FARE_MARKET);

    bool ret = el.areFlexFaresAccCodesCorpIdsMatched(&eInfo, ptf, *_trx, flexFares::ACC_CODES, vec);
    CPPUNIT_ASSERT_EQUAL(false, ret);
  }

  void test_areFlexFaresAccCodesCorpIdsMatched_context_fareusage_pass()
  {
    EligibilityInfo eInfo;
    eInfo.acctCode() = "TST01";

    Eligibility el;
    PaxTypeFare ptf;
    _trx->setFlexFare(true);
    ptf.initializeFlexFareValidationStatus(*_trx);
    std::vector<std::string> vec;

    initFlexFaresData(el, ptf, RuleValidationContext::PU_FP);

    bool ret = el.areFlexFaresAccCodesCorpIdsMatched(&eInfo, ptf, *_trx, flexFares::ACC_CODES, vec);
    CPPUNIT_ASSERT_EQUAL(true, ret);
  }

  void test_areFlexFaresAccCodesCorpIdsMatched_context_fareusage_fail()
  {
    EligibilityInfo eInfo;
    eInfo.acctCode() = "TST00";

    Eligibility el;
    PaxTypeFare ptf;
    std::vector<std::string> vec;

    initFlexFaresData(el, ptf, RuleValidationContext::PU_FP);

    bool ret = el.areFlexFaresAccCodesCorpIdsMatched(&eInfo, ptf, *_trx, flexFares::ACC_CODES, vec);
    CPPUNIT_ASSERT_EQUAL(false, ret);
  }

  void test_checkFlexFareAcctCodeCorpId_pass()
  {
    EligibilityInfo eInfo;
    eInfo.acctCode() = "TST01";

    Eligibility el;
    PaxTypeFare ptf;
    _trx->setFlexFare(true);
    ptf.initializeFlexFareValidationStatus(*_trx);
    std::vector<std::string> vec;

    initFlexFaresData(el, ptf, RuleValidationContext::PU_FP);

    Record3ReturnTypes ret =
        el.checkFlexFareAcctCodeCorpId(&eInfo, ptf, *_trx, _diagPtr, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }

  void test_checkFlexFareAcctCodeCorpId_fail()
  {
    EligibilityInfo eInfo;
    eInfo.acctCode() = "TST00";

    Eligibility el;

    FareClassAppSegInfo fcas;
    PaxType pt;
    PaxTypeFare ptf;
    ptf.fareClassAppSegInfo() = &fcas;

    std::shared_ptr<RuleValidationChancelor> chancelor(new RuleValidationChancelor());
    chancelor->updateContextType(RuleValidationContext::PU_FP);
    el.setChancelor(chancelor);

    Record3ReturnTypes ret =
        el.checkFlexFareAcctCodeCorpId(&eInfo, ptf, *_trx, _diagPtr, _diagEnabled);
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

private:
  PricingTrx* _trx;
  PricingOptions* _options;
  PaxType* _ptADT;
  Fare* _fareADTpax;
  DCFactory* _factory;
  DiagCollector* _diagPtr;
  FareUsage* _fu;
  PaxTypeFare* _paxTypeFare;
  TestMemHandle _memHandle;

  bool _diagEnabled;
  bool _isQualifyingCat;
  PricingRequest _request;
};

CPPUNIT_TEST_SUITE_REGISTRATION(EligibilityTest);

} // tse
