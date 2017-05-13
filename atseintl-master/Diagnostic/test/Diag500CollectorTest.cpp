#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "Diagnostic/Diag500Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
class Diag500CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag500CollectorTest);
  CPPUNIT_TEST(testPrintHeader);
  CPPUNIT_TEST(testPrintRulePhase_NormalValidation);
  CPPUNIT_TEST(testPrintRulePhase_PreValidation);
  CPPUNIT_TEST(testPrintRulePhase_PreCombinabilityValidation);
  CPPUNIT_TEST(testPrintRulePhase_PURuleValidation);
  CPPUNIT_TEST(testPrintRulePhase_FPRuleValidation);
  CPPUNIT_TEST(testPrintRulePhase_DynamicValidation);
  CPPUNIT_TEST(testPrintRulePhase_ShoppingComponentValidation);
  CPPUNIT_TEST(testPrintRulePhase_ShoppingComponentWithFlightsValidation);
  CPPUNIT_TEST(testDiag500Collector);
  CPPUNIT_TEST(testDiag500Collector_StatusRule_Pass);
  CPPUNIT_TEST(testDiag500Collector_StatusRule_Skip);
  CPPUNIT_TEST(testDiag500Collector_StatusRule_Fail);
  CPPUNIT_TEST(testDiag500Collector_StatusRule_Soft);
  CPPUNIT_TEST(testDiag500Collector_StatusRule_Stop);
  CPPUNIT_TEST(testDiag500Collector_StatusRule_StopSOft);
  CPPUNIT_TEST(testDiag500Collector_StatusRule_Incorrect);
  CPPUNIT_TEST(testDiag500Collector_FootNoteCtrlInfo);
  CPPUNIT_TEST(testDiag500Collector_FareByRuleCtrlInfo);
  CPPUNIT_SKIP_TEST(testDisplayRelation_If);
  CPPUNIT_SKIP_TEST(testDisplayRelation_Then);
  CPPUNIT_SKIP_TEST(testDisplayRelation_Or);
  CPPUNIT_SKIP_TEST(testDisplayRelation_And);
  CPPUNIT_SKIP_TEST(testDisplayRelation_Else);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  Diag500Collector* _diag;

