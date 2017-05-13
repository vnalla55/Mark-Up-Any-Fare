#include <string>

#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "Fares/FareByRuleOverrideCategoryMatcher.h"
#include "Rules/RuleConst.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/MultiAirportCity.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class FareByRuleOverrideCategoryMatcherTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareByRuleOverrideCategoryMatcherTest);
  CPPUNIT_TEST(isFieldConditionalTest);
  CPPUNIT_TEST(isStringFieldConditionalTest);
  CPPUNIT_TEST(isNotFieldConditionalTest);
  CPPUNIT_TEST(isNotFieldConditionalTest2);
  CPPUNIT_TEST(isNotFieldConditionalTest3);
  CPPUNIT_TEST(isNotStringFieldConditionalTest);
  CPPUNIT_TEST(isNotStringFieldConditionalTest2);
  CPPUNIT_TEST(isNotStringFieldConditionalTest3);

  CPPUNIT_TEST(matchFareClassTest);
  CPPUNIT_TEST(matchFareClassTest2);
  CPPUNIT_TEST(noMatchFareClassTest);
  CPPUNIT_TEST(matchFareClassTest3);
  CPPUNIT_TEST(matchFareClassTest4);
  CPPUNIT_TEST(matchFareClassTest5);
  CPPUNIT_TEST(matchFareClassTest6);
  CPPUNIT_TEST(noMatchFareClassTest2);
  CPPUNIT_TEST(noMatchFareClassTest3);
  CPPUNIT_TEST(matchFareClassTest7);
  CPPUNIT_TEST(matchFareClassTest8);
  CPPUNIT_TEST(matchFareClassTest9);
  CPPUNIT_TEST(matchFareClassTest10);
  CPPUNIT_TEST(noMatchFareClassTest4);
  CPPUNIT_TEST(noMatchFareClassTest5);
  CPPUNIT_TEST(matchFareClassTest11);
  CPPUNIT_TEST(matchFareClassTest12);
  CPPUNIT_TEST(noMatchFareClassTest6);
  CPPUNIT_TEST(matchFareClassTest13);
  CPPUNIT_TEST(matchFareClassTest14);
  CPPUNIT_TEST(noMatchFareClassTest7);
  CPPUNIT_TEST(matchFareClassTest15);
  CPPUNIT_TEST(matchFareClassTest16);
  CPPUNIT_TEST(noMatchFareClassTest8);
  CPPUNIT_TEST(matchFareClassTest17);
  CPPUNIT_TEST(noMatchFareClassTest9);
  CPPUNIT_TEST(matchFareClassTest18);
  CPPUNIT_TEST(matchFareClassTest19);
  CPPUNIT_TEST(noMatchFareClassTest10);
  CPPUNIT_TEST(matchFareClassTest20);
  CPPUNIT_TEST(noMatchFareClassTest11);
  CPPUNIT_TEST(noMatchFareClassTest12);
  CPPUNIT_TEST(matchFareClassTest21);
  CPPUNIT_TEST(matchFareClassTest22);
  CPPUNIT_TEST(matchFareClassTest23);
  CPPUNIT_TEST(matchFareClassTest24);
  CPPUNIT_TEST(matchFareClassTest25);
  CPPUNIT_TEST(matchFareClassTest26);
  CPPUNIT_TEST(noMatchFareClassTest27);

  CPPUNIT_TEST(tryMatchRec2Test);
  CPPUNIT_TEST(tryMatchRec2Test2);
  CPPUNIT_TEST(tryNotMatchRec2Test);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    TestFixture::setUp();
    _mdh = _memHandle.create<MyDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

  void isFieldConditionalTest()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    Indicator fbrField = RuleConst::BLANK;
    Indicator rec2Field = 'X';

    CPPUNIT_ASSERT(matcher.isFieldConditional(fbrField, rec2Field));
  }

  void isStringFieldConditionalTest()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    std::string fbrField;
    std::string rec2Field = "X";

    CPPUNIT_ASSERT(matcher.isFieldConditional(fbrField, rec2Field));
  }

  void isNotFieldConditionalTest()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    Indicator fbrField = RuleConst::BLANK;
    Indicator rec2Field = RuleConst::BLANK;

    CPPUNIT_ASSERT(!matcher.isFieldConditional(fbrField, rec2Field));
  }

  void isNotFieldConditionalTest2()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    Indicator fbrField = 'X';
    Indicator rec2Field = RuleConst::BLANK;

    CPPUNIT_ASSERT(!matcher.isFieldConditional(fbrField, rec2Field));
  }

  void isNotFieldConditionalTest3()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    Indicator fbrField = 'X';
    Indicator rec2Field = 'X';

    CPPUNIT_ASSERT(!matcher.isFieldConditional(fbrField, rec2Field));
  }

  void isNotStringFieldConditionalTest()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    std::string fbrField;
    std::string rec2Field;

    CPPUNIT_ASSERT(!matcher.isFieldConditional(fbrField, rec2Field));
  }

  void isNotStringFieldConditionalTest2()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    std::string fbrField = "X";
    std::string rec2Field;

    CPPUNIT_ASSERT(!matcher.isFieldConditional(fbrField, rec2Field));
  }

  void isNotStringFieldConditionalTest3()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    std::string fbrField = "X";
    std::string rec2Field = "X";

    CPPUNIT_ASSERT(!matcher.isFieldConditional(fbrField, rec2Field));
  }

  void matchFareClassTest()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "-ABC";
    FareClassCode fbrFareClassName = "*ABC";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(!matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest2()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "-ABC";
    FareClassCode fbrFareClassName = "*ABCD";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(!matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void noMatchFareClassTest()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "-ABC";
    FareClassCode fbrFareClassName = "*ADD";

    CPPUNIT_ASSERT(!matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest3()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "-ABC";
    FareClassCode fbrFareClassName = "-ABC";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest4()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "-ABC";
    FareClassCode fbrFareClassName = "X-";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest5()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "-ABC";
    FareClassCode fbrFareClassName = "XABC";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(!matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest6()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "-ABC";
    FareClassCode fbrFareClassName = "XABCD";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(!matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void noMatchFareClassTest2()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "-ABC";
    FareClassCode fbrFareClassName = "ABC";

    CPPUNIT_ASSERT(!matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
  }

  void noMatchFareClassTest3()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "-ABC";
    FareClassCode fbrFareClassName = "XADBC";

    CPPUNIT_ASSERT(!matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest7()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "-ABC";
    FareClassCode fbrFareClassName;

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest8()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "X-BC";
    FareClassCode fbrFareClassName = "*ABC";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest9()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XA-C";
    FareClassCode fbrFareClassName = "*ABC";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest10()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XA-C";
    FareClassCode fbrFareClassName = "*AB1C";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void noMatchFareClassTest4()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XA-C";
    FareClassCode fbrFareClassName = "*ABD";

    CPPUNIT_ASSERT(!matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
  }

  void noMatchFareClassTest5()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XA-C";
    FareClassCode fbrFareClassName = "*DBC";

    CPPUNIT_ASSERT(!matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest11()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XA-C";
    FareClassCode fbrFareClassName = "-AC";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest12()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XA-C";
    FareClassCode fbrFareClassName = "X-";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void noMatchFareClassTest6()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XA-C";
    FareClassCode fbrFareClassName = "Z-";

    CPPUNIT_ASSERT(!matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest13()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XA-C";
    FareClassCode fbrFareClassName = "XABC";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(!matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest14()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XA-C";
    FareClassCode fbrFareClassName = "XABDC";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(!matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void noMatchFareClassTest7()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XA-C";
    FareClassCode fbrFareClassName = "XXBC";

    CPPUNIT_ASSERT(!matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest15()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XA-C";
    FareClassCode fbrFareClassName;

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest16()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XABC";
    FareClassCode fbrFareClassName = "*ABC";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void noMatchFareClassTest8()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XABC";
    FareClassCode fbrFareClassName = "*ABCD";

    CPPUNIT_ASSERT(!matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest17()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XABC";
    FareClassCode fbrFareClassName = "-ABC";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void noMatchFareClassTest9()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XABC";
    FareClassCode fbrFareClassName = "-XABC";

    CPPUNIT_ASSERT(!matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest18()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XABCDEFG";
    FareClassCode fbrFareClassName = "-XXX"; // base fare fareClass can be truncated to 8 chars

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest19()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XABC";
    FareClassCode fbrFareClassName = "X-";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void noMatchFareClassTest10()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XABC";
    FareClassCode fbrFareClassName = "Z-";

    CPPUNIT_ASSERT(!matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest20()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XABC";
    FareClassCode fbrFareClassName = "XABC";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(!matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void noMatchFareClassTest11()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XABC";
    FareClassCode fbrFareClassName = "XABCD";

    CPPUNIT_ASSERT(!matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(!matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void noMatchFareClassTest12()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XABC";
    FareClassCode fbrFareClassName = "XXBC";

    CPPUNIT_ASSERT(!matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest21()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "XABC";
    FareClassCode fbrFareClassName;

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest22()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName;
    FareClassCode fbrFareClassName = "*ACB";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(!matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest23()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName;
    FareClassCode fbrFareClassName = "-ABC";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(!matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest24()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName;
    FareClassCode fbrFareClassName = "X-";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(!matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest25()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName;
    FareClassCode fbrFareClassName = "XABC";

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(!matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void matchFareClassTest26()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName;
    FareClassCode fbrFareClassName;

    CPPUNIT_ASSERT(matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
    CPPUNIT_ASSERT(!matcher.isFareClassNameConditional(fbrFareClassName, rec2FareClassName));
  }

  void noMatchFareClassTest27()
  {
    FareByRuleItemInfo fbrItemInfo;
    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    FareClassCode rec2FareClassName = "X-";
    FareClassCode fbrFareClassName = "AXBC";

    CPPUNIT_ASSERT(!matcher.matchFareClass(fbrFareClassName, rec2FareClassName));
  }

  void tryMatchRec2Test()
  {
    FareByRuleItemInfo fbrItemInfo;
    fbrItemInfo.resultowrt() = '2';
    // fbrItemInfo.resultFareType1() empty
    fbrItemInfo.resultseasonType() = 'X';
    fbrItemInfo.resultdowType() = 'X';
    fbrItemInfo.resultFareClass1() = "ABC";

    GeneralFareRuleInfo rec2;
    rec2.owrt() = '2';
    rec2.seasonType() = 'X';
    rec2.dowType() = 'X';
    rec2.fareClass() = "ABC";

    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    Rec2Wrapper* rec2Wrapper = matcher.tryMatchRec2(rec2);
    CPPUNIT_ASSERT(rec2Wrapper);
    CPPUNIT_ASSERT(!rec2Wrapper->isConditional());
  }

  void tryMatchRec2Test2()
  {
    FareByRuleItemInfo fbrItemInfo;
    fbrItemInfo.resultowrt() = ' ';
    // fbrItemInfo.resultFareType1() empty
    fbrItemInfo.resultseasonType() = 'X';
    fbrItemInfo.resultdowType() = 'X';
    fbrItemInfo.resultFareClass1() = "ABC";

    GeneralFareRuleInfo rec2;
    rec2.owrt() = '2';
    rec2.seasonType() = 'X';
    rec2.dowType() = 'X';
    rec2.fareClass() = "ABC";

    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    Rec2Wrapper* rec2Wrapper = matcher.tryMatchRec2(rec2);
    CPPUNIT_ASSERT(rec2Wrapper);
    CPPUNIT_ASSERT(rec2Wrapper->isConditional());
  }

  void tryNotMatchRec2Test()
  {
    FareByRuleItemInfo fbrItemInfo;
    fbrItemInfo.resultowrt() = '5';
    // fbrItemInfo.resultFareType1() empty
    fbrItemInfo.resultseasonType() = 'X';
    fbrItemInfo.resultdowType() = 'X';
    fbrItemInfo.resultFareClass1() = "ABC";

    GeneralFareRuleInfo rec2;
    rec2.owrt() = '2';
    rec2.seasonType() = 'X';
    rec2.dowType() = 'X';
    rec2.fareClass() = "ABC";

    PaxTypeFare ptFare;
    FareByRuleOverrideCategoryMatcher matcher(_trx, fbrItemInfo, ptFare);

    Rec2Wrapper* rec2Wrapper = matcher.tryMatchRec2(rec2);
    CPPUNIT_ASSERT(!rec2Wrapper);
  }

private:
  class MyDataHandle : public DataHandleMock
  {
  public:
    const std::vector<MultiAirportCity*>& getMultiCityAirport(const LocCode& city)
    {
      return *_memHandle.create<std::vector<MultiAirportCity*> >();
    }

  private:
    TestMemHandle _memHandle;
  };

  PricingTrx _trx;
  MyDataHandle* _mdh;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareByRuleOverrideCategoryMatcherTest);
}
