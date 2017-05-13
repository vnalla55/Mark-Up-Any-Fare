#include <boost/assign/std/vector.hpp>

#include "Common/Config/ConfigMan.h"
#include "Common/DateTime.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/ATPResNationZones.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MultiAirportCity.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestFactoryManager.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestZoneInfoFactory.h"

namespace tse
{
using boost::assign::operator+=;

class LocUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(LocUtilTest);
  CPPUNIT_TEST(testIsLocInZone);
  CPPUNIT_TEST(testGetMileage);
  CPPUNIT_TEST(testIsInLoc);
  CPPUNIT_TEST(testgetTPM);
  CPPUNIT_TEST(testOverloadedIsInLoc);
  CPPUNIT_TEST(testIsOverWater);
  CPPUNIT_TEST(testForeignDomesticInternational);
  CPPUNIT_TEST(testIsTransBorder);
  CPPUNIT_TEST(testIsScandinavia);
  CPPUNIT_TEST(testIsUSTerritoryGuam);
  CPPUNIT_TEST(testIsUSTerritoryBulgaria);
  CPPUNIT_TEST(testIsUSTerritoryUS);
  CPPUNIT_TEST(testIsMinFareUSTerritoryGuam);
  CPPUNIT_TEST(testIsMinFareUSTerritoryBulgaria);
  CPPUNIT_TEST(testIsMinFareUSTerritoryUS);
  CPPUNIT_TEST(testIsAirportInCity);
  CPPUNIT_TEST(testGetMultiCityAirports);
  CPPUNIT_TEST(testIsMultiAirport);
  CPPUNIT_TEST(testIsUSTerritoryRule);
  CPPUNIT_TEST(testIsTransAtlantic);
  CPPUNIT_TEST(testIsTransPacific);
  CPPUNIT_TEST(testIsTransOceanic);
  CPPUNIT_TEST(testGetOverWaterSegments1);
  CPPUNIT_TEST(testGetOverWaterSegments2);
  CPPUNIT_TEST(testGetInternationalSegments1);
  CPPUNIT_TEST(testGetInternationalSegments2);
  CPPUNIT_TEST(testIsInterContinental);
  CPPUNIT_TEST(testIsSameCity);
  CPPUNIT_TEST(testIsRTWOrigArea1);
  CPPUNIT_TEST(testIsRTWOrigArea2Or3);
  CPPUNIT_TEST(testGetContinentalZoneCode_000_forUS);
  CPPUNIT_TEST(testGetContinentalZoneCode_000_forCA);
  CPPUNIT_TEST(testGetContinentalZoneCode_000_forHawaii);
  CPPUNIT_TEST(testGetContinentalZoneCode_000_forMX);
  CPPUNIT_TEST(testGetContinentalZoneCode_170_forBazil);
  CPPUNIT_TEST(testGetContinentalZoneCode_210_forGemany);
  CPPUNIT_TEST(testGetContinentalZoneCode_210_forSpain);
  CPPUNIT_TEST(testGetContinentalZoneCode_210_forSweden);
  CPPUNIT_TEST(testGetContinentalZoneCode_210_forRussiaWestOfUrals);
  CPPUNIT_TEST(testGetContinentalZoneCode_220_forIran);
  CPPUNIT_TEST(testGetContinentalZoneCode_230_forKenya);
  CPPUNIT_TEST(testGetContinentalZoneCode_310_forEastAsia);
  CPPUNIT_TEST(testGetContinentalZoneCode_320_forChinaAndHkg);
  CPPUNIT_TEST(testGetContinentalZoneCode_330_forIndia);
  CPPUNIT_TEST(testGetContinentalZoneCode_340_forAustralia);
  CPPUNIT_TEST(testGetContinentalZoneCode_350_forCanton);
  CPPUNIT_TEST(testGetContinentalZoneCode_360_forRussiaEastOfUrals);
  CPPUNIT_TEST(testIsFormerNetherlandsAntillesUS);
  CPPUNIT_TEST(testIsFormerNetherlandsAntillesSX);
  CPPUNIT_TEST(testIsWholeTravelInUSTerritory_WholeInUS);
  CPPUNIT_TEST(testIsWholeTravelInUSTerritory_PartInUS);
  CPPUNIT_TEST(testIsWholeTravelInUSTerritory_NoneInUS);
  CPPUNIT_TEST(testIsCityInSpanishOverseaIslands);
  CPPUNIT_TEST(testIsSameMultiTransportCity_PASS);
  CPPUNIT_TEST(testIsSameMultiTransportCity_FAIL);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    // Cleanup cache between runs
    _mdh = _memHandle.create<MyDataHandle>();
    TestFactoryManager::instance()->destroyAll();
  }

  void tearDown()
  {
    TestFactoryManager::instance()->destroyAll();
    _memHandle.clear();
    // Make sure it is cleaned before other tests run
  }

  TravelSeg* createAirSeg(const NationCode& originNation, const NationCode& destNation)
  {
    TravelSeg* travelSeg = _memHandle.create<AirSeg>();
    Loc* origin = _memHandle.create<Loc>();
    Loc* destination = _memHandle.create<Loc>();

    travelSeg->origin() = origin;
    travelSeg->destination() = destination;

    origin->nation() = originNation;
    destination->nation() = destNation;

    return travelSeg;
  }

  void testIsInLoc()
  {
    Loc* dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");

    // -------------------------------------------------
    // Test everything except zone
    // -------------------------------------------------
    CPPUNIT_ASSERT(LocUtil::isInLoc(*dfw, LOCTYPE_AREA, "1"));
    CPPUNIT_ASSERT(LocUtil::isInLoc(*dfw, LOCTYPE_SUBAREA, "11"));
    CPPUNIT_ASSERT(LocUtil::isInLoc(*dfw, LOCTYPE_CITY, "DFW"));
    CPPUNIT_ASSERT(LocUtil::isInLoc(*dfw, LOCTYPE_AIRPORT, "DFW") == true);
    CPPUNIT_ASSERT(LocUtil::isInLoc(*dfw, LOCTYPE_NATION, "US"));
    CPPUNIT_ASSERT(LocUtil::isInLoc(*dfw, LOCTYPE_STATE, "USTX"));

    dfw->cityInd() = false;

    CPPUNIT_ASSERT(LocUtil::isInLoc(*dfw, LOCTYPE_AREA, "X") == false);
    CPPUNIT_ASSERT(LocUtil::isInLoc(*dfw, LOCTYPE_SUBAREA, "X") == false);
    CPPUNIT_ASSERT(LocUtil::isInLoc(*dfw, LOCTYPE_CITY, "DFW") == true);
    CPPUNIT_ASSERT(LocUtil::isInLoc(*dfw, LOCTYPE_AIRPORT, "DFW"));
    CPPUNIT_ASSERT(LocUtil::isInLoc(*dfw, LOCTYPE_NATION, "X") == false);

    CPPUNIT_ASSERT(LocUtil::isInLoc(*dfw, LOCTYPE_STATE, "XXXX") == false);

    // -------------------------------------------------
    // Now do the zone tests
    // -------------------------------------------------

    Loc locZoneCA;
    locZoneCA.loc() = "YYZ";
    locZoneCA.state() = "CAON";
    locZoneCA.nation() = "CA";
    locZoneCA.subarea() = "11";
    locZoneCA.area() = "1";

    Loc* locZoneUS = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");

    // SABR Manual Zone 1 has only CA, Zone 2 has CA and US

    CPPUNIT_ASSERT(LocUtil::isInLoc(locZoneCA, LOCTYPE_ZONE, "1", Vendor::SABRE, MANUAL) == true);
    CPPUNIT_ASSERT(LocUtil::isInLoc(*locZoneUS, LOCTYPE_ZONE, "1", Vendor::SABRE, MANUAL) == false);

    CPPUNIT_ASSERT(LocUtil::isInLoc(locZoneCA, LOCTYPE_ZONE, "2", Vendor::SABRE, MANUAL) == true);
    CPPUNIT_ASSERT(LocUtil::isInLoc(*locZoneUS, LOCTYPE_ZONE, "2", Vendor::SABRE, MANUAL) == true);

    // Reserved Zone 1 is US except Hawaii, Alaska, Puerto Rico, and Virgin Islands
    Loc locZoneHI;
    locZoneHI.loc() = "HNL";

    CPPUNIT_ASSERT(LocUtil::isInLoc(locZoneCA, LOCTYPE_ZONE, "1", Vendor::ATPCO) == false);
    CPPUNIT_ASSERT(LocUtil::isInLoc(*locZoneUS, LOCTYPE_ZONE, "1", Vendor::ATPCO) == true);
    CPPUNIT_ASSERT(LocUtil::isInLoc(locZoneHI, LOCTYPE_ZONE, "1", Vendor::ATPCO) == false);

    // Vendor must be specified for a zone lookup
    CPPUNIT_ASSERT(LocUtil::isInLoc(*locZoneUS, LOCTYPE_ZONE, "1", "") == false);
    // Market must be specified for a zone lookup
    locZoneUS->loc() = "";
    CPPUNIT_ASSERT(LocUtil::isInLoc(*locZoneUS, LOCTYPE_ZONE, "1", Vendor::ATPCO) == false);

    //    Test for the Residency_city.
    //    (LocUtil::OTHER --> good result)

    // 1.
    LocCode lc = "PAR";

    // PASS
    CPPUNIT_ASSERT(LocUtil::isInLoc(lc, LOCTYPE_AREA, "2"));
    CPPUNIT_ASSERT(LocUtil::isInLoc(lc, LOCTYPE_CITY, "PAR"));

    CPPUNIT_ASSERT(
        LocUtil::isInLoc(
            lc, LOCTYPE_ZONE, "210", Vendor::ATPCO, RESERVED, GeoTravelType::International, LocUtil::OTHER) ==
        true);
    CPPUNIT_ASSERT(
        LocUtil::isInLoc(
            lc, LOCTYPE_NATION, "FR", Vendor::ATPCO, RESERVED, GeoTravelType::International, LocUtil::OTHER) ==
        true);
    // FAIL
    CPPUNIT_ASSERT(
        LocUtil::isInLoc(
            lc, LOCTYPE_CITY, "CDG", Vendor::ATPCO, RESERVED, GeoTravelType::International, LocUtil::OTHER) ==
        false);

    CPPUNIT_ASSERT(LocUtil::isInLoc(lc,
                                    LOCTYPE_CITY,
                                    "CDG",
                                    Vendor::ATPCO,
                                    RESERVED,
                                    GeoTravelType::International,
                                    LocUtil::RECORD1_2_6) == false);

    // 2.
    LocCode lc1 = "ORY";

    // PASS
    CPPUNIT_ASSERT(LocUtil::isInLoc(lc1, LOCTYPE_AREA, "2"));
    CPPUNIT_ASSERT(LocUtil::isInLoc(lc1, LOCTYPE_CITY, "ORY"));
    CPPUNIT_ASSERT(LocUtil::isInLoc(lc1, LOCTYPE_CITY, "PAR"));

    // do not use LocUtil::RECORD1_2_6  for the PAX residency_city logic !
    CPPUNIT_ASSERT(LocUtil::isInLoc(lc1,
                                    LOCTYPE_CITY,
                                    "PAR",
                                    Vendor::ATPCO,
                                    RESERVED,
                                    GeoTravelType::International,
                                    LocUtil::RECORD1_2_6) == false);

    CPPUNIT_ASSERT(
        LocUtil::isInLoc(
            lc1, LOCTYPE_ZONE, "210", Vendor::ATPCO, RESERVED, GeoTravelType::International, LocUtil::OTHER) ==
        true);
    CPPUNIT_ASSERT(
        LocUtil::isInLoc(
            lc1, LOCTYPE_NATION, "FR", Vendor::ATPCO, RESERVED, GeoTravelType::International, LocUtil::OTHER) ==
        true);
    // FAIL:
    CPPUNIT_ASSERT(
        LocUtil::isInLoc(
            lc1, LOCTYPE_CITY, "CDG", Vendor::ATPCO, RESERVED, GeoTravelType::International, LocUtil::OTHER) ==
        false);

    CPPUNIT_ASSERT(
        LocUtil::isInLoc(
            lc1, LOCTYPE_CITY, "CDG", Vendor::ATPCO, RESERVED, GeoTravelType::Domestic, LocUtil::OTHER) == false);

    CPPUNIT_ASSERT(LocUtil::isInLoc(lc1,
                                    LOCTYPE_CITY,
                                    "CDG",
                                    Vendor::ATPCO,
                                    RESERVED,
                                    GeoTravelType::International,
                                    LocUtil::RECORD1_2_6) == false);
    // 3.
    LocCode lc2 = "CDG";
    CPPUNIT_ASSERT(LocUtil::isInLoc(lc2,
                                    LOCTYPE_CITY,
                                    "ORY",
                                    Vendor::ATPCO,
                                    RESERVED,
                                    GeoTravelType::International,
                                    LocUtil::RECORD1_2_6) == false);
    CPPUNIT_ASSERT(LocUtil::isInLoc(lc2, LOCTYPE_CITY, "PAR"));

    // 4.  vendor=SABR
    lc = "PAR";

    // PASS
    CPPUNIT_ASSERT(LocUtil::isInLoc(lc, LOCTYPE_AREA, "2"));
    CPPUNIT_ASSERT(LocUtil::isInLoc(lc, LOCTYPE_CITY, "PAR"));

    CPPUNIT_ASSERT(
        LocUtil::isInLoc(
            lc, LOCTYPE_ZONE, "210", Vendor::SABRE, RESERVED, GeoTravelType::International, LocUtil::OTHER) ==
        true);
    CPPUNIT_ASSERT(
        LocUtil::isInLoc(
            lc, LOCTYPE_NATION, "FR", Vendor::SABRE, RESERVED, GeoTravelType::International, LocUtil::OTHER) ==
        true);
    // FAIL
    CPPUNIT_ASSERT(
        LocUtil::isInLoc(
            lc, LOCTYPE_CITY, "CDG", Vendor::SABRE, RESERVED, GeoTravelType::International, LocUtil::OTHER) ==
        false);

    CPPUNIT_ASSERT(LocUtil::isInLoc(lc,
                                    LOCTYPE_CITY,
                                    "CDG",
                                    Vendor::SABRE,
                                    RESERVED,
                                    GeoTravelType::International,
                                    LocUtil::RECORD1_2_6) == false);

    // 5.
    lc1 = "ORY";

    // PASS
    CPPUNIT_ASSERT(LocUtil::isInLoc(lc1, LOCTYPE_AREA, "2"));
    CPPUNIT_ASSERT(LocUtil::isInLoc(lc1, LOCTYPE_CITY, "ORY"));
    CPPUNIT_ASSERT(LocUtil::isInLoc(lc1, LOCTYPE_CITY, "PAR"));

    // do not use LocUtil::RECORD1_2_6  for the PAX residency_city logic !
    CPPUNIT_ASSERT(LocUtil::isInLoc(lc1,
                                    LOCTYPE_CITY,
                                    "PAR",
                                    Vendor::SABRE,
                                    RESERVED,
                                    GeoTravelType::International,
                                    LocUtil::RECORD1_2_6) == false);

    CPPUNIT_ASSERT(
        LocUtil::isInLoc(
            lc1, LOCTYPE_ZONE, "210", Vendor::SABRE, RESERVED, GeoTravelType::International, LocUtil::OTHER) ==
        true);
    CPPUNIT_ASSERT(
        LocUtil::isInLoc(
            lc1, LOCTYPE_NATION, "FR", Vendor::SABRE, RESERVED, GeoTravelType::International, LocUtil::OTHER) ==
        true);
    // FAIL:
    CPPUNIT_ASSERT(
        LocUtil::isInLoc(
            lc1, LOCTYPE_CITY, "CDG", Vendor::SABRE, RESERVED, GeoTravelType::International, LocUtil::OTHER) ==
        false);

    CPPUNIT_ASSERT(
        LocUtil::isInLoc(
            lc1, LOCTYPE_CITY, "CDG", Vendor::SABRE, RESERVED, GeoTravelType::Domestic, LocUtil::OTHER) == false);

    CPPUNIT_ASSERT(LocUtil::isInLoc(lc1,
                                    LOCTYPE_CITY,
                                    "CDG",
                                    Vendor::SABRE,
                                    RESERVED,
                                    GeoTravelType::International,
                                    LocUtil::RECORD1_2_6) == false);
  }

  void testOverloadedIsInLoc()
  {

    // -------------------------------------------------
    // Test everything except zone
    // -------------------------------------------------

    CPPUNIT_ASSERT(LocUtil::isInLoc("DFW", LOCTYPE_AREA, "1"));
    CPPUNIT_ASSERT(LocUtil::isInLoc("DFW", LOCTYPE_SUBAREA, "11"));
    CPPUNIT_ASSERT(LocUtil::isInLoc("DFW", LOCTYPE_CITY, "DFW"));
    //    CPPUNIT_ASSERT( LocUtil::isInLoc( "DFW", LOCTYPE_AIRPORT, "DFW") == false);
    CPPUNIT_ASSERT(LocUtil::isInLoc("DFW", LOCTYPE_AIRPORT, "DFW") == true);
    CPPUNIT_ASSERT(LocUtil::isInLoc("DFW", LOCTYPE_NATION, "US"));
    CPPUNIT_ASSERT(LocUtil::isInLoc("DFW", LOCTYPE_STATE, "USTX"));

    CPPUNIT_ASSERT(LocUtil::isInLoc("DFW", LOCTYPE_AREA, "X") == false);
    // test Zone
    CPPUNIT_ASSERT(LocUtil::isInLoc("YVR", LOCTYPE_ZONE, "1", Vendor::SABRE, MANUAL) == true);
    CPPUNIT_ASSERT(LocUtil::isInLoc("LAX", LOCTYPE_ZONE, "1", Vendor::SABRE, MANUAL) == false);
    // check error condition : database cant find the Loc

    CPPUNIT_ASSERT(LocUtil::isInLoc("", LOCTYPE_AREA, "1") == false);

    CPPUNIT_ASSERT(!LocUtil::isInLoc("BCN", LOCTYPE_ZONE, "1235", Vendor::SABRE, MANUAL));
    CPPUNIT_ASSERT(!LocUtil::isInLoc("HNL", LOCTYPE_ZONE, "0000001", Vendor::ATPCO, RESERVED));
  }

  void testGetMileage()
  {
    DataHandle dataHandle;
    const Loc* mnl = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMNL.xml");
    const Loc* loc2 = dataHandle.getLoc("NCL", DateTime::localTime());
    uint16_t mil =
        LocUtil::getTPM(*mnl, *loc2, GlobalDirection::EH, DateTime::localTime(), dataHandle);

    CPPUNIT_ASSERT_EQUAL((uint16_t)7700, mil);
  }

  void testIsTransBorder()
  {
    const Loc* dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    const Loc* yvr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYVR.xml");
    const Loc* yyz = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYYZ.xml");
    const Loc* yto = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYTO.xml");
    bool rc = LocUtil::isTransBorder(*dfw, *yvr);
    CPPUNIT_ASSERT(rc);
    rc = LocUtil::isTransBorder(*yyz, *yvr);
    CPPUNIT_ASSERT(!rc);
    rc = LocUtil::isTransBorder(*yyz, *dfw);
    CPPUNIT_ASSERT(rc);
    rc = LocUtil::isTransBorder(*yto, *dfw);
    CPPUNIT_ASSERT(rc);

    rc = LocUtil::isTransBorder(*dfw, *yto);
    CPPUNIT_ASSERT(rc);
    rc = LocUtil::isDomestic(*yyz, *dfw);
    CPPUNIT_ASSERT(!rc);
    rc = LocUtil::isUSTerritory(*yyz, *dfw);
    CPPUNIT_ASSERT(rc);
  }

  void testIsOverWater()
  {
    Loc loc1;
    Loc loc2;
    // case1 : orgin dfw to alaska
    loc1.nation() = "US";
    loc1.state() = "USLA";
    loc2.nation() = "US";
    loc2.state() = ALASKA;
    bool rc = LocUtil::isOverWater(loc1, loc2);
    CPPUNIT_ASSERT(rc);
    // origin lax to hawai
    loc1.nation() = "US";
    loc2.state() = "USLA";
    loc2.nation() = "US";
    loc1.state() = HAWAII;
    rc = LocUtil::isOverWater(loc1, loc2);
    CPPUNIT_ASSERT(rc);
    // from alaska to hawai
    loc1.nation() = "US";
    loc2.state() = ALASKA;
    loc2.nation() = "US";
    loc1.state() = HAWAII;
    rc = LocUtil::isOverWater(loc1, loc2);
    CPPUNIT_ASSERT(rc);

    // from alaska to puertorico
    loc1.nation() = "US";
    loc1.state() = PUERTORICO;
    loc2.nation() = "US";
    loc2.state() = ALASKA;
    rc = LocUtil::isOverWater(loc1, loc2);
    CPPUNIT_ASSERT(rc);

    loc2.state() = VIRGIN_ISLAND;
    rc = LocUtil::isOverWater(loc1, loc2);
    CPPUNIT_ASSERT(!rc);

    loc1.state() = PUERTORICO;
    rc = LocUtil::isOverWater(loc1, loc2);
    CPPUNIT_ASSERT(!rc);
    // bad data
    loc1.nation() = "US";
    loc1.state() = "";
    loc2.nation() = "US";
    loc2.state() = "";
    rc = LocUtil::isOverWater(loc1, loc2);
    CPPUNIT_ASSERT(!rc);
    // from alaska to any other state
    loc1.nation() = "US";
    loc1.state() = "USZZ";
    loc2.nation() = "ABC";
    loc2.state() = ALASKA;
    rc = LocUtil::isOverWater(loc1, loc2);
    CPPUNIT_ASSERT(!rc);

    // case2
    // from canad to alaska
    loc1.nation() = "CA";
    loc1.state() = "CABC";
    loc2.nation() = "US";
    loc1.state() = PUERTORICO;
    rc = LocUtil::isOverWater(loc1, loc2);
    CPPUNIT_ASSERT(rc);

    // from canada to HAWAII
    loc1.nation() = "CA";
    loc1.state() = "ON";
    loc2.nation() = "US";
    loc2.state() = HAWAII;
    rc = LocUtil::isOverWater(loc1, loc2);
    CPPUNIT_ASSERT(rc);

    // from bangkok to hawai
    loc1.nation() = "BK";
    loc1.state() = "BKLL";
    loc1.area() = "3";
    loc2.nation() = "US";
    loc2.state() = "USCA";
    loc2.area() = "1";
    rc = LocUtil::isOverWater(loc1, loc2);
    CPPUNIT_ASSERT(rc);

    // from lax to lon
    loc1.nation() = "US";
    loc1.area() = "1";
    loc2.nation() = "GB";
    loc1.area() = "2";
    rc = LocUtil::isOverWater(loc1, loc2);
    CPPUNIT_ASSERT(rc);
    // from Lon to Man
    loc1.nation() = "GB";
    loc1.area() = "2";
    loc2.nation() = "GB";
    loc1.area() = "2";
    rc = LocUtil::isOverWater(loc1, loc2);
    CPPUNIT_ASSERT(!rc);

    DataHandle dataHandle;
    const Loc* dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    const Loc* mex = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEX.xml");
    const Loc* gig = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocGIG.xml");
    const Loc* hav = dataHandle.getLoc("HAV", DateTime::localTime());
    const Loc* pap = dataHandle.getLoc("PAP", DateTime::localTime());
    const Loc* yvi = dataHandle.getLoc("YVI", DateTime::localTime());
    const Loc* mia = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMIA.xml");

    CPPUNIT_ASSERT(LocUtil::isOverWater(*dfw, *hav));
    CPPUNIT_ASSERT(LocUtil::isOverWater(*dfw, *pap)); // Port Au Prince,Haiti
    CPPUNIT_ASSERT(LocUtil::isOverWater(*dfw, *gig));
    CPPUNIT_ASSERT(LocUtil::isOverWater(*dfw, *yvi)); // San Jose,Costa Rica

    CPPUNIT_ASSERT(LocUtil::isOverWater(*hav, *yvi)); // Havana,Cuba to San Jose
    CPPUNIT_ASSERT(LocUtil::isOverWater(*yvi, *hav));
    CPPUNIT_ASSERT(LocUtil::isOverWater(*hav, *gig)); // Havana to Rio de Janeiro
    CPPUNIT_ASSERT(LocUtil::isOverWater(*gig, *hav));

    CPPUNIT_ASSERT(!LocUtil::isOverWater(*gig, *yvi)); // Rio to San Jose

    CPPUNIT_ASSERT(!LocUtil::isOverWater(*dfw, *mex));
    CPPUNIT_ASSERT(!LocUtil::isOverWater(*mia, *mex));
    CPPUNIT_ASSERT(!LocUtil::isOverWater(*mex, *mia));
  }

  void testForeignDomesticInternational()
  {
    Loc loc1;
    Loc loc2;
    // isDomestic
    loc1.nation() = "US";
    loc2.nation() = "US";
    bool rc = LocUtil::isDomestic(loc1, loc2);
    CPPUNIT_ASSERT(rc);
    // isUSTerritory
    rc = LocUtil::isUSTerritory(loc1, loc2);
    CPPUNIT_ASSERT(rc);
    loc1.nation() = "CA";
    loc2.nation() = "GU";
    LocUtil::isUSTerritory(loc1, loc2);
    CPPUNIT_ASSERT(rc);
    // isDomestic() to Fail
    loc2.nation() = "BG";
    loc1.nation() = "CA";
    rc = LocUtil::isDomestic(loc1, loc2);
    CPPUNIT_ASSERT(!rc);
    // isForeignDomestic()
    loc1.nation() = "BK";
    loc2.nation() = "BK";
    rc = LocUtil::isForeignDomestic(loc1, loc2);
    CPPUNIT_ASSERT(rc);
    loc1.nation() = "SI";
    // isForeignDomestic() to Fail
    rc = LocUtil::isForeignDomestic(loc1, loc2);
    CPPUNIT_ASSERT(!rc);
  }

  void testIsUSTerritoryGuam()
  {
    Loc loc1;
    Loc loc2;
    loc1.nation() = "CA";
    loc2.nation() = "GU";
    CPPUNIT_ASSERT(LocUtil::isUSTerritory(loc1, loc2));
  }

  void testIsUSTerritoryBulgaria()
  {
    Loc loc1;
    Loc loc2;
    loc2.nation() = "BG";
    loc1.nation() = "CA";
    CPPUNIT_ASSERT(!LocUtil::isUSTerritory(loc1, loc2));
  }

  void testIsUSTerritoryUS()
  {
    Loc loc1;
    Loc loc2;
    loc1.nation() = "US";
    loc2.nation() = "US";
    CPPUNIT_ASSERT(LocUtil::isUSTerritory(loc1, loc2));
  }

  void testIsMinFareUSTerritoryGuam()
  {
    Loc loc;
    loc.nation() = "GU";
    CPPUNIT_ASSERT(LocUtil::isMinFareUSTerritory(loc));
  }

  void testIsMinFareUSTerritoryBulgaria()
  {
    Loc loc;
    loc.nation() = "CA";
    CPPUNIT_ASSERT(!LocUtil::isMinFareUSTerritory(loc));
  }

  void testIsMinFareUSTerritoryUS()
  {
    Loc loc;
    loc.nation() = "US";
    CPPUNIT_ASSERT(LocUtil::isMinFareUSTerritory(loc));
  }

  void testIsScandinavia()
  {
    Loc loc1;
    Loc loc2;
    // isScandinavia() to Fail
    loc1.nation() = "US";
    loc2.nation() = "US";
    bool rc = LocUtil::isScandinavia(loc1);
    CPPUNIT_ASSERT(!rc);
    loc1.nation() = "DK";
    rc = LocUtil::isScandinavia(loc1);
    CPPUNIT_ASSERT(rc);
    // isScandinavia()
    loc1.nation() = "NO";
    rc = LocUtil::isScandinavia(loc1);
    CPPUNIT_ASSERT(rc);
    // Bad Data
    loc1.nation() = "";
    rc = LocUtil::isScandinavia(loc1);
    CPPUNIT_ASSERT(!rc);
  }

  void testIsAirportInCity()
  {
    LocCode dfw = "DFW";
    LocCode jfk = "JFK";
    LocCode lga = "LGA";
    LocCode nyc = "NYC";

    bool rc = LocUtil::isAirportInCity(dfw, nyc);
    CPPUNIT_ASSERT(!rc);

    rc = LocUtil::isAirportInCity(jfk, nyc);
    CPPUNIT_ASSERT(rc);

    rc = LocUtil::isAirportInCity(lga, nyc);
    CPPUNIT_ASSERT(rc);
  }

  void testGetMultiCityAirports()
  {
    LocCode nyc = "NYC";
    const std::vector<tse::MultiAirportCity*>& multiAirportityList =
        LocUtil::getMultiCityAirports(nyc);
    CPPUNIT_ASSERT(!multiAirportityList.empty());
  }

  void testIsMultiAirport()
  {
    LocCode nyc = "NYC";
    bool rc = LocUtil::isMultiAirport(nyc);
    CPPUNIT_ASSERT(rc);
  }

  void testIsUSTerritoryRule()
  {
    Loc loc;

    // isUSTerritoryRule
    loc.nation() = "CA";
    bool rc = LocUtil::isUSTerritoryRule(loc);
    CPPUNIT_ASSERT(rc);
    // isUSTerritoryRule() to Fail
    loc.nation() = "GB";
    rc = LocUtil::isUSTerritoryRule(loc);
    CPPUNIT_ASSERT(!rc);
    // isUSTerritoryRule()
    loc.nation() = "US";
    rc = LocUtil::isUSTerritoryRule(loc);
    CPPUNIT_ASSERT(rc);
    // isUSTerritoryRule()
    loc.nation() = "GU";
    rc = LocUtil::isUSTerritoryRule(loc);
    CPPUNIT_ASSERT(rc);
  }

  void testgetTPM()
  {
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("RES", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("ASU", DateTime::localTime());
    uint32_t mil =
        LocUtil::getTPM(*loc1, *loc2, GlobalDirection::WH, DateTime::localTime(), dataHandle);
    CPPUNIT_ASSERT_EQUAL((uint32_t)178, mil);
  }

  void testIsTransAtlantic()
  {
    Loc loc1;
    Loc loc2;

    // Within IATA Area 1
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(!LocUtil::isTransAtlantic(loc1, loc2, GlobalDirection::US));

    // Within IATA Area 2
    loc1.area() = IATA_AREA2;
    loc2.area() = IATA_AREA2;
    CPPUNIT_ASSERT(!LocUtil::isTransAtlantic(loc1, loc2, GlobalDirection::TT));

    // Within IATA Area 3
    loc1.area() = IATA_AREA3;
    loc2.area() = IATA_AREA3;
    CPPUNIT_ASSERT(!LocUtil::isTransAtlantic(loc1, loc2, GlobalDirection::RU));

    // IATA Area 1 to 2 via Atlantic
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA2;
    CPPUNIT_ASSERT(LocUtil::isTransAtlantic(loc1, loc2, GlobalDirection::AT));

    // IATA Area 2 to 1 via Atlantic
    loc1.area() = IATA_AREA2;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(LocUtil::isTransAtlantic(loc1, loc2, GlobalDirection::AT));

    // IATA Area 1 to 2 via North Pacific
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA2;
    CPPUNIT_ASSERT(!LocUtil::isTransAtlantic(loc1, loc2, GlobalDirection::NP));

    // IATA Area 2 to 1 via North Pacific
    loc1.area() = IATA_AREA2;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(!LocUtil::isTransAtlantic(loc1, loc2, GlobalDirection::NP));

    // IATA Area 1 to 3 via Atlantic
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA3;
    CPPUNIT_ASSERT(LocUtil::isTransAtlantic(loc1, loc2, GlobalDirection::AT));

    // IATA Area 3 to 1 via Atlantic
    loc1.area() = IATA_AREA3;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(LocUtil::isTransAtlantic(loc1, loc2, GlobalDirection::AT));

    // IATA Area 1 to 3 via North Pacific
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA3;
    CPPUNIT_ASSERT(!LocUtil::isTransAtlantic(loc1, loc2, GlobalDirection::NP));

    // IATA Area 3 to 1 via North Pacific
    loc1.area() = IATA_AREA3;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(!LocUtil::isTransAtlantic(loc1, loc2, GlobalDirection::NP));
  }

  void testIsTransPacific()
  {
    Loc loc1;
    Loc loc2;

    // Within IATA Area 1
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(!LocUtil::isTransPacific(loc1, loc2, GlobalDirection::US));

    // Within IATA Area 2
    loc1.area() = IATA_AREA2;
    loc2.area() = IATA_AREA2;
    CPPUNIT_ASSERT(!LocUtil::isTransPacific(loc1, loc2, GlobalDirection::TT));

    // Within IATA Area 3
    loc1.area() = IATA_AREA3;
    loc2.area() = IATA_AREA3;
    CPPUNIT_ASSERT(!LocUtil::isTransPacific(loc1, loc2, GlobalDirection::RU));

    // IATA Area 1 to 2 via Atlantic
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA2;
    CPPUNIT_ASSERT(!LocUtil::isTransPacific(loc1, loc2, GlobalDirection::AT));

    // IATA Area 2 to 1 via Atlantic
    loc1.area() = IATA_AREA2;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(!LocUtil::isTransPacific(loc1, loc2, GlobalDirection::AT));

    // IATA Area 1 to 2 via North Pacific - This is not valid
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA2;
    CPPUNIT_ASSERT(!LocUtil::isTransPacific(loc1, loc2, GlobalDirection::NP));

    // IATA Area 2 to 1 via North Pacific - This is not valid
    loc1.area() = IATA_AREA2;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(!LocUtil::isTransPacific(loc1, loc2, GlobalDirection::NP));

    // IATA Area 1 to 3 via Atlantic
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA3;
    CPPUNIT_ASSERT(!LocUtil::isTransPacific(loc1, loc2, GlobalDirection::AT));

    // IATA Area 3 to 1 via Atlantic
    loc1.area() = IATA_AREA3;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(!LocUtil::isTransPacific(loc1, loc2, GlobalDirection::AT));

    // IATA Area 1 to 3 via North Pacific
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA3;
    CPPUNIT_ASSERT(LocUtil::isTransPacific(loc1, loc2, GlobalDirection::NP));

    // IATA Area 3 to 1 via North Pacific
    loc1.area() = IATA_AREA3;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(LocUtil::isTransPacific(loc1, loc2, GlobalDirection::NP));

    // IATA Area 1 to 3 via South, Central or North Pacific
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA3;
    CPPUNIT_ASSERT(LocUtil::isTransPacific(loc1, loc2, GlobalDirection::PA));

    // IATA Area 3 to 1 via South, Central or North Pacific
    loc1.area() = IATA_AREA3;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(LocUtil::isTransPacific(loc1, loc2, GlobalDirection::PA));
  }

  void testIsTransOceanic()
  {
    Loc loc1;
    Loc loc2;

    // Within IATA Area 1
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(!LocUtil::isTransOceanic(loc1, loc2, GlobalDirection::US));

    // Within IATA Area 2
    loc1.area() = IATA_AREA2;
    loc2.area() = IATA_AREA2;
    CPPUNIT_ASSERT(!LocUtil::isTransOceanic(loc1, loc2, GlobalDirection::TT));

    // Within IATA Area 3
    loc1.area() = IATA_AREA3;
    loc2.area() = IATA_AREA3;
    CPPUNIT_ASSERT(!LocUtil::isTransOceanic(loc1, loc2, GlobalDirection::RU));

    // IATA Area 1 to 2 via Atlantic
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA2;
    CPPUNIT_ASSERT(LocUtil::isTransOceanic(loc1, loc2, GlobalDirection::AT));

    // IATA Area 2 to 1 via Atlantic
    loc1.area() = IATA_AREA2;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(LocUtil::isTransOceanic(loc1, loc2, GlobalDirection::AT));

    // IATA Area 1 to 2 via North Pacific
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA2;
    CPPUNIT_ASSERT(!LocUtil::isTransOceanic(loc1, loc2, GlobalDirection::NP));

    // IATA Area 2 to 1 via North Pacific
    loc1.area() = IATA_AREA2;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(!LocUtil::isTransOceanic(loc1, loc2, GlobalDirection::NP));

    // IATA Area 1 to 3 via Atlantic
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA3;
    CPPUNIT_ASSERT(LocUtil::isTransOceanic(loc1, loc2, GlobalDirection::AT));

    // IATA Area 3 to 1 via Atlantic
    loc1.area() = IATA_AREA3;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(LocUtil::isTransOceanic(loc1, loc2, GlobalDirection::AT));

    // IATA Area 1 to 3 via North Pacific
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA3;
    CPPUNIT_ASSERT(LocUtil::isTransOceanic(loc1, loc2, GlobalDirection::NP));

    // IATA Area 3 to 1 via North Pacific
    loc1.area() = IATA_AREA3;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(LocUtil::isTransOceanic(loc1, loc2, GlobalDirection::NP));

    // IATA Area 1 to 3 via South, Central or North Pacific
    loc1.area() = IATA_AREA1;
    loc2.area() = IATA_AREA3;
    CPPUNIT_ASSERT(LocUtil::isTransOceanic(loc1, loc2, GlobalDirection::PA));

    // IATA Area 3 to 1 via South, Central or North Pacific
    loc1.area() = IATA_AREA3;
    loc2.area() = IATA_AREA1;
    CPPUNIT_ASSERT(LocUtil::isTransOceanic(loc1, loc2, GlobalDirection::PA));
  }

  void testGetOverWaterSegments1()
  {
    const Loc* yyz = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYYZ.xml");
    const Loc* lga = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLGA.xml");
    const Loc* jfk = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    const Loc* stt = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSTT.xml");

    AirSeg yyz_lga;
    yyz_lga.origin() = yyz;
    yyz_lga.destination() = lga;

    AirSeg lga_jfk;
    lga_jfk.origin() = lga;
    lga_jfk.destination() = jfk;

    AirSeg jfk_stt;
    jfk_stt.origin() = jfk;
    jfk_stt.destination() = stt;

    AirSeg stt_jfk;
    stt_jfk.origin() = stt;
    stt_jfk.destination() = jfk;

    ArunkSeg jfk_lga;
    jfk_lga.origin() = jfk;
    jfk_lga.destination() = lga;

    AirSeg lga_yyz;
    lga_yyz.origin() = lga;
    lga_yyz.destination() = yyz;

    std::vector<TravelSeg*> travel;
    travel.push_back(&yyz_lga);
    travel.push_back(&lga_jfk);
    travel.push_back(&jfk_stt);
    travel.push_back(&stt_jfk);
    travel.push_back(&lga_yyz);

    std::vector<TravelSeg*> overWaterSegments;

    LocUtil::getOverWaterSegments(travel, overWaterSegments);

    CPPUNIT_ASSERT(overWaterSegments.size() == 2);

    // JFK-STT and STT-JFK are the overwater segments
    //

    CPPUNIT_ASSERT(*(overWaterSegments[0]->origin()) == *jfk);
    CPPUNIT_ASSERT(*(overWaterSegments[0]->destination()) == *stt);

    CPPUNIT_ASSERT(*(overWaterSegments[1]->origin()) == *stt);
    CPPUNIT_ASSERT(*(overWaterSegments[1]->destination()) == *jfk);
  }

  void testGetOverWaterSegments2()
  {
    const Loc* gig = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocGIG.xml");
    const Loc* lax = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");
    const Loc* hnl = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHNL.xml");
    const Loc* mia = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMIA.xml");

    AirSeg gig_lax;
    gig_lax.origin() = gig;
    gig_lax.destination() = lax;

    AirSeg lax_hnl;
    lax_hnl.origin() = lax;
    lax_hnl.destination() = hnl;

    AirSeg hnl_lax;
    hnl_lax.origin() = hnl;
    hnl_lax.destination() = lax;

    AirSeg lax_mia;
    lax_mia.origin() = lax;
    lax_mia.destination() = mia;

    AirSeg mia_gig;
    mia_gig.origin() = mia;
    mia_gig.destination() = gig;

    std::vector<TravelSeg*> travel;
    travel.push_back(&gig_lax);
    travel.push_back(&lax_hnl);
    travel.push_back(&hnl_lax);
    travel.push_back(&lax_mia);
    travel.push_back(&mia_gig);

    std::vector<TravelSeg*> overWaterSegments;

    LocUtil::getOverWaterSegments(travel, overWaterSegments);

    CPPUNIT_ASSERT(overWaterSegments.size() == 4);

    // GIG-LAX, LAX-HNL, HNL-LAX, MIA-GIG are the overwater segments
    //

    CPPUNIT_ASSERT(*(overWaterSegments[0]->origin()) == *gig);
    CPPUNIT_ASSERT(*(overWaterSegments[0]->destination()) == *lax);

    CPPUNIT_ASSERT(*(overWaterSegments[1]->origin()) == *lax);
    CPPUNIT_ASSERT(*(overWaterSegments[1]->destination()) == *hnl);

    CPPUNIT_ASSERT(*(overWaterSegments[2]->origin()) == *hnl);
    CPPUNIT_ASSERT(*(overWaterSegments[2]->destination()) == *lax);

    CPPUNIT_ASSERT(*(overWaterSegments[3]->origin()) == *mia);
    CPPUNIT_ASSERT(*(overWaterSegments[3]->destination()) == *gig);
  }

  void testGetInternationalSegments1()
  {
    const Loc* yyz = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYYZ.xml");
    const Loc* lga = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLGA.xml");
    const Loc* jfk = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    const Loc* stt = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSTT.xml");

    AirSeg yyz_lga;
    yyz_lga.origin() = yyz;
    yyz_lga.destination() = lga;

    AirSeg lga_jfk;
    lga_jfk.origin() = lga;
    lga_jfk.destination() = jfk;

    AirSeg jfk_stt;
    jfk_stt.origin() = jfk;
    jfk_stt.destination() = stt;

    AirSeg stt_jfk;
    stt_jfk.origin() = stt;
    stt_jfk.destination() = jfk;

    ArunkSeg jfk_lga;
    jfk_lga.origin() = jfk;
    jfk_lga.destination() = lga;

    AirSeg lga_yyz;
    lga_yyz.origin() = lga;
    lga_yyz.destination() = yyz;

    std::vector<TravelSeg*> travel;
    travel.push_back(&yyz_lga);
    travel.push_back(&lga_jfk);
    travel.push_back(&jfk_stt);
    travel.push_back(&stt_jfk);
    travel.push_back(&lga_yyz);

    std::vector<TravelSeg*> internationalSegments;

    LocUtil::getInternationalSegments(travel, internationalSegments);

    CPPUNIT_ASSERT(internationalSegments.size() == 2);

    // YYZ-LGA and LGA-YYZ are the international segments
    //

    CPPUNIT_ASSERT(*(internationalSegments[0]->origin()) == *yyz);
    CPPUNIT_ASSERT(*(internationalSegments[0]->destination()) == *lga);

    CPPUNIT_ASSERT(*(internationalSegments[1]->origin()) == *lga);
    CPPUNIT_ASSERT(*(internationalSegments[1]->destination()) == *yyz);
  }

  void testGetInternationalSegments2()
  {
    const Loc* yyz = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYYZ.xml");
    const Loc* lga = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLGA.xml");
    const Loc* lhr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLHR.xml");
    const Loc* par = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPAR.xml");
    const Loc* yhz = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYHZ.xml");

    AirSeg yyz_lga;
    yyz_lga.origin() = yyz;
    yyz_lga.destination() = lga;

    AirSeg lga_lhr;
    lga_lhr.origin() = lga;
    lga_lhr.destination() = lhr;

    AirSeg lhr_par;
    lhr_par.origin() = lhr;
    lhr_par.destination() = par;

    AirSeg par_yhz;
    par_yhz.origin() = par;
    par_yhz.destination() = yhz;

    ArunkSeg yhz_lga;
    yhz_lga.origin() = yhz;
    yhz_lga.destination() = lga;

    AirSeg lga_yyz;
    lga_yyz.origin() = lga;
    lga_yyz.destination() = yyz;

    std::vector<TravelSeg*> travel;
    travel.push_back(&yyz_lga);
    travel.push_back(&lga_lhr);
    travel.push_back(&lhr_par);
    travel.push_back(&par_yhz);
    travel.push_back(&yhz_lga);
    travel.push_back(&lga_yyz);

    std::vector<TravelSeg*> internationalSegments;

    LocUtil::getInternationalSegments(travel, internationalSegments);

    CPPUNIT_ASSERT(internationalSegments.size() == 3);

    // YYZ-LGA and LGA-YYZ are the international segments
    //

    CPPUNIT_ASSERT(*(internationalSegments[0]->origin()) == *lga);
    CPPUNIT_ASSERT(*(internationalSegments[0]->destination()) == *lhr);

    CPPUNIT_ASSERT(*(internationalSegments[1]->origin()) == *lhr);
    CPPUNIT_ASSERT(*(internationalSegments[1]->destination()) == *par);

    CPPUNIT_ASSERT(*(internationalSegments[2]->origin()) == *par);
    CPPUNIT_ASSERT(*(internationalSegments[2]->destination()) == *yhz);
  }

  void testIsInterContinental()
  {
    // Toronto, Canada
    const Loc* yyz = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYYZ.xml");

    // Halifax, Canada
    const Loc* yhz = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYHZ.xml");

    const Loc* lga = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLGA.xml");
    const Loc* dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");

    // Moscow
    const Loc* mow = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMOW.xml");

    // Chelyabinsk, East Ural Russia
    const Loc* cek = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCEK.xml");
    // Loc* temp = const_cast<Loc*>(mow);
    // TestLocFactory::write(string("/vobs/atseintl/test/testdata/data/LocMOW"), *temp);
    // temp = const_cast<Loc*>(cek);
    // TestLocFactory::write(string("/vobs/atseintl/test/testdata/data/LocCEK"), *temp);

    // London
    const Loc* lhr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLHR.xml");

    const Loc* par = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPAR.xml");

    // Athens, Greece
    const Loc* ath = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocATH.xml");

    // Johannesburg, South Africa
    const Loc* jnb = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJNB.xml");

    // Nairobi, Kenya
    const Loc* nbo = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNBO.xml");

    // Mexico City
    const Loc* mex = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEX.xml");

    // Buenos Aires, Argentina
    const Loc* eze = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocEZE.xml");

    // Rio de Janeiro, Brazil
    const Loc* gig = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocGIG.xml");

    // Hong Kong
    const Loc* hkg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHKG.xml");

    // Sydney, Australia
    const Loc* syd = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");

    // Melbourne, Australia
    const Loc* mel = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEL.xml");

    // Riyadh, Saudi Arabia
    const Loc* ruh = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocRUH.xml");

    // Tehran, Iran
    const Loc* thr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTHR.xml");

    // Narssaq, Greenland
    const Loc* jns = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJNS.xml");

    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*yyz, *yhz, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*yhz, *yyz, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*yhz, *jns, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*jns, *yhz, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*yyz, *dfw, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*dfw, *yyz, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*lga, *dfw, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*dfw, *lga, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*dfw, *mex, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*mex, *dfw, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*eze, *gig, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*gig, *eze, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*syd, *mel, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*mel, *syd, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*lhr, *par, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*par, *lhr, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*ruh, *thr, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*thr, *ruh, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*par, *ath, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*ath, *par, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*jnb, *nbo, "ATP"));
    CPPUNIT_ASSERT(!LocUtil::isInterContinental(*nbo, *jnb, "ATP"));

    CPPUNIT_ASSERT(LocUtil::isInterContinental(*mow, *cek, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*cek, *mow, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*lga, *lhr, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*lhr, *lga, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*lga, *par, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*par, *lga, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*ath, *jnb, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*jnb, *ath, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*par, *jnb, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*jnb, *par, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*mex, *gig, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*gig, *mex, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*mex, *eze, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*eze, *mex, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*dfw, *eze, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*eze, *dfw, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*hkg, *syd, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*syd, *hkg, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*thr, *cek, "ATP"));
    CPPUNIT_ASSERT(LocUtil::isInterContinental(*cek, *thr, "ATP"));
  }

  void testIsSameCity()
  {
    DataHandle dataHandle;
    CPPUNIT_ASSERT(LocUtil::isSameCity("NYC", "NYC", dataHandle));
    CPPUNIT_ASSERT(LocUtil::isSameCity("EWR", "NYC", dataHandle));
    CPPUNIT_ASSERT(LocUtil::isSameCity("EWR", "LGA", dataHandle));
    CPPUNIT_ASSERT(LocUtil::isSameCity("LGA", "NYC", dataHandle));
    CPPUNIT_ASSERT(LocUtil::isSameCity("BOS", "BOS", dataHandle));
    CPPUNIT_ASSERT(LocUtil::isSameCity("JBC", "BOS", dataHandle));
    CPPUNIT_ASSERT(LocUtil::isSameCity("BOS", "BED", dataHandle));
    CPPUNIT_ASSERT(LocUtil::isSameCity("JBC", "BED", dataHandle));
    CPPUNIT_ASSERT(!LocUtil::isSameCity("NYC", "BOS", dataHandle));
    CPPUNIT_ASSERT(!LocUtil::isSameCity("BOS", "NYC", dataHandle));
    CPPUNIT_ASSERT(!LocUtil::isSameCity("NYC", "JBC", dataHandle));
    CPPUNIT_ASSERT(!LocUtil::isSameCity("LGA", "BOS", dataHandle));
    CPPUNIT_ASSERT(!LocUtil::isSameCity("LGA", "JBC", dataHandle));
  }

  void testIsLocInZone()
  {
    Loc* dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    Loc* sao = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSAO.xml");
    Loc* rio = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocRIO.xml");
    Loc* man = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMAN.xml");
    Loc* hkg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHKG.xml");
    Loc* tyo = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTYO.xml");
    Loc* lpa = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLPA.xml");
    Loc* tci = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTCI.xml");
    Loc* lis = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLIS.xml");

    // zone 3379: between BR and BR
    CPPUNIT_ASSERT(!LocUtil::isInZone(*rio, *dfw, "SABR", "0003379", 'M'));
    CPPUNIT_ASSERT(!LocUtil::isInZone(*dfw, *rio, "SABR", "0003379", 'M'));
    CPPUNIT_ASSERT(LocUtil::isInZone(*sao, *rio, "SABR", "0003379", 'M'));
    // zone 1142: from area 1 to GB or
    //            between area 2 and area 3 except JP
    CPPUNIT_ASSERT(LocUtil::isInZone(*dfw, *man, "SABR", "0001142", 'M'));
    CPPUNIT_ASSERT(!LocUtil::isInZone(*man, *dfw, "SABR", "0001142", 'M'));
    CPPUNIT_ASSERT(!LocUtil::isInZone(*dfw, *tyo, "SABR", "0001142", 'M'));
    CPPUNIT_ASSERT(!LocUtil::isInZone(*dfw, *rio, "SABR", "0001142", 'M'));
    CPPUNIT_ASSERT(LocUtil::isInZone(*man, *hkg, "SABR", "0001142", 'M'));
    CPPUNIT_ASSERT(LocUtil::isInZone(*hkg, *man, "SABR", "0001142", 'M'));
    CPPUNIT_ASSERT(!LocUtil::isInZone(*man, *tyo, "SABR", "0001142", 'M'));
    CPPUNIT_ASSERT(!LocUtil::isInZone(*tyo, *man, "SABR", "0001142", 'M'));
    // zone 2261: LPA/TCI/ACE/FUE/SPC (no direction) or
    //            from anywhere to LIS/OPO/CAS/TNC
    CPPUNIT_ASSERT(LocUtil::isInZone(*lpa, *tci, "SABR", "0002261", 'M'));
    CPPUNIT_ASSERT(!LocUtil::isInZone(*lpa, *dfw, "SABR", "0002261", 'M'));
    CPPUNIT_ASSERT(LocUtil::isInZone(*lpa, *lis, "SABR", "0002261", 'M'));
    CPPUNIT_ASSERT(LocUtil::isInZone(*dfw, *lis, "SABR", "0002261", 'M'));
    CPPUNIT_ASSERT(!LocUtil::isInZone(*lis, *lpa, "SABR", "0002261", 'M'));
  }

  void setupPU()
  {
    _ptf1.fareMarket() = &_fm1;
    _fu1.paxTypeFare() = &_ptf1;
    _ptf2.fareMarket() = &_fm2;
    _fu2.paxTypeFare() = &_ptf2;
    _pu.fareUsage().push_back(&_fu1);
    _pu.fareUsage().push_back(&_fu2);
  }

  void testIsRTWOrigArea1()
  {
    setupPU();
    _fm1.setGlobalDirection(GlobalDirection::AT);
    _fm2.setGlobalDirection(GlobalDirection::PA);
    CPPUNIT_ASSERT(LocUtil::isRtwOrigArea1(_pu));

    _fm1.setGlobalDirection(GlobalDirection::SA);
    _fm2.setGlobalDirection(GlobalDirection::PA);
    CPPUNIT_ASSERT(LocUtil::isRtwOrigArea1(_pu));

    _fm1.setGlobalDirection(GlobalDirection::AT);
    _fm2.setGlobalDirection(GlobalDirection::SA);
    CPPUNIT_ASSERT(!LocUtil::isRtwOrigArea1(_pu));

    _fm1.setGlobalDirection(GlobalDirection::AP);
    _fm2.setGlobalDirection(GlobalDirection::AP);
    CPPUNIT_ASSERT(!LocUtil::isRtwOrigArea1(_pu));
  }

  void testIsRTWOrigArea2Or3()
  {
    setupPU();
    _fm1.setGlobalDirection(GlobalDirection::AT);
    _fm2.setGlobalDirection(GlobalDirection::PA);
    CPPUNIT_ASSERT(LocUtil::isRtwOrigArea2Or3(_pu));

    _fm1.setGlobalDirection(GlobalDirection::SA);
    _fm2.setGlobalDirection(GlobalDirection::PA);
    CPPUNIT_ASSERT(LocUtil::isRtwOrigArea2Or3(_pu));

    _fm1.setGlobalDirection(GlobalDirection::AT);
    _fm2.setGlobalDirection(GlobalDirection::SA);
    CPPUNIT_ASSERT(!LocUtil::isRtwOrigArea2Or3(_pu));

    _fm1.setGlobalDirection(GlobalDirection::AP);
    _fm2.setGlobalDirection(GlobalDirection::AP);
    CPPUNIT_ASSERT(LocUtil::isRtwOrigArea2Or3(_pu));

    _fm1.setGlobalDirection(GlobalDirection::AP);
    _fm2.setGlobalDirection(GlobalDirection::PA);
    CPPUNIT_ASSERT(!LocUtil::isRtwOrigArea2Or3(_pu));
  }

  void testGetContinentalZoneCode_000_forUS()
  {
    Zone zoneCode;
    Loc* dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *dfw, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("000"), zoneCode);
  }

  void testGetContinentalZoneCode_000_forCA()
  {
    Zone zoneCode;
    Loc* ont = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocONT.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *ont, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("000"), zoneCode);
  }

  void testGetContinentalZoneCode_000_forHawaii()
  {
    Zone zoneCode;
    Loc* ogg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocOGG.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *ogg, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("000"), zoneCode);
  }

  void testGetContinentalZoneCode_000_forMX()
  {
    Zone zoneCode;
    Loc* cancun = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCUN.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *cancun, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("000"), zoneCode);
  }

  void testGetContinentalZoneCode_170_forBazil()
  {
    Zone zoneCode;
    Loc* cwb = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCWB.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *cwb, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("170"), zoneCode);
  }

  void testGetContinentalZoneCode_210_forGemany()
  {
    Zone zoneCode;
    Loc* fra = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *fra, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("210"), zoneCode);
  }

  void testGetContinentalZoneCode_210_forSpain()
  {
    Zone zoneCode;
    Loc* lpa = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLPA.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *lpa, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("210"), zoneCode);
  }

  void testGetContinentalZoneCode_210_forSweden()
  {
    Zone zoneCode;
    Loc* sturup = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMMX.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *sturup, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("210"), zoneCode);
  }

  void testGetContinentalZoneCode_210_forRussiaWestOfUrals()
  {
    Zone zoneCode;
    Loc* mow = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMOW.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *mow, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("210"), zoneCode);
  }

  void testGetContinentalZoneCode_220_forIran()
  {
    Zone zoneCode;
    const Loc* thr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTHR.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *thr, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("220"), zoneCode);
  }

  void testGetContinentalZoneCode_230_forKenya()
  {
    Zone zoneCode;
    const Loc* nbo = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNBO.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *nbo, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("230"), zoneCode);
  }

  void testGetContinentalZoneCode_310_forEastAsia()
  {
    Zone zoneCode;
    const Loc* nrt = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNRT.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *nrt, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("310"), zoneCode);
  }

  void testGetContinentalZoneCode_320_forChinaAndHkg()
  {
    Zone zoneCode;
    const Loc* pvg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPVG.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *pvg, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("320"), zoneCode);

    const Loc* hkg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHKG.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *hkg, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("320"), zoneCode);
  }

  void testGetContinentalZoneCode_330_forIndia()
  {
    Zone zoneCode;
    const Loc* bom = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBOM.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *bom, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("330"), zoneCode);
  }

  void testGetContinentalZoneCode_340_forAustralia()
  {
    Zone zoneCode;
    const Loc* syd = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *syd, Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL(Zone("340"), zoneCode);
  }

  void testGetContinentalZoneCode_350_forCanton() {}

  void testGetContinentalZoneCode_360_forRussiaEastOfUrals()
  {
    Zone zoneCode;
    DataHandle dataHandle;
    const Loc* hta = dataHandle.getLoc("HTA", DateTime::localTime());

    CPPUNIT_ASSERT(LocUtil::getContinentalZoneCode(zoneCode, *hta, Vendor::SITA));
    CPPUNIT_ASSERT_EQUAL(Zone("360"), zoneCode);
  }

  void testIsFormerNetherlandsAntillesUS()
  {
    NationCode nation = "US";
    CPPUNIT_ASSERT(!LocUtil::isFormerNetherlandsAntilles(nation));
  }

  void testIsFormerNetherlandsAntillesSX()
  {
    NationCode nation = "SX";
    CPPUNIT_ASSERT(LocUtil::isFormerNetherlandsAntilles(nation));
  }

  void testIsWholeTravelInUSTerritory_WholeInUS()
  {
    std::vector<TravelSeg*> travelSegs;

    TravelSeg* ts1 = createAirSeg("US", "US");
    TravelSeg* ts2 = createAirSeg("US", "US");

    travelSegs += ts1, ts2;

    CPPUNIT_ASSERT_EQUAL(true, LocUtil::isWholeTravelInUSTerritory(travelSegs));
  }

  void testIsWholeTravelInUSTerritory_PartInUS()
  {
    std::vector<TravelSeg*> travelSegs;

    TravelSeg* ts1 = createAirSeg("US", "PL");
    TravelSeg* ts2 = createAirSeg("US", "US");

    travelSegs += ts1, ts2;

    CPPUNIT_ASSERT_EQUAL(false, LocUtil::isWholeTravelInUSTerritory(travelSegs));
  }

  void testIsWholeTravelInUSTerritory_NoneInUS()
  {
    std::vector<TravelSeg*> travelSegs;

    TravelSeg* ts1 = createAirSeg("PL", "PL");
    TravelSeg* ts2 = createAirSeg("PL", "PL");

    travelSegs += ts1, ts2;

    CPPUNIT_ASSERT_EQUAL(false, LocUtil::isWholeTravelInUSTerritory(travelSegs));
  }

  void testIsCityInSpanishOverseaIslands()
  {
    CPPUNIT_ASSERT(LocUtil::isCityInSpanishOverseaIslands("TCI"));
    CPPUNIT_ASSERT(LocUtil::isCityInSpanishOverseaIslands("PMI"));
    CPPUNIT_ASSERT(LocUtil::isCityInSpanishOverseaIslands("MLN"));
    CPPUNIT_ASSERT(LocUtil::isCityInSpanishOverseaIslands("LPA"));
    CPPUNIT_ASSERT(LocUtil::isCityInSpanishOverseaIslands("FUE"));
    CPPUNIT_ASSERT(LocUtil::isCityInSpanishOverseaIslands("ACE"));
    CPPUNIT_ASSERT(LocUtil::isCityInSpanishOverseaIslands("MAH"));
    CPPUNIT_ASSERT(LocUtil::isCityInSpanishOverseaIslands("IBZ"));
    CPPUNIT_ASSERT(LocUtil::isCityInSpanishOverseaIslands("JCU"));
    CPPUNIT_ASSERT(!LocUtil::isCityInSpanishOverseaIslands("MAD"));
    CPPUNIT_ASSERT(!LocUtil::isCityInSpanishOverseaIslands(""));
  }

  void testIsSameMultiTransportCity_PASS()
  {
    CPPUNIT_ASSERT(
        LocUtil::isSameMultiTransportCity("JFK", "LGA", "", GeoTravelType::International, DateTime::localTime()));
  }

  void testIsSameMultiTransportCity_FAIL()
  {
    CPPUNIT_ASSERT(
        !LocUtil::isSameMultiTransportCity("JFK", "CDG", "", GeoTravelType::International, DateTime::localTime()));
  }

