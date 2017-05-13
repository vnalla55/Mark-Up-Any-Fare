//----------------------------------------------------------------------------
//  Copyright Sabre 2014
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

#include "Pricing/PreserveRestoreRuleBasedFares.h"

#include "DBAccess/CombinabilityRuleInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PaxTypeFare.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/PricingUtil.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/include/GtestHelperMacros.h"

#include <memory>

using namespace ::testing;

namespace tse
{

class PreserveRestorePURuleBasedFaresMock : public PreserveRestorePURuleBasedFares
{
public:
  MOCK_CONST_METHOD1(determinePaxTypeFare, tse::PaxTypeFare*(tse::PaxTypeFare*));
  virtual ~PreserveRestorePURuleBasedFaresMock() {};
};

class PreserveRestorePURuleBasedFaresTest : public Test
{
public:
  void SetUp()
  {
    _pricingUnit.reset(new PricingUnit());
    _fare1.reset(new PaxTypeFare());
    _fare2.reset(new PaxTypeFare());
    _rec2Cat10.reset(new CombinabilityRuleInfo());
    _diag.reset(new DiagCollector());

    _fareUsage.paxTypeFare() = _fare1.get();
    _fareUsage.rec2Cat10() = _rec2Cat10.get();
    _fareUsageVec.push_back(&_fareUsage);
    _pricingUnit->fareUsage() = _fareUsageVec;
  }
  void TearDown() {}

  FareUsage _fareUsage;
  std::vector<FareUsage*> _fareUsageVec;
  std::shared_ptr<tse::DiagCollector> _diag;
  std::shared_ptr<PricingUnit> _pricingUnit;
  std::shared_ptr<tse::PaxTypeFare> _fare1;
  std::shared_ptr<tse::PaxTypeFare> _fare2;
  std::shared_ptr<tse::CombinabilityRuleInfo> _rec2Cat10;
};


TEST_F(PreserveRestorePURuleBasedFaresTest, testPreserveDifferentPaxTypeFares)
{
  PreserveRestorePURuleBasedFaresMock* _preserveRestoreMock =
      new PreserveRestorePURuleBasedFaresMock();

  EXPECT_CALL(*_preserveRestoreMock, determinePaxTypeFare(_)).WillOnce(Return(_fare2.get()));

  _preserveRestoreMock->Preserve(_pricingUnit.get(), _diag.get());
  ASSERT_EQ(1, _pricingUnit->getFaresMapSizeCat10overrideCat25());

  delete _preserveRestoreMock;
  ASSERT_EQ(0, _pricingUnit->getFaresMapSizeCat10overrideCat25());
}

TEST_F(PreserveRestorePURuleBasedFaresTest, testPreserveTheSamePaxTypeFares)
{
  PreserveRestorePURuleBasedFaresMock* _preserveRestoreMock =
      new PreserveRestorePURuleBasedFaresMock();

  EXPECT_CALL(*_preserveRestoreMock, determinePaxTypeFare(_)).WillOnce(Return(_fare1.get()));

  _preserveRestoreMock->Preserve(_pricingUnit.get(), _diag.get());
  ASSERT_EQ(0, _pricingUnit->getFaresMapSizeCat10overrideCat25());

  delete _preserveRestoreMock;
  ASSERT_EQ(0, _pricingUnit->getFaresMapSizeCat10overrideCat25());
}

TEST_F(PreserveRestorePURuleBasedFaresTest, testPreserveFarePath)
{
  PreserveRestorePURuleBasedFaresMock* _preserveRestoreMock =
      new PreserveRestorePURuleBasedFaresMock();

  std::vector<PricingUnit *> PUs;
  PUs.push_back(_pricingUnit.get());

  EXPECT_CALL(*_preserveRestoreMock, determinePaxTypeFare(_)).WillOnce(Return(_fare2.get()));

  _preserveRestoreMock->Preserve(PUs, _diag.get());
  ASSERT_EQ(1, _pricingUnit->getFaresMapSizeCat10overrideCat25());

  delete _preserveRestoreMock;
  ASSERT_EQ(0, _pricingUnit->getFaresMapSizeCat10overrideCat25());
}

} // namespace tse
