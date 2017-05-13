//-------------------------------------------------------------------
//
//  Authors:     Artur de Sousa Rocha
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/include/GtestHelperMacros.h"

#include "Common/TNBrands/BrandingOptionSpacesDeduplicator.h"

#include <memory>

using namespace ::testing;

namespace tse
{

namespace skipper
{

class MockFarePathBrandKeyBuilder
{
public:
  using FarePathBrandKey = std::vector<GoverningCarrierBrand>;
  MOCK_CONST_METHOD3(buildKey, FarePathBrandKeyBuilder<>::FarePathBrandKey(\
                                 const PricingTrx&, const Itin&, const FarePath&));
};

class BrandingOptionSpacesDeduplicatorTest: public Test
{
public:
  typedef BrandingOptionSpacesDeduplicatorTemplate<MockFarePathBrandKeyBuilder>
    TestedBrandingOptionSpacesDeduplicator;

  void SetUp()
  {
    _trx.reset(new PricingTrx);

    _keyBuilder = new MockFarePathBrandKeyBuilder();
    _deduplicator.reset(new TestedBrandingOptionSpacesDeduplicator(*_trx, _keyBuilder));

    _itin.reset(new Itin);
    _farePath0.reset(new FarePath);
    _farePath0a.reset(new FarePath);
    _farePath1.reset(new FarePath);
    _farePath2.reset(new FarePath);
    _paxType1.reset(new PaxType);
    _paxType2.reset(new PaxType);
  }

  void TearDown(){}

  MockFarePathBrandKeyBuilder* _keyBuilder;
  std::shared_ptr<TestedBrandingOptionSpacesDeduplicator> _deduplicator;

  std::shared_ptr<Itin> _itin;
  std::shared_ptr<FarePath> _farePath0;
  std::shared_ptr<FarePath> _farePath0a;
  std::shared_ptr<FarePath> _farePath1;
  std::shared_ptr<FarePath> _farePath2;
  std::shared_ptr<PaxType> _paxType1;
  std::shared_ptr<PaxType> _paxType2;
  std::shared_ptr<PricingTrx> _trx;
};


TEST_F(BrandingOptionSpacesDeduplicatorTest, testNoCheapest)
{
  EXPECT_CALL(*_keyBuilder, buildKey(_, _, _))
    .Times(0);

  ASSERT_EQ(INVALID_BRAND_INDEX, _deduplicator->findDuplicateOptionIndex(*_itin));
}

TEST_F(BrandingOptionSpacesDeduplicatorTest, testOnlyCheapest)
{
  _farePath0->brandIndex() = CHEAPEST_OPTION_INDEX;
  _farePath0->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath0.get());

  FarePathBrandKeyBuilder<>::FarePathBrandKey key;
  key.push_back(GoverningCarrierBrand("VA", NO_BRAND));

  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath0)))
    .WillOnce(Return(key));

  ASSERT_EQ(INVALID_BRAND_INDEX, _deduplicator->findDuplicateOptionIndex(*_itin));
}

TEST_F(BrandingOptionSpacesDeduplicatorTest, testDeduplicateCheapestSinglePax)
{
  _farePath0->brandIndex() = CHEAPEST_OPTION_INDEX;
  _farePath0->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath0.get());
  _farePath1->brandIndex() = CHEAPEST_OPTION_INDEX + 1;
  _farePath1->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath1.get());
  _farePath2->brandIndex() = CHEAPEST_OPTION_INDEX + 2;
  _farePath2->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath2.get());

  FarePathBrandKeyBuilder<>::FarePathBrandKey key1, key2;
  key1.push_back(GoverningCarrierBrand("VA", "SL"));
  key2.push_back(GoverningCarrierBrand("VA", "SV"));

  // Order not relevant
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath0)))
    .WillOnce(Return(key1));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath1)))
    .WillOnce(Return(key1));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath2)))
    .WillOnce(Return(key2));

  ASSERT_EQ(_farePath1->brandIndex(), _deduplicator->findDuplicateOptionIndex(*_itin));
}

