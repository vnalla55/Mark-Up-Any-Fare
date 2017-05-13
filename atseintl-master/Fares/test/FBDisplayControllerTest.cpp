#include "test/include/CppUnitHelperMacros.h"

#include "Fares/FBDisplayController.h"
#include "test/include/TestMemHandle.h"
#include "Rules/RuleConst.h"
#include "DBAccess/GeneralFareRuleInfo.h"

namespace tse
{
class FBDisplayControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FBDisplayControllerTest);

  CPPUNIT_TEST(fareRuleTakesPrecedenceOverGeneralRule_false_noRuleInfo);
  CPPUNIT_TEST(fareRuleTakesPrecedenceOverGeneralRule_false_cat);
  CPPUNIT_TEST(fareRuleTakesPrecedenceOverGeneralRule_true_cat8);
  CPPUNIT_TEST(fareRuleTakesPrecedenceOverGeneralRule_true_cat9);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _trx = _memHandle.create<FareDisplayTrx>();
    _fbDisplayController = _memHandle.insert(new FBDisplayController(*_trx));
    _ruleInfo = _memHandle.create<GeneralFareRuleInfo>();
    _ruleInfo->segcount() = 1;
  }

  void tearDown() { _memHandle.clear(); }

protected:
  TestMemHandle _memHandle;
  FareDisplayTrx* _trx;
  FBDisplayController* _fbDisplayController;
  GeneralFareRuleInfo* _ruleInfo;

public:
  // TESTS
  void fareRuleTakesPrecedenceOverGeneralRule_false_noRuleInfo()
  {
    CPPUNIT_ASSERT(!_fbDisplayController->fareRuleTakesPrecedenceOverGeneralRule(0, 0));
  }

  void fareRuleTakesPrecedenceOverGeneralRule_false_cat()
  {
    CPPUNIT_ASSERT(!_fbDisplayController->fareRuleTakesPrecedenceOverGeneralRule(
        _ruleInfo, RuleConst::ELIGIBILITY_RULE));
  }

  void fareRuleTakesPrecedenceOverGeneralRule_true_cat8()
  {
    CPPUNIT_ASSERT(_fbDisplayController->fareRuleTakesPrecedenceOverGeneralRule(
        _ruleInfo, RuleConst::STOPOVER_RULE));
  }

  void fareRuleTakesPrecedenceOverGeneralRule_true_cat9()
  {
    CPPUNIT_ASSERT(_fbDisplayController->fareRuleTakesPrecedenceOverGeneralRule(
        _ruleInfo, RuleConst::TRANSFER_RULE));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FBDisplayControllerTest);
} // tse
