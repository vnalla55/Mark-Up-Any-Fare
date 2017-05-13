#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/FareCompInfo.h"
#include "RexPricing/PenaltyAdjuster.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class PenaltyAdjusterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PenaltyAdjusterTest);

  CPPUNIT_TEST(test_FCZero);
  CPPUNIT_TEST(test_FUZero);
  CPPUNIT_TEST(test_FCOnlyOne);
  CPPUNIT_TEST(test_FUOnlyOne);
  CPPUNIT_TEST(test_FCThree);
  CPPUNIT_TEST(test_FUThree);
  CPPUNIT_TEST(test_FCThreeWithSurcharge);
  CPPUNIT_TEST_SUITE_END();

private:
  PricingUnit* _pu;
  TestMemHandle _memory;

public:
  void setUp()
  {
    _pu = _memory.insert(new PricingUnit);
    MinFarePlusUpItem* mfpui = _memory.insert(new MinFarePlusUpItem);
    mfpui->plusUpAmount = 2.0;
    _pu->minFarePlusUp().addItem(CTM, mfpui);
  }

  void tearDown() { _memory.clear(); }

  void addFareUsage(const MoneyAmount& amt)
  {
    _pu->fareUsage().push_back(_memory.insert(new FareUsage));
    _pu->fareUsage().back()->paxTypeFare() = _memory.insert(new PaxTypeFare);
    _pu->fareUsage().back()->paxTypeFare()->setFare(_memory.insert(new Fare));
    _pu->fareUsage().back()->paxTypeFare()->nucFareAmount() = amt;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(amt, _pu->fareUsage().back()->totalFareAmount(), EPSILON);
  }

  void addFareComponent(const MoneyAmount& amt)
  {
    _pu->fareUsage().push_back(_memory.insert(new FareUsage));
    _pu->fareUsage().back()->paxTypeFare() = _memory.insert(new PaxTypeFare);
    _pu->fareUsage().back()->paxTypeFare()->fareMarket() = _memory.insert(new FareMarket);
    _pu->fareUsage().back()->paxTypeFare()->fareMarket()->fareCompInfo() =
        _memory.insert(new FareCompInfo);
    _pu->fareUsage().back()->paxTypeFare()->fareMarket()->fareCompInfo()->fareCalcFareAmt() = amt;
  }

  void test_FCZero()
  {
    addFareComponent(5.0);
    _pu->minFarePlusUp().clear();

    PenaltyAdjuster pa(*_pu, PenaltyAdjuster::SUMARIZE_FC);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, pa.adjustedPuAmt(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, pa.adjustedFuAmt(*_pu->fareUsage().back()), EPSILON);
  }

  void test_FUZero()
  {
    addFareUsage(5.0);
    _pu->minFarePlusUp().clear();

    PenaltyAdjuster pa(*_pu, PenaltyAdjuster::SUMARIZE_FU);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, pa.adjustedPuAmt(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, pa.adjustedFuAmt(*_pu->fareUsage().back()), EPSILON);
  }

  void test_FCOnlyOne()
  {
    addFareComponent(5.0);

    PenaltyAdjuster pa(*_pu, PenaltyAdjuster::SUMARIZE_FC);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, pa.adjustedPuAmt(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, pa.adjustedFuAmt(*_pu->fareUsage()[0]), EPSILON);
  }

  void test_FUOnlyOne()
  {
    addFareUsage(5.0);

    PenaltyAdjuster pa(*_pu, PenaltyAdjuster::SUMARIZE_FU);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, pa.adjustedPuAmt(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, pa.adjustedFuAmt(*_pu->fareUsage()[0]), EPSILON);
  }

  void test_FCThree()
  {
    addFareComponent(1.0);
    addFareComponent(2.0);
    addFareComponent(3.0);

    PenaltyAdjuster pa(*_pu, PenaltyAdjuster::SUMARIZE_FC);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, pa.adjustedPuAmt(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0 + 1.0 / 3.0, pa.adjustedFuAmt(*_pu->fareUsage()[0]), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0 + 2.0 / 3.0, pa.adjustedFuAmt(*_pu->fareUsage()[1]), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, pa.adjustedFuAmt(*_pu->fareUsage()[2]), EPSILON);
  }

  void test_FUThree()
  {
    addFareUsage(1.0);
    addFareUsage(2.0);
    addFareUsage(3.0);

    PenaltyAdjuster pa(*_pu, PenaltyAdjuster::SUMARIZE_FU);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, pa.adjustedPuAmt(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0 + 1.0 / 3.0, pa.adjustedFuAmt(*_pu->fareUsage()[0]), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0 + 2.0 / 3.0, pa.adjustedFuAmt(*_pu->fareUsage()[1]), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, pa.adjustedFuAmt(*_pu->fareUsage()[2]), EPSILON);
  }

  void test_FCThreeWithSurcharge()
  {
    addFareComponent(1.0);
    addFareComponent(2.0);
    addFareComponent(0.0);
    _pu->fareUsage()[2]->surchargeAmt() = 3.0;

    PenaltyAdjuster pa(*_pu, PenaltyAdjuster::SUMARIZE_FC);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, pa.adjustedPuAmt(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0 + 1.0 / 3.0, pa.adjustedFuAmt(*_pu->fareUsage()[0]), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0 + 2.0 / 3.0, pa.adjustedFuAmt(*_pu->fareUsage()[1]), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, pa.adjustedFuAmt(*_pu->fareUsage()[2]), EPSILON);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PenaltyAdjusterTest);

} // end of tse namespace
