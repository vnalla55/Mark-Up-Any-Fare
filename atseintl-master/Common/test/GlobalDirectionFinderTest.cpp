//-------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"
#include "Common/GlobalDirectionFinder.h"
#include "Common/Global.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/SurfaceSeg.h"
#include "DBAccess/GlobalDir.h"
#include "DBAccess/GlobalDirSeg.h"
#include "DBAccess/Loc.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/testdata/TestLocFactory.h"

namespace tse
{
class GlobalDirectionFinderTest : public testing::Test
{
protected:
  typedef GlobalDirectionFinder::Location Location;

  class MyDataHandleMock : public DataHandleMock
  {
  public:
    const Loc* getLoc(const LocCode& locCode, const DateTime& date)
    {
      if (locCode == "DXB")
        return TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDXB.xml");
      if (locCode == "KHI")
        return TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocKHI.xml");

      return DataHandleMock::getLoc(locCode, date);
    }
  };

  std::vector<Location> _locations;
  std::set<CarrierCode> _carriers;
  PricingTrx* _trx;
  TestMemHandle _memHandle;
  MyDataHandleMock* _dataHandle;

  Location createLocation(const LocCode& locCode, bool isHidden)
  {
    return Location(_dataHandle->getLoc(locCode, DateTime::localTime()), isHidden);
  }

  bool rtwCall(Itin::TripCharacteristics tch, GlobalDirection& globalDir)
  {
    Itin itin;
    itin.tripCharacteristics().set(tch);
    _trx->itin().push_back(&itin);
    return GlobalDirectionFinder(_locations).setGlobalDirectionForRW(_trx, globalDir);
  }

  bool traverses3areas(const std::vector<Location>& locations)
  {
    return GlobalDirectionFinder(locations).traverses3areas();
  }

  size_t countTransfers(const std::vector<Location>& locations,
                        const IATAAreaCode& firstArea,
                        const IATAAreaCode& secondArea)
  {
    return GlobalDirectionFinder(locations).countTransfers(firstArea, secondArea);
  }

  bool applyRestrictionsforATglobal(const std::vector<Location>& locations)
  {
    return GlobalDirectionFinder(locations).applyRestrictionsforATglobal();
  }

  bool
  validateAllTrvOnCxrLogic(const std::set<CarrierCode>& carriers, const GlobalDirSeg& globalDir)
  {
    return GlobalDirectionFinder(_locations).validateAllTrvOnCxrLogic(carriers, globalDir);
  }

  bool
  validateMustBeViaLoc(const std::vector<Location>& locations, const GlobalDirSeg& globalDir) const
  {
    return GlobalDirectionFinder(locations).validateMustBeViaLoc(globalDir);
  }

  bool validateMustNotBeViaLoc(const std::vector<Location>& locations,
                               const GlobalDirSeg& globalDir) const
  {
    return GlobalDirectionFinder(locations).validateMustNotBeViaLoc(globalDir);
  }

  bool validateMustBeViaIntermediateLoc(const std::vector<Location>& locations,
                                        const GlobalDirSeg& globalDir) const
  {
    return GlobalDirectionFinder(locations).validateMustBeViaIntermediateLoc(globalDir);
  }

  bool validateMustNotBeViaIntermediateLoc(const std::vector<Location>& locations,
                                           const GlobalDirSeg& globalDir) const
  {
    return GlobalDirectionFinder(locations).validateMustNotBeViaIntermediateLoc(globalDir);
  }

  bool validateGlobalDir(const Location& origin,
                         const Location& dest,
                         const GlobalDirSeg& globalDir,
                         const bool withinOneAreaOnly) const
  {
    return GlobalDirectionFinder(_locations)
        .validateGlobalDir(origin, dest, globalDir, withinOneAreaOnly);
  }

public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _dataHandle = _memHandle.create<MyDataHandleMock>();

    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->setRequest(_memHandle.create<PricingRequest>());

    _locations.clear();
    _carriers.clear();
  }

  void TearDown() { _memHandle.clear(); }
};

