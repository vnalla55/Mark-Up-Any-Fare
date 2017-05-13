#include "Rules/test/RuleUtilTSITest_Base.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class RuleUtilTSITest_GetGeoData : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_GetGeoData);
  CPPUNIT_TEST(testGetGeoData_GeoBothReq_locSFONone_locSYDNone);
  CPPUNIT_TEST(testGetGeoData_GeoBothReq_locSFONone_locSYDtype);
  CPPUNIT_TEST(testGetGeoData_GeoBothReq_locSFOtype_locSYDNone);
  CPPUNIT_TEST(testGetGeoData_GeoBothReq_locSFOtype_locSYDtype);
  CPPUNIT_TEST(testGetGeoData_GeoBothReq_locSFOEmpty_locSYDEmpty);
  CPPUNIT_TEST(testGetGeoData_GeoBothReq_locSFOEmpty_locSYDloc);
  CPPUNIT_TEST(testGetGeoData_GeoBothReq_locSFOloc_locSYDEmpty);
  CPPUNIT_TEST(testGetGeoData_GeoBothReq_locSFOloc_locSYDloc);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSFONone_locSYDNone);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSFO_locSYD_NoGeoItin);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSFOArea);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSFOSubarea);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSFOCity);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSFOAirport);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSFONation);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSFOState);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSFOZone);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSYDArea);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSYDSubarea);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSYDCity);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSYDAirport);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSYDNation);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSYDState);
  CPPUNIT_TEST(testGetGeoData_GeotypeReq_locSYDZone);
  CPPUNIT_TEST(testGetGeoData_GetFromItin_NoGetFromItin);
  CPPUNIT_TEST(testGetGeoData_GetFromItin_GetFromItin);
  CPPUNIT_TEST(testGetGeoData_GeoBlank);
  CPPUNIT_TEST_SUITE_END();

public:
  void testGetGeoData_GeoBothReq_locSFONone_locSYDNone()
  {
    // if both blank, return false
    getGeoDataData(RuleConst::TSI_GEO_BOTH_REQUIRED);
    _locKey1.locType() = _locKey2.locType() = LOCTYPE_NONE;
    assertGetGeoDataLoc(_trx, false);
  }
  void testGetGeoData_GeoBothReq_locSFONone_locSYDtype()
  {
    // pass on loc1
    getGeoDataData(RuleConst::TSI_GEO_BOTH_REQUIRED);
    _locKey1.locType() = LOCTYPE_NONE;
    assertGetGeoDataLoc(_trx, true);
  }
  void testGetGeoData_GeoBothReq_locSFOtype_locSYDNone()
  {
    // pass on loc2
    getGeoDataData(RuleConst::TSI_GEO_BOTH_REQUIRED);
    _locKey2.locType() = LOCTYPE_NONE;
    assertGetGeoDataLoc(_trx, true);
  }
  void testGetGeoData_GeoBothReq_locSFOtype_locSYDtype()
  {
    // pass on both
    getGeoDataData(RuleConst::TSI_GEO_BOTH_REQUIRED);
    assertGetGeoDataLoc(_trx, true);
  }
  void testGetGeoData_GeoBothReq_locSFOEmpty_locSYDEmpty()
  {
    // if both empty, return false
    getGeoDataData(RuleConst::TSI_GEO_BOTH_REQUIRED);
    _locKey1.loc() = _locKey2.loc() = "";
    assertGetGeoDataLoc(_trx, false);
  }
  void testGetGeoData_GeoBothReq_locSFOEmpty_locSYDloc()
  {
    // pass on loc1
    getGeoDataData(RuleConst::TSI_GEO_BOTH_REQUIRED);
    _locKey1.loc() = "";
    assertGetGeoDataLoc(_trx, true);
  }
  void testGetGeoData_GeoBothReq_locSFOloc_locSYDEmpty()
  {
    // pass on loc2
    getGeoDataData(RuleConst::TSI_GEO_BOTH_REQUIRED);
    _locKey1.loc() = "";
    assertGetGeoDataLoc(_trx, true);
  }
  void testGetGeoData_GeoBothReq_locSFOloc_locSYDloc()
  {
    // pass on both
    getGeoDataData(RuleConst::TSI_GEO_BOTH_REQUIRED);
    assertGetGeoDataLoc(_trx, true);
  }
  void testGetGeoData_GeotypeReq_locSFONone_locSYDNone()
  {
    // if bothnot set, return false
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    _locKey1.locType() = _locKey2.locType() = LOCTYPE_NONE;
    assertGetGeoDataLoc(_trx, false);
  }
  void testGetGeoData_GeotypeReq_locSFO_locSYD_NoGeoItin()
  {
    // if unable to get locataion from itin, fail
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    _tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_BLANK;
    assertGetGeoDataLoc(_trx, false);
  }
  void testGetGeoData_GeotypeReq_locSFOArea()
  {
    // get area for loc1
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    assertGetGeoDataLoc(_trx, true, LOCTYPE_AREA, "1", LOCTYPE_NONE, "");
  }
  void testGetGeoData_GeotypeReq_locSFOSubarea()
  {
    // get area for loc1
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    assertGetGeoDataLoc(_trx, true, LOCTYPE_SUBAREA, "11", LOCTYPE_NONE, "");
  }
  void testGetGeoData_GeotypeReq_locSFOCity()
  {
    // get city for loc1
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    assertGetGeoDataLoc(_trx, true, LOCTYPE_CITY, "SFO", LOCTYPE_NONE, "");
  }
  void testGetGeoData_GeotypeReq_locSFOAirport()
  {
    // get city for loc1
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    assertGetGeoDataLoc(_trx, true, LOCTYPE_AIRPORT, "SFO", LOCTYPE_NONE, "");
  }
  void testGetGeoData_GeotypeReq_locSFONation()
  {
    // get nation for loc1
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    assertGetGeoDataLoc(_trx, true, LOCTYPE_NATION, "US", LOCTYPE_NONE, "");
  }
  void testGetGeoData_GeotypeReq_locSFOState()
  {
    // get state for loc1
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    assertGetGeoDataLoc(_trx, true, LOCTYPE_STATE, "USCA", LOCTYPE_NONE, "");
  }
  void testGetGeoData_GeotypeReq_locSFOZone()
  {
    // zone is not supported
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    assertGetGeoDataLoc(_trx, false, LOCTYPE_ZONE, "", LOCTYPE_NONE, "");
  }
  void testGetGeoData_GeotypeReq_locSYDArea()
  {
    // get area for loc2
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    assertGetGeoDataLoc(_trx, true, LOCTYPE_NONE, "", LOCTYPE_AREA, "1");
  }
  void testGetGeoData_GeotypeReq_locSYDSubarea()
  {
    // get area for loc2
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    assertGetGeoDataLoc(_trx, true, LOCTYPE_NONE, "", LOCTYPE_SUBAREA, "11");
  }
  void testGetGeoData_GeotypeReq_locSYDCity()
  {
    // get city for loc2
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    assertGetGeoDataLoc(_trx, true, LOCTYPE_NONE, "", LOCTYPE_CITY, "SFO");
  }
  void testGetGeoData_GeotypeReq_locSYDAirport()
  {
    // get city for loc2
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    assertGetGeoDataLoc(_trx, true, LOCTYPE_NONE, "", LOCTYPE_AIRPORT, "SFO");
  }
  void testGetGeoData_GeotypeReq_locSYDNation()
  {
    // get nation for loc2
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    assertGetGeoDataLoc(_trx, true, LOCTYPE_NONE, "", LOCTYPE_NATION, "US");
  }
  void testGetGeoData_GeotypeReq_locSYDState()
  {
    // get state for loc2
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    assertGetGeoDataLoc(_trx, true, LOCTYPE_NONE, "", LOCTYPE_STATE, "USCA");
  }
  void testGetGeoData_GeotypeReq_locSYDZone()
  {
    // zone is not supported
    getGeoDataData(RuleConst::TSI_GEO_GEOTYPE_REQUIRED);
    assertGetGeoDataLoc(_trx, false, LOCTYPE_NONE, "", LOCTYPE_ZONE, "");
  }
  void testGetGeoData_GetFromItin_NoGetFromItin()
  {
    // if unable to get locataion from itin, fail
    getGeoDataData(RuleConst::TSI_GEO_GET_FROM_ITIN);
    _tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_BLANK;
    assertGetGeoDataLoc(_trx, false);
  }
  void testGetGeoData_GetFromItin_GetFromItin()
  {
    // if get loc, then use nation
    getGeoDataData(RuleConst::TSI_GEO_GET_FROM_ITIN);
    assertGetGeoDataLoc(_trx, true, LOCTYPE_NATION, "US", LOCTYPE_NONE, "");
  }
  void testGetGeoData_GeoBlank()
  {
    // for blank, all loc information is copied
    getGeoDataData(RuleConst::TSI_GEO_BLANK);
    assertGetGeoDataLoc(_trx, true);
  }
  void setUp()
  {
    RuleUtilTSITest_Base::setUp();
    _vendor = "ATP";
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_ORIG;
    _tsi = new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, &_itin, 0, 0);
    _memHandle.insert(_tsi);
    _locKey1.locType() = LOCTYPE_NATION;
    _locKey2.locType() = LOCTYPE_NATION;
    _locKey1.loc() = "US";
    _locKey2.loc() = "AU";
    _loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    _airSeg.origin() = _loc;
    _itin.travelSeg().push_back(&_airSeg);
  }

private:
  void getGeoDataData(RuleConst::TSIGeoRequired req = RuleConst::TSI_GEO_BLANK)
  {
    _tsiInfo.geoRequired() = req;
  }
  void assertGetGeoDataLoc(PricingTrx* trx, bool result)
  {
    CPPUNIT_ASSERT_EQUAL(result, RuleUtilTSI::getGeoData(*trx, *_tsi, _locKey1, _locKey2));
    if (result)
    {
      CPPUNIT_ASSERT_EQUAL(_locKey1.locType(), _tsi->locType1());
      CPPUNIT_ASSERT_EQUAL(_locKey2.locType(), _tsi->locType2());
      CPPUNIT_ASSERT_EQUAL(_locKey1.loc(), _tsi->locCode1());
      CPPUNIT_ASSERT_EQUAL(_locKey2.loc(), _tsi->locCode2());
    }
  }
  void assertGetGeoDataLoc(
      PricingTrx* trx, bool result, LocTypeCode lt1, LocCode loc1, LocTypeCode lt2, LocCode loc2)
  {
    _locKey1.locType() = lt1;
    _locKey2.locType() = lt2;
    _locKey1.loc() = loc1;
    _locKey2.loc() = loc2;
    assertGetGeoDataLoc(trx, result);
  }
  RuleUtilTSI::TSIData* _tsi;
  TSIInfo _tsiInfo;
  VendorCode _vendor;
  RuleConst::TSIScopeType _tsiScope;
  Itin _itin;
  AirSeg _airSeg;
  LocKey _locKey1;
  LocKey _locKey2;
  Loc* _loc;
};

class RuleUtilTSITest_ReverseTravel : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_ReverseTravel);
  CPPUNIT_TEST(testReverseTravel_AAA_0);
  CPPUNIT_TEST(testReverseTravel_AAA_1);
  CPPUNIT_TEST(testReverseTravel_AAA_2);
  CPPUNIT_TEST(testReverseTravel_AKAK_0);
  CPPUNIT_TEST(testReverseTravel_AKAK_1);
  CPPUNIT_TEST(testReverseTravel_AKAK_2);
  CPPUNIT_TEST(testReverseTravel_AKAK_3);
  CPPUNIT_TEST(testReverseTravel_KKA_3);

  CPPUNIT_TEST(testGetReversedTravelSegs_A);
  CPPUNIT_TEST(testGetReversedTravelSegs_AA);
  CPPUNIT_TEST(testGetReversedTravelSegs_AAA);
  CPPUNIT_TEST(testGetReversedTravelSegs_AAK);
  CPPUNIT_TEST(testGetReversedTravelSegs_AKAK);
  CPPUNIT_TEST(testGetReversedTravelSegs_AAKK);
  CPPUNIT_TEST(testGetReversedTravelSegs_AAK_Stopover);
  CPPUNIT_TEST(testGetReversedTravelSegs_AAK_ForcedStopover);
  CPPUNIT_TEST(testGetReversedTravelSegs_AAK_ForcedConx);
  CPPUNIT_TEST(testGetReversedTravelSegs_AAK_FurthestPoint);

  CPPUNIT_TEST_SUITE_END();

