#include "Common/CurrencyConversionFacade.h"
#include "Common/Money.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "Server/TseServer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#define TOLERANCE .0000000001
#define EXAUSTIVE_PENNY_COUNT 1000000

namespace tse
{

class CurrencyConversionFacadeNucRounding : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(CurrencyConversionFacadeNucRounding);
  CPPUNIT_TEST(RoundingHasNoEffectOnEvenPennyAmounts);
  CPPUNIT_TEST(OneTenthOfAPennyRoundsDownward);
  CPPUNIT_TEST(NineTenthsOfAPennyTruncatesRatherThanRounding);
  CPPUNIT_TEST(ValueKnownToBeTroublesomeRoundsCleanly);
  CPPUNIT_SKIP_TEST(Exaustive_callOnce);
  CPPUNIT_SKIP_TEST(Exaustive_callTwice);
  CPPUNIT_TEST(FindMagicValuesThatFail);
  CPPUNIT_TEST(noConvertZero);
  CPPUNIT_TEST(noConversionNeededWhenSameCurrency);

  CPPUNIT_TEST(testDefineConversionDateReturnOriginalIssueDateWhenNotReissueExchange);
  CPPUNIT_TEST(testDefineConversionDateReturnHistoricalBsrDateForExchangeItin);
  CPPUNIT_TEST(testDefineConversionDateReturnFirstConverionDateWhenFlagForSecondNotOn);
  CPPUNIT_TEST(testDefineConversionDateReturnSecondConversionDateWhenFlagOn);
  CPPUNIT_TEST_SUITE_END();

  PricingTrx* _trx;
  CurrencyConversionFacade* _ccf;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _ccf = _memHandle.create<CurrencyConversionFacade>();
    _trx = _memHandle.create<PricingTrx>();
  }
  void tearDown()
  {
    _memHandle.clear();
  }
  void checkRound(double alreadyRounded) { checkRound(alreadyRounded, alreadyRounded); }
  void checkRound(double valueToRound, double expectedValue)
  {
    Money money(valueToRound, NUC);
    _ccf->round(money, *_trx);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expectedValue, money.value(), TOLERANCE);
  }

  void testDefineConversionDateReturnOriginalIssueDateWhenNotReissueExchange()
  {
    RexPricingTrx rexTrx;
    rexTrx.setOriginalTktIssueDT() = DateTime(2010, 3, 9);
    CPPUNIT_ASSERT_EQUAL(rexTrx.originalTktIssueDT(), _ccf->defineConversionDate(rexTrx));
  }

  void testDefineConversionDateReturnHistoricalBsrDateForExchangeItin()
  {
    RexPricingTrx rexTrx;
    PricingOptions options;
    rexTrx.setOptions(&options);
    rexTrx.setAnalyzingExcItin(true);
    rexTrx.setRexPrimaryProcessType('A');
    rexTrx.setHistoricalBsrRoeDate();
    rexTrx.setOriginalTktIssueDT() = DateTime(2010, 3, 9);
    rexTrx.previousExchangeDT() = DateTime(2010, 3, 10);
    CPPUNIT_ASSERT_EQUAL(rexTrx.getHistoricalBsrRoeDate(), _ccf->defineConversionDate(rexTrx));
  }

  void testDefineConversionDateReturnFirstConverionDateWhenFlagForSecondNotOn()
  {
    RexPricingTrx rexTrx;
    rexTrx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    rexTrx.setRexPrimaryProcessType('A');
    rexTrx.newItinROEConversionDate() = DateTime(2010, 3, 10);
    rexTrx.useSecondROEConversionDate() = false;
    CPPUNIT_ASSERT_EQUAL(rexTrx.newItinROEConversionDate(), _ccf->defineConversionDate(rexTrx));
  }

  void testDefineConversionDateReturnSecondConversionDateWhenFlagOn()
  {
    RexPricingTrx rexTrx;
    rexTrx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    rexTrx.setRexPrimaryProcessType('A');
    rexTrx.newItinSecondROEConversionDate() = DateTime(2010, 3, 11);
    rexTrx.useSecondROEConversionDate() = true;
    CPPUNIT_ASSERT_EQUAL(rexTrx.newItinSecondROEConversionDate(),
                         _ccf->defineConversionDate(rexTrx));
    rexTrx.useSecondROEConversionDate() = false;
    _ccf->setUseSecondRoeDate(true);
    CPPUNIT_ASSERT_EQUAL(rexTrx.newItinSecondROEConversionDate(),
                         _ccf->defineConversionDate(rexTrx));
  }

protected:
  void noConvertZero()
  {
    Money source(0.000000001, NUC);
    Money target(-1.0, NUC);
    PricingTrx* trx = 0;
    bool result = _ccf->convertCalc(target, source, *trx);
    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT_EQUAL(0.0, target.value());
  }
  void noConversionNeededWhenSameCurrency()
  {
    Money source(1.0, NUC);
    Money target(2.0, NUC);
    PricingTrx* trx = 0;
    bool result = _ccf->convertCalc(target, source, *trx);
    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT_EQUAL(1.0, target.value());
  }
  void RoundingHasNoEffectOnEvenPennyAmounts() { checkRound(123); }
  void OneTenthOfAPennyRoundsDownward() { checkRound(123.451, 123.45); }
  void NineTenthsOfAPennyTruncatesRatherThanRounding() { checkRound(123.459, 123.45); }
  void ValueKnownToBeTroublesomeRoundsCleanly() { checkRound(1587.89); }

  void FindMagicValuesThatFail()
  {
    checkRound(.56);
    checkRound(.57);
    checkRound(.58);
    checkRound(1264.9); // PL 16792
  }
  void Exaustive_callOnce()
  {
    for (int x = 0; x < EXAUSTIVE_PENNY_COUNT; x += 1)
      checkRound(x / 100.0);
  }
  void Exaustive_callTwice()
  {
    double val = 0.0;
    for (int x = 0; x < EXAUSTIVE_PENNY_COUNT; x += 1)
    {
      val = x / 100.0L;
      Money moneyOnce(val, NUC);
      _ccf->round(moneyOnce, *_trx);

      Money moneyTwice(val, NUC);
      _ccf->round(moneyTwice, *_trx);
      _ccf->round(moneyTwice, *_trx);
      //       printf ("%22.18f \t %22.18f \t %22.18f\n", val, moneyOnce.value(),
      //       moneyTwice.value());
      CPPUNIT_ASSERT_DOUBLES_EQUAL(moneyOnce.value(), moneyTwice.value(), TOLERANCE);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CurrencyConversionFacadeNucRounding);

}; // namespace