TEST_F(GlobalDirectionFinderTest, testTraverses3areasWhenTravelIncludes2Areas)
{
  _locations.push_back(createLocation("BKK", false));
  _locations.push_back(createLocation("LON", false));
  _locations.push_back(createLocation("DXB", false));

  ASSERT_FALSE(traverses3areas(_locations));
}

TEST_F(GlobalDirectionFinderTest, testTraverses3areasWhenTravelIncludes3AreasIncludingHiddenStop)
{
  _locations.push_back(createLocation("AKL", false));
  _locations.push_back(createLocation("LAX", true));
  _locations.push_back(createLocation("LON", false));
  _locations.push_back(createLocation("DXB", false));

  ASSERT_TRUE(traverses3areas(_locations));
}

TEST_F(GlobalDirectionFinderTest, testTraverses3areasWhenTravelIncludes3Areas)
{
  _locations.push_back(createLocation("AKL", false));
  _locations.push_back(createLocation("LON", false));
  _locations.push_back(createLocation("DXB", false));
  ;
  _locations.push_back(createLocation("NYC", false));

  ASSERT_TRUE(traverses3areas(_locations));
}

TEST_F(GlobalDirectionFinderTest, testCountTransfers)
{
  _locations.push_back(createLocation("NYC", false));
  _locations.push_back(createLocation("AKL", false));
  _locations.push_back(createLocation("LAX", true));
  _locations.push_back(createLocation("LON", false));
  _locations.push_back(createLocation("DXB", false));
  _locations.push_back(createLocation("AKL", false));
  _locations.push_back(createLocation("NYC", false));

  ASSERT_EQ(3u, countTransfers(_locations, "1", "3"));
}

TEST_F(GlobalDirectionFinderTest, testRestrATglobalWhenTravelHasOneTransferFromA1ToA2)
{
  _locations.push_back(createLocation("YTO", false));
  _locations.push_back(createLocation("LON", false));
  _locations.push_back(createLocation("BKK", false));
  _locations.push_back(createLocation("DXB", false));

  ASSERT_TRUE(applyRestrictionsforATglobal(_locations));
}

TEST_F(GlobalDirectionFinderTest, testRestrATglobalWhenTravelHasDirectFlightFromA1ToSubA33)
{
  _locations.push_back(createLocation("YTO", false));
  _locations.push_back(createLocation("KHI", false));
  _locations.push_back(createLocation("DXB", false));

  ASSERT_TRUE(applyRestrictionsforATglobal(_locations));
}

TEST_F(GlobalDirectionFinderTest, testRestrATglobalWhenTravelHasDirectFlightFromSubA33ToA1)
{
  _locations.push_back(createLocation("DXB", false));
  _locations.push_back(createLocation("KHI", false));
  _locations.push_back(createLocation("YTO", false));

  ASSERT_TRUE(applyRestrictionsforATglobal(_locations));
}

TEST_F(GlobalDirectionFinderTest,
       testRestrATglobalWhenTravelHasNotTransferFromA1ToA2andDirectFlightFromA1ToSubA33)
{
  _locations.push_back(createLocation("DXB", false));
  _locations.push_back(createLocation("BKK", false));
  _locations.push_back(createLocation("YTO", false));

  ASSERT_FALSE(applyRestrictionsforATglobal(_locations));
}

TEST_F(GlobalDirectionFinderTest, testRWfail)
{
  GlobalDirection globalDir = GlobalDirection::XX;
  ASSERT_FALSE(rtwCall(Itin::OneWay, globalDir));
  ASSERT_EQ(GlobalDirection::XX, globalDir);
}

TEST_F(GlobalDirectionFinderTest, testRtwSfc)
{
  GlobalDirection globalDir = GlobalDirection::XX;
  ASSERT_TRUE(rtwCall(Itin::RW_SFC, globalDir));
  ASSERT_EQ(GlobalDirection::RW, globalDir);
}

