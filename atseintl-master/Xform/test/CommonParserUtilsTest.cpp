

#include "Xform/CommonParserUtils.h"

#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/TravelSeg.h"

#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>
namespace tse
{
class CommonParserUtilsTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _rexRequest = _memHandle.create<RexBaseRequest>();
    _excItin = _memHandle.create<ExcItin>();
    _excItin->calculationCurrency() = "EUR";
    _discounts = &_rexRequest->excDiscounts();
    _fc1 = _memHandle.create<FareCompInfo>();
    _fm1 = _memHandle.create<FareMarket>();
    _fc1->fareMarket() = _fm1;
    _fc2 = _memHandle.create<FareCompInfo>();
    _fm2 = _memHandle.create<FareMarket>();
    _fc2->fareMarket() = _fm2;
    _excItin->fareComponent().push_back(_fc1);
    _excItin->fareComponent().push_back(_fc2);
  }

  class Shopping
  {
  public:
    void
    initTvlSegAndPushIntoFareMarket(int16_t pnrSegment, FareMarket& fm, TestMemHandle& memHandle)
    {
      TravelSeg* tvlSeg = memHandle.create<AirSeg>();
      tvlSeg->pnrSegment() = pnrSegment;
      fm.travelSeg().push_back(tvlSeg);
    }

    void callInitFromCommonParserUtils(RexBaseRequest* rexRequest, ExcItin* excItin)
    {
      CommonParserUtils::initShoppingDiscountForSegsWithNoDiscountIfReqHasDiscount(*rexRequest,
                                                                                   excItin);
    }

    int16_t getSegmentValue(const TravelSeg* seg) { return seg->pnrSegment(); }
  };

  class Pricing
  {
  public:
    void
    initTvlSegAndPushIntoFareMarket(int16_t segmentOrder, FareMarket& fm, TestMemHandle& memHandle)
    {
      TravelSeg* tvlSeg = memHandle.create<AirSeg>();
      tvlSeg->segmentOrder() = segmentOrder;
      fm.travelSeg().push_back(tvlSeg);
    }

    void callInitFromCommonParserUtils(RexBaseRequest* rexRequest, ExcItin* excItin)
    {
      CommonParserUtils::initPricingDiscountForSegsWithNoDiscountIfReqHasDiscount(*rexRequest,
                                                                                  excItin);
    }

    int16_t getSegmentValue(const TravelSeg* seg) { return seg->segmentOrder(); }
  };

  template <class T>
  void buildAndCallEmptinessAssertions()
  {
    _excItin = nullptr;
    T().callInitFromCommonParserUtils(_rexRequest, _excItin);

    ASSERT_TRUE(_discounts->getAmounts().empty());
    ASSERT_TRUE(_discounts->getPercentages().empty());
  }

  template <class T>
  void buildAmountFullMarkUp()
  {
    _discounts->addAmount(0, 1, -100, "EUR");
    _discounts->addAmount(0, 4, -300, "EUR");
    EXPECT_TRUE(_rexRequest->excDiscounts().isPAEntry());
    EXPECT_FALSE(_rexRequest->excDiscounts().isPPEntry());

    T().initTvlSegAndPushIntoFareMarket(1, *_fm1, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(2, *_fm1, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(3, *_fm1, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(4, *_fm2, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(5, *_fm2, _memHandle);
  }

  template <class T>
  void buildAmountPartialMarkUp()
  {
    _discounts->addAmount(0, 4, -300, "EUR");
    EXPECT_TRUE(_rexRequest->excDiscounts().isPAEntry());
    EXPECT_FALSE(_rexRequest->excDiscounts().isPPEntry());

    T().initTvlSegAndPushIntoFareMarket(1, *_fm1, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(2, *_fm2, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(3, *_fm2, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(4, *_fm2, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(5, *_fm2, _memHandle);
  }

  template <class T>
  void callAmountAssertions()
  {
    T().callInitFromCommonParserUtils(_rexRequest, _excItin);
    ASSERT_TRUE(_discounts->getPercentages().empty());

    for (const FareCompInfo* fc : _excItin->fareComponent())
    {
      const std::vector<TravelSeg*>* tvlSegVec = &fc->fareMarket()->travelSeg();

      if (std::any_of(tvlSegVec->cbegin(),
                      tvlSegVec->cend(),
                      [&](const TravelSeg* seg)
                      { return _discounts->getAmount(T().getSegmentValue(seg)); }))
      {
        const DiscountAmount* firstSegAmount =
            _discounts->getAmount(T().getSegmentValue(fc->fareMarket()->travelSeg().front()));
        for (const TravelSeg* seg : fc->fareMarket()->travelSeg())
        {
          ASSERT_TRUE(_discounts->getAmount(T().getSegmentValue(seg)) == firstSegAmount);
        }
      }
    }
  }

  template <class T>
  void buildPercentageFullMarkUp()
  {
    _discounts->addPercentage(1, -5);
    _discounts->addPercentage(3, -10);
    EXPECT_TRUE(_rexRequest->excDiscounts().isPPEntry());
    EXPECT_FALSE(_rexRequest->excDiscounts().isPAEntry());

    T().initTvlSegAndPushIntoFareMarket(1, *_fm1, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(2, *_fm1, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(3, *_fm2, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(4, *_fm2, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(5, *_fm2, _memHandle);
  }

  template <class T>
  void buildPercentagePartialMarkUp()
  {
    _discounts->addPercentage(1, -5);
    EXPECT_TRUE(_rexRequest->excDiscounts().isPPEntry());
    EXPECT_FALSE(_rexRequest->excDiscounts().isPAEntry());

    T().initTvlSegAndPushIntoFareMarket(1, *_fm1, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(2, *_fm2, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(3, *_fm2, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(4, *_fm2, _memHandle);
    T().initTvlSegAndPushIntoFareMarket(5, *_fm2, _memHandle);
  }

  template <class T>
  void callPercentageAssertions()
  {
    T().callInitFromCommonParserUtils(_rexRequest, _excItin);

    ASSERT_TRUE(_discounts->getAmounts().empty());

    for (const FareCompInfo* fc : _excItin->fareComponent())
    {
      const std::vector<TravelSeg*>* tvlSegVec = &fc->fareMarket()->travelSeg();

      if (std::any_of(
              tvlSegVec->cbegin(),
              tvlSegVec->cend(),
              [&](const TravelSeg* seg)
              { return _discounts->getPercentage(T().getSegmentValue(seg), false) != nullptr; }))
      {
        for (const TravelSeg* seg : fc->fareMarket()->travelSeg())
        {
          ASSERT_TRUE(_discounts->getPercentage(T().getSegmentValue(seg), false) != nullptr);
        }
      }
    }
  }

protected:
  TestMemHandle _memHandle;
  RexBaseRequest* _rexRequest;
  ExcItin* _excItin;
  Discounts* _discounts;
  FareCompInfo* _fc1, *_fc2;
  FareMarket* _fm1, *_fm2;
};

TEST_F(CommonParserUtilsTest, initShoppingDiscountForSegsEmptySegs)
{
  buildAndCallEmptinessAssertions<Shopping>();
}

TEST_F(CommonParserUtilsTest, initPricingDiscountForSegsEmptySegs)
{
  buildAndCallEmptinessAssertions<Pricing>();
}

TEST_F(CommonParserUtilsTest, initShoppingDiscountForSegsAmountsiFullMarkUp)
{
  buildAmountFullMarkUp<Shopping>();
  callAmountAssertions<Shopping>();
}

TEST_F(CommonParserUtilsTest, initPricingDiscountForSegsAmountsFullMarkUp)
{
  buildAmountFullMarkUp<Pricing>();
  callAmountAssertions<Pricing>();
}

TEST_F(CommonParserUtilsTest, initShoppingDiscountForSegsPercentageFullMarkUp)
{
  buildPercentageFullMarkUp<Shopping>();
  callPercentageAssertions<Shopping>();
}
TEST_F(CommonParserUtilsTest, initPricingDiscountForSegsPercentageFullMarkUp)
{
  buildPercentageFullMarkUp<Pricing>();
  callPercentageAssertions<Pricing>();
}

TEST_F(CommonParserUtilsTest, initShoppingDiscountForSegsAmountsPartialMarkUp)
{
  buildAmountPartialMarkUp<Shopping>();
  callAmountAssertions<Shopping>();
}

TEST_F(CommonParserUtilsTest, initPricingDiscountForSegsAmountsPartialMarkUp)
{
  buildAmountPartialMarkUp<Pricing>();
  callAmountAssertions<Pricing>();
}

TEST_F(CommonParserUtilsTest, initShoppingDiscountForSegsPercentagePartialMarkUp)
{
  buildPercentagePartialMarkUp<Shopping>();
  callPercentageAssertions<Shopping>();
}

TEST_F(CommonParserUtilsTest, initPricingDiscountForSegsPercentagePartialMarkUp)
{
  buildPercentagePartialMarkUp<Pricing>();
  callPercentageAssertions<Pricing>();
}
} // tse