public:
  void testReverseTravel_AAA_0()
  {
    // create 3 element vector of air segments, reverse third, and check results
    std::vector<TravelSeg*> vec;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(true, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(true, "JJJ", "KKK", "LLL", "MMM", 2));
    comapreReverseTravelSeg(RuleUtilTSI::reverseTravel(_trx->dataHandle(), vec, 0), vec[2], 0);
  }
  void testReverseTravel_AAA_1()
  {
    // create 3 element vector of air segments, reverse second, and check results
    std::vector<TravelSeg*> vec;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(true, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(true, "JJJ", "KKK", "LLL", "MMM", 2));
    comapreReverseTravelSeg(RuleUtilTSI::reverseTravel(_trx->dataHandle(), vec, 1), vec[1], 1);
  }
  void testReverseTravel_AAA_2()
  {
    // create 3 element vector of air segments, reverse first, and check results
    std::vector<TravelSeg*> vec;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(true, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(true, "JJJ", "KKK", "LLL", "MMM", 2));
    comapreReverseTravelSeg(RuleUtilTSI::reverseTravel(_trx->dataHandle(), vec, 2), vec[0], 2);
  }
  void testReverseTravel_AKAK_0()
  {
    // create 4 element vector of air/arunk segments, reverse forth, and check results
    std::vector<TravelSeg*> vec;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(false, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(true, "JJJ", "KKK", "LLL", "MMM", 2));
    vec.push_back(createTravelSeg(true, "NNN", "OOO", "PPP", "QQQ", 3));
    comapreReverseTravelSeg(RuleUtilTSI::reverseTravel(_trx->dataHandle(), vec, 0), vec[3], 0);
  }
  void testReverseTravel_AKAK_1()
  {
    // create 4 element vector of air/arunk segments, reverse third, and check results
    std::vector<TravelSeg*> vec;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(false, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(true, "JJJ", "KKK", "LLL", "MMM", 2));
    vec.push_back(createTravelSeg(true, "NNN", "OOO", "PPP", "QQQ", 3));
    comapreReverseTravelSeg(RuleUtilTSI::reverseTravel(_trx->dataHandle(), vec, 1), vec[2], 1);
  }
  void testReverseTravel_AKAK_2()
  {
    // create 4 element vector of air/arunk segments, reverse second, and check results
    std::vector<TravelSeg*> vec;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(false, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(true, "JJJ", "KKK", "LLL", "MMM", 2));
    vec.push_back(createTravelSeg(true, "NNN", "OOO", "PPP", "QQQ", 3));
    comapreReverseTravelSeg(RuleUtilTSI::reverseTravel(_trx->dataHandle(), vec, 2), vec[1], 2);
  }
  void testReverseTravel_AKAK_3()
  {
    // create 4 element vector of air/arunk segments, reverse first, and check results
    std::vector<TravelSeg*> vec;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(false, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(true, "JJJ", "KKK", "LLL", "MMM", 2));
    vec.push_back(createTravelSeg(false, "NNN", "OOO", "PPP", "QQQ", 3));
    comapreReverseTravelSeg(RuleUtilTSI::reverseTravel(_trx->dataHandle(), vec, 3), vec[0], 3);
  }
  void testReverseTravel_KKA_3()
  {
    // create 3 element vector of travel segments, request ot of range, nothing returned
    std::vector<TravelSeg*> vec;
    vec.push_back(createTravelSeg(false, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(false, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(true, "JJJ", "KKK", "LLL", "MMM", 2));
    CPPUNIT_ASSERT(0 == RuleUtilTSI::reverseTravel(_trx->dataHandle(), vec, 3));
  }
  void testGetReversedTravelSegs_A()
  {
    // create 1 element vector of travel segments, check if reversed correctly
    std::vector<TravelSeg*> vec, vecRev;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    CPPUNIT_ASSERT(RuleUtilTSI::getReversedTravelSegs(_trx->dataHandle(), vecRev, vec));
    comapreReverseTravelSeg(vec, vecRev);
  }
  void testGetReversedTravelSegs_AA()
  {
    // create 2 element vector of travel segments, check if reversed correctly
    std::vector<TravelSeg*> vec, vecRev;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(true, "FFF", "GGG", "HHH", "III", 1));
    CPPUNIT_ASSERT(RuleUtilTSI::getReversedTravelSegs(_trx->dataHandle(), vecRev, vec));
    comapreReverseTravelSeg(vec, vecRev);
  }
  void testGetReversedTravelSegs_AAA()
  {
    // create 3 element vector of travel segments, check if reversed correctly
    std::vector<TravelSeg*> vec, vecRev;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(true, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(true, "JJJ", "KKK", "LLL", "MMM", 2));
    CPPUNIT_ASSERT(RuleUtilTSI::getReversedTravelSegs(_trx->dataHandle(), vecRev, vec));
    comapreReverseTravelSeg(vec, vecRev);
  }
  void testGetReversedTravelSegs_AAK()
  {
    // create 3 element vector of travel segments, check if reversed correctly
    std::vector<TravelSeg*> vec, vecRev;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(true, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(true, "JJJ", "KKK", "LLL", "MMM", 2));
    CPPUNIT_ASSERT(RuleUtilTSI::getReversedTravelSegs(_trx->dataHandle(), vecRev, vec));
    comapreReverseTravelSeg(vec, vecRev);
  }
  void testGetReversedTravelSegs_AKAK()
  {
    // create 4 element vector of travel segments, check if reversed correctly
    std::vector<TravelSeg*> vec, vecRev;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(false, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(true, "JJJ", "KKK", "LLL", "MMM", 2));
    vec.push_back(createTravelSeg(false, "NNN", "OOO", "PPP", "QQQ", 4));
    CPPUNIT_ASSERT(RuleUtilTSI::getReversedTravelSegs(_trx->dataHandle(), vecRev, vec));
    comapreReverseTravelSeg(vec, vecRev);
  }
  void testGetReversedTravelSegs_AAKK()
  {
    // create 4 element vector of travel segments, check if reversed correctly
    std::vector<TravelSeg*> vec, vecRev;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(true, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(false, "JJJ", "KKK", "LLL", "MMM", 2));
    vec.push_back(createTravelSeg(false, "NNN", "OOO", "PPP", "QQQ", 4));
    CPPUNIT_ASSERT(RuleUtilTSI::getReversedTravelSegs(_trx->dataHandle(), vecRev, vec));
    comapreReverseTravelSeg(vec, vecRev);
  }
  void testGetReversedTravelSegs_AAK_Stopover()
  {
    // create 3 element vector of travel segments, add stopover, check if reversed correctly
    std::vector<TravelSeg*> vec, vecRev;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(true, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(false, "JJJ", "KKK", "LLL", "MMM", 2));
    vec[0]->stopOver() = true;
    CPPUNIT_ASSERT(RuleUtilTSI::getReversedTravelSegs(_trx->dataHandle(), vecRev, vec));
    comapreReverseTravelSeg(vec, vecRev);
  }
  void testGetReversedTravelSegs_AAK_ForcedStopover()
  {
    // create 3 element vector of travel segments, add forcedstopover, check if reversed correctly
    std::vector<TravelSeg*> vec, vecRev;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(true, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(false, "JJJ", "KKK", "LLL", "MMM", 2));
    vec[0]->forcedStopOver() = true;
    CPPUNIT_ASSERT(RuleUtilTSI::getReversedTravelSegs(_trx->dataHandle(), vecRev, vec));
    comapreReverseTravelSeg(vec, vecRev);
  }
  void testGetReversedTravelSegs_AAK_ForcedConx()
  {
    // create 3 element vector of travel segments, add forced connection, check if reversed
    // correctly
    std::vector<TravelSeg*> vec, vecRev;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(true, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(false, "JJJ", "KKK", "LLL", "MMM", 2));
    vec[0]->forcedConx() = true;
    CPPUNIT_ASSERT(RuleUtilTSI::getReversedTravelSegs(_trx->dataHandle(), vecRev, vec));
    comapreReverseTravelSeg(vec, vecRev);
  }
  void testGetReversedTravelSegs_AAK_FurthestPoint()
  {
    // create 3 element vector of travel segments, add furthestPoint, check if reversed correctly
    std::vector<TravelSeg*> vec, vecRev;
    vec.push_back(createTravelSeg(true, "AAA", "BBB", "CCC", "DDD", 0));
    vec.push_back(createTravelSeg(true, "FFF", "GGG", "HHH", "III", 1));
    vec.push_back(createTravelSeg(false, "JJJ", "KKK", "LLL", "MMM", 2));
    vec[0]->furthestPoint() = true;
    CPPUNIT_ASSERT(RuleUtilTSI::getReversedTravelSegs(_trx->dataHandle(), vecRev, vec));
    comapreReverseTravelSeg(vec, vecRev);
  }

private:
  TravelSeg*
  createTravelSeg(bool air, LocCode origin, LocCode dest, LocCode boardMc, LocCode offMC, char pnr)
  {
    TravelSeg* seg = air ? static_cast<TravelSeg*>(_memHandle.create<AirSeg>())
                         : static_cast<TravelSeg*>(_memHandle.create<ArunkSeg>());
    Loc* loc = _memHandle.create<Loc>();
    loc->loc() = origin;
    seg->origin() = loc;
    loc = _memHandle.create<Loc>();
    loc->loc() = dest;
    seg->destination() = loc;
    seg->boardMultiCity() = boardMc;
    seg->offMultiCity() = offMC;
    seg->pnrSegment() = pnr;
    seg->segmentType() = air ? Air : Arunk;
    seg->geoTravelType() = GeoTravelType::International;
    seg->origAirport() = origin;
    seg->destAirport() = dest;
    return seg;
  }
  void comapreReverseTravelSeg(TravelSeg* t1, TravelSeg* t2, short segOrder)
  {
    CPPUNIT_ASSERT((t1 != 0) && (t2 != 0));
    CPPUNIT_ASSERT_EQUAL(t1->origin()->loc(), t2->destination()->loc());
    CPPUNIT_ASSERT_EQUAL(t1->destination()->loc(), t2->origin()->loc());
    CPPUNIT_ASSERT(t1->geoTravelType() == t1->geoTravelType());
    CPPUNIT_ASSERT_EQUAL(t1->boardMultiCity(), t2->offMultiCity());
    CPPUNIT_ASSERT_EQUAL(t1->offMultiCity(), t2->boardMultiCity());
    CPPUNIT_ASSERT_EQUAL(t1->segmentType(), t2->segmentType());
    CPPUNIT_ASSERT_EQUAL(t1->origAirport(), t2->destAirport());
    CPPUNIT_ASSERT_EQUAL(t1->destAirport(), t2->origAirport());
    CPPUNIT_ASSERT_EQUAL(t1->pssDepartureDate(), t2->pssDepartureDate());
    CPPUNIT_ASSERT_EQUAL(t1->pssDepartureTime(), t2->pssDepartureTime());
    CPPUNIT_ASSERT_EQUAL(t1->pssArrivalDate(), t2->pssArrivalDate());
    CPPUNIT_ASSERT_EQUAL(t1->pssArrivalTime(), t2->pssArrivalTime());
    CPPUNIT_ASSERT_EQUAL(t1->departureDT(), t2->departureDT());
    CPPUNIT_ASSERT_EQUAL(t1->arrivalDT(), t2->arrivalDT());
    // tis is from forward
    CPPUNIT_ASSERT_EQUAL(segOrder, t1->pnrSegment());
  }
  void comapreReverseTravelSeg(std::vector<TravelSeg*>& s1, std::vector<TravelSeg*>& s2)
  {
    CPPUNIT_ASSERT_EQUAL(s1.size(), s2.size());
    std::vector<TravelSeg*>::iterator it = s1.begin();
    std::vector<TravelSeg*>::iterator ite = s1.end();
    std::vector<TravelSeg*>::reverse_iterator itr = s2.rbegin();
    std::vector<TravelSeg*>::reverse_iterator itre = s2.rend();
    for (; it != ite; it++, itr++)
    {
      comapreReverseTravelSeg(*it, *itr, (*it)->pnrSegment());
      if ((itr + 1) != itre)
      {
        CPPUNIT_ASSERT_EQUAL((*it)->stopOver(), (*(itr + 1))->stopOver());
        CPPUNIT_ASSERT_EQUAL((*it)->forcedStopOver(), (*(itr + 1))->forcedStopOver());
        CPPUNIT_ASSERT_EQUAL((*it)->forcedConx(), (*(itr + 1))->forcedConx());
        CPPUNIT_ASSERT_EQUAL((*it)->furthestPoint(), (*(itr + 1))->furthestPoint());
      }
    }
  }
};

class RuleUtilTSITest_GetTSIOrigDestCheck : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_GetTSIOrigDestCheck);
  CPPUNIT_TEST(testGetTSIOrigDestCheck_Orig);
  CPPUNIT_TEST(testGetTSIOrigDestCheck_Dest);
  CPPUNIT_TEST(testGetTSIOrigDestCheck_Both);
  CPPUNIT_TEST(testGetTSIOrigDestCheck_Blank);

  CPPUNIT_TEST(testGetCheckOrigDest_CkeckOrig);
  CPPUNIT_TEST(testGetCheckOrigDest_CkeckDest);
  CPPUNIT_TEST(testGetCheckOrigDest_CkeckBoth);
  CPPUNIT_TEST(testGetCheckOrigDest_CkeckBlank);

  CPPUNIT_TEST(testGetDirectionsFromLoop_LoopFrwd);
  CPPUNIT_TEST(testGetDirectionsFromLoop_LoopFrwdBrk);
  CPPUNIT_TEST(testGetDirectionsFromLoop_LoopBack);
  CPPUNIT_TEST(testGetDirectionsFromLoop_LoopBackBrk);
  CPPUNIT_TEST(testGetDirectionsFromLoop_LoopBlank);

  CPPUNIT_TEST_SUITE_END();

public:
  void testGetTSIOrigDestCheck_Orig()
  {
    // for orig only orig set to true
    _tsiInfo.type() = RuleConst::TSI_APP_CHECK_ORIG;
    assertGetTSIOrigDestCheck(true, true, false);
  }
  void testGetTSIOrigDestCheck_Dest()
  {
    // for dest only dest set to true
    _tsiInfo.type() = RuleConst::TSI_APP_CHECK_DEST;
    assertGetTSIOrigDestCheck(true, false, true);
  }
  void testGetTSIOrigDestCheck_Both()
  {
    // for orig dest both set to true
    _tsiInfo.type() = RuleConst::TSI_APP_CHECK_ORIG_DEST;
    assertGetTSIOrigDestCheck(true, true, true);
  }
  void testGetTSIOrigDestCheck_Blank()
  {
    // for blank, return false
    _tsiInfo.type() = (RuleConst::TSIApplicationType)' ';
    assertGetTSIOrigDestCheck(false);
  }

  void testGetCheckOrigDest_CkeckOrig()
  {
    // only origin set
    _tsiInfo.type() = RuleConst::TSI_APP_CHECK_ORIG;
    assertGetCheckOrigDest(true, false);
  }
  void testGetCheckOrigDest_CkeckDest()
  {
    // only destinatin set
    _tsiInfo.type() = RuleConst::TSI_APP_CHECK_DEST;
    assertGetCheckOrigDest(false, true);
  }
  void testGetCheckOrigDest_CkeckBoth()
  {
    // orig and destination set
    _tsiInfo.type() = RuleConst::TSI_APP_CHECK_ORIG_DEST;
    assertGetCheckOrigDest(true, true);
  }
  void testGetCheckOrigDest_CkeckBlank()
  {
    // none set
    _tsiInfo.type() = (RuleConst::TSIApplicationType)' ';
    assertGetCheckOrigDest(false, false);
  }

  void testGetDirectionsFromLoop_LoopFrwd()
  {
    // succedd, only forward
    _tsiInfo.loopDirection() = RuleConst::TSI_LOOP_FORWARD;
    assertGetDirectionsFromLoop(true, true, false);
  }
  void testGetDirectionsFromLoop_LoopFrwdBrk()
  {
    // succedd, forward & break
    _tsiInfo.loopDirection() = RuleConst::TSI_LOOP_FORWARD_BREAK_ARRIVAL;
    assertGetDirectionsFromLoop(true, true, true);
  }
  void testGetDirectionsFromLoop_LoopBack()
  {
    // succedd, only backward
    _tsiInfo.loopDirection() = RuleConst::TSI_LOOP_BACKWARD;
    assertGetDirectionsFromLoop(true, false, false);
  }
  void testGetDirectionsFromLoop_LoopBackBrk()
  {
    // succedd, backward and break
    _tsiInfo.loopDirection() = RuleConst::TSI_LOOP_BACKWARD_BREAK_DEPART;
    assertGetDirectionsFromLoop(true, false, true);
  }
  void testGetDirectionsFromLoop_LoopBlank()
  {
    // unknown value - fail
    _tsiInfo.loopDirection() = ' ';
    assertGetDirectionsFromLoop(false, false, false);
  }

  void setUp()
  {
    RuleUtilTSITest_Base::setUp();
    _tsi = _memHandle.insert(new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, 0, 0, 0));
  }

private:
  void assertGetDirectionsFromLoop(bool ret, bool fwrd, bool brk)
  {
    bool loopFwd, loobBrk;
    CPPUNIT_ASSERT_EQUAL(ret,
                         RuleUtilTSI::getDirectionsFromLoop(*_trx, _tsiInfo, loopFwd, loobBrk));
    if (ret)
    {
      CPPUNIT_ASSERT_EQUAL(fwrd, loopFwd);
      CPPUNIT_ASSERT_EQUAL(brk, loobBrk);
    }
  }
  void assertGetTSIOrigDestCheck(bool result, bool checkOrig = false, bool checkDest = false)
  {
    bool orig, dest;
    CPPUNIT_ASSERT_EQUAL(result, RuleUtilTSI::getTSIOrigDestCheck(&_tsiInfo, orig, dest));
    if (result)
    {
      CPPUNIT_ASSERT_EQUAL(checkOrig, orig);
      CPPUNIT_ASSERT_EQUAL(checkDest, dest);
    }
  }
  void assertGetCheckOrigDest(bool orig, bool dest)
  {
    bool origCheck, destCheck;
    PricingTrx trx;
    PricingOptions opt;
    trx.setOptions(&opt);
    RuleUtilTSI::TSITravelSegMarkup markup;
    std::string reason;

    RuleUtilTSI::TSIMatchCriteria tsiMC(trx, *_tsi, markup, reason);
    tsiMC.getCheckOrigDest(origCheck, destCheck);
    CPPUNIT_ASSERT_EQUAL(orig, origCheck);
    CPPUNIT_ASSERT_EQUAL(dest, destCheck);
  }
  RuleUtilTSI::TSIData* _tsi;
  TSIInfo _tsiInfo;
  VendorCode _vendor;
  RuleConst::TSIScopeType _tsiScope;
};

class RuleUtilTSITest_IdentifyIntlDomTransfers : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_IdentifyIntlDomTransfers);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_ABA1I_ABA2D_ABA3I_ABA4D);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_K_ABA1I_ABA2D_ABA3I);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_ABA1I_K_ABA2D_ABA3I);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_ABA1I_ABA2D_K_ABA3I);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_ABA1I_ABA2D_ABA3I_K);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_K_K_ABA1I_ABA2D);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_ABA1I_K_K_ABA2D);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_ABA1I_ABA2D_K_K);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_ABA1I_K);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_ABA1I_ABA1D);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_ABA1I_ABA2I);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_ALO1I_ALO1D);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_ALO1I_ALO2I_ALO3I_ALO4D_ALO5D);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_ALO1I_ALO1D_fareBreak);
  CPPUNIT_TEST(testIdentifyIntlDomTransfers_ALO1I_ALO1D_NoMatchCriteria);
  CPPUNIT_TEST_SUITE_END();

public:
  void testIdentifyIntlDomTransfers_ABA1I_ABA2D_ABA3I_ABA4D()
  {
    // different flight number, transer on all air segments
    testIntDomTransfData("ABA1I_ABA2D_ABA3I_ABA4D");
    assertIdentifyIntlDomTransfers(3, 0, 1, 2);
  }
  void testIdentifyIntlDomTransfers_K_ABA1I_ABA2D_ABA3I()
  {
    // different flight number, transer on all air segments
    testIntDomTransfData("K_ABA1I_ABA2D_ABA3I");
    assertIdentifyIntlDomTransfers(2, 1, 2);
  }
  void testIdentifyIntlDomTransfers_ABA1I_K_ABA2D_ABA3I()
  {
    // different flight number, transer on all air segments
    testIntDomTransfData("ABA1I_K_ABA2D_ABA3I");
    assertIdentifyIntlDomTransfers(2, 0, 2);
  }
  void testIdentifyIntlDomTransfers_ABA1I_ABA2D_K_ABA3I()
  {
    // different flight number, transer on all air segments
    testIntDomTransfData("ABA1I_ABA2D_K_ABA3I");
    assertIdentifyIntlDomTransfers(2, 0, 1);
  }
  void testIdentifyIntlDomTransfers_ABA1I_ABA2D_ABA3I_K()
  {
    // different flight number, transer on all air segments
    testIntDomTransfData("ABA1I_ABA2D_ABA3I_K");
    assertIdentifyIntlDomTransfers(2, 0, 1);
  }
  void testIdentifyIntlDomTransfers_K_K_ABA1I_ABA2D()
  {
    // different flight number, transer on all air segments
    testIntDomTransfData("K_K_ABA1I_ABA2D");
    assertIdentifyIntlDomTransfers(1, 2);
  }
  void testIdentifyIntlDomTransfers_ABA1I_K_K_ABA2D()
  {
    // different flight number, transer on all air segments
    testIntDomTransfData("ABA1I_K_K_ABA2D");
    assertIdentifyIntlDomTransfers(1, 0);
  }
  void testIdentifyIntlDomTransfers_ABA1I_ABA2D_K_K()
  {
    // different flight number, transer on all air segments
    testIntDomTransfData("ABA1I_ABA2D_K_K");
    assertIdentifyIntlDomTransfers(1, 0);
  }
  void testIdentifyIntlDomTransfers_ABA1I_K()
  {
    // only 1 air seg, no transfers
    testIntDomTransfData("ABA1I_K");
    assertIdentifyIntlDomTransfers(0);
  }
  void testIdentifyIntlDomTransfers_ABA1I_ABA1D()
  {
    // same carriers and flight noumbers - no transfer
    testIntDomTransfData("ABA1I_ABA1D");
    assertIdentifyIntlDomTransfers(0);
  }
  void testIdentifyIntlDomTransfers_ABA1I_ABA2I()
  {
    // all segmenst international - no transfer
    testIntDomTransfData("ABA1I_ABA2I");
    assertIdentifyIntlDomTransfers(0);
  }
  void testIdentifyIntlDomTransfers_ALO1I_ALO1D()
  {
    // same carriers and flight noumbers - no transfer
    testIntDomTransfData("ALO1I_ALO1D");
    assertIdentifyIntlDomTransfers(0);
  }
  void testIdentifyIntlDomTransfers_ALO1I_ALO2I_ALO3I_ALO4D_ALO5D()
  {
    // transfer from int to dom on 2nd seg only
    testIntDomTransfData("ALO1I_ALO2I_ALO3I_ALO4D_ALO5D");
    assertIdentifyIntlDomTransfers(1, 2);
  }
  void testIdentifyIntlDomTransfers_ALO1I_ALO1D_fareBreak()
  {
    // fareBreakAtDestination set - no transfer
    testIntDomTransfData("ALO1I_ALO1D");
    _markVec.begin()->fareBreakAtDestination() = true;
    assertIdentifyIntlDomTransfers(0);
  }
  void testIdentifyIntlDomTransfers_ALO1I_ALO1D_NoMatchCriteria()
  {
    // no match criteria - no transfer
    testIntDomTransfData("ALO1I_ALO1D");
    _tsiInfo.matchCriteria().pop_back(); // remove TSIInfo::INTL_DOM_TRANSFER
    assertIdentifyIntlDomTransfers(0);
  }

  void setUp()
  {
    RuleUtilTSITest_Base::setUp();
    _vendor = "ATP";
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsi = new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, 0, 0, 0);
    _memHandle.insert(_tsi);
    _tsiInfo.matchCriteria() += TSIInfo::STOP_OVER, TSIInfo::INBOUND, TSIInfo::OUTBOUND,
        TSIInfo::INTERNATIONAL, TSIInfo::INTL_DOM_TRANSFER;
  }

