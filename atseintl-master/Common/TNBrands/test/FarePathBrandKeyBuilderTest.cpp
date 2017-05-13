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

#include "DataModel/ArunkSeg.h"
#include "DataModel/FareUsage.h"
#include "Common/TNBrands/BrandingOptionSpacesDeduplicator.h"

#include <memory>

using namespace ::testing;

namespace tse
{

namespace skipper
{

class MockFareUsageBrandFinder
{
public:
  using FarePathBrandKey = std::vector<GoverningCarrierBrand>;

  MOCK_CONST_METHOD4(findBrand, GoverningCarrierBrand(const PricingTrx&,
                                                      const Itin&,
                                                      const FarePath&,
                                                      const FareUsage&));
};

class FarePathBrandKeyBuilderTest: public Test
{
public:
  using TestedFarePathBrandKeyBuilder = FarePathBrandKeyBuilder<MockFareUsageBrandFinder>;
  using TestedFarePathBrandKeyWithPaxTypeBuilder =
          FarePathBrandKeyWithPaxTypeBuilder<MockFareUsageBrandFinder>;

  void SetUp()
  {
    _brandFinder = new MockFareUsageBrandFinder();
    _brandFinderWithPax = new MockFareUsageBrandFinder();
    _builder.reset(new TestedFarePathBrandKeyBuilder(_brandFinder));
    _builderWithPaxType.reset(new TestedFarePathBrandKeyWithPaxTypeBuilder(_brandFinderWithPax));

    _itin.reset(new Itin);
    _farePath.reset(new FarePath);
    _pu1.reset(new PricingUnit);
    _pu2.reset(new PricingUnit);
    _fu1.reset(new FareUsage);
    _fu2.reset(new FareUsage);
    _fu3.reset(new FareUsage);
    _fu4.reset(new FareUsage);
    _seg1.reset(new AirSeg);
    _seg2.reset(new AirSeg);
    _seg3.reset(new AirSeg);
    _seg4.reset(new AirSeg);
    _arunk.reset(new ArunkSeg);
    _trx.reset(new PricingTrx);
    _paxType1.reset(new PaxType);
    _farePath->paxType() = _paxType1.get();
  }

  void TearDown(){}

  MockFareUsageBrandFinder* _brandFinder;
  MockFareUsageBrandFinder* _brandFinderWithPax;
  std::shared_ptr<TestedFarePathBrandKeyBuilder> _builder;
  std::shared_ptr<TestedFarePathBrandKeyWithPaxTypeBuilder> _builderWithPaxType;

  std::shared_ptr<Itin> _itin;
  std::shared_ptr<FarePath> _farePath;
  std::shared_ptr<PaxType> _paxType1;
  std::shared_ptr<PricingUnit> _pu1;
  std::shared_ptr<PricingUnit> _pu2;
  std::shared_ptr<FareUsage> _fu1;
  std::shared_ptr<FareUsage> _fu2;
  std::shared_ptr<FareUsage> _fu3;
  std::shared_ptr<FareUsage> _fu4;
  std::shared_ptr<AirSeg> _seg1;
  std::shared_ptr<AirSeg> _seg2;
  std::shared_ptr<AirSeg> _seg3;
  std::shared_ptr<AirSeg> _seg4;
  std::shared_ptr<ArunkSeg> _arunk;
  std::shared_ptr<PricingTrx> _trx;
};


TEST_F(FarePathBrandKeyBuilderTest, testOneSegment)
{
  _itin->travelSeg().push_back(_seg1.get());

  _farePath->pricingUnit().push_back(_pu1.get());
  _pu1->fareUsage().push_back(_fu1.get());
  _fu1->travelSeg().push_back(_seg1.get());

  GoverningCarrierBrand gcb("VA", "FL");

  EXPECT_CALL(*_brandFinder, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu1)))
    .WillOnce(Return(gcb));

  TestedFarePathBrandKeyBuilder::FarePathBrandKey key =
    _builder->buildKey(*_trx, *_itin, *_farePath);

  ASSERT_EQ(1, key.size());
  ASSERT_EQ(gcb, key[0]);
}

