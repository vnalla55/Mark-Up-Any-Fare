#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"
#include "Diagnostic/Diag452Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "test/include/TestMemHandle.h"
#include "Routing/TravelRoute.h"
#include "Common/TseEnums.h"
#include "DBAccess/TPMExclusion.h"

using namespace std;

namespace tse
{
namespace
{
class MockTPMExclusion : public TPMExclusion
{
public:
  MockTPMExclusion(const Indicator& userApplTypeVal = NO_PARAM,
                   const UserApplCode& userApplCode = "")
  {
    carrier() = "LO";
    seqNo() = 1;
    notApplToYY() = 'A';
    onlineSrvOnly() = 'B';
    directionality() = FROM;
    loc1type() = SUBAREA;
    loc2type() = SUBAREA;
    loc1() = "20";
    loc2() = "30";
    globalDir() = GlobalDirection::AT;
    sec1Appl() = 'G';
    sec1Loc1Type() = IATA_AREA;
    sec1Loc1() = "40";
    sec1Loc2Type() = MARKET;
    sec1Loc2() = "AAA";
    sec2Appl() = 'J';
    sec2Loc1Type() = IATA_AREA;
    sec2Loc1() = "50";
    sec2Loc2Type() = MARKET;
    sec2Loc2() = "BBB";
    viaPointRest() = 'M';
    consecMustBeOnGovCxr() = 'N';
    surfacePermitted() = 'O';
    createDate() = 0;
    effDate() = 0;
    discDate() = 0;
    expireDate() = 0;
    userApplType() = userApplTypeVal;
    userAppl() = userApplCode;
  }
};

class MockDiag452Collector : public Diag452Collector
{
public:
  MockDiag452Collector(Diagnostic& root) : Diag452Collector(root), _zoneInfo(NULL) {}

  void setZoneInfo(bool include = true)
  {
    _zoneInfo = _memHandle.insert(new ZoneInfo());
    ZoneInfo::ZoneSeg zoneSeg;

    zoneSeg.inclExclInd() = include ? 'I' : 'E';
    vector<ZoneInfo::ZoneSeg> segVect;
    zoneSeg.locType() = NATION;
    zoneSeg.loc() = "PL";
    segVect.push_back(zoneSeg);
    zoneSeg.locType() = MARKET;
    zoneSeg.loc() = "FRA";
    segVect.push_back(zoneSeg);
    _zoneInfo->sets().push_back(segVect);
    _zoneInfo->sets().push_back(segVect);
    _zoneInfo->zone() = "7628";
  }

  const ZoneInfo* getZoneInfo(const LocCode& locCode) { return _zoneInfo; }

  // Data
  ZoneInfo* _zoneInfo;
  TestMemHandle _memHandle;
};

class MockMileageRouteItems : public MileageRouteItems
{
public:
  MockMileageRouteItems(bool emptyData = true, bool isSurface = false)
  {
    if (!emptyData)
    {
      _loc1.loc() = "KRK";
      _loc2.loc() = "FRA";
      _loc3.loc() = "MIA";

      MileageRouteItem item1;
      MileageRouteItem item2;

      item1.city1() = &_loc1;
      item1.city2() = &_loc2;
      item2.city2() = &_loc3;
      item1.segmentCarrier() = isSurface ? INDUSTRY_CARRIER : "LH";
      item2.segmentCarrier() = isSurface ? INDUSTRY_CARRIER : "LH";
      item1.isSurface() = isSurface;
      item2.isSurface() = isSurface;

      push_back(item1);
      push_back(item2);
    }
  }

  // Data
  Loc _loc1;
  Loc _loc2;
  Loc _loc3;
};

class MockMileageRoute : public MileageRoute
{
public:
  MockMileageRoute(bool emptyData = true, bool fOutbound = true) : _routeItems(emptyData)
  {
    if (emptyData)
    {
      globalDirection() = GlobalDirection::ZZ;
      governingCarrier() = "";
    }
    else
    {
      ticketingDT() = DateTime(2000, 1, 1);
      globalDirection() = GlobalDirection::AT;
      governingCarrier() = "LH";
      mileageRouteItems() = _routeItems;
    }
    isOutbound() = fOutbound;
  }

