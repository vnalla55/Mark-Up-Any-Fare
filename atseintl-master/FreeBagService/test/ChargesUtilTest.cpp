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

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/ChargesUtil.h"
#include "FreeBagService/test/S5Builder.h"
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

using S5Vec = std::vector<SubCodeInfo*>;

class DataHandleGMock : public DataHandleMock
{
public:
  MOCK_METHOD5(getSubCode,
               const S5Vec&(const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const ServiceTypeCode& serviceTypeCode,
                            const ServiceGroup& serviceGroup,
                            const DateTime& date));
};

const VendorCode ATP = "ATP";
const VendorCode MMGR = "MMGR";
const CarrierCode AA = "AA";
const ServiceTypeCode OC = "OC";
const ServiceGroup BG = "BG";
const ServiceGroup PT = "PT";
const std::vector<SubCodeInfo*> EMPTY_S5 = {};

class ChargesUtilTest : public Test
{
public:
  void SetUp() override
  {
    _mh.create<TestConfigInitializer>();
    _trx = _mh(new PricingTrx);
    _trx->setRequest(_mh(new PricingRequest));
    _trx->ticketingDate() = DateTime(9999, 1, 1);
    _itin = _mh(new Itin);
    AirSegBuilder ab(_mh);
    _itin->travelSeg() = {ab.withCxr("AA", "AA").withLocs("L1", "L2").build(),
                          ab.withCxr("AA", "AA").withLocs("L2", "L3").build()};
    _dh = _mh(new DataHandleGMock);
  }

  void TearDown() override { _mh.clear(); }

protected:
  TestMemHandle _mh;
  PricingTrx* _trx = nullptr;
  Itin* _itin = nullptr;
  DataHandleGMock* _dh = nullptr;
};

TEST_F(ChargesUtilTest, testRetrieveS5_Empty)
{
  ON_CALL(*_dh, getSubCode(_, _, _, _, _)).WillByDefault(ReturnRef(EMPTY_S5));
  EXPECT_CALL(*_dh, getSubCode(ATP, AA, OC, BG, _)).Times(1);
  EXPECT_CALL(*_dh, getSubCode(ATP, AA, OC, PT, _)).Times(1);
  EXPECT_CALL(*_dh, getSubCode(MMGR, AA, OC, BG, _)).Times(1);
  EXPECT_CALL(*_dh, getSubCode(MMGR, AA, OC, PT, _)).Times(1);

  EXPECT_TRUE(ChargesUtil::retrieveS5(*_trx, *_itin, "AA").empty());
}

TEST_F(ChargesUtilTest, testRetrieveS5_NotCharge)
{
  S5Builder bld(&_mh);
  const S5Vec s5 = {bld.withFltTktMerchInd('A').withDesc("03").build()};

  ON_CALL(*_dh, getSubCode(_, _, _, _, _)).WillByDefault(ReturnRef(EMPTY_S5));
  EXPECT_CALL(*_dh, getSubCode(_, _, _, _, _)).Times(AnyNumber());
  EXPECT_CALL(*_dh, getSubCode(ATP, AA, OC, BG, _)).Times(1).WillOnce(ReturnRef(s5));

  EXPECT_TRUE(ChargesUtil::retrieveS5(*_trx, *_itin, "AA").empty());
}

TEST_F(ChargesUtilTest, testRetrieveS5_InvalidSvcGroup)
{
  S5Builder bld(&_mh);
  const S5Vec s5 = {bld.withFltTktMerchInd('C').withSubGroup("SBG").withDesc("03").build()};

  ON_CALL(*_dh, getSubCode(_, _, _, _, _)).WillByDefault(ReturnRef(EMPTY_S5));
  EXPECT_CALL(*_dh, getSubCode(_, _, _, _, _)).Times(AnyNumber());
  EXPECT_CALL(*_dh, getSubCode(ATP, AA, OC, BG, _)).Times(1).WillOnce(ReturnRef(s5));

  EXPECT_TRUE(ChargesUtil::retrieveS5(*_trx, *_itin, "AA").empty());
}

TEST_F(ChargesUtilTest, testRetrieveS5_InvalidDescription)
{
  S5Builder bld(&_mh);
  const S5Vec s5 = {bld.withFltTktMerchInd('C').withDesc("0X").build()};

  ON_CALL(*_dh, getSubCode(_, _, _, _, _)).WillByDefault(ReturnRef(EMPTY_S5));
  EXPECT_CALL(*_dh, getSubCode(_, _, _, _, _)).Times(AnyNumber());
  EXPECT_CALL(*_dh, getSubCode(ATP, AA, OC, BG, _)).Times(1).WillOnce(ReturnRef(s5));

  EXPECT_TRUE(ChargesUtil::retrieveS5(*_trx, *_itin, "AA").empty());
}

