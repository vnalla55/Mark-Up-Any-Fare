#include "test/include/CppUnitHelperMacros.h"

#include <iostream>

#include "MinFares/MatchRuleLevelExclTable.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "Common/TseBoostStringTypes.h"
#include "Diagnostic/DiagCollector.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"

#include "test/include/TimeBomb.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{

class MatchRuleLevelExclTableTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MatchRuleLevelExclTableTest);
  CPPUNIT_TEST(test_displayRuleLevel_NewSameGroupFields_EmptyWhenNotActivated);
  CPPUNIT_TEST(test_displayRuleLevel_NewSameGroupFields_NotEmptyWhenActivated);
  CPPUNIT_TEST(test_displayRuleLevel_NewSameGroupFields_EnableHIPApplySameGrp);
  CPPUNIT_TEST(test_displayRuleLevel_NewSameGroupFields_EnableCTMApplySameGrp);
  CPPUNIT_TEST(test_displayRuleLevel_NewSameGroupFields_EnableBHCApplySameGrp);
  CPPUNIT_TEST(test_displayRuleLevel_NewSameGroupFields_DisableHIPApplySameGrp);
  CPPUNIT_TEST(test_displayRuleLevel_NewSameGroupFields_DisableCTMApplySameGrp);
  CPPUNIT_TEST(test_displayRuleLevel_NewSameGroupFields_DisableBHCApplySameGrp);
  CPPUNIT_TEST(test_displayRuleLevel_NewSameGroupFields_EnableHIPApplySameGrp_Child);
  CPPUNIT_TEST(test_displayRuleLevel_NewSameGroupFields_DisableHIPApplySameGrp_Child);
  CPPUNIT_TEST(test_checkRuleLevelExclusionSameFareGroupCheck_HIP_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionSameFareGroupCheck_BHC_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionSameFareGroupCheck_CTM_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionSameFareGroupCheck_COM_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionSameFareGroupCheck_DMC_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionSameFareGroupCheck_COP_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionSameFareGroupCheck_OSC_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionSameFareGroupCheck_RSC_Compare_YES);
  CPPUNIT_TEST(test_checkRuleLevelExclusionSameFareGroupCheck_CPM_Compare_YES);
  CPPUNIT_TEST(test_passRuleLevelExclusionSameGroup_NoDataInRuleLevelExcl_DoNotMatch);
  CPPUNIT_TEST(test_passRuleLevelExclusionSameGroup_DataInRuleLevelExcl_DifferentSet_DoNotMatch);
  CPPUNIT_TEST(test_passRuleLevelExclusionSameGroup_matchInterMediateFareThruFare);
  CPPUNIT_TEST(test_passRuleLevelExclusionSameGroup_doNotMatchInterMediateFareThruFare);
  CPPUNIT_TEST(test_passRuleLevelExclusionSameGroup_MatchInterMediateFareThruFare_FareType);
  CPPUNIT_TEST(test_passRuleLevelExclusionSameGroup_MatchInterMediateFareThruFare_FareRule);
  CPPUNIT_TEST(test_passRuleLevelExclusionSameGroup_MatchInterMediateFareThruFare_FareTariff);

  CPPUNIT_TEST_SUITE_END();
  TestMemHandle _memHandle;