public:
  void setUp()
  {
    Diagnostic* diag = _memHandle.insert(new Diagnostic(Diagnostic500));
    diag->activate();
    _diag = _memHandle.insert(new Diag500Collector(*diag));
    _diag->enable(Diagnostic500);
  }

  void tearDown() { _memHandle.clear(); }

  void testPrintHeader()
  {
    CPPUNIT_ASSERT(_diag->isActive());
    _diag->printHeader();

    string expected;
    expected += "---------------------------------------------------------------\n";
    expected += "PAX MARKET FARE     FARE     O VND CXR TRF RULE  SEQ    SEG RUL\n";
    expected += "           CLASS    BASIS    I         NBR NBR   NBR    CNT CAT\n";
    expected += "---------------------------------------------------------------\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testPrintRulePhase_NormalValidation()
  {
    _diag->printRulePhase(NormalValidation);
    string expected = "RULE PHASE: FARE COMPONENT VALIDATION\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testPrintRulePhase_PreValidation()
  {
    _diag->printRulePhase(PreValidation);
    string expected = "RULE PHASE: PRE-VALIDATION\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testPrintRulePhase_PreCombinabilityValidation()
  {
    _diag->printRulePhase(PreCombinabilityValidation);
    string expected = "RULE PHASE: PRE-COMBINABILITY VALIDATION\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testPrintRulePhase_PURuleValidation()
  {
    _diag->printRulePhase(PURuleValidation);
    string expected = "RULE PHASE: PU FACTORY RULE VALIDATION\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testPrintRulePhase_FPRuleValidation()
  {
    _diag->printRulePhase(FPRuleValidation);
    string expected = "RULE PHASE: FAREPATH FACTORY RULE VALIDATION\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testPrintRulePhase_DynamicValidation()
  {
    _diag->printRulePhase(DynamicValidation);
    string expected = "RULE PHASE: DYNAMIC-VALIDATION\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testPrintRulePhase_ShoppingComponentValidation()
  {
    _diag->printRulePhase(ShoppingComponentValidation);
    string expected = "RULE PHASE: SHOPPING-COMPONENT-VALIDATION\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testPrintRulePhase_ShoppingComponentWithFlightsValidation()
  {
    _diag->printRulePhase(ShoppingComponentWithFlightsValidation);
    string expected = "RULE PHASE: SHOPPING-COMPONENT-WITH-FLIGHTS-VALIDATION\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void preparePaxTypeFare(PaxTypeFare& paxFare)
  {
    paxFare.fareMarket() = _memHandle.create<FareMarket>();

    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = "NYC";
    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = "KRK";

    paxFare.fareMarket()->origin() = origin;
    paxFare.fareMarket()->destination() = destination;

    TariffCrossRefInfo* tcr = _memHandle.create<TariffCrossRefInfo>();

    Fare* fare = _memHandle.create<Fare>();
    fare->setTariffCrossRefInfo(tcr);

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->fareClass() = "FAREID";

    fare->setFareInfo(fareInfo);
    paxFare.setFare(fare);
  }

  void prepareGeneralFareRuleInfo(GeneralFareRuleInfo& rule)
  {
    rule.vendorCode() = "ATP";
    rule.carrierCode() = "AA";
    rule.tariffNumber() = 123;
    rule.ruleNumber() = "456";

    rule.sequenceNumber() = 789987;
    rule.segcount() = 5;
  }

  void testDiag500Collector()
  {
    PaxTypeFare* paxFare = _memHandle.create<PaxTypeFare>();
    GeneralFareRuleInfo* rule = _memHandle.create<GeneralFareRuleInfo>();
    preparePaxTypeFare(*paxFare);
    prepareGeneralFareRuleInfo(*rule);
    _diag->diag500Collector(*paxFare, rule, false);
    string expected;
    expected = "-R1:NYCKRK FAREID   FAREID   I ATP  AA 123  456  789987  5  G0 \n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDiag500Collector_StatusRule_Pass()
  {
    _diag->diag500Collector(PASS);
    string expected;
    expected += "                                              ---PASS--- ";
    expected += "\n---------------------------------------------------------\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDiag500Collector_StatusRule_Skip()
  {
    _diag->diag500Collector(SKIP);
    string expected;
    expected += "                                              ---SKIP--- ";
    expected += "\n---------------------------------------------------------\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDiag500Collector_StatusRule_Fail()
  {
    _diag->diag500Collector(FAIL);
    string expected;
    expected += "                                              ---FAIL--- ";
    expected += "\n---------------------------------------------------------\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDiag500Collector_StatusRule_Soft()
  {
    _diag->diag500Collector(SOFTPASS);
    string expected;
    expected += "                                              ---SOFT--- ";
    expected += "\n---------------------------------------------------------\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDiag500Collector_StatusRule_Stop()
  {
    _diag->diag500Collector(STOP);
    string expected;
    expected += "                                              ---FAIL--- ";
    expected += "\n---------------------------------------------------------\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDiag500Collector_StatusRule_StopSOft()
  {
    _diag->diag500Collector(STOP_SOFT);
    string expected;
    expected += "                                              ---SOFT--- ";
    expected += "\n---------------------------------------------------------\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDiag500Collector_StatusRule_Incorrect()
  {
    _diag->diag500Collector((Record3ReturnTypes)22);
    string expected;
    expected += "                                              ---XXXX--- ";
    expected += "\n---------------------------------------------------------\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void prepareFootNoteCtrlInfo(FootNoteCtrlInfo& rule)
  {
    rule.fareTariff() = 321;
    rule.footNote() = "BC";
    rule.sequenceNumber() = 456456;
    rule.segcount() = 8;
    rule.categoryNumber() = 15;
  }

  void testDiag500Collector_FootNoteCtrlInfo()
  {
    PaxTypeFare* paxFare = _memHandle.create<PaxTypeFare>();
    FootNoteCtrlInfo* rule = _memHandle.create<FootNoteCtrlInfo>();
    preparePaxTypeFare(*paxFare);
    prepareFootNoteCtrlInfo(*rule);
    _diag->diag500Collector(*paxFare, rule);
    string expected;
    expected = "-R1:NYCKRK FAREID   I R2:        321  BC   456456  8   F15\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void prepareFareByRuleCtrlInfo(FareByRuleCtrlInfo& rule)
  {
    rule.sequenceNumber() = 123123;
    rule.categoryNumber() = 25;
  }

  void testDiag500Collector_FareByRuleCtrlInfo()
  {
    PaxTypeFare* paxFare = _memHandle.create<PaxTypeFare>();
    FareByRuleCtrlInfo* rule = _memHandle.create<FareByRuleCtrlInfo>();
    preparePaxTypeFare(*paxFare);
    prepareFareByRuleCtrlInfo(*rule);
    _diag->diag500Collector(*paxFare, rule);
    string expected;
    expected = "-R1:NYCKRK FAREID   I R2:        0         123123   R25\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void dataForDisplayRelation(PaxTypeFare& paxFare, CategoryRuleItemInfo& rule)
  {
    preparePaxTypeFare(paxFare);
    rule.setItemcat(15);
    rule.setItemNo(10);
    rule.setDirectionality(' ');
    rule.setInOutInd(' ');
  }

  void testDisplayRelation_If()
  {
    PaxTypeFare* paxFare = _memHandle.create<PaxTypeFare>();
    CategoryRuleItemInfo* rule = _memHandle.create<CategoryRuleItemInfo>();
    dataForDisplayRelation(*paxFare, *rule);

    _diag->displayRelation(*paxFare, rule, PASS);
    string expected = "            IF      15 - 10       DIR-N  O/I-N   PASS  \n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDisplayRelation_Then()
  {
    PaxTypeFare* paxFare = _memHandle.create<PaxTypeFare>();
    CategoryRuleItemInfo* rule = _memHandle.create<CategoryRuleItemInfo>();
    dataForDisplayRelation(*paxFare, *rule);
    rule->setRelationalInd(CategoryRuleItemInfo::THEN);

    _diag->displayRelation(*paxFare, rule, FAIL);
    string expected = "            THEN    15 - 10       DIR-N  O/I-N   FAIL  \n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDisplayRelation_Or()
  {
    PaxTypeFare* paxFare = _memHandle.create<PaxTypeFare>();
    CategoryRuleItemInfo* rule = _memHandle.create<CategoryRuleItemInfo>();
    dataForDisplayRelation(*paxFare, *rule);
    rule->setRelationalInd(CategoryRuleItemInfo::OR);

    _diag->displayRelation(*paxFare, rule, SKIP);
    string expected = "            OR      15 - 10       DIR-N  O/I-N   SKIP  \n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDisplayRelation_And()
  {
    PaxTypeFare* paxFare = _memHandle.create<PaxTypeFare>();
    CategoryRuleItemInfo* rule = _memHandle.create<CategoryRuleItemInfo>();
    dataForDisplayRelation(*paxFare, *rule);
    rule->setRelationalInd(CategoryRuleItemInfo::AND);

    _diag->displayRelation(*paxFare, rule, SOFTPASS);
    string expected = "            AND     15 - 10       DIR-N  O/I-N   SOFT  \n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDisplayRelation_Else()
  {
    PaxTypeFare* paxFare = _memHandle.create<PaxTypeFare>();
    CategoryRuleItemInfo* rule = _memHandle.create<CategoryRuleItemInfo>();
    dataForDisplayRelation(*paxFare, *rule);
    rule->setRelationalInd(CategoryRuleItemInfo::ELSE);

    _diag->displayRelation(*paxFare, rule, STOP);
    string expected = "            ELSE    15 - 10       DIR-N  O/I-N   FAIL  \n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag500CollectorTest);
}