TEST_F(BrandingOptionSpacesDeduplicatorTest, testCheapestOptionIsNotADuplicate)
{
  _farePath0->brandIndex() = CHEAPEST_OPTION_INDEX;
  _farePath0->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath0.get());
  _farePath1->brandIndex() = CHEAPEST_OPTION_INDEX + 1;
  _farePath1->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath1.get());
  _farePath2->brandIndex() = CHEAPEST_OPTION_INDEX + 2;
  _farePath2->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath2.get());

  FarePathBrandKeyBuilder<>::FarePathBrandKey key0, key1, key2;
  key0.push_back(GoverningCarrierBrand("VA", "SV"));
  key0.push_back(GoverningCarrierBrand("VA", "SL"));
  key1.push_back(GoverningCarrierBrand("VA", "SV"));
  key1.push_back(GoverningCarrierBrand("VA", "SV"));
  key2.push_back(GoverningCarrierBrand("VA", "FL"));
  key2.push_back(GoverningCarrierBrand("VA", "FL"));

  // Order not relevant
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath0)))
    .WillOnce(Return(key0));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath1)))
    .WillOnce(Return(key1));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath2)))
    .WillOnce(Return(key2));

  ASSERT_EQ(INVALID_BRAND_INDEX, _deduplicator->findDuplicateOptionIndex(*_itin));
}

TEST_F(BrandingOptionSpacesDeduplicatorTest, testDeduplicateCheapestMultipax)
{
  _farePath0->brandIndex() = CHEAPEST_OPTION_INDEX;
  _farePath0->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath0.get());
  _farePath0a->brandIndex() = CHEAPEST_OPTION_INDEX;
  _farePath0a->paxType() = _paxType2.get();
  _itin->farePath().push_back(_farePath0a.get());

  _farePath2->brandIndex() = CHEAPEST_OPTION_INDEX + 1;
  _farePath2->paxType() = _paxType2.get();
  _itin->farePath().push_back(_farePath2.get());
  _farePath1->brandIndex() = CHEAPEST_OPTION_INDEX + 1;
  _farePath1->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath1.get());

  FarePathBrandKeyBuilder<>::FarePathBrandKey key;
  key.push_back(GoverningCarrierBrand("VA", "SV"));

  // Order not relevant
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath0)))
    .WillOnce(Return(key));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath0a)))
    .WillOnce(Return(key));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath1)))
    .WillOnce(Return(key));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath2)))
    .WillOnce(Return(key));

  ASSERT_EQ(_farePath2->brandIndex(), _deduplicator->findDuplicateOptionIndex(*_itin));
}

TEST_F(BrandingOptionSpacesDeduplicatorTest, testCalculatesDeduplicationMapCorrectly)
{
  _farePath0->brandIndex() = CHEAPEST_OPTION_INDEX;
  _farePath0->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath0.get());
  _farePath0a->brandIndex() = CHEAPEST_OPTION_INDEX + 1;
  _farePath0a->paxType() = _paxType2.get();
  _itin->farePath().push_back(_farePath0a.get());

  _farePath2->brandIndex() = CHEAPEST_OPTION_INDEX + 2;
  _farePath2->paxType() = _paxType2.get();
  _itin->farePath().push_back(_farePath2.get());
  _farePath1->brandIndex() = CHEAPEST_OPTION_INDEX + 3;
  _farePath1->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath1.get());

  FarePathBrandKeyBuilder<>::FarePathBrandKey key0, key1, key2;
  key0.push_back(GoverningCarrierBrand("VA", "SV"));
  key0.push_back(GoverningCarrierBrand("VA", "SL"));
  key1.push_back(GoverningCarrierBrand("VA", "SV"));
  key1.push_back(GoverningCarrierBrand("VA", "SV"));
  key2.push_back(GoverningCarrierBrand("VA", "FL"));
  key2.push_back(GoverningCarrierBrand("VA", "FL"));


  // Order not relevant
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath0)))
    .WillOnce(Return(key0));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath0a)))
    .WillOnce(Return(key1));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath1)))
    .WillOnce(Return(key2));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath2)))
    .WillOnce(Return(key0));

  BrandingOptionSpacesDeduplicator::KeyBrandsMap expected, result;
  expected[key0].insert(_farePath0->brandIndex());
  expected[key0].insert(_farePath2->brandIndex());
  expected[key1].insert(_farePath0a->brandIndex());
  expected[key2].insert(_farePath1->brandIndex());

  _deduplicator->calculateDeduplicationMap(*_itin, result);
  ASSERT_EQ(expected, result);
}

