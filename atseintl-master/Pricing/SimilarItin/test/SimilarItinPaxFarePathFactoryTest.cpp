//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/PaxFarePathFactory.h"
#include "Pricing/SimilarItin/Context.h"
#include "Pricing/SimilarItin/FarePathValidator.h"
#include "Pricing/SimilarItin/SimilarItinPaxFarePathFactory.h"
#include "Pricing/test/FactoriesConfigStub.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <vector>

namespace tse
{
namespace
{
using ItemList = std::list<FPPQItem*>;
using ValidPair = std::pair<bool, ItemList>;

class MockFarePathValidator
{
public:
  bool
  isFarePathValid(FPPQItem&,
                  FarePathSettings&,
                  const SavedFPPQItem::Stage stage,
                  DiagCollector&,
                  ItemList& clonedFPPQItems)
  {
    auto p = isFarePathValid();
    clonedFPPQItems = p.second;
    return p.first;
  }
  void setConfig(PaxFarePathFactoryBase*) {}

  MOCK_METHOD0(isFarePathValid, ValidPair());
  MOCK_METHOD1(setLowerBoundFPPQItem, void(FPPQItem* lowerBoundFPPQItem));
  MOCK_CONST_METHOD1(pricedThroughFare, bool(const FarePath&));
  MOCK_METHOD2(processFarePathClones, void(FPPQItem*& item, ItemList& clonedFPPQItems));
};

class MockFarePathBuilder
{
public:
  MOCK_METHOD2(cloneFarePaths, std::vector<FarePath*>(const GroupFarePath&, Itin&));
  MOCK_METHOD2(cloneFPPQItem, FPPQItem*(const FPPQItem& source, Itin& est));
};
}

using ::testing::_;
using ::testing::ByRef;
using ::testing::Return;
using ::testing::Eq;
namespace similaritin
{
using SIPaxFarePathFactory = similaritin::SimilarItinPaxFarePathFactory<NoDiagnostic,
                                                                        MockFarePathBuilder,
                                                                        MockFarePathValidator>;

class SimilarItinPaxFarePathFactoryTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _factoriesConfig = _memHandle.create<test::FactoriesConfigStub>();
    _trx = _memHandle.create<PricingTrx>();
    PricingRequest* req = _memHandle.create<PricingRequest>();
    _trx->setRequest(req);
    _diag = _memHandle.create<DiagCollector>();
    _builder = _memHandle.create<MockFarePathBuilder>();
    _est = _memHandle.create<Itin>();
    _pfpfOrig = _memHandle.create<PaxFarePathFactory>(*_factoriesConfig);
    _validator = _memHandle.create<MockFarePathValidator>();
    _pfpf =
        _memHandle.create<SIPaxFarePathFactory>(_pfpfOrig, *_validator, *_builder, _noDiag, *_est, *_trx);
    PricingUnitFactory* puf = _memHandle.create<PricingUnitFactory>();
    _allPUF.push_back(puf);
  }

  void TearDown() { _memHandle.clear(); }

protected:
  FPPQItem* createFPPQItem(const uint16_t xPoint, const MoneyAmount& amount)
  {
    FPPQItem* result = _memHandle.create<FPPQItem>();
    result->xPoint() = xPoint;
    Itin* itin = _memHandle.create<Itin>();
    FarePath* fp = _memHandle.create<FarePath>();
    fp->itin() = itin;
    fp->setTotalNUCAmount(amount);
    result->farePath() = fp;
    return result;
  }

  void verifyFPPQItem(const uint32_t index, DiagCollector& diag, const uint16_t xPoint)
  {
    FPPQItem* item = _pfpf->getFPPQItem(index, diag);
    ASSERT_TRUE(item);
    ASSERT_EQ(xPoint, item->xPoint());
  }

  TestMemHandle _memHandle;
  test::FactoriesConfigStub* _factoriesConfig;

  MockFarePathBuilder* _builder;
  PaxFarePathFactory* _pfpfOrig;
  PricingTrx* _trx;
  DiagCollector* _diag;
  Itin* _est;
  NoDiagnostic _noDiag;
  std::vector<PricingUnitFactory*> _allPUF;
  SIPaxFarePathFactory* _pfpf;
  MockFarePathValidator* _validator;
};

using SIPFPFTest = SimilarItinPaxFarePathFactoryTest;