private:
  // to create air seg pass A +cxr+flight noumber+DomInd (5bytes) like ABA1D
  // create domestic BA segment with flight noumber 1
  // to creat arunk just pass K,
  // all segments separated by one char. String in not validated!!
  void testIntDomTransfData(const char* segs)
  {
    std::vector<TravelSeg*> travelSegs;
    TravelSeg* seg;
    for (int i = 0; segs[i] != 0; i++)
    {
      if (segs[i] != 'A')
      {
        seg = _memHandle.create<ArunkSeg>();
      }
      else
      {
        seg = _memHandle.create<AirSeg>();
        char cxr[3] = {};
        strncpy(cxr, &segs[i + 1], 2);
        static_cast<AirSeg*>(seg)->carrier() = cxr;
        static_cast<AirSeg*>(seg)->flightNumber() = segs[i + 3] - '0';
        static_cast<AirSeg*>(seg)->bbrCarrier() = segs[i + 4] == 'I';
        i += 4;
      }
      if (segs[i + 1])
        i++; // separator skip
      travelSegs.push_back(seg);
    }
    std::vector<TravelSeg*>::iterator ib = travelSegs.begin();
    std::vector<TravelSeg*>::iterator ibe = travelSegs.end();
    for (; ib != ibe; ib++)
    {
      RuleUtilTSI::TSITravelSegMarkup tsm;
      tsm.travelSeg() = *ib;
      if ((ib + 1) != ibe)
        tsm.nextTravelSeg() = *(ib + 1);
      tsm.fareBreakAtDestination() = false;
      if (dynamic_cast<AirSeg*>(*ib))
        tsm.isInternational() = dynamic_cast<AirSeg*>(*ib)->bbrCarrier();
      _markVec.push_back(tsm);
    }
  }
  void assertIdentifyIntlDomTransfers(int listLen, ...)
  {
    va_list args;
    va_start(args, listLen);
    std::set<int> list;
    for (int i = 0; i < listLen; i++)
      list.insert(va_arg(args, int));
    va_end(args);
    RuleUtilTSI::identifyIntlDomTransfers(*_tsi, _markVec);
    // for each segment check if flag is set (is segment index passed to the function)
    RuleUtilTSI::TSITravelSegMarkupContainer::iterator it = _markVec.begin();
    RuleUtilTSI::TSITravelSegMarkupContainer::iterator ite = _markVec.end();
    for (int i = 0; it != ite; i++, it++)
    {
      char msg[40];
      sprintf(msg, "failed on segment: %i, set to %i", i, it->isIntlDomTransfer());
      CPPUNIT_ASSERT_MESSAGE(msg, it->isIntlDomTransfer() == (list.find(i) != list.end()));
    }
  }
  RuleUtilTSI::TSIData* _tsi;
  TSIInfo _tsiInfo;
  VendorCode _vendor;
  RuleConst::TSIScopeType _tsiScope;
  RuleUtilTSI::TSITravelSegMarkupContainer _markVec;
};

class RuleUtilTSITest_ProcessTravelSegs : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_ProcessTravelSegs);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchAll_2);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchAll_3);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchAll_5);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchFirst_2);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchFirst_3);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchFirst_5);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchOnce_2);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchOnce_3);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchOnce_5);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchLast_2);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchLast_3);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchLast_5);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchFirstAll_2);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchFirstAll_3);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchFirstAll_5);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchSoft_2);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchSoft_3);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchSoft_5);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchSecondFirst_2);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchSecondFirst_3);
  CPPUNIT_TEST(testProcessTravelSegsSetupMatchCriteria_MatchSecondFirst_5);

  CPPUNIT_TEST(testProcessTravelSegsSetupTSIMatch_MatchAll_PassTsiTrvSeg);
  CPPUNIT_TEST(testProcessTravelSegsSetupTSIMatch_MatchAll_SoftTsiTrvSeg);
  CPPUNIT_TEST(testProcessTravelSegsSetupTSIMatch_MatchAll_FailTsiTrvSeg);
  CPPUNIT_TEST(testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevPass_PassTsiTrvSeg);
  CPPUNIT_TEST(testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevSoft_PassTsiTrvSeg);
  CPPUNIT_TEST(testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevFail_PassTsiTrvSeg);
  CPPUNIT_TEST(testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevPass_SoftTsiTrvSeg);
  CPPUNIT_TEST(testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevSoft_SoftTsiTrvSeg);
  CPPUNIT_TEST(testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevFail_SoftTsiTrvSeg);
  CPPUNIT_TEST(testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevPass_FailTsiTrvSeg);
  CPPUNIT_TEST(testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevSoft_FailTsiTrvSeg);
  CPPUNIT_TEST(testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevFail_FailTsiTrvSeg);

  CPPUNIT_TEST(testProcessTravelSegsAddPrev_MatchOrig_Save);
  CPPUNIT_TEST(testProcessTravelSegsAddPrev_MatchOrig_NoSave);
  CPPUNIT_TEST(testProcessTravelSegsAddPrev_NoMatchOrig_Save);
  CPPUNIT_TEST(testProcessTravelSegsAddPrev_NoMatchOrig_NoSave);
  CPPUNIT_TEST(testProcessTravelSegsAddCurr_MatchOrig_Save);
  CPPUNIT_TEST(testProcessTravelSegsAddCurr_MatchOrig_NoSave);
  CPPUNIT_TEST(testProcessTravelSegsAddCurr_NoMatchOrig_Save);
  CPPUNIT_TEST(testProcessTravelSegsAddCurr_NoMatchOrig_NoSave);
  CPPUNIT_TEST(testProcessTravelSegsAddNext_MatchOrig_Save);
  CPPUNIT_TEST(testProcessTravelSegsAddNext_MatchOrig_NoSave);
  CPPUNIT_TEST(testProcessTravelSegsAddNext_NoMatchOrig_Save);
  CPPUNIT_TEST(testProcessTravelSegsAddNext_NoMatchOrig_NoSave);

  CPPUNIT_TEST(testProcessTravelSegsForward_MatchAll_LoopCurr);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchAll_LoopPrev);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchAll_LoopNext);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchAll_LoopCurrPrev);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchAll_LoopCurrNext);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchAll_FirstFail);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchAll_FareBreakOrig_ChkOrig);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchAll_FareBreakOrig_NoChkOrig);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchAll_FareBreakDest_ChkDest);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchAll_FareBreakDest_NoChkDest);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchAll_LoopOffset);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchSecFirst_LoopCurr);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchSecFirst_LoopPrev);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchSecFirst_LoopNext);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchSecFirst_LoopCurrPrev);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchSecFirst_LoopCurrNext);
  CPPUNIT_TEST(testProcessTravelSegsForward_MatchSecFirst_FirstFail);

  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchAll_LoopCurr);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchAll_LoopPrev);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchAll_LoopNext);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchAll_LoopCurrPrev);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchAll_LoopCurrNext);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchAll_FirstFail);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchAll_FareBreakOrig_ChkOrig);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchAll_FareBreakOrig_NoChkOrig);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchAll_FareBreakDest_ChkDest);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchAll_FareBreakDest_NoChkDest);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchAll_LoopOffset);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchSecFirst_LoopCurr);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchSecFirst_LoopPrev);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchSecFirst_LoopNext);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchSecFirst_LoopCurrPrev);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchSecFirst_LoopCurrNext);
  CPPUNIT_TEST(testProcessTravelSegsBackward_MatchSecFirst_FirstFail);

  CPPUNIT_TEST(testProcessTravelSegs_Fwrd_GeoOrig_TypeOrig);
  CPPUNIT_TEST(testProcessTravelSegs_Fwrd_GeoOrig_TypeDest);
  CPPUNIT_TEST(testProcessTravelSegs_Fwrd_GeoOrig_TypeBoth);
  CPPUNIT_TEST(testProcessTravelSegs_Fwrd_GeoDest_TypeOrig);
  CPPUNIT_TEST(testProcessTravelSegs_Fwrd_GeoDest_TypeDest);
  CPPUNIT_TEST(testProcessTravelSegs_Fwrd_GeoDest_TypeBoth);
  CPPUNIT_TEST(testProcessTravelSegs_Fwrd_GeoBoth_TypeOrig);
  CPPUNIT_TEST(testProcessTravelSegs_Fwrd_GeoBoth_TypeDest);
  CPPUNIT_TEST(testProcessTravelSegs_Fwrd_GeoBoth_TypeBoth);
  CPPUNIT_TEST(testProcessTravelSegs_FwrdBrk_GeoOrig_TypeOrig);
  CPPUNIT_TEST(testProcessTravelSegs_FwrdBrk_GeoOrig_TypeDest);
  CPPUNIT_TEST(testProcessTravelSegs_FwrdBrk_GeoOrig_TypeBoth);
  CPPUNIT_TEST(testProcessTravelSegs_FwrdBrk_GeoDest_TypeOrig);
  CPPUNIT_TEST(testProcessTravelSegs_FwrdBrk_GeoDest_TypeDest);
  CPPUNIT_TEST(testProcessTravelSegs_FwrdBrk_GeoDest_TypeBoth);
  CPPUNIT_TEST(testProcessTravelSegs_FwrdBrk_GeoBoth_TypeOrig);
  CPPUNIT_TEST(testProcessTravelSegs_FwrdBrk_GeoBoth_TypeDest);
  CPPUNIT_TEST(testProcessTravelSegs_FwrdBrk_GeoBoth_TypeBoth);
  CPPUNIT_TEST(testProcessTravelSegs_Back_GeoOrig_TypeOrig);
  CPPUNIT_TEST(testProcessTravelSegs_Back_GeoOrig_TypeDest);
  CPPUNIT_TEST(testProcessTravelSegs_Back_GeoOrig_TypeBoth);
  CPPUNIT_TEST(testProcessTravelSegs_Back_GeoDest_TypeOrig);
  CPPUNIT_TEST(testProcessTravelSegs_Back_GeoDest_TypeDest);
  CPPUNIT_TEST(testProcessTravelSegs_Back_GeoDest_TypeBoth);
  CPPUNIT_TEST(testProcessTravelSegs_Back_GeoBoth_TypeOrig);
  CPPUNIT_TEST(testProcessTravelSegs_Back_GeoBoth_TypeDest);
  CPPUNIT_TEST(testProcessTravelSegs_Back_GeoBoth_TypeBoth);
  CPPUNIT_TEST(testProcessTravelSegs_BackBrk_GeoOrig_TypeOrig);
  CPPUNIT_TEST(testProcessTravelSegs_BackBrk_GeoOrig_TypeDest);
  CPPUNIT_TEST(testProcessTravelSegs_BackBrk_GeoOrig_TypeBoth);
  CPPUNIT_TEST(testProcessTravelSegs_BackBrk_GeoDest_TypeOrig);
  CPPUNIT_TEST(testProcessTravelSegs_BackBrk_GeoDest_TypeDest);
  CPPUNIT_TEST(testProcessTravelSegs_BackBrk_GeoDest_TypeBoth);
  CPPUNIT_TEST(testProcessTravelSegs_BackBrk_GeoBoth_TypeOrig);
  CPPUNIT_TEST(testProcessTravelSegs_BackBrk_GeoBoth_TypeDest);
  CPPUNIT_TEST(testProcessTravelSegs_BackBrk_GeoBoth_TypeBoth);
  CPPUNIT_TEST_SUITE_END();

