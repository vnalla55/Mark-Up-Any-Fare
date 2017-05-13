//----------------------------------------------------------------
//
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
//-----------------------------------------------------------------

#include "DataModel/FareMarket.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "Rules/TransfersStopoversUtil.h"

#include <gtest/gtest.h>
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

namespace tse
{

class TransfersStopoversUtilTest : public ::testing::Test
{
public:

  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    _req = _memHandle.create<PricingRequest>();
    _req->ticketingDT() = DateTime::localTime();
    _pro = _memHandle.create<PricingOptions>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_req);
    _trx->setOptions(_pro);
    _fm = _memHandle.create<FareMarket>();
    _fm->geoTravelType() = GeoTravelType::International;
    _fm->travelDate() = DateTime::localTime();
    _vendor = ATPCO_VENDOR_CODE;
  }

  void TearDown() { _memHandle.clear(); }

protected:

  struct MyDataHandle : public DataHandleMock
  {
    MyDataHandle()
    {
      aptToCity[LOC_EWR] = LOC_EWR;
      aptToCity["JFK"] = LOC_NYC;
    }

    const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                            const CarrierCode& carrier,
                                            GeoTravelType tvlType,
                                            const DateTime& tvlDate)
    {
      if (aptToCity.count(locCode))
        return aptToCity[locCode];
      return getMultiTransportCityCode(locCode, carrier, tvlType, tvlDate);
    }

  private:
    std::map<LocCode, LocCode> aptToCity;
  };

  const Loc* getLoc(const std::string& testLocFile)
  {
    return TestLocFactory::create("/vobs/atseintl/test/testdata/data/" + testLocFile);
  }

  const LocKey getLocKey(const LocTypeCode& locType, const LocCode& loc)
  {
    LocKey key;
    key.locType() = locType;
    key.loc() = loc;
    return key;
  }

  TestMemHandle _memHandle;
  PricingRequest* _req;
  PricingOptions* _pro;
  PricingTrx* _trx;
  FareMarket* _fm;
  VendorCode _vendor;
};

TEST_F(TransfersStopoversUtilTest, testGetCity_JFK)
{
  const Loc& ewr = *getLoc("LocJFK.xml");
  EXPECT_EQ(LOC_NYC, TransStopUtil::getCity(*_trx, _fm, ewr));
}

TEST_F(TransfersStopoversUtilTest, testGetCity_EWR)
{
  const Loc& ewr = *getLoc("LocEWR.xml");
  EXPECT_EQ(LOC_EWR, TransStopUtil::getCity(*_trx, _fm, ewr));
}

TEST_F(TransfersStopoversUtilTest, testGetCity_EWR_RTW)
{
  _pro->setRtw(true);
  const Loc& ewr = *getLoc("LocEWR.xml");
  EXPECT_EQ(LOC_NYC, TransStopUtil::getCity(*_trx, _fm, ewr));
}

TEST_F(TransfersStopoversUtilTest, testGetLocOfType_EWR)
{
  const Loc& ewr = *getLoc("LocEWR.xml");
  EXPECT_EQ(std::string("1"), TransStopUtil::getLocOfType(*_trx, _fm, _vendor, ewr, LOCTYPE_AREA));
  EXPECT_EQ(std::string("11"),
            TransStopUtil::getLocOfType(*_trx, _fm, _vendor, ewr, LOCTYPE_SUBAREA));
  EXPECT_EQ(std::string("USNJ"),
            TransStopUtil::getLocOfType(*_trx, _fm, _vendor, ewr, LOCTYPE_STATE));
  EXPECT_EQ(std::string("US"),
            TransStopUtil::getLocOfType(*_trx, _fm, _vendor, ewr, LOCTYPE_NATION));
  EXPECT_EQ(std::string("EWR"),
            TransStopUtil::getLocOfType(*_trx, _fm, _vendor, ewr, LOCTYPE_CITY));
  EXPECT_EQ(std::string("EWR"),
            TransStopUtil::getLocOfType(*_trx, _fm, _vendor, ewr, LOCTYPE_AIRPORT));
}

TEST_F(TransfersStopoversUtilTest, testGetLocOfType_EWR_RTW)
{
  _pro->setRtw(true);
  const Loc& ewr = *getLoc("LocEWR.xml");
  EXPECT_EQ(std::string("1"), TransStopUtil::getLocOfType(*_trx, _fm, _vendor, ewr, LOCTYPE_AREA));
  EXPECT_EQ(std::string("11"),
            TransStopUtil::getLocOfType(*_trx, _fm, _vendor, ewr, LOCTYPE_SUBAREA));
  EXPECT_EQ(std::string("USNJ"),
            TransStopUtil::getLocOfType(*_trx, _fm, _vendor, ewr, LOCTYPE_STATE));
  EXPECT_EQ(std::string("US"),
            TransStopUtil::getLocOfType(*_trx, _fm, _vendor, ewr, LOCTYPE_NATION));
  EXPECT_EQ(std::string("NYC"),
            TransStopUtil::getLocOfType(*_trx, _fm, _vendor, ewr, LOCTYPE_CITY));
  EXPECT_EQ(std::string("EWR"),
            TransStopUtil::getLocOfType(*_trx, _fm, _vendor, ewr, LOCTYPE_AIRPORT));
}

TEST_F(TransfersStopoversUtilTest, testIsInLoc_JFK_All)
{
  const Loc& jfk = *getLoc("LocJFK.xml");
  EXPECT_TRUE(TransStopUtil::isInLoc(*_trx, _vendor, jfk, getLocKey(LOCTYPE_AREA, IATA_AREA1)));
  EXPECT_TRUE(
      TransStopUtil::isInLoc(*_trx, _vendor, jfk, getLocKey(LOCTYPE_SUBAREA, IATA_SUB_AREA_11())));
  EXPECT_TRUE(TransStopUtil::isInLoc(*_trx, _vendor, jfk, getLocKey(LOCTYPE_NATION, NATION_US)));
  EXPECT_TRUE(TransStopUtil::isInLoc(*_trx, _vendor, jfk, getLocKey(LOCTYPE_STATE, "USNY")));
  EXPECT_TRUE(TransStopUtil::isInLoc(*_trx, _vendor, jfk, getLocKey(LOCTYPE_CITY, LOC_NYC)));
  EXPECT_TRUE(TransStopUtil::isInLoc(*_trx, _vendor, jfk, getLocKey(LOCTYPE_AIRPORT, "JFK")));
}

TEST_F(TransfersStopoversUtilTest, testIsInLoc_EWR_NYC)
{
  const Loc& ewr = *getLoc("LocEWR.xml");
  EXPECT_FALSE(TransStopUtil::isInLoc(*_trx, _vendor, ewr, getLocKey(LOCTYPE_CITY, LOC_NYC)));
}

TEST_F(TransfersStopoversUtilTest, testIsInLoc_EWR_NYC_RTW)
{
  _pro->setRtw(true);
  const Loc& ewr = *getLoc("LocEWR.xml");
  EXPECT_TRUE(TransStopUtil::isInLoc(*_trx, _vendor, ewr, getLocKey(LOCTYPE_CITY, LOC_NYC)));
}

}