  // Data
  MockMileageRouteItems _routeItems;
};
}

class Diag452CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag452CollectorTest);
  CPPUNIT_TEST(testConstructor);

  CPPUNIT_TEST(testPrintHeader);
  CPPUNIT_TEST(testPrintHeader_NotActive);

  CPPUNIT_TEST(testPrintFooter_NotActive);
  CPPUNIT_TEST(testPrintFooter_NotMatched);
  CPPUNIT_TEST(testPrintFooter_Matched);
  CPPUNIT_TEST(testPrintFooter_NoRecordDisplayed);

  CPPUNIT_TEST(testDisplayTPMExclusion_NotActive);
  CPPUNIT_TEST(testDisplayTPMExclusion);

  CPPUNIT_TEST(testDisplayFailCode_NotActive);
  CPPUNIT_TEST(testDisplayFailCode);
  CPPUNIT_TEST(testDisplayFailCode_Matched);

  CPPUNIT_TEST(testPrintFareMarketHeader_EmptyData);
  CPPUNIT_TEST(testPrintFareMarketHeader_Outbound);
  CPPUNIT_TEST(testPrintFareMarketHeader_Inbound);
  CPPUNIT_TEST(testPrintFareMarketHeader_NotActive);

  CPPUNIT_TEST(testDisplayMileageRouteItems);
  CPPUNIT_TEST(testDisplayMileageRouteItems_WithSurface);

  CPPUNIT_TEST(testPrintLineTitle_Crs);
  CPPUNIT_TEST(testPrintLineTitle_GovCxr);
  CPPUNIT_TEST(testPrintLineTitle_Seqno);

  CPPUNIT_TEST(testPrintDirectionality_From);
  CPPUNIT_TEST(testPrintDirectionality_Between);
  CPPUNIT_TEST(testPrintDirectionality_To);

  CPPUNIT_TEST(testDecodeLoc_Area);
  CPPUNIT_TEST(testDecodeLoc_SubArea);
  CPPUNIT_TEST(testDecodeLoc_Market);
  CPPUNIT_TEST(testDecodeLoc_Nation);
  CPPUNIT_TEST(testDecodeLoc_State);
  CPPUNIT_TEST(testDecodeLoc_ZoneEmpty);
  CPPUNIT_TEST(testDecodeLoc_ZoneNotEmpty);
  CPPUNIT_TEST(testDecodeLoc_ZoneNotEmptyExcluded);

  CPPUNIT_TEST(testPrintTextWithNewLine_NewLine);
  CPPUNIT_TEST(testPrintTextWithNewLine_NoNewLine);

  CPPUNIT_TEST(testPrintCrsMultihost_NotDDAll);
  CPPUNIT_TEST(testPrintCrsMultihost_CRS);
  CPPUNIT_TEST(testPrintCrsMultihost_Multihost);
  CPPUNIT_TEST(testPrintCrsMultihost_Blank);
  CPPUNIT_TEST_SUITE_END();

  // data
private:
  Diag452Collector* _collector;
  Diagnostic* _diagroot;
  TestMemHandle _memHandle;

  // helper methods
