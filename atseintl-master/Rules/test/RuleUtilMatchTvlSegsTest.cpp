#include "Common/DateTime.h"
#include "Common/ItinUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Rules/RuleUtil.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

#include "test/include/CppUnitHelperMacros.h"
#include <iostream>
#include <set>
#include <vector>
#include <time.h>

namespace tse
{

class RuleUtilMatchTvlSegsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RuleUtilMatchTvlSegsTest);

  CPPUNIT_TEST(testMatchTvlSegsFromTo_Key1Btw_Key1And_Empty_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_Two_Segments_Key1Btw_Key1And_LOCs_NoMatch_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_Two_Segments_Key1Btw_Key1And_Exists_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_Two_Segments_Key1Btw_Key1And_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_Two_Segments_Key1And_Key1Btw_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_Three_Segments_Key1Btw_Key1And_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_Three_Segments_Key1And_Key1Btw_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_Five_Segments_Key1Btw_Key1And_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_Five_Segments_Key1And_Key1Btw_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_Two_Segments_Key1Btw_Key1And_Plus_Hidden_Points_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_Two_Segments_Key1And_Key1Btw_Plus_Hidden_Points_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_Two_Segments_Key1Btw_Key1And_Plus_Hidden_Points_Exists_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_Two_Segments_Key1And_Key1Btw_Plus_Hidden_Points_Exists_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_Two_Segments_Key1BtwNDE_Key1AndZ210_Exists_Sub_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_Two_Segments_Key1AndZ210_Key1BtwNDE_Exists_Sub_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC1_Three_Segments_Key1Btw_USTX_Key1And_NUS_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC1_Three_Segments_Key1And_NUS_Key1Btw_USTX_Exists_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC2_Three_Segments_Key1Btw_USTX_Key1And_NUS_Exists_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC2_Three_Segments_Key1And_NUS_Key1Btw_USTX_Exists_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC3_Three_Segments_Key1Btw_USTX_Key1And_NUS_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC3_Three_Segments_Key1And_NUS_Key1Btw_USTX_Exists_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC4_Two_Segments_Key1Btw_USTX_Key1And_NUS_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC4_Two_Segments_Key1And_NUS_Key1Btw_USTX_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC5_Three_Segments_Key1Btw_Z210_Key1And_A2_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC5_Three_Segments_Key1And_A2_Key1Btw_Z210_Exists_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC6_Two_Segments_Key1Btw_Z210_Key1And_A2_Exists_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC6_Two_Segments_Key1And_A2_Key1Btw_Z210_Exists_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC7_Two_Segments_Key1Btw_Z210_Key1And_Z210_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC7_Two_Segments_Key1And_Z210_Key1Btw_Z210_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC8_Two_Segments_Key1Btw_NUS_Key1And_NDE_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC8_Two_Segments_Key1And_NDE_Key1Btw_NUS_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC8_Two_Segments_Key1Btw_NUS_Key1And_NDE_Exists_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC8_Two_Segments_Key1Btw_NDE_Key1And_NUS_Exists_False);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC9_Two_Segments_Key1Btw_NSG_Key1And_A3_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC9_Two_Segments_Key1Btw_A3_Key1And_NSG_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC10_Two_Segments_Key1Btw_NSG_Key1And_Z340_Exists_True);
  CPPUNIT_TEST(testMatchTvlSegsFromTo_SC10_Two_Segments_Key1Btw_Z340_Key1And_NSG_Exists_False);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->getRequest()->ticketingDT() = DateTime::localTime();

    anc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocANC.xml");
    atl = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocATL.xml");
    cdg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCDG.xml");
    dal = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDAL.xml");
    dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    dus = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDUS.xml");
    fra = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");
    gig = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocGIG.xml");
    hkg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHKG.xml");
    iah = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocIAH.xml");
    jnb = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJNB.xml");
    lax = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");
    lon = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    mel = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEL.xml");
    mia = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMIA.xml");
    nrt = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNRT.xml");
    nyc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    sin = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSIN.xml");
    syd = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");
    tul = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTUL.xml");
    yvr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYVR.xml");
    yyz = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYYZ.xml");

  }

  void tearDown() { _memHandle.clear(); }

  void testMatchTvlSegsFromTo_Key1Btw_Key1And_Empty_False()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;
    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));
  }

  void testMatchTvlSegsFromTo_Two_Segments_Key1Btw_Key1And_LOCs_NoMatch_False()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3, tvlSeg4, tvlSeg5, tvlSeg6;
    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "US";
    locKey2.loc() = "CA";
    const Loc* loc1 = lon;
    const Loc* loc2 = lax;
    const Loc* loc3 = gig;
    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
  }

  void testMatchTvlSegsFromTo_Two_Segments_Key1Btw_Key1And_Exists_False()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "AU";
    locKey2.loc() = "CA";
    const Loc* loc1 = nyc;
    const Loc* loc2 = yyz;
    const Loc* loc3 = lax;

    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
  }

  void testMatchTvlSegsFromTo_Two_Segments_Key1Btw_Key1And_Exists_True()
  {
    AirSeg tvlSeg2, tvlSeg1;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "US";
    locKey2.loc() = "CA";
    const Loc* loc1 = nyc;
    const Loc* loc2 = yyz;
    const Loc* loc3 = lax;

    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 1);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) == tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_Two_Segments_Key1And_Key1Btw_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "US";
    locKey2.loc() = "CA";
    const Loc* loc1 = nyc;
    const Loc* loc2 = yyz;
    const Loc* loc3 = lax;

    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 1);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_Three_Segments_Key1Btw_Key1And_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "US";
    locKey2.loc() = "CA";
    const Loc* loc1 = nyc;
    const Loc* loc2 = yyz;
    const Loc* loc3 = lax;
    const Loc* loc4 = yvr;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 2);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg3) != tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_Three_Segments_Key1And_Key1Btw_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "US";
    locKey2.loc() = "CA";
    const Loc* loc1 = nyc;
    const Loc* loc2 = yyz;
    const Loc* loc3 = lax;
    const Loc* loc4 = yvr;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 1);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_Five_Segments_Key1Btw_Key1And_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3, tvlSeg4, tvlSeg5, tvlSeg6;
    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);
    tvlSeg4.departureDT() = DateTime(2012, 8, 11, 8, 0, 0);
    tvlSeg5.departureDT() = DateTime(2012, 8, 12, 8, 0, 0);
    tvlSeg6.departureDT() = DateTime(2012, 8, 13, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);
    allTvlSegs.push_back(&tvlSeg4);
    allTvlSegs.push_back(&tvlSeg5);
    allTvlSegs.push_back(&tvlSeg6);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "US";
    locKey2.loc() = "CA";
    const Loc* loc1 = nyc;
    const Loc* loc2 = yyz;
    const Loc* loc3 = lax;
    const Loc* loc4 = yvr;
    const Loc* loc5 = anc;
    const Loc* loc6 = tul;
    const Loc* loc7 = dal;
    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;

    tvlSeg4.segmentOrder() = 4;
    tvlSeg4.origin() = loc4;
    tvlSeg4.destination() = loc5;

    tvlSeg5.segmentOrder() = 5;
    tvlSeg5.origin() = loc5;
    tvlSeg5.destination() = loc6;

    tvlSeg6.segmentOrder() = 6;
    tvlSeg6.origin() = loc6;
    tvlSeg6.destination() = loc7;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 2);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg3) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg4) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg5) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg6) == tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_Five_Segments_Key1And_Key1Btw_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3, tvlSeg4, tvlSeg5, tvlSeg6;
    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);
    tvlSeg4.departureDT() = DateTime(2012, 8, 11, 8, 0, 0);
    tvlSeg5.departureDT() = DateTime(2012, 8, 12, 8, 0, 0);
    tvlSeg6.departureDT() = DateTime(2012, 8, 13, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);
    allTvlSegs.push_back(&tvlSeg4);
    allTvlSegs.push_back(&tvlSeg5);
    allTvlSegs.push_back(&tvlSeg6);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "US";
    locKey2.loc() = "CA";
    const Loc* loc1 = nyc;
    const Loc* loc2 = yyz;
    const Loc* loc3 = lax;
    const Loc* loc4 = yvr;
    const Loc* loc5 = anc;
    const Loc* loc6 = tul;
    const Loc* loc7 = dal;
    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;

    tvlSeg4.segmentOrder() = 4;
    tvlSeg4.origin() = loc4;
    tvlSeg4.destination() = loc5;

    tvlSeg5.segmentOrder() = 5;
    tvlSeg5.origin() = loc5;
    tvlSeg5.destination() = loc6;

    tvlSeg6.segmentOrder() = 6;
    tvlSeg6.origin() = loc6;
    tvlSeg6.destination() = loc7;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 2);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg3) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg4) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg5) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg6) == tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_Two_Segments_Key1Btw_Key1And_Plus_Hidden_Points_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3, tvlSeg4, tvlSeg5, tvlSeg6;
    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "US";
    locKey2.loc() = "CA";
    const Loc* loc1 = lon;
    Loc* locH1 = const_cast<Loc*>(nyc);
    Loc* locH2 = const_cast<Loc*>(yyz);
    const Loc* loc2 = lax;
    Loc* locH3 = const_cast<Loc*>(yvr);
    Loc* locH4 = const_cast<Loc*>(anc);
    Loc* locH5 = const_cast<Loc*>(dal);
    const Loc* loc3 = gig;
    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg1.hiddenStops().push_back(locH1);
    tvlSeg1.hiddenStops().push_back(locH2);
    tvlSeg2.hiddenStops().push_back(locH3);
    tvlSeg2.hiddenStops().push_back(locH4);
    tvlSeg2.hiddenStops().push_back(locH5);

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, true, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 2);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_Two_Segments_Key1And_Key1Btw_Plus_Hidden_Points_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3, tvlSeg4, tvlSeg5, tvlSeg6;
    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "US";
    locKey2.loc() = "CA";
    const Loc* loc1 = lon;
    Loc* locH1 = const_cast<Loc*>(nyc);
    Loc* locH2 = const_cast<Loc*>(yyz);
    const Loc* loc2 = lax;
    Loc* locH3 = const_cast<Loc*>(yvr);
    Loc* locH4 = const_cast<Loc*>(anc);
    Loc* locH5 = const_cast<Loc*>(dal);
    const Loc* loc3 = gig;
    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg1.hiddenStops().push_back(locH1);
    tvlSeg1.hiddenStops().push_back(locH2);
    tvlSeg2.hiddenStops().push_back(locH3);
    tvlSeg2.hiddenStops().push_back(locH4);
    tvlSeg2.hiddenStops().push_back(locH5);

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, true, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 2);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_Two_Segments_Key1Btw_Key1And_Plus_Hidden_Points_Exists_False()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3, tvlSeg4, tvlSeg5, tvlSeg6;
    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "US";
    locKey2.loc() = "CA";
    const Loc* loc1 = lon;
    Loc* locH1 = const_cast<Loc*>(syd);
    const Loc* loc2 = lax;
    Loc* locH2 = const_cast<Loc*>(mel);
    const Loc* loc3 = gig;
    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg1.hiddenStops().push_back(locH1);
    tvlSeg2.hiddenStops().push_back(locH2);

    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, true, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
  }

  void testMatchTvlSegsFromTo_Two_Segments_Key1And_Key1Btw_Plus_Hidden_Points_Exists_False()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3, tvlSeg4, tvlSeg5, tvlSeg6;
    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "US";
    locKey2.loc() = "CA";
    const Loc* loc1 = lon;
    Loc* locH1 = const_cast<Loc*>(syd);
    const Loc* loc2 = lax;
    Loc* locH2 = const_cast<Loc*>(mel);
    const Loc* loc3 = gig;
    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg1.hiddenStops().push_back(locH1);
    tvlSeg2.hiddenStops().push_back(locH2);

    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, true, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
  }

  void testMatchTvlSegsFromTo_Two_Segments_Key1BtwNDE_Key1AndZ210_Exists_Sub_True()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_ZONE;
    locKey1.loc() = "DE";
    locKey2.loc() = "210";
    const Loc* loc1 = fra;
    const Loc* loc2 = dus;
    const Loc* loc3 = cdg;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 1);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_Two_Segments_Key1AndZ210_Key1BtwNDE_Exists_Sub_False()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_ZONE;
    locKey1.loc() = "DE";
    locKey2.loc() = "210";
    const Loc* loc1 = fra;
    const Loc* loc2 = dus;
    const Loc* loc3 = cdg;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
  }

  // Tests cases below are created to satisfy the ATP Cat4 - between GEO resolution, on Nov 3,2010
  void testMatchTvlSegsFromTo_SC1_Three_Segments_Key1Btw_USTX_Key1And_NUS_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_STATE;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "USTX";
    locKey2.loc() = "US";
    const Loc* loc1 = dfw;
    const Loc* loc2 = iah;
    const Loc* loc3 = mia;
    const Loc* loc4 = sin;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 1);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg3) == tvlSegsRtn.end());
  }
  void testMatchTvlSegsFromTo_SC1_Three_Segments_Key1And_NUS_Key1Btw_USTX_Exists_False()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_STATE;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "USTX";
    locKey2.loc() = "US";
    const Loc* loc1 = dfw;
    const Loc* loc2 = iah;
    const Loc* loc3 = mia;
    const Loc* loc4 = sin;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;

    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
  }
  void testMatchTvlSegsFromTo_SC2_Three_Segments_Key1Btw_USTX_Key1And_NUS_Exists_False()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_STATE;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "USTX";
    locKey2.loc() = "US";
    const Loc* loc1 = dfw;
    const Loc* loc2 = iah;
    const Loc* loc3 = hkg;
    const Loc* loc4 = sin;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;
    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
  }

  void testMatchTvlSegsFromTo_SC2_Three_Segments_Key1And_NUS_Key1Btw_USTX_Exists_False()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_STATE;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "USTX";
    locKey2.loc() = "US";
    const Loc* loc1 = dfw;
    const Loc* loc2 = iah;
    const Loc* loc3 = hkg;
    const Loc* loc4 = sin;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;
    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
  }

  void testMatchTvlSegsFromTo_SC3_Three_Segments_Key1Btw_USTX_Key1And_NUS_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_STATE;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "USTX";
    locKey2.loc() = "US";
    const Loc* loc1 = dfw;
    const Loc* loc2 = mia;
    const Loc* loc3 = lax;
    const Loc* loc4 = sin;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;
    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 1);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg3) == tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_SC3_Three_Segments_Key1And_NUS_Key1Btw_USTX_Exists_False()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_STATE;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "USTX";
    locKey2.loc() = "US";
    const Loc* loc1 = dfw;
    const Loc* loc2 = mia;
    const Loc* loc3 = lax;
    const Loc* loc4 = sin;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;
    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
  }

  void testMatchTvlSegsFromTo_SC4_Two_Segments_Key1Btw_USTX_Key1And_NUS_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::Domestic;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_STATE;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "USTX";
    locKey2.loc() = "US";
    const Loc* loc1 = atl;
    const Loc* loc2 = dfw;
    const Loc* loc3 = mia;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 1);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_SC4_Two_Segments_Key1And_NUS_Key1Btw_USTX_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_STATE;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "USTX";
    locKey2.loc() = "US";
    const Loc* loc1 = atl;
    const Loc* loc2 = dfw;
    const Loc* loc3 = mia;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 1);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) == tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_SC5_Three_Segments_Key1Btw_Z210_Key1And_A2_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_ZONE;
    locKey2.locType() = LOCTYPE_AREA;
    locKey1.loc() = "210";
    locKey2.loc() = "2";
    const Loc* loc1 = lon;
    const Loc* loc2 = cdg;
    const Loc* loc3 = jnb;
    const Loc* loc4 = sin;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;
    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 1);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg3) == tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_SC5_Three_Segments_Key1And_A2_Key1Btw_Z210_Exists_False()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_ZONE;
    locKey2.locType() = LOCTYPE_AREA;
    locKey1.loc() = "210";
    locKey2.loc() = "2";
    const Loc* loc1 = lon;
    const Loc* loc2 = cdg;
    const Loc* loc3 = jnb;
    const Loc* loc4 = sin;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;
    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
  }

  void testMatchTvlSegsFromTo_SC6_Two_Segments_Key1Btw_Z210_Key1And_A2_Exists_False()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_ZONE;
    locKey2.locType() = LOCTYPE_AREA;
    locKey1.loc() = "210";
    locKey2.loc() = "2";
    const Loc* loc1 = nyc;
    const Loc* loc2 = cdg;
    const Loc* loc3 = lon;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
  }

  void testMatchTvlSegsFromTo_SC6_Two_Segments_Key1And_A2_Key1Btw_Z210_Exists_False()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_ZONE;
    locKey2.locType() = LOCTYPE_AREA;
    locKey1.loc() = "210";
    locKey2.loc() = "2";
    const Loc* loc1 = nyc;
    const Loc* loc2 = cdg;
    const Loc* loc3 = lon;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
  }

  void testMatchTvlSegsFromTo_SC7_Two_Segments_Key1Btw_Z210_Key1And_Z210_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_ZONE;
    locKey2.locType() = LOCTYPE_ZONE;
    locKey1.loc() = "210";
    locKey2.loc() = "210";
    const Loc* loc1 = nyc;
    const Loc* loc2 = cdg;
    const Loc* loc3 = lon;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 1);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_SC7_Two_Segments_Key1And_Z210_Key1Btw_Z210_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_ZONE;
    locKey2.locType() = LOCTYPE_ZONE;
    locKey1.loc() = "210";
    locKey2.loc() = "210";
    const Loc* loc1 = nyc;
    const Loc* loc2 = cdg;
    const Loc* loc3 = lon;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 1);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_SC8_Two_Segments_Key1Btw_NUS_Key1And_NDE_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "US";
    locKey2.loc() = "DE";
    const Loc* loc1 = nyc;
    const Loc* loc2 = cdg;
    const Loc* loc3 = lon;
    const Loc* loc4 = fra;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 3);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg3) != tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_SC8_Two_Segments_Key1And_NDE_Key1Btw_NUS_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "DE";
    locKey2.loc() = "US";
    const Loc* loc1 = nyc;
    const Loc* loc2 = cdg;
    const Loc* loc3 = lon;
    const Loc* loc4 = fra;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 3);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg3) != tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_SC8_Two_Segments_Key1Btw_NUS_Key1And_NDE_Exists_False()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "US";
    locKey2.loc() = "DE";
    const Loc* loc1 = nyc;
    const Loc* loc2 = cdg;
    const Loc* loc3 = lon;
    const Loc* loc4 = sin;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;

    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg3) == tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_SC8_Two_Segments_Key1Btw_NDE_Key1And_NUS_Exists_False()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2012, 8, 10, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NATION;
    locKey1.loc() = "US";
    locKey2.loc() = "DE";
    const Loc* loc1 = nyc;
    const Loc* loc2 = cdg;
    const Loc* loc3 = lon;
    const Loc* loc4 = sin;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    tvlSeg3.segmentOrder() = 3;
    tvlSeg3.origin() = loc3;
    tvlSeg3.destination() = loc4;

    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg3) == tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_SC9_Two_Segments_Key1Btw_NSG_Key1And_A3_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_AREA;
    locKey1.loc() = "SG";
    locKey2.loc() = "3";
    const Loc* loc1 = nrt;
    const Loc* loc2 = sin;
    const Loc* loc3 = syd;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 1);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_SC9_Two_Segments_Key1Btw_A3_Key1And_NSG_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_AREA;
    locKey1.loc() = "SG";
    locKey2.loc() = "3";
    const Loc* loc1 = nrt;
    const Loc* loc2 = sin;
    const Loc* loc3 = syd;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 1);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) != tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) == tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_SC10_Two_Segments_Key1Btw_NSG_Key1And_Z340_Exists_True()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_ZONE;
    locKey1.loc() = "SG";
    locKey2.loc() = "340";
    const Loc* loc1 = nrt;
    const Loc* loc2 = sin;
    const Loc* loc3 = syd;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        true,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey1, locKey2, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 1);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) != tvlSegsRtn.end());
  }

  void testMatchTvlSegsFromTo_SC10_Two_Segments_Key1Btw_Z340_Key1And_NSG_Exists_False()
  {
    AirSeg tvlSeg1, tvlSeg2;

    tvlSeg1.departureDT() = DateTime(2012, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2012, 8, 9, 8, 0, 0);

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);

    std::set<TravelSeg*> tvlSegsRtn;

    VendorCode vendorCode = "ATP";
    GeoTravelType geoTvlType = GeoTravelType::International;

    LocKey locKey1;
    LocKey locKey2;
    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_ZONE;
    locKey1.loc() = "SG";
    locKey2.loc() = "340";
    const Loc* loc1 = nrt;
    const Loc* loc2 = sin;
    const Loc* loc3 = syd;

    // create the travel segments
    //
    tvlSeg1.segmentOrder() = 1;
    tvlSeg1.origin() = loc1;
    tvlSeg1.destination() = loc2;

    tvlSeg2.segmentOrder() = 2;
    tvlSeg2.origin() = loc2;
    tvlSeg2.destination() = loc3;

    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::matchAllTvlSegsFromTo(
            *_trx, locKey2, locKey1, vendorCode, allTvlSegs, false, tvlSegsRtn, geoTvlType));

    CPPUNIT_ASSERT(tvlSegsRtn.size() == 0);
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg1) == tvlSegsRtn.end());
    CPPUNIT_ASSERT(tvlSegsRtn.find(&tvlSeg2) == tvlSegsRtn.end());
  }

private:
  PricingTrx* _trx;
  PricingRequest* _request;
  TestMemHandle _memHandle;

  const Loc* anc;
  const Loc* atl;
  const Loc* cdg;
  const Loc* dal;
  const Loc* dfw;
  const Loc* dus;
  const Loc* fra;
  const Loc* gig;
  const Loc* hkg;
  const Loc* iah;
  const Loc* jnb;
  const Loc* lax;
  const Loc* lon;
  const Loc* mel;
  const Loc* mia;
  const Loc* nrt;
  const Loc* nyc;
  const Loc* sin;
  const Loc* syd;
  const Loc* tul;
  const Loc* yvr;
  const Loc* yyz;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilMatchTvlSegsTest);
} //tse