TEST_F(BrandingOptionSpacesDeduplicatorTest, testGetBrandOptionIndexesToDeduplicate_Option_Deduplicate_Exists)
{
  _farePath0->brandIndex() = CHEAPEST_OPTION_INDEX;
  _farePath0->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath0.get());

  _farePath0a->brandIndex() = CHEAPEST_OPTION_INDEX + 1;
  _farePath0a->paxType() = _paxType2.get();
  _itin->farePath().push_back(_farePath0a.get());

  _farePath2->brandIndex() = CHEAPEST_OPTION_INDEX + 2;
  _farePath2->paxType() = _paxType2.get();
  _itin->farePath().push_back(_farePath2.get());
  _farePath1->brandIndex() = CHEAPEST_OPTION_INDEX + 3;
  _farePath1->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath1.get());

  FarePathBrandKeyBuilder<>::FarePathBrandKey key0, key1, key2;
  key0.push_back(GoverningCarrierBrand("VA", "SV"));
  key0.push_back(GoverningCarrierBrand("VA", "SL"));
  key1.push_back(GoverningCarrierBrand("VA", "SV"));
  key1.push_back(GoverningCarrierBrand("VA", "SV"));
  key2.push_back(GoverningCarrierBrand("VA", "FL"));
  key2.push_back(GoverningCarrierBrand("VA", "FL"));


  // Order not relevant
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath0)))
    .WillOnce(Return(key0));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath0a)))
    .WillOnce(Return(key1));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath1)))
    .WillOnce(Return(key2));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath2)))
    .WillOnce(Return(key0));

  BrandingOptionSpacesDeduplicator::KeyBrandsMap result;
  _deduplicator->calculateDeduplicationMap(*_itin, result);
  auto indexesToRemove = _deduplicator->getBrandOptionIndexesToDeduplicate(*_itin, result);

  ASSERT_EQ(indexesToRemove.size(), 1);
  ASSERT_EQ(indexesToRemove.count(CHEAPEST_OPTION_INDEX + 2), true);
}

TEST_F(BrandingOptionSpacesDeduplicatorTest, testGetBrandOptionIndexesToDeduplicate_Option_Deduplicate_Doesn_Exist)
{
  _farePath0->brandIndex() = CHEAPEST_OPTION_INDEX;
  _farePath0->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath0.get());

  _farePath0a->brandIndex() = CHEAPEST_OPTION_INDEX + 1;
  _farePath0a->paxType() = _paxType2.get();
  _itin->farePath().push_back(_farePath0a.get());

  _farePath1->brandIndex() = CHEAPEST_OPTION_INDEX + 2;
  _farePath1->paxType() = _paxType1.get();
  _itin->farePath().push_back(_farePath1.get());

  FarePathBrandKeyBuilder<>::FarePathBrandKey key0, key1, key2;
  key0.push_back(GoverningCarrierBrand("VA", "SV"));
  key0.push_back(GoverningCarrierBrand("VA", "SL"));
  key1.push_back(GoverningCarrierBrand("VA", "SV"));
  key1.push_back(GoverningCarrierBrand("VA", "SV"));
  key2.push_back(GoverningCarrierBrand("VA", "FL"));
  key2.push_back(GoverningCarrierBrand("VA", "FL"));


  // Order not relevant
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath0)))
    .WillOnce(Return(key0));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath0a)))
    .WillOnce(Return(key1));
  EXPECT_CALL(*_keyBuilder, buildKey(Ref(*_trx), Ref(*_itin), Ref(*_farePath1)))
    .WillOnce(Return(key2));

  BrandingOptionSpacesDeduplicator::KeyBrandsMap result;
  _deduplicator->calculateDeduplicationMap(*_itin, result);
  auto indexesToRemove = _deduplicator->getBrandOptionIndexesToDeduplicate(*_itin, result);

  ASSERT_EQ(indexesToRemove.size(), 0);
}

} // namespace skipper

} // namespace tse