TEST_F(ChargesUtilTest, testRetrieveS5_Pass)
{
  S5Builder bld(&_mh);
  const S5Vec s5 = {bld.withFltTktMerchInd('C').withDesc("03").withSubCode("0GO").build()};

  ON_CALL(*_dh, getSubCode(_, _, _, _, _)).WillByDefault(ReturnRef(EMPTY_S5));
  EXPECT_CALL(*_dh, getSubCode(_, _, _, _, _)).Times(AnyNumber());
  EXPECT_CALL(*_dh, getSubCode(ATP, AA, OC, BG, _)).Times(1).WillOnce(ReturnRef(s5));

  const auto& ret = ChargesUtil::retrieveS5(*_trx, *_itin, "AA");
  ASSERT_EQ(size_t(1), ret.size());
  ASSERT_EQ(s5.front(), ret.front());
}

TEST_F(ChargesUtilTest, testRetrieveS5_Interline)
{
  S5Builder bld(&_mh);
  const S5Vec s5 = {bld.withFltTktMerchInd('C').withDesc("03").withSubCode("0GO").build()};
  AirSeg* as = _itin->travelSeg().back()->toAirSeg();
  as->setMarketingCarrierCode("BA");
  as->setOperatingCarrierCode("BA");

  ON_CALL(*_dh, getSubCode(_, _, _, _, _)).WillByDefault(ReturnRef(EMPTY_S5));
  EXPECT_CALL(*_dh, getSubCode(_, _, _, _, _)).Times(AnyNumber());
  EXPECT_CALL(*_dh, getSubCode(ATP, AA, OC, BG, _)).Times(1).WillOnce(ReturnRef(s5));

  EXPECT_TRUE(ChargesUtil::retrieveS5(*_trx, *_itin, "AA").empty());
}

TEST_F(ChargesUtilTest, testRetrieveS5_CodeShare)
{
  S5Builder bld(&_mh);
  const S5Vec s5 = {bld.withFltTktMerchInd('C').withDesc("03").withSubCode("0GO").build()};
  AirSeg* as = _itin->travelSeg().back()->toAirSeg();
  as->setOperatingCarrierCode("BA");

  ON_CALL(*_dh, getSubCode(_, _, _, _, _)).WillByDefault(ReturnRef(EMPTY_S5));
  EXPECT_CALL(*_dh, getSubCode(_, _, _, _, _)).Times(AnyNumber());
  EXPECT_CALL(*_dh, getSubCode(ATP, AA, OC, BG, _)).Times(1).WillOnce(ReturnRef(s5));

  EXPECT_TRUE(ChargesUtil::retrieveS5(*_trx, *_itin, "AA").empty());
}

TEST_F(ChargesUtilTest, testRetrieveS5_InterlinePass)
{
  S5Builder bld(&_mh);
  const S5Vec s5 = {
      bld.withFltTktMerchInd('C').withDesc("03").withSubCode("0GO").withIndustryCarrier().build()};
  AirSeg* as = _itin->travelSeg().back()->toAirSeg();
  as->setOperatingCarrierCode("BA");

  ON_CALL(*_dh, getSubCode(_, _, _, _, _)).WillByDefault(ReturnRef(EMPTY_S5));
  EXPECT_CALL(*_dh, getSubCode(_, _, _, _, _)).Times(AnyNumber());
  EXPECT_CALL(*_dh, getSubCode(ATP, AA, OC, BG, _)).Times(1).WillOnce(ReturnRef(s5));

  const auto& ret = ChargesUtil::retrieveS5(*_trx, *_itin, "AA");
  ASSERT_EQ(size_t(1), ret.size());
  ASSERT_EQ(s5.front(), ret.front());
}

TEST_F(ChargesUtilTest, testRetrieveS5_PassSort)
{
  S5Builder bld(&_mh);
  const S5Vec atpS5 = {bld.withFltTktMerchInd('C').withDesc("20").withSubCode("0AO").build(),
                       bld.withFltTktMerchInd('C').withDesc("10").withSubCode("0CO").build()};
  const S5Vec mmgrS5 = {bld.withFltTktMerchInd('C').withDesc("25").withSubCode("0DO").build(),
                        bld.withFltTktMerchInd('C').withDesc("15").withSubCode("0BO").build()};

  ON_CALL(*_dh, getSubCode(_, _, _, _, _)).WillByDefault(ReturnRef(EMPTY_S5));
  EXPECT_CALL(*_dh, getSubCode(_, _, _, _, _)).Times(AnyNumber());
  EXPECT_CALL(*_dh, getSubCode(ATP, AA, OC, BG, _)).Times(1).WillOnce(ReturnRef(atpS5));
  EXPECT_CALL(*_dh, getSubCode(MMGR, AA, OC, BG, _)).Times(1).WillOnce(ReturnRef(mmgrS5));

  const auto& ret = ChargesUtil::retrieveS5(*_trx, *_itin, "AA");
  ASSERT_EQ(size_t(4), ret.size());
  ASSERT_EQ(ServiceSubTypeCode("0AO"), ret[0]->serviceSubTypeCode());
  ASSERT_EQ(ServiceSubTypeCode("0BO"), ret[1]->serviceSubTypeCode());
  ASSERT_EQ(ServiceSubTypeCode("0CO"), ret[2]->serviceSubTypeCode());
  ASSERT_EQ(ServiceSubTypeCode("0DO"), ret[3]->serviceSubTypeCode());
}
}
}