public:
  void setUp()
  {
    try
    {
      _diagroot = _memHandle.insert(new Diagnostic(Diagnostic452));
      _diagroot->activate();
      _collector = _memHandle.insert(new MockDiag452Collector(*_diagroot));
      _collector->enable(Diagnostic452);
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void tearDown() { _memHandle.clear(); }

  void setCollectorFlags(bool active = true, bool matched = true, bool separatorNeeded = true)
  {
    _collector->_active = active;
    _collector->_matched = matched;
    _collector->_separatorNeeded = separatorNeeded;
  }

  // tests
public:
  void testConstructor()
  {
    try
    {
      Diag452Collector diag;
      CPPUNIT_ASSERT(diag.str().empty());
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testPrintHeader()
  {
    setCollectorFlags();
    _collector->printHeader();
    CPPUNIT_ASSERT_EQUAL(
        string("*******************  TPM EXCLUSION TABLE  *********************\n"),
        _collector->str());
  }

  void testPrintHeader_NotActive()
  {
    setCollectorFlags(false);
    _collector->printHeader();
    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testPrintFooter_NotActive()
  {
    setCollectorFlags(false);
    _collector->printFooter();
    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testPrintFooter_NotMatched()
  {
    setCollectorFlags(true, false, false);
    _collector->printFooter();
    CPPUNIT_ASSERT_EQUAL(
        string("TPM EXCLUSION IS NOT APPLIED\n"
               "***************************************************************\n"),
        _collector->str());
  }

  void testPrintFooter_Matched()
  {
    setCollectorFlags(true, true, false);
    _collector->printFooter();
    CPPUNIT_ASSERT_EQUAL(
        string("TPM EXCLUSION IS APPLIED\n"
               "***************************************************************\n"),
        _collector->str());
  }

  void testPrintFooter_NoRecordDisplayed()
  {
    setCollectorFlags();
    _collector->printFooter();
    CPPUNIT_ASSERT_EQUAL(
        string("***************************************************************\n"
               "TPM EXCLUSION IS APPLIED\n"
               "***************************************************************\n"),
        _collector->str());
  }

  void testDisplayTPMExclusion_NotActive()
  {
    setCollectorFlags(false);
    *_collector << MockTPMExclusion();
    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testDisplayTPMExclusion()
  {
    setCollectorFlags();
    *_collector << MockTPMExclusion();
    string ret = "GOV CXR            :LO\n"
                 "SEQ NO             :1\n"
                 "NOT APPL TO YY     :A\n"
                 "ONLINE SERVICE ONLY:B\n"
                 "DIR FARE COMPONENT :FROM\n"
                 "FC LOC 1           :SUBAREA 20\n"
                 "FC LOC 2           :SUBAREA 30\n"
                 "GI                 :AT\n"
                 "SR 1 APPL          :G\n"
                 "SR 1 LOC 1         :AREA 40\n"
                 "SR 1 LOC 2         :AAA\n"
                 "SR 2 APPL          :J\n"
                 "SR 2 LOC 1         :AREA 50\n"
                 "SR 2 LOC 2         :BBB\n"
                 "VIA POINT RES      :M\n"
                 "CSR ONLINE ON GOV  :N\n"
                 "SURFACE PER        :O\n"
                 "CREATE DATE        :01JAN1970\n"
                 "FIRST SALE DATE    :01JAN1970\n"
                 "LAST SALE DATE     :01JAN1970\n"
                 "EXPIRE DATE        :01JAN1970\n";

    CPPUNIT_ASSERT_EQUAL(ret, _collector->str());
  }

  void testDisplayFailCode_NotActive()
  {
    setCollectorFlags(false);
    *_collector << SouthAtlanticTPMExclusion::MATCHED;
    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testDisplayFailCode()
  {
    setCollectorFlags();
    *_collector << SouthAtlanticTPMExclusion::FAILED_CRS;
    CPPUNIT_ASSERT_EQUAL(
        string("VALIDATION RESULT:   FAIL - CRS\n"
               "***************************************************************\n"),
        _collector->str());
  }

  void testDisplayFailCode_Matched()
  {
    setCollectorFlags();
    *_collector << SouthAtlanticTPMExclusion::MATCHED;
    CPPUNIT_ASSERT_EQUAL(
        string("VALIDATION RESULT:   MATCHED\n"
               "***************************************************************\n"),
        _collector->str());
  }

  void testPrintFareMarketHeader_EmptyData()
  {
    setCollectorFlags();
    _collector->printFareMarketHeader(MockMileageRoute());
    CPPUNIT_ASSERT_EQUAL(
        string("     /CXR-/GI- DIR-OUT\n"
               "***************************************************************\n"),
        _collector->str());
  }

  void testPrintFareMarketHeader_Outbound()
  {
    setCollectorFlags();
    _collector->printFareMarketHeader(MockMileageRoute(false));
    CPPUNIT_ASSERT_EQUAL(
        string("KRK-LH-FRA-LH-MIA     /CXR-LH/GI-AT DIR-OUT\n"
               "***************************************************************\n"),
        _collector->str());
  }
  void testPrintFareMarketHeader_Inbound()
  {
    setCollectorFlags();
    _collector->printFareMarketHeader(MockMileageRoute(false, false));
    CPPUNIT_ASSERT_EQUAL(
        string("KRK-LH-FRA-LH-MIA     /CXR-LH/GI-AT DIR-IN\n"
               "***************************************************************\n"),
        _collector->str());
  }

  void testPrintFareMarketHeader_NotActive()
  {
    setCollectorFlags(false);
    _collector->printFareMarketHeader(MockMileageRoute());
    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testDisplayMileageRouteItems()
  {
    _collector->displayMileageRouteItems(MockMileageRouteItems(false));
    CPPUNIT_ASSERT_EQUAL(string("KRK-LH-FRA-LH-MIA"), _collector->str());
  }

  void testDisplayMileageRouteItems_WithSurface()
  {
    _collector->displayMileageRouteItems(MockMileageRouteItems(false, true));
    CPPUNIT_ASSERT_EQUAL(string("KRK // FRA // MIA"), _collector->str());
  }

  void testPrintLineTitle_Crs()
  {
    _collector->printLineTitle(SouthAtlanticTPMExclusion::FAILED_CRS);
    CPPUNIT_ASSERT_EQUAL(string("CRS                :"), _collector->str());
  }

  void testPrintLineTitle_GovCxr()
  {
    _collector->printLineTitle(SouthAtlanticTPMExclusion::FAILED_GOV_CXR);
    CPPUNIT_ASSERT_EQUAL(string("GOV CXR            :"), _collector->str());
  }

  void testPrintLineTitle_Seqno()
  {
    _collector->printLineTitle(SouthAtlanticTPMExclusion::FAILED_SEQ_NO);
    CPPUNIT_ASSERT_EQUAL(string("SEQ NO             :"), _collector->str());
  }

  void testPrintDirectionality_From()
  {
    _collector->printDirectionality(FROM);
    CPPUNIT_ASSERT_EQUAL(string("FROM"), _collector->str());
  }

  void testPrintDirectionality_Between()
  {
    _collector->printDirectionality(BETWEEN);
    CPPUNIT_ASSERT_EQUAL(string("BETWEEN"), _collector->str());
  }

  void testPrintDirectionality_To()
  {
    _collector->printDirectionality(TO);
    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testDecodeLoc_Area()
  {
    _collector->decodeLoc(IATA_AREA, "3");
    CPPUNIT_ASSERT_EQUAL(string("AREA 3\n"), _collector->str());
  }

  void testDecodeLoc_SubArea()
  {
    _collector->decodeLoc(SUBAREA, "23");
    CPPUNIT_ASSERT_EQUAL(string("SUBAREA 23\n"), _collector->str());
  }

  void testDecodeLoc_Market()
  {
    _collector->decodeLoc(MARKET, "FRA");
    CPPUNIT_ASSERT_EQUAL(string("FRA\n"), _collector->str());
  }

  void testDecodeLoc_Nation()
  {
    _collector->decodeLoc(NATION, "PL");
    CPPUNIT_ASSERT_EQUAL(string("PL\n"), _collector->str());
  }

  void testDecodeLoc_State()
  {
    _collector->decodeLoc(STATE_PROVINCE, "TX");
    CPPUNIT_ASSERT_EQUAL(string("TX\n"), _collector->str());
  }

  void testDecodeLoc_ZoneEmpty()
  {
    _collector->decodeLoc(ZONE, "7628");
    CPPUNIT_ASSERT_EQUAL(string("\n"), _collector->str());
  }

  void testDecodeLoc_ZoneNotEmpty()
  {
    dynamic_cast<MockDiag452Collector*>(_collector)->setZoneInfo();
    _collector->decodeLoc(ZONE, "7628");
    CPPUNIT_ASSERT_EQUAL(string("PL, FRA, PL, FRA\n"), _collector->str());
  }

  void testDecodeLoc_ZoneNotEmptyExcluded()
  {
    dynamic_cast<MockDiag452Collector*>(_collector)->setZoneInfo(false);
    _collector->decodeLoc(ZONE, "7628");
    CPPUNIT_ASSERT_EQUAL(string("ZONE 7628\n"), _collector->str());
  }

  void testPrintTextWithNewLine_NewLine()
  {
    int pos = 51;
    _collector->printTextWithNewLine(string("13 characters"), pos);
    CPPUNIT_ASSERT_EQUAL(string("\n          13 characters"), _collector->str());
  }

  void testPrintTextWithNewLine_NoNewLine()
  {
    int pos = 50;
    _collector->printTextWithNewLine(string("13 characters"), pos);
    CPPUNIT_ASSERT_EQUAL(string("13 characters"), _collector->str());
  }

  void testPrintCrsMultihost_NotDDAll()
  {
    _collector->printCrsMultihost(MockTPMExclusion());
    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testPrintCrsMultihost_CRS()
  {
    _diagroot->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "ALL";
    _collector->printCrsMultihost(MockTPMExclusion(CRS_USER_APPL, "SABR"));
    CPPUNIT_ASSERT_EQUAL(string("CRS                :SABR\n"
                                "MULTIHOST          :\n"),
                         _collector->str());
  }

  void testPrintCrsMultihost_Multihost()
  {
    _diagroot->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "ALL";
    _collector->printCrsMultihost(MockTPMExclusion(MULTIHOST_USER_APPL, "SU"));
    CPPUNIT_ASSERT_EQUAL(string("CRS                :\n"
                                "MULTIHOST          :SU\n"),
                         _collector->str());
  }

  void testPrintCrsMultihost_Blank()
  {
    _diagroot->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "ALL";
    _collector->printCrsMultihost(MockTPMExclusion());
    CPPUNIT_ASSERT_EQUAL(string("CRS                :\n"
                                "MULTIHOST          :\n"),
                         _collector->str());
  }

}; // class

CPPUNIT_TEST_SUITE_REGISTRATION(Diag452CollectorTest);
}
