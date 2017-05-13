#include "Rules/test/RuleUtilTSITest_Base.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class RuleUtilTSITest_GetLocationMatchings : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_GetLocationMatchings);
  CPPUNIT_TEST(testGetLocationMatchings_Match_NONE);
  CPPUNIT_TEST(testGetLocationMatchings_Match_C_SFO);
  CPPUNIT_TEST(testGetLocationMatchings_Match_A_1);
  CPPUNIT_TEST(testGetLocationMatchings_Match_S_11);
  CPPUNIT_TEST(testGetLocationMatchings_Match_N_US);

  CPPUNIT_TEST(testGetLocationMatchings_MatchNONE_MatchNONE);
  CPPUNIT_TEST(testGetLocationMatchings_MatchUS_MatchNONE);
  CPPUNIT_TEST(testGetLocationMatchings_MatchAU_MatchNONE);
  CPPUNIT_TEST(testGetLocationMatchings_MatchNONE_MatchUS);
  CPPUNIT_TEST(testGetLocationMatchings_MatchNONE_MatchAU);
  CPPUNIT_TEST(testGetLocationMatchings_MatchUS_MatchAU);
  CPPUNIT_TEST(testGetLocationMatchings_MatchAU_MatchUS);
  CPPUNIT_TEST_SUITE_END();

public:
  void testGetLocationMatchings_Match_NONE()
  {
    CPPUNIT_ASSERT(
        !RuleUtilTSI::getLocationMatchings(*_trx, *_locSFO, 'N', "XX", "ATP", GeoTravelType::International));
  }
  void testGetLocationMatchings_Match_C_SFO()
  {
    CPPUNIT_ASSERT(
        RuleUtilTSI::getLocationMatchings(*_trx, *_locSFO, 'C', "SFO", "ATP", GeoTravelType::International));
  }
  void testGetLocationMatchings_Match_A_1()
  {
    CPPUNIT_ASSERT(
        RuleUtilTSI::getLocationMatchings(*_trx, *_locSFO, 'A', "1", "ATP", GeoTravelType::International));
  }
  void testGetLocationMatchings_Match_S_11()
  {
    CPPUNIT_ASSERT(
        RuleUtilTSI::getLocationMatchings(*_trx, *_locSFO, '*', "11", "ATP", GeoTravelType::International));
  }
  void testGetLocationMatchings_Match_N_US()
  {
    CPPUNIT_ASSERT(
        RuleUtilTSI::getLocationMatchings(*_trx, *_locSFO, 'N', "US", "ATP", GeoTravelType::International));
  }
  void testGetLocationMatchings_MatchNONE_MatchNONE()
  {
    // no locations, match on all
    RuleUtilTSI::TSIData* tsi = initTSIData();
    assertGetLocationMatchings(tsi, true, true, true, true);
  }
  void testGetLocationMatchings_MatchUS_MatchNONE()
  {
    // no locations 2, match on loc1 origin
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "US");
    assertGetLocationMatchings(tsi, true, false, false, false);
  }
  void testGetLocationMatchings_MatchAU_MatchNONE()
  {
    // no locations 2, match on loc1 destination
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "AU");
    assertGetLocationMatchings(tsi, false, true, false, false);
  }
  void testGetLocationMatchings_MatchNONE_MatchUS()
  {
    // no locations 1, match on loc2 origin
    RuleUtilTSI::TSIData* tsi = initTSIData(' ', "", 'N', "US");
    assertGetLocationMatchings(tsi, false, false, true, false);
  }
  void testGetLocationMatchings_MatchNONE_MatchAU()
  {
    // no locations 1, match on loc2 destination
    RuleUtilTSI::TSIData* tsi = initTSIData(' ', "", 'N', "AU");
    assertGetLocationMatchings(tsi, false, false, false, true);
  }
  void testGetLocationMatchings_MatchUS_MatchAU()
  {
    // loc 1 match on origin, loc2 match on destiantion
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "US", 'N', "AU");
    assertGetLocationMatchings(tsi, true, false, false, true);
  }
  void testGetLocationMatchings_MatchAU_MatchUS()
  {
    // loc1 match on destination, loc2 match on origin
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "AU", 'N', "US");
    assertGetLocationMatchings(tsi, false, true, true, false);
  }

private:
  void assertGetLocationMatchings(
      RuleUtilTSI::TSIData* tsi, bool l1orig, bool l1dest, bool l2orig, bool l2dest)
  {
    bool loc1OrigMatch = false, loc1DestMatch = false, loc2OrigMatch = false, loc2DestMatch = false;
    RuleUtilTSI::getLocationMatchings(
        *_trx, *tsi, _segSFOSYD, loc1OrigMatch, loc1DestMatch, loc2OrigMatch, loc2DestMatch);
    CPPUNIT_ASSERT_EQUAL(l1orig, loc1OrigMatch);
    CPPUNIT_ASSERT_EQUAL(l1dest, loc1DestMatch);
    CPPUNIT_ASSERT_EQUAL(l2orig, loc2OrigMatch);
    CPPUNIT_ASSERT_EQUAL(l2dest, loc2DestMatch);
  }
};


class RuleUtilTSITest_CheckGeoData : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_CheckGeoData);
  CPPUNIT_TEST(testCheckGeoData_Inc_Orig_None);
  CPPUNIT_TEST(testCheckGeoData_Inc_Orig_locSFO);
  CPPUNIT_TEST(testCheckGeoData_Inc_Orig_locSYD);
  CPPUNIT_TEST(testCheckGeoData_Inc_Orig_Both);
  CPPUNIT_TEST(testCheckGeoData_Inc_Dest_None);
  CPPUNIT_TEST(testCheckGeoData_Inc_Dest_locSFO);
  CPPUNIT_TEST(testCheckGeoData_Inc_Dest_locSYD);
  CPPUNIT_TEST(testCheckGeoData_Inc_Dest_Both);
  CPPUNIT_TEST(testCheckGeoData_Inc_Both_None);
  CPPUNIT_TEST(testCheckGeoData_Inc_Both_locSFO);
  CPPUNIT_TEST(testCheckGeoData_Inc_Both_locSYD);
  CPPUNIT_TEST(testCheckGeoData_Inc_Both_Both);
  CPPUNIT_TEST(testCheckGeoData_Exc_Orig_None);
  CPPUNIT_TEST(testCheckGeoData_Exc_Orig_locSFO);
  CPPUNIT_TEST(testCheckGeoData_Exc_Orig_locSYD);
  CPPUNIT_TEST(testCheckGeoData_Exc_Orig_Both);
  CPPUNIT_TEST(testCheckGeoData_Exc_Dest_None);
  CPPUNIT_TEST(testCheckGeoData_Exc_Dest_locSFO);
  CPPUNIT_TEST(testCheckGeoData_Exc_Dest_locSYD);
  CPPUNIT_TEST(testCheckGeoData_Exc_Dest_Both);
  CPPUNIT_TEST(testCheckGeoData_Exc_Both_None);
  CPPUNIT_TEST(testCheckGeoData_Exc_Both_locSFO);
  CPPUNIT_TEST(testCheckGeoData_Exc_Both_locSYD);
  CPPUNIT_TEST(testCheckGeoData_Exc_Both_Both);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCheckGeoData_Inc_Orig_None()
  {
    // loc1 and loc2 don't match, fail checkGeo
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "PL", 'N', "UK", 'O');
    assertCheckGeoData(tsi, false, false, false);
  }
  void testCheckGeoData_Inc_Orig_locSFO()
  {
    // loc1 match and loc2 don't match, pass checkGeo, orig match
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "US", 'N', "UK", 'O');
    assertCheckGeoData(tsi, true, true, false);
  }
  void testCheckGeoData_Inc_Orig_locSYD()
  {
    // loc1 don't match and loc2 match, pass checkGeo, orig match
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "PL", 'N', "US", 'O');
    assertCheckGeoData(tsi, true, true, false);
  }
  void testCheckGeoData_Inc_Orig_Both()
  {
    // loc1 and loc2 match, pass checkGeo, orig match
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "US", 'N', "US", 'O');
    assertCheckGeoData(tsi, true, true, false);
  }
  void testCheckGeoData_Inc_Dest_None()
  {
    // loc1 and loc2 don't match, fail checkGeo
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "PL", 'N', "UK", 'D');
    assertCheckGeoData(tsi, false, false, false);
  }
  void testCheckGeoData_Inc_Dest_locSFO()
  {
    // loc1 match and loc2 don't match, pass checkGeo, match on dest
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "AU", 'N', "UK", 'D');
    assertCheckGeoData(tsi, true, false, true);
  }
  void testCheckGeoData_Inc_Dest_locSYD()
  {
    // loc1 don't match and loc2 match, pass checkGeo, match on dest
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "PL", 'N', "AU", 'D');
    assertCheckGeoData(tsi, true, false, true);
  }
  void testCheckGeoData_Inc_Dest_Both()
  {
    // loc1 and loc2 match, pass checkGeo, match on dest
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "AU", 'N', "AU", 'D');
    assertCheckGeoData(tsi, true, false, true);
  }
  void testCheckGeoData_Inc_Both_None()
  {
    // loc1 and loc2 don't match, fail checkGeo
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "PL", 'N', "UK", 'B');
    assertCheckGeoData(tsi, false, false, false);
  }
  void testCheckGeoData_Inc_Both_locSFO()
  {
    // loc1 macth and loc2 don't match, pass checkGeo, match on origin
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "US", 'N', "UK", 'B');
    assertCheckGeoData(tsi, true, true, false);
  }
  void testCheckGeoData_Inc_Both_locSYD()
  {
    // loc1 don't macth and loc2 't match, pass checkGeo, match on destination
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "PL", 'N', "AU", 'B');
    assertCheckGeoData(tsi, true, false, true);
  }
  void testCheckGeoData_Inc_Both_Both()
  {
    // loc1 loc2 't match, pass checkGeo, match on origin and destination
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "US", 'N', "AU", 'B');
    assertCheckGeoData(tsi, true, true, true);
  }
  void testCheckGeoData_Exc_Orig_None()
  {
    // loc1 and loc2 don't match, pass checkGeo, match on origin and destination
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "PL", 'N', "UK", 'O', 'E');
    assertCheckGeoData(tsi, true, true, false);
  }
  void testCheckGeoData_Exc_Orig_locSFO()
  {
    // loc1 match and loc2 don't match, fail
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "US", 'N', "UK", 'O', 'E');
    assertCheckGeoData(tsi, false, false, false);
  }
  void testCheckGeoData_Exc_Orig_locSYD()
  {
    // loc1 don't match and loc2 t match, fail
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "PL", 'N', "US", 'O', 'E');
    assertCheckGeoData(tsi, false, false, false);
  }
  void testCheckGeoData_Exc_Orig_Both()
  {
    // loc1 and loc2 t match, fail checkGeo
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "US", 'N', "US", 'O', 'E');
    assertCheckGeoData(tsi, false, false, false);
  }
  void testCheckGeoData_Exc_Dest_None()
  {
    // loc1 and loc2 t do'nt match, pass checkGeo, match on destination
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "PL", 'N', "UK", 'D', 'E');
    assertCheckGeoData(tsi, true, false, true);
  }
  void testCheckGeoData_Exc_Dest_locSFO()
  {
    // loc1 match and loc2 t do'nt match, fail
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "AU", 'N', "UK", 'D', 'E');
    assertCheckGeoData(tsi, false, false, false);
  }
  void testCheckGeoData_Exc_Dest_locSYD()
  {
    // loc1 don't match and loc2 t match, fail checkGeo
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "PL", 'N', "AU", 'D', 'E');
    assertCheckGeoData(tsi, false, false, false);
  }
  void testCheckGeoData_Exc_Dest_Both()
  {
    // loc1 and loc2 t match, fail checkGeo
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "US", 'N', "AU", 'D', 'E');
    assertCheckGeoData(tsi, false, false, false);
  }
  void testCheckGeoData_Exc_Both_None()
  {
    // loc1 and loc2 t do'nt match, pass checkGeo, match on origin and destination
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "PL", 'N', "UK", 'B', 'E');
    assertCheckGeoData(tsi, true, true, true);
  }
  void testCheckGeoData_Exc_Both_locSFO()
  {
    // loc1 match and loc2 t don't match, pass checkGeo, match on destination
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "US", 'N', "UK", 'B', 'E');
    assertCheckGeoData(tsi, true, false, true);
  }
  void testCheckGeoData_Exc_Both_locSYD()
  {
    // loc1 don't match and loc2 t match, pass checkGeo, match on origin
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "PL", 'N', "AU", 'B', 'E');
    assertCheckGeoData(tsi, true, true, false);
  }
  void testCheckGeoData_Exc_Both_Both()
  {
    // loc1 and loc2 t match, fail checkGeo
    RuleUtilTSI::TSIData* tsi = initTSIData('N', "US", 'N', "AU", 'B', 'E');
    assertCheckGeoData(tsi, false, false, false);
  }

