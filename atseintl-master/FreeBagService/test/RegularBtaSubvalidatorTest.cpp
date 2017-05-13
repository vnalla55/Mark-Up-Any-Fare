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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "FreeBagService/IS7RecordFieldsValidator.h"
#include "FreeBagService/RegularBtaSubvalidator.h"
#include "ServiceFees/OCFees.h"

#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "FreeBagService/test/S5Builder.h"
#include "FreeBagService/test/S7Builder.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestDataBuilders.h"
#include "test/include/TestMemHandle.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace tse
{
using namespace ::testing;

class S7ValidatorMock : public IS7RecordFieldsValidator
{
public:
  MOCK_CONST_METHOD1(determineSpecialSegmentForT186, const TravelSeg*(const BaggageTravel& bt));
  MOCK_CONST_METHOD2(checkCabinInSegment, bool(TravelSeg* segment, const Indicator cabin));
  MOCK_CONST_METHOD4(checkRBDInSegment,
                     bool(TravelSeg* segment,
                          OCFees& ocFees,
                          uint32_t serviceFeesResBkgDesigTblItemNo,
                          const std::vector<SvcFeesResBkgDesigInfo*>& rbdInfos));
  MOCK_CONST_METHOD4(checkResultingFareClassInSegment,
                     bool(const PaxTypeFare* paxTypeFare,
                          uint32_t serviceFeesCxrResultingFclTblItemNo,
                          OCFees& ocFees,
                          const std::vector<SvcFeesCxrResultingFCLInfo*>& resFCLInfo));
  MOCK_CONST_METHOD3(checkOutputTicketDesignatorInSegment,
                     bool(TravelSeg* segment, const PaxTypeFare*, const OptionalServicesInfo& s7));
  MOCK_CONST_METHOD3(checkRuleInSegment,
                     bool(const PaxTypeFare* paxTypeFareLinkedWithSegment,
                          const RuleNumber& rule,
                          OCFees& ocFees));
  MOCK_CONST_METHOD3(checkRuleTariffInSegment,
                     bool(const PaxTypeFare* paxTypeFareLinkedWithSegment,
                          uint16_t ruleTariff,
                          OCFees& ocFees));
  MOCK_CONST_METHOD2(checkFareIndInSegment,
                     bool(const PaxTypeFare* paxTypeFareLinkedWithSegment,
                          const Indicator& fareInd));
  MOCK_CONST_METHOD3(checkCarrierFlightApplT186InSegment,
                     bool(const TravelSeg* segment, const VendorCode& vendor, uint32_t itemNo));
};

class MyDataHandleMock : public DataHandleMock
{
  const std::vector<SvcFeesResBkgDesigInfo*>& getSvcFeesResBkgDesig(const VendorCode&, const int)
  {
    static const std::vector<SvcFeesResBkgDesigInfo*> empty;
    return empty;
  }

  const std::vector<SvcFeesCxrResultingFCLInfo*>&
  getSvcFeesCxrResultingFCL(const VendorCode&, const int)
  {
    static const std::vector<SvcFeesCxrResultingFCLInfo*> empty;
    return empty;
  }
};

class RegularBtaSubvalidatorTest : public Test
{
public:
  void SetUp()
  {
    _mh.create<TestConfigInitializer>();
    _mh.create<MyDataHandleMock>();
    _trx = _mh(new PricingTrx);
    _s7Validator = _mh(new S7ValidatorMock);
    _segs = {_mh(new AirSeg), _mh(new ArunkSeg), _mh(new AirSeg), _mh(new AirSeg)};
    _itin = _mh(new Itin);
    _itin->travelSeg() = _segs;
    _itin->travelSeg().push_back(_mh(new AirSeg));
    FarePath* fp = _mh(new FarePath);
    fp->itin() = _itin;
    FareMarketBuilder fmb(_mh);
    _fares = {createPtf(fmb.withSegs(_segs.begin(), _segs.begin() + 3).build()),
              createPtf(fmb.withSegs(_segs.begin() + 3, _segs.end()).build())};
    _bt = BaggageTravelBuilder(&_mh).withTrx(_trx).withTravelSeg(_segs).withFarePath(fp).build();
    _validator = _mh(new RegularBtaSubvalidator(*_bt, *_s7Validator, _ts2ss, nullptr));
    _oc = createOCFees();
  }

  void TearDown() { _mh.clear(); }

protected:
  PaxTypeFare* createPtf(FareMarket* fm)
  {
    PaxTypeFare* ptf = _mh(new PaxTypeFare);
    ptf->fareMarket() = fm;

    for (TravelSeg* ts : fm->travelSeg())
      _ts2ss[ts] = std::make_pair(nullptr, ptf);

    return ptf;
  }

  OCFees* createOCFees()
  {
    OCFees* oc = _mh(new OCFees);
    oc->subCodeInfo() = S5Builder(&_mh).withFltTktMerchInd('A').build();
    return oc;
  }

  TestMemHandle _mh;
  PricingTrx* _trx = nullptr;
  S7ValidatorMock* _s7Validator = nullptr;
  std::vector<TravelSeg*> _segs;
  std::vector<PaxTypeFare*> _fares;
  Ts2ss _ts2ss;
  Itin* _itin = nullptr;
  BaggageTravel* _bt = nullptr;
  RegularBtaSubvalidator* _validator = nullptr;
  OCFees* _oc = nullptr;
};

TEST_F(RegularBtaSubvalidatorTest, testAllBlankEmpty)
{
  const auto s7 = S7Builder(&_mh).build();
  s7->ruleTariff() = uint16_t(-1);
  EXPECT_EQ(PASS_S7, _validator->validate(*s7, *_oc));
}

TEST_F(RegularBtaSubvalidatorTest, testAllValidS)
{
  const auto s7 = S7Builder(&_mh).withBTA('S').build();
  s7->cabin() = 'C';
  s7->serviceFeesResBkgDesigTblItemNo() = 1;
  s7->carrierFltTblItemNo() = 2;
  s7->serviceFeesCxrResultingFclTblItemNo() = 3;
  s7->resultServiceFeesTktDesigTblItemNo() = 4;
  s7->ruleTariff() = 5;
  s7->rule() = "RULE";
  s7->fareInd() = 'F';

  EXPECT_CALL(*_s7Validator, determineSpecialSegmentForT186(_)).Times(AnyNumber());
  EXPECT_CALL(*_s7Validator, checkCabinInSegment(_segs.front(), 'C')).Times(1).WillOnce(
      Return(true));
  EXPECT_CALL(*_s7Validator, checkRBDInSegment(_segs.front(), _, 1, _)).Times(1).WillOnce(
      Return(true));
  EXPECT_CALL(*_s7Validator, checkCarrierFlightApplT186InSegment(_segs.front(), _, 2))
      .Times(1)
      .WillOnce(Return(true));
  EXPECT_CALL(*_s7Validator, checkResultingFareClassInSegment(_fares.front(), 3, _, _))
      .Times(1)
      .WillOnce(Return(true));
  EXPECT_CALL(*_s7Validator, checkOutputTicketDesignatorInSegment(_segs.front(), _fares.front(), *s7))
      .Times(1)
      .WillOnce(Return(true));
  EXPECT_CALL(*_s7Validator, checkRuleTariffInSegment(_fares.front(), 5, _)).Times(1).WillOnce(
      Return(true));
  EXPECT_CALL(*_s7Validator, checkRuleInSegment(_fares.front(), _, _)).Times(1).WillOnce(
      Return(true));
  EXPECT_CALL(*_s7Validator, checkFareIndInSegment(_fares.front(), 'F')).Times(1).WillOnce(
      Return(true));

  EXPECT_EQ(PASS_S7, _validator->validate(*s7, *_oc));
}

TEST_F(RegularBtaSubvalidatorTest, testLastValidJ)
{
  const auto s7 = S7Builder(&_mh).withBTA('J').build();
  s7->cabin() = 'C';
  s7->ruleTariff() = uint16_t(-1);

  EXPECT_CALL(*_s7Validator, determineSpecialSegmentForT186(_)).Times(AnyNumber());
  EXPECT_CALL(*_s7Validator, checkCabinInSegment(_itin->travelSeg().back(), 'C')).Times(1).WillOnce(
      Return(true));
  EXPECT_CALL(*_s7Validator, checkCabinInSegment(Not(_itin->travelSeg().back()), 'C'))
      .Times(3)
      .WillRepeatedly(Return(false));
  EXPECT_EQ(PASS_S7, _validator->validate(*s7, *_oc));
}

TEST_F(RegularBtaSubvalidatorTest, testAllFailJ)
{
  const auto s7 = S7Builder(&_mh).withBTA('J').build();
  s7->cabin() = 'C';
  s7->ruleTariff() = uint16_t(-1);

  EXPECT_CALL(*_s7Validator, determineSpecialSegmentForT186(_)).Times(AnyNumber());
  EXPECT_CALL(*_s7Validator, checkCabinInSegment(_, 'C')).Times(4).WillRepeatedly(Return(false));
  EXPECT_EQ(FAIL_S7_CABIN, _validator->validate(*s7, *_oc));
}

TEST_F(RegularBtaSubvalidatorTest, testValidM)
{
  const auto s7 = S7Builder(&_mh).withBTA('M').build();
  s7->ruleTariff() = 1;
  _bt->_MSS = _segs.begin();

  EXPECT_CALL(*_s7Validator, determineSpecialSegmentForT186(_)).Times(AnyNumber());
  EXPECT_CALL(*_s7Validator, checkRuleTariffInSegment(_fares.front(), _, _)).Times(1).WillOnce(
      Return(true));
  EXPECT_CALL(*_s7Validator, checkRuleTariffInSegment(_fares.back(), _, _)).Times(0);
  EXPECT_EQ(PASS_S7, _validator->validate(*s7, *_oc));
}

TEST_F(RegularBtaSubvalidatorTest, testFailM)
{
  const auto s7 = S7Builder(&_mh).withBTA('M').build();
  s7->ruleTariff() = 1;
  _bt->_MSS = _segs.end() - 1;

  EXPECT_CALL(*_s7Validator, determineSpecialSegmentForT186(_)).Times(AnyNumber());
  EXPECT_CALL(*_s7Validator, checkRuleTariffInSegment(_fares.back(), _, _)).Times(1).WillOnce(
      Return(false));
  EXPECT_CALL(*_s7Validator, checkRuleTariffInSegment(_fares.front(), _, _)).Times(0);
  EXPECT_EQ(FAIL_S7_RULE_TARIFF, _validator->validate(*s7, *_oc));
}

TEST_F(RegularBtaSubvalidatorTest, testValidSegsInvalidFaresA)
{
  const auto s7 = S7Builder(&_mh).withBTA('A').build();
  s7->cabin() = 'C';
  s7->ruleTariff() = 1;

  EXPECT_CALL(*_s7Validator, determineSpecialSegmentForT186(_)).Times(AnyNumber());
  EXPECT_CALL(*_s7Validator, checkCabinInSegment(_, 'C')).Times(AnyNumber()).WillRepeatedly(
      Return(true));
  EXPECT_CALL(*_s7Validator, checkRuleTariffInSegment(_fares.front(), _, _)).Times(1).WillOnce(
      Return(true));
  EXPECT_CALL(*_s7Validator, checkRuleTariffInSegment(_fares.back(), _, _)).Times(1).WillOnce(
      Return(false));
  EXPECT_EQ(FAIL_S7_RULE_TARIFF, _validator->validate(*s7, *_oc));
}

TEST_F(RegularBtaSubvalidatorTest, testInvalidSegsValidFaresA)
{
  const auto s7 = S7Builder(&_mh).withBTA('A').build();
  s7->cabin() = 'C';
  s7->ruleTariff() = 1;

  EXPECT_CALL(*_s7Validator, determineSpecialSegmentForT186(_)).Times(AnyNumber());
  EXPECT_CALL(*_s7Validator, checkCabinInSegment(_segs[0], 'C')).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*_s7Validator, checkCabinInSegment(_segs[1], 'C')).Times(0); // arunk
  EXPECT_CALL(*_s7Validator, checkCabinInSegment(_segs[2], 'C')).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*_s7Validator, checkCabinInSegment(_segs[3], 'C')).Times(1).WillOnce(Return(false));
  EXPECT_CALL(*_s7Validator, checkRuleTariffInSegment(_, _, _)).Times(AnyNumber()).WillRepeatedly(
      Return(true));
  EXPECT_EQ(FAIL_S7_CABIN, _validator->validate(*s7, *_oc));
}

