// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Common/Config/ConfigurableValue.h"
#include "Common/Memory/Config.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>

namespace tse
{
namespace Memory
{

namespace
{
constexpr size_t GIGABYTE = 1024L * 1024 * 1024;
constexpr size_t THRESHOLD_CRIT = 25 * GIGABYTE;
constexpr size_t THRESHOLD_SOFT = 20 * GIGABYTE;
constexpr size_t WATERMARK_CRIT = 6 * GIGABYTE;
constexpr size_t WATERMARK_SOFT = 9 * GIGABYTE;

}

using namespace ::testing;

class ConfigTest : public Test
{
public:
  void SetUp()
  {
    _memH(new TestConfigInitializer);
    _logger = _memH(new TestLogger("atseintl.Common.Memory.Config"));
    _criticalThreshold = 0;
    _globalThreshold = 0;
    _criticalFreeRssWatermark = 0;
    _softFreeRssWatermark = 0;
  }
  void TearDown()
  {
    _memH.clear();
  }

protected:
  void setUpThresholds(size_t crit, size_t soft)
  {
    TestConfigInitializer::setValue("CRITICAL_THRESHOLD", crit, "MEMORY");
    TestConfigInitializer::setValue("GLOBAL_THRESHOLD", soft, "MEMORY");
  }
  void setUpWatermarks(size_t crit, size_t soft)
  {
    TestConfigInitializer::setValue("CRITICAL_FREE_RSS_WATERMARK", crit, "MEMORY");
    TestConfigInitializer::setValue("SOFT_FREE_RSS_WATERMARK", soft, "MEMORY");
  }

  TestMemHandle _memH;
  TestLogger* _logger;
};

TEST_F(ConfigTest, testConfigure_Defaults)
{
  configure();

  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), criticalThreshold);
  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), globalThreshold);
  ASSERT_EQ((size_t) 0, criticalFreeRssWatermark);
  ASSERT_EQ((size_t) 0, softFreeRssWatermark);
}

TEST_F(ConfigTest, testConfigure_ThresholdsCriticalIsZero)
{
  setUpThresholds(0, THRESHOLD_SOFT);
  setUpWatermarks(0, 0);

  configure();

  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), criticalThreshold);
  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), globalThreshold);
  ASSERT_EQ((size_t) 0, criticalFreeRssWatermark);
  ASSERT_EQ((size_t) 0, softFreeRssWatermark);
}

TEST_F(ConfigTest, testConfigure_ThresholdsSoftIsZero)
{
  setUpThresholds(THRESHOLD_CRIT, 0);
  setUpWatermarks(0, 0);

  configure();

  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), criticalThreshold);
  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), globalThreshold);
  ASSERT_EQ((size_t) 0, criticalFreeRssWatermark);
  ASSERT_EQ((size_t) 0, softFreeRssWatermark);
}

TEST_F(ConfigTest, testConfigure_ThresholdsDefault)
{
  setUpThresholds(THRESHOLD_CRIT, THRESHOLD_SOFT);
  setUpWatermarks(0, 0);

  configure();

  ASSERT_EQ((size_t) THRESHOLD_CRIT, criticalThreshold);
  ASSERT_EQ((size_t) THRESHOLD_SOFT, globalThreshold);
  ASSERT_EQ((size_t) 0, criticalFreeRssWatermark);
  ASSERT_EQ((size_t) 0, softFreeRssWatermark);
}

TEST_F(ConfigTest, testConfigure_ThresholdsButCriticalWatermarkIsNotZero)
{
  setUpThresholds(THRESHOLD_CRIT, THRESHOLD_SOFT);
  setUpWatermarks(WATERMARK_CRIT, 0);

  configure();

  ASSERT_EQ((size_t) THRESHOLD_CRIT, criticalThreshold);
  ASSERT_EQ((size_t) THRESHOLD_SOFT, globalThreshold);
  ASSERT_EQ((size_t) 0, criticalFreeRssWatermark);
  ASSERT_EQ((size_t) 0, softFreeRssWatermark);
}

TEST_F(ConfigTest, testConfigure_ThresholdsButSoftWatermarkIsNotZero)
{
  setUpThresholds(THRESHOLD_CRIT, THRESHOLD_SOFT);
  setUpWatermarks(0, WATERMARK_SOFT);

  configure();

  ASSERT_EQ((size_t) THRESHOLD_CRIT, criticalThreshold);
  ASSERT_EQ((size_t) THRESHOLD_SOFT, globalThreshold);
  ASSERT_EQ((size_t) 0, criticalFreeRssWatermark);
  ASSERT_EQ((size_t) 0, softFreeRssWatermark);
}

TEST_F(ConfigTest, testConfigure_WatermarksDefault)
{
  setUpThresholds(0, 0);
  setUpWatermarks(WATERMARK_CRIT, WATERMARK_SOFT);

  configure();

  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), criticalThreshold);
  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), globalThreshold);
  ASSERT_EQ((size_t) WATERMARK_CRIT, criticalFreeRssWatermark);
  ASSERT_EQ((size_t) WATERMARK_SOFT, softFreeRssWatermark);
}

TEST_F(ConfigTest, testConfigure_WatermarksButCriticalThresholdIsNotZero)
{
  setUpThresholds(THRESHOLD_CRIT, 0);
  setUpWatermarks(WATERMARK_CRIT, WATERMARK_SOFT);

  configure();

  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), criticalThreshold);
  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), globalThreshold);
  ASSERT_EQ((size_t) WATERMARK_CRIT, criticalFreeRssWatermark);
  ASSERT_EQ((size_t) WATERMARK_SOFT, softFreeRssWatermark);
}

TEST_F(ConfigTest, testConfigure_WatermarksButSoftThresholdIsNotZero)
{
  setUpThresholds(0, THRESHOLD_SOFT);
  setUpWatermarks(WATERMARK_CRIT, WATERMARK_SOFT);

  configure();

  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), criticalThreshold);
  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), globalThreshold);
  ASSERT_EQ((size_t) WATERMARK_CRIT, criticalFreeRssWatermark);
  ASSERT_EQ((size_t) WATERMARK_SOFT, softFreeRssWatermark);
}

TEST_F(ConfigTest, testConfigure_WatermarksCriticalAboveSoft)
{
  setUpThresholds(0, 0);
  setUpWatermarks(WATERMARK_SOFT, WATERMARK_CRIT);

  configure();

  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), criticalThreshold);
  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), globalThreshold);
  ASSERT_EQ((size_t) WATERMARK_CRIT, criticalFreeRssWatermark);
  ASSERT_EQ((size_t) WATERMARK_CRIT, softFreeRssWatermark);
}

TEST_F(ConfigTest, testConfigure_WatermarksCriticalIsZero)
{
  setUpThresholds(0, 0);
  setUpWatermarks(0, WATERMARK_SOFT);
  configure();

  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), criticalThreshold);
  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), globalThreshold);
  ASSERT_EQ((size_t) 0, criticalFreeRssWatermark);
  ASSERT_EQ((size_t) 0, softFreeRssWatermark);
}

TEST_F(ConfigTest, testConfigure_WatermarksSoftIsZero)
{
  setUpThresholds(0, 0);
  setUpWatermarks(WATERMARK_CRIT, 0);
  configure();

  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), criticalThreshold);
  ASSERT_EQ((size_t) std::numeric_limits<size_t>::max(), globalThreshold);
  ASSERT_EQ((size_t) 0, criticalFreeRssWatermark);
  ASSERT_EQ((size_t) 0, softFreeRssWatermark);
}

}
}