TEST_F(FarePathBrandKeyBuilderTest, testTwoSegmentsOneFareComponent)
{
  _itin->travelSeg().push_back(_seg1.get());
  _itin->travelSeg().push_back(_seg2.get());

  _farePath->pricingUnit().push_back(_pu1.get());
  _pu1->fareUsage().push_back(_fu1.get());
  _fu1->travelSeg().push_back(_seg1.get());
  _fu1->travelSeg().push_back(_seg2.get());

  GoverningCarrierBrand gcb("VA", "FL");

  EXPECT_CALL(*_brandFinder, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu1)))
    .WillOnce(Return(gcb));

  TestedFarePathBrandKeyBuilder::FarePathBrandKey key =
    _builder->buildKey(*_trx, *_itin, *_farePath);

  ASSERT_EQ(2, key.size());
  ASSERT_EQ(gcb, key[0]);
  ASSERT_EQ(gcb, key[1]);
}

TEST_F(FarePathBrandKeyBuilderTest, testOnePricingUnitTwoFareComponents)
{
  _itin->travelSeg().push_back(_seg1.get());
  _itin->travelSeg().push_back(_seg2.get());

  _farePath->pricingUnit().push_back(_pu1.get());

  _pu1->fareUsage().push_back(_fu1.get());
  _fu1->travelSeg().push_back(_seg1.get());

  _pu1->fareUsage().push_back(_fu2.get());
  _fu2->travelSeg().push_back(_seg2.get());

  GoverningCarrierBrand gcb1("VA", "FL");
  GoverningCarrierBrand gcb2("EY", "EF");

  EXPECT_CALL(*_brandFinder, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu1)))
    .WillOnce(Return(gcb1));
  EXPECT_CALL(*_brandFinder, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu2)))
    .WillOnce(Return(gcb2));

  TestedFarePathBrandKeyBuilder::FarePathBrandKey key =
    _builder->buildKey(*_trx, *_itin, *_farePath);

  ASSERT_EQ(2, key.size());
  ASSERT_EQ(gcb1, key[0]);
  ASSERT_EQ(gcb2, key[1]);
}

TEST_F(FarePathBrandKeyBuilderTest, testTwoPricingUnits)
{
  _itin->travelSeg().push_back(_seg1.get());
  _itin->travelSeg().push_back(_seg2.get());
  _itin->travelSeg().push_back(_seg3.get());

  _farePath->pricingUnit().push_back(_pu1.get());
  _pu1->fareUsage().push_back(_fu1.get());
  _fu1->travelSeg().push_back(_seg1.get());

  _farePath->pricingUnit().push_back(_pu2.get());
  _pu2->fareUsage().push_back(_fu2.get());
  _fu2->travelSeg().push_back(_seg2.get());
  _fu2->travelSeg().push_back(_seg3.get());

  GoverningCarrierBrand gcb1("VA", "FL");
  GoverningCarrierBrand gcb2("EY", "EF");

  EXPECT_CALL(*_brandFinder, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu1)))
    .WillOnce(Return(gcb1));
  EXPECT_CALL(*_brandFinder, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu2)))
    .WillOnce(Return(gcb2));

  TestedFarePathBrandKeyBuilder::FarePathBrandKey key =
    _builder->buildKey(*_trx, *_itin, *_farePath);

  ASSERT_EQ(3, key.size());
  ASSERT_EQ(gcb1, key[0]);
  ASSERT_EQ(gcb2, key[1]);
  ASSERT_EQ(gcb2, key[2]);
}

TEST_F(FarePathBrandKeyBuilderTest, testArunkAddedInFareUsageDuringPO)
{
  _itin->travelSeg().push_back(_seg1.get());
  _itin->travelSeg().push_back(_seg2.get());
  // No arunk

  _farePath->pricingUnit().push_back(_pu1.get());
  _pu1->fareUsage().push_back(_fu1.get());
  _fu1->travelSeg().push_back(_seg1.get());
  _fu1->travelSeg().push_back(_seg2.get());
  _fu1->travelSeg().push_back(_arunk.get());

  GoverningCarrierBrand gcb("VA", "FL");

  EXPECT_CALL(*_brandFinder, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu1)))
    .WillOnce(Return(gcb));

  TestedFarePathBrandKeyBuilder::FarePathBrandKey key = _builder->buildKey(*_trx, *_itin, *_farePath);

  ASSERT_EQ(2, key.size());
  ASSERT_EQ(gcb, key[0]);
  ASSERT_EQ(gcb, key[1]);
}

