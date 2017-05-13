// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
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

#include "DataModel/BaggageTravel.h"

#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/AllowanceFinder.h"
#include "FreeBagService/AllowanceStepProcessor.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "FreeBagService/BagValidationOpt.h"

#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestDataBuilders.h"
#include "test/include/TestMemHandle.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace tse
{
namespace
{
using namespace ::testing;

class DataHandleGMock : public DataHandleMock
{
public:
  MOCK_METHOD1(isCarrierUSDot, bool(const CarrierCode& cxr));
  MOCK_METHOD1(isCarrierCTA, bool(const CarrierCode& cxr));
};

class AllowanceStepMock : public AllowanceStepProcessor
{
public:
  MOCK_METHOD1(matchAllowance, AllowanceStepResult(const BagValidationOpt& opt));
  MOCK_METHOD1(dotTableCheckFailed, void(const BagValidationOpt& opt));
};

class ChargeStepMock : public ChargeStepProcessor
{
public:
  MOCK_METHOD1(matchCharges, void(const BagValidationOpt& opt));
};

class AllowanceFinderTest : public Test
{
public:
  void SetUp()
  {
    _mh.create<TestConfigInitializer>();
    _trx = _mh(new PricingTrx);
    _trx->setRequest(_mh(new PricingRequest));
    _trx->ticketingDate() = DateTime(9999,1,1);
    _itin = _mh(new Itin);
    AirSegBuilder ab(_mh);
    _itin->travelSeg() = {ab.withCxr("FM", "FO").withLocs("L1", "L2").build(),
                          ab.withCxr("MM", "MO").withLocs("L2", "L3").build(),
                          ab.withCxr("GM", "GO").withLocs("L3", "L4").build(),
                          ab.withCxr("NM", "NO").withLocs("L4", "L5").build()};
    _bt = _mh(new BaggageTravel);
    _bt->_trx = _trx;
    _bt->setItin(*_itin);
    _bt->updateSegmentsRange(_itin->travelSeg().begin() + 2, _itin->travelSeg().end());
    _bt->_MSS = _itin->travelSeg().begin() + 3;
    _bt->_MSSJourney = _itin->travelSeg().begin() + 1;
    _mock = _mh(new AllowanceStepMock);
    _chargeMock = _mh(new ChargeStepMock);
    _finder = _mh(new AllowanceFinder(*_mock, *_chargeMock, *_bt, CheckedPoint()));
    _dh = _mh(new DataHandleGMock);
  }

  void TearDown()
  {
    _mh.clear();
  }

protected:
  void callFinder()
  {
    _finder->findAllowanceAndCharges();
  }

  TestMemHandle _mh;
  PricingTrx* _trx = nullptr;
  Itin* _itin = nullptr;
  BaggageTravel* _bt = nullptr;
  AllowanceStepMock* _mock = nullptr;
  ChargeStepMock* _chargeMock = nullptr;
  AllowanceFinder* _finder = nullptr;
  DataHandleGMock* _dh = nullptr;
};

// arg is of type const BagValidationOpt&
MATCHER_P2(ForCxrPair, s5Cxr, deferCxr, "")
{
  return arg._bt._allowanceCxr == s5Cxr && arg._deferTargetCxr == deferCxr;
}

TEST_F(AllowanceFinderTest, testWhollyWithin_FailS5)
{
  _itin->setBaggageTripType(BaggageTripType::WHOLLY_WITHIN_US);

  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("FM", ""))).Times(1).WillOnce(
      Return(AllowanceStepResult::S5_FAIL));
  EXPECT_CALL(*_mock, matchAllowance(Not(ForCxrPair("FM", "")))).Times(0);
  EXPECT_CALL(*_mock, dotTableCheckFailed(_)).Times(0);
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}