public:
  void testProcessTravelSegsSetupMatchCriteria_MatchAll_2()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_ALL, 2, 'S', 'I');
    assertPTSMatchCriteria(tsiInfo, true, 2);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchAll_3()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_ALL, 3, 'S', 'I', 'O');
    assertPTSMatchCriteria(tsiInfo, true, 3);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchAll_5()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_ALL, 5, 'S', 'I', 'O', 'F', 'D');
    assertPTSMatchCriteria(tsiInfo, true, 5);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchFirst_2()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_FIRST, 2, 'S', 'I');
    assertPTSMatchCriteria(tsiInfo, true, 2);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchFirst_3()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_FIRST, 3, 'S', 'I', 'O');
    assertPTSMatchCriteria(tsiInfo, true, 3);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchFirst_5()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_FIRST, 5, 'S', 'I', 'O', 'F', 'D');
    assertPTSMatchCriteria(tsiInfo, true, 5);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchOnce_2()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_ONCE, 2, 'S', 'I');
    assertPTSMatchCriteria(tsiInfo, true, 2);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchOnce_3()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_ONCE, 3, 'S', 'I', 'O');
    assertPTSMatchCriteria(tsiInfo, true, 3);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchOnce_5()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_ONCE, 5, 'S', 'I', 'O', 'F', 'D');
    assertPTSMatchCriteria(tsiInfo, true, 5);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchLast_2()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_LAST, 2, 'S', 'I');
    assertPTSMatchCriteria(tsiInfo, true, 2);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchLast_3()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_LAST, 3, 'S', 'I', 'O');
    assertPTSMatchCriteria(tsiInfo, true, 3);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchLast_5()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_LAST, 5, 'S', 'I', 'O', 'F', 'D');
    assertPTSMatchCriteria(tsiInfo, true, 5);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchFirstAll_2()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_FIRST_ALL, 2, 'S', 'I');
    assertPTSMatchCriteria(tsiInfo, true, 2);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchFirstAll_3()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_FIRST_ALL, 3, 'S', 'I', 'O');
    assertPTSMatchCriteria(tsiInfo, true, 3);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchFirstAll_5()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo =
        getPTSMatchCriteria(RuleConst::TSI_MATCH_FIRST_ALL, 5, 'S', 'I', 'O', 'F', 'D');
    assertPTSMatchCriteria(tsiInfo, true, 5);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchSoft_2()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_SOFT_MATCH, 2, 'S', 'I');
    assertPTSMatchCriteria(tsiInfo, true, 2);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchSoft_3()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_SOFT_MATCH, 3, 'S', 'I', 'O');
    assertPTSMatchCriteria(tsiInfo, true, 3);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchSoft_5()
  {
    // all match criteria returned in first vector
    TSIInfo* tsiInfo =
        getPTSMatchCriteria(RuleConst::TSI_MATCH_SOFT_MATCH, 5, 'S', 'I', 'O', 'F', 'D');
    assertPTSMatchCriteria(tsiInfo, true, 5);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchSecondFirst_2()
  {
    // match second firstshould have only 3 segments - fail
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_SECOND_FIRST, 2, 'S', 'I');
    assertPTSMatchCriteria(tsiInfo, false);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchSecondFirst_3()
  {
    // first two segments in v1, third in v2
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_SECOND_FIRST, 3, 'S', 'I', 'O');
    assertPTSMatchCriteria(tsiInfo, true, 2, 1);
  }
  void testProcessTravelSegsSetupMatchCriteria_MatchSecondFirst_5()
  {
    // match second firstshould have only 3 segments - fail
    TSIInfo* tsiInfo = getPTSMatchCriteria(RuleConst::TSI_MATCH_SECOND_FIRST, 2, 'S', 'I');
    assertPTSMatchCriteria(tsiInfo, false);
  }
  void testProcessTravelSegsSetupTSIMatch_MatchAll_PassTsiTrvSeg()
  {
    // match on checkTSIMatchTravelSeg.
    travelSegTSIMatchData(RuleConst::TSI_MATCH_ALL);
    assertProcessTravSegTSIMatch(true, false, "", RuleConst::TSI_MATCH, RuleConst::TSI_NOT_MATCH);
  }
  void testProcessTravelSegsSetupTSIMatch_MatchAll_SoftTsiTrvSeg()
  {
    // soft match on checkTSIMatchTravelSeg.
    travelSegTSIMatchData(RuleConst::TSI_MATCH_ALL);
    _tsi->locCode1() = "PL";
    assertProcessTravSegTSIMatch(
        false, false, "LOCALE ", RuleConst::TSI_SOFT_MATCH, RuleConst::TSI_NOT_MATCH);
  }
  void testProcessTravelSegsSetupTSIMatch_MatchAll_FailTsiTrvSeg()
  {
    // no match on checkTSIMatchTravelSeg.
    travelSegTSIMatchData(RuleConst::TSI_MATCH_ALL);
    _mc1.push_back(TSIInfo::OVER_WATER);
    assertProcessTravSegTSIMatch(true,
                                 false,
                                 RuleConst::MATCH_OVER_WATER_DESC,
                                 RuleConst::TSI_NOT_MATCH,
                                 RuleConst::TSI_NOT_MATCH);
  }
  void testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevPass_PassTsiTrvSeg()
  {
    // match on checkTSIMatchTravelSeg.
    travelSegTSIMatchData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_MATCH);
    assertProcessTravSegTSIMatch(true, false, "", RuleConst::TSI_MATCH, RuleConst::TSI_MATCH);
  }
  void testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevSoft_PassTsiTrvSeg()
  {
    // match on checkTSIMatchTravelSeg.
    travelSegTSIMatchData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_SOFT_MATCH);
    assertProcessTravSegTSIMatch(true, false, "", RuleConst::TSI_MATCH, RuleConst::TSI_SOFT_MATCH);
  }
  void testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevFail_PassTsiTrvSeg()
  {
    // match on checkTSIMatchTravelSeg. validate second
    travelSegTSIMatchData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_NOT_MATCH);
    assertProcessTravSegTSIMatch(
        true, false, "- 1ST -", RuleConst::TSI_NOT_MATCH, RuleConst::TSI_MATCH);
  }
  void testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevPass_SoftTsiTrvSeg()
  {
    // soft match on checkTSIMatchTravelSeg.
    travelSegTSIMatchData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_MATCH);
    _tsi->locCode1() = "PL";
    assertProcessTravSegTSIMatch(
        false, false, "LOCALE ", RuleConst::TSI_SOFT_MATCH, RuleConst::TSI_MATCH);
  }
  void testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevSoft_SoftTsiTrvSeg()
  {
    // soft match on checkTSIMatchTravelSeg.
    travelSegTSIMatchData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_SOFT_MATCH);
    _tsi->locCode1() = "PL";
    assertProcessTravSegTSIMatch(
        false, false, "LOCALE ", RuleConst::TSI_SOFT_MATCH, RuleConst::TSI_SOFT_MATCH);
  }
  void testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevFail_SoftTsiTrvSeg()
  {
    // soft match on checkTSIMatchTravelSeg.
    travelSegTSIMatchData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_NOT_MATCH);
    _tsi->locCode1() = "PL";
    assertProcessTravSegTSIMatch(
        false, false, "- 1ST -", RuleConst::TSI_NOT_MATCH, RuleConst::TSI_SOFT_MATCH);
  }
  void testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevPass_FailTsiTrvSeg()
  {
    // no match on checkTSIMatchTravelSeg. validate using mc2
    travelSegTSIMatchData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_MATCH);
    _mc2.push_back(TSIInfo::OVER_WATER);
    assertProcessTravSegTSIMatch(true,
                                 false,
                                 RuleConst::MATCH_OVER_WATER_DESC,
                                 RuleConst::TSI_NOT_MATCH,
                                 RuleConst::TSI_MATCH);
  }
  void testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevSoft_FailTsiTrvSeg()
  {
    // no match on checkTSIMatchTravelSeg. validate using mc2
    travelSegTSIMatchData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_SOFT_MATCH);
    _mc2.push_back(TSIInfo::OVER_WATER);
    assertProcessTravSegTSIMatch(true,
                                 false,
                                 RuleConst::MATCH_OVER_WATER_DESC,
                                 RuleConst::TSI_NOT_MATCH,
                                 RuleConst::TSI_SOFT_MATCH);
  }
  void testProcessTravelSegsSetupTSIMatch_MatchSecondFirst_PrevFail_FailTsiTrvSeg()
  {
    // no match on checkTSIMatchTravelSeg. validate using mc1
    travelSegTSIMatchData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_NOT_MATCH);
    _mc1.push_back(TSIInfo::OVER_WATER);
    assertProcessTravSegTSIMatch(true,
                                 false,
                                 RuleConst::MATCH_OVER_WATER_DESC,
                                 RuleConst::TSI_NOT_MATCH,
                                 RuleConst::TSI_NOT_MATCH);
  }
  void testProcessTravelSegsAddPrev_MatchOrig_Save()
  {
    // check if get corectly filled segment wrapper
    RuleUtilTSI::TravelSegWrapper* tsw =
        RuleUtilTSI::processTravelSegsAddPrev(*_trx, _tsm1, true, true);
    CPPUNIT_ASSERT(_tsm1.destSave());
    CPPUNIT_ASSERT(tsw->destMatch());
    CPPUNIT_ASSERT(_tsm1.travelSeg() == tsw->travelSeg());
  }
  void testProcessTravelSegsAddPrev_MatchOrig_NoSave()
  {
    // check if get corectly filled segment wrapper
    RuleUtilTSI::TravelSegWrapper* tsw =
        RuleUtilTSI::processTravelSegsAddPrev(*_trx, _tsm1, true, false);
    CPPUNIT_ASSERT(!_tsm1.destSave());
    CPPUNIT_ASSERT(!tsw->destMatch());
    CPPUNIT_ASSERT(_tsm1.travelSeg() == tsw->travelSeg());
  }
  void testProcessTravelSegsAddPrev_NoMatchOrig_Save()
  {
    // check if get corectly filled segment wrapper
    RuleUtilTSI::TravelSegWrapper* tsw =
        RuleUtilTSI::processTravelSegsAddPrev(*_trx, _tsm1, false, true);
    CPPUNIT_ASSERT(!_tsm1.destSave());
    CPPUNIT_ASSERT(!tsw->destMatch());
    CPPUNIT_ASSERT(_tsm1.travelSeg() == tsw->travelSeg());
  }
  void testProcessTravelSegsAddPrev_NoMatchOrig_NoSave()
  {
    // check if get corectly filled segment wrapper
    RuleUtilTSI::TravelSegWrapper* tsw =
        RuleUtilTSI::processTravelSegsAddPrev(*_trx, _tsm1, false, false);
    CPPUNIT_ASSERT(!_tsm1.destSave());
    CPPUNIT_ASSERT(!tsw->destMatch());
    CPPUNIT_ASSERT(_tsm1.travelSeg() == tsw->travelSeg());
  }
  void testProcessTravelSegsAddCurr_MatchOrig_Save()
  {
    // check if get corectly filled segment wrapper
    RuleUtilTSI::TravelSegWrapper* tsw =
        RuleUtilTSI::processTravelSegsAddCurr(*_trx, _tsm1, true, true, true, true);
    CPPUNIT_ASSERT(_tsm1.origSave() && _tsm1.destSave());
    CPPUNIT_ASSERT(tsw->destMatch() && tsw->origMatch());
    CPPUNIT_ASSERT(_tsm1.travelSeg() == tsw->travelSeg());
  }
  void testProcessTravelSegsAddCurr_MatchOrig_NoSave()
  {
    // check if get corectly filled segment wrapper
    RuleUtilTSI::TravelSegWrapper* tsw =
        RuleUtilTSI::processTravelSegsAddCurr(*_trx, _tsm1, true, true, false, false);
    CPPUNIT_ASSERT(!_tsm1.origSave() && !_tsm1.destSave());
    CPPUNIT_ASSERT(!tsw->destMatch() && !tsw->origMatch());
    CPPUNIT_ASSERT(_tsm1.travelSeg() == tsw->travelSeg());
  }
  void testProcessTravelSegsAddCurr_NoMatchOrig_Save()
  {
    // check if get corectly filled segment wrapper
    RuleUtilTSI::TravelSegWrapper* tsw =
        RuleUtilTSI::processTravelSegsAddCurr(*_trx, _tsm1, false, false, true, true);
    CPPUNIT_ASSERT(!_tsm1.origSave() && !_tsm1.destSave());
    CPPUNIT_ASSERT(!tsw->destMatch() && !tsw->origMatch());
    CPPUNIT_ASSERT(_tsm1.travelSeg() == tsw->travelSeg());
  }
  void testProcessTravelSegsAddCurr_NoMatchOrig_NoSave()
  {
    // check if get corectly filled segment wrapper
    RuleUtilTSI::TravelSegWrapper* tsw =
        RuleUtilTSI::processTravelSegsAddCurr(*_trx, _tsm1, false, false, false, false);
    CPPUNIT_ASSERT(!_tsm1.origSave() && !_tsm1.destSave());
    CPPUNIT_ASSERT(!tsw->destMatch() && !tsw->origMatch());
    CPPUNIT_ASSERT(_tsm1.travelSeg() == tsw->travelSeg());
  }
  void testProcessTravelSegsAddNext_MatchOrig_Save()
  {
    // check if get corectly filled segment wrapper
    RuleUtilTSI::TravelSegWrapper* tsw =
        RuleUtilTSI::processTravelSegsAddNext(*_trx, _tsm1, true, true);
    CPPUNIT_ASSERT(_tsm1.origSave());
    CPPUNIT_ASSERT(tsw->origMatch());
    CPPUNIT_ASSERT(_tsm1.travelSeg() == tsw->travelSeg());
  }
  void testProcessTravelSegsAddNext_MatchOrig_NoSave()
  {
    // check if get corectly filled segment wrapper
    RuleUtilTSI::TravelSegWrapper* tsw =
        RuleUtilTSI::processTravelSegsAddNext(*_trx, _tsm1, true, false);
    CPPUNIT_ASSERT(!_tsm1.origSave());
    CPPUNIT_ASSERT(!tsw->origMatch());
    CPPUNIT_ASSERT(_tsm1.travelSeg() == tsw->travelSeg());
  }
  void testProcessTravelSegsAddNext_NoMatchOrig_Save()
  {
    // check if get corectly filled segment wrapper
    RuleUtilTSI::TravelSegWrapper* tsw =
        RuleUtilTSI::processTravelSegsAddNext(*_trx, _tsm1, false, true);
    CPPUNIT_ASSERT(!_tsm1.origSave());
    CPPUNIT_ASSERT(!tsw->origMatch());
    CPPUNIT_ASSERT(_tsm1.travelSeg() == tsw->travelSeg());
  }
  void testProcessTravelSegsAddNext_NoMatchOrig_NoSave()
  {
    // check if get corectly filled segment wrapper
    RuleUtilTSI::TravelSegWrapper* tsw =
        RuleUtilTSI::processTravelSegsAddNext(*_trx, _tsm1, false, false);
    CPPUNIT_ASSERT(!_tsm1.origSave());
    CPPUNIT_ASSERT(!tsw->origMatch());
    CPPUNIT_ASSERT(_tsm1.travelSeg() == tsw->travelSeg());
  }
  void testProcessTravelSegsForward_MatchAll_LoopCurr()
  { // all segments
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    testProcessTravelSegsForward(true, 3);
    assertSegmentLocs(0, "SFO", "SYD");
    assertSegmentLocs(1, "SYD", "DUB");
    assertSegmentLocs(2, "DUB", "LON");
  }
  void testProcessTravelSegsForward_MatchAll_LoopPrev()
  {
    // first segment don't have previous
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_PREVIOUS);
    testProcessTravelSegsForward(true, 2);
    assertSegmentLocs(0, "SFO", "SYD");
    assertSegmentLocs(1, "SYD", "DUB");
  }
  void testProcessTravelSegsForward_MatchAll_LoopNext()
  { // last segment don't have next
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_NEXT);
    testProcessTravelSegsForward(true, 2);
    assertSegmentLocs(0, "SYD", "DUB");
    assertSegmentLocs(1, "DUB", "LON");
  }
  void testProcessTravelSegsForward_MatchAll_LoopCurrPrev()
  {
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CUR_PREV);
    testProcessTravelSegsForward(true, 5);
    // no previous
    assertSegmentLocs(0, "SFO", "SYD"); // current
    assertSegmentLocs(1, "SFO", "SYD"); // prev
    assertSegmentLocs(2, "SYD", "DUB"); // current
    assertSegmentLocs(3, "SYD", "DUB"); // prev
    assertSegmentLocs(4, "DUB", "LON"); // current
  }
  void testProcessTravelSegsForward_MatchAll_LoopCurrNext()
  {
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    testProcessTravelSegsForward(true, 5);
    assertSegmentLocs(0, "SFO", "SYD"); // current
    assertSegmentLocs(1, "SYD", "DUB"); // next
    assertSegmentLocs(2, "SYD", "DUB"); // current
    assertSegmentLocs(3, "DUB", "LON"); // next
    assertSegmentLocs(4, "DUB", "LON"); // current
    // no next
  }
  void testProcessTravelSegsForward_MatchAll_FirstFail()
  {
    // fail match criteria on first segmnet, match on second
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    (_tsms.begin())->isInternational() = false;
    testProcessTravelSegsForward(true, 2);
    assertSegmentLocs(0, "SYD", "DUB");
    assertSegmentLocs(1, "DUB", "LON");
  }
  void testProcessTravelSegsForward_MatchAll_FareBreakOrig_ChkOrig()
  {
    // fare break origin on first segment
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    _checkFareBreaksOnly = true;
    (_tsms.begin())->fareBreakAtOrigin() = true;
    testProcessTravelSegsForward(true, 1);
    assertSegmentLocs(0, "SFO", "SYD");
  }
  void testProcessTravelSegsForward_MatchAll_FareBreakOrig_NoChkOrig()
  {
    // fare break origin on first segment
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    _checkFareBreaksOnly = true;
    _checkOrig = false;
    (_tsms.begin())->fareBreakAtOrigin() = true;
    testProcessTravelSegsForward(true, 0);
  }
  void testProcessTravelSegsForward_MatchAll_FareBreakDest_ChkDest()
  {
    // fare break destination on first segment
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    _checkFareBreaksOnly = true;
    (_tsms.begin())->fareBreakAtDestination() = true;
    testProcessTravelSegsForward(true, 1);
    assertSegmentLocs(0, "SFO", "SYD");
  }
  void testProcessTravelSegsForward_MatchAll_FareBreakDest_NoChkDest()
  {
    // fare break destination on first segment
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    _checkFareBreaksOnly = true;
    _checkDest = false;
    (_tsms.begin())->fareBreakAtDestination() = true;
    testProcessTravelSegsForward(true, 0);
  }
  void testProcessTravelSegsForward_MatchAll_LoopOffset()
  {
    // loop offset set, skip first element
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    _tsiInfo.loopOffset() = 1;
    testProcessTravelSegsForward(true, 2);
    assertSegmentLocs(0, "SYD", "DUB");
    assertSegmentLocs(1, "DUB", "LON");
  }
  void testProcessTravelSegsForward_MatchSecFirst_LoopCurr()
  { // all segments
    processTravelSegsData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    testProcessTravelSegsForward(true, 1);
    assertSegmentLocs(0, "SYD", "DUB");
  }
  void testProcessTravelSegsForward_MatchSecFirst_LoopPrev()
  {
    // match on second
    processTravelSegsData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_PREVIOUS);
    testProcessTravelSegsForward(true, 1);
    assertSegmentLocs(0, "SFO", "SYD");
  }
  void testProcessTravelSegsForward_MatchSecFirst_LoopNext()
  {
    // match on second
    processTravelSegsData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_NEXT);
    testProcessTravelSegsForward(true, 1);
    assertSegmentLocs(0, "DUB", "LON");
  }
  void testProcessTravelSegsForward_MatchSecFirst_LoopCurrPrev()
  {
    // match on second
    processTravelSegsData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_PREV);
    testProcessTravelSegsForward(true, 2);
    assertSegmentLocs(0, "SFO", "SYD");
    assertSegmentLocs(1, "SYD", "DUB");
  }
  void testProcessTravelSegsForward_MatchSecFirst_LoopCurrNext()
  {
    // match on second
    processTravelSegsData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    testProcessTravelSegsForward(true, 2);
    assertSegmentLocs(0, "SYD", "DUB"); // current
    assertSegmentLocs(1, "DUB", "LON"); // next
  }
  void testProcessTravelSegsForward_MatchSecFirst_FirstFail()
  {
    // fail match criteria on first segmnet, match on second
    processTravelSegsData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    (_tsms.begin())->isInternational() = false;
    testProcessTravelSegsForward(true, 1);
    assertSegmentLocs(0, "DUB", "LON");
  }
  void testProcessTravelSegsBackward_MatchAll_LoopCurr()
  {
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    testProcessTravelSegsBackward(true, 3);
    assertSegmentLocs(0, "SFO", "SYD");
    assertSegmentLocs(1, "SYD", "DUB");
    assertSegmentLocs(2, "DUB", "LON");
  }
  void testProcessTravelSegsBackward_MatchAll_LoopPrev()
  {
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_PREVIOUS);
    testProcessTravelSegsBackward(true, 2);
    assertSegmentLocs(0, "SFO", "SYD");
    assertSegmentLocs(1, "SYD", "DUB");
  }
  void testProcessTravelSegsBackward_MatchAll_LoopNext()
  {
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_NEXT);
    testProcessTravelSegsBackward(true, 2);
    assertSegmentLocs(0, "SYD", "DUB");
    assertSegmentLocs(1, "DUB", "LON");
  }
  void testProcessTravelSegsBackward_MatchAll_LoopCurrPrev()
  {
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CUR_PREV);
    testProcessTravelSegsBackward(true, 5);
    assertSegmentLocs(0, "SFO", "SYD"); // current
    // no previous
    assertSegmentLocs(1, "SFO", "SYD"); // current
    assertSegmentLocs(2, "SYD", "DUB"); // prev
    assertSegmentLocs(3, "SYD", "DUB"); // current
    assertSegmentLocs(4, "DUB", "LON"); // prev
  }
  void testProcessTravelSegsBackward_MatchAll_LoopCurrNext()
  {
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    testProcessTravelSegsBackward(true, 5);
    assertSegmentLocs(0, "SFO", "SYD"); // next
    assertSegmentLocs(1, "SYD", "DUB"); // current
    assertSegmentLocs(2, "SYD", "DUB"); // next
    assertSegmentLocs(3, "DUB", "LON"); // current
    // no next
    assertSegmentLocs(4, "DUB", "LON"); // current
  }
  void testProcessTravelSegsBackward_MatchAll_FirstFail()
  {
    // fail match criteria on first segmnet, TSI_MATCH_ALLon second
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    (_tsms.rbegin())->isInternational() = false;
    testProcessTravelSegsBackward(true, 2);
    assertSegmentLocs(0, "SFO", "SYD");
    assertSegmentLocs(1, "SYD", "DUB");
  }
  void testProcessTravelSegsBackward_MatchAll_FareBreakOrig_ChkOrig()
  {
    // fare break origin on first segment
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    _checkFareBreaksOnly = true;
    (_tsms.rbegin())->fareBreakAtOrigin() = true;
    testProcessTravelSegsBackward(true, 1);
    assertSegmentLocs(0, "DUB", "LON");
  }
  void testProcessTravelSegsBackward_MatchAll_FareBreakOrig_NoChkOrig()
  {
    // fare break origin on first segment
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    _checkFareBreaksOnly = true;
    _checkOrig = false;
    (_tsms.rbegin())->fareBreakAtOrigin() = true;
    testProcessTravelSegsBackward(true, 0);
  }
  void testProcessTravelSegsBackward_MatchAll_FareBreakDest_ChkDest()
  {
    // fare break destination on first segment
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    _checkFareBreaksOnly = true;
    (_tsms.rbegin())->fareBreakAtDestination() = true;
    testProcessTravelSegsBackward(true, 1);
    assertSegmentLocs(0, "DUB", "LON");
  }
  void testProcessTravelSegsBackward_MatchAll_FareBreakDest_NoChkDest()
  {
    // fare break destination on first segment
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    _checkFareBreaksOnly = true;
    _checkDest = false;
    (_tsms.rbegin())->fareBreakAtDestination() = true;
    testProcessTravelSegsBackward(true, 0);
  }
  void testProcessTravelSegsBackward_MatchAll_LoopOffset()
  {
    // loop offset set, skip first element
    processTravelSegsData(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    _tsiInfo.loopOffset() = 1;
    testProcessTravelSegsBackward(true, 2);
    assertSegmentLocs(0, "SYD", "DUB");
    assertSegmentLocs(1, "DUB", "LON");
  }
  void testProcessTravelSegsBackward_MatchSecFirst_LoopCurr()
  {
    processTravelSegsData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    testProcessTravelSegsBackward(true, 1);
    assertSegmentLocs(0, "SYD", "DUB");
  }
  void testProcessTravelSegsBackward_MatchSecFirst_LoopPrev()
  {
    processTravelSegsData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_PREVIOUS);
    testProcessTravelSegsBackward(true, 1);
    assertSegmentLocs(0, "SFO", "SYD");
  }
  void testProcessTravelSegsBackward_MatchSecFirst_LoopNext()
  {
    processTravelSegsData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_NEXT);
    testProcessTravelSegsBackward(true, 1);
    assertSegmentLocs(0, "DUB", "LON");
  }
  void testProcessTravelSegsBackward_MatchSecFirst_LoopCurrPrev()
  {
    processTravelSegsData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_PREV);
    testProcessTravelSegsBackward(true, 2);
    assertSegmentLocs(0, "SFO", "SYD");
    assertSegmentLocs(1, "SYD", "DUB");
  }
  void testProcessTravelSegsBackward_MatchSecFirst_LoopCurrNext()
  {
    processTravelSegsData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    testProcessTravelSegsBackward(true, 2);
    assertSegmentLocs(0, "SYD", "DUB");
    assertSegmentLocs(1, "DUB", "LON");
  }
  void testProcessTravelSegsBackward_MatchSecFirst_FirstFail()
  {
    // fail match criteria on first segmnet, TSI_MATCH_ALLon second
    processTravelSegsData(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    (_tsms.rbegin())->isInternational() = false;
    testProcessTravelSegsBackward(true, 1);
    assertSegmentLocs(0, "SFO", "SYD");
  }
  void testProcessTravelSegs_Fwrd_GeoOrig_TypeOrig()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(
        RuleConst::TSI_LOOP_FORWARD, RuleConst::TSI_APP_CHECK_ORIG, RuleConst::TSI_APP_CHECK_ORIG);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", true, false);
    assertSegmentMarkup(0, true, false);
  }
  void testProcessTravelSegs_Fwrd_GeoOrig_TypeDest()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(
        RuleConst::TSI_LOOP_FORWARD, RuleConst::TSI_APP_CHECK_ORIG, RuleConst::TSI_APP_CHECK_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", false, false);
    assertSegmentMarkup(0, false, false);
  }
  void testProcessTravelSegs_Fwrd_GeoOrig_TypeBoth()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(RuleConst::TSI_LOOP_FORWARD,
                          RuleConst::TSI_APP_CHECK_ORIG,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", true, false);
    assertSegmentMarkup(0, true, false);
  }
  void testProcessTravelSegs_Fwrd_GeoDest_TypeOrig()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(
        RuleConst::TSI_LOOP_FORWARD, RuleConst::TSI_APP_CHECK_DEST, RuleConst::TSI_APP_CHECK_ORIG);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", false, false);
    assertSegmentMarkup(0, false, false);
  }
  void testProcessTravelSegs_Fwrd_GeoDest_TypeDest()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(
        RuleConst::TSI_LOOP_FORWARD, RuleConst::TSI_APP_CHECK_DEST, RuleConst::TSI_APP_CHECK_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", false, true);
    assertSegmentMarkup(0, false, true);
  }
  void testProcessTravelSegs_Fwrd_GeoDest_TypeBoth()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(RuleConst::TSI_LOOP_FORWARD,
                          RuleConst::TSI_APP_CHECK_DEST,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", false, true);
    assertSegmentMarkup(0, false, true);
  }
  void testProcessTravelSegs_Fwrd_GeoBoth_TypeOrig()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(RuleConst::TSI_LOOP_FORWARD,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST,
                          RuleConst::TSI_APP_CHECK_ORIG);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", true, false);
    assertSegmentMarkup(0, true, false);
  }
  void testProcessTravelSegs_Fwrd_GeoBoth_TypeDest()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(RuleConst::TSI_LOOP_FORWARD,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST,
                          RuleConst::TSI_APP_CHECK_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", false, true);
    assertSegmentMarkup(0, false, true);
  }
  void testProcessTravelSegs_Fwrd_GeoBoth_TypeBoth()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(RuleConst::TSI_LOOP_FORWARD,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", true, true);
    assertSegmentMarkup(0, true, true);
  }
  void testProcessTravelSegs_FwrdBrk_GeoOrig_TypeOrig()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_FORWARD_BREAK_ARRIVAL,
                          RuleConst::TSI_APP_CHECK_ORIG,
                          RuleConst::TSI_APP_CHECK_ORIG);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", true, false);
    assertSegmentMarkup(0, true, false);
  }
  void testProcessTravelSegs_FwrdBrk_GeoOrig_TypeDest()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_FORWARD_BREAK_ARRIVAL,
                          RuleConst::TSI_APP_CHECK_ORIG,
                          RuleConst::TSI_APP_CHECK_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", false, false);
    assertSegmentMarkup(0, false, false);
  }
  void testProcessTravelSegs_FwrdBrk_GeoOrig_TypeBoth()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_FORWARD_BREAK_ARRIVAL,
                          RuleConst::TSI_APP_CHECK_ORIG,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", true, false);
    assertSegmentMarkup(0, true, false);
  }
  void testProcessTravelSegs_FwrdBrk_GeoDest_TypeOrig()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_FORWARD_BREAK_ARRIVAL,
                          RuleConst::TSI_APP_CHECK_DEST,
                          RuleConst::TSI_APP_CHECK_ORIG);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", false, false);
    assertSegmentMarkup(2, false, false);
  }
  void testProcessTravelSegs_FwrdBrk_GeoDest_TypeDest()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_FORWARD_BREAK_ARRIVAL,
                          RuleConst::TSI_APP_CHECK_DEST,
                          RuleConst::TSI_APP_CHECK_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", false, true);
    assertSegmentMarkup(2, false, true);
  }
  void testProcessTravelSegs_FwrdBrk_GeoDest_TypeBoth()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_FORWARD_BREAK_ARRIVAL,
                          RuleConst::TSI_APP_CHECK_DEST,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", false, true);
    assertSegmentMarkup(2, false, true);
  }
  void testProcessTravelSegs_FwrdBrk_GeoBoth_TypeOrig()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_FORWARD_BREAK_ARRIVAL,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST,
                          RuleConst::TSI_APP_CHECK_ORIG);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", true, false);
    assertSegmentMarkup(0, true, false);
  }
  void testProcessTravelSegs_FwrdBrk_GeoBoth_TypeDest()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_FORWARD_BREAK_ARRIVAL,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST,
                          RuleConst::TSI_APP_CHECK_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", false, true);
    assertSegmentMarkup(0, false, true);
  }
  void testProcessTravelSegs_FwrdBrk_GeoBoth_TypeBoth()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_FORWARD_BREAK_ARRIVAL,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", true, true);
    assertSegmentMarkup(0, true, true);
  }
  void testProcessTravelSegs_Back_GeoOrig_TypeOrig()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(
        RuleConst::TSI_LOOP_BACKWARD, RuleConst::TSI_APP_CHECK_ORIG, RuleConst::TSI_APP_CHECK_ORIG);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", true, false);
    assertSegmentMarkup(2, true, false);
  }
  void testProcessTravelSegs_Back_GeoOrig_TypeDest()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(
        RuleConst::TSI_LOOP_BACKWARD, RuleConst::TSI_APP_CHECK_ORIG, RuleConst::TSI_APP_CHECK_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", false, false);
    assertSegmentMarkup(2, false, false);
  }
  void testProcessTravelSegs_Back_GeoOrig_TypeBoth()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(RuleConst::TSI_LOOP_BACKWARD,
                          RuleConst::TSI_APP_CHECK_ORIG,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", true, false);
    assertSegmentMarkup(2, true, false);
  }
  void testProcessTravelSegs_Back_GeoDest_TypeOrig()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(
        RuleConst::TSI_LOOP_BACKWARD, RuleConst::TSI_APP_CHECK_DEST, RuleConst::TSI_APP_CHECK_ORIG);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", false, false);
    assertSegmentMarkup(2, false, false);
  }
  void testProcessTravelSegs_Back_GeoDest_TypeDest()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(
        RuleConst::TSI_LOOP_BACKWARD, RuleConst::TSI_APP_CHECK_DEST, RuleConst::TSI_APP_CHECK_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", false, true);
    assertSegmentMarkup(2, false, true);
  }
  void testProcessTravelSegs_Back_GeoDest_TypeBoth()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(RuleConst::TSI_LOOP_BACKWARD,
                          RuleConst::TSI_APP_CHECK_DEST,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", false, true);
    assertSegmentMarkup(2, false, true);
  }
  void testProcessTravelSegs_Back_GeoBoth_TypeOrig()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(RuleConst::TSI_LOOP_BACKWARD,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST,
                          RuleConst::TSI_APP_CHECK_ORIG);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", true, false);
    assertSegmentMarkup(2, true, false);
  }
  void testProcessTravelSegs_Back_GeoBoth_TypeDest()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(RuleConst::TSI_LOOP_BACKWARD,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST,
                          RuleConst::TSI_APP_CHECK_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", false, true);
    assertSegmentMarkup(2, false, true);
  }
  void testProcessTravelSegs_Back_GeoBoth_TypeBoth()
  {
    // should match type and geotype (geovalidation always pass)
    processTravelSegsData(RuleConst::TSI_LOOP_BACKWARD,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", true, true);
    assertSegmentMarkup(2, true, true);
  }
  void testProcessTravelSegs_BackBrk_GeoOrig_TypeOrig()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_BACKWARD_BREAK_DEPART,
                          RuleConst::TSI_APP_CHECK_ORIG,
                          RuleConst::TSI_APP_CHECK_ORIG);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", true, false);
    assertSegmentMarkup(0, true, false);
  }
  void testProcessTravelSegs_BackBrk_GeoOrig_TypeDest()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_BACKWARD_BREAK_DEPART,
                          RuleConst::TSI_APP_CHECK_ORIG,
                          RuleConst::TSI_APP_CHECK_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", false, false);
    assertSegmentMarkup(0, false, false);
  }
  void testProcessTravelSegs_BackBrk_GeoOrig_TypeBoth()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_BACKWARD_BREAK_DEPART,
                          RuleConst::TSI_APP_CHECK_ORIG,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "SFO", "SYD", true, false);
    assertSegmentMarkup(0, true, false);
  }
  void testProcessTravelSegs_BackBrk_GeoDest_TypeOrig()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_BACKWARD_BREAK_DEPART,
                          RuleConst::TSI_APP_CHECK_DEST,
                          RuleConst::TSI_APP_CHECK_ORIG);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", false, false);
    assertSegmentMarkup(2, false, false);
  }
  void testProcessTravelSegs_BackBrk_GeoDest_TypeDest()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_BACKWARD_BREAK_DEPART,
                          RuleConst::TSI_APP_CHECK_DEST,
                          RuleConst::TSI_APP_CHECK_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", false, true);
    assertSegmentMarkup(2, false, true);
  }
  void testProcessTravelSegs_BackBrk_GeoDest_TypeBoth()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_BACKWARD_BREAK_DEPART,
                          RuleConst::TSI_APP_CHECK_DEST,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", false, true);
    assertSegmentMarkup(2, false, true);
  }
  void testProcessTravelSegs_BackBrk_GeoBoth_TypeOrig()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_BACKWARD_BREAK_DEPART,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST,
                          RuleConst::TSI_APP_CHECK_ORIG);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", true, false);
    assertSegmentMarkup(2, true, false);
  }
  void testProcessTravelSegs_BackBrk_GeoBoth_TypeDest()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_BACKWARD_BREAK_DEPART,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST,
                          RuleConst::TSI_APP_CHECK_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", false, true);
    assertSegmentMarkup(2, false, true);
  }
  void testProcessTravelSegs_BackBrk_GeoBoth_TypeBoth()
  {
    // should match origin on SFOSYD and destination on LONDUB
    processTravelSegsData(RuleConst::TSI_LOOP_BACKWARD_BREAK_DEPART,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST,
                          RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertProcessTravelSegs(true, 1);
    assertSegmentLocs(0, "DUB", "LON", true, true);
    assertSegmentMarkup(2, true, true);
  }

  void setUp()
  {
    RuleUtilTSITest_Base::setUp();
    _vendor = "ATP";
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsiInfo.type() = RuleConst::TSI_APP_CHECK_ORIG_DEST;
    _tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG_DEST;
    _tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_INCLUDE;

    _tsiInfo.matchCriteria() += TSIInfo::INBOUND, TSIInfo::INTERNATIONAL, TSIInfo::FURTHEST;
    _tsi = _memHandle.insert(new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, 0, 0, 0));
    initTravelSegMarkup(_tsm1,
                        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml"),
                        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml"));
    initTravelSegMarkup(_tsm2,
                        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml"),
                        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDUB.xml"));
    initTravelSegMarkup(_tsm3,
                        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDUB.xml"),
                        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml"));

    _locSpecified = _checkFareBreaksOnly = false;
    _checkOrig = _checkDest = _origSave = _destSave = true;
    // processTravelSegsSetupTSIMatch should always pass with this data
    _currMatch = RuleConst::TSI_NOT_MATCH;
    _origMatch = false;
    _destMatch = false;
  }

protected:
  void initTravelSegMarkup(RuleUtilTSI::TSITravelSegMarkup& tsm, Loc* loc1, Loc* loc2)
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->origin() = loc1;
    seg->destination() = loc2;
    tsm.travelSeg() = seg;
    tsm.direction() = RuleConst::INBOUND;
    tsm.isFurthest() = true;
    tsm.isInternational() = true;
    _tsms.push_back(tsm);
  }
  void processTravelSegsData(RuleConst::TSILoopDirection ld,
                             RuleConst::TSIApplicationType geoType,
                             RuleConst::TSIApplicationType type)
  {
    // first match, return current
    _tsiInfo.loopDirection() = ld;
    _tsiInfo.geoCheck() = geoType;
    _tsiInfo.type() = type;
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_FIRST;
    _tsiInfo.loopToSet() = RuleConst::TSI_LOOP_SET_CURRENT;
    // no geo locations - GeoValidation match on origin and destioantion
    _tsi->locType1() = LOCTYPE_NONE;
    _tsi->locType2() = LOCTYPE_NONE;
    _tsi->locCode1() = "";
    _tsi->locCode2() = "";

    // set fare breakpoint origin SFOSYD, destination DUBLON
    (_tsms.begin() + 0)->fareBreakAtOrigin() = true;
    (_tsms.begin() + 2)->fareBreakAtDestination() = true;
  }
  void
  processTravelSegsData(RuleConst::TSILoopMatch loopMatch, RuleConst::TSILoopItemToSet loopToSet)
  {
    _tsiInfo.loopMatch() = loopMatch;
    _tsiInfo.loopToSet() = loopToSet;
  }
  void testProcessTravelSegsForward(bool result, size_t size)
  {
    CPPUNIT_ASSERT_EQUAL(result,
                         RuleUtilTSI::processTravelSegsForward(*_trx,
                                                               *_tsi,
                                                               _wraps,
                                                               _tsms,
                                                               _locSpecified,
                                                               _checkOrig,
                                                               _checkDest,
                                                               _origSave,
                                                               _destSave,
                                                               _checkFareBreaksOnly));
    CPPUNIT_ASSERT_EQUAL(size, _wraps.size());
  }
  void testProcessTravelSegsBackward(bool result, size_t size)
  {
    CPPUNIT_ASSERT_EQUAL(result,
                         RuleUtilTSI::processTravelSegsBackward(*_trx,
                                                                *_tsi,
                                                                _wraps,
                                                                _tsms,
                                                                _locSpecified,
                                                                _checkOrig,
                                                                _checkDest,
                                                                _origSave,
                                                                _destSave,
                                                                _checkFareBreaksOnly));
    CPPUNIT_ASSERT_EQUAL(size, _wraps.size());
  }
  void assertProcessTravelSegs(bool result, size_t size)
  {
    CPPUNIT_ASSERT_EQUAL(
        result, RuleUtilTSI::processTravelSegs(*_trx, *_tsi, _wraps, _tsms, _locSpecified));
    CPPUNIT_ASSERT_EQUAL(size, _wraps.size());
  }
  void assertSegmentLocs(size_t pos, LocCode loc1, LocCode loc2)
  {
    CPPUNIT_ASSERT(pos <= _wraps.size());
    CPPUNIT_ASSERT_EQUAL(loc1, _wraps[pos]->travelSeg()->origin()->loc());
    CPPUNIT_ASSERT_EQUAL(loc2, _wraps[pos]->travelSeg()->destination()->loc());
  }
  void assertSegmentLocs(size_t pos, LocCode loc1, LocCode loc2, bool orig, bool dest)
  {
    CPPUNIT_ASSERT(pos <= _wraps.size());
    CPPUNIT_ASSERT_EQUAL(loc1, _wraps[pos]->travelSeg()->origin()->loc());
    CPPUNIT_ASSERT_EQUAL(loc2, _wraps[pos]->travelSeg()->destination()->loc());
    CPPUNIT_ASSERT_EQUAL(orig, _wraps[pos]->origMatch());
    CPPUNIT_ASSERT_EQUAL(dest, _wraps[pos]->destMatch());
  }
  void assertSegmentMarkup(int pos, bool orig, bool dest)
  {
    CPPUNIT_ASSERT(pos >= 0 && pos < 3);
    RuleUtilTSI::TSITravelSegMarkupI fIter = _tsms.begin();
    for (int i = 0; i < pos; i++, fIter++)
      ;
    CPPUNIT_ASSERT_EQUAL(orig, fIter->origSave());
    CPPUNIT_ASSERT_EQUAL(dest, fIter->destSave());
  }
  TSIInfo* getPTSMatchCriteria(RuleConst::TSILoopMatch loopMatch, int len, ...)
  {
    va_list args;
    va_start(args, len);
    TSIInfo* tsi = _memHandle.create<TSIInfo>();
    tsi->loopMatch() = loopMatch;
    for (int i = 0; i < len; i++)
    {
      TSIInfo::TSIMatchCriteria mc = TSIInfo::TSIMatchCriteria(va_arg(args, int));
      tsi->matchCriteria().push_back(mc);
    }
    va_end(args);
    return tsi;
  }
  void assertPTSMatchCriteria(TSIInfo* tsiInfo, bool res, size_t v1 = 0, size_t v2 = 0)
  {
    std::vector<TSIInfo::TSIMatchCriteria> mc1;
    std::vector<TSIInfo::TSIMatchCriteria> mc2;
    CPPUNIT_ASSERT_EQUAL(
        res, RuleUtilTSI::processTravelSegsSetupMatchCriteria(*_trx, *tsiInfo, mc1, mc2));
    if (res)
    {
      CPPUNIT_ASSERT_EQUAL(v1, mc1.size());
      CPPUNIT_ASSERT_EQUAL(v2, mc2.size());
    }
  }
  void
  travelSegTSIMatchData(RuleConst::TSILoopMatch loopMatch,
                        RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH,
                        RuleConst::TSIApplicationType type = RuleConst::TSI_APP_CHECK_ORIG_DEST)
  {
    _tsiInfo.loopMatch() = loopMatch;
    _prevMatch = prevMatch;
    _tsiInfo.type() = type;
    _tsiInfo.geoCheck() = type;
    // pass on match criteria
    _mc1.push_back(TSIInfo::FURTHEST);
    _tsm1.isFurthest() = true;
    // match on origin
    _tsi->locType1() = 'N';
    _tsi->locType2() = ' ';
    _tsi->locCode1() = "US";
    _tsi->locCode2() = "";
  }
  void assertProcessTravSegTSIMatch(bool origMatch,
                                    bool destMatch,
                                    std::string reason,
                                    RuleConst::TSIMatch currMatch,
                                    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH)
  {
    RuleUtilTSI::processTravelSegsSetupTSIMatch(
        *_trx, *_tsi, _mc1, _mc2, _tsm1, _origMatch, _destMatch, _currMatch, _prevMatch);
    CPPUNIT_ASSERT_EQUAL(currMatch, _currMatch);
    CPPUNIT_ASSERT_EQUAL(prevMatch, _prevMatch);
    CPPUNIT_ASSERT_EQUAL(origMatch, _origMatch);
    CPPUNIT_ASSERT_EQUAL(destMatch, _destMatch);
    CPPUNIT_ASSERT_EQUAL(currMatch, _tsm1.match());
    CPPUNIT_ASSERT_EQUAL(origMatch, _tsm1.origMatch());
    CPPUNIT_ASSERT_EQUAL(destMatch, _tsm1.destMatch());
    CPPUNIT_ASSERT_EQUAL(reason, _tsm1.noMatchReason());
  }
  RuleUtilTSI::TSIData* _tsi;
  RuleUtilTSI::TSITravelSegMarkup _tsm1;
  RuleUtilTSI::TSITravelSegMarkup _tsm2;
  RuleUtilTSI::TSITravelSegMarkup _tsm3;
  RuleUtilTSI::TSITravelSegMarkupContainer _tsms;
  RuleUtilTSI::TravelSegWrapperVector _wraps;
  TSIInfo _tsiInfo;
  VendorCode _vendor;
  RuleConst::TSIScopeType _tsiScope;
  bool _locSpecified;
  bool _checkOrig;
  bool _checkDest;
  bool _origSave;
  bool _destSave;
  bool _checkFareBreaksOnly;

  std::vector<TSIInfo::TSIMatchCriteria> _mc1;
  std::vector<TSIInfo::TSIMatchCriteria> _mc2;
  bool _origMatch;
  bool _destMatch;
  RuleConst::TSIMatch _currMatch;
  RuleConst::TSIMatch _prevMatch;
};
class RuleUtilTSITest_ScopeTSIGeo : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_ScopeTSIGeo);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_Jour_Allow_Jour);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_Jour_Allow_SubJour);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_Jour_Allow_Fare);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_Jour_NotAllow_Jour);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_Jour_NotAllow_SubJour);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_Jour_NotAllow_Fare);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_SubJour_Allow_Jour);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_SubJour_Allow_SubJour);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_SubJour_Allow_Fare);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_SubJour_NotAllow_Jour);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_SubJour_NotAllow_SubJour);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_SubJour_NotAllow_Fare);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_Fare_Allow_Jour);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_Fare_Allow_SubJour);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_Fare_Allow_Fare);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_Fare_NotAllow_Jour);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_Fare_NotAllow_SubJour);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_Fare_NotAllow_Fare);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_SjFc_Jour);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_SjFc_SubJour);
  CPPUNIT_TEST(testScopeTSIGeoScopeSetup_SjFc_Fare);

  CPPUNIT_TEST(testScopeTSIGeoScopeValidate_Jour_Itin_FarePath_Itin);
  CPPUNIT_TEST(testScopeTSIGeoScopeValidate_Jour_Itin_FarePath_NoItin);
  CPPUNIT_TEST(testScopeTSIGeoScopeValidate_Jour_Itin_NoFarePath);
  CPPUNIT_TEST(testScopeTSIGeoScopeValidate_Jour_NoItin_FarePath_Itin);
  CPPUNIT_TEST(testScopeTSIGeoScopeValidate_Jour_NoItin_FarePath_NoItin);
  CPPUNIT_TEST(testScopeTSIGeoScopeValidate_Jour_NoItin_NoFarePath);
  CPPUNIT_TEST(testScopeTSIGeoScopeValidate_SubJour_PricingUnit);
  CPPUNIT_TEST(testScopeTSIGeoScopeValidate_SubJour_NoPricingUnit);
  CPPUNIT_TEST(testScopeTSIGeoScopeValidate_Fare_FareMarket);
  CPPUNIT_TEST(testScopeTSIGeoScopeValidate_Fare_NoFareMarket);

  CPPUNIT_TEST(testScopeTSIGeoCheckDMC_NoPU);
  CPPUNIT_TEST(testScopeTSIGeoCheckDMC_PU_NoRevDir);
  CPPUNIT_TEST(testScopeTSIGeoCheckDMC_PU_RevDir_Jour);
  CPPUNIT_TEST(testScopeTSIGeoCheckDMC_PU_RevDir_SubJour);
  CPPUNIT_TEST(testScopeTSIGeoCheckDMC_PU_RevDir_Fare_RevFM);
  CPPUNIT_TEST(testScopeTSIGeoCheckDMC_PU_RevDir_Fare_NoRevFM);

  CPPUNIT_TEST(testScopeTSIGeoCreateDiag_OK);
  CPPUNIT_TEST(testScopeTSIGeoCreateDiag_TrxDiagTypeNone);
  CPPUNIT_TEST(testScopeTSIGeoCreateDiag_TrxDiagType299);
  CPPUNIT_TEST(testScopeTSIGeoCreateDiag_TrxDiagType370);
  CPPUNIT_TEST(testScopeTSIGeoCreateDiag_TrxDiagParamMSGEO);
  CPPUNIT_TEST(testScopeTSIGeoCreateDiag_TrxDiagParamDDALL);
  CPPUNIT_TEST(testScopeTSIGeoCreateDiag_CallDiagTypeNone);
  CPPUNIT_TEST(testScopeTSIGeoCreateDiag_CallDiagType301);

  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcAll_Match);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcAll_NotMatch);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcFirst_Match);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcFirst_NotMatch);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcOnce_Match);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcOnce_NotMatch);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcLast_Match);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcLast_NotMatch);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcFirstAll_Match_ScopeJour);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcFirstAll_Match_ScopeSubJour);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcFirstAll_Match_ScopeFare);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcFirstAll_NotMatch_ScopeJour);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcFirstAll_NotMatch_ScopeSubJour);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcFirstAll_NotMatch_ScopeFare);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcSoft_Match);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcSoft_NotMatch);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcSecondFirst_Match);
  CPPUNIT_TEST(testScopeTSIGeoWriteMatch_MatcSecondFirst_NotMatch);

  CPPUNIT_TEST_SUITE_END();