TEST_F(GlobalDirectionFinderTest, testCtSfc)
{
  GlobalDirection globalDir = GlobalDirection::XX;
  ASSERT_TRUE(rtwCall(Itin::CT_SFC, globalDir));
  ASSERT_EQ(GlobalDirection::CT, globalDir);
}

TEST_F(GlobalDirectionFinderTest, testGetGlobalDirection_MOV_FRA_SEL_TYO_TS)
{
  _locations.push_back(createLocation("MOW", false));
  _locations.push_back(createLocation("FRA", false));
  _locations.push_back(createLocation("SEL", false));
  _locations.push_back(createLocation("TYO", false));

  _carriers.insert("AA");

  GlobalDirection globalDir;
  ASSERT_TRUE(GlobalDirectionFinder(_locations)
                  .getGlobalDirection(_trx, DateTime::localTime(), _carriers, globalDir));
  ASSERT_EQ(GlobalDirection::TS, globalDir);
}

TEST_F(GlobalDirectionFinderTest, testGetGlobalDirection_TYO_MOW_RU)
{
  _locations.push_back(createLocation("MOW", false));
  _locations.push_back(createLocation("TYO", false));

  _carriers.insert("AA");

  GlobalDirection globalDir;
  ASSERT_TRUE(GlobalDirectionFinder(_locations)
                  .getGlobalDirection(_trx, DateTime::localTime(), _carriers, globalDir));
  ASSERT_EQ(GlobalDirection::RU, globalDir);
}

TEST_F(GlobalDirectionFinderTest, testGetGlobalDirection_MIA_PAR_SIT_XX)
{
  _locations.push_back(createLocation("MIA", false));
  _locations.push_back(createLocation("PAR", false));
  _locations.push_back(createLocation("MIA", false));

  _carriers.insert("AA");

  GlobalDirection globalDir;
  ASSERT_TRUE(GlobalDirectionFinder(_locations)
                  .getGlobalDirection(_trx, DateTime::localTime(), _carriers, globalDir));
  ASSERT_EQ(GlobalDirection::XX, globalDir);
}

TEST_F(GlobalDirectionFinderTest, testGetGlobalDirection_TLS_PAR_PPT_AP)
{
  _locations.push_back(createLocation("TLS", false));
  _locations.push_back(createLocation("PAR", false));
  _locations.push_back(createLocation("LAX", true));
  _locations.push_back(createLocation("PPT", false));

  _carriers.insert("AA");

  GlobalDirection globalDir;
  ASSERT_TRUE(GlobalDirectionFinder(_locations)
                  .getGlobalDirection(_trx, DateTime::localTime(), _carriers, globalDir));
  ASSERT_EQ(GlobalDirection::AP, globalDir);
}

TEST_F(GlobalDirectionFinderTest, testGetGlobalDirection_BUE_JNB_BKK_SA)
{
  _locations.push_back(createLocation("BUE", false));
  _locations.push_back(createLocation("JNB", false));
  _locations.push_back(createLocation("BKK", false));

  _carriers.insert("AA");

  GlobalDirection globalDir;
  ASSERT_TRUE(GlobalDirectionFinder(_locations)
                  .getGlobalDirection(_trx, DateTime::localTime(), _carriers, globalDir));
  ASSERT_EQ(GlobalDirection::SA, globalDir);
}

TEST_F(GlobalDirectionFinderTest, testGetGlobalDirection_SVO_AMS_NRT_TS)
{
  _locations.push_back(createLocation("SVO", false));
  _locations.push_back(createLocation("AMS", false));
  _locations.push_back(createLocation("NRT", false));

  _carriers.insert("AA");

  GlobalDirection globalDir;
  ASSERT_TRUE(GlobalDirectionFinder(_locations)
                  .getGlobalDirection(_trx, DateTime::localTime(), _carriers, globalDir));
  ASSERT_EQ(GlobalDirection::TS, globalDir);
}

