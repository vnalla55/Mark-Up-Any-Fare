#include "Pricing/FarePathFactory.h"

#include "Common/ClassOfService.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/FactoriesConfig.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <vector>

namespace tse
{
using testing::_;
using testing::Return;

namespace
{
class FareByRuleRevalidatorMock
{
public:
  MOCK_CONST_METHOD4(checkFBR,
                     bool(const PaxTypeFare*,
                          const uint16_t,
                          const std::vector<std::vector<ClassOfService*>*>&,
                          const std::vector<TravelSeg*>&));
};

class FarePathFactoryProxy : public FarePathFactory
{
public:
  FarePathFactoryProxy(const FactoriesConfig& config) : FarePathFactory(config) {}

  using FarePathFactory::checkFBR;
};
}

class FarePathFactory_checkFBRTest : public testing::Test
{
public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _factoriesConfig = _memHandle.create<FactoriesConfig>(Global::config());
    _factory = _memHandle.create<FarePathFactoryProxy>(*_factoriesConfig);
    _revalidator = _memHandle.create<FareByRuleRevalidatorMock>();
  }

  void TearDown()
  {
    _memHandle.clear();
  }

protected:
  TestMemHandle _memHandle;

  FactoriesConfig* _factoriesConfig;
  FarePathFactoryProxy* _factory;
  FareByRuleRevalidatorMock* _revalidator;

  PaxTypeFare* createFBR()
  {
    PaxTypeFare* result = _memHandle.create<PaxTypeFare>();
    result->status().set(PaxTypeFare::PTF_FareByRule);
    result->fareMarket() = _memHandle.create<FareMarket>();
    return result;
  }

  TravelSeg* createTS()
  {
    TravelSeg* result = _memHandle.create<AirSeg>();
    result->classOfService().resize(2, nullptr);
    return result;
  }

  void fillFMWithTS(const uint16_t count, FareMarket& fm)
  {
    for (uint16_t i = 0; i < count; ++i)
      fm.travelSeg().push_back(createTS());
  }

  void fillFMWithTSansCOS(const uint16_t count, FareMarket& fm)
  {
    for (uint16_t i = 0; i < count; ++i)
      fm.classOfServiceVec().push_back(_memHandle.create<std::vector<ClassOfService*>>(1, nullptr));
  }

  FPPQItem* create2MarketsWithCOS()
  {
    FPPQItem* result = _memHandle.create<FPPQItem>();
    result->farePath() = _memHandle.create<FarePath>();
    FarePath& fp = *result->farePath();
    fp.pricingUnit().push_back(_memHandle.create<PricingUnit>());

    FareUsage* fu1 = _memHandle.create<FareUsage>();
    fu1->paxTypeFare() = createFBR();
    FareMarket& fm1 = *fu1->paxTypeFare()->fareMarket();
    fillFMWithTS(2, fm1);
    fillFMWithTSansCOS(2, fm1);
    fp.pricingUnit().front()->fareUsage().push_back(fu1);

    FareUsage* fu2 = _memHandle.create<FareUsage>();
    fu2->paxTypeFare() = createFBR();
    FareMarket& fm2 = *fu2->paxTypeFare()->fareMarket();
    fillFMWithTS(1, fm2);
    fillFMWithTSansCOS(1, fm2);
    fp.pricingUnit().front()->fareUsage().push_back(fu2);

    return result;
  }
};

TEST_F(FarePathFactory_checkFBRTest, checkFMAvailabilityPass)
{
  FPPQItem* fppqItem = create2MarketsWithCOS();

  PaxTypeFare* ptf = fppqItem->farePath()->pricingUnit().front()->fareUsage().front()->paxTypeFare();
  FareMarket* fm = ptf->fareMarket();
  EXPECT_CALL(*_revalidator, checkFBR(ptf, 1, fm->classOfServiceVec(), fm->travelSeg()))
      .WillOnce(Return(true));

  ptf = fppqItem->farePath()->pricingUnit().front()->fareUsage().back()->paxTypeFare();
  fm = ptf->fareMarket();
  EXPECT_CALL(*_revalidator, checkFBR(ptf, 1, fm->classOfServiceVec(), fm->travelSeg()))
      .WillOnce(Return(true));

  ASSERT_TRUE(_factory->checkFBR(*fppqItem, 1, *_revalidator));
}

TEST_F(FarePathFactory_checkFBRTest, checkLocalAvailabilityFail)
{
  FPPQItem* fppqItem = create2MarketsWithCOS();

  PaxTypeFare* ptf = fppqItem->farePath()->pricingUnit().front()->fareUsage().front()->paxTypeFare();
  FareMarket* fm = ptf->fareMarket();
  EXPECT_CALL(*_revalidator, checkFBR(ptf, 1, fm->classOfServiceVec(), fm->travelSeg()))
      .WillOnce(Return(true));

  ptf = fppqItem->farePath()->pricingUnit().front()->fareUsage().back()->paxTypeFare();
  fm = ptf->fareMarket();
  fm->classOfServiceVec().clear();
  std::vector<std::vector<ClassOfService*>*> expected;
  for (TravelSeg* segment : fm->travelSeg())
    expected.push_back(&segment->classOfService());

  EXPECT_CALL(*_revalidator, checkFBR(ptf, 1, expected, fm->travelSeg())).WillOnce(Return(false));

  ASSERT_FALSE(_factory->checkFBR(*fppqItem, 1, *_revalidator));
}
}