private:
  class MyDataHandle : public DataHandleMock
  {
    TestMemHandle _memHandle;
    ATPResNationZones* getZone(std::string nation, std::string zone)
    {
      ATPResNationZones* ret = _memHandle.create<ATPResNationZones>();
      ret->nation() = nation;
      ret->zones().push_back(zone);
      return ret;
    }

    MultiAirportCity* getMultAirCity(std::string airport, std::string city)
    {
      MultiAirportCity* mc = _memHandle.create<MultiAirportCity>();
      mc->airportCode() = airport;
      mc->city() = city;
      return mc;
    }

  public:
    const Loc* getLoc(const LocCode& locCode, const DateTime& date)
    {
      if (locCode == "")
        return 0;
      return DataHandleMock::getLoc(locCode, date);
    }

    const std::vector<ATPResNationZones*>& getATPResNationZones(const NationCode& nation)
    {
      std::vector<ATPResNationZones*>& ret = *_memHandle.create<std::vector<ATPResNationZones*> >();
      if (nation == "AU")
        ret.push_back(getZone("AU", "0000340"));
      else if (nation == "BR")
      {
        ret.push_back(getZone("BR", "0000170"));
        ret.push_back(getZone("BR", "0000171"));
      }
      else if (nation == "BS")
        ret.push_back(getZone("BS", "0000171"));
      else if (nation == "HK")
        ret.push_back(getZone("HS", "0000320"));
      else if (nation == "JP")
        ret.push_back(getZone("JP", "0000310"));
      else if (nation == "SG")
        ret.push_back(getZone("SG", "0000320"));
      else if (nation == "US")
      {
        ret.push_back(getZone("US", "0000000"));
        ret.push_back(getZone("US", "0000001"));
      }
      else
        return DataHandleMock::getATPResNationZones(nation);
      return ret;
    }

    const std::vector<MultiAirportCity*>& getMultiCityAirport(const LocCode& city)
    {
      std::vector<MultiAirportCity*>& ret = *_memHandle.create<std::vector<MultiAirportCity*> >();
      if (city == "NYC")
      {
        ret.push_back(getMultAirCity(city, "EWR"));
        ret.push_back(getMultAirCity(city, "JFK"));
        ret.push_back(getMultAirCity(city, "JRA"));
        ret.push_back(getMultAirCity(city, "JRB"));
        ret.push_back(getMultAirCity(city, "JRE"));
        ret.push_back(getMultAirCity(city, "LGA"));
        ret.push_back(getMultAirCity(city, "TSS"));
      }
      else
        return DataHandleMock::getMultiCityAirport(city);
      return ret;
    }

    const std::vector<MultiAirportCity*>& getMultiAirportCity(const LocCode& city)
    {
      std::vector<MultiAirportCity*>& ret = *_memHandle.create<std::vector<MultiAirportCity*> >();
      if (city == "EWR" || city == "LGA")
        ret.push_back(getMultAirCity(city, "NYC"));
      else if (city == "JBC" || city == "BED")
        ret.push_back(getMultAirCity(city, "BOS"));
      else if (city == "NYC" || city == "BOS")
        return ret;
      else
        return DataHandleMock::getMultiAirportCity(city);
      return ret;
    }

    const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                            const CarrierCode& carrierCode,
                                            GeoTravelType tvlType,
                                            const DateTime& tvlDate)
    {
      if (locCode == "DFW")
        return "DFW";
      else if (locCode == "LGA" || locCode == "JFK")
        return "NYC";
      else if (locCode == "ORY" || locCode == "CDG")
        return "PAR";
      else if (locCode == "PAR" || locCode == "NYC")
        return "";
      return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
    }

    const Mileage* getMileage(const LocCode& origin,
                              const LocCode& dest,
                              Indicator mileageType,
                              const GlobalDirection globalDir,
                              const DateTime& date)
    {
      Mileage* ret = _memHandle.create<Mileage>();
      ret->orig() = origin;
      ret->dest() = dest;
      ret->mileageType() = mileageType;
      ret->globaldir() = globalDir;
      if (origin == "MNL" && dest == "NCL")
        ret->mileage() = 7700;
      else if (origin == "RES" && dest == "ASU")
        ret->mileage() = 178;
      else
        return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);
      return ret;
    }

    const ZoneInfo*
    getZone(const VendorCode& vendor, const Zone& zone, Indicator zoneType, const DateTime& date)
    {
      if (vendor == "SABR" && zone == "0000210" && zoneType == 'R')
        return 0;
      return DataHandleMock::getZone(vendor, zone, zoneType, date);
    }

    const LocCode getMultiTransportCity(const LocCode& locCode)
    {
      if (locCode == "EWR")
        return "NYC";
      return DataHandleMock::getMultiTransportCity(locCode);
    }
  };

  PricingUnit _pu;
  FareUsage _fu1, _fu2;
  PaxTypeFare _ptf1, _ptf2;
  FareMarket _fm1, _fm2;
  MyDataHandle* _mdh;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(LocUtilTest);
}