public:
  void testScopeTSIGeoScopeSetup_Jour_Allow_Jour()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_JOURNEY;
    assertScopeTSIGeoScopeSetup(
        RuleConst::TSI_SCOPE_JOURNEY, RuleConst::TSI_SCOPE_PARAM_JOURNEY, true, false, false);
  }
  void testScopeTSIGeoScopeSetup_Jour_Allow_SubJour()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_JOURNEY;
    assertScopeTSIGeoScopeSetup(
        RuleConst::TSI_SCOPE_JOURNEY, RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY, true, false, false);
  }
  void testScopeTSIGeoScopeSetup_Jour_Allow_Fare()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_JOURNEY;
    assertScopeTSIGeoScopeSetup(RuleConst::TSI_SCOPE_JOURNEY,
                                RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                true,
                                false,
                                false);
  }
  void testScopeTSIGeoScopeSetup_Jour_NotAllow_Jour()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_JOURNEY;
    assertScopeTSIGeoScopeSetup(
        RuleConst::TSI_SCOPE_JOURNEY, RuleConst::TSI_SCOPE_PARAM_JOURNEY, false, false, false);
  }
  void testScopeTSIGeoScopeSetup_Jour_NotAllow_SubJour()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_JOURNEY;
    assertScopeTSIGeoScopeSetup(
        RuleConst::TSI_SCOPE_JOURNEY, RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY, false, false, false);
  }
  void testScopeTSIGeoScopeSetup_Jour_NotAllow_Fare()
  {
    // change the scope to fare component
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_JOURNEY;
    assertScopeTSIGeoScopeSetup(RuleConst::TSI_SCOPE_FARE_COMPONENT,
                                RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                false,
                                false,
                                false);
  }
  void testScopeTSIGeoScopeSetup_SubJour_Allow_Jour()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertScopeTSIGeoScopeSetup(
        RuleConst::TSI_SCOPE_SUB_JOURNEY, RuleConst::TSI_SCOPE_PARAM_JOURNEY, false, true, false);
  }
  void testScopeTSIGeoScopeSetup_SubJour_Allow_SubJour()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertScopeTSIGeoScopeSetup(RuleConst::TSI_SCOPE_SUB_JOURNEY,
                                RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                false,
                                true,
                                false);
  }
  void testScopeTSIGeoScopeSetup_SubJour_Allow_Fare()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertScopeTSIGeoScopeSetup(RuleConst::TSI_SCOPE_SUB_JOURNEY,
                                RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                false,
                                true,
                                false);
  }
  void testScopeTSIGeoScopeSetup_SubJour_NotAllow_Jour()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertScopeTSIGeoScopeSetup(
        RuleConst::TSI_SCOPE_SUB_JOURNEY, RuleConst::TSI_SCOPE_PARAM_JOURNEY, false, false, false);
  }
  void testScopeTSIGeoScopeSetup_SubJour_NotAllow_SubJour()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertScopeTSIGeoScopeSetup(RuleConst::TSI_SCOPE_SUB_JOURNEY,
                                RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                false,
                                false,
                                false);
  }
  void testScopeTSIGeoScopeSetup_SubJour_NotAllow_Fare()
  {
    // change the scope to fare component
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertScopeTSIGeoScopeSetup(RuleConst::TSI_SCOPE_FARE_COMPONENT,
                                RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                false,
                                false,
                                false);
  }
  void testScopeTSIGeoScopeSetup_Fare_Allow_Jour()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertScopeTSIGeoScopeSetup(RuleConst::TSI_SCOPE_FARE_COMPONENT,
                                RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                false,
                                false,
                                true);
  }
  void testScopeTSIGeoScopeSetup_Fare_Allow_SubJour()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertScopeTSIGeoScopeSetup(RuleConst::TSI_SCOPE_FARE_COMPONENT,
                                RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                false,
                                false,
                                true);
  }
  void testScopeTSIGeoScopeSetup_Fare_Allow_Fare()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertScopeTSIGeoScopeSetup(RuleConst::TSI_SCOPE_FARE_COMPONENT,
                                RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                false,
                                false,
                                true);
  }
  void testScopeTSIGeoScopeSetup_Fare_NotAllow_Jour()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertScopeTSIGeoScopeSetup(RuleConst::TSI_SCOPE_FARE_COMPONENT,
                                RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                false,
                                false,
                                false);
  }
  void testScopeTSIGeoScopeSetup_Fare_NotAllow_SubJour()
  {
    // change the scope to sub journey
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertScopeTSIGeoScopeSetup(RuleConst::TSI_SCOPE_SUB_JOURNEY,
                                RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                false,
                                false,
                                false);
  }
  void testScopeTSIGeoScopeSetup_Fare_NotAllow_Fare()
  {
    // no change in the scope
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertScopeTSIGeoScopeSetup(RuleConst::TSI_SCOPE_FARE_COMPONENT,
                                RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                false,
                                false,
                                false);
  }
  void testScopeTSIGeoScopeSetup_SjFc_Jour()
  {
    // change the scope to journey
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_SJ_AND_FC;
    assertScopeTSIGeoScopeSetup(
        RuleConst::TSI_SCOPE_JOURNEY, RuleConst::TSI_SCOPE_PARAM_JOURNEY, false, false, false);
  }
  void testScopeTSIGeoScopeSetup_SjFc_SubJour()
  {
    // change the scope to sub journey
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_SJ_AND_FC;
    assertScopeTSIGeoScopeSetup(RuleConst::TSI_SCOPE_SUB_JOURNEY,
                                RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                false,
                                false,
                                false);
  }
  void testScopeTSIGeoScopeSetup_SjFc_Fare()
  {
    // change the scope to fare component
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_SJ_AND_FC;
    assertScopeTSIGeoScopeSetup(RuleConst::TSI_SCOPE_FARE_COMPONENT,
                                RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                false,
                                false,
                                false);
  }
  void testScopeTSIGeoScopeValidate_Jour_Itin_FarePath_Itin()
  {
    // all filed - pass
    _farePath.itin() = &_itin;
    CPPUNIT_ASSERT_EQUAL(true,
                         RuleUtilTSI::scopeTSIGeoScopeValidate(
                             0, RuleConst::TSI_SCOPE_JOURNEY, *_trx, &_farePath, &_itin, 0, 0));
  }
  void testScopeTSIGeoScopeValidate_Jour_Itin_FarePath_NoItin()
  {
    // fare path withou itin - fail
    CPPUNIT_ASSERT_EQUAL(false,
                         RuleUtilTSI::scopeTSIGeoScopeValidate(
                             0, RuleConst::TSI_SCOPE_JOURNEY, *_trx, &_farePath, &_itin, 0, 0));
  }
  void testScopeTSIGeoScopeValidate_Jour_Itin_NoFarePath()
  {
    // itin pased - pass
    _farePath.itin() = &_itin;
    CPPUNIT_ASSERT_EQUAL(true,
                         RuleUtilTSI::scopeTSIGeoScopeValidate(
                             0, RuleConst::TSI_SCOPE_JOURNEY, *_trx, 0, &_itin, 0, 0));
  }
  void testScopeTSIGeoScopeValidate_Jour_NoItin_FarePath_Itin()
  {
    // fare path with itin - pass
    _farePath.itin() = &_itin;
    CPPUNIT_ASSERT_EQUAL(true,
                         RuleUtilTSI::scopeTSIGeoScopeValidate(
                             0, RuleConst::TSI_SCOPE_JOURNEY, *_trx, &_farePath, 0, 0, 0));
  }
  void testScopeTSIGeoScopeValidate_Jour_NoItin_FarePath_NoItin()
  {
    // fare path without itin - fail
    CPPUNIT_ASSERT_EQUAL(false,
                         RuleUtilTSI::scopeTSIGeoScopeValidate(
                             0, RuleConst::TSI_SCOPE_JOURNEY, *_trx, &_farePath, 0, 0, 0));
  }
  void testScopeTSIGeoScopeValidate_Jour_NoItin_NoFarePath()
  {
    // no fare path or itin - fail
    _farePath.itin() = &_itin;
    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtilTSI::scopeTSIGeoScopeValidate(0, RuleConst::TSI_SCOPE_JOURNEY, *_trx, 0, 0, 0, 0));
  }
  void testScopeTSIGeoScopeValidate_SubJour_PricingUnit()
  {
    // pricing unit passed - pass
    CPPUNIT_ASSERT_EQUAL(true,
                         RuleUtilTSI::scopeTSIGeoScopeValidate(
                             0, RuleConst::TSI_SCOPE_SUB_JOURNEY, *_trx, 0, 0, &_pu, 0));
  }
  void testScopeTSIGeoScopeValidate_SubJour_NoPricingUnit()
  {
    // no pricing unit - fail
    CPPUNIT_ASSERT_EQUAL(false,
                         RuleUtilTSI::scopeTSIGeoScopeValidate(
                             0, RuleConst::TSI_SCOPE_SUB_JOURNEY, *_trx, 0, 0, 0, 0));
  }
  void testScopeTSIGeoScopeValidate_Fare_FareMarket()
  {
    // fare market passed - pass
    CPPUNIT_ASSERT_EQUAL(true,
                         RuleUtilTSI::scopeTSIGeoScopeValidate(
                             0, RuleConst::TSI_SCOPE_FARE_COMPONENT, *_trx, 0, 0, 0, &_fm));
  }
  void testScopeTSIGeoScopeValidate_Fare_NoFareMarket()
  {
    // no fare market - fail
    CPPUNIT_ASSERT_EQUAL(false,
                         RuleUtilTSI::scopeTSIGeoScopeValidate(
                             0, RuleConst::TSI_SCOPE_FARE_COMPONENT, *_trx, 0, 0, 0, 0));
  }
  void testScopeTSIGeoCheckDMC_NoPU()
  {
    // no pricing unit - no reversing
    _pPU = 0;
    assertScopeTSIGeoCheckDMC(false);
  }
  void testScopeTSIGeoCheckDMC_PU_NoRevDir()
  {
    // no reverse direction on ffarePath - no reversing
    _pu.fareDirectionReversed() = false;
    assertScopeTSIGeoCheckDMC(false);
  }
  void testScopeTSIGeoCheckDMC_PU_RevDir_Jour()
  {
    // reversing only for FareComponent
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    assertScopeTSIGeoCheckDMC(false);
  }
  void testScopeTSIGeoCheckDMC_PU_RevDir_SubJour()
  {
    // reversing only for FareComponent
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertScopeTSIGeoCheckDMC(false);
  }
  void testScopeTSIGeoCheckDMC_PU_RevDir_Fare_RevFM()
  {
    // reversed amrket already exists
    _fm.reversedFareMarket() = (FareMarket*)123456;
    assertScopeTSIGeoCheckDMC(false);
  }
  void testScopeTSIGeoCheckDMC_PU_RevDir_Fare_NoRevFM() { assertScopeTSIGeoCheckDMC(true); }
  void testScopeTSIGeoCreateDiag_OK() { assertScopeTSIGeoCheckDMC(true); }
  void testScopeTSIGeoCreateDiag_TrxDiagTypeNone()
  {
    // not filled - don't create
    _trx->diagnostic().diagnosticType() = DiagnosticNone;
    assertScopeTSIGeoCreateDiag(false);
  }
  void testScopeTSIGeoCreateDiag_TrxDiagType299()
  {
    // less then 300
    _trx->diagnostic().diagnosticType() = Diagnostic299;
    assertScopeTSIGeoCreateDiag(false);
  }
  void testScopeTSIGeoCreateDiag_TrxDiagType370()
  {
    // greater then 350
    _trx->diagnostic().diagnosticType() = Diagnostic370;
    assertScopeTSIGeoCreateDiag(false);
  }
  void testScopeTSIGeoCreateDiag_TrxDiagParamMSGEO()
  {
    // no DD diag param - fail
    _trx->diagnostic().diagParamMap().clear();
    _trx->diagnostic().diagParamMap().insert(
        std::pair<std::string, std::string>("MS", RuleConst::DIAGNOSTIC_INCLUDE_GEO));
    assertScopeTSIGeoCreateDiag(false);
  }
  void testScopeTSIGeoCreateDiag_TrxDiagParamDDALL()
  {
    // DD param with ALL not GEO - fail
    _trx->diagnostic().diagParamMap()["DD"] = "ALL";
    assertScopeTSIGeoCreateDiag(false);
  }
  void testScopeTSIGeoCreateDiag_CallDiagTypeNone()
  {
    // caller diag none - fail
    _callerDiag = DiagnosticNone;
    assertScopeTSIGeoCreateDiag(false);
  }
  void testScopeTSIGeoCreateDiag_CallDiagType301()
  {
    // caller diag diffrent the trx diag - fail
    _callerDiag = Diagnostic301;
    assertScopeTSIGeoCreateDiag(false);
  }
  void testScopeTSIGeoWriteMatch_MatcAll_Match()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_ALL;
    assertScopeTSIGeoWriteMatch(true, "ALL ITEMS CHECKED\n");
  }
  void testScopeTSIGeoWriteMatch_MatcAll_NotMatch()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_ALL;
    assertScopeTSIGeoWriteMatch(false, "ALL ITEMS CHECKED\n");
  }
  void testScopeTSIGeoWriteMatch_MatcFirst_Match()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_FIRST;
    assertScopeTSIGeoWriteMatch(true, "FIRST MATCH FOUND\n");
  }
  void testScopeTSIGeoWriteMatch_MatcFirst_NotMatch()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_FIRST;
    assertScopeTSIGeoWriteMatch(false, "FIRST MATCH NOT FOUND\n");
  }
  void testScopeTSIGeoWriteMatch_MatcOnce_Match()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_ONCE;
    assertScopeTSIGeoWriteMatch(true, "ONLY 1 ITEM CHECKED. MATCH FOUND\n");
  }
  void testScopeTSIGeoWriteMatch_MatcOnce_NotMatch()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_ONCE;
    assertScopeTSIGeoWriteMatch(false, "ONLY 1 ITEM CHECKED. MATCH NOT FOUND\n");
  }
  void testScopeTSIGeoWriteMatch_MatcLast_Match()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_LAST;
    assertScopeTSIGeoWriteMatch(true, "LAST MATCH FOUND\n");
  }
  void testScopeTSIGeoWriteMatch_MatcLast_NotMatch()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_LAST;
    assertScopeTSIGeoWriteMatch(false, "LAST MATCH NOT FOUND\n");
  }
  void testScopeTSIGeoWriteMatch_MatcFirstAll_Match_ScopeJour()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_FIRST_ALL;
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    assertScopeTSIGeoWriteMatch(true, "ERROR!! MATCH-FIRST-ALL NOT ALLOWED WITH JOURNEY SCOPE\n");
  }
  void testScopeTSIGeoWriteMatch_MatcFirstAll_Match_ScopeSubJour()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_FIRST_ALL;
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertScopeTSIGeoWriteMatch(true, "ALL ITEMS CHECKED\n");
  }
  void testScopeTSIGeoWriteMatch_MatcFirstAll_Match_ScopeFare()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_FIRST_ALL;
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertScopeTSIGeoWriteMatch(true, "FIRST MATCH FOUND\n");
  }
  void testScopeTSIGeoWriteMatch_MatcFirstAll_NotMatch_ScopeJour()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_FIRST_ALL;
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    assertScopeTSIGeoWriteMatch(false, "ERROR!! MATCH-FIRST-ALL NOT ALLOWED WITH JOURNEY SCOPE\n");
  }
  void testScopeTSIGeoWriteMatch_MatcFirstAll_NotMatch_ScopeSubJour()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_FIRST_ALL;
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertScopeTSIGeoWriteMatch(false, "ALL ITEMS CHECKED\n");
  }
  void testScopeTSIGeoWriteMatch_MatcFirstAll_NotMatch_ScopeFare()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_FIRST_ALL;
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertScopeTSIGeoWriteMatch(false, "FIRST MATCH NOT FOUND\n");
  }
  void testScopeTSIGeoWriteMatch_MatcSoft_Match()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_SOFT_MATCH;
    assertScopeTSIGeoWriteMatch(true, "SOFT MATCH REQUESTED\n");
  }
  void testScopeTSIGeoWriteMatch_MatcSoft_NotMatch()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_SOFT_MATCH;
    assertScopeTSIGeoWriteMatch(false, "SOFT MATCH REQUESTED\n");
  }
  void testScopeTSIGeoWriteMatch_MatcSecondFirst_Match()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_SECOND_FIRST;
    assertScopeTSIGeoWriteMatch(true, "MATCH SECOND FIRST REQUESTED\n");
  }
  void testScopeTSIGeoWriteMatch_MatcSecondFirst_NotMatch()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_SECOND_FIRST;
    assertScopeTSIGeoWriteMatch(false, "MATCH SECOND FIRST REQUESTED\n");
  }

  void setUp()
  {
    RuleUtilTSITest_Base::setUp();
    // conditions match for reversed directions
    _pu.fareDirectionReversed() = true;
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _pPU = &_pu;
    _fm.travelSeg().push_back(_segSFOSYD);
    _tsi = _memHandle.insert(new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, 0, 0, 0));

    // diagnostic stuff
    _callerDiag = Diagnostic315;
    _trx->diagnostic().diagnosticType() = Diagnostic315; // between 300 and 350
    _trx->diagnostic().diagParamMap().insert(std::pair<std::string, std::string>(
        Diagnostic::DISPLAY_DETAIL, RuleConst::DIAGNOSTIC_INCLUDE_GEO));

    _diag = _memHandle.insert(new DiagCollector(_diagRoot));
    _diagRoot.activate();
    _diag->enable(DiagnosticNone);
  }

