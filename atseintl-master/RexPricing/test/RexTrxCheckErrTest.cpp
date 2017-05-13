#include "test/include/CppUnitHelperMacros.h"

#include "Common/ErrorResponseException.h"
#include "Common/PaxTypeUtil.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DataModel/FareCompInfo.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "RexPricing/CheckTrxError.h"
#include "Rules/RuleConst.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestFactoryManager.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

class RexTrxCheckErrTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexTrxCheckErrTest);
  CPPUNIT_TEST(testUnableToMatchFaresException);
  CPPUNIT_TEST(testUnableToFindReissueRule);
  CPPUNIT_TEST(testMatchedFareAndFoundRuleUnknownMatchOrNot);
  CPPUNIT_TEST(testUnableToMatchReissueRule);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<RexPricingTrx>();

    ExcItin* itin = _memHandle.create<ExcItin>();
    _trx->exchangeItin().push_back(itin);

    _fc1 = _memHandle.create<FareCompInfo>();
    itin->fareComponent().push_back(_fc1);

    FareMarket* fm1 = _memHandle.create<FareMarket>();
    _fc1->fareMarket() = fm1;

    fm1->paxTypeCortege().resize(1);
    fm1->paxTypeCortege().front().paxTypeFare().clear();

    _paxTypeFare1 = _memHandle.create<PaxTypeFare>();
    _paxTypeFare1->fareMarket() = fm1;
    _paxTypeFare1->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, false);
    fm1->allPaxTypeFare().push_back(_paxTypeFare1);
    fm1->paxTypeCortege().front().paxTypeFare().push_back(_paxTypeFare1);
    _paxTypeFare1->fareMarket() = fm1;
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->departureDT() = DateTime(2016, 5, 13, 0, 0, 0);
    fm1->travelSeg().push_back(seg);

    _paxTypeFare2 = _memHandle.create<PaxTypeFare>();
    _paxTypeFare2->fareMarket() = fm1;
    _paxTypeFare2->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, false);
    fm1->allPaxTypeFare().push_back(_paxTypeFare2);
    fm1->paxTypeCortege().front().paxTypeFare().push_back(_paxTypeFare2);
    _paxTypeFare2->fareMarket() = fm1;
  }

  void tearDown()
  {
    _memHandle.clear();
    TestFactoryManager::instance()->destroyAll();
  }

  void testUnableToMatchFaresException()
  {
    _trx->trxPhase() = RexPricingTrx::REPRICE_EXCITIN_PHASE;

    CheckTrxError checkTrxError(*_trx);

    try
    {
      checkTrxError.process();
      CPPUNIT_FAIL("Did not throw exception!");
    }
    catch (ErrorResponseException e)
    {
      CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO MATCH FARES"), e.message());
      CPPUNIT_ASSERT_EQUAL(ErrorResponseException::UNABLE_TO_MATCH_FARE, e.code());
    }
    catch (CppUnit::Exception e1) { throw; }
    catch (...) { CPPUNIT_FAIL("Threw wrong exception!"); }
  }

  void testUnableToFindReissueRule()
  {
    _trx->trxPhase() = RexPricingTrx::REPRICE_EXCITIN_PHASE;
    _fc1->matchedFares().push_back(_paxTypeFare1);

    _paxTypeFare1->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, false);

    CheckTrxError checkTrxError(*_trx);

    try
    {
      checkTrxError.process();
      CPPUNIT_FAIL("Did not throw exception!");
    }
    catch (ErrorResponseException e)
    {
      CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO REPRICE - VOLUNTARY CHANGE RULES UNAVAILABLE"),
                           e.message());
      CPPUNIT_ASSERT_EQUAL(ErrorResponseException::UNABLE_TO_MATCH_REISSUE_RULES, e.code());
    }
    catch (CppUnit::Exception e1) { throw; }
    catch (...) { CPPUNIT_FAIL("Threw wrong exception!"); }

    _fc1->matchedFares().push_back(_paxTypeFare2);

    _paxTypeFare2->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, false);

    try
    {
      checkTrxError.process();
      CPPUNIT_FAIL("Did not throw exception!");
    }
    catch (ErrorResponseException e)
    {
      CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO REPRICE - VOLUNTARY CHANGE RULES UNAVAILABLE"),
                           e.message());
      CPPUNIT_ASSERT_EQUAL(ErrorResponseException::UNABLE_TO_MATCH_REISSUE_RULES, e.code());
    }
    catch (CppUnit::Exception e1) { throw; }
    catch (...) { CPPUNIT_FAIL("Threw wrong exception!"); }

    _fc1->matchedFares().push_back(_paxTypeFare1);
    _fc1->matchedFares().push_back(_paxTypeFare2);

    try
    {
      checkTrxError.process();
      CPPUNIT_FAIL("Did not throw exception!");
    }
    catch (ErrorResponseException e)
    {
      CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO REPRICE - VOLUNTARY CHANGE RULES UNAVAILABLE"),
                           e.message());
      CPPUNIT_ASSERT_EQUAL(ErrorResponseException::UNABLE_TO_MATCH_REISSUE_RULES, e.code());
    }
    catch (CppUnit::Exception e1) { throw; }
    catch (...) { CPPUNIT_FAIL("Threw wrong exception!"); }
  }

  void testMatchedFareAndFoundRuleUnknownMatchOrNot()
  {
    _trx->trxPhase() = RexPricingTrx::REPRICE_EXCITIN_PHASE;
    _fc1->matchedFares().push_back(_paxTypeFare1);

    _paxTypeFare1->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);

    CheckTrxError checkTrxError(*_trx);

    try { checkTrxError.process(); }
    catch (ErrorResponseException e) { CPPUNIT_FAIL("Unexpected error exception!"); }
    catch (...) { CPPUNIT_FAIL("Unexpected exception!"); }
  }

  void testUnableToMatchReissueRule()
  {
    _trx->trxPhase() = RexPricingTrx::MATCH_EXC_RULE_PHASE;

    FarePath* farePath = _memHandle.create<FarePath>();
    _trx->exchangeItin().front()->farePath().push_back(farePath);

    PricingUnit* pu = _memHandle.create<PricingUnit>();
    farePath->pricingUnit().push_back(pu);

    FareUsage* fu = _memHandle.create<FareUsage>();
    pu->fareUsage().push_back(fu);

    fu->paxTypeFare() = _paxTypeFare1;
    _paxTypeFare1->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);

    CheckTrxError checkTrxError(*_trx);

    try
    {
      checkTrxError.process();
      CPPUNIT_FAIL("Did not throw exception!");
    }
    catch (CppUnit::Exception e1) { throw; }
    catch (ErrorResponseException e)
    {
      CPPUNIT_ASSERT_EQUAL(std::string("UNABLE TO REPRICE - VOLUNTARY CHANGE RULES FAILED"),
                           e.message());
      CPPUNIT_ASSERT_EQUAL(ErrorResponseException::REISSUE_RULES_FAIL, e.code());
    }
    catch (...) { CPPUNIT_FAIL("Threw wrong exception!"); }

    VoluntaryChangesInfo* rec3 = _memHandle.create<VoluntaryChangesInfo>();

    _trx->reissueOptions().insertOption(_paxTypeFare1, rec3);

    try { checkTrxError.process(); }
    catch (CppUnit::Exception e1) { throw; }
    catch (...) { CPPUNIT_FAIL("Threw wrong exception!"); }

    _trx->reissueOptions().insertOption(_paxTypeFare2, rec3);

    try { checkTrxError.process(); }
    catch (CppUnit::Exception e1) { throw; }
    catch (...) { CPPUNIT_FAIL("Threw wrong exception!"); }
  }

protected:
private:
  RexPricingTrx* _trx;
  FareCompInfo* _fc1;
  PaxTypeFare* _paxTypeFare1;
  PaxTypeFare* _paxTypeFare2;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(RexTrxCheckErrTest);

} // tse