TEST_F(AllowanceFinderTest, testWhollyWithin_PassS7)
{
  _itin->setBaggageTripType(BaggageTripType::WHOLLY_WITHIN_CA);

  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("FM", ""))).Times(1).WillOnce(
      Return(AllowanceStepResult::S7_PASS));
  EXPECT_CALL(*_mock, matchAllowance(Not(ForCxrPair("FM", "")))).Times(0);
  EXPECT_CALL(*_mock, dotTableCheckFailed(_)).Times(0);
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}

TEST_F(AllowanceFinderTest, testUsDot_DotTableFailed)
{
  _itin->setBaggageTripType(BaggageTripType::BETWEEN_US_CA);

  EXPECT_CALL(*_dh, isCarrierUSDot(_)).Times(AnyNumber()).WillRepeatedly(Return(true));
  EXPECT_CALL(*_dh, isCarrierCTA(_)).Times(AnyNumber()).WillRepeatedly(Return(false));
  EXPECT_CALL(*_mock, matchAllowance(_)).Times(0);
  EXPECT_CALL(*_mock, dotTableCheckFailed(_)).Times(1);
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}

TEST_F(AllowanceFinderTest, testUsDot_FailS5)
{
  _itin->setBaggageTripType(BaggageTripType::TO_FROM_US);

  EXPECT_CALL(*_dh, isCarrierUSDot(CarrierCode("FM"))).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("FM", "MM"))).Times(1).WillOnce(
      Return(AllowanceStepResult::S5_FAIL));
  EXPECT_CALL(*_mock, matchAllowance(Not(ForCxrPair("FM", "MM")))).Times(0);
  EXPECT_CALL(*_mock, dotTableCheckFailed(_)).Times(0);
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}

TEST_F(AllowanceFinderTest, testUsDot_FailS7)
{
  _itin->setBaggageTripType(BaggageTripType::TO_FROM_US);

  EXPECT_CALL(*_dh, isCarrierUSDot(CarrierCode("FM"))).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("FM", "MM"))).Times(1).WillOnce(
      Return(AllowanceStepResult::S7_FAIL));
  EXPECT_CALL(*_mock, matchAllowance(Not(ForCxrPair("FM", "MM")))).Times(0);
  EXPECT_CALL(*_mock, dotTableCheckFailed(_)).Times(0);
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}

TEST_F(AllowanceFinderTest, testUsDot_PassS7)
{
  _itin->setBaggageTripType(BaggageTripType::TO_FROM_US);

  EXPECT_CALL(*_dh, isCarrierUSDot(CarrierCode("FM"))).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("FM", "MM"))).Times(1).WillOnce(
      Return(AllowanceStepResult::S7_PASS));
  EXPECT_CALL(*_mock, matchAllowance(Not(ForCxrPair("FM", "MM")))).Times(0);
  EXPECT_CALL(*_mock, dotTableCheckFailed(_)).Times(0);
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}

TEST_F(AllowanceFinderTest, testUsDot_Defer_DotTableFail)
{
  _itin->setBaggageTripType(BaggageTripType::TO_FROM_CA);

  EXPECT_CALL(*_dh, isCarrierCTA(CarrierCode("FM"))).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*_dh, isCarrierCTA(CarrierCode("MM"))).Times(1).WillOnce(Return(false));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("FM", "MM"))).Times(1).WillOnce(
      Return(AllowanceStepResult::S7_DEFER));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("FM", ""))).Times(1).WillOnce(
        Return(AllowanceStepResult::S7_PASS));
  EXPECT_CALL(*_mock, dotTableCheckFailed(_)).Times(0);
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}

TEST_F(AllowanceFinderTest, testUsDot_Defer_FailS5)
{
  _itin->setBaggageTripType(BaggageTripType::TO_FROM_CA);

  EXPECT_CALL(*_dh, isCarrierCTA(CarrierCode("FM"))).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*_dh, isCarrierCTA(CarrierCode("MM"))).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("FM", "MM"))).Times(1).WillOnce(
      Return(AllowanceStepResult::S7_DEFER));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("MM", ""))).Times(1).WillOnce(
        Return(AllowanceStepResult::S5_FAIL));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("FM", ""))).Times(1).WillOnce(
        Return(AllowanceStepResult::S7_PASS));
  EXPECT_CALL(*_mock, dotTableCheckFailed(_)).Times(0);
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}

