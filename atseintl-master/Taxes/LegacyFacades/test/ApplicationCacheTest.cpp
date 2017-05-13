// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "DBAccess/ChildCacheNotifier.h"
#include "test/include/GtestHelperMacros.h"
#include "Taxes/LegacyFacades/ApplicationCache.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

namespace tse
{
typedef int Key;
typedef int Value;
typedef std::shared_ptr<Value> SharedValue;

using testing::Return;

class MockDataReader
{
public:
  MOCK_METHOD1(getVal, Value (const Key&));

  SharedValue
  operator()(const Key& key)
  {
    Value val = getVal(key);
    return SharedValue(new Value(val));
  }
};

template <typename KeyT>
class MockCacheChildNotifier : public ChildCacheNotifier<KeyT>
{
public:
  void removeKey(KeyT key) { this->keyRemoved(key); }
  void clearCache() { this->cacheCleared(); }
};

class ApplicationCacheTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _dataReader.reset(new testing::StrictMock<MockDataReader>());
    _cache.reset(new ApplicationCache<Key, Value>());
  }

  void TearDown() {}

protected:
  std::unique_ptr<testing::StrictMock<MockDataReader>> _dataReader;
  std::unique_ptr<ApplicationCache<Key, Value>> _cache;
};

TEST_F(ApplicationCacheTest, testGet)
{
  EXPECT_CALL(*_dataReader, getVal(1)).WillOnce(Return(2));

  ASSERT_EQ(2, *_cache->get(1, boost::ref(*_dataReader)));
  ASSERT_EQ(2, *_cache->get(1, boost::ref(*_dataReader)));
}

TEST_F(ApplicationCacheTest, testDeleteKey)
{
  MockCacheChildNotifier<Key> notifier;
  _cache->setCacheNotifier(&notifier);

  EXPECT_CALL(*_dataReader, getVal(1)).WillOnce(Return(2)).WillOnce(Return(3));
  EXPECT_CALL(*_dataReader, getVal(2)).WillOnce(Return(2));

  ASSERT_EQ(2, *_cache->get(1, boost::ref(*_dataReader)));
  ASSERT_EQ(2, *_cache->get(2, boost::ref(*_dataReader)));

  notifier.removeKey(1);

  ASSERT_EQ(3, *_cache->get(1, boost::ref(*_dataReader)));
  ASSERT_EQ(2, *_cache->get(2, boost::ref(*_dataReader)));
}

TEST_F(ApplicationCacheTest, testClear)
{
  MockCacheChildNotifier<Key> notifier;
  _cache->setCacheNotifier(&notifier);

  EXPECT_CALL(*_dataReader, getVal(1)).WillOnce(Return(2)).WillOnce(Return(3));
  EXPECT_CALL(*_dataReader, getVal(2)).WillOnce(Return(2)).WillOnce(Return(4));

  ASSERT_EQ(2, *_cache->get(1, boost::ref(*_dataReader)));
  ASSERT_EQ(2, *_cache->get(2, boost::ref(*_dataReader)));

  notifier.clearCache();

  ASSERT_EQ(3, *_cache->get(1, boost::ref(*_dataReader)));
  ASSERT_EQ(4, *_cache->get(2, boost::ref(*_dataReader)));
}

}