private:
  RuleConst::TSIScopeType _tsiScope;
  TSIInfo _tsiInfo;
  RuleUtilTSI::TSIData* _tsi;
  VendorCode _vendor;
  Itin _itin;
  FarePath _farePath;
  PricingUnit _pu;
  PricingUnit* _pPU;
  FareMarket _fm;
  AirSeg _a1;
  DiagnosticTypes _callerDiag;
  DiagCollector* _diag;
  Diagnostic _diagRoot;

  void assertScopeTSIGeoWriteMatch(bool match, std::string ret)
  {
    RuleUtilTSI::scopeTSIGeoWriteMatch(_diag, &_tsiInfo, _tsiScope, match);
    CPPUNIT_ASSERT_EQUAL(ret, _diag->str());
  }
  void assertScopeTSIGeoCreateDiag(bool ret)
  {
    DCFactory* factory = 0;
    DiagCollector* diag = 0;
    LocKey loc1, loc2;
    CPPUNIT_ASSERT_EQUAL(ret,
                         RuleUtilTSI::scopeTSIGeoCreateDiag(*_trx,
                                                            factory,
                                                            diag,
                                                            _callerDiag,
                                                            *_tsi,
                                                            RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                                            _tsiScope,
                                                            false,
                                                            false,
                                                            false,
                                                            loc1,
                                                            loc2));
    if (ret)
    {
      // is created diag && factory
      CPPUNIT_ASSERT(factory);
      CPPUNIT_ASSERT(diag);
    }
    else
    {
      // not created diag && factory
      CPPUNIT_ASSERT(factory == 0);
      CPPUNIT_ASSERT(diag == 0);
    }
  }
  void assertScopeTSIGeoCheckDMC(bool reversedFM)
  {
    const FareMarket* fm = &_fm;
    const FareMarket* retFM;
    // false only if mem alloc fail
    CPPUNIT_ASSERT(RuleUtilTSI::scopeTSIGeoCheckDMC(_tsiScope, *_trx, _pPU, fm, retFM));
    if (reversedFM)
    {
      CPPUNIT_ASSERT(retFM != fm);
      CPPUNIT_ASSERT(retFM->travelSeg().size() == 1);
      AirSeg* seg = dynamic_cast<AirSeg*>(retFM->travelSeg().front());
      CPPUNIT_ASSERT(seg);
      // reversed directions
      CPPUNIT_ASSERT_EQUAL(_segSFOSYD->origin()->loc(), seg->destination()->loc());
      CPPUNIT_ASSERT_EQUAL(_segSFOSYD->destination()->loc(), seg->origin()->loc());
    }
    else
    {
      // the same pooiner
      //
      if (fm->reversedFareMarket())
      {
        CPPUNIT_ASSERT_EQUAL(retFM, (const FareMarket*)(fm->reversedFareMarket()));
      }
      else
      {
        CPPUNIT_ASSERT_EQUAL(retFM, fm);
      }
    }
  }

  void assertScopeTSIGeoScopeSetup(RuleConst::TSIScopeType ret,
                                   RuleConst::TSIScopeParamType defaultScope,
                                   bool allowJourneyScopeOverride,
                                   bool allowPUScopeOverride,
                                   bool allowFCScopeOverride)
  {
    CPPUNIT_ASSERT_EQUAL(ret,
                         RuleUtilTSI::scopeTSIGeoScopeSetup(&_tsiInfo,
                                                            defaultScope,
                                                            allowJourneyScopeOverride,
                                                            allowPUScopeOverride,
                                                            allowFCScopeOverride));
  }
};

