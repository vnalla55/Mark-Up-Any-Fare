#include "test/include/CppUnitHelperMacros.h"
#include "Pricing/test/PricingOrchestratorTestShoppingCommon.h"
#include "test/include/TestConfigInitializer.h"

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Pricing/Shopping/Diversity/DiversityModelBasic.h"
#include "Pricing/Shopping/Diversity/DmcRequirementsFacade.h"
#include "Pricing/Shopping/Diversity/test/DiversityTestUtil.h"

#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{

namespace shpq
{

class DiversityModelBasic_AdditionalNonStopsTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(DiversityModelBasic_AdditionalNonStopsTest);

  CPPUNIT_TEST(testContinueProcessingOneMore);
  CPPUNIT_TEST(testContinueProcessingAllFound);
  CPPUNIT_TEST(testContinueProcessingNothingToSeekFor);
  CPPUNIT_TEST(testContinueProcessingNeedMoreButNothingLeft);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testContinueProcessingOneMore()
  {
    DiversityModelBasic* model = setupAdditionalNonStops(4, 8, 1, 1, 0.5, 15.0);

    CPPUNIT_ASSERT_EQUAL(true, model->continueProcessing(10.0));
    CPPUNIT_ASSERT_EQUAL(false, model->continueProcessing(20.0));
  }

  void testContinueProcessingAllFound()
  {
    DiversityModelBasic* model = setupAdditionalNonStops(4, 4, 1, 2, 0.5, 15.0);

    CPPUNIT_ASSERT_EQUAL(true, model->continueProcessing(10.0));
    CPPUNIT_ASSERT_EQUAL(false, model->continueProcessing(20.0));
  }

  void testContinueProcessingNothingToSeekFor()
  {
    DiversityModelBasic* model = setupAdditionalNonStops(4, 4, 1, 0, 0.0, 15.0);

    CPPUNIT_ASSERT_EQUAL(true, model->continueProcessing(10.0));
    CPPUNIT_ASSERT_EQUAL(false, model->continueProcessing(20.0));
  }

  void testContinueProcessingNeedMoreButNothingLeft()
  {
    DiversityModelBasic* model = setupAdditionalNonStops(4, 1, 1, 0, 0.5, 15.0);

    CPPUNIT_ASSERT_EQUAL(true, model->continueProcessing(10.0));
    CPPUNIT_ASSERT_EQUAL(false, model->continueProcessing(20.0));
  }

protected:
  DiversityModelBasic* setupAdditionalNonStops(int16_t optionsToGenerate,
                                               size_t maxNonStopCount,
                                               size_t normalFound,
                                               size_t additionalFound,
                                               float percentage,
                                               float cutoffAmount)
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* trx = PricingOrchestratorTestShoppingCommon::createOwTrx(
        *dataHandle, "/vobs/atseintl/Pricing/Shopping/PQ/test/testdata/ShoppingNGSTrx.xml", true);

    Diversity& diversity = trx->diversity();
    PricingOptions* options = trx->getOptions();
    options->setRequestedNumberOfSolutions(optionsToGenerate);
    diversity.setNonStopOptionsCount(0u);
    diversity.setNonStopOptionsPercentage(percentage);
    diversity.initialize(0, *trx, std::map<ItinIndex::Key, CarrierCode>());
    diversity.setBucketDistribution(Diversity::GOLD, 0.0);
    diversity.setBucketDistribution(Diversity::LUXURY, 0.0);
    diversity.setBucketDistribution(Diversity::JUNK, 0.0);
    diversity.setBucketDistribution(Diversity::UGLY, 0.0);

    DiversityTestUtil diversityTestUtil(diversity);
    diversityTestUtil.setMaxOnlineNonStopCount(maxNonStopCount);

    diversity.setFareCutoffAmount(cutoffAmount);

    ItinStatisticMock* stats = _memHandle.create<ItinStatisticMock>(*trx);
    stats->setEnabledStatistics(ItinStatistic::STAT_NON_STOP_COUNT, this);
    stats->setNonStopCount(normalFound);
    stats->setAdditionalNonStopCount(additionalFound);

    DiagCollector* dc = 0;

    return _memHandle.create<DiversityModelBasic>(*trx, *stats, dc);
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

CPPUNIT_TEST_SUITE_REGISTRATION(DiversityModelBasic_AdditionalNonStopsTest);
} // shpq

} // tse
