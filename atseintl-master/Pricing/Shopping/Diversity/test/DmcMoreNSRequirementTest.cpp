#include "test/include/CppUnitHelperMacros.h"
#include "Pricing/test/PricingOrchestratorTestShoppingCommon.h"

#include "Common/TsePrimitiveTypes.h"
#include "Pricing/Shopping/Diversity/DmcAtLeastOneNSRequirement.h"
#include "Pricing/Shopping/Diversity/DmcBucketRequirement.h"
#include "Pricing/Shopping/Diversity/DmcCarrierRequirement.h"
#include "Pricing/Shopping/Diversity/DmcCustomRequirement.h"
#include "Pricing/Shopping/Diversity/DmcMoreNSRequirement.h"
#include "Pricing/Shopping/Diversity/DmcNSPerCarrierRequirement.h"
#include "Pricing/Shopping/Diversity/DmcNSRequirementCommonCheck.h"
#include "Pricing/Shopping/Diversity/DmcRequirementsFacade.h"
#include "Pricing/Shopping/PQ/SoloGroupFarePath.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"

#include "Pricing/Shopping/Diversity/test/DiversityTestUtil.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{

namespace shpq
{

class DmcMoreNSRequirementTest : public CppUnit::TestFixture
{
  typedef DmcMoreNSRequirement::Value Value;

  CPPUNIT_TEST_SUITE(DmcMoreNSRequirementTest);

  CPPUNIT_TEST(testGetStatusOneMore);
  CPPUNIT_TEST(testGetStatusAllFound);
  CPPUNIT_TEST(testGetStatusNothingToSeekFor);
  CPPUNIT_TEST(testGetStatusNeedMoreButNothingLeft);

  CPPUNIT_TEST(testGetCouldSatisfyAdjustmentOneMorePositive);
  CPPUNIT_TEST(testGetCouldSatisfyAdjustmentOneMoreNegative);

  CPPUNIT_TEST(testGetCouldSatisfyAdjustmentAllFoundPositive);
  CPPUNIT_TEST(testGetCouldSatisfyAdjustmentAllFoundNegative);

  CPPUNIT_TEST(testGetCouldSatisfyAdjustmentNothingToSeekForPositive);
  CPPUNIT_TEST(testGetCouldSatisfyAdjustmentNothingToSeekForNegative);

  CPPUNIT_TEST(testGetCouldSatisfyAdjustmentNeedMoreButNothingLeftPositive);
  CPPUNIT_TEST(testGetCouldSatisfyAdjustmentNeedMoreButNothingLeftNegative);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testGetStatusOneMore()
  {
    DmcMoreNSRequirement* requirement = setupGetStatus(4, 4, 1, 0.5);

    CPPUNIT_ASSERT_EQUAL(DmcMoreNSRequirement::Value(DmcRequirement::NEED_ADDITIONAL_NONSTOPS),
                         requirement->getStatus());
  }

  void testGetStatusAllFound()
  {
    DmcMoreNSRequirement* requirement = setupGetStatus(4, 4, 2, 0.5);

    CPPUNIT_ASSERT_EQUAL(DmcMoreNSRequirement::Value(0), requirement->getStatus());
  }

  void testGetStatusNothingToSeekFor()
  {
    DmcMoreNSRequirement* requirement = setupGetStatus(4, 4, 1, 0.0);

    CPPUNIT_ASSERT_EQUAL(DmcMoreNSRequirement::Value(0), requirement->getStatus());
  }

  void testGetStatusNeedMoreButNothingLeft()
  {
    DmcMoreNSRequirement* requirement = setupGetStatus(4, 0, 0, 0.5);

    CPPUNIT_ASSERT_EQUAL(DmcMoreNSRequirement::Value(0), requirement->getStatus());
  }

  void testGetCouldSatisfyAdjustmentOneMorePositive()
  {
    DmcMoreNSRequirement* requirement = setupGetStatus(4, 4, 1, 0.5);

    CPPUNIT_ASSERT_EQUAL(DmcMoreNSRequirement::Value(DmcRequirement::NEED_ADDITIONAL_NONSTOPS),
                         requirement->getCouldSatisfyAdjustment(0));
  }

  void testGetCouldSatisfyAdjustmentOneMoreNegative()
  {
    DmcMoreNSRequirement* requirement = setupGetStatus(4, 4, 1, 0.5);

    CPPUNIT_ASSERT_EQUAL(DmcMoreNSRequirement::Value(0),
                         requirement->getCouldSatisfyAdjustment(32));
  }