class RuleUtilTSITest_WriteTSIDiag : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_WriteTSIDiag);
  CPPUNIT_TEST(testWriteTSIDiagHeader);
  CPPUNIT_TEST(testWriteTSIDiagHeader_FirstLine);
  CPPUNIT_TEST(testWriteTSIDiagHeader_Tsi);
  CPPUNIT_TEST(testWriteTSIDiagHeader_Description);
  CPPUNIT_TEST(testWriteTSIDiagHeader_DefScopeJour);
  CPPUNIT_TEST(testWriteTSIDiagHeader_DefScopeSubJour);
  CPPUNIT_TEST(testWriteTSIDiagHeader_DefScopeFare);
  CPPUNIT_TEST(testWriteTSIDiagHeader_OverrJour);
  CPPUNIT_TEST(testWriteTSIDiagHeader_OverrSubJour);
  CPPUNIT_TEST(testWriteTSIDiagHeader_OverrFare);
  CPPUNIT_TEST(testWriteTSIDiagHeader_ProcScopeJour);
  CPPUNIT_TEST(testWriteTSIDiagHeader_ProcScopeSubJour);
  CPPUNIT_TEST(testWriteTSIDiagHeader_ProcScopeFare);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoType1);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoLoc1);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoType2);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoLoc1);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoReqBothReq);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoReqGeotypeReq);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoReqFromItin);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoReqBlank);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoNotCity);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoNotBoth);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoNotThree);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoNotBlank);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoOutInclude);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoOutExclude);
  CPPUNIT_TEST(testWriteTSIDiagHeader_ItinPartOrig);
  CPPUNIT_TEST(testWriteTSIDiagHeader_ItinPartDest);
  CPPUNIT_TEST(testWriteTSIDiagHeader_ItinPartTurn);
  CPPUNIT_TEST(testWriteTSIDiagHeader_ItinPartBlank);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoCheckOrig);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoCheckDest);
  CPPUNIT_TEST(testWriteTSIDiagHeader_GeoCheckOrigDest);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopFrwd);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopBack);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopFrwdBrk);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopBackBrk);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopOff2);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopSetPrev);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopSetCurr);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopSetNext);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopSetCurNext);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopSetCurPrev);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopMatchAll);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopMatchFirst);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopMatchOnce);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopMatchLast);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopMatchFirstAll);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopMatchSoft);
  CPPUNIT_TEST(testWriteTSIDiagHeader_LoopMatchSecFirst);
  CPPUNIT_TEST(testWriteTSIDiagHeader_ScopeJour);
  CPPUNIT_TEST(testWriteTSIDiagHeader_ScopeSubJour);
  CPPUNIT_TEST(testWriteTSIDiagHeader_ScopeFare);
  CPPUNIT_TEST(testWriteTSIDiagHeader_ScopeSjFc);
  CPPUNIT_TEST(testWriteTSIDiagHeader_TypeOrig);
  CPPUNIT_TEST(testWriteTSIDiagHeader_TypeDest);
  CPPUNIT_TEST(testWriteTSIDiagHeader_TypeOrigDest);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchStopover);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchInbound);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchOutbound);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchDomestic);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchOneCountry);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchInternational);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchGateway);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchOrigGateway);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchDestGateway);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchTransTalantic);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchTransPacific);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchTransOceanic);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchIntercontinental);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchOverWater);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchIntlDomTransfer);
  CPPUNIT_TEST(testWriteTSIDiagHeader_MatchFurthest);

  CPPUNIT_TEST(testWriteTSIDiagDetails_Line);
  CPPUNIT_TEST(testWriteTSIDiagDetails_NoPuFc);
  CPPUNIT_TEST(testWriteTSIDiagDetails_PuNmbr);
  CPPUNIT_TEST(testWriteTSIDiagDetails_FcNmbr);
  CPPUNIT_TEST(testWriteTSIDiagDetails_Stop);
  CPPUNIT_TEST(testWriteTSIDiagDetails_Orig);
  CPPUNIT_TEST(testWriteTSIDiagDetails_Dest);
  CPPUNIT_TEST(testWriteTSIDiagDetails_NoMatch);
  CPPUNIT_TEST(testWriteTSIDiagDetails_MatchOrig);
  CPPUNIT_TEST(testWriteTSIDiagDetails_MatchDest);
  CPPUNIT_TEST(testWriteTSIDiagDetails_OrigSave);
  CPPUNIT_TEST(testWriteTSIDiagDetails_DestSave);
  CPPUNIT_TEST(testWriteTSIDiagDetails_NoMatchReason);

  CPPUNIT_TEST_SUITE_END();

