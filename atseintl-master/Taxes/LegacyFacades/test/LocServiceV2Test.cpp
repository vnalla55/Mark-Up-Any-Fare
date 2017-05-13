// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "test/include/GtestHelperMacros.h"

#include "Taxes/LegacyFacades/LocServiceV2.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Common/DateTime.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/Loc.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/AtpcoTaxes/Common/LocZone.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using testing::_;
using testing::Return;

class LocServiceV2Test : public testing::Test
{
protected:
  class IsInLocMock
  {
  public:
    MOCK_METHOD8(getValue,
                       bool(const Loc& city,
                            const LocTypeCode& locType,
                            const LocCode& locCode,
                            const VendorCode& vendor,
                            const ZoneType zoneType,
                            GeoTravelType geoTvlType,
                            const DateTime& ticketDate,
                            LocUtil::ApplicationType applType));

    bool operator()(const Loc& city,
                    const LocTypeCode& locType,
                    const LocCode& locCode,
                    const VendorCode& vendor,
                    const ZoneType zoneType,
                    GeoTravelType geoTvlType,
                    const DateTime& ticketDate,
                    LocUtil::ApplicationType applType)
    {
      return getValue(city, locType, locCode, vendor, zoneType, geoTvlType, ticketDate, applType);
    }
  };

  class MyDataHandle : public DataHandleMock
  {
  public:
    MOCK_METHOD2(getLoc, Loc*(const LocCode&, const DateTime&));
  };

public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _dataHandle = _memHandle.create<MyDataHandle>();
    DateTime date(2014, 1, 1);
    _date = _memHandle.create<DateTime>(date);

    _request = _memHandle.create<PricingRequest>();
    _request->ticketingDT() = *_date;

    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_request);

    _legacyMethodKeeper = _memHandle.create<IsInLocMock>();

    _locServiceV2 = _memHandle.create<LocServiceV2>(_trx->getRequest()->ticketingDT());
    _locServiceV2->_legacyIsInLoc =
        LocServiceV2::LegacyIsInLocFunction(boost::ref(*_legacyMethodKeeper));
  }

  void TearDown() { _memHandle.clear(); }


protected:
  bool toTse(char locTypeChar) const
  {
    tax::type::LocType locType = static_cast<tax::type::LocType>(locTypeChar);
    return (_locServiceV2->toTse(locType) != LocTypeCode());
  }

  TestMemHandle _memHandle;
  MyDataHandle* _dataHandle;
  DateTime* _date;
  PricingRequest* _request;
  PricingTrx* _trx;
  LocServiceV2* _locServiceV2;
  IsInLocMock* _legacyMethodKeeper;
};

TEST_F(LocServiceV2Test, testIsInLoc)
{
  tax::type::AirportCode locCode("LAX");
  Indicator tseLocType = 'N';
  LocCode nation = "US";
  tax::LocZone locZone;
  locZone.type() = tax::type::LocType::Nation;
  locZone.code() = toTaxLocZoneCode(nation);
  tse::VendorCode vendor("ATP");
  Loc loc;
  EXPECT_CALL(*_dataHandle, getLoc(toTseAirportCode(locCode), *_date)).WillOnce(Return(&loc));
  EXPECT_CALL(*_legacyMethodKeeper, getValue(_, tseLocType, nation, vendor, _, _, *_date, _))
      .WillOnce(Return(true));

  ASSERT_EQ(true, _locServiceV2->isInLoc(locCode, locZone, tse::toTaxVendorCode(vendor)));
}

TEST_F(LocServiceV2Test, testToTse)
{
  uint32_t successCounter = 0;
  for (char locTypeChar = ' '; locTypeChar <= 'Z'; locTypeChar++)
  {
    successCounter += toTse(locTypeChar);
  }

  ASSERT_EQ(9u, successCounter);
}

TEST_F(LocServiceV2Test, testMatchPassengerLocation)
{

}

} // namespace tse