TEST_F(FarePathBrandKeyBuilderTest, testArunkFromItinerary)
{
  _itin->travelSeg().push_back(_seg1.get());
  _itin->travelSeg().push_back(_arunk.get());
  _itin->travelSeg().push_back(_seg2.get());

  _farePath->pricingUnit().push_back(_pu1.get());

  _pu1->fareUsage().push_back(_fu1.get());
  _fu1->travelSeg().push_back(_seg1.get());
  _fu1->travelSeg().push_back(_arunk.get());

  _pu1->fareUsage().push_back(_fu2.get());
  _fu2->travelSeg().push_back(_seg2.get());

  GoverningCarrierBrand gcb("VA", "FL");
  GoverningCarrierBrand gcbArunk;

  EXPECT_CALL(*_brandFinder, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu1)))
    .WillOnce(Return(gcb));
  EXPECT_CALL(*_brandFinder, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu2)))
    .WillOnce(Return(gcb));

  TestedFarePathBrandKeyBuilder::FarePathBrandKey key =
    _builder->buildKey(*_trx, *_itin, *_farePath);

  ASSERT_EQ(3, key.size());
  ASSERT_EQ(gcb, key[0]);
  ASSERT_EQ(gcbArunk, key[1]);
  ASSERT_EQ(gcb, key[2]);
}

TEST_F(FarePathBrandKeyBuilderTest, testSnowmanPUPath)
{
  _itin->travelSeg().push_back(_seg1.get());
  _itin->travelSeg().push_back(_seg2.get());
  _itin->travelSeg().push_back(_seg3.get());
  _itin->travelSeg().push_back(_seg4.get());

  _farePath->pricingUnit().push_back(_pu1.get());

  _pu1->fareUsage().push_back(_fu1.get());
  _fu1->travelSeg().push_back(_seg1.get());

  _pu1->fareUsage().push_back(_fu4.get());
  _fu4->travelSeg().push_back(_seg4.get());

  _farePath->pricingUnit().push_back(_pu2.get());

  _pu2->fareUsage().push_back(_fu2.get());
  _fu2->travelSeg().push_back(_seg2.get());

  _pu2->fareUsage().push_back(_fu3.get());
  _fu3->travelSeg().push_back(_seg3.get());

  GoverningCarrierBrand gcb1("VA", "FL");
  GoverningCarrierBrand gcb2("EY", "EF");

  EXPECT_CALL(*_brandFinder, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu1)))
    .WillOnce(Return(gcb1));
  EXPECT_CALL(*_brandFinder, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu2)))
    .WillOnce(Return(gcb2));
  EXPECT_CALL(*_brandFinder, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu3)))
    .WillOnce(Return(gcb2));
  EXPECT_CALL(*_brandFinder, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu4)))
    .WillOnce(Return(gcb1));

  TestedFarePathBrandKeyBuilder::FarePathBrandKey key =
    _builder->buildKey(*_trx, *_itin, *_farePath);

  ASSERT_EQ(4, key.size());
  ASSERT_EQ(gcb1, key[0]);
  ASSERT_EQ(gcb2, key[1]);
  ASSERT_EQ(gcb2, key[2]);
  ASSERT_EQ(gcb1, key[3]);
}

TEST_F(FarePathBrandKeyBuilderTest, testOneSegmentWithPax)
{
  _itin->travelSeg().push_back(_seg1.get());

  _farePath->pricingUnit().push_back(_pu1.get());
  _pu1->fareUsage().push_back(_fu1.get());
  _fu1->travelSeg().push_back(_seg1.get());

  GoverningCarrierBrand gcb("VA", "FL");

  EXPECT_CALL(*_brandFinderWithPax, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu1)))
    .WillOnce(Return(gcb));

  TestedFarePathBrandKeyWithPaxTypeBuilder::FarePathBrandKey key =
    _builderWithPaxType->buildKey(*_trx, *_itin, *_farePath);

  ASSERT_EQ(1, key.second.size());
  ASSERT_EQ(gcb, key.second[0]);
  ASSERT_EQ(_paxType1.get(), key.first);
}