public:
  void testWriteTSIDiagHeader()
  {
    assertWriteTSIDiagHeader(0, 0, 0);
    std::string response =
        " \n"
        "TSI DATA PASSED TO PROCESS\n"
        "TSI:  1 - DESCRIPTION\n"
        " PASSED SCOPE: JOURNEY\n"
        " PERMITTED SCOPE OVERRIDES:  JRNY-FC PU-FC FC-PU\n"
        " PROCESSING TSI USING SCOPE: JOURNEY\n"
        " GEO TYPE-1: C LOCALE-1: SFO GEO TYPE-2: C LOCALE-2: SYD\n"
        " \n"
        "TSI DATA FROM TABLE\n"
        "--------GEO--------  -------LOOP-------  ---SAVE---\n"
        "REQ NOT OUT ITIN CK  DIR  OFF SET MATCH  SCOPE TYPE\n"
        "REQ  C      ORIG O   FRWD   1  -1 ALL    JURNY  O \n"
        "\n"
        "MATCH CRITERIA\n"
        "STOPOVER INBOUND  OUTBOUND DOMESTIC ONECNTRY INTL     GATEWAY  O-GATEWY D-GATEWY TRANSATL "
        "TRANSPAC TRANSOCN INTERCON OVERWATR INDOMTFR FURTHEST \n";
    CPPUNIT_ASSERT_EQUAL(response, _diag->str());
  }
  //
  // for each parameter check if change is reflected in diagnostic output
  //
  void testWriteTSIDiagHeader_FirstLine()
  {
    assertWriteTSIDiagHeader(1, 0, "TSI DATA PASSED TO PROCESS");
  }
  void testWriteTSIDiagHeader_Tsi()
  {
    _tsiInfo.tsi() = 51;
    assertWriteTSIDiagHeader(2, 4, " 51");
  }
  void testWriteTSIDiagHeader_Description()
  {
    _tsiInfo.description() = "KUPA";
    assertWriteTSIDiagHeader(2, 10, "KUPA");
  }
  void testWriteTSIDiagHeader_DefScopeJour()
  {
    _defaultScope = RuleConst::TSI_SCOPE_PARAM_JOURNEY;
    assertWriteTSIDiagHeader(3, 15, "JOURNEY");
  }
  void testWriteTSIDiagHeader_DefScopeSubJour()
  {
    _defaultScope = RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY;
    assertWriteTSIDiagHeader(3, 15, "SUB JOURNEY");
  }
  void testWriteTSIDiagHeader_DefScopeFare()
  {
    _defaultScope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
    assertWriteTSIDiagHeader(3, 15, "FARE");
  }
  void testWriteTSIDiagHeader_OverrJour()
  {
    _allowPUScopeOverride = false;
    _allowFCScopeOverride = false;
    assertWriteTSIDiagHeader(4, 29, "JRNY-FC");
  }
  void testWriteTSIDiagHeader_OverrSubJour()
  {
    _allowJourneyScopeOverride = false;
    _allowFCScopeOverride = false;
    assertWriteTSIDiagHeader(4, 29, "PU-FC");
  }
  void testWriteTSIDiagHeader_OverrFare()
  {
    _allowJourneyScopeOverride = false;
    _allowPUScopeOverride = false;
    assertWriteTSIDiagHeader(4, 29, "FC-PU");
  }
  void testWriteTSIDiagHeader_ProcScopeJour()
  {
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    assertWriteTSIDiagHeader(5, 29, "JOURNEY");
  }
  void testWriteTSIDiagHeader_ProcScopeSubJour()
  {
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertWriteTSIDiagHeader(5, 29, "SUB JOURNEY");
  }
  void testWriteTSIDiagHeader_ProcScopeFare()
  {
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertWriteTSIDiagHeader(5, 29, "FARE");
  }
  void testWriteTSIDiagHeader_GeoType1()
  {
    _locSFO.locType() = 'N';
    assertWriteTSIDiagHeader(6, 13, "N");
  }
  void testWriteTSIDiagHeader_GeoLoc1()
  {
    _locSFO.loc() = "US";
    assertWriteTSIDiagHeader(6, 25, "US");
  }
  void testWriteTSIDiagHeader_GeoType2()
  {
    _locSYD.locType() = 'N';
    assertWriteTSIDiagHeader(6, 41, "N");
  }
  void testWriteTSIDiagHeader_GeoLoc2()
  {
    _locSFO.loc() = "PL";
    assertWriteTSIDiagHeader(6, 53, "PL");
  }
  void testWriteTSIDiagHeader_GeoReqBothReq()
  {
    _tsiInfo.geoRequired() = RuleConst::TSI_GEO_BOTH_REQUIRED;
    assertWriteTSIDiagHeader(11, 0, "REQ");
  }
  void testWriteTSIDiagHeader_GeoReqGeotypeReq()
  {
    _tsiInfo.geoRequired() = RuleConst::TSI_GEO_GEOTYPE_REQUIRED;
    assertWriteTSIDiagHeader(11, 0, "TYP");
  }
  void testWriteTSIDiagHeader_GeoReqFromItin()
  {
    _tsiInfo.geoRequired() = RuleConst::TSI_GEO_GET_FROM_ITIN;
    assertWriteTSIDiagHeader(11, 0, "GET");
  }
  void testWriteTSIDiagHeader_GeoReqBlank()
  {
    _tsiInfo.geoRequired() = RuleConst::TSI_GEO_BLANK;
    assertWriteTSIDiagHeader(11, 0, "   ");
  }
  void testWriteTSIDiagHeader_GeoNotCity()
  {
    _tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_CITY;
    assertWriteTSIDiagHeader(11, 5, "C");
  }
  void testWriteTSIDiagHeader_GeoNotBoth()
  {
    _tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_BOTH;
    assertWriteTSIDiagHeader(11, 5, "B");
  }
  void testWriteTSIDiagHeader_GeoNotThree()
  {
    _tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;
    assertWriteTSIDiagHeader(11, 5, "T");
  }
  void testWriteTSIDiagHeader_GeoNotBlank()
  {
    _tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_BLANK;
    assertWriteTSIDiagHeader(11, 5, " ");
  }
  void testWriteTSIDiagHeader_GeoOutInclude()
  {
    _tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_INCLUDE;
    assertWriteTSIDiagHeader(11, 8, "   ");
  }
  void testWriteTSIDiagHeader_GeoOutExclude()
  {
    _tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_EXCLUDE;
    assertWriteTSIDiagHeader(11, 8, "EXC");
  }
  void testWriteTSIDiagHeader_ItinPartOrig()
  {
    _tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_ORIG;
    assertWriteTSIDiagHeader(11, 12, "ORIG");
  }
  void testWriteTSIDiagHeader_ItinPartDest()
  {
    _tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_DEST;
    assertWriteTSIDiagHeader(11, 12, "DEST");
  }
  void testWriteTSIDiagHeader_ItinPartTurn()
  {
    _tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_TURNAROUND;
    assertWriteTSIDiagHeader(11, 12, "TURN");
  }
  void testWriteTSIDiagHeader_ItinPartBlank()
  {
    _tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_BLANK;
    assertWriteTSIDiagHeader(11, 12, "    ");
  }
  void testWriteTSIDiagHeader_GeoCheckOrig()
  {
    _tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG;
    assertWriteTSIDiagHeader(11, 17, "O ");
  }
  void testWriteTSIDiagHeader_GeoCheckDest()
  {
    _tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_DEST;
    assertWriteTSIDiagHeader(11, 17, "D ");
  }
  void testWriteTSIDiagHeader_GeoCheckOrigDest()
  {
    _tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG_DEST;
    assertWriteTSIDiagHeader(11, 17, "OD");
  }
  void testWriteTSIDiagHeader_LoopFrwd()
  {
    _tsiInfo.loopDirection() = RuleConst::TSI_LOOP_FORWARD;
    assertWriteTSIDiagHeader(11, 21, "FRWD");
  }
  void testWriteTSIDiagHeader_LoopBack()
  {
    _tsiInfo.loopDirection() = RuleConst::TSI_LOOP_BACKWARD;
    assertWriteTSIDiagHeader(11, 21, "BKWD");
  }
  void testWriteTSIDiagHeader_LoopFrwdBrk()
  {
    _tsiInfo.loopDirection() = RuleConst::TSI_LOOP_FORWARD_BREAK_ARRIVAL;
    assertWriteTSIDiagHeader(11, 21, "FWBA");
  }
  void testWriteTSIDiagHeader_LoopBackBrk()
  {
    _tsiInfo.loopDirection() = RuleConst::TSI_LOOP_BACKWARD_BREAK_DEPART;
    assertWriteTSIDiagHeader(11, 21, "BKBD");
  }
  void testWriteTSIDiagHeader_LoopOff2()
  {
    _tsiInfo.loopOffset() = 2;
    assertWriteTSIDiagHeader(11, 28, "2");
  }
  void testWriteTSIDiagHeader_LoopSetPrev()
  {
    _tsiInfo.loopToSet() = RuleConst::TSI_LOOP_SET_PREVIOUS;
    assertWriteTSIDiagHeader(11, 30, " -1");
  }
  void testWriteTSIDiagHeader_LoopSetCurr()
  {
    _tsiInfo.loopToSet() = RuleConst::TSI_LOOP_SET_CURRENT;
    assertWriteTSIDiagHeader(11, 30, "  0");
  }
  void testWriteTSIDiagHeader_LoopSetNext()
  {
    _tsiInfo.loopToSet() = RuleConst::TSI_LOOP_SET_NEXT;
    assertWriteTSIDiagHeader(11, 30, "  1");
  }
  void testWriteTSIDiagHeader_LoopSetCurNext()
  {
    _tsiInfo.loopToSet() = RuleConst::TSI_LOOP_SET_CUR_NEXT;
    assertWriteTSIDiagHeader(11, 30, "101");
  }
  void testWriteTSIDiagHeader_LoopSetCurPrev()
  {
    _tsiInfo.loopToSet() = RuleConst::TSI_LOOP_SET_CUR_PREV;
    assertWriteTSIDiagHeader(11, 30, " 99");
  }
  void testWriteTSIDiagHeader_LoopMatchAll()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_ALL;
    assertWriteTSIDiagHeader(11, 34, "ALL  ");
  }
  void testWriteTSIDiagHeader_LoopMatchFirst()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_FIRST;
    assertWriteTSIDiagHeader(11, 34, "FIRST");
  }
  void testWriteTSIDiagHeader_LoopMatchOnce()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_ONCE;
    assertWriteTSIDiagHeader(11, 34, "ONCE ");
  }
  void testWriteTSIDiagHeader_LoopMatchLast()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_LAST;
    assertWriteTSIDiagHeader(11, 34, "LAST ");
  }
  void testWriteTSIDiagHeader_LoopMatchFirstAll()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_FIRST_ALL;
    assertWriteTSIDiagHeader(11, 34, "1/ALL");
  }
  void testWriteTSIDiagHeader_LoopMatchSoft()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_SOFT_MATCH;
    assertWriteTSIDiagHeader(11, 34, "SOFT ");
  }
  void testWriteTSIDiagHeader_LoopMatchSecFirst()
  {
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_SECOND_FIRST;
    assertWriteTSIDiagHeader(11, 34, "2FRST");
  }
  void testWriteTSIDiagHeader_ScopeJour()
  {
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_JOURNEY;
    assertWriteTSIDiagHeader(11, 41, "JURNY");
  }
  void testWriteTSIDiagHeader_ScopeSubJour()
  {
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertWriteTSIDiagHeader(11, 41, "SUBJY");
  }
  void testWriteTSIDiagHeader_ScopeFare()
  {
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertWriteTSIDiagHeader(11, 41, "FARE ");
  }
  void testWriteTSIDiagHeader_ScopeSjFc()
  {
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_SJ_AND_FC;
    assertWriteTSIDiagHeader(11, 41, "SJ/FC");
  }
  void testWriteTSIDiagHeader_TypeOrig()
  {
    _tsiInfo.type() = RuleConst::TSI_APP_CHECK_ORIG;
    assertWriteTSIDiagHeader(11, 48, "O ");
  }
  void testWriteTSIDiagHeader_TypeDest()
  {
    _tsiInfo.type() = RuleConst::TSI_APP_CHECK_DEST;
    assertWriteTSIDiagHeader(11, 48, "D ");
  }
  void testWriteTSIDiagHeader_TypeOrigDest()
  {
    _tsiInfo.type() = RuleConst::TSI_APP_CHECK_ORIG_DEST;
    assertWriteTSIDiagHeader(11, 48, "OD");
  }
  void testWriteTSIDiagHeader_MatchStopover()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_STOP_OVER_DESC);
  }
  void testWriteTSIDiagHeader_MatchInbound()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::INBOUND);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_INBOUND_DESC);
  }
  void testWriteTSIDiagHeader_MatchOutbound()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::OUTBOUND);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_OUTBOUND_DESC);
  }
  void testWriteTSIDiagHeader_MatchDomestic()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::DOMESTIC);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_DOMESTIC_DESC);
  }
  void testWriteTSIDiagHeader_MatchOneCountry()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::ONE_COUNTRY);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_ONE_COUNTRY_DESC);
  }
  void testWriteTSIDiagHeader_MatchInternational()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::INTERNATIONAL);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_INTERNATIONAL_DESC);
  }
  void testWriteTSIDiagHeader_MatchGateway()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::GATEWAY);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testWriteTSIDiagHeader_MatchOrigGateway()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::ORIG_GATEWAY);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_ORIG_GATEWAY_DESC);
  }
  void testWriteTSIDiagHeader_MatchDestGateway()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::DEST_GATEWAY);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_DEST_GATEWAY_DESC);
  }
  void testWriteTSIDiagHeader_MatchTransTalantic()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::TRANS_ATLANTIC);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_TRANS_ATLANTIC_DESC);
  }
  void testWriteTSIDiagHeader_MatchTransPacific()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::TRANS_PACIFIC);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_TRANS_PACIFIC_DESC);
  }
  void testWriteTSIDiagHeader_MatchTransOceanic()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::TRANS_OCEANIC);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_TRANS_OCEANIC_DESC);
  }
  void testWriteTSIDiagHeader_MatchIntercontinental()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::INTERCONTINENTAL);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_INTERCONTINENTAL_DESC);
  }
  void testWriteTSIDiagHeader_MatchOverWater()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::OVER_WATER);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_OVER_WATER_DESC);
  }
  void testWriteTSIDiagHeader_MatchIntlDomTransfer()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::INTL_DOM_TRANSFER);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_INTL_DOM_TRANSFER_DESC);
  }
  void testWriteTSIDiagHeader_MatchFurthest()
  {
    _tsiInfo.matchCriteria().clear();
    _tsiInfo.matchCriteria().push_back(TSIInfo::FURTHEST);
    assertWriteTSIDiagHeader(14, 0, RuleConst::MATCH_FURTHEST_DESC);
  }

  void testWriteTSIDiagDetails_Line()
  {
    assertWriteTSIDiagTravelSeg(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL(std::string("   1    1   SFO SYD    --     --    NOT CHECKED\n"),
                         _diag->str());
  }
  void testWriteTSIDiagDetails_NoPuFc()
  {
    _displayPuFc = false;
    assertWriteTSIDiagTravelSeg(0, 0, "          ");
  }
  void testWriteTSIDiagDetails_PuNmbr()
  {
    _puNumber = 2;
    assertWriteTSIDiagTravelSeg(0, 3, "2");
  }
  void testWriteTSIDiagDetails_FcNmbr()
  {
    _fcNumber = 2;
    assertWriteTSIDiagTravelSeg(0, 8, "2");
  }
  void testWriteTSIDiagDetails_Stop()
  {
    _tsm.isStopover() = true;
    assertWriteTSIDiagTravelSeg(0, 10, "O ");
  }
  void testWriteTSIDiagDetails_Orig() { assertWriteTSIDiagTravelSeg(0, 12, "SFO"); }
  void testWriteTSIDiagDetails_Dest() { assertWriteTSIDiagTravelSeg(0, 16, "SYD"); }
  void testWriteTSIDiagDetails_NoMatch()
  {
    _tsm.match() = RuleConst::TSI_NOT_MATCH;
    assertWriteTSIDiagTravelSeg(0, 23, "--");
  }
  void testWriteTSIDiagDetails_MatchOrig()
  {
    _tsm.origMatch() = true;
    assertWriteTSIDiagTravelSeg(0, 23, "O");
  }
  void testWriteTSIDiagDetails_MatchDest()
  {
    _tsm.destMatch() = true;
    assertWriteTSIDiagTravelSeg(0, 24, "D");
  }
  void testWriteTSIDiagDetails_OrigSave()
  {
    _tsm.origSave() = true;
    assertWriteTSIDiagTravelSeg(0, 30, "O");
  }
  void testWriteTSIDiagDetails_DestSave()
  {
    _tsm.destSave() = true;
    assertWriteTSIDiagTravelSeg(0, 31, "D");
  }
  void testWriteTSIDiagDetails_NoMatchReason()
  {
    _tsm.noMatchReason() = "KUPA";
    assertWriteTSIDiagTravelSeg(0, 36, "KUPA");
  }

  void setUp()
  {
    RuleUtilTSITest_Base::setUp();
    _vendor = "ATP";
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsi = _memHandle.insert(new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, 0, 0, 0));

    _tsiInfo.tsi() = 1;
    _tsiInfo.description() = "DESCRIPTION";
    _tsiInfo.geoRequired() = RuleConst::TSI_GEO_BOTH_REQUIRED;
    _tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_CITY;
    _tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_INCLUDE;
    _tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_ORIG;
    _tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG;
    _tsiInfo.loopDirection() = RuleConst::TSI_LOOP_FORWARD;
    _tsiInfo.loopOffset() = 1;
    _tsiInfo.loopToSet() = RuleConst::TSI_LOOP_SET_PREVIOUS;
    _tsiInfo.loopMatch() = RuleConst::TSI_MATCH_ALL;
    _tsiInfo.scope() = RuleConst::TSI_SCOPE_JOURNEY;
    _tsiInfo.type() = RuleConst::TSI_APP_CHECK_ORIG;

    _defaultScope = RuleConst::TSI_SCOPE_PARAM_JOURNEY;

    _tsiInfo.matchCriteria() += TSIInfo::STOP_OVER, TSIInfo::INBOUND, TSIInfo::OUTBOUND,
        TSIInfo::DOMESTIC, TSIInfo::ONE_COUNTRY, TSIInfo::INTERNATIONAL, TSIInfo::GATEWAY,
        TSIInfo::ORIG_GATEWAY, TSIInfo::DEST_GATEWAY, TSIInfo::TRANS_ATLANTIC,
        TSIInfo::TRANS_PACIFIC, TSIInfo::TRANS_OCEANIC, TSIInfo::INTERCONTINENTAL,
        TSIInfo::OVER_WATER, TSIInfo::INTL_DOM_TRANSFER, TSIInfo::FURTHEST;

    _locSFO.locType() = 'C';
    _locSYD.locType() = 'C';
    _locSFO.loc() = "SFO";
    _locSYD.loc() = "SYD";
    _allowJourneyScopeOverride = true;
    _allowPUScopeOverride = true;
    _allowFCScopeOverride = true;

    _diag = _memHandle.insert(new DiagCollector(_diagRoot));
    _diagRoot.activate();
    _diag->enable(DiagnosticNone);
    _displayPuFc = 1;
    _puNumber = 1;
    _fcNumber = 1;
    _tsm.travelSeg() = _segSFOSYD;
    _tsm.match() = RuleConst::TSI_MATCH;
    _tsm.origMatch() = false;
    _tsm.destMatch() = false;
    _tsm.noMatchReason() = "NOT CHECKED";
  }

protected:
  void assertWriteTSIDiagHeader(int line, int chPos, const std::string& str)
  {
    assertWriteTSIDiagHeader(line, chPos, str.c_str());
  }
  void assertWriteTSIDiagHeader(int line, int chPos, const char* msg)
  {
    RuleUtilTSI::writeTSIDiagHeader(*_diag,
                                    *_tsi,
                                    _defaultScope,
                                    _tsiScope,
                                    _allowJourneyScopeOverride,
                                    _allowPUScopeOverride,
                                    _allowFCScopeOverride,
                                    _locSFO,
                                    _locSYD);

    if (msg)
      assertWriteTSIDiagText(line, chPos, msg, _diag->str());
  }
  void assertWriteTSIDiagTravelSeg(int line, int chPos, const char* msg)
  {
    RuleUtilTSI::writeTSIDiagTravelSeg(*_diag, _puNumber, _fcNumber, _displayPuFc, _tsm);
    if (msg)
      assertWriteTSIDiagText(line, chPos, msg, _diag->str());
  }
  void assertWriteTSIDiagText(int line, int chPos, const char* msg, const std::string& resp)
  {
    size_t offset = 0;

    for (int i = 0; i < line; ++i)
    {
      CPPUNIT_ASSERT(offset < resp.size());

      offset = resp.find('\n', offset);
      CPPUNIT_ASSERT(offset != std::string::npos);

      ++offset; // Skip the '\n'
    }

    const size_t pos = resp.find('\n', offset);
    CPPUNIT_ASSERT(pos == std::string::npos || int(pos - offset) >= chPos);

    offset += chPos;
    CPPUNIT_ASSERT(offset < resp.size());

    const int res = strncmp(&resp[offset], msg, strlen(msg));
    CPPUNIT_ASSERT(res == 0);
  }

private:
  RuleUtilTSI::TSITravelSegMarkup _tsm;
  RuleUtilTSI::TSIData* _tsi;
  TSIInfo _tsiInfo;
  VendorCode _vendor;
  RuleConst::TSIScopeType _tsiScope;
  RuleConst::TSIScopeParamType _defaultScope;
  DiagCollector* _diag;
  Diagnostic _diagRoot;
  LocKey _locSFO;
  LocKey _locSYD;
  bool _allowJourneyScopeOverride;
  bool _allowPUScopeOverride;
  bool _allowFCScopeOverride;
  int32_t _puNumber;
  int32_t _fcNumber;
  bool _displayPuFc;
};
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_GetGeoData);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_ReverseTravel);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_GetTSIOrigDestCheck);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_IdentifyIntlDomTransfers);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_ProcessTravelSegs);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_ScopeTSIGeo);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_WriteTSIDiag);
}