TEST_F(SIPFPFTest, testEmptySavedFPPQItems)
{
  std::vector<SavedFPPQItem> items;
  EXPECT_CALL(*_validator, setLowerBoundFPPQItem(_)).Times(0);
  _pfpf->initQueue(std::move(items));
  EXPECT_CALL(*_validator, isFarePathValid()).Times(0);
  EXPECT_CALL(*_validator, processFarePathClones(_, _)).Times(0);
  ASSERT_TRUE(_pfpf->getFPPQItem(0, *_diag) == nullptr);
}

TEST_F(SIPFPFTest, testGetFPPQItem)
{
  std::vector<SavedFPPQItem> items;
  FarePathSettings settings;
  items.emplace_back(0, 4, createFPPQItem(1, 100), settings, SavedFPPQItem::Stage::FP_LEVEL);
  items.emplace_back(1, 4, createFPPQItem(2, 200), settings, SavedFPPQItem::Stage::FP_LEVEL);
  items.emplace_back(2, 4, createFPPQItem(3, 300), settings, SavedFPPQItem::Stage::FP_LEVEL);
  items.emplace_back(3, 4, createFPPQItem(4, 400), settings, SavedFPPQItem::Stage::FP_LEVEL);
  std::vector<SavedFPPQItem> clonedItems(items);

  EXPECT_CALL(*_validator, setLowerBoundFPPQItem(_));
  _pfpf->initQueue(std::move(items));
  EXPECT_CALL(*_validator, isFarePathValid())
      .WillOnce(Return(ValidPair{true, {}}))
      .WillOnce(Return(ValidPair{true, {}}))
      .WillOnce(Return(ValidPair{false, {}}))
      .WillOnce(Return(ValidPair{true, {}}));
  EXPECT_CALL(*_validator, processFarePathClones(_, _)).Times(0);
  EXPECT_CALL(*_validator, pricedThroughFare(_)).WillRepeatedly(Return(false));
  EXPECT_CALL(*_builder, cloneFPPQItem(_, _))
      .WillOnce(Return(clonedItems[0]._fppqItem))
      .WillOnce(Return(clonedItems[1]._fppqItem))
      .WillOnce(Return(clonedItems[2]._fppqItem))
      .WillOnce(Return(clonedItems[3]._fppqItem));

  verifyFPPQItem(0, *_diag, 1);
  verifyFPPQItem(1, *_diag, 2);
  verifyFPPQItem(2, *_diag, 4);
}

TEST_F(SIPFPFTest, testGetFPPQItem_GSA)
{
  std::vector<SavedFPPQItem> items;
  FarePathSettings settings;
  FPPQItem* item = createFPPQItem(1, 100);
  item->farePath()->validatingCarriers() = {"XX", "YY"};
  FPPQItem* itemYY = createFPPQItem(1, 100);
  item->farePath()->validatingCarriers() = {"YY"};
  items.emplace_back(0, 4, item, settings, SavedFPPQItem::Stage::FP_LEVEL);
  std::vector<SavedFPPQItem> clonedItems(items);

  EXPECT_CALL(*_validator, setLowerBoundFPPQItem(_));
  _pfpf->initQueue(std::move(items));
  EXPECT_CALL(*_validator, isFarePathValid())
      .WillOnce(Return(ValidPair{true, {itemYY}}));
  EXPECT_CALL(*_validator, processFarePathClones(Eq(item), Eq(ItemList{itemYY}))).Times(1);
  EXPECT_CALL(*_validator, pricedThroughFare(_)).WillRepeatedly(Return(false));
  EXPECT_CALL(*_builder, cloneFPPQItem(_, _))
      .WillOnce(Return(clonedItems[0]._fppqItem));

  verifyFPPQItem(0, *_diag, 1);
}
TEST_F(SIPFPFTest, testCheckSpecifiedValidatingCarrier)
{
  CarrierCode originalCxr = CarrierCode("AA");
  CarrierCode specifiedCxr = CarrierCode("EY");
  _est->validatingCarrier() = originalCxr;
  _trx->getRequest()->validatingCarrier() = specifiedCxr;
  SIPaxFarePathFactory siPFPFactory(_pfpfOrig, *_validator, *_builder, _noDiag, *_est, *_trx);
  ASSERT_TRUE(_est->validatingCarrier() == specifiedCxr);
}
}
}
