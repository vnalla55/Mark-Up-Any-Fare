#include "test/include/CppUnitHelperMacros.h"
#include "Pricing/test/PricingOrchestratorTestShoppingCommon.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Pricing/Shopping/Diversity/test/DiversityTestUtil.h"
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

#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{

namespace shpq
{

class DmcRequirementsFacadeTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(DmcRequirementsFacadeTest);

  CPPUNIT_TEST(testAdditionalNonStopsOneMore);
  CPPUNIT_TEST(testAdditionalNonStopsAllFound);
  CPPUNIT_TEST(testAdditionalNonStopsNothingToSeekFor);
  CPPUNIT_TEST(testAdditionalNonStopsNeedMoreButNothingLeft);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testAdditionalNonStopsOneMore()
  {
    DmcRequirementsFacade* requirement = setupAdditionalNonStops(4, 8, 1, 1, 0.5);

    CPPUNIT_ASSERT(DmcRequirement::NEED_ADDITIONAL_NONSTOPS == requirement->getStatus());
  }

  void testAdditionalNonStopsAllFound()
  {
    DmcRequirementsFacade* requirement = setupAdditionalNonStops(4, 4, 1, 2, 0.5);

    CPPUNIT_ASSERT(0 == requirement->getStatus());
  }

  void testAdditionalNonStopsNothingToSeekFor()
  {
    DmcRequirementsFacade* requirement = setupAdditionalNonStops(4, 4, 1, 0, 0.0);

    CPPUNIT_ASSERT(0 == requirement->getStatus());
  }

  void testAdditionalNonStopsNeedMoreButNothingLeft()
  {
    DmcRequirementsFacade* requirement = setupAdditionalNonStops(4, 1, 1, 0, 0.5);

    CPPUNIT_ASSERT(0 == requirement->getStatus());
  }

protected:
  DmcRequirementsFacade* setupAdditionalNonStops(int16_t optionsToGenerate,
                                                 size_t maxNonStopCount,
                                                 size_t normalFound,
                                                 size_t additionalFound,
                                                 float percentage)
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* trx = PricingOrchestratorTestShoppingCommon::createOwTrx(
        *dataHandle, "/vobs/atseintl/Pricing/Shopping/PQ/test/testdata/ShoppingNGSTrx.xml", true);

    Diversity& diversity = trx->diversity();
    PricingOptions* options = trx->getOptions();
    options->setRequestedNumberOfSolutions(optionsToGenerate);
    diversity.setNonStopOptionsPercentage(percentage);
    diversity.initialize(0, *trx, std::map<ItinIndex::Key, CarrierCode>());
    diversity.setBucketDistribution(Diversity::GOLD, 0.0);
    diversity.setBucketDistribution(Diversity::LUXURY, 0.0);
    diversity.setBucketDistribution(Diversity::JUNK, 0.0);
    diversity.setBucketDistribution(Diversity::UGLY, 0.0);

    DiversityTestUtil diversityTestUtil(diversity);
    diversityTestUtil.setMaxOnlineNonStopCount(maxNonStopCount);

    ItinStatisticMock* stats = _memHandle.create<ItinStatisticMock>(*trx);
    stats->setEnabledStatistics(ItinStatistic::STAT_NON_STOP_COUNT, this);
    stats->setNonStopCount(normalFound);
    stats->setAdditionalNonStopCount(additionalFound);

    DiagCollector* dc = 0;

    return _memHandle.create<DmcRequirementsFacade>(*stats, dc, *trx);
  }

  // Mocks
  class ItinStatisticMock : public ItinStatistic
  {
  public:
    ItinStatisticMock(ShoppingTrx& trx) : ItinStatistic(trx) {}

    void setAdditionalNonStopCount(size_t count) { _additionalOnlineNonStopsCount = count; }
    void setNonStopCount(size_t count) { _onlineNonStopsCount = count; }
  };

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(DmcRequirementsFacadeTest);
} // shpq

} // tse