public:
  DiagCollector diagCollector;
  MatchRuleLevelExclTable* matchRule;
  MinimumFareModule module;
  PricingTrx trx;
  Itin itin;
  PaxTypeFare* paxTypeFare;
  DateTime tvlDate;
  MinFareRuleLevelExcl ruleLevelExcl;
  Fare* fare;
  FareInfo* fareInfo;
  PaxType* paxType;
  TariffCrossRefInfo* tariffRefInfo;
  FareMarket fareMarket;
  FareClassAppInfo* appInfo;
  FareClassAppSegInfo* fareClassAppSegInfo;

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    matchRule = new MatchRuleLevelExclTable(module, trx, itin, *paxTypeFare, tvlDate, false);

    matchRule->diagCollector() = &diagCollector;
    diagCollector.activate();
    ruleLevelExcl.hipSameGroupAppl() = 'N';
    ruleLevelExcl.backhaulSameGroupAppl() = 'N';
    ruleLevelExcl.ctmSameGroupAppl() = 'N';

    paxTypeFare = new PaxTypeFare();
    fare = new Fare();
    fareInfo = new FareInfo();
    paxType = new PaxType();
    tariffRefInfo = new TariffCrossRefInfo();
    appInfo = new FareClassAppInfo();
    fareClassAppSegInfo = new FareClassAppSegInfo();
  }

  void tearDown()
  {
    delete matchRule;
    delete fareInfo;
    delete paxType;
    delete tariffRefInfo;
    delete appInfo;
    delete fareClassAppSegInfo;
    delete fare;
    delete paxTypeFare;
    _memHandle.clear();
  }

  void setSameFareGroupChildData()
  {
    // Set Same Fare Group Child Data
    ruleLevelExcl.fareSet().push_back(1);
    ruleLevelExcl.sameFareGroupTariff().push_back(2);
    ruleLevelExcl.sameFareGroupRules().push_back("abcd");
    ruleLevelExcl.sameFareGroupFareClasses().push_back("efghijk");
    ruleLevelExcl.sameFareGroupFareTypes().push_back("ijk");
  }

  void createPaxTypeFare(PaxTypeFare& ptf)
  {
    paxType->vendorCode() = "ATP";
    fareInfo->_fareClass = "C";
    fareInfo->_ruleNumber = " ";
    tariffRefInfo->_ruleTariff = 0;
    fare->initialize(Fare::FS_International, fareInfo, fareMarket, tariffRefInfo);
    ptf.initialize(fare, paxType, &fareMarket);

    appInfo->_fareType = " ";
    appInfo->_fareClass = "C";
    ptf.fareClassAppInfo() = appInfo;
    fareClassAppSegInfo->_paxType = "ADT";
    paxTypeFare->fareClassAppSegInfo() = fareClassAppSegInfo;
  }

  void createPaxTypeThruFare(PaxTypeFare& ptf)
  {
    fareInfo->_fareClass = "C1234R";
    fare->initialize(Fare::FS_International, fareInfo, fareMarket, tariffRefInfo);
    ptf.initialize(fare, paxType, &fareMarket);
    appInfo->_fareClass = "C1234R";

    ptf.fareClassAppInfo() = appInfo;
    fareClassAppSegInfo->_paxType = "ADT";
    ptf.fareClassAppSegInfo() = fareClassAppSegInfo;
  }

  void createPaxTypeThruFare1(PaxTypeFare& ptf)
  {
    fareInfo->_fareClass = "JR";
    fare->initialize(Fare::FS_International, fareInfo, fareMarket, tariffRefInfo);
    ptf.initialize(fare, paxType, &fareMarket);
    appInfo->_fareClass = "JR";

    ptf.fareClassAppInfo() = appInfo;
    fareClassAppSegInfo->_paxType = "ADT";
    ptf.fareClassAppSegInfo() = fareClassAppSegInfo;
  }

  void createPaxTypeThruFare2(PaxTypeFare& ptf)
  {
    fareInfo->_fareClass = " ";
    fare->initialize(Fare::FS_International, fareInfo, fareMarket, tariffRefInfo);
    ptf.initialize(fare, paxType, &fareMarket);
    appInfo->_fareClass = "JA";
    appInfo->_fareType = "FU";

    ptf.fareClassAppInfo() = appInfo;
    fareClassAppSegInfo->_paxType = "ADT";
    ptf.fareClassAppSegInfo() = fareClassAppSegInfo;
  }

  void createPaxTypeThruFare3(PaxTypeFare& ptf)
  {
    fareInfo->_fareClass = " ";
    fareInfo->_ruleNumber = "5000";

    fare->initialize(Fare::FS_International, fareInfo, fareMarket, tariffRefInfo);
    ptf.initialize(fare, paxType, &fareMarket);
    appInfo->_fareClass = " ";
    appInfo->_fareType = " ";

    ptf.fareClassAppInfo() = appInfo;
    fareClassAppSegInfo->_paxType = "ADT";
    ptf.fareClassAppSegInfo() = fareClassAppSegInfo;
  }

  void createPaxTypeThruFare4(PaxTypeFare& ptf)
  {
    fareInfo->_fareClass = " ";
    fareInfo->_ruleNumber = " ";
    tariffRefInfo->_ruleTariff = 300;

    fare->initialize(Fare::FS_International, fareInfo, fareMarket, tariffRefInfo);
    ptf.initialize(fare, paxType, &fareMarket);
    appInfo->_fareClass = " ";
    appInfo->_fareType = " ";

    ptf.fareClassAppInfo() = appInfo;
    fareClassAppSegInfo->_paxType = "ADT";
    ptf.fareClassAppSegInfo() = fareClassAppSegInfo;
  }

  void setSameFareGroupChildData_1()
  {
    // Set Same Fare Group Child Data
    for (int i = 0; i <= 7; i++)
    {
      ruleLevelExcl.fareSet().push_back(1);
      ruleLevelExcl.sameFareGroupTariff().push_back(-1);
      ruleLevelExcl.sameFareGroupRules().push_back(" ");
      ruleLevelExcl.sameFareGroupFareTypes().push_back(" ");
    }
    ruleLevelExcl.sameFareGroupFareClasses().push_back("C");
    ruleLevelExcl.sameFareGroupFareClasses().push_back("C1234");
    ruleLevelExcl.sameFareGroupFareClasses().push_back("C1234A");
    ruleLevelExcl.sameFareGroupFareClasses().push_back("C1234AR");
    ruleLevelExcl.sameFareGroupFareClasses().push_back("C1234R");
    ruleLevelExcl.sameFareGroupFareClasses().push_back("CR");
    ruleLevelExcl.sameFareGroupFareClasses().push_back("CS");
    ruleLevelExcl.sameFareGroupFareClasses().push_back("CSR");
  }

  void setSameFareGroupChildData_1a()
  {
    // Set Same Fare Group Child Data
    for (int i = 0; i <= 1; i++)
    {
      ruleLevelExcl.sameFareGroupTariff().push_back(-1);
      ruleLevelExcl.sameFareGroupRules().push_back(" ");
      ruleLevelExcl.sameFareGroupFareTypes().push_back(" ");
    }
    ruleLevelExcl.fareSet().push_back(1);
    ruleLevelExcl.fareSet().push_back(2);

    ruleLevelExcl.sameFareGroupFareClasses().push_back("C");
    ruleLevelExcl.sameFareGroupFareClasses().push_back("C1234R");
  }

  void setSameFareGroupChildData_2()
  {
    // Set Same Fare Group Child Data
    for (int i = 0; i <= 1; i++)
    {
      ruleLevelExcl.fareSet().push_back(1);
      ruleLevelExcl.sameFareGroupTariff().push_back(-1);
      ruleLevelExcl.sameFareGroupRules().push_back(" ");
    }
    ruleLevelExcl.sameFareGroupFareClasses().push_back("C");
    ruleLevelExcl.sameFareGroupFareTypes().push_back(" ");

    ruleLevelExcl.sameFareGroupFareClasses().push_back(" ");
    ruleLevelExcl.sameFareGroupFareTypes().push_back("FU");
  }

  void setSameFareGroupChildData_3()
  {
    // Set Same Fare Group Child Data
    for (int i = 0; i <= 1; i++)
    {
      ruleLevelExcl.fareSet().push_back(1);
      ruleLevelExcl.sameFareGroupTariff().push_back(-1);
      ruleLevelExcl.sameFareGroupFareTypes().push_back(" ");
    }

    ruleLevelExcl.sameFareGroupRules().push_back(" ");
    ruleLevelExcl.sameFareGroupFareClasses().push_back("C");

    ruleLevelExcl.sameFareGroupRules().push_back("5000");
    ruleLevelExcl.sameFareGroupFareClasses().push_back(" ");
  }

  void setSameFareGroupChildData_4()
  {
    // Set Same Fare Group Child Data
    for (int i = 0; i <= 1; i++)
    {
      ruleLevelExcl.fareSet().push_back(1);
      ruleLevelExcl.sameFareGroupRules().push_back(" ");
      ruleLevelExcl.sameFareGroupFareTypes().push_back(" ");
    }

    ruleLevelExcl.sameFareGroupTariff().push_back(-1);
    ruleLevelExcl.sameFareGroupFareClasses().push_back("C");

    ruleLevelExcl.sameFareGroupTariff().push_back(300);
    ruleLevelExcl.sameFareGroupFareClasses().push_back(" ");
  }

  void assertContains(DiagCollector& diagCollector, const string& expected)
  {
    CPPUNIT_ASSERT_MESSAGE(diagCollector.str(), diagCollector.str().find(expected) != string::npos);
  }

  void assertDoesNotContain(DiagCollector& diagCollector, const string& expected)
  {
    CPPUNIT_ASSERT_MESSAGE(diagCollector.str(), diagCollector.str().find(expected) == string::npos);
  }

  void test_displayRuleLevel_NewSameGroupFields_EmptyWhenNotActivated()
  {
    DiagCollector collector;
    matchRule->diagCollector() = &collector;
    matchRule->displayRuleLevel(ruleLevelExcl);

    CPPUNIT_ASSERT_EQUAL(string(""), collector.str());
  }

  void test_displayRuleLevel_NewSameGroupFields_NotEmptyWhenActivated()
  {
    matchRule->displayRuleLevel(ruleLevelExcl);
    CPPUNIT_ASSERT(diagCollector.str().length() > 0);
  }

  void test_displayRuleLevel_NewSameGroupFields_EnableHIPApplySameGrp()
  {
    ruleLevelExcl.hipSameGroupAppl() = 'P';
    matchRule->displayRuleLevel(ruleLevelExcl);
    assertContains(diagCollector, "HIP DONOTAPPLY -   DONOTUSEFORCOMP -   APPLYSAMEGRPCOMP - P");
  }

  void test_displayRuleLevel_NewSameGroupFields_DisableHIPApplySameGrp()
  {
    ruleLevelExcl.hipSameGroupAppl() = 'N';
    matchRule->displayRuleLevel(ruleLevelExcl);
    assertContains(diagCollector, "HIP DONOTAPPLY -   DONOTUSEFORCOMP -   APPLYSAMEGRPCOMP - N");
  }

  void test_displayRuleLevel_NewSameGroupFields_EnableCTMApplySameGrp()
  {
    ruleLevelExcl.ctmSameGroupAppl() = 'P';
    matchRule->displayRuleLevel(ruleLevelExcl);
    assertContains(diagCollector, "CTM DONOTAPPLY -   DONOTUSEFORCOMP -   APPLYSAMEGRPCOMP - P");
  }

  void test_displayRuleLevel_NewSameGroupFields_DisableCTMApplySameGrp()
  {
    ruleLevelExcl.ctmSameGroupAppl() = 'N';
    matchRule->displayRuleLevel(ruleLevelExcl);
    assertContains(diagCollector, "CTM DONOTAPPLY -   DONOTUSEFORCOMP -   APPLYSAMEGRPCOMP - N");
  }

  void test_displayRuleLevel_NewSameGroupFields_EnableBHCApplySameGrp()
  {
    ruleLevelExcl.backhaulSameGroupAppl() = 'P';
    matchRule->displayRuleLevel(ruleLevelExcl);
    assertContains(diagCollector, "BHC DONOTAPPLY -   DONOTUSEFORCOMP -   APPLYSAMEGRPCOMP - P");
  }

  void test_displayRuleLevel_NewSameGroupFields_DisableBHCApplySameGrp()
  {
    ruleLevelExcl.backhaulSameGroupAppl() = 'N';
    matchRule->displayRuleLevel(ruleLevelExcl);
    assertContains(diagCollector, "BHC DONOTAPPLY -   DONOTUSEFORCOMP -   APPLYSAMEGRPCOMP - N");
  }

  void test_displayRuleLevel_NewSameGroupFields_EnableHIPApplySameGrp_Child()
  {
    ruleLevelExcl.hipSameGroupAppl() = 'P';
    setSameFareGroupChildData();
    matchRule->displayRuleLevel(ruleLevelExcl);
    assertContains(diagCollector, "GRP - SET -     1TRF -     2RULE - abcdFT - ijkFCLS -  efghijk");
  }

  void test_displayRuleLevel_NewSameGroupFields_DisableHIPApplySameGrp_Child()
  {
    ruleLevelExcl.hipSameGroupAppl() = 'N';
    setSameFareGroupChildData();
    matchRule->displayRuleLevel(ruleLevelExcl);
    assertDoesNotContain(diagCollector,
                         "GRP - SET -     1TRF -     2RULE - abcdFT - ijkFCLS -  efghijk");
  }

  void test_checkRuleLevelExclusionSameFareGroupCheck_HIP_Compare_YES()
  {
    ruleLevelExcl.hipFareCompAppl() = 'Y';
    setSameFareGroupChildData();
    CPPUNIT_ASSERT(
        matchRule->checkRuleLevelExclusionSameFareGroupCheck(ruleLevelExcl, *paxTypeFare));
  }

  void test_checkRuleLevelExclusionSameFareGroupCheck_BHC_Compare_YES()
  {
    ruleLevelExcl.backhaulFareCompAppl() = 'Y';
    setSameFareGroupChildData();
    CPPUNIT_ASSERT(
        matchRule->checkRuleLevelExclusionSameFareGroupCheck(ruleLevelExcl, *paxTypeFare));
  }

  void test_checkRuleLevelExclusionSameFareGroupCheck_CTM_Compare_YES()
  {
    ruleLevelExcl.ctmFareCompAppl() = 'Y';
    setSameFareGroupChildData();
    CPPUNIT_ASSERT(
        matchRule->checkRuleLevelExclusionSameFareGroupCheck(ruleLevelExcl, *paxTypeFare));
  }

  void test_checkRuleLevelExclusionSameFareGroupCheck_COM_Compare_YES()
  {
    ruleLevelExcl.comFareCompAppl() = 'Y';
    setSameFareGroupChildData();
    CPPUNIT_ASSERT(
        matchRule->checkRuleLevelExclusionSameFareGroupCheck(ruleLevelExcl, *paxTypeFare));
  }

  void test_checkRuleLevelExclusionSameFareGroupCheck_DMC_Compare_YES()
  {
    ruleLevelExcl.dmcFareCompAppl() = 'Y';
    setSameFareGroupChildData();
    CPPUNIT_ASSERT(
        matchRule->checkRuleLevelExclusionSameFareGroupCheck(ruleLevelExcl, *paxTypeFare));
  }

  void test_checkRuleLevelExclusionSameFareGroupCheck_COP_Compare_YES()
  {
    ruleLevelExcl.copFareCompAppl() = 'Y';
    setSameFareGroupChildData();
    CPPUNIT_ASSERT(
        matchRule->checkRuleLevelExclusionSameFareGroupCheck(ruleLevelExcl, *paxTypeFare));
  }

  void test_checkRuleLevelExclusionSameFareGroupCheck_OSC_Compare_YES()
  {
    ruleLevelExcl.oscFareCompAppl() = 'Y';
    setSameFareGroupChildData();
    CPPUNIT_ASSERT(
        matchRule->checkRuleLevelExclusionSameFareGroupCheck(ruleLevelExcl, *paxTypeFare));
  }

  void test_checkRuleLevelExclusionSameFareGroupCheck_RSC_Compare_YES()
  {
    ruleLevelExcl.rscFareCompAppl() = 'Y';
    setSameFareGroupChildData();
    CPPUNIT_ASSERT(
        matchRule->checkRuleLevelExclusionSameFareGroupCheck(ruleLevelExcl, *paxTypeFare));
  }

  void test_checkRuleLevelExclusionSameFareGroupCheck_CPM_Compare_YES()
  {
    ruleLevelExcl.cpmFareCompAppl() = 'Y';
    setSameFareGroupChildData();
    CPPUNIT_ASSERT(
        matchRule->checkRuleLevelExclusionSameFareGroupCheck(ruleLevelExcl, *paxTypeFare));
  }

  void test_passRuleLevelExclusionSameGroup_NoDataInRuleLevelExcl_DoNotMatch()
  {
    createPaxTypeFare(*paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(
        false, matchRule->passRuleLevelExclusionSameGroup(ruleLevelExcl, *paxTypeFare, false));
    createPaxTypeThruFare(*paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(
        false, matchRule->passRuleLevelExclusionSameGroup(ruleLevelExcl, *paxTypeFare, true));
  }

  void test_passRuleLevelExclusionSameGroup_DataInRuleLevelExcl_DifferentSet_DoNotMatch()
  {
    setSameFareGroupChildData_1a();
    createPaxTypeFare(*paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(
        true, matchRule->passRuleLevelExclusionSameGroup(ruleLevelExcl, *paxTypeFare, false));
    createPaxTypeThruFare(*paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(
        false, matchRule->passRuleLevelExclusionSameGroup(ruleLevelExcl, *paxTypeFare, true));
  }

  void test_passRuleLevelExclusionSameGroup_matchInterMediateFareThruFare()
  {
    setSameFareGroupChildData_1();
    createPaxTypeFare(*paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(
        true, matchRule->passRuleLevelExclusionSameGroup(ruleLevelExcl, *paxTypeFare, false));
    createPaxTypeThruFare(*paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(
        true, matchRule->passRuleLevelExclusionSameGroup(ruleLevelExcl, *paxTypeFare, true));
  }

  void test_passRuleLevelExclusionSameGroup_doNotMatchInterMediateFareThruFare()
  {
    setSameFareGroupChildData_1();
    createPaxTypeFare(*paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(
        true, matchRule->passRuleLevelExclusionSameGroup(ruleLevelExcl, *paxTypeFare, false));
    createPaxTypeThruFare1(*paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(
        false, matchRule->passRuleLevelExclusionSameGroup(ruleLevelExcl, *paxTypeFare, true));
  }

  void test_passRuleLevelExclusionSameGroup_MatchInterMediateFareThruFare_FareType()
  {
    setSameFareGroupChildData_2();
    createPaxTypeFare(*paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(
        true, matchRule->passRuleLevelExclusionSameGroup(ruleLevelExcl, *paxTypeFare, false));
    createPaxTypeThruFare2(*paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(
        true, matchRule->passRuleLevelExclusionSameGroup(ruleLevelExcl, *paxTypeFare, true));
  }

  void test_passRuleLevelExclusionSameGroup_MatchInterMediateFareThruFare_FareRule()
  {
    setSameFareGroupChildData_3();
    createPaxTypeFare(*paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(
        true, matchRule->passRuleLevelExclusionSameGroup(ruleLevelExcl, *paxTypeFare, false));
    createPaxTypeThruFare3(*paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(
        true, matchRule->passRuleLevelExclusionSameGroup(ruleLevelExcl, *paxTypeFare, true));
  }

  void test_passRuleLevelExclusionSameGroup_MatchInterMediateFareThruFare_FareTariff()
  {
    setSameFareGroupChildData_4();
    createPaxTypeFare(*paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(
        true, matchRule->passRuleLevelExclusionSameGroup(ruleLevelExcl, *paxTypeFare, false));
    createPaxTypeThruFare4(*paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(
        true, matchRule->passRuleLevelExclusionSameGroup(ruleLevelExcl, *paxTypeFare, true));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MatchRuleLevelExclTableTest);
}