TEST_F(GlobalDirectionFinderTest, testGetGlobalDirection_TYO_SFO_PA)
{
  _locations.push_back(createLocation("TYO", false));
  _locations.push_back(createLocation("SFO", false));

  GlobalDirection globalDir;
  ASSERT_TRUE(GlobalDirectionFinder(_locations)
                  .getGlobalDirection(_trx, DateTime::localTime(), _carriers, globalDir));
  ASSERT_EQ(GlobalDirection::PA, globalDir);
}

TEST_F(GlobalDirectionFinderTest, testGetGlobalDirection_SurfaceSectorFix)
{
  _locations.push_back(createLocation("RUH", false));
  _locations.push_back(createLocation("CAI", false));
  _locations.push_back(createLocation("MIL", false));
  _locations.push_back(createLocation("MIA", false));

  _carriers.insert("AA");

  GlobalDirection globalDir;
  ASSERT_TRUE(GlobalDirectionFinder(_locations)
                  .getGlobalDirection(_trx, DateTime::localTime(), _carriers, globalDir));
  ASSERT_EQ(GlobalDirection::AT, globalDir);
}

TEST_F(GlobalDirectionFinderTest, testValidateGlobalDir)
{
  GlobalDirSeg gds;

  // ---------------------------------
  // Single area tests
  // ---------------------------------

  gds.directionality() = GlobalDirectionFinder::BETWEEN_TWO_AREAS;
  gds.loc1Type() = NATION;
  gds.loc1() = "US";

  ASSERT_FALSE(
      validateGlobalDir(createLocation("LAX", false), createLocation("FRA", false), gds, true));

  gds.directionality() = GlobalDirectionFinder::WITHIN_ONE_AREA;

  ASSERT_TRUE(
      validateGlobalDir(createLocation("LAX", false), createLocation("SFO", false), gds, true));

  // Test wrong nation, should fail
  gds.loc1() = "GB";
  ASSERT_FALSE(
      validateGlobalDir(createLocation("LAX", false), createLocation("SFO", false), gds, true));

  // Test wrong type, should fail
  gds.loc1() = "US";
  gds.loc1Type() = MARKET;
  ASSERT_FALSE(
      validateGlobalDir(createLocation("LAX", false), createLocation("SFO", false), gds, true));

  // ---------------------------------
  // Multiple area tests
  // ---------------------------------

  // BETWEEN_TWO_AREAS should match even if the cities are flipped
  // FROM_ONE_AREA should only match if the cities are correct
  //
  GlobalDirSeg gds2;

  Location origin = createLocation("DFW", false);
  Location dest = createLocation("MAN", false);

  gds2.loc1Type() = MARKET;
  gds2.loc1() = "DFW";

  gds2.loc2Type() = MARKET;
  gds2.loc2() = "MAN";

  gds2.directionality() = GlobalDirectionFinder::BETWEEN_TWO_AREAS;

  ASSERT_TRUE(validateGlobalDir(origin, dest, gds2, false));
  ASSERT_TRUE(validateGlobalDir(dest, origin, gds2, false));

  gds2.directionality() = GlobalDirectionFinder::FROM_ONE_AREA;

  ASSERT_TRUE(validateGlobalDir(origin, dest, gds2, false));
  ASSERT_FALSE(validateGlobalDir(dest, origin, gds2, false));
}

TEST_F(GlobalDirectionFinderTest, testValidateAllTrvOnCxrLogic)
{
  GlobalDirSeg gds;

  _carriers.insert("AA");

  // First make sure it fails when not set
  ASSERT_FALSE(validateAllTrvOnCxrLogic(_carriers, gds));

  gds.allTvlOnCarrier() = "AA";
  ASSERT_TRUE(validateAllTrvOnCxrLogic(_carriers, gds));

  gds.allTvlOnCarrier() = "BA";
  ASSERT_FALSE(validateAllTrvOnCxrLogic(_carriers, gds));

  // Now add another carrier to make sure it works correctly too
  _carriers.insert("AB");

  gds.allTvlOnCarrier() = "AA";
  ASSERT_FALSE(validateAllTrvOnCxrLogic(_carriers, gds));

  gds.allTvlOnCarrier() = "BA";
  ASSERT_FALSE(validateAllTrvOnCxrLogic(_carriers, gds));
}