TEST_F(FarePathBrandKeyBuilderTest, testTwoSegmentsOneFareComponentWithPax)
{
  _itin->travelSeg().push_back(_seg1.get());
  _itin->travelSeg().push_back(_seg2.get());

  _farePath->pricingUnit().push_back(_pu1.get());
  _pu1->fareUsage().push_back(_fu1.get());
  _fu1->travelSeg().push_back(_seg1.get());
  _fu1->travelSeg().push_back(_seg2.get());

  GoverningCarrierBrand gcb("VA", "FL");

  EXPECT_CALL(*_brandFinderWithPax, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu1)))
    .WillOnce(Return(gcb));

  TestedFarePathBrandKeyWithPaxTypeBuilder::FarePathBrandKey key =
    _builderWithPaxType->buildKey(*_trx, *_itin, *_farePath);

  ASSERT_EQ(2, key.second.size());
  ASSERT_EQ(gcb, key.second[0]);
  ASSERT_EQ(gcb, key.second[1]);
  ASSERT_EQ(_paxType1.get(), key.first);
}

TEST_F(FarePathBrandKeyBuilderTest, testOnePricingUnitTwoFareComponentsWithPax)
{
  _itin->travelSeg().push_back(_seg1.get());
  _itin->travelSeg().push_back(_seg2.get());

  _farePath->pricingUnit().push_back(_pu1.get());

  _pu1->fareUsage().push_back(_fu1.get());
  _fu1->travelSeg().push_back(_seg1.get());

  _pu1->fareUsage().push_back(_fu2.get());
  _fu2->travelSeg().push_back(_seg2.get());

  GoverningCarrierBrand gcb1("VA", "FL");
  GoverningCarrierBrand gcb2("EY", "EF");

  EXPECT_CALL(*_brandFinderWithPax, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu1)))
    .WillOnce(Return(gcb1));
  EXPECT_CALL(*_brandFinderWithPax, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu2)))
    .WillOnce(Return(gcb2));

  TestedFarePathBrandKeyWithPaxTypeBuilder::FarePathBrandKey key =
    _builderWithPaxType->buildKey(*_trx, *_itin, *_farePath);

  ASSERT_EQ(2, key.second.size());
  ASSERT_EQ(gcb1, key.second[0]);
  ASSERT_EQ(gcb2, key.second[1]);
  ASSERT_EQ(_paxType1.get(), key.first);
}

TEST_F(FarePathBrandKeyBuilderTest, testTwoPricingUnitsWithPax)
{
  _itin->travelSeg().push_back(_seg1.get());
  _itin->travelSeg().push_back(_seg2.get());
  _itin->travelSeg().push_back(_seg3.get());

  _farePath->pricingUnit().push_back(_pu1.get());
  _pu1->fareUsage().push_back(_fu1.get());
  _fu1->travelSeg().push_back(_seg1.get());

  _farePath->pricingUnit().push_back(_pu2.get());
  _pu2->fareUsage().push_back(_fu2.get());
  _fu2->travelSeg().push_back(_seg2.get());
  _fu2->travelSeg().push_back(_seg3.get());

  GoverningCarrierBrand gcb1("VA", "FL");
  GoverningCarrierBrand gcb2("EY", "EF");

  EXPECT_CALL(*_brandFinderWithPax, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu1)))
    .WillOnce(Return(gcb1));
  EXPECT_CALL(*_brandFinderWithPax, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu2)))
    .WillOnce(Return(gcb2));

  TestedFarePathBrandKeyWithPaxTypeBuilder::FarePathBrandKey key =
    _builderWithPaxType->buildKey(*_trx, *_itin, *_farePath);

  ASSERT_EQ(3, key.second.size());
  ASSERT_EQ(gcb1, key.second[0]);
  ASSERT_EQ(gcb2, key.second[1]);
  ASSERT_EQ(gcb2, key.second[2]);
  ASSERT_EQ(_paxType1.get(), key.first);
}

