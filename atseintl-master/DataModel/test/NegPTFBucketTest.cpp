// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include <boost/assign/std/vector.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"
#include "DataModel/NegPTFBucket.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/CategoryRuleItemInfo.h"

#include "test/include/TestMemHandle.h"

using boost::assign::operator+=;
using ::testing::Return;

namespace tse
{

class NegPTFBucketTest : public ::testing::Test
{
protected:
  class NegFareValidatorMock
  {
  public:
    MOCK_METHOD1(isValid, bool(PaxTypeFare*));
    bool operator()(PaxTypeFare* ptf) { return isValid(ptf); }
  };

  NegFareValidatorMock* _validator;
  NegPTFBucket* _bucket;
  TestMemHandle _memHandle;

  std::vector<NegPTFBucket::R3NegFares>& getBucketData(NegPTFBucket& bucket) const
  {
    return bucket._data;
  }

public:
  void SetUp()
  {
    _validator = _memHandle.create<NegFareValidatorMock>();
    _bucket = _memHandle.create<NegPTFBucket>();
  }
  void TearDown() { _memHandle.clear(); }
};

TEST_F(NegPTFBucketTest, testCollectValidNegFaresFirstValid)
{
  const size_t size = 3;
  std::vector<PaxTypeFare*> ptf(size);
  std::vector<NegPaxTypeFareData> negData(size);
  for (size_t i = 0; i < size; ++i)
  {
    ptf[i] = _memHandle.create<PaxTypeFare>();
    negData[i].ptf = ptf[i];
  }

  std::set<PaxTypeFare*> result;
  std::vector<NegPTFBucket::R3NegFares>& data = getBucketData(*_bucket);
  data.resize(2);
  data[0] += negData[0], negData[1]; // valid
  data[1] += negData[2]; // whatever

  EXPECT_CALL(*_validator, isValid(ptf[0])).WillOnce(Return(true));
  EXPECT_CALL(*_validator, isValid(ptf[1])).Times(0);
  EXPECT_CALL(*_validator, isValid(ptf[2])).Times(0);
  _bucket->collectValidNegFares<NegFareValidatorMock&>(*_validator, result);

  ASSERT_EQ(2u, result.size());
  ASSERT_EQ(1u, result.count(ptf[0]));
  ASSERT_EQ(1u, result.count(ptf[1]));
}

TEST_F(NegPTFBucketTest, testCollectValidNegFaresFirstInvalid)
{
  const size_t size = 5;
  std::vector<PaxTypeFare*> ptf(size);
  std::vector<NegPaxTypeFareData> negData(size);
  for (size_t i = 0; i < size; ++i)
  {
    ptf[i] = _memHandle.create<PaxTypeFare>();
    negData[i].ptf = ptf[i];
  }

  std::set<PaxTypeFare*> result;
  std::vector<NegPTFBucket::R3NegFares>& data = getBucketData(*_bucket);
  data.resize(3);
  data[0] += negData[0], negData[1]; // invalid
  data[1] += negData[2]; // valid
  data[2] += negData[3], negData[4]; // whatever

  EXPECT_CALL(*_validator, isValid(ptf[0])).WillOnce(Return(false));
  EXPECT_CALL(*_validator, isValid(ptf[1])).Times(0);
  EXPECT_CALL(*_validator, isValid(ptf[2])).WillOnce(Return(true));
  EXPECT_CALL(*_validator, isValid(ptf[3])).Times(0);
  EXPECT_CALL(*_validator, isValid(ptf[4])).Times(0);
  _bucket->collectValidNegFares<NegFareValidatorMock&>(*_validator, result);

  ASSERT_EQ(1u, result.size());
  ASSERT_EQ(1u, result.count(ptf[2]));
}

TEST_F(NegPTFBucketTest, testInsertTwoSameR3)
{
  NegPTFBucket& bucket = *_bucket;

  NegPaxTypeFareData data1, data2;
  CategoryRuleItemInfo catRuleItemInfo;
  data1.catRuleItemInfo = data2.catRuleItemInfo = &catRuleItemInfo;

  bucket.insert(data1);
  bucket.insert(data2);

  ASSERT_EQ(1u, bucket.size());
  ASSERT_EQ(2u, bucket[0].size());
}

TEST_F(NegPTFBucketTest, testInsertTwoDifferentR3)
{
  NegPTFBucket& bucket = *_bucket;

  NegPaxTypeFareData data1, data2;
  CategoryRuleItemInfo catRuleItemInfo1, catRuleItemInfo2;
  catRuleItemInfo1.setItemNo(10);
  catRuleItemInfo2.setItemNo(20);
  data1.catRuleItemInfo = &catRuleItemInfo1;
  data2.catRuleItemInfo = &catRuleItemInfo2;

  bucket.insert(data1);
  bucket.insert(data2);

  ASSERT_EQ(2u, bucket.size());
  ASSERT_EQ(1u, bucket[0].size());
  ASSERT_EQ(1u, bucket[1].size());
}
}