  void testGetCouldSatisfyAdjustmentAllFoundPositive()
  {
    DmcMoreNSRequirement* requirement = setupGetStatus(4, 4, 2, 0.5);

    CPPUNIT_ASSERT_EQUAL(DmcMoreNSRequirement::Value(0), requirement->getCouldSatisfyAdjustment(0));
  }

  void testGetCouldSatisfyAdjustmentAllFoundNegative()
  {
    DmcMoreNSRequirement* requirement = setupGetStatus(4, 4, 2, 0.5);

    CPPUNIT_ASSERT_EQUAL(DmcMoreNSRequirement::Value(0),
                         requirement->getCouldSatisfyAdjustment(32));
  }

  void testGetCouldSatisfyAdjustmentNothingToSeekForPositive()
  {
    DmcMoreNSRequirement* requirement = setupGetStatus(4, 4, 1, 0.0);

    CPPUNIT_ASSERT_EQUAL(DmcMoreNSRequirement::Value(0), requirement->getCouldSatisfyAdjustment(0));
  }

  void testGetCouldSatisfyAdjustmentNothingToSeekForNegative()
  {
    DmcMoreNSRequirement* requirement = setupGetStatus(4, 4, 1, 0.0);

    CPPUNIT_ASSERT_EQUAL(DmcMoreNSRequirement::Value(0),
                         requirement->getCouldSatisfyAdjustment(32));
  }

  void testGetCouldSatisfyAdjustmentNeedMoreButNothingLeftPositive()
  {
    DmcMoreNSRequirement* requirement = setupGetStatus(4, 0, 0, 0.5);

    CPPUNIT_ASSERT_EQUAL(DmcMoreNSRequirement::Value(0), requirement->getCouldSatisfyAdjustment(0));
  }

  void testGetCouldSatisfyAdjustmentNeedMoreButNothingLeftNegative()
  {
    DmcMoreNSRequirement* requirement = setupGetStatus(4, 0, 0, 0.5);

    CPPUNIT_ASSERT_EQUAL(DmcMoreNSRequirement::Value(0),
                         requirement->getCouldSatisfyAdjustment(32));
  }

protected:
  DmcMoreNSRequirement* setupGetStatus(int16_t optionsToGenerate,
                                       size_t maxNonStopCount,
                                       size_t alreadyFound,
                                       float percentage)
  {
    ShoppingTrx* trx = TestShoppingTrxFactory::create(
        "/vobs/atseintl/Pricing/Shopping/PQ/test/testdata/ShoppingNGSTrx.xml", true);

    Diversity* diversity = _memHandle.create<Diversity>();
    diversity->setNonStopOptionsPercentage(percentage);

    DiversityTestUtil diversityTestUtil(*diversity);

    diversityTestUtil.setNumberOfOptionsToGenerate(optionsToGenerate);
    diversityTestUtil.setMaxOnlineNonStopCount(maxNonStopCount);
    diversityTestUtil.setMaxInterlineNonStopCount(0);
    diversityTestUtil.setNonStopOptionsCount(static_cast<size_t>(percentage * optionsToGenerate));

    ItinStatisticMock* stats = _memHandle.create<ItinStatisticMock>(*trx);
    stats->setEnabledStatistics(ItinStatistic::STAT_NON_STOP_COUNT, this);
    stats->setAdditionalNonStopCount(alreadyFound);

    ContextMock* sharedCtx = _memHandle.create<ContextMock>(*diversity, *stats, *trx);

    return _memHandle.create<DmcMoreNSRequirement>(*sharedCtx);
  }

  // Mocks
  class ItinStatisticMock : public ItinStatistic
  {
  public:
    ItinStatisticMock(ShoppingTrx& trx) : ItinStatistic(trx) {}

    void setAdditionalNonStopCount(int count) { _additionalOnlineNonStopsCount = count; }
  };

  class ContextMock : public DmcRequirementsSharedContext
  {
  public:
    ContextMock(Diversity& diversity, ItinStatistic& stats, ShoppingTrx& trx)
      : DmcRequirementsSharedContext(diversity, stats, 0, trx)
    {
    }

    void printRequirements(bool bucketsOnly) {};
    void printCarriersRequirements(bool directOnly = false) {};
  };

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(DmcMoreNSRequirementTest);
} // shpq
} // tse
