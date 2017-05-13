// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestConfigInitializer.h"

#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "ServiceFees/AncillaryPriceModifierProcessor.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "ServiceFees/ServiceFeesGroup.h"

namespace tse
{

class OcAmountRounderMock : public OCFees::OcAmountRounder
{
  public:
    OcAmountRounderMock(PricingTrx& trx) : OCFees::OcAmountRounder(trx) {}

  public:
    MOCK_CONST_METHOD1(getRoundedFeeAmount, MoneyAmount(const Money&));
};

MoneyAmount fakeGetRoundedFeeAmountMethod(const Money &amount)
{
  return round(amount.value());
}

class AncillaryPriceModifierProcessorTest : public ::testing::Test
{
public:
  void SetUp() override
  {
    _memHandle.insert(new TestConfigInitializer);
    TestConfigInitializer::setValue("MAX_PSS_OUTPUT", 0, "OUTPUT_LIMITS");
    _farePath = _memHandle.insert(new FarePath);
    _trx = _memHandle.insert(new AncillaryPricingTrx);
    _itin = _memHandle.insert(new Itin);
    _serviceFeesGroups = createServiceGroupsWithOneTravelSeg();
  }

  void TearDown() override
  {
    _memHandle.clear();
  }

  std::vector<ServiceFeesGroup*>* createServiceGroupsWithOneTravelSeg();
  ServiceFeesGroup* createServiceGroupWithOneTravelSeg();
  OCFees* createOCFeesOneTravelSeg();
  void makeTrxSecondCallForMonetaryPricing();

protected:
  TestMemHandle _memHandle;
  AncillaryPricingTrx* _trx;
  Itin* _itin;
  FarePath* _farePath;
  std::vector<ServiceFeesGroup*>* _serviceFeesGroups;
};

std::vector<ServiceFeesGroup*>* AncillaryPriceModifierProcessorTest::createServiceGroupsWithOneTravelSeg()
{
  std::vector<ServiceFeesGroup*>* serviceFeesGroups = _memHandle.insert(new std::vector<ServiceFeesGroup*>);
  serviceFeesGroups->push_back(createServiceGroupWithOneTravelSeg());
  return serviceFeesGroups;
}

ServiceFeesGroup* AncillaryPriceModifierProcessorTest::createServiceGroupWithOneTravelSeg()
{
  ServiceFeesGroup* serviceFeesGroup = _memHandle.insert(new ServiceFeesGroup);
  serviceFeesGroup->ocFeesMap()[_farePath].push_back(createOCFeesOneTravelSeg());
  return serviceFeesGroup;
}

OCFees* AncillaryPriceModifierProcessorTest::createOCFeesOneTravelSeg()
{
  OCFees* ocFees = _memHandle.insert(new OCFees);

  ocFees->carrierCode() = "LH";
  ocFees->feeNoDec() = 0;
  SubCodeInfo* subCodeInfo = _memHandle.insert(new SubCodeInfo);
  subCodeInfo->commercialName() = "BAGGAGE";
  subCodeInfo->serviceGroup() = "BG";
  subCodeInfo->fltTktMerchInd() = 'A';
  subCodeInfo->serviceSubTypeCode() = ServiceSubTypeCode("0DF");
  ocFees->subCodeInfo() = subCodeInfo;

  OptionalServicesInfo* optServicesInfo = _memHandle.insert(new OptionalServicesInfo);
  optServicesInfo->seqNo() = 1234;
  optServicesInfo->upgrdServiceFeesResBkgDesigTblItemNo() = 3000;
  ocFees->optFee() = optServicesInfo;

  Loc* originLoc = _memHandle.insert(new Loc);
  originLoc->loc() = "DEL";
  AirSeg* travelStartSeg = _memHandle.insert(new AirSeg);
  travelStartSeg->origin() = originLoc;
  travelStartSeg->pnrSegment() = 1;
  Loc* destLoc = _memHandle.insert(new Loc);
  destLoc->loc() = "FRA";
  travelStartSeg->destination() = destLoc;
  ocFees->travelStart() = travelStartSeg;
  ocFees->travelEnd() = travelStartSeg;
  ocFees->feeAmount() = 25;
  ocFees->feeCurrency() = "USD";

  ocFees->setDisplayOnly(false);
  return ocFees;
}

void AncillaryPriceModifierProcessorTest::makeTrxSecondCallForMonetaryPricing()
{
  _trx->modifiableActivationFlags().setMonetaryDiscount(true);
  _itin->addAncillaryPriceModifier(AncillaryIdentifier(*(_serviceFeesGroups->at(0)->ocFeesMap()[_farePath][0])), AncillaryPriceModifier());
  _trx->itin().push_back(_itin);
}

TEST_F(AncillaryPriceModifierProcessorTest, givenOcFeesPriceModifierDoesNotExist_whenNotHandlingSecondCallTrx_shouldCreateDefaultWithQuantityEqual1)
{
  auto ocAmountRounderMock = std::make_unique<OcAmountRounderMock>(*_trx);
  EXPECT_CALL(*ocAmountRounderMock, getRoundedFeeAmount(testing::_)).Times(0);

  AncillaryPriceModifierProcessor ancillaryPriceModifierProcessor(*_trx, *_itin, *ocAmountRounderMock);
  ancillaryPriceModifierProcessor.processGroups(*_serviceFeesGroups);

  OCFees* ocFees = _serviceFeesGroups->front()->ocFeesMap()[_farePath].front();
  EXPECT_EQ(ocFees->getCurrentSeg()->_ancPriceModification.get().second._quantity, 1);
}

TEST_F(AncillaryPriceModifierProcessorTest, givenOcFeesPriceModifierDoesNotExist_whenHandlingSecondCallTrx_shouldNotCreateDefaultPriceModifier)
{
  makeTrxSecondCallForMonetaryPricing();

  auto ocAmountRounderMock = std::make_unique<OcAmountRounderMock>(*_trx);
  EXPECT_CALL(*ocAmountRounderMock, getRoundedFeeAmount(testing::_)).Times(0);

  AncillaryPriceModifierProcessor ancillaryPriceModifierProcessor(*_trx, *_itin, *ocAmountRounderMock);
  ancillaryPriceModifierProcessor.processGroups(*_serviceFeesGroups);

  OCFees* ocFees = _serviceFeesGroups->front()->ocFeesMap()[_farePath].front();
  EXPECT_FALSE(ocFees->getCurrentSeg()->_ancPriceModification);
}

TEST_F(AncillaryPriceModifierProcessorTest, whenAncillaryPriceModifierQuantityIs3_shouldAddNewSegmentWithOcFeeMultipliedBy3)
{
  OCFees* ocFees = _serviceFeesGroups->front()->ocFeesMap()[_farePath].front();
  _itin->addAncillaryPriceModifier(AncillaryIdentifier(*ocFees), AncillaryPriceModifier{std::string("id2"), 3});

  auto ocAmountRounderMock = std::make_unique<OcAmountRounderMock>(*_trx);
  EXPECT_CALL(*ocAmountRounderMock, getRoundedFeeAmount(testing::_)).Times(0);

  AncillaryPriceModifierProcessor ancillaryPriceModifierProcessor(*_trx, *_itin, *ocAmountRounderMock);
  ancillaryPriceModifierProcessor.processGroups(*_serviceFeesGroups);

  EXPECT_EQ(ocFees->segCount(), 2);
  EXPECT_EQ(ocFees->getSegPtr(0)->_feeAmount, 25);
  EXPECT_EQ(ocFees->getSegPtr(0)->_ancPriceModification.get().second._quantity, 1);
  EXPECT_EQ(ocFees->getSegPtr(1)->_feeAmount, 75);
  EXPECT_EQ(ocFees->getSegPtr(1)->_ancPriceModification.get().second._quantity, 3);
  EXPECT_EQ(ocFees->getSegPtr(1)->_ancPriceModification.get().second._identifier.get(), "id2");
}

TEST_F(AncillaryPriceModifierProcessorTest, whenAncillaryPriceModifierPercentageDiscountIsInitialized_shouldAddNewSegmentWithOcFeeDiscounted)
{
  OCFees* ocFees = _serviceFeesGroups->front()->ocFeesMap()[_farePath].front();
  _itin->addAncillaryPriceModifier(AncillaryIdentifier(*ocFees), AncillaryPriceModifier{std::string("id2"), 1, AncillaryPriceModifier::Type::DISCOUNT, 10});

  auto ocAmountRounderMock = std::make_unique<OcAmountRounderMock>(*_trx);
  EXPECT_CALL(*ocAmountRounderMock, getRoundedFeeAmount(testing::_)).Times(1)
                                                                    .WillRepeatedly(testing::Invoke(fakeGetRoundedFeeAmountMethod));

  AncillaryPriceModifierProcessor ancillaryPriceModifierProcessor(*_trx, *_itin, *ocAmountRounderMock);
  ancillaryPriceModifierProcessor.processGroups(*_serviceFeesGroups);

  EXPECT_EQ(ocFees->segCount(), 2);
  EXPECT_EQ(ocFees->getSegPtr(0)->_feeAmount, 25);
  EXPECT_FALSE(ocFees->getSegPtr(0)->_ancPriceModification.get().second._percentage);
  EXPECT_EQ(ocFees->getSegPtr(1)->_feeAmount, 23);
  EXPECT_TRUE((bool)ocFees->getSegPtr(1)->_ancPriceModification.get().second._percentage);
  EXPECT_EQ(ocFees->getSegPtr(1)->_ancPriceModification.get().second._percentage.get(), 10);
  EXPECT_EQ(ocFees->getSegPtr(1)->_ancPriceModification.get().second._identifier.get(), "id2");
}

TEST_F(AncillaryPriceModifierProcessorTest, whenAncillaryPriceModifierMoneyDiscountIsInitialized_shouldAddNewSegmentWithOcFeeDiscounted)
{
  OCFees* ocFees = _serviceFeesGroups->front()->ocFeesMap()[_farePath].front();
  AncillaryPriceModifier ancillaryPriceModifier{std::string("id2"), 1, AncillaryPriceModifier::Type::DISCOUNT};
  ancillaryPriceModifier._money = Money(20, "USD");
  _itin->addAncillaryPriceModifier(AncillaryIdentifier(*ocFees), ancillaryPriceModifier);

  auto ocAmountRounderMock = std::make_unique<OcAmountRounderMock>(*_trx);
  EXPECT_CALL(*ocAmountRounderMock, getRoundedFeeAmount(testing::_)).Times(1)
                                                                    .WillRepeatedly(testing::Invoke(fakeGetRoundedFeeAmountMethod));

  AncillaryPriceModifierProcessor ancillaryPriceModifierProcessor(*_trx, *_itin, *ocAmountRounderMock);
  ancillaryPriceModifierProcessor.processGroups(*_serviceFeesGroups);

  EXPECT_EQ(ocFees->segCount(), 2);
  EXPECT_EQ(ocFees->getSegPtr(0)->_feeAmount, 25);
  EXPECT_FALSE(ocFees->getSegPtr(0)->_ancPriceModification.get().second._money);
  EXPECT_EQ(ocFees->getSegPtr(1)->_feeAmount, 5);
  EXPECT_TRUE((bool)ocFees->getSegPtr(1)->_ancPriceModification.get().second._money);
  EXPECT_EQ(ocFees->getSegPtr(1)->_ancPriceModification.get().second._money.get().value(), 20);
  EXPECT_EQ(ocFees->getSegPtr(1)->_ancPriceModification.get().second._identifier.get(), "id2");
}

TEST_F(AncillaryPriceModifierProcessorTest, whenAncillaryPriceModifierMoneyDiscountIsInitializedAndItIsGreaterThanBasePrice_shouldAddNewSegmentWithOcFeeAmountEqual0)
{
  OCFees* ocFees = _serviceFeesGroups->front()->ocFeesMap()[_farePath].front();
  AncillaryPriceModifier ancillaryPriceModifier{std::string("id2"), 1, AncillaryPriceModifier::Type::DISCOUNT};
  ancillaryPriceModifier._money = Money(35, "USD");
  _itin->addAncillaryPriceModifier(AncillaryIdentifier(*ocFees), ancillaryPriceModifier);

  auto ocAmountRounderMock = std::make_unique<OcAmountRounderMock>(*_trx);
  EXPECT_CALL(*ocAmountRounderMock, getRoundedFeeAmount(testing::_)).Times(1)
                                                                    .WillRepeatedly(testing::Invoke(fakeGetRoundedFeeAmountMethod));

  AncillaryPriceModifierProcessor ancillaryPriceModifierProcessor(*_trx, *_itin, *ocAmountRounderMock);
  ancillaryPriceModifierProcessor.processGroups(*_serviceFeesGroups);

  EXPECT_EQ(ocFees->segCount(), 2);
  EXPECT_EQ(ocFees->getSegPtr(0)->_feeAmount, 25);
  EXPECT_FALSE(ocFees->getSegPtr(0)->_ancPriceModification.get().second._money);
  EXPECT_EQ(ocFees->getSegPtr(1)->_feeAmount, 0.0);
  EXPECT_TRUE((bool)ocFees->getSegPtr(1)->_ancPriceModification.get().second._money);
  EXPECT_EQ(ocFees->getSegPtr(1)->_ancPriceModification.get().second._money.get().value(), 35);
  EXPECT_EQ(ocFees->getSegPtr(1)->_ancPriceModification.get().second._identifier.get(), "id2");
}

TEST_F(AncillaryPriceModifierProcessorTest, whenAncillaryPriceModifierMoneyMarkupIsInitialized_shouldAddNewSegmentWithHigherPrice)
{
  OCFees* ocFees = _serviceFeesGroups->front()->ocFeesMap()[_farePath].front();
  AncillaryPriceModifier ancillaryPriceModifier{std::string("id2"), 1, AncillaryPriceModifier::Type::RISE};
  ancillaryPriceModifier._money = Money(10, "USD");
  _itin->addAncillaryPriceModifier(AncillaryIdentifier(*ocFees), ancillaryPriceModifier);

  auto ocAmountRounderMock = std::make_unique<OcAmountRounderMock>(*_trx);
  EXPECT_CALL(*ocAmountRounderMock, getRoundedFeeAmount(testing::_)).Times(1)
                                                                    .WillRepeatedly(testing::Invoke(fakeGetRoundedFeeAmountMethod));

  AncillaryPriceModifierProcessor ancillaryPriceModifierProcessor(*_trx, *_itin, *ocAmountRounderMock);
  ancillaryPriceModifierProcessor.processGroups(*_serviceFeesGroups);

  EXPECT_EQ(ocFees->segCount(), 2);
  EXPECT_EQ(ocFees->getSegPtr(0)->_feeAmount, 25);
  EXPECT_FALSE(ocFees->getSegPtr(0)->_ancPriceModification.get().second._money);
  EXPECT_EQ(ocFees->getSegPtr(1)->_feeAmount, 35);
  EXPECT_TRUE((bool)ocFees->getSegPtr(1)->_ancPriceModification.get().second._money);
  EXPECT_EQ(ocFees->getSegPtr(1)->_ancPriceModification.get().second._money.get().value(), 10);
  EXPECT_EQ(ocFees->getSegPtr(1)->_ancPriceModification.get().second._identifier.get(), "id2");
}

TEST_F(AncillaryPriceModifierProcessorTest, whenAncillaryPriceModifierPercentageMarkupIsInitialized_shouldAddNewSegmentWithHigherPrice)
{
  OCFees* ocFees = _serviceFeesGroups->front()->ocFeesMap()[_farePath].front();
  AncillaryPriceModifier ancillaryPriceModifier{std::string("id2"), 1, AncillaryPriceModifier::Type::RISE, 10};
  _itin->addAncillaryPriceModifier(AncillaryIdentifier(*ocFees), ancillaryPriceModifier);

  auto ocAmountRounderMock = std::make_unique<OcAmountRounderMock>(*_trx);
  EXPECT_CALL(*ocAmountRounderMock, getRoundedFeeAmount(testing::_)).Times(1)
                                                                    .WillRepeatedly(testing::Invoke(fakeGetRoundedFeeAmountMethod));

  AncillaryPriceModifierProcessor ancillaryPriceModifierProcessor(*_trx, *_itin, *ocAmountRounderMock);
  ancillaryPriceModifierProcessor.processGroups(*_serviceFeesGroups);

  EXPECT_EQ(ocFees->segCount(), 2);
  EXPECT_EQ(ocFees->getSegPtr(0)->_feeAmount, 25);
  EXPECT_FALSE(ocFees->getSegPtr(0)->_ancPriceModification.get().second._money);
  EXPECT_EQ(ocFees->getSegPtr(1)->_feeAmount, 28);
  EXPECT_TRUE((bool)ocFees->getSegPtr(1)->_ancPriceModification.get().second._percentage);
  EXPECT_EQ(ocFees->getSegPtr(1)->_ancPriceModification.get().second._percentage.get(), 10);
  EXPECT_EQ(ocFees->getSegPtr(1)->_ancPriceModification.get().second._identifier.get(), "id2");
}

}