private:
  void assertCheckGeoData(RuleUtilTSI::TSIData* tsi, bool res, bool orig, bool dest)
  {
    bool origMatch, destMatch;
    CPPUNIT_ASSERT_EQUAL(res,
                         RuleUtilTSI::checkGeoData(*_trx, *tsi, _segSFOSYD, origMatch, destMatch));
    CPPUNIT_ASSERT_EQUAL(orig, origMatch);
    CPPUNIT_ASSERT_EQUAL(dest, destMatch);
  }
};

class RuleUtilTSITest_CheckGeoNotType : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_CheckGeoNotType);
  CPPUNIT_TEST(testCheckGeoNotType_City_locSFOCity);
  CPPUNIT_TEST(testCheckGeoNotType_City_locSYDCity);
  CPPUNIT_TEST(testCheckGeoNotType_City_locSFOAirport);
  CPPUNIT_TEST(testCheckGeoNotType_City_locSYDAirport);
  CPPUNIT_TEST(testCheckGeoNotType_City_locSFOZone);
  CPPUNIT_TEST(testCheckGeoNotType_City_locSYDZone);
  CPPUNIT_TEST(testCheckGeoNotType_City_locSFONation);
  CPPUNIT_TEST(testCheckGeoNotType_City_locSYDNation);
  CPPUNIT_TEST(testCheckGeoNotType_Both_locSFOCity);
  CPPUNIT_TEST(testCheckGeoNotType_Both_locSYDCity);
  CPPUNIT_TEST(testCheckGeoNotType_Both_locSFOAirport);
  CPPUNIT_TEST(testCheckGeoNotType_Both_locSYDAirport);
  CPPUNIT_TEST(testCheckGeoNotType_Both_locSFOZone);
  CPPUNIT_TEST(testCheckGeoNotType_Both_locSYDZone);
  CPPUNIT_TEST(testCheckGeoNotType_Both_locSFONation);
  CPPUNIT_TEST(testCheckGeoNotType_Both_locSYDNation);
  CPPUNIT_TEST(testCheckGeoNotType_Three_locSFOCity);
  CPPUNIT_TEST(testCheckGeoNotType_Three_locSYDCity);
  CPPUNIT_TEST(testCheckGeoNotType_Three_locSFOAirport);
  CPPUNIT_TEST(testCheckGeoNotType_Three_locSYDAirport);
  CPPUNIT_TEST(testCheckGeoNotType_Three_locSFOZone);
  CPPUNIT_TEST(testCheckGeoNotType_Three_locSYDZone);
  CPPUNIT_TEST(testCheckGeoNotType_Three_locSFONation);
  CPPUNIT_TEST(testCheckGeoNotType_Three_locSYDNation);
  CPPUNIT_TEST(testCheckGeoNotType_NotBlank);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCheckGeoNotType_City_locSFOCity()
  {
    // loc1 = city, fail
    RuleUtilTSI::TSIData* tsi =
        initTSIData(LOCTYPE_CITY, "", ' ', "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_CITY);
    CPPUNIT_ASSERT(!RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_City_locSYDCity()
  {
    // loc2 = city, fail
    RuleUtilTSI::TSIData* tsi =
        initTSIData(' ', "", LOCTYPE_CITY, "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_CITY);
    CPPUNIT_ASSERT(!RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_City_locSFOAirport()
  {
    // loc1 = airport, pass
    RuleUtilTSI::TSIData* tsi =
        initTSIData(LOCTYPE_AIRPORT, "", ' ', "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_CITY);
    CPPUNIT_ASSERT(RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_City_locSYDAirport()
  {
    // loc2 = airport, pass
    RuleUtilTSI::TSIData* tsi =
        initTSIData(' ', "", LOCTYPE_AIRPORT, "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_CITY);
    CPPUNIT_ASSERT(RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_City_locSFOZone()
  {
    // loc1 = zone, pass
    RuleUtilTSI::TSIData* tsi =
        initTSIData(LOCTYPE_ZONE, "", ' ', "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_CITY);
    CPPUNIT_ASSERT(RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_City_locSYDZone()
  {
    // loc1 = zone, pass
    RuleUtilTSI::TSIData* tsi =
        initTSIData(' ', "", LOCTYPE_ZONE, "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_CITY);
    CPPUNIT_ASSERT(RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_City_locSFONation()
  {
    // loc1 = nation, pass
    RuleUtilTSI::TSIData* tsi =
        initTSIData(LOCTYPE_NATION, "", ' ', "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_CITY);
    CPPUNIT_ASSERT(RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_City_locSYDNation()
  {
    // loc2 = nation, pass
    RuleUtilTSI::TSIData* tsi =
        initTSIData(' ', "", LOCTYPE_NATION, "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_CITY);
    CPPUNIT_ASSERT(RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Both_locSFOCity()
  {
    // loc1 = city, fail
    RuleUtilTSI::TSIData* tsi =
        initTSIData(LOCTYPE_CITY, "", ' ', "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_BOTH);
    CPPUNIT_ASSERT(!RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Both_locSYDCity()
  {
    // loc2 = city, fail
    RuleUtilTSI::TSIData* tsi =
        initTSIData(' ', "", LOCTYPE_CITY, "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_BOTH);
    CPPUNIT_ASSERT(!RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Both_locSFOAirport()
  {
    // loc1 = airport, fail
    RuleUtilTSI::TSIData* tsi =
        initTSIData(LOCTYPE_AIRPORT, "", ' ', "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_BOTH);
    CPPUNIT_ASSERT(!RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Both_locSYDAirport()
  {
    // loc2 = airport, fail
    RuleUtilTSI::TSIData* tsi =
        initTSIData(' ', "", LOCTYPE_AIRPORT, "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_BOTH);
    CPPUNIT_ASSERT(!RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Both_locSFOZone()
  {
    // loc1 = zone, pass
    RuleUtilTSI::TSIData* tsi =
        initTSIData(LOCTYPE_ZONE, "", ' ', "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_BOTH);
    CPPUNIT_ASSERT(RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Both_locSYDZone()
  {
    // loc1 = zone, pass
    RuleUtilTSI::TSIData* tsi =
        initTSIData(' ', "", LOCTYPE_ZONE, "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_BOTH);
    CPPUNIT_ASSERT(RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Both_locSFONation()
  {
    // loc1 = nation, pass
    RuleUtilTSI::TSIData* tsi =
        initTSIData(LOCTYPE_NATION, "", ' ', "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_BOTH);
    CPPUNIT_ASSERT(RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Both_locSYDNation()
  {
    // loc2 = nation, pass
    RuleUtilTSI::TSIData* tsi =
        initTSIData(' ', "", LOCTYPE_NATION, "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_BOTH);
    CPPUNIT_ASSERT(RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Three_locSFOCity()
  {
    // loc1 = city, fail
    RuleUtilTSI::TSIData* tsi =
        initTSIData(LOCTYPE_CITY, "", ' ', "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_THREE);
    CPPUNIT_ASSERT(!RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Three_locSYDCity()
  {
    // loc2 = city, fail
    RuleUtilTSI::TSIData* tsi =
        initTSIData(' ', "", LOCTYPE_CITY, "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_THREE);
    CPPUNIT_ASSERT(!RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Three_locSFOAirport()
  {
    // loc1 = airport, fail
    RuleUtilTSI::TSIData* tsi =
        initTSIData(LOCTYPE_AIRPORT, "", ' ', "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_THREE);
    CPPUNIT_ASSERT(!RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Three_locSYDAirport()
  {
    // loc2 = airport, fail
    RuleUtilTSI::TSIData* tsi =
        initTSIData(' ', "", LOCTYPE_AIRPORT, "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_THREE);
    CPPUNIT_ASSERT(!RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Three_locSFOZone()
  {
    // loc1 = zone, fail
    RuleUtilTSI::TSIData* tsi =
        initTSIData(LOCTYPE_ZONE, "", ' ', "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_THREE);
    CPPUNIT_ASSERT(!RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Three_locSYDZone()
  {
    // loc1 = zone, fail
    RuleUtilTSI::TSIData* tsi =
        initTSIData(' ', "", LOCTYPE_ZONE, "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_THREE);
    CPPUNIT_ASSERT(!RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Three_locSFONation()
  {
    // loc1 = nation, pass
    RuleUtilTSI::TSIData* tsi =
        initTSIData(LOCTYPE_NATION, "", ' ', "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_THREE);
    CPPUNIT_ASSERT(RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_Three_locSYDNation()
  {
    RuleUtilTSI::TSIData* tsi =
        initTSIData(' ', "", LOCTYPE_NATION, "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_THREE);
    // loc2 = nation, pass

    CPPUNIT_ASSERT(RuleUtilTSI::checkGeoNotType(*tsi));
  }
  void testCheckGeoNotType_NotBlank()
  {
    // loc1 = blank, pass always??
    RuleUtilTSI::TSIData* tsi =
        initTSIData(' ', "", ' ', "", ' ', ' ', RuleConst::TSI_GEO_NOT_TYPE_BLANK);
    CPPUNIT_ASSERT(RuleUtilTSI::checkGeoNotType(*tsi));
  }
};

class RuleUtilTSITest_GetLoopItemToSetLoopSet : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_GetLoopItemToSetLoopSet);
  CPPUNIT_TEST(testGetLoopItemToSetLoopSetPrevious);
  CPPUNIT_TEST(testGetLoopItemToSetLoopSetCurrent);
  CPPUNIT_TEST(testGetLoopItemToSetLoopSetNext);
  CPPUNIT_TEST(testGetLoopItemToSetLoopSetCurNext);
  CPPUNIT_TEST(testGetLoopItemToSetLoopSetCurPrevious);
  CPPUNIT_TEST_SUITE_END();

public:
  void testGetLoopItemToSetLoopSetPrevious()
  {
    _tsi.loopToSet() = RuleConst::TSI_LOOP_SET_PREVIOUS; // Mark previous item only
    assertGetLoopItemToSetLoopSet(true, false, false);
  }
  void testGetLoopItemToSetLoopSetCurrent()
  {
    _tsi.loopToSet() = RuleConst::TSI_LOOP_SET_CURRENT; // Current item only
    assertGetLoopItemToSetLoopSet(false, true, false);
  }
  void testGetLoopItemToSetLoopSetNext()
  {
    _tsi.loopToSet() = RuleConst::TSI_LOOP_SET_NEXT; // Next item only
    assertGetLoopItemToSetLoopSet(false, false, true);
  }
  void testGetLoopItemToSetLoopSetCurNext()
  {
    _tsi.loopToSet() = RuleConst::TSI_LOOP_SET_CUR_NEXT; // Current and next items
    assertGetLoopItemToSetLoopSet(false, true, true);
  }
  void testGetLoopItemToSetLoopSetCurPrevious()
  {
    _tsi.loopToSet() = RuleConst::TSI_LOOP_SET_CUR_PREV; // Current and previous items
    assertGetLoopItemToSetLoopSet(true, true, false);
  }

private:
  void assertGetLoopItemToSetLoopSet(bool prev, bool curr, bool next)
  {
    bool addPrevTravSeg, addCurrTravSeg, addNextTravSeg;
    RuleUtilTSI::getLoopItemToSet(_tsi, addPrevTravSeg, addCurrTravSeg, addNextTravSeg);
    CPPUNIT_ASSERT_EQUAL(prev, addPrevTravSeg);
    CPPUNIT_ASSERT_EQUAL(curr, addCurrTravSeg);
    CPPUNIT_ASSERT_EQUAL(next, addNextTravSeg);
  }
  TSIInfo _tsi;
};

class RuleUtilTSITest_CheckLoopMatch : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_CheckLoopMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchAll_LoopPrev_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchAll_LoopPrev_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchAll_LoopCur_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchAll_LoopCur_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchAll_LoopNext_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchAll_LoopNext_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchAll_LoopCurNext_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchAll_LoopCurNext_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchAll_LoopCurPrev_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchAll_LoopCurPrev_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirst_LoopPrev_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirst_LoopPrev_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirst_LoopCur_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirst_LoopCur_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirst_LoopNext_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirst_LoopNext_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirst_LoopCurNext_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirst_LoopCurNext_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirst_LoopCurPrev_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirst_LoopCurPrev_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopPrev_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopPrev_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopPrev_CurNotMatchTsi52Loc);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopPrev_CurNotMatchTsi52NotLoc);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopCur_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopCur_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopCur_CurNotMatchTsi52Loc);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopCur_CurNotMatchTsi52NotLoc);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopNext_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopNext_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopNext_CurNotMatchTsi52Loc);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopNext_CurNotMatchTsi52NotLoc);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopCurNext_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopCurNext_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopCurNext_CurNotMatchTsi52Loc);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopCurNext_CurNotMatchTsi52NotLoc);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopCurPrev_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopCurPrev_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopCurPrev_CurNotMatchTsi52Loc);
  CPPUNIT_TEST(testCheckLoopMatch_MatchOnce_LoopCurPrev_CurNotMatchTsi52NotLoc);
  CPPUNIT_TEST(testCheckLoopMatch_MatchLast);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopPrev_CurMatch_SubJour);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopPrev_CurMatch_FareComp);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopPrev_CurMatch_Other);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopPrev_CurNotMatch_SubJour);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopPrev_CurNotMatch_FareComp);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopPrev_CurNotMatch_Other);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCur_CurMatch_SubJour);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCur_CurMatch_FareComp);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCur_CurMatch_Other);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCur_CurNotMatch_SubJour);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCur_CurNotMatch_FareComp);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCur_CurNotMatch_Other);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopNext_CurMatch_SubJour);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopNext_CurMatch_FareComp);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopNext_CurMatch_Other);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopNext_CurNotMatch_SubJour);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopNext_CurNotMatch_FareComp);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopNext_CurNotMatch_Other);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCurNext_CurMatch_SubJour);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCurNext_CurMatch_FareComp);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCurNext_CurMatch_Other);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCurNext_CurNotMatch_SubJour);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCurNext_CurNotMatch_FareComp);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCurNext_CurNotMatch_Other);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCurPrev_CurMatch_SubJour);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCurPrev_CurMatch_FareComp);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCurPrev_CurMatch_Other);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCurPrev_CurNotMatch_SubJour);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCurPrev_CurNotMatch_FareComp);
  CPPUNIT_TEST(testCheckLoopMatch_MatchFirstAll_LoopCurPrev_CurNotMatch_Other);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopPrev_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopPrev_CurSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopPrev_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopCur_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopCur_CurSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopCur_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopNext_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopNext_CurSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopNext_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopCurNext_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopCurNext_CurSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopCurNext_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopCurPrev_CurMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopCurPrev_CurSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSoft_LoopCurPrev_CurNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurMatch_PrevNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurSoftMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurSoftMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurSoftMatch_PrevNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurNotMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurNotMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurNotMatch_PrevNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCur_CurMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCur_CurMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCur_CurMatch_PrevNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCur_CurSoftMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCur_CurSoftMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCur_CurSoftMatch_PrevNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCur_CurNotMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCur_CurNotMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCur_CurNotMatch_PrevNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopNext_CurMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopNext_CurMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopNext_CurMatch_PrevNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopNext_CurSoftMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopNext_CurSoftMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopNext_CurSoftMatch_PrevNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopNext_CurNotMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopNext_CurNotMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopNext_CurNotMatch_PrevNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurMatch_PrevNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurSoftMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurSoftMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurSoftMatch_PrevNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurNotMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurNotMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurNotMatch_PrevNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurMatch_PrevNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurSoftMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurSoftMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurSoftMatch_PrevNotMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurNotMatch_PrevMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurNotMatch_PrevSoftMatch);
  CPPUNIT_TEST(testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurNotMatch_PrevNotMatch);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCheckLoopMatch_MatchAll_LoopPrev_CurMatch()
  {
    // match all, current match, return previous and continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, false, false, true);
  }
  void testCheckLoopMatch_MatchAll_LoopPrev_CurNotMatch()
  {
    // match all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchAll_LoopCur_CurMatch()
  {
    // match all, current match, return current and continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, false, true);
  }
  void testCheckLoopMatch_MatchAll_LoopCur_CurNotMatch()
  {
    // match all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchAll_LoopNext_CurMatch()
  {
    // match all, current match, return next and continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, true, true);
  }
  void testCheckLoopMatch_MatchAll_LoopNext_CurNotMatch()
  {
    // match all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchAll_LoopCurNext_CurMatch()
  {
    // match all, current match, return current and next and continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, true, true);
  }
  void testCheckLoopMatch_MatchAll_LoopCurNext_CurNotMatch()
  {
    // match all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchAll_LoopCurPrev_CurMatch()
  {
    // match all, current match, return current, previous and continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, true, false, true);
  }
  void testCheckLoopMatch_MatchAll_LoopCurPrev_CurNotMatch()
  {
    // match all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ALL, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirst_LoopPrev_CurMatch()
  {
    // match first, current match, return previous
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_FIRST, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, false, false, false);
  }
  void testCheckLoopMatch_MatchFirst_LoopPrev_CurNotMatch()
  {
    // match first, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_FIRST, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirst_LoopCur_CurMatch()
  {
    // match first, current match, return current
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, false, false);
  }
  void testCheckLoopMatch_MatchFirst_LoopCur_CurNotMatch()
  {
    // match first, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirst_LoopNext_CurMatch()
  {
    // match first, current match, return next
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_FIRST, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, true, false);
  }
  void testCheckLoopMatch_MatchFirst_LoopNext_CurNotMatch()
  {
    // match first, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_FIRST, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirst_LoopCurNext_CurMatch()
  {
    // match first, current match, return current and next
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_FIRST, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, true, false);
  }
  void testCheckLoopMatch_MatchFirst_LoopCurNext_CurNotMatch()
  {
    // match first, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_FIRST, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirst_LoopCurPrev_CurMatch()
  {
    // match first, current match, return current, previous
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_FIRST, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, true, false, false);
  }
  void testCheckLoopMatch_MatchFirst_LoopCurPrev_CurNotMatch()
  {
    // match first, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_FIRST, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchOnce_LoopPrev_CurMatch()
  {
    // match once, current match, return previous
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, false, false, false);
  }
  void testCheckLoopMatch_MatchOnce_LoopPrev_CurNotMatch()
  {
    // match once, current not match, tsi != 52, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchOnce_LoopPrev_CurNotMatchTsi52Loc()
  {
    // match once, current not match, tsi =52, loc specified, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_PREVIOUS, 52);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, true, false, false, false, true);
  }
  void testCheckLoopMatch_MatchOnce_LoopPrev_CurNotMatchTsi52NotLoc()
  {
    // match once, current not match, tsi=52, loc not specified, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_PREVIOUS, 52);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchOnce_LoopCur_CurMatch()
  {
    // match once, current match, return current
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, false, false);
  }
  void testCheckLoopMatch_MatchOnce_LoopCur_CurNotMatch()
  {
    // match once, current not match, tsi !=52, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchOnce_LoopCur_CurNotMatchTsi52Loc()
  {
    // match once, current not match, tsi=52, loc specified, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_CURRENT, 52);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, true, false, false, false, true);
  }
  void testCheckLoopMatch_MatchOnce_LoopCur_CurNotMatchTsi52NotLoc()
  {
    // match once, current not match, tsi=52, loc not specified, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_CURRENT, 52);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchOnce_LoopNext_CurMatch()
  {
    // match once, current match, return next
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, true, false);
  }
  void testCheckLoopMatch_MatchOnce_LoopNext_CurNotMatch()
  {
    // match once, current not match, tsi!=52, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchOnce_LoopNext_CurNotMatchTsi52Loc()
  {
    // match once, current not match, tsi=52, loc specified, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_NEXT, 52);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, true, false, false, false, true);
  }
  void testCheckLoopMatch_MatchOnce_LoopNext_CurNotMatchTsi52NotLoc()
  {
    // match once, current not match, tsi=52, loc not specified, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_NEXT, 52);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchOnce_LoopCurNext_CurMatch()
  {
    // match once, current match, return current and next
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, true, false);
  }
  void testCheckLoopMatch_MatchOnce_LoopCurNext_CurNotMatch()
  {
    // match once, current not match, tsi !=52, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchOnce_LoopCurNext_CurNotMatchTsi52Loc()
  {
    // match once, current not match, tsi=52, loc specified, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_CUR_NEXT, 52);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, true, false, false, false, true);
  }
  void testCheckLoopMatch_MatchOnce_LoopCurNext_CurNotMatchTsi52NotLoc()
  {
    // match once, current not match, tsi=52, loc not specified, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_CUR_NEXT, 52);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchOnce_LoopCurPrev_CurMatch()
  {
    // match once, current match, return current, previous
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, true, false, false);
  }
  void testCheckLoopMatch_MatchOnce_LoopCurPrev_CurNotMatch()
  {
    // match once, current not match, tsi !=52, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchOnce_LoopCurPrev_CurNotMatchTsi52Loc()
  {
    // match once, current not match, tsi=52, loc specified, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_CUR_PREV, 52);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, true, false, false, false, true);
  }
  void testCheckLoopMatch_MatchOnce_LoopCurPrev_CurNotMatchTsi52NotLoc()
  {
    // match once, current not match, tsi=52, loc not specified, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_ONCE, RuleConst::TSI_LOOP_SET_CUR_PREV, 52);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchLast()
  {
    // match last, unsupported, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_LAST, RuleConst::TSI_LOOP_SET_CUR_PREV, 52);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopPrev_CurMatch_SubJour()
  {
    // match first all, current match, return previous and continue loop
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_PREVIOUS,
                                                1,
                                                RuleConst::TSI_SCOPE_SUB_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, false, false, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopPrev_CurMatch_FareComp()
  {
    // match first all, current match, return previous
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_PREVIOUS,
                                                1,
                                                RuleConst::TSI_SCOPE_FARE_COMPONENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, false, false, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopPrev_CurMatch_Other()
  {
    // match first all, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_PREVIOUS,
                                                1,
                                                RuleConst::TSI_SCOPE_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopPrev_CurNotMatch_SubJour()
  {
    // match first all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_PREVIOUS,
                                                1,
                                                RuleConst::TSI_SCOPE_SUB_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopPrev_CurNotMatch_FareComp()
  {
    // match first all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_PREVIOUS,
                                                1,
                                                RuleConst::TSI_SCOPE_FARE_COMPONENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopPrev_CurNotMatch_Other()
  {
    // match first all, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_PREVIOUS,
                                                1,
                                                RuleConst::TSI_SCOPE_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCur_CurMatch_SubJour()
  {
    // match first all, current match, return current and continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CURRENT,
                                                1,
                                                RuleConst::TSI_SCOPE_SUB_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, false, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCur_CurMatch_FareComp()
  {
    // match first all, current match, return current
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CURRENT,
                                                1,
                                                RuleConst::TSI_SCOPE_FARE_COMPONENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, false, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCur_CurMatch_Other()
  {
    // match first all, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CURRENT,
                                                1,
                                                RuleConst::TSI_SCOPE_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCur_CurNotMatch_SubJour()
  {
    // match first all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CURRENT,
                                                1,
                                                RuleConst::TSI_SCOPE_SUB_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCur_CurNotMatch_FareComp()
  {
    // match first all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CURRENT,
                                                1,
                                                RuleConst::TSI_SCOPE_FARE_COMPONENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCur_CurNotMatch_Other()
  {
    // match first all, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CURRENT,
                                                1,
                                                RuleConst::TSI_SCOPE_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopNext_CurMatch_SubJour()
  {
    // match first all, current match, return next and continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_NEXT,
                                                1,
                                                RuleConst::TSI_SCOPE_SUB_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, true, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopNext_CurMatch_FareComp()
  {
    // match first all, current match, return next
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_NEXT,
                                                1,
                                                RuleConst::TSI_SCOPE_FARE_COMPONENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, true, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopNext_CurMatch_Other()
  {
    // match first all, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_NEXT,
                                                1,
                                                RuleConst::TSI_SCOPE_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopNext_CurNotMatch_SubJour()
  {
    // match first all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_NEXT,
                                                1,
                                                RuleConst::TSI_SCOPE_SUB_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopNext_CurNotMatch_FareComp()
  {
    // match first all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_NEXT,
                                                1,
                                                RuleConst::TSI_SCOPE_FARE_COMPONENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopNext_CurNotMatch_Other()
  {
    // match first all, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_NEXT,
                                                1,
                                                RuleConst::TSI_SCOPE_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCurNext_CurMatch_SubJour()
  {
    // match first all, current match, return current and next and continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CUR_NEXT,
                                                1,
                                                RuleConst::TSI_SCOPE_SUB_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, true, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCurNext_CurMatch_FareComp()
  {
    // match first all, current match, return current and next
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CUR_NEXT,
                                                1,
                                                RuleConst::TSI_SCOPE_FARE_COMPONENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, true, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCurNext_CurMatch_Other()
  {
    // match first all, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CUR_NEXT,
                                                1,
                                                RuleConst::TSI_SCOPE_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCurNext_CurNotMatch_SubJour()
  {
    // match first all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CUR_NEXT,
                                                1,
                                                RuleConst::TSI_SCOPE_SUB_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCurNext_CurNotMatch_FareComp()
  {
    // match first all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CUR_NEXT,
                                                1,
                                                RuleConst::TSI_SCOPE_FARE_COMPONENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCurNext_CurNotMatch_Other()
  {
    // match first all, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CUR_NEXT,
                                                1,
                                                RuleConst::TSI_SCOPE_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCurPrev_CurMatch_SubJour()
  {
    // match first all, current match, return current, previous and continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CUR_PREV,
                                                1,
                                                RuleConst::TSI_SCOPE_SUB_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, true, false, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCurPrev_CurMatch_FareComp()
  {
    // match first all, current match, return current, previous
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CUR_PREV,
                                                1,
                                                RuleConst::TSI_SCOPE_FARE_COMPONENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, true, false, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCurPrev_CurMatch_Other()
  {
    // match first all, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CUR_PREV,
                                                1,
                                                RuleConst::TSI_SCOPE_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCurPrev_CurNotMatch_SubJour()
  {
    // match first all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CUR_PREV,
                                                1,
                                                RuleConst::TSI_SCOPE_SUB_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCurPrev_CurNotMatch_FareComp()
  {
    // match first all, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CUR_PREV,
                                                1,
                                                RuleConst::TSI_SCOPE_FARE_COMPONENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchFirstAll_LoopCurPrev_CurNotMatch_Other()
  {
    // match first all, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi = initTSIDataLoop(RuleConst::TSI_MATCH_FIRST_ALL,
                                                RuleConst::TSI_LOOP_SET_CUR_PREV,
                                                1,
                                                RuleConst::TSI_SCOPE_JOURNEY);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false);
  }
  void testCheckLoopMatch_MatchSoft_LoopPrev_CurMatch()
  {
    // match soft, current match, return previous
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, false, false, false);
  }
  void testCheckLoopMatch_MatchSoft_LoopPrev_CurSoftMatch()
  {
    // match all, current soft match, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchSoft_LoopPrev_CurNotMatch()
  {
    // match soft, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSoft_LoopCur_CurMatch()
  {
    // match soft, current match, return current
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, false, false);
  }
  void testCheckLoopMatch_MatchSoft_LoopCur_CurSoftMatch()
  {
    // match soft, current soft match, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchSoft_LoopCur_CurNotMatch()
  {
    // match soft, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSoft_LoopNext_CurMatch()
  {
    // match soft, current match, return next
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, true, false);
  }
  void testCheckLoopMatch_MatchSoft_LoopNext_CurSoftMatch()
  {
    // match soft, current soft match, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchSoft_LoopNext_CurNotMatch()
  {
    // match soft, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSoft_LoopCurNext_CurMatch()
  {
    // match soft, current match, return current and next
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, true, false);
  }
  void testCheckLoopMatch_MatchSoft_LoopCurNext_CurSoftMatch()
  {
    // match soft, current soft match, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchSoft_LoopCurNext_CurNotMatch()
  {
    // match soft, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSoft_LoopCurPrev_CurMatch()
  {
    // match soft, current match, return current, previous
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, true, false, false);
  }
  void testCheckLoopMatch_MatchSoft_LoopCurPrev_CurSoftMatch()
  {
    // match soft, current soft match, return nothing
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, false);
  }
  void testCheckLoopMatch_MatchSoft_LoopCurPrev_CurNotMatch()
  {
    // match soft, current not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SOFT_MATCH, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurMatch_PrevMatch()
  {
    // match second first, current match, previous match, return previous
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, false, false, false);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurMatch_PrevSoftMatch()
  {
    // match second first, current match, previous soft match, return previous
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, false, false, false);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurMatch_PrevNotMatch()
  {
    // match second first, current match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurSoftMatch_PrevMatch()
  {
    // match second first, current soft matc, previous match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurSoftMatch_PrevSoftMatch()
  {
    // match second first, current soft matc, previous soft match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH,
                        currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurSoftMatch_PrevNotMatch()
  {
    // match second first, current soft match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurNotMatch_PrevMatch()
  {
    // match second first, current not match, previous match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurNotMatch_PrevSoftMatch()
  {
    // match second first, current not match, previous soft match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopPrev_CurNotMatch_PrevNotMatch()
  {
    // match second first, current not match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_PREVIOUS);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCur_CurMatch_PrevMatch()
  {
    // match second first, current match, previous match, return current
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, false, false);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCur_CurMatch_PrevSoftMatch()
  {
    // match second first, current match, previous soft match, return current
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, false, false);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCur_CurMatch_PrevNotMatch()
  {
    // match second first, current match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCur_CurSoftMatch_PrevMatch()
  {
    // match second first, current soft match, previous match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCur_CurSoftMatch_PrevSoftMatch()
  {
    // match second first, current soft match, previous soft match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH,
                        currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCur_CurSoftMatch_PrevNotMatch()
  {
    // match second first, current soft match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCur_CurNotMatch_PrevMatch()
  {
    // match second first, current not match, previous match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCur_CurNotMatch_PrevSoftMatch()
  {
    // match second first, current not match, previous soft match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCur_CurNotMatch_PrevNotMatch()
  {
    // match second first, current not match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CURRENT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopNext_CurMatch_PrevMatch()
  {
    // match second first, current match, previous match, return next
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, true, false);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopNext_CurMatch_PrevSoftMatch()
  {
    // match second first, current match, previous soft match, return next
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, true, false);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopNext_CurMatch_PrevNotMatch()
  {
    // match second first, current match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopNext_CurSoftMatch_PrevMatch()
  {
    // match second first, current soft match, previous match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopNext_CurSoftMatch_PrevSoftMatch()
  {
    // match second first, current soft match, previous soft match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH,
                        currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopNext_CurSoftMatch_PrevNotMatch()
  {
    // match second first, current soft match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopNext_CurNotMatch_PrevMatch()
  {
    // match second first, current not match, previous match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopNext_CurNotMatch_PrevSoftMatch()
  {
    // match second first, current not match, previous soft match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopNext_CurNotMatch_PrevNotMatch()
  {
    // match second first, current not match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurMatch_PrevMatch()
  {
    // match second first, current match, previous match, return current and next
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, true, false);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurMatch_PrevSoftMatch()
  {
    // match second first, current match, previous match, return current and next
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, true, true, false);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurMatch_PrevNotMatch()
  {
    // match second first, current match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurSoftMatch_PrevMatch()
  {
    // match second first, current soft match, previous match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurSoftMatch_PrevSoftMatch()
  {
    // match second first, current soft match, previous soft match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH,
                        currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurSoftMatch_PrevNotMatch()
  {
    // match second first, current soft match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurNotMatch_PrevMatch()
  {
    // match second first, current not match, previous match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurNotMatch_PrevSoftMatch()
  {
    // match second first, current not match, previous soft match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurNext_CurNotMatch_PrevNotMatch()
  {
    // match second first, current not match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_NEXT);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurMatch_PrevMatch()
  {
    // match second first, current match, previous match, previous and current
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, true, false, false);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurMatch_PrevSoftMatch()
  {
    // match second first, current match, previous soft match, previous and current
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, true, true, false, false);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurMatch_PrevNotMatch()
  {
    // match second first, current match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurSoftMatch_PrevMatch()
  {
    // match second first, current soft match, previous match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurSoftMatch_PrevSoftMatch()
  {
    // match second first, current soft match, previous soft match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH,
                        currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurSoftMatch_PrevNotMatch()
  {
    // match second first, current soft match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_SOFT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurNotMatch_PrevMatch()
  {
    // match second first, current not match, previous match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurNotMatch_PrevSoftMatch()
  {
    // match second first, current not match, previous soft match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_SOFT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }
  void testCheckLoopMatch_MatchSecondFirst_LoopCurPrev_CurNotMatch_PrevNotMatch()
  {
    // match second first, current not match, previous not match, return continue looping
    RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH, currMatch = RuleConst::TSI_NOT_MATCH;
    RuleUtilTSI::TSIData* tsi =
        initTSIDataLoop(RuleConst::TSI_MATCH_SECOND_FIRST, RuleConst::TSI_LOOP_SET_CUR_PREV);
    assertFunCheckLoopMatch(*tsi, prevMatch, currMatch, false, false, false, false, true);
  }

private:
  RuleUtilTSI::TSIData*
  initTSIDataLoop(char loopMatch,
                  char loopToSet,
                  int16_t tsi = 1,
                  RuleConst::TSIScopeType scope = RuleConst::TSI_SCOPE_JOURNEY)
  {
    TSIInfo* tsiInfo = _memHandle.create<TSIInfo>();
    RuleConst::TSIScopeType* tsiScope = _memHandle.create<RuleConst::TSIScopeType>();
    VendorCode* vendor = _memHandle.create<VendorCode>();

    tsiInfo->loopMatch() = loopMatch;
    tsiInfo->loopToSet() = loopToSet;
    tsiInfo->tsi() = tsi;
    tsiInfo->scope() = scope;
    *tsiScope = scope;
    *vendor = "ATP";
    RuleUtilTSI::TSIData* tsiData =
        _memHandle.insert(new RuleUtilTSI::TSIData(*tsiInfo, *tsiScope, *vendor, 0, 0, 0, 0));
    return tsiData;
  }
  void assertFunCheckLoopMatch(RuleUtilTSI::TSIData& tsi,
                               RuleConst::TSIMatch& prevMatch,
                               RuleConst::TSIMatch& currMatch,
                               bool locSpecified,
                               bool addPrevTrav,
                               bool addCurrTrav,
                               bool addNextTrav,
                               bool continueLoop)
  {
    bool addPrevTravSeg = false;
    bool addCurrTravSeg = false;
    bool addNextTravSeg = false;
    bool continueLooping = false;
    RuleUtilTSI::checkLoopMatch(tsi,
                                prevMatch,
                                currMatch,
                                addPrevTravSeg,
                                addCurrTravSeg,
                                addNextTravSeg,
                                continueLooping,
                                locSpecified);
    CPPUNIT_ASSERT_EQUAL(addPrevTrav, addPrevTravSeg);
    CPPUNIT_ASSERT_EQUAL(addCurrTrav, addCurrTravSeg);
    CPPUNIT_ASSERT_EQUAL(addNextTrav, addNextTravSeg);
    CPPUNIT_ASSERT_EQUAL(continueLoop, continueLooping);
  }
  void assertFunCheckLoopMatch(RuleUtilTSI::TSIData& tsi,
                               RuleConst::TSIMatch& prevMatch,
                               RuleConst::TSIMatch& currMatch,
                               bool locSpecified,
                               bool addPrevTrav,
                               bool addCurrTrav,
                               bool addNextTrav)
  {
    bool addPrevTravSeg = false;
    bool addCurrTravSeg = false;
    bool addNextTravSeg = false;
    bool continueLooping = false;
    RuleUtilTSI::checkLoopMatch(tsi,
                                prevMatch,
                                currMatch,
                                addPrevTravSeg,
                                addCurrTravSeg,
                                addNextTravSeg,
                                continueLooping,
                                locSpecified);
    CPPUNIT_ASSERT_EQUAL(addPrevTrav, addPrevTravSeg);
    CPPUNIT_ASSERT_EQUAL(addCurrTrav, addCurrTravSeg);
    CPPUNIT_ASSERT_EQUAL(addNextTrav, addNextTravSeg);
    CPPUNIT_ASSERT_EQUAL(false, continueLooping);
    continueLooping = true;
    RuleUtilTSI::checkLoopMatch(tsi,
                                prevMatch,
                                currMatch,
                                addPrevTravSeg,
                                addCurrTravSeg,
                                addNextTravSeg,
                                continueLooping,
                                locSpecified);
    CPPUNIT_ASSERT_EQUAL(true, continueLooping);
  }
};

class RuleUtilTSITest_CheckMatchCriteria : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_CheckMatchCriteria);
  CPPUNIT_TEST(testCheckMatchCriteria_Empty);
  CPPUNIT_TEST(testCheckMatchCriteria_StopOver_LastTravelSeg);
  CPPUNIT_TEST(testCheckMatchCriteria_StopOver_IsStopOver);
  CPPUNIT_TEST(testCheckMatchCriteria_StopOver_IsTurnAroundPoint);
  CPPUNIT_TEST(testCheckMatchCriteria_StopOver_None);
  CPPUNIT_TEST(testCheckMatchCriteria_Inbound_Inbound);
  CPPUNIT_TEST(testCheckMatchCriteria_Inbound_Outbound);
  CPPUNIT_TEST(testCheckMatchCriteria_Outbound_Inbound);
  CPPUNIT_TEST(testCheckMatchCriteria_Outbound_Outbound);
  CPPUNIT_TEST(testCheckMatchCriteria_Furthest_Furthest);
  CPPUNIT_TEST(testCheckMatchCriteria_Furthest_NotFurthest);
  CPPUNIT_TEST(testCheckMatchCriteria_Domestic_Domestic);
  CPPUNIT_TEST(testCheckMatchCriteria_Domestic_Interantional);
  CPPUNIT_TEST(testCheckMatchCriteria_OneCountry_USAU);
  CPPUNIT_TEST(testCheckMatchCriteria_OneCountry_USUS);
  CPPUNIT_TEST(testCheckMatchCriteria_International_International);
  CPPUNIT_TEST(testCheckMatchCriteria_International_NotInternational);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_DepDstGtw_ArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_DepDstGtw_ArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_DepDstGtw_NotArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_NotDepDstGtw_ArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_NotDepDstGtw_ArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_NotDepDstGtw_NotArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_DepDstGtw_ArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_DepDstGtw_ArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_DepDstGtw_NotArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_DepDstGtw_NotArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_DepDstGtw_NotArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_NotDepDstGtw_NotArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_NotDepDstGtw_ArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_NotDepDstGtw_ArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_NotDepDstGtw_NotArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_NotDepDstGtw_NotArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_DepDstGtw_ArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_DepDstGtw_ArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_DepDstGtw_NotArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_NotDepDstGtw_ArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_NotDepDstGtw_ArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_NotDepDstGtw_NotArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_DepDstGtw_ArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_DepDstGtw_ArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_DepDstGtw_NotArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_DepDstGtw_NotArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_DepDstGtw_NotArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_NotDepDstGtw_NotArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_NotDepDstGtw_ArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_NotDepDstGtw_ArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_NotDepDstGtw_NotArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_NotDepDstGtw_NotArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Both_DepOrgGtw_DepDstGtw_ArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Both_DepOrgGtw_DepDstGtw_ArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Both_DepOrgGtw_DepDstGtw_NotArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Both_DepOrgGtw_NotDepDstGtw_ArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Both_DepOrgGtw_NotDepDstGtw_ArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Both_DepOrgGtw_NotDepDstGtw_NotArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_DepDstGtw_ArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_DepDstGtw_ArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_DepDstGtw_NotArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Both_DepOrgGtw_DepDstGtw_NotArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_DepDstGtw_NotArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Both_DepOrgGtw_NotDepDstGtw_NotArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_NotDepDstGtw_ArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_NotDepDstGtw_ArrOrgGtw_NotArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_NotDepDstGtw_NotArrOrgGtw_ArrDstGtw);
  CPPUNIT_TEST(
      testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_NotDepDstGtw_NotArrOrgGtw_NotArrDstGtw);
  ;
  CPPUNIT_TEST(testCheckMatchCriteria_OrigGateway_Orig_NotDepOrgGtw_NotArrOrgGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_OrigGateway_Orig_NotDepOrgGtw_ArrOrgGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_OrigGateway_Orig_DepOrgGtw_NotArrOrgGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_OrigGateway_Orig_DepOrgGtw_ArrOrgGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_OrigGateway_Dest_NotDepOrgGtw_NotArrOrgGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_OrigGateway_Dest_NotDepOrgGtw_ArrOrgGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_OrigGateway_Dest_DepOrgGtw_NotArrOrgGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_OrigGateway_Dest_DepOrgGtw_ArrOrgGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_OrigGateway_Both_NotDepOrgGtw_NotArrOrgGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_OrigGateway_Both_NotDepOrgGtw_ArrOrgGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_OrigGateway_Both_DepOrgGtw_NotArrOrgGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_OrigGateway_Both_DepOrgGtw_ArrOrgGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_DestGateway_Orig_NotDepDstGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_DestGateway_Orig_NotDepDstGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_DestGateway_Orig_DepDstGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_DestGateway_Orig_DepDstGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_DestGateway_Dest_NotDepDstGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_DestGateway_Dest_NotDepDstGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_DestGateway_Dest_DepDstGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_DestGateway_Dest_DepDstGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_DestGateway_Both_NotDepDstGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_DestGateway_Both_NotDepDstGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_DestGateway_Both_DepDstGtw_NotArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_DestGateway_Both_DepDstGtw_ArrDstGtw);
  CPPUNIT_TEST(testCheckMatchCriteria_TransAtlantic_I1I2_AT);
  CPPUNIT_TEST(testCheckMatchCriteria_TransAtlantic_I1I3_AT);
  CPPUNIT_TEST(testCheckMatchCriteria_TransAtlantic_I1I2_SN);
  CPPUNIT_TEST(testCheckMatchCriteria_TransAtlantic_I1I3_SN);
  CPPUNIT_TEST(testCheckMatchCriteria_TransAtlantic_I1I2_SA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransAtlantic_I1I3_SA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransAtlantic_I1I2_AP);
  CPPUNIT_TEST(testCheckMatchCriteria_TransAtlantic_I1I3_AP);
  CPPUNIT_TEST(testCheckMatchCriteria_TransAtlantic_I1I1_AT);
  CPPUNIT_TEST(testCheckMatchCriteria_TransAtlantic_I2I2_AT);
  CPPUNIT_TEST(testCheckMatchCriteria_TransAtlantic_I2I3_AT);
  CPPUNIT_TEST(testCheckMatchCriteria_TransAtlantic_I3I3_AT);
  CPPUNIT_TEST(testCheckMatchCriteria_TransAtlantic_I1I3_PA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransPacific_I1I3_NP);
  CPPUNIT_TEST(testCheckMatchCriteria_TransPacific_I1I3_PA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransPacific_I1I3_PN);
  CPPUNIT_TEST(testCheckMatchCriteria_TransPacific_I1I3_AP);
  CPPUNIT_TEST(testCheckMatchCriteria_TransPacific_I1I1_PA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransPacific_I1I2_PA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransPacific_I2I2_PA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransPacific_I2I3_PA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransPacific_I3I3_PA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I1I2_AT);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I1I3_AT);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I1I2_SN);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I1I3_SN);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I1I2_SA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I1I3_SA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I1I2_AP);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I1I3_AP);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I1I1_AT);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I2I2_AT);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I2I3_AT);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I3I3_AT);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I1I3_PA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I1I3_NP);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I1I3_PN);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I1I1_PA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I1I2_PA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I2I2_PA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I2I3_PA);
  CPPUNIT_TEST(testCheckMatchCriteria_TransOceanic_I3I3_PA);
  CPPUNIT_TEST(testCheckMatchCriteria_OverWater_IsOverWater);
  CPPUNIT_TEST(testCheckMatchCriteria_OverWater_NotOverWater);
  CPPUNIT_TEST(testCheckMatchCriteria_IntlDomTransfer_IsIntlDomTransfer);
  CPPUNIT_TEST(testCheckMatchCriteria_IntlDomTransfer_NotIntlDomTransfer);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCheckMatchCriteria_Empty()
  {
    // no match criteria - pass
    matchCriteria();
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_StopOver_LastTravelSeg()
  {
    // stopover, fail on teh last TravelSeg
    matchCriteria(TSIInfo::STOP_OVER);
    _tsm.isLastTravelSeg() = true;
    assertMatchCriteria(false, RuleConst::NO_MATCH_LAST_SEGMENT);
  }
  void testCheckMatchCriteria_StopOver_IsStopOver()
  {
    // stopover, match on stopover
    matchCriteria(TSIInfo::STOP_OVER);
    _tsm.isStopover() = true;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_StopOver_IsTurnAroundPoint()
  {
    // stopover, match on turnaround point
    matchCriteria(TSIInfo::STOP_OVER);
    _tsm.destIsTurnAroundPoint() = true;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_StopOver_None()
  {
    // stopover, not stopover or turnaround - fail
    matchCriteria(TSIInfo::STOP_OVER);
    assertMatchCriteria(false, RuleConst::MATCH_STOP_OVER_DESC);
  }
  void testCheckMatchCriteria_Inbound_Inbound()
  {
    // inbound, match on inbound fare
    matchCriteria(TSIInfo::INBOUND);
    _tsm.direction() = RuleConst::INBOUND;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_Inbound_Outbound()
  {
    // inbound, fail on onbound fare
    matchCriteria(TSIInfo::INBOUND);
    _tsm.direction() = RuleConst::OUTBOUND;
    assertMatchCriteria(false, RuleConst::MATCH_INBOUND_DESC);
  }
  void testCheckMatchCriteria_Outbound_Inbound()
  {
    // outbound,, fail on inboud fare
    matchCriteria(TSIInfo::OUTBOUND);
    _tsm.direction() = RuleConst::INBOUND;
    assertMatchCriteria(false, RuleConst::MATCH_OUTBOUND_DESC);
  }
  void testCheckMatchCriteria_Outbound_Outbound()
  {
    // outbound, match on outbound fare
    matchCriteria(TSIInfo::OUTBOUND);
    _tsm.direction() = RuleConst::OUTBOUND;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_Furthest_Furthest()
  {
    // furthest, match on farthest
    matchCriteria(TSIInfo::FURTHEST);
    _tsm.isFurthest() = true;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_Furthest_NotFurthest()
  {
    // furthest, fail on farthest
    matchCriteria(TSIInfo::FURTHEST);
    _tsm.isFurthest() = false;
    assertMatchCriteria(false, RuleConst::MATCH_FURTHEST_DESC);
  }
  void testCheckMatchCriteria_Domestic_Domestic()
  {
    // domestic, pass on domestic
    matchCriteria(TSIInfo::DOMESTIC);
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_Domestic_Interantional()
  {
    // domestic, fail on Interantional
    matchCriteria(TSIInfo::DOMESTIC);
    assertMatchCriteria(false, RuleConst::MATCH_DOMESTIC_DESC);
  }
  void testCheckMatchCriteria_OneCountry_USAU()
  {
    // one country, fail on US-AU
    matchCriteria(TSIInfo::ONE_COUNTRY);
    assertMatchCriteria(false, RuleConst::MATCH_ONE_COUNTRY_DESC);
  }
  void testCheckMatchCriteria_OneCountry_USUS()
  {
    // one country, pass on US-US
    matchCriteria(TSIInfo::ONE_COUNTRY);
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_International_International()
  {
    // international, match on international
    matchCriteria(TSIInfo::INTERNATIONAL);
    _tsm.isInternational() = true;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_International_NotInternational()
  {
    // international, fail on not international
    matchCriteria(TSIInfo::INTERNATIONAL);
    _tsm.isInternational() = false;
    assertMatchCriteria(false, RuleConst::MATCH_INTERNATIONAL_DESC);
  }
  void testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_DepDstGtw_ArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(true, true, true, true, true);
  }
  void testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_DepDstGtw_ArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(true, true, true, false, true);
  }
  void testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_DepDstGtw_NotArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(true, true, false, true, true);
  }
  void testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_NotDepDstGtw_ArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(true, false, true, true, true);
  }
  void testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_NotDepDstGtw_ArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(true, false, true, false, true);
  }
  void testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_NotDepDstGtw_NotArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(true, false, false, true, true);
  }
  void testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_DepDstGtw_ArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(false, true, true, true, true);
  }
  void testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_DepDstGtw_ArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(false, true, true, false, true);
  }
  void testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_DepDstGtw_NotArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(false, true, false, true, true);
  }
  void testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_DepDstGtw_NotArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match on departue
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(true, true, false, false, true);
  }
  void testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_DepDstGtw_NotArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match on departue
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(false, true, false, false, true);
  }
  void testCheckMatchCriteria_Gateway_Orig_DepOrgGtw_NotDepDstGtw_NotArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match on departue
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(true, false, false, false, true);
  }
  void testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_NotDepDstGtw_ArrOrgGtw_ArrDstGtw()
  {
    // gateway - fail on deparure
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(false, false, true, true, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_NotDepDstGtw_ArrOrgGtw_NotArrDstGtw()
  {
    // gateway - fail on deparure
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(false, false, true, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_NotDepDstGtw_NotArrOrgGtw_ArrDstGtw()
  {
    // gateway - fail on deparure
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(false, false, false, true, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_Gateway_Orig_NotDepOrgGtw_NotDepDstGtw_NotArrOrgGtw_NotArrDstGtw()
  {
    // gateway - fail on deparure
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(false, false, false, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_DepDstGtw_ArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(true, true, true, true, true);
  }
  void testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_DepDstGtw_ArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(true, true, true, false, true);
  }
  void testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_DepDstGtw_NotArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(true, true, false, true, true);
  }
  void testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_NotDepDstGtw_ArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(true, false, true, true, true);
  }
  void testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_NotDepDstGtw_ArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(true, false, true, false, true);
  }
  void testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_NotDepDstGtw_NotArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(true, false, false, true, true);
  }
  void testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_DepDstGtw_ArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(false, true, true, true, true);
  }
  void testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_DepDstGtw_ArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(false, true, true, false, true);
  }
  void testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_DepDstGtw_NotArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(false, true, false, true, true);
  }

  void testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_DepDstGtw_NotArrOrgGtw_NotArrDstGtw()
  {
    // gateway - fail on arrive
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(true, true, false, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_DepDstGtw_NotArrOrgGtw_NotArrDstGtw()
  {
    // gateway - fail on arrive
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(false, true, false, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_Gateway_Dest_DepOrgGtw_NotDepDstGtw_NotArrOrgGtw_NotArrDstGtw()
  {
    // gateway - fail on arrive
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(true, false, false, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_NotDepDstGtw_ArrOrgGtw_ArrDstGtw()
  {
    // gateway - pass  on arriva
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(false, false, true, true, true);
  }
  void testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_NotDepDstGtw_ArrOrgGtw_NotArrDstGtw()
  {
    // gateway - pass  on arriva
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(false, false, true, false, true);
  }
  void testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_NotDepDstGtw_NotArrOrgGtw_ArrDstGtw()
  {
    // gateway - pass  on arriva
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(false, false, false, true, true);
  }
  void testCheckMatchCriteria_Gateway_Dest_NotDepOrgGtw_NotDepDstGtw_NotArrOrgGtw_NotArrDstGtw()
  {
    // gateway - fail on arrive
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(false, false, false, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_Gateway_Both_DepOrgGtw_DepDstGtw_ArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(true, true, true, true, true);
  }
  void testCheckMatchCriteria_Gateway_Both_DepOrgGtw_DepDstGtw_ArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(true, true, true, false, true);
  }
  void testCheckMatchCriteria_Gateway_Both_DepOrgGtw_DepDstGtw_NotArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(true, true, false, true, true);
  }
  void testCheckMatchCriteria_Gateway_Both_DepOrgGtw_NotDepDstGtw_ArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(true, false, true, true, true);
  }
  void testCheckMatchCriteria_Gateway_Both_DepOrgGtw_NotDepDstGtw_ArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(true, false, true, false, true);
  }
  void testCheckMatchCriteria_Gateway_Both_DepOrgGtw_NotDepDstGtw_NotArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(true, false, false, true, true);
  }
  void testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_DepDstGtw_ArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(false, true, true, true, true);
  }
  void testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_DepDstGtw_ArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(false, true, true, false, true);
  }
  void testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_DepDstGtw_NotArrOrgGtw_ArrDstGtw()
  {
    // gateway - match
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(false, true, false, true, true);
  }
  void testCheckMatchCriteria_Gateway_Both_DepOrgGtw_DepDstGtw_NotArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match on departue
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(true, true, false, false, true);
  }
  void testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_DepDstGtw_NotArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match on departue
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(false, true, false, false, true);
  }
  void testCheckMatchCriteria_Gateway_Both_DepOrgGtw_NotDepDstGtw_NotArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match on departue
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(true, false, false, false, true);
  }
  void testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_NotDepDstGtw_ArrOrgGtw_ArrDstGtw()
  {
    // gateway - match on arrive
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(false, false, true, true, true);
  }
  void testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_NotDepDstGtw_ArrOrgGtw_NotArrDstGtw()
  {
    // gateway - match on arrive
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(false, false, true, false, true);
  }
  void testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_NotDepDstGtw_NotArrOrgGtw_ArrDstGtw()
  {
    // gateway - match on arrive
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(false, false, false, true, true);
  }
  void testCheckMatchCriteria_Gateway_Both_NotDepOrgGtw_NotDepDstGtw_NotArrOrgGtw_NotArrDstGtw()
  {
    // gateway - fail on both
    matchCriteria(TSIInfo::GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(false, false, false, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  // TODO: should return RuleConst::MATCH_ORIG_GATEWAY_DESC when faile?????????????
  void testCheckMatchCriteria_OrigGateway_Orig_NotDepOrgGtw_NotArrOrgGtw()
  {
    // origin gateway - no match
    matchCriteria(TSIInfo::ORIG_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(false, false, false, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_OrigGateway_Orig_NotDepOrgGtw_ArrOrgGtw()
  {
    // origin gateway - no match
    matchCriteria(TSIInfo::ORIG_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(false, false, true, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_OrigGateway_Orig_DepOrgGtw_NotArrOrgGtw()
  {
    // origin gateway - match on departure gateway
    matchCriteria(TSIInfo::ORIG_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(true, false, false, false, true);
  }
  void testCheckMatchCriteria_OrigGateway_Orig_DepOrgGtw_ArrOrgGtw()
  {
    // origin gateway - match on departure gateway
    matchCriteria(TSIInfo::ORIG_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(true, false, true, false, true);
  }
  void testCheckMatchCriteria_OrigGateway_Dest_NotDepOrgGtw_NotArrOrgGtw()
  {
    // origin gateway - no match
    matchCriteria(TSIInfo::ORIG_GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(false, false, false, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_OrigGateway_Dest_NotDepOrgGtw_ArrOrgGtw()
  {
    // origin gateway - match on arrive gateway
    matchCriteria(TSIInfo::ORIG_GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(false, false, true, false, true);
  }
  void testCheckMatchCriteria_OrigGateway_Dest_DepOrgGtw_NotArrOrgGtw()
  {
    // origin gateway - no match
    matchCriteria(TSIInfo::ORIG_GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(true, false, false, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_OrigGateway_Dest_DepOrgGtw_ArrOrgGtw()
  {
    // origin gateway - match on arrive gateway
    matchCriteria(TSIInfo::ORIG_GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(true, false, true, false, true);
  }
  void testCheckMatchCriteria_OrigGateway_Both_NotDepOrgGtw_NotArrOrgGtw()
  {
    // origin gateway - no match
    matchCriteria(TSIInfo::ORIG_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(false, false, false, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_OrigGateway_Both_NotDepOrgGtw_ArrOrgGtw()
  {
    // origin gateway - match on departure gateway
    matchCriteria(TSIInfo::ORIG_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(false, false, true, false, true);
  }
  void testCheckMatchCriteria_OrigGateway_Both_DepOrgGtw_NotArrOrgGtw()
  {
    // origin gateway - match on arrive gateway
    matchCriteria(TSIInfo::ORIG_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(true, false, false, false, true);
  }
  void testCheckMatchCriteria_OrigGateway_Both_DepOrgGtw_ArrOrgGtw()
  {
    // origin gateway - match on both
    matchCriteria(TSIInfo::ORIG_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(true, false, true, false, true);
  }
  // TODO: should return RuleConst::MATCH_DEST_GATEWAY_DESC when faile?????????????
  void testCheckMatchCriteria_DestGateway_Orig_NotDepDstGtw_NotArrDstGtw()
  {
    // destination gateway - no match
    matchCriteria(TSIInfo::DEST_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(false, false, false, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_DestGateway_Orig_NotDepDstGtw_ArrDstGtw()
  {
    // destination gateway - no match
    matchCriteria(TSIInfo::DEST_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(false, false, false, true, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_DestGateway_Orig_DepDstGtw_NotArrDstGtw()
  {
    // destination gateway - match on departure gateway
    matchCriteria(TSIInfo::DEST_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(false, true, false, false, true);
  }
  void testCheckMatchCriteria_DestGateway_Orig_DepDstGtw_ArrDstGtw()
  {
    // destination gateway - match on departure gateway
    matchCriteria(TSIInfo::DEST_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG);
    assertMatchCriteriaGateways(false, true, false, true, true);
  }
  void testCheckMatchCriteria_DestGateway_Dest_NotDepDstGtw_NotArrDstGtw()
  {
    // destination gateway - no match
    matchCriteria(TSIInfo::DEST_GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(false, false, false, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_DestGateway_Dest_NotDepDstGtw_ArrDstGtw()
  {
    // destination gateway - match on arrive gateway
    matchCriteria(TSIInfo::DEST_GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(false, false, false, true, true);
  }
  void testCheckMatchCriteria_DestGateway_Dest_DepDstGtw_NotArrDstGtw()
  {
    // destination gateway - no match
    matchCriteria(TSIInfo::DEST_GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(false, true, false, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_DestGateway_Dest_DepDstGtw_ArrDstGtw()
  {
    // destination gateway - match on arrive gateway
    matchCriteria(TSIInfo::DEST_GATEWAY, RuleConst::TSI_APP_CHECK_DEST);
    assertMatchCriteriaGateways(true, true, true, true, true);
  }
  void testCheckMatchCriteria_DestGateway_Both_NotDepDstGtw_NotArrDstGtw()
  {
    // destination gateway - no match
    matchCriteria(TSIInfo::DEST_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(false, false, false, false, false, RuleConst::MATCH_GATEWAY_DESC);
  }
  void testCheckMatchCriteria_DestGateway_Both_NotDepDstGtw_ArrDstGtw()
  {
    // destination gateway - match on arrive gateway
    matchCriteria(TSIInfo::DEST_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(false, false, false, true, true);
  }
  void testCheckMatchCriteria_DestGateway_Both_DepDstGtw_NotArrDstGtw()
  {
    // destination gateway - match on arrive gateway
    matchCriteria(TSIInfo::DEST_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(false, true, false, false, true);
  }
  void testCheckMatchCriteria_DestGateway_Both_DepDstGtw_ArrDstGtw()
  {
    // destination gateway - macth on both
    matchCriteria(TSIInfo::DEST_GATEWAY, RuleConst::TSI_APP_CHECK_ORIG_DEST);
    assertMatchCriteriaGateways(false, true, false, true, true);
  }
  void testCheckMatchCriteria_TransAtlantic_I1I2_AT()
  {
    // transatlantic - IATA 1->2 via atlantic
    matchCriteria(TSIInfo::TRANS_ATLANTIC);
    _tsm.globalDirection() = GlobalDirection::AT;
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransAtlantic_I1I3_AT()
  {
    // transatlantic - IATA 1->3 via atlantic
    matchCriteria(TSIInfo::TRANS_ATLANTIC);
    _tsm.globalDirection() = GlobalDirection::AT;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransAtlantic_I1I2_SN()
  {
    // transatlantic - IATA 1->2 via atlantic
    matchCriteria(TSIInfo::TRANS_ATLANTIC);
    _tsm.globalDirection() = GlobalDirection::SN;
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransAtlantic_I1I3_SN()
  {
    // transatlantic - IATA 1->3 via atlantic
    matchCriteria(TSIInfo::TRANS_ATLANTIC);
    _tsm.globalDirection() = GlobalDirection::SN;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransAtlantic_I1I2_SA()
  {
    // transatlantic - IATA 1->2 via atlantic
    matchCriteria(TSIInfo::TRANS_ATLANTIC);
    _tsm.globalDirection() = GlobalDirection::SA;
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransAtlantic_I1I3_SA()
  {
    // transatlantic - IATA 1->3 via atlantic
    matchCriteria(TSIInfo::TRANS_ATLANTIC);
    _tsm.globalDirection() = GlobalDirection::SA;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransAtlantic_I1I2_AP()
  {
    // transatlantic - IATA 1->2 via atlantic
    matchCriteria(TSIInfo::TRANS_ATLANTIC);
    _tsm.globalDirection() = GlobalDirection::AP;
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransAtlantic_I1I3_AP()
  {
    // transatlantic - IATA 1->3 via atlantic
    matchCriteria(TSIInfo::TRANS_ATLANTIC);
    _tsm.globalDirection() = GlobalDirection::AT;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransAtlantic_I1I1_AT()
  {
    // transatlantic - IATA 1->1 via atlantic
    matchCriteria(TSIInfo::TRANS_ATLANTIC);
    _tsm.globalDirection() = GlobalDirection::AT;
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_ATLANTIC_DESC);
  }
  void testCheckMatchCriteria_TransAtlantic_I2I2_AT()
  {
    // transatlantic - IATA 2->2 via atlantic
    matchCriteria(TSIInfo::TRANS_ATLANTIC);
    _tsm.globalDirection() = GlobalDirection::AT;
    _tsm.travelSeg()->origin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_ATLANTIC_DESC);
  }
  void testCheckMatchCriteria_TransAtlantic_I2I3_AT()
  {
    // transatlantic - IATA 2->3 via atlantic
    matchCriteria(TSIInfo::TRANS_ATLANTIC);
    _tsm.globalDirection() = GlobalDirection::AT;
    _tsm.travelSeg()->origin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_ATLANTIC_DESC);
  }
  void testCheckMatchCriteria_TransAtlantic_I3I3_AT()
  {
    // transatlantic - IATA 3->3 via atlantic
    matchCriteria(TSIInfo::TRANS_ATLANTIC);
    _tsm.globalDirection() = GlobalDirection::AP;
    _tsm.travelSeg()->origin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_ATLANTIC_DESC);
  }
  void testCheckMatchCriteria_TransAtlantic_I1I3_PA()
  {
    // transatlantic - IATA 1->3 via pacific
    matchCriteria(TSIInfo::TRANS_ATLANTIC);
    _tsm.globalDirection() = GlobalDirection::PA;
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_ATLANTIC_DESC);
  }
  void testCheckMatchCriteria_TransPacific_I1I3_NP()
  {
    // Pacific - IATA 1->3 via pacific
    matchCriteria(TSIInfo::TRANS_PACIFIC);
    _tsm.globalDirection() = GlobalDirection::NP;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransPacific_I1I3_PA()
  {
    // Pacific - IATA 1->3 via pacific
    matchCriteria(TSIInfo::TRANS_PACIFIC);
    _tsm.globalDirection() = GlobalDirection::PA;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransPacific_I1I3_PN()
  {
    // Pacific - IATA 1->3 via pacific
    matchCriteria(TSIInfo::TRANS_PACIFIC);
    _tsm.globalDirection() = GlobalDirection::PN;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransPacific_I1I3_AP()
  {
    // Pacific - IATA 1->3 via pacific
    matchCriteria(TSIInfo::TRANS_PACIFIC);
    _tsm.globalDirection() = GlobalDirection::AP;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransPacific_I1I1_PA()
  {
    // Pacific - IATA 1->1 via pacific
    matchCriteria(TSIInfo::TRANS_PACIFIC);
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _tsm.globalDirection() = GlobalDirection::PA;
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_PACIFIC_DESC);
  }
  void testCheckMatchCriteria_TransPacific_I1I2_PA()
  {
    // Pacific - IATA 1->1 via pacific
    matchCriteria(TSIInfo::TRANS_PACIFIC);
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    _tsm.globalDirection() = GlobalDirection::PA;
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_PACIFIC_DESC);
  }
  void testCheckMatchCriteria_TransPacific_I2I2_PA()
  {
    // Pacific - IATA 2->2 via pacific
    matchCriteria(TSIInfo::TRANS_PACIFIC);
    _tsm.travelSeg()->origin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    _tsm.globalDirection() = GlobalDirection::PA;
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_PACIFIC_DESC);
  }
  void testCheckMatchCriteria_TransPacific_I2I3_PA()
  {
    // Pacific - IATA 2->3 via pacific
    matchCriteria(TSIInfo::TRANS_PACIFIC);
    _tsm.travelSeg()->origin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    _tsm.globalDirection() = GlobalDirection::PA;
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_PACIFIC_DESC);
  }
  void testCheckMatchCriteria_TransPacific_I3I3_PA()
  {
    // Pacific - IATA 3->3 via pacific
    matchCriteria(TSIInfo::TRANS_PACIFIC);
    _tsm.travelSeg()->origin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");
    _tsm.globalDirection() = GlobalDirection::PA;
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_PACIFIC_DESC);
  }
  void testCheckMatchCriteria_TransOceanic_I1I2_AT()
  {
    // transoceanic - IATA 1->2 via atlantic
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::AT;
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransOceanic_I1I3_AT()
  {
    // transoceanic - IATA 1->3 via atlantic
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::AT;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransOceanic_I1I2_SN()
  {
    // transoceanic - IATA 1->2 via atlantic
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::SN;
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransOceanic_I1I3_SN()
  {
    // transoceanic - IATA 1->3 via atlantic
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::SN;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransOceanic_I1I2_SA()
  {
    // transoceanic - IATA 1->2 via atlantic
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::SA;
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransOceanic_I1I3_SA()
  {
    // transoceanic - IATA 1->3 via atlantic
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::SA;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransOceanic_I1I2_AP()
  {
    // transoceanic - IATA 1->2 via atlantic
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::AP;
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransOceanic_I1I3_AP()
  {
    // transoceanic - IATA 1->3 via atlantic
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::AT;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransOceanic_I1I1_AT()
  {
    // transoceanic - IATA 1->1 via atlantic
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::AT;
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_OCEANIC_DESC);
  }
  void testCheckMatchCriteria_TransOceanic_I2I2_AT()
  {
    // transoceanic - IATA 2->2 via atlantic
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::AT;
    _tsm.travelSeg()->origin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_OCEANIC_DESC);
  }
  void testCheckMatchCriteria_TransOceanic_I2I3_AT()
  {
    // transoceanic - IATA 2->3 via atlantic
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::AT;
    _tsm.travelSeg()->origin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_OCEANIC_DESC);
  }
  void testCheckMatchCriteria_TransOceanic_I3I3_AT()
  {
    // transoceanic - IATA 3->3 via atlantic
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::AP;
    _tsm.travelSeg()->origin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_OCEANIC_DESC);
  }
  void testCheckMatchCriteria_TransOceanic_I1I3_PA()
  {
    // transoceanic - IATA 1->3 via pacific
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::PA;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransOceanic_I1I3_NP()
  {
    // transoceanic - IATA 1->3 via pacific
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::NP;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransOceanic_I1I3_PN()
  {
    // transoceanic - IATA 1->3 via pacific
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.globalDirection() = GlobalDirection::PN;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_TransOceanic_I1I1_PA()
  {
    // transoceanic - IATA 1->1 via pacific
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _tsm.globalDirection() = GlobalDirection::PA;
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_OCEANIC_DESC);
  }
  void testCheckMatchCriteria_TransOceanic_I1I2_PA()
  {
    // transoceanic - IATA 1->1 via pacific
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    _tsm.globalDirection() = GlobalDirection::PA;
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_OCEANIC_DESC);
  }
  void testCheckMatchCriteria_TransOceanic_I2I2_PA()
  {
    // transoceanic - IATA 2->2 via pacific
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.travelSeg()->origin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    _tsm.travelSeg()->destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    _tsm.globalDirection() = GlobalDirection::PA;
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_OCEANIC_DESC);
  }
  void testCheckMatchCriteria_TransOceanic_I2I3_PA()
  {
    // transoceanic - IATA 2->3 via pacific
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.travelSeg()->origin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    _tsm.globalDirection() = GlobalDirection::PA;
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_OCEANIC_DESC);
  }
  void testCheckMatchCriteria_TransOceanic_I3I3_PA()
  {
    // transoceanic - IATA 3->3 via pacific
    matchCriteria(TSIInfo::TRANS_OCEANIC);
    _tsm.travelSeg()->origin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");
    _tsm.globalDirection() = GlobalDirection::PA;
    assertMatchCriteria(false, RuleConst::MATCH_TRANS_OCEANIC_DESC);
  }
  void testCheckMatchCriteria_OverWater_IsOverWater()
  {
    // is over water - match
    matchCriteria(TSIInfo::OVER_WATER);
    _tsm.isOverWater() = true;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_OverWater_NotOverWater()
  {
    // is over water - not match
    matchCriteria(TSIInfo::OVER_WATER);
    _tsm.isOverWater() = false;
    assertMatchCriteria(false, RuleConst::MATCH_OVER_WATER_DESC);
  }
  void testCheckMatchCriteria_IntlDomTransfer_IsIntlDomTransfer()
  {
    // isIntlDomTransfer - match
    matchCriteria(TSIInfo::INTL_DOM_TRANSFER);
    _tsm.isIntlDomTransfer() = true;
    assertMatchCriteria(true);
  }
  void testCheckMatchCriteria_IntlDomTransfer_NotIntlDomTransfer()
  {
    // isIntlDomTransfer - match
    matchCriteria(TSIInfo::INTL_DOM_TRANSFER);
    _tsm.isIntlDomTransfer() = false;
    assertMatchCriteria(false, RuleConst::MATCH_INTL_DOM_TRANSFER_DESC);
  }

  void setUp()
  {
    RuleUtilTSITest_Base::setUp();
    _vendor = "ATP";
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsi = _memHandle.insert(new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, 0, 0, 0));
  }

private:
  void matchCriteria() { _tsm.travelSeg() = _segSFOSYD; }
  void matchCriteria(TSIInfo::TSIMatchCriteria mc,
                     RuleConst::TSIApplicationType type = RuleConst::TSI_APP_CHECK_ORIG_DEST)
  {
    _tsm.travelSeg() = _segSFOSYD;
    _mc.push_back(mc);
    _tsiInfo.type() = type;
  }
  void assertMatchCriteria(bool result, std::string noMatchReason = "")
  {
    CPPUNIT_ASSERT_EQUAL(result,
                         RuleUtilTSI::checkMatchCriteria(*_tsi, _tsm, _mc, _noMatchReason, *_trx));
    if (!result)
      CPPUNIT_ASSERT_EQUAL(noMatchReason, _noMatchReason);
  }
  void assertMatchCriteriaGateways(bool departsOrigGateway,
                                   bool departsDestGateway,
                                   bool arrivesOrigGateway,
                                   bool arrivesDestGateway,
                                   bool result,
                                   std::string noMatchReason = "")
  {
    _tsm.departsOrigGateway() = departsOrigGateway;
    _tsm.departsDestGateway() = departsDestGateway;
    _tsm.arrivesOrigGateway() = arrivesOrigGateway;
    _tsm.arrivesDestGateway() = arrivesDestGateway;
    assertMatchCriteria(result, noMatchReason);
  }
  RuleUtilTSI::TSIData* _tsi;
  RuleUtilTSI::TSITravelSegMarkup _tsm;
  std::vector<TSIInfo::TSIMatchCriteria> _mc;
  std::string _noMatchReason;
  TSIInfo _tsiInfo;
  VendorCode _vendor;
  RuleConst::TSIScopeType _tsiScope;
};

class RuleUtilTSITest_CheckTSIMatchTravelSeg : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_CheckTSIMatchTravelSeg);
  CPPUNIT_TEST(testCheckTSIMatchTravelSeg_FailGeo_FailMatch);
  CPPUNIT_TEST(testCheckTSIMatchTravelSeg_FailGeo_PassMatch);
  CPPUNIT_TEST(testCheckTSIMatchTravelSeg_PassGeo_FailMatch);
  CPPUNIT_TEST(testCheckTSIMatchTravelSeg_PassGeo_PassMatch);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCheckTSIMatchTravelSeg_FailGeo_FailMatch()
  {
    matchCriteria(TSIInfo::OVER_WATER); // fail on over water
    _tsi = initTSIData('N', "PL", 'N', "GB"); // fail on location
    assertCheckTSIMatchTravelSeg(RuleConst::TSI_NOT_MATCH,
                                 std::string(RuleConst::MATCH_OVER_WATER_DESC) +
                                     std::string(" LOCALE"));
  }
  void testCheckTSIMatchTravelSeg_FailGeo_PassMatch()
  {
    matchCriteria(TSIInfo::OVER_WATER); // fail on over water
    _tsi = initTSIData(); // no location - pass
    assertCheckTSIMatchTravelSeg(RuleConst::TSI_NOT_MATCH, RuleConst::MATCH_OVER_WATER_DESC);
  }
  void testCheckTSIMatchTravelSeg_PassGeo_FailMatch()
  {
    // no match criteria - pass
    _tsi = initTSIData('N', "PL", 'N', "GB"); // fail on location
    assertCheckTSIMatchTravelSeg(RuleConst::TSI_SOFT_MATCH, "LOCALE ");
  }
  void testCheckTSIMatchTravelSeg_PassGeo_PassMatch()
  {
    // no match criteria - pass
    _tsi = initTSIData(); // no location - pass
    assertCheckTSIMatchTravelSeg(RuleConst::TSI_MATCH, "");
  }

  void setUp()
  {
    RuleUtilTSITest_Base::setUp();
    _tsm.travelSeg() = _segSFOSYD;
    _vendor = "ATP";
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
  }

private:
  void assertCheckTSIMatchTravelSeg(RuleConst::TSIMatch ret, std::string reason)
  {
    bool origMatch, destMatch;
    CPPUNIT_ASSERT_EQUAL(ret,
                         RuleUtilTSI::checkTSIMatchTravelSeg(
                             *_trx, *_tsi, _tsm, _mc, origMatch, destMatch, _noMatchReason));
    CPPUNIT_ASSERT_EQUAL(reason, _noMatchReason);
  }

  void matchCriteria(TSIInfo::TSIMatchCriteria mc) { _mc.push_back(mc); }

  RuleUtilTSI::TSIData* _tsi;
  RuleUtilTSI::TSITravelSegMarkup _tsm;
  std::vector<TSIInfo::TSIMatchCriteria> _mc;
  std::string _noMatchReason;
  TSIInfo _tsiInfo;
  VendorCode _vendor;
  RuleConst::TSIScopeType _tsiScope;
};

class RuleUtilTSITest_GetGeoLocaleFromItin : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_GetGeoLocaleFromItin);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_Journey_GeoOrig);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_Journey_GeoDest);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_Journey_GeoTurnAround_seg1);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_Journey_GeoTurnAround_seg2);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_Journey_GeoTurnAround_seg5_noPu);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_Journey_GeoTurnAround_seg5_Pu_noTurnAround);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_Journey_GeoTurnAround_seg5_Pu_TurnAround);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_Journey_Blank);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_SubJourney_GeoOrig);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_SubJourney_GeoDest);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_SegJourney_GeoTurnAround_NoTurnAround);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_SegJourney_GeoTurnAround_TurnAround);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_SubJourney_Blank);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_FareComp_GeoOrig);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_FareComp_GeoDest);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_FareComp_GeoTurnAround_seg1);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_FareComp_GeoTurnAround_seg2);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_FareComp_GeoTurnAround_seg5_noPu);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_FareComp_GeoTurnAround_seg5_Pu_noTurnAround);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_FareComp_GeoTurnAround_seg5_Pu_TurnAround);
  CPPUNIT_TEST(testGetGeoLocaleFromItin_FareComp_Blank);
  CPPUNIT_TEST_SUITE_END();

public:
  void testGetGeoLocaleFromItin_Journey_GeoOrig()
  {
    // origin from first itin travel segment
    geoLocaleFromItinData(RuleConst::TSI_SCOPE_JOURNEY, RuleConst::TSI_GEO_ITIN_PART_ORIG);
    assertGetGeoLocaleFromItin(true, "SFO");
  }
  void testGetGeoLocaleFromItin_Journey_GeoDest()
  {
    // destination from last itin tarvel segment
    geoLocaleFromItinData(RuleConst::TSI_SCOPE_JOURNEY, RuleConst::TSI_GEO_ITIN_PART_DEST);
    assertGetGeoLocaleFromItin(true, "SYD");
  }
  void testGetGeoLocaleFromItin_Journey_GeoTurnAround_seg1()
  {
    // first itin travel segment origin
    geoLocaleFromItinData(RuleConst::TSI_SCOPE_JOURNEY, RuleConst::TSI_GEO_ITIN_PART_TURNAROUND);
    _itin.furthestPointSegmentOrder() = 1;
    assertGetGeoLocaleFromItin(true, "SFO");
  }
  void testGetGeoLocaleFromItin_Journey_GeoTurnAround_seg2()
  {
    // second itin travel segment origin
    geoLocaleFromItinData(RuleConst::TSI_SCOPE_JOURNEY, RuleConst::TSI_GEO_ITIN_PART_TURNAROUND);
    _itin.furthestPointSegmentOrder() = 2;
    assertGetGeoLocaleFromItin(true, "DUB");
  }
  void testGetGeoLocaleFromItin_Journey_GeoTurnAround_seg5_noPu()
  {
    // no turnaround, no pricing unit
    geoLocaleFromItinData(RuleConst::TSI_SCOPE_JOURNEY, RuleConst::TSI_GEO_ITIN_PART_TURNAROUND);
    _itin.furthestPointSegmentOrder() = 5;
    assertGetGeoLocaleFromItin(false);
  }
  void testGetGeoLocaleFromItin_Journey_GeoTurnAround_seg5_Pu_noTurnAround()
  {
    // no itin turnaround, no turnaround in PU
    geoLocaleFromItinData(
        RuleConst::TSI_SCOPE_JOURNEY, RuleConst::TSI_GEO_ITIN_PART_TURNAROUND, true);
    _itin.furthestPointSegmentOrder() = 5;
    assertGetGeoLocaleFromItin(false);
  }
  void testGetGeoLocaleFromItin_Journey_GeoTurnAround_seg5_Pu_TurnAround()
  {
    // return turnaround from PU
    geoLocaleFromItinData(
        RuleConst::TSI_SCOPE_JOURNEY, RuleConst::TSI_GEO_ITIN_PART_TURNAROUND, true);
    _itin.furthestPointSegmentOrder() = 5;
    _pu->turnAroundPoint() = &_a2;
    assertGetGeoLocaleFromItin(true, "DUB");
  }
  void testGetGeoLocaleFromItin_Journey_Blank()
  {
    // blank is not handled
    geoLocaleFromItinData(RuleConst::TSI_SCOPE_JOURNEY, RuleConst::TSI_GEO_ITIN_PART_BLANK, true);
    assertGetGeoLocaleFromItin(false);
  }
  void testGetGeoLocaleFromItin_SubJourney_GeoOrig()
  {
    // get origin from first segment in PU
    geoLocaleFromItinData(
        RuleConst::TSI_SCOPE_SUB_JOURNEY, RuleConst::TSI_GEO_ITIN_PART_ORIG, true);
    assertGetGeoLocaleFromItin(true, "TYO");
  }
  void testGetGeoLocaleFromItin_SubJourney_GeoDest()
  {
    // destination from last segment in PU
    geoLocaleFromItinData(
        RuleConst::TSI_SCOPE_SUB_JOURNEY, RuleConst::TSI_GEO_ITIN_PART_DEST, true);
    assertGetGeoLocaleFromItin(true, "LON");
  }
  void testGetGeoLocaleFromItin_SegJourney_GeoTurnAround_NoTurnAround()
  {
    // noo turnaround in PU
    geoLocaleFromItinData(
        RuleConst::TSI_SCOPE_SUB_JOURNEY, RuleConst::TSI_GEO_ITIN_PART_TURNAROUND, true);
    assertGetGeoLocaleFromItin(false);
  }
  void testGetGeoLocaleFromItin_SegJourney_GeoTurnAround_TurnAround()
  {
    // return turnaround from PU
    geoLocaleFromItinData(
        RuleConst::TSI_SCOPE_SUB_JOURNEY, RuleConst::TSI_GEO_ITIN_PART_TURNAROUND, true);
    _pu->turnAroundPoint() = &_a3;
    assertGetGeoLocaleFromItin(true, "TYO");
  }
  void testGetGeoLocaleFromItin_SubJourney_Blank()
  {
    // blank is not handled
    geoLocaleFromItinData(
        RuleConst::TSI_SCOPE_SUB_JOURNEY, RuleConst::TSI_GEO_ITIN_PART_BLANK, true);
    assertGetGeoLocaleFromItin(false);
  }
  void testGetGeoLocaleFromItin_FareComp_GeoOrig()
  {
    // return origin from first fare market segment
    geoLocaleFromItinData(RuleConst::TSI_SCOPE_FARE_COMPONENT, RuleConst::TSI_GEO_ITIN_PART_ORIG);
    assertGetGeoLocaleFromItin(true, "DUB");
  }
  void testGetGeoLocaleFromItin_FareComp_GeoDest()
  {
    // destination from last faremarket segment
    geoLocaleFromItinData(RuleConst::TSI_SCOPE_FARE_COMPONENT, RuleConst::TSI_GEO_ITIN_PART_DEST);
    assertGetGeoLocaleFromItin(true, "DFW");
  }
  void testGetGeoLocaleFromItin_FareComp_GeoTurnAround_seg1()
  {
    // return fisrt itin segment origin as turnaround
    geoLocaleFromItinData(RuleConst::TSI_SCOPE_FARE_COMPONENT,
                          RuleConst::TSI_GEO_ITIN_PART_TURNAROUND);
    _itin.furthestPointSegmentOrder() = 1;
    assertGetGeoLocaleFromItin(true, "SFO");
  }
  void testGetGeoLocaleFromItin_FareComp_GeoTurnAround_seg2()
  {
    // return second itin segment as turnaround
    geoLocaleFromItinData(RuleConst::TSI_SCOPE_FARE_COMPONENT,
                          RuleConst::TSI_GEO_ITIN_PART_TURNAROUND);
    _itin.furthestPointSegmentOrder() = 2;
    assertGetGeoLocaleFromItin(true, "DUB");
  }
  void testGetGeoLocaleFromItin_FareComp_GeoTurnAround_seg5_noPu()
  {
    // no turnaround from itin, no PU
    geoLocaleFromItinData(RuleConst::TSI_SCOPE_FARE_COMPONENT,
                          RuleConst::TSI_GEO_ITIN_PART_TURNAROUND);
    _itin.furthestPointSegmentOrder() = 5;
    assertGetGeoLocaleFromItin(false);
  }
  void testGetGeoLocaleFromItin_FareComp_GeoTurnAround_seg5_Pu_noTurnAround()
  {
    // no turnaround set in PU
    geoLocaleFromItinData(
        RuleConst::TSI_SCOPE_FARE_COMPONENT, RuleConst::TSI_GEO_ITIN_PART_TURNAROUND, true);
    _itin.furthestPointSegmentOrder() = 5;
    assertGetGeoLocaleFromItin(false);
  }
  void testGetGeoLocaleFromItin_FareComp_GeoTurnAround_seg5_Pu_TurnAround()
  {
    // turnaround set in PU
    geoLocaleFromItinData(
        RuleConst::TSI_SCOPE_FARE_COMPONENT, RuleConst::TSI_GEO_ITIN_PART_TURNAROUND, true);
    _itin.furthestPointSegmentOrder() = 5;
    _pu->turnAroundPoint() = &_a2;
    assertGetGeoLocaleFromItin(true, "DUB");
  }
  void testGetGeoLocaleFromItin_FareComp_Blank()
  {
    // blank not suported
    geoLocaleFromItinData(
        RuleConst::TSI_SCOPE_FARE_COMPONENT, RuleConst::TSI_GEO_ITIN_PART_BLANK, true);
    assertGetGeoLocaleFromItin(false);
  }

  void setUp()
  {
    RuleUtilTSITest_Base::setUp();
    _vendor = "ATP";
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _a1.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    _a1.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _a2.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDUB.xml");
    _a2.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    _a3.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTYO.xml");
    _a3.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");
    _itin.travelSeg() += &_a1, &_a2, &_a3;
    _fm.travelSeg() += &_a2, &_a3, &_a1;
  }

private:
  void geoLocaleFromItinData(RuleConst::TSIScopeType scope,
                             RuleConst::TSIGeoItinPart geoItin,
                             bool pu = false)
  {
    if (pu)
    {
      _pu = _memHandle.create<PricingUnit>();
      _pu->travelSeg() += &_a3, &_a1, &_a2;
    }
    else
      _pu = 0;
    _tsiScope = scope;
    // call now when PricingUnit is created (or not)
    _tsi = _memHandle.insert(
        new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, &_itin, _pu, &_fm));
    _tsiInfo.scope() = scope;
    _tsiInfo.geoItinPart() = geoItin;
  }
  void assertGetGeoLocaleFromItin(bool result, LocCode locCode = "")
  {
    const Loc* loc = 0;
    CPPUNIT_ASSERT_EQUAL(result, RuleUtilTSI::getGeoLocaleFromItin(*_tsi, loc));
    if (result)
      CPPUNIT_ASSERT_EQUAL(locCode, loc->loc());
  }
  RuleConst::TSIScopeType _tsiScope;
  RuleUtilTSI::TSIData* _tsi;
  TSIInfo _tsiInfo;
  VendorCode _vendor;
  Itin _itin;
  PricingUnit* _pu;
  FareMarket _fm;
  AirSeg _a1;
  AirSeg _a2;
  AirSeg _a3;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_GetLocationMatchings);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_CheckGeoData);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_CheckGeoNotType);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_GetLoopItemToSetLoopSet);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_CheckLoopMatch);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_CheckMatchCriteria);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_CheckTSIMatchTravelSeg);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_GetGeoLocaleFromItin);
}