TEST_F(RegularBtaSubvalidatorTest, testAllValidA)
{
  const auto s7 = S7Builder(&_mh).withBTA('A').build();
  s7->cabin() = 'C';
  s7->ruleTariff() = 1;

  EXPECT_CALL(*_s7Validator, determineSpecialSegmentForT186(_)).Times(AnyNumber());
  EXPECT_CALL(*_s7Validator, checkCabinInSegment(_, 'C')).Times(3).WillRepeatedly(Return(true));
  EXPECT_CALL(*_s7Validator, checkRuleTariffInSegment(_, _, _)).Times(2).WillRepeatedly(
      Return(true));
  EXPECT_EQ(PASS_S7, _validator->validate(*s7, *_oc));
}

TEST_F(RegularBtaSubvalidatorTest, testDeferValidEmpty)
{
  const auto s7 = S7Builder(&_mh).withBTA(' ').withNotAvailNoCharge('D').build();
  s7->carrierFltTblItemNo() = 1;
  s7->ruleTariff() = uint16_t(-1);

  EXPECT_CALL(*_s7Validator, determineSpecialSegmentForT186(_)).Times(AnyNumber()).WillRepeatedly(
      Return(_segs.front()));
  EXPECT_CALL(*_s7Validator, checkCarrierFlightApplT186InSegment(_segs.front(), _, _))
      .Times(1)
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*_s7Validator, checkCarrierFlightApplT186InSegment(Not(_segs.front()), _, _))
      .Times(AnyNumber())
      .WillRepeatedly(Return(false));
  EXPECT_EQ(PASS_S7, _validator->validate(*s7, *_oc));
}

TEST_F(RegularBtaSubvalidatorTest, testDeferInvalidEmpty)
{
  const auto s7 = S7Builder(&_mh).withBTA(' ').withNotAvailNoCharge('D').build();
  s7->carrierFltTblItemNo() = 1;
  s7->ruleTariff() = uint16_t(-1);

  EXPECT_CALL(*_s7Validator, determineSpecialSegmentForT186(_)).Times(AnyNumber()).WillRepeatedly(
      Return(_segs.front()));
  EXPECT_CALL(*_s7Validator, checkCarrierFlightApplT186InSegment(_segs.front(), _, _))
      .Times(1)
      .WillRepeatedly(Return(false));
  EXPECT_CALL(*_s7Validator, checkCarrierFlightApplT186InSegment(Not(_segs.front()), _, _))
      .Times(AnyNumber())
      .WillRepeatedly(Return(true));
  EXPECT_EQ(FAIL_S7_CXR_FLT_T186, _validator->validate(*s7, *_oc));
}
}