TEST_F(GlobalDirectionFinderTest, testValidateMustBeViaLoc)
{
  GlobalDirSeg gds;

  _locations.push_back(createLocation("LAX", false));
  _locations.push_back(createLocation("SFO", false));
  _locations.push_back(createLocation("LON", false));
  _locations.push_back(createLocation("FRA", false));

  gds.mustBeViaLocType() = NATION;
  gds.mustBeViaLoc() = "GB";
  ASSERT_TRUE(validateMustBeViaLoc(_locations, gds));

  gds.mustBeViaLoc() = "XX";
  ASSERT_FALSE(validateMustBeViaLoc(_locations, gds));

  // @todo add tests for hidden stops
}

TEST_F(GlobalDirectionFinderTest, testValidateMustBeViaIntermediateLoc)
{
  GlobalDirSeg gds;

  _locations.push_back(createLocation("LAX", false));
  _locations.push_back(createLocation("SFO", false));
  _locations.push_back(createLocation("BKK", false));
  _locations.push_back(createLocation("FRA", false));

  gds.viaInterLoc1Type() = NATION;
  gds.viaInterLoc1() = "US";
  gds.viaInterLoc2Type() = NATION;
  gds.viaInterLoc2() = "TH";
  ASSERT_TRUE(validateMustBeViaIntermediateLoc(_locations, gds));

  gds.viaInterLoc1Type() = NATION;
  gds.viaInterLoc1() = "DE";
  gds.viaInterLoc2Type() = NATION;
  gds.viaInterLoc2() = "TH";
  ASSERT_TRUE(validateMustBeViaIntermediateLoc(_locations, gds));

  gds.viaInterLoc1Type() = IATA_AREA;
  gds.viaInterLoc1() = "3";
  gds.viaInterLoc2Type() = IATA_AREA;
  gds.viaInterLoc2() = "1";
  gds.directionality() = 'B';
  ASSERT_TRUE(validateMustBeViaIntermediateLoc(_locations, gds));

  gds.viaInterLoc1Type() = IATA_AREA;
  gds.viaInterLoc1() = "2";
  gds.viaInterLoc2Type() = IATA_AREA;
  gds.viaInterLoc2() = "2";
  ASSERT_FALSE(validateMustBeViaIntermediateLoc(_locations, gds));
}

TEST_F(GlobalDirectionFinderTest, testValidateMustNotBeViaLoc)
{
  GlobalDirSeg gds;

  _locations.push_back(createLocation("LAX", false));
  _locations.push_back(createLocation("SFO", false));
  _locations.push_back(createLocation("LON", false));
  _locations.push_back(createLocation("FRA", false));

  gds.mustNotBeViaLocType() = NATION;
  gds.mustNotBeViaLoc() = "GB";
  ASSERT_FALSE(validateMustNotBeViaLoc(_locations, gds));
}

TEST_F(GlobalDirectionFinderTest, testValidateMustNotBeViaIntermediateLoc)
{
  GlobalDirSeg gds;

  _locations.push_back(createLocation("LAX", false));
  _locations.push_back(createLocation("SFO", false));
  _locations.push_back(createLocation("LON", false));
  _locations.push_back(createLocation("FRA", false));

  gds.notViaInterLoc1Type() = NATION;
  gds.notViaInterLoc1() = "US";
  gds.notViaInterLoc2Type() = NATION;
  gds.notViaInterLoc2() = "GB";
  ASSERT_FALSE(validateMustNotBeViaIntermediateLoc(_locations, gds));

  gds.notViaInterLoc1Type() = NATION;
  gds.mustNotBeViaLocInd() = 'Y';
  gds.notViaInterLoc1() = "DE";
  gds.notViaInterLoc2Type() = NATION;
  gds.notViaInterLoc2() = "GB";
  ASSERT_FALSE(validateMustNotBeViaIntermediateLoc(_locations, gds));
}
}