TEST_F(FarePathBrandKeyBuilderTest, testArunkAddedInFareUsageDuringPOWithPax)
{
  _itin->travelSeg().push_back(_seg1.get());
  _itin->travelSeg().push_back(_seg2.get());
  // No arunk

  _farePath->pricingUnit().push_back(_pu1.get());
  _pu1->fareUsage().push_back(_fu1.get());
  _fu1->travelSeg().push_back(_seg1.get());
  _fu1->travelSeg().push_back(_seg2.get());
  _fu1->travelSeg().push_back(_arunk.get());

  GoverningCarrierBrand gcb("VA", "FL");

  EXPECT_CALL(*_brandFinderWithPax, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu1)))
    .WillOnce(Return(gcb));

  TestedFarePathBrandKeyWithPaxTypeBuilder::FarePathBrandKey key =
    _builderWithPaxType->buildKey(*_trx, *_itin, *_farePath);

  ASSERT_EQ(2, key.second.size());
  ASSERT_EQ(gcb, key.second[0]);
  ASSERT_EQ(gcb, key.second[1]);
  ASSERT_EQ(_paxType1.get(), key.first);
}

TEST_F(FarePathBrandKeyBuilderTest, testArunkFromItineraryWithPax)
{
  _itin->travelSeg().push_back(_seg1.get());
  _itin->travelSeg().push_back(_arunk.get());
  _itin->travelSeg().push_back(_seg2.get());

  _farePath->pricingUnit().push_back(_pu1.get());

  _pu1->fareUsage().push_back(_fu1.get());
  _fu1->travelSeg().push_back(_seg1.get());
  _fu1->travelSeg().push_back(_arunk.get());

  _pu1->fareUsage().push_back(_fu2.get());
  _fu2->travelSeg().push_back(_seg2.get());

  GoverningCarrierBrand gcb("VA", "FL");
  GoverningCarrierBrand gcbArunk;

  EXPECT_CALL(*_brandFinderWithPax, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu1)))
    .WillOnce(Return(gcb));
  EXPECT_CALL(*_brandFinderWithPax, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu2)))
    .WillOnce(Return(gcb));

  TestedFarePathBrandKeyWithPaxTypeBuilder::FarePathBrandKey key =
    _builderWithPaxType->buildKey(*_trx, *_itin, *_farePath);

  ASSERT_EQ(3, key.second.size());
  ASSERT_EQ(gcb, key.second[0]);
  ASSERT_EQ(gcbArunk, key.second[1]);
  ASSERT_EQ(gcb, key.second[2]);
  ASSERT_EQ(_paxType1.get(), key.first);
}

TEST_F(FarePathBrandKeyBuilderTest, testSnowmanPUPathWithPax)
{
  _itin->travelSeg().push_back(_seg1.get());
  _itin->travelSeg().push_back(_seg2.get());
  _itin->travelSeg().push_back(_seg3.get());
  _itin->travelSeg().push_back(_seg4.get());

  _farePath->pricingUnit().push_back(_pu1.get());

  _pu1->fareUsage().push_back(_fu1.get());
  _fu1->travelSeg().push_back(_seg1.get());

  _pu1->fareUsage().push_back(_fu4.get());
  _fu4->travelSeg().push_back(_seg4.get());

  _farePath->pricingUnit().push_back(_pu2.get());

  _pu2->fareUsage().push_back(_fu2.get());
  _fu2->travelSeg().push_back(_seg2.get());

  _pu2->fareUsage().push_back(_fu3.get());
  _fu3->travelSeg().push_back(_seg3.get());

  GoverningCarrierBrand gcb1("VA", "FL");
  GoverningCarrierBrand gcb2("EY", "EF");

  EXPECT_CALL(*_brandFinderWithPax, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu1)))
    .WillOnce(Return(gcb1));
  EXPECT_CALL(*_brandFinderWithPax, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu2)))
    .WillOnce(Return(gcb2));
  EXPECT_CALL(*_brandFinderWithPax, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu3)))
    .WillOnce(Return(gcb2));
  EXPECT_CALL(*_brandFinderWithPax, findBrand(Ref(*_trx), Ref(*_itin), Ref(*_farePath), Ref(*_fu4)))
    .WillOnce(Return(gcb1));

  TestedFarePathBrandKeyWithPaxTypeBuilder::FarePathBrandKey key =
    _builderWithPaxType->buildKey(*_trx, *_itin, *_farePath);

  ASSERT_EQ(4, key.second.size());
  ASSERT_EQ(gcb1, key.second[0]);
  ASSERT_EQ(gcb2, key.second[1]);
  ASSERT_EQ(gcb2, key.second[2]);
  ASSERT_EQ(gcb1, key.second[3]);
  ASSERT_EQ(_paxType1.get(), key.first);
}

} // namespace skipper

} // namespace tse

