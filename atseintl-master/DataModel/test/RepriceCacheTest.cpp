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

#include "DataModel/RepriceCache.h"
#include "DataModel/RepricingTrx.h"

#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/ThreadFactory.h"

#include <chrono>
#include <condition_variable>
#include <vector>
#include <mutex>
#include <thread>

namespace tse
{
using namespace ::testing;

class RepriceCacheTest : public Test
{
public:
  void SetUp() override
  {
    key1.fmDirectionOverride = FMDirection::UNKNOWN;
    key1.wpncsFlagIndicator = ' ';
    key1.optionsFareFamilyType = ' ';
    key1.skipRuleValidation = false;
    key1.retrieveFbrFares = false;
    key1.retrieveNegFares = false;
    key1.privateFareCheck = false;
  }

  void TearDown() override { threads.join(); }

protected:
  ThreadFactory threads;
  TestMemHandle memHandle;

  RepriceCache::Key key1;
};

TEST_F(RepriceCacheTest, testEmptyResultDoesNothing)
{
  RepriceCache::Result empty;
}

TEST_F(RepriceCacheTest, testFirstGetMisses)
{
  RepriceCache cache;
  auto result = cache.get(key1);

  EXPECT_FALSE(result.hit());
}

TEST_F(RepriceCacheTest, testAfterUnfilledMiss_GetMisses)
{
  RepriceCache cache;

  {
    auto result = cache.get(key1);
    ASSERT_FALSE(result.hit());
  }

  auto result = cache.get(key1);

  EXPECT_FALSE(result.hit());
}

TEST_F(RepriceCacheTest, testAfterUnfilledMiss_GetMisses_Concurrent)
{
  RepriceCache cache;

  bool hit1 = true, hit2 = true;

  threads.spawn([&]()
  {
    auto result = cache.get(key1);
    hit1 = result.hit();
  });
  threads.spawn([&]()
  {
    auto result = cache.get(key1);
    hit2 = result.hit();
  });

  threads.join();

  EXPECT_FALSE(hit1);
  EXPECT_FALSE(hit2);
}

TEST_F(RepriceCacheTest, testAfterFilledMiss_GetHits)
{
  RepriceCache cache;
  RepricingTrx* trx = nullptr;

  {
    auto result = cache.get(key1);
    ASSERT_FALSE(result.hit());
    result.fill(trx = memHandle.create<RepricingTrx>());
  }

  auto result = cache.get(key1);

  ASSERT_TRUE(result.hit());
  EXPECT_EQ(trx, result.result());
}

TEST_F(RepriceCacheTest, testAfterFilledMiss_GetHits_Concurrent)
{
  RepriceCache cache;

  RepricingTrx* trx1 = nullptr;
  RepricingTrx* trx2 = nullptr;

  threads.spawn([&]()
  {
    auto result = cache.get(key1);
    if (result.hit())
      trx1 = result.result();
    else
      result.fill(trx1 = memHandle.create<RepricingTrx>());
  });
  threads.spawn([&]()
  {
    auto result = cache.get(key1);
    if (result.hit())
      trx2 = result.result();
    else
      result.fill(trx2 = memHandle.create<RepricingTrx>());
  });

  threads.join();

  EXPECT_NE(nullptr, trx1);
  EXPECT_NE(nullptr, trx2);
  EXPECT_EQ(trx1, trx2);
}

TEST_F(RepriceCacheTest, testWithDifferentKeys_HitsAreSeparate)
{
  auto key2 = key1;
  key2.fmDirectionOverride = FMDirection::OUTBOUND;

  RepriceCache cache;
  RepricingTrx* trx1 = nullptr;
  RepricingTrx* trx2 = nullptr;

  {
    auto result = cache.get(key1);
    ASSERT_FALSE(result.hit());
    result.fill(trx1 = memHandle.create<RepricingTrx>());
  }

  {
    auto result = cache.get(key2);
    ASSERT_FALSE(result.hit());
    result.fill(trx2 = memHandle.create<RepricingTrx>());
  }

  {
    auto result = cache.get(key1);
    ASSERT_TRUE(result.hit());
    EXPECT_EQ(trx1, result.result());
  }

  {
    auto result = cache.get(key2);
    ASSERT_TRUE(result.hit());
    EXPECT_EQ(trx2, result.result());
  }
}

TEST_F(RepriceCacheTest, testWithDifferentKeys_HitsAreSeparate_Concurrent)
{
  auto key2 = key1;
  key2.fmDirectionOverride = FMDirection::OUTBOUND;

  RepriceCache cache;

  RepricingTrx* trx1 = nullptr;
  RepricingTrx* trx2 = nullptr;
  RepricingTrx* trx3 = nullptr;
  RepricingTrx* trx4 = nullptr;

  threads.spawn([&]()
  {
    auto result = cache.get(key1);
    if (result.hit())
      trx1 = result.result();
    else
      result.fill(trx1 = memHandle.create<RepricingTrx>());
  });
  threads.spawn([&]()
  {
    auto result = cache.get(key2);
    if (result.hit())
      trx2 = result.result();
    else
      result.fill(trx2 = memHandle.create<RepricingTrx>());
  });
  threads.spawn([&]()
  {
    auto result = cache.get(key1);
    if (result.hit())
      trx3 = result.result();
    else
      result.fill(trx3 = memHandle.create<RepricingTrx>());
  });
  threads.spawn([&]()
  {
    auto result = cache.get(key2);
    if (result.hit())
      trx4 = result.result();
    else
      result.fill(trx4 = memHandle.create<RepricingTrx>());
  });

  threads.join();

  EXPECT_NE(nullptr, trx1);
  EXPECT_NE(nullptr, trx2);
  EXPECT_NE(nullptr, trx3);
  EXPECT_NE(nullptr, trx4);

  EXPECT_EQ(trx1, trx3);
  EXPECT_EQ(trx2, trx4);

  EXPECT_NE(trx1, trx2);
}
}