TEST_F(AllowanceFinderTest, testUsDot_Defer_PassS7)
{
  _itin->setBaggageTripType(BaggageTripType::TO_FROM_CA);

  EXPECT_CALL(*_dh, isCarrierCTA(CarrierCode("FM"))).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*_dh, isCarrierCTA(CarrierCode("MM"))).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("FM", "MM"))).Times(1).WillOnce(
      Return(AllowanceStepResult::S7_DEFER));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("MM", ""))).Times(1).WillOnce(
        Return(AllowanceStepResult::S7_PASS));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("FM", ""))).Times(0);
  EXPECT_CALL(*_mock, dotTableCheckFailed(_)).Times(0);
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}

TEST_F(AllowanceFinderTest, testNonUsDot_FailS5)
{
  _itin->setBaggageTripType(BaggageTripType::OTHER);

  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("NM", "NO"))).Times(1).WillOnce(
      Return(AllowanceStepResult::S5_FAIL));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("GM", "GO"))).Times(1).WillOnce(
      Return(AllowanceStepResult::S7_PASS));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("GO", ""))).Times(0);
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}

TEST_F(AllowanceFinderTest, testNonUsDot_FailS7)
{
  _itin->setBaggageTripType(BaggageTripType::OTHER);

  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("NM", "NO"))).Times(1).WillOnce(
      Return(AllowanceStepResult::S7_FAIL));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("GM", "GO"))).Times(0);
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("GO", ""))).Times(0);
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}

TEST_F(AllowanceFinderTest, testNonUsDot_PassS7)
{
  _itin->setBaggageTripType(BaggageTripType::OTHER);

  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("NM", "NO"))).Times(1).WillOnce(
      Return(AllowanceStepResult::S7_PASS));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("GM", "GO"))).Times(0);
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("GO", ""))).Times(0);
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}

TEST_F(AllowanceFinderTest, testNonUsDot_Defer_FailS5)
{
  _itin->setBaggageTripType(BaggageTripType::OTHER);

  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("NM", "NO"))).Times(1).WillOnce(
      Return(AllowanceStepResult::S7_DEFER));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("NO", ""))).Times(1).WillOnce(
      Return(AllowanceStepResult::S5_FAIL));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("GM", "GO"))).Times(1).WillOnce(
        Return(AllowanceStepResult::S7_PASS));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("GO", ""))).Times(0);
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}

TEST_F(AllowanceFinderTest, testNonUsDot_Defer_FailS5_Defer)
{
  _itin->setBaggageTripType(BaggageTripType::OTHER);

  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("NM", "NO"))).Times(1).WillOnce(
      Return(AllowanceStepResult::S7_DEFER));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("NO", ""))).Times(1).WillOnce(
      Return(AllowanceStepResult::S5_FAIL));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("GM", "GO"))).Times(1).WillOnce(
        Return(AllowanceStepResult::S7_DEFER));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("GO", ""))).Times(1).WillOnce(
        Return(AllowanceStepResult::S7_PASS));
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}

TEST_F(AllowanceFinderTest, testNonUsDot_Defer_PassS7)
{
  _itin->setBaggageTripType(BaggageTripType::OTHER);

  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("NM", "NO"))).Times(1).WillOnce(
      Return(AllowanceStepResult::S7_DEFER));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("NO", ""))).Times(1).WillOnce(
      Return(AllowanceStepResult::S7_PASS));
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("GM", "GO"))).Times(0);
  EXPECT_CALL(*_mock, matchAllowance(ForCxrPair("GO", ""))).Times(0);
  EXPECT_CALL(*_chargeMock, matchCharges(_)).Times(1);
  callFinder();
}
}
}
