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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"
#include "Taxes/LegacyFacades/TransactionCache.h"

#include <memory>

namespace tse
{
typedef int Key;
typedef int Value;
typedef std::shared_ptr<const Value> SharedConstValue;

using testing::Return;

class MockDataReader
{
public:
  MOCK_METHOD1(getVal, Value (const Key&));

  SharedConstValue
  operator()(const Key& key)
  {
    return SharedConstValue(new Value(getVal(key)));
  }
};

class TransactionCacheTest : public testing::Test
{
public:
  void SetUp()
  {
    _dataReader.reset(new testing::StrictMock<MockDataReader>());
    _cache.reset(new TransactionCache<Key, Value>(boost::ref(*_dataReader)));
  }

  void TearDown() {}

protected:
  std::unique_ptr<testing::StrictMock<MockDataReader>> _dataReader;
  std::unique_ptr<TransactionCache<Key, Value>> _cache;
};

TEST_F(TransactionCacheTest, testGet)
{
  EXPECT_CALL(*_dataReader, getVal(1)).WillOnce(Return(2));
  EXPECT_CALL(*_dataReader, getVal(2)).WillOnce(Return(4));
  EXPECT_CALL(*_dataReader, getVal(4)).WillOnce(Return(5));

  ASSERT_EQ(2, *_cache->get(1));
  ASSERT_EQ(4, *_cache->get(2));
  ASSERT_EQ(2, *_cache->get(1));
  ASSERT_EQ(5, *_cache->get(4));
  ASSERT_EQ(4, *_cache->get(2));
  ASSERT_EQ(5, *_cache->get(4));
  ASSERT_EQ(4, *_cache->get(2));
  ASSERT_EQ(2, *_cache->get(1));
  ASSERT_EQ(5, *_cache->get(4));
}

TEST_F(TransactionCacheTest, testGetWithReset)
{
  EXPECT_CALL(*_dataReader, getVal(1)).WillOnce(Return(2)).WillOnce(Return(20));
  EXPECT_CALL(*_dataReader, getVal(2)).WillOnce(Return(4));
  EXPECT_CALL(*_dataReader, getVal(4)).WillOnce(Return(5)).WillOnce(Return(31));

  ASSERT_EQ(2, *_cache->get(1));
  ASSERT_EQ(4, *_cache->get(2));
  ASSERT_EQ(2, *_cache->get(1));
  ASSERT_EQ(5, *_cache->get(4));
  ASSERT_EQ(4, *_cache->get(2));
  ASSERT_EQ(5, *_cache->get(4));
  ASSERT_EQ(4, *_cache->get(2));

  _cache.reset(new TransactionCache<Key, Value>(boost::ref(*_dataReader)));

  ASSERT_EQ(20, *_cache->get(1));
  ASSERT_EQ(31, *_cache->get(4));
}

}
