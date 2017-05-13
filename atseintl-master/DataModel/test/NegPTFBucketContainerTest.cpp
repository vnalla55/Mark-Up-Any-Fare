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

#include <algorithm>
#include <set>

#include <boost/assign/std/vector.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"
#include "Common/TseConsts.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DataModel/NegPaxTypeFareData.h"
#include "DataModel/NegPaxTypeFareDataComparator.h"
#include "DataModel/NegPTFBucketContainer.h"

#include "test/include/TestMemHandle.h"

using boost::assign::operator+=;
using ::testing::ByRef;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::_;

namespace tse
{
struct MockNegPaxTypeFareDataComparator : public NegPaxTypeFareDataComparator
{
public:
  MOCK_CONST_METHOD2(areEquivalent, bool(const NegPaxTypeFareData&, const NegPaxTypeFareData&));
};

class MockFaresGetter
{
public:
  MOCK_CONST_METHOD0(getFares, std::set<PaxTypeFare*>());
};

struct MockBucket : public NegPTFBucket
{
public:
  MockBucket() : _faresGetter(0) {}
  MockBucket(const MockBucket& bucket) : _faresGetter(0) {}
  template <typename Validator>
  void collectValidNegFares(Validator validator, std::set<PaxTypeFare*>& result) const
  {
    if (_faresGetter)
      _faresGetter->getFares().swap(result);
  }

  MockFaresGetter* _faresGetter;
};

class NegPTFBucketContainerTest : public ::testing::Test
{
protected:
  struct ValidatorStub
  {
  };
  typedef MockNegPaxTypeFareDataComparator Comparator;
  typedef NegPTFBucketContainer<Comparator, MockBucket> BucketContainer;
  TestMemHandle _memHandle;
  Comparator* _comparator;
  BucketContainer* _buckets;

  void initDuplicatedNoDirectionality(NegPaxTypeFareData& data1, NegPaxTypeFareData& data2) const
  {
    data1.psgType = data2.psgType = ADULT;
    data1.isPricingOption = data2.isPricingOption = true;
    data1.isQualifierPresent = data2.isQualifierPresent = false;
    data1.isDirectionalityApplied = data2.isDirectionalityApplied = false;
  }

  void
  initNotDuplicatedDifferentPaxTypes(NegPaxTypeFareData& data1, NegPaxTypeFareData& data2) const
  {
    data1.psgType = ADULT;
    data2.psgType = CHILD;
  }

  std::vector<MockBucket>& getBucketVec(BucketContainer& bc) const { return bc._buckets; }

public:
  void SetUp()
  {
    _buckets = _memHandle.create<BucketContainer>();
    _comparator = &_buckets->getComparator();
  }

  void TearDown() { _memHandle.clear(); }
};

TEST_F(NegPTFBucketContainerTest, testInsertOne)
{
  NegPaxTypeFareData data;
  _buckets->insert(data);

  ASSERT_EQ(1u, _buckets->size());
  ASSERT_EQ(1u, _buckets->begin()->size());
  ASSERT_EQ(1u, _buckets->begin()->begin()->size());
  ASSERT_EQ(data, _buckets->begin()->begin()->back());
}

TEST_F(NegPTFBucketContainerTest, testInsertTwoNotDuplicated)
{
  BucketContainer& buckets = *_buckets;

  NegPaxTypeFareData data1, data2;

  EXPECT_CALL(*_comparator, areEquivalent(_, data2)).WillOnce(Return(false));

  buckets.insert(data1);
  buckets.insert(data2);

  ASSERT_EQ(2u, buckets.size());
}

TEST_F(NegPTFBucketContainerTest, testInsertTwoDuplicated)
{
  BucketContainer& buckets = *_buckets;

  NegPaxTypeFareData data1, data2;
  CategoryRuleItemInfo catRuleItemInfo;
  data1.catRuleItemInfo = data2.catRuleItemInfo = &catRuleItemInfo;

  EXPECT_CALL(*_comparator, areEquivalent(_, data2)).WillOnce(Return(true));

  buckets.insert(data1);
  buckets.insert(data2);

  ASSERT_EQ(1u, buckets.size());
}

TEST_F(NegPTFBucketContainerTest, testCollectValidNegFares)
{
  BucketContainer& buckets = *_buckets;

  ValidatorStub validator;
  std::set<PaxTypeFare*> fares1, fares2, fares3;
  for (size_t i = 0; i < 4; ++i)
    fares1.insert(_memHandle.create<PaxTypeFare>());
  for (size_t i = 0; i < 2; ++i)
    fares3.insert(_memHandle.create<PaxTypeFare>());

  std::set<PaxTypeFare*> result;

  getBucketVec(buckets).resize(3);
  MockBucket& bucket0 = getBucketVec(buckets)[0];
  MockBucket& bucket1 = getBucketVec(buckets)[1];
  MockBucket& bucket2 = getBucketVec(buckets)[2];
  bucket0._faresGetter = _memHandle.create<MockFaresGetter>();
  bucket1._faresGetter = _memHandle.create<MockFaresGetter>();
  bucket2._faresGetter = _memHandle.create<MockFaresGetter>();

  EXPECT_CALL(*bucket0._faresGetter, getFares()).WillOnce(Return(ByRef(fares1)));
  EXPECT_CALL(*bucket1._faresGetter, getFares()).WillOnce(Return(ByRef(fares2)));
  EXPECT_CALL(*bucket2._faresGetter, getFares()).WillOnce(Return(ByRef(fares3)));

  buckets.collectValidNegFares(validator, result);

  ASSERT_EQ(fares1.size() + fares2.size() + fares3.size(), result.size());
  ASSERT_TRUE(std::includes(result.begin(), result.end(), fares1.begin(), fares1.end()));
  ASSERT_TRUE(std::includes(result.begin(), result.end(), fares2.begin(), fares2.end()));
  ASSERT_TRUE(std::includes(result.begin(), result.end(), fares3.begin(), fares3.end()));
}

} // tse
