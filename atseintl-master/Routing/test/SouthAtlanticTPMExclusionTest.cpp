#include "test/include/CppUnitHelperMacros.h"
#include <string>
#include "test/include/TestMemHandle.h"
#include "Routing/SouthAtlanticTPMExclusion.h"
#include "DBAccess/TPMExclusion.h"
#include "Routing/MileageRoute.h"
#include "Routing/MileageRouteItem.h"
#include "test/testdata/TestMileageRouteFactory.h"
#include "test/testdata/TestTPMExclusionFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "Diagnostic/Diag452Collector.h"
#include "Common/TseConsts.h"
#include "DBAccess/DataHandle.h"
#include "Routing/MultiTransportRetriever.h"
#include <boost/assign/std/vector.hpp>

using namespace std;
using namespace boost::assign;

namespace tse
{

std::vector<TPMExclusion*> g_tpmExc;

struct SouthAtlanticTPMExclusionMock : public SouthAtlanticTPMExclusion
{
  struct MultiTransportRetrieverMock : public MultiTransportRetriever
  {
    bool retrieve(MileageRouteItem& mit, DataHandle&, Indicator mileageType = TPM) const
    {
      if (mit.city1())
      {
        mit.multiTransportOrigin() = getCity(mit.city1()->loc());
      }
      if (mit.city2())
      {
        mit.multiTransportDestination() = getCity(mit.city2()->loc());
      }
      return true;
    }
    Loc* getCity(const LocCode& loc) const
    {
      if (loc == "EWR" || loc == "JFK")
        return TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");

      return 0;
    }
  } _mutliTransRetriever;
  bool matchLocation(const Loc* loc,
                     const LocTypeCode& locType,
                     const LocCode& lc,
                     MileageRoute& mRoute,
                     CarrierCode cxr) const
  {
    if (locType == 'Z')
    {
      if (lc == LocCode("01015"))
        return loc->nation() == "AR" || loc->nation() == "BR" || loc->nation() == "CL" ||
               loc->nation() == "PY" || loc->nation() == "UY";
      if (lc == LocCode("01188"))
        return loc->subarea() == "21";
      if (lc == LocCode("01199"))
        return loc->subarea() == "22";
      if (lc == LocCode("01249"))
        return loc->loc() == "MIA" || loc->loc() == "DFW";
      if (lc == LocCode("01899"))
        return loc->loc() == "MIA" || loc->loc() == "NYC" || loc->loc() == "YTO" ||
               loc->loc() == "YUL";
      if (lc == LocCode("01900"))
        return loc->loc() == "MIA" || loc->loc() == "NYC" || loc->loc() == "DFW" ||
               loc->loc() == "CHI";
      if (lc == LocCode("02946"))
        return loc->loc() == "MIA" || loc->loc() == "NYC" || loc->loc() == "YTO" ||
               loc->loc() == "YUL" || loc->loc() == "ATL" || loc->loc() == "WAS";
      if (lc == LocCode("06820"))
        return loc->area() == "3" || loc->subarea() == "21";
      if (lc == LocCode("07629"))
        return loc->loc() == "BOS" || loc->loc() == "NYC" || loc->loc() == "CHI";
      // return false to prevent seg fault

      return false;
    }
    return SouthAtlanticTPMExclusion::matchLocation(loc, locType, lc, mRoute);
  }
  const std::vector<TPMExclusion*>& getTPMExclus(MileageRoute& mRoute) const
  {
    // const function - use global
    g_tpmExc.clear();
    if (mRoute.governingCarrier() == "AA")
    {
      g_tpmExc.push_back(TestTPMExclusionFactory::create("data/TPMExclusion_AA_1.xml"));
      g_tpmExc.push_back(TestTPMExclusionFactory::create("data/TPMExclusion_AA_2.xml"));
      g_tpmExc.push_back(TestTPMExclusionFactory::create("data/TPMExclusion_AA_3.xml"));
      g_tpmExc.push_back(TestTPMExclusionFactory::create("data/TPMExclusion_AA_4.xml"));
      g_tpmExc.push_back(TestTPMExclusionFactory::create("data/TPMExclusion_AA_5.xml"));
      g_tpmExc.push_back(TestTPMExclusionFactory::create("data/TPMExclusion_AA_6.xml"));
      g_tpmExc.push_back(TestTPMExclusionFactory::create("data/TPMExclusion_BLANK_1.xml"));
      g_tpmExc.push_back(TestTPMExclusionFactory::create("data/TPMExclusion_BLANK_2.xml"));
    }
    return g_tpmExc;
  }
  const MultiTransportRetriever& getMultTransretriever() const { return _mutliTransRetriever; }
};

class SouthAtlanticTPMExclusionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SouthAtlanticTPMExclusionTest);

  CPPUNIT_TEST(testMatchGlobalDirection_Blank);
  CPPUNIT_TEST(testMatchGlobalDirection_MatchDir);
  CPPUNIT_TEST(testMatchGlobalDirection_NoMatchDir);

  CPPUNIT_TEST(testMatchYY_MatchYY_Indfare);
  CPPUNIT_TEST(testMatchYY_MatchYY_NotIndfare);
  CPPUNIT_TEST(testMatchYY_NoMatchYY_Indfare);
  CPPUNIT_TEST(testMatchYY_NoMatchYY_NotIndfare);

  CPPUNIT_TEST(testMatchOlineSvcOnly_NoMatch);
  CPPUNIT_TEST(testMatchOlineSvcOnly_Match_MatchCxr);
  CPPUNIT_TEST(testMatchOlineSvcOnly_Match_NoMatchCxr);

  CPPUNIT_TEST(testMachLocation1_None);
  CPPUNIT_TEST(testMachLocation1_From_MatchOrig);
  CPPUNIT_TEST(testMachLocation1_From_NoMatchOrig);
  CPPUNIT_TEST(testMachLocation1_Between_MatchOrig);
  CPPUNIT_TEST(testMachLocation1_Between_MatchDest);
  CPPUNIT_TEST(testMachLocation1_Between_NoMatch);

  CPPUNIT_TEST(testMachLocation2_None);
  CPPUNIT_TEST(testMachLocation2_From_MatchDest);
  CPPUNIT_TEST(testMachLocation2_From_NoMatchDest);
  CPPUNIT_TEST(testMachLocation2_Between_MatchOrig);
  CPPUNIT_TEST(testMachLocation2_Between_MatchDest);
  CPPUNIT_TEST(testMachLocation2_Between_NoMatch);

  CPPUNIT_TEST(testMatchCRSReturnsTrueWhen_NO_PARAM);
  CPPUNIT_TEST(testMatchCRSReturnsTrueWhen_CRS_Match);
  CPPUNIT_TEST(testMatchCRSReturnsFalseWhen_CRS_NotMatch);
  CPPUNIT_TEST(testMatchMultiHostReturnsTrueWhen_MultiHost_Match);
  CPPUNIT_TEST(testMatchMultiHostReturnsFalseWhen_MultiHost_NotMatch);

  CPPUNIT_TEST(testMatchConsecutive_Asc_NoSec);
  CPPUNIT_TEST(testMatchConsecutive_Asc_OneSec);
  CPPUNIT_TEST(testMatchConsecutive_Asc_TwoSec);
  CPPUNIT_TEST(testMatchConsecutive_Desc_NoSec);
  CPPUNIT_TEST(testMatchConsecutive_Desc_OneSec);
  CPPUNIT_TEST(testMatchConsecutive_Desc_TwoSec);

  CPPUNIT_TEST(testMatchConsecMustBeOnGov_CheckNo_S1AA_S2UA);
  CPPUNIT_TEST(testMatchConsecMustBeOnGov_CheckYes_S1AA_S2AA);
  CPPUNIT_TEST(testMatchConsecMustBeOnGov_CheckYes_S1AA_S2UA);
  CPPUNIT_TEST(testMatchConsecMustBeOnGov_CheckYes_S1UA_S2AA);

  CPPUNIT_TEST(testMatchSurface_Permitted);
  CPPUNIT_TEST(testMatchSurface_NotPermitted);

  CPPUNIT_TEST(testApplySegments);
  CPPUNIT_TEST(testApplyGRULHR);
  CPPUNIT_TEST(testApplyLHRGRU);

  CPPUNIT_TEST(testMatchSector1_Directionality_FROM_Match);
  CPPUNIT_TEST(testMatchSector1_Directionality_FROM_NotMatch);
  CPPUNIT_TEST(testMatchSector1_Directionality_FROM_NotMatchOppositeDir);
  CPPUNIT_TEST(testMatchSector1_Directionality_BETWEEN_Match);
  CPPUNIT_TEST(testMatchSector1_Directionality_BETWEEN_NotMatch);
  CPPUNIT_TEST(testMatchSector1_Directionality_BETWEEN_MatchOppositeDir);
  CPPUNIT_TEST(testMatchSector2_Directionality_FROM_Match);
  CPPUNIT_TEST(testMatchSector2_Directionality_FROM_NotMatch);
  CPPUNIT_TEST(testMatchSector2_Directionality_FROM_NotMatchOppositeDir);
  CPPUNIT_TEST(testMatchSector2_Directionality_BETWEEN_Match);
  CPPUNIT_TEST(testMatchSector2_Directionality_BETWEEN_NotMatch);
  CPPUNIT_TEST(testMatchSector2_Directionality_BETWEEN_MatchOppositeDir);

  CPPUNIT_TEST(testMarkSectors);

  CPPUNIT_TEST(testMatchViaPointOfIntrestr_T_Asc_SameLocations);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_T_Asc_NotSameLocations);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_T_Desc_SameLocations);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_T_Desc_NotSameLocations);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_G_Asc_2Segments);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_G_Asc_Surface);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_G_Asc_SameCarrier);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_G_Asc_DiffCarrier);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_G_Desc_2Segments);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_G_Desc_Surface);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_G_Desc_SameCarrier);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_G_Desc_DiffCarrier);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_A_Asc_2Segments);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_A_Asc_Surface);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_A_Asc_NotSurface);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_A_Desc_2Segments);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_A_Desc_Surface);
  CPPUNIT_TEST(testMatchViaPointOfIntrestr_A_Desc_NotSurface);

  CPPUNIT_TEST(testMatchSectorAppl_D_Sec1);
  CPPUNIT_TEST(testMatchSectorAppl_D_Sec2);
  CPPUNIT_TEST(testMatchSectorAppl_N_Sec1_NoHideStop);
  CPPUNIT_TEST(testMatchSectorAppl_N_Sec1_HideStop);
  CPPUNIT_TEST(testMatchSectorAppl_N_Sec2_NoHideStop);
  CPPUNIT_TEST(testMatchSectorAppl_N_Sec2_HideStop);

  CPPUNIT_TEST(testMultiCityOrig_MultiTransportOriginNull_CityNotEmpty);
  CPPUNIT_TEST(testMultiCityOrig_MultiTransportOriginNotNull_CityEmpty);
  CPPUNIT_TEST(testMultiCityOrig_MultiTransportOriginNotNull_CityNotEmpty);

  CPPUNIT_TEST(testMultiCityDest_MultiTransportDestinationNull_CityNotEmpty);
  CPPUNIT_TEST(testMultiCityDest_MultiTransportDestinationNotNull_CityEmpty);
  CPPUNIT_TEST(testMultiCityDest_MultiTransportDestinationNotNull_CityNotEmpty);

  CPPUNIT_TEST(testCloneMileageRouteGRU_NYC_FRA);
  CPPUNIT_TEST(testCloneMileageRouteGRU_NYC_EWR_FRA);
  CPPUNIT_TEST(testCloneMileageRouteGRU_NYC_EWR_JFK_FRA);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _mRoute = TestMileageRouteFactory::create("data/MileageRoute.xml", true);
    _mItem = _memHandle.create<MileageRouteItem>();
    _tpm = _memHandle.create<TPMExclusion>();
    // set some dummy data
    _tpm->loc1type() = LOCTYPE_AREA;
    _tpm->loc1() = "2";
    _tpm->loc2type() = LOCTYPE_AREA;
    _tpm->loc2() = "2";
    _tpm->globalDir() = GlobalDirection::EH;
    _tpm->notApplToYY() = YES;
    _tpm->onlineSrvOnly() = YES;
    _tpm->directionality() = TERMINATE;
    _tpm->sec1Appl() = ' ';
    _tpm->sec2Appl() = ' ';
    _tpmValidator = _memHandle.create<SouthAtlanticTPMExclusionMock>();
    _rootDiag = _memHandle.create<Diagnostic>();
    _diag = _memHandle.insert(new Diag452Collector(*_rootDiag));
    _rootDiag->activate();
  }
  void tearDown()
  {
    _memHandle.clear();
  }
  ////////////////////////////////////////////////////////////////////////////////

  void testMatchGlobalDirection_Blank()
  { // match on blank direction
    _tpm->globalDir() = GlobalDirection::ZZ;
    CPPUNIT_ASSERT(_tpmValidator->matchGlobalDirection(*_mRoute, *_tpm));
  }
  void testMatchGlobalDirection_MatchDir()
  { // match on direction
    _tpm->globalDir() = GlobalDirection::WH;
    CPPUNIT_ASSERT(_tpmValidator->matchGlobalDirection(*_mRoute, *_tpm));
  }
  void testMatchGlobalDirection_NoMatchDir()
  { // no match on direction
    _tpm->globalDir() = GlobalDirection::PA;
    CPPUNIT_ASSERT(!_tpmValidator->matchGlobalDirection(*_mRoute, *_tpm));
  }
  void testMatchYY_MatchYY_Indfare()
  { // can match - pass
    _tpm->notApplToYY() = NO;
    CPPUNIT_ASSERT(_tpmValidator->matchYY(*_mRoute, *_tpm));
  }
  void testMatchYY_MatchYY_NotIndfare()
  { // can match - pass
    _tpm->notApplToYY() = NO;
    _mRoute->isYYFare() = false;
    CPPUNIT_ASSERT(_tpmValidator->matchYY(*_mRoute, *_tpm));
  }
  void testMatchYY_NoMatchYY_Indfare()
  { // can't match, fare is ind - fail
    _tpm->notApplToYY() = YES;
    CPPUNIT_ASSERT(!_tpmValidator->matchYY(*_mRoute, *_tpm));
  }
  void testMatchYY_NoMatchYY_NotIndfare()
  { // can't match, fare is not ind - pass
    _tpm->notApplToYY() = YES;
    _mRoute->isYYFare() = false;
    CPPUNIT_ASSERT(_tpmValidator->matchYY(*_mRoute, *_tpm));
  }
  void testMatchOlineSvcOnly_NoMatch()
  { // no check for carrier
    _tpm->onlineSrvOnly() = NO;
    CPPUNIT_ASSERT(_tpmValidator->matchOlineSvcOnly(*_mRoute, *_tpm));
  }
  void testMatchOlineSvcOnly_Match_MatchCxr()
  { // match on carrier
    _tpm->onlineSrvOnly() = YES;
    CPPUNIT_ASSERT(_tpmValidator->matchOlineSvcOnly(*_mRoute, *_tpm));
  }
  void testMatchOlineSvcOnly_Match_NoMatchCxr()
  { // no match on  governing carrier
    _tpm->onlineSrvOnly() = YES;
    _mRoute->mileageRouteItems().front().segmentCarrier() = "XX";
    CPPUNIT_ASSERT(!_tpmValidator->matchOlineSvcOnly(*_mRoute, *_tpm));
  }
  void testMachLocation1_None()
  { // no location - always pass
    _tpm->loc1type() = LOCTYPE_NONE;
    CPPUNIT_ASSERT(_tpmValidator->matchLocation1(*_mRoute, *_tpm));
  }
  void testMachLocation1_From_MatchOrig()
  { // match on location
    _tpm->directionality() = FROM;
    _tpm->loc1type() = LOCTYPE_NATION;
    _tpm->loc1() = "BR";
    CPPUNIT_ASSERT(_tpmValidator->matchLocation1(*_mRoute, *_tpm));
  }
  void testMachLocation1_From_NoMatchOrig()
  { // match on location
    _tpm->directionality() = FROM;
    _tpm->loc1type() = LOCTYPE_NATION;
    _tpm->loc1() = "AU";
    CPPUNIT_ASSERT(!_tpmValidator->matchLocation1(*_mRoute, *_tpm));
  }
  void testMachLocation1_Between_MatchOrig()
  {
    _tpm->directionality() = BETWEEN;
    _tpm->loc1type() = LOCTYPE_NATION;
    _tpm->loc1() = "BR";
    CPPUNIT_ASSERT(_tpmValidator->matchLocation1(*_mRoute, *_tpm));
  }
  void testMachLocation1_Between_MatchDest()
  {
    _tpm->directionality() = BETWEEN;
    _tpm->loc1type() = LOCTYPE_NATION;
    _tpm->loc1() = "US";
    CPPUNIT_ASSERT(_tpmValidator->matchLocation1(*_mRoute, *_tpm));
  }
  void testMachLocation1_Between_NoMatch()
  {
    _tpm->directionality() = BETWEEN;
    _tpm->loc1type() = LOCTYPE_NATION;
    _tpm->loc1() = "AU";
    CPPUNIT_ASSERT(!_tpmValidator->matchLocation1(*_mRoute, *_tpm));
  }
  void testMachLocation2_None()
  { // no location - always pass
    _tpm->loc2type() = LOCTYPE_NONE;
    CPPUNIT_ASSERT(_tpmValidator->matchLocation2(*_mRoute, *_tpm));
  }
  void testMachLocation2_From_MatchDest()
  { // match on location
    _tpm->directionality() = FROM;
    _tpm->loc2type() = LOCTYPE_NATION;
    _tpm->loc2() = "US";
    CPPUNIT_ASSERT(_tpmValidator->matchLocation2(*_mRoute, *_tpm));
  }
  void testMachLocation2_From_NoMatchDest()
  { // match on location
    _tpm->directionality() = FROM;
    _tpm->loc2type() = LOCTYPE_NATION;
    _tpm->loc2() = "AU";
    CPPUNIT_ASSERT(!_tpmValidator->matchLocation2(*_mRoute, *_tpm));
  }
  void testMachLocation2_Between_MatchOrig()
  {
    _tpm->directionality() = BETWEEN;
    _tpm->loc2type() = LOCTYPE_NATION;
    _tpm->loc2() = "BR";
    CPPUNIT_ASSERT(_tpmValidator->matchLocation2(*_mRoute, *_tpm));
  }
  void testMachLocation2_Between_MatchDest()
  {
    _tpm->directionality() = BETWEEN;
    _tpm->loc2type() = LOCTYPE_NATION;
    _tpm->loc2() = "US";
    CPPUNIT_ASSERT(_tpmValidator->matchLocation2(*_mRoute, *_tpm));
  }
  void testMachLocation2_Between_NoMatch()
  {
    _tpm->directionality() = BETWEEN;
    _tpm->loc2type() = LOCTYPE_NATION;
    _tpm->loc2() = "AU";
    CPPUNIT_ASSERT(!_tpmValidator->matchLocation2(*_mRoute, *_tpm));
  }
  void testMatchCRSReturnsTrueWhen_NO_PARAM()
  {
    _tpm->userApplType() = NO_PARAM;
    _tpm->userAppl() == EMPTY_STRING();
    CPPUNIT_ASSERT(_tpmValidator->matchCRS(*_mRoute, *_tpm));
  }
  void testMatchCRSReturnsTrueWhen_CRS_Match()
  {
    _tpm->userApplType() = CRS_USER_APPL;
    _tpm->userAppl() = "ABAC";
    _mRoute->crs() = "ABAC";
    CPPUNIT_ASSERT(_tpmValidator->matchCRS(*_mRoute, *_tpm));
  }
  void testMatchCRSReturnsFalseWhen_CRS_NotMatch()
  {
    _tpm->userApplType() = CRS_USER_APPL;
    _tpm->userAppl() = "ABAC";
    _mRoute->crs() = "SABR";
    CPPUNIT_ASSERT(!_tpmValidator->matchCRS(*_mRoute, *_tpm));
  }
  void testMatchMultiHostReturnsTrueWhen_NO_PARAM()
  {
    _tpm->userApplType() = NO_PARAM;
    _tpm->userAppl() == EMPTY_STRING();
    CPPUNIT_ASSERT(_tpmValidator->matchMultiHost(*_mRoute, *_tpm));
  }
  void testMatchMultiHostReturnsTrueWhen_MultiHost_Match()
  {
    _tpm->userApplType() = MULTIHOST_USER_APPL;
    _tpm->userAppl() = "SU";
    _mRoute->multiHost() = "SU";
    CPPUNIT_ASSERT(_tpmValidator->matchMultiHost(*_mRoute, *_tpm));
  }

  void testMatchMultiHostReturnsFalseWhen_MultiHost_NotMatch()
  {
    _tpm->userApplType() = MULTIHOST_USER_APPL;
    _tpm->userAppl() = "SU";
    _mRoute->multiHost() = "F9";
    CPPUNIT_ASSERT(!_tpmValidator->matchMultiHost(*_mRoute, *_tpm));
  }
  void testMatchConsecutive_Asc_NoSec()
  {
    createMileageRouteVect(2, true);
    CPPUNIT_ASSERT(_tpmValidator->matchConsecutive(*_mRoute, true, _sec1, _sec2));
  }
  void testMatchConsecutive_Asc_OneSec()
  {
    createMileageRouteVect(3, true);
    CPPUNIT_ASSERT(_tpmValidator->matchConsecutive(*_mRoute, true, _sec1, _sec2));
  }
  void testMatchConsecutive_Asc_TwoSec()
  {
    createMileageRouteVect(4, true);
    CPPUNIT_ASSERT(!_tpmValidator->matchConsecutive(*_mRoute, true, _sec1, _sec2));
  }
  void testMatchConsecutive_Desc_NoSec()
  {
    createMileageRouteVect(2, false);
    CPPUNIT_ASSERT(_tpmValidator->matchConsecutive(*_mRoute, false, _sec1, _sec2));
  }
  void testMatchConsecutive_Desc_OneSec()
  {
    createMileageRouteVect(3, false);
    CPPUNIT_ASSERT(_tpmValidator->matchConsecutive(*_mRoute, false, _sec1, _sec2));
  }
  void testMatchConsecutive_Desc_TwoSec()
  {
    createMileageRouteVect(4, false);
    CPPUNIT_ASSERT(!_tpmValidator->matchConsecutive(*_mRoute, false, _sec1, _sec2));
  }
  void testMatchConsecMustBeOnGov_CheckNo_S1AA_S2UA()
  {
    loadRouteLHRGRU();
    _tpm->consecMustBeOnGovCxr() = NO;
    _sec2->segmentCarrier() = "UA";
    CPPUNIT_ASSERT(_tpmValidator->matchConsecMustBeOnGov(*_mRoute, *_tpm, true, _sec1, _sec2));
  }
  void testMatchConsecMustBeOnGov_CheckYes_S1AA_S2AA()
  {
    loadRouteLHRGRU();
    _tpm->consecMustBeOnGovCxr() = YES;
    CPPUNIT_ASSERT(_tpmValidator->matchConsecMustBeOnGov(*_mRoute, *_tpm, true, _sec1, _sec2));
  }
  void testMatchConsecMustBeOnGov_CheckYes_S1AA_S2UA()
  {
    loadRouteLHRGRU();
    _tpm->consecMustBeOnGovCxr() = YES;
    _sec2->segmentCarrier() = "UA";
    CPPUNIT_ASSERT(!_tpmValidator->matchConsecMustBeOnGov(*_mRoute, *_tpm, true, _sec1, _sec2));
  }
  void testMatchConsecMustBeOnGov_CheckYes_S1UA_S2AA()
  {
    loadRouteLHRGRU();
    _tpm->consecMustBeOnGovCxr() = YES;
    _sec1->segmentCarrier() = "UA";
    CPPUNIT_ASSERT(!_tpmValidator->matchConsecMustBeOnGov(*_mRoute, *_tpm, true, _sec1, _sec2));
  }
  void testMatchSurface_Permitted()
  {
    loadRouteGRULHR();
    _tpm = TestTPMExclusionFactory::create("data/TPMExclusion_AA_2.xml");
    _tpm->surfacePermitted() = YES;
    CPPUNIT_ASSERT(_tpmValidator->matchSurfacePermitted(*_mRoute, *_tpm, true, _sec1, _sec2));
  }
  void testMatchSurface_NotPermitted()
  {
    loadRouteGRULHR();
    _tpm = TestTPMExclusionFactory::create("data/TPMExclusion_AA_2.xml");
    _tpm->surfacePermitted() = NO;
    CPPUNIT_ASSERT(!_tpmValidator->matchSurfacePermitted(*_mRoute, *_tpm, true, _sec1, _sec2));
    _tpm->surfacePermitted() = YES;
  }
  void testApplySegments()
  { // failed on number of segments
    _mRoute->dataHandle() = (DataHandle*)0xCFCFCFCF;
    CPPUNIT_ASSERT(!_tpmValidator->apply(*_mRoute));
  }

  void testApplyGRULHR()
  { // failed on stopover
    loadRouteGRULHR();
    _mRoute->dataHandle() = (DataHandle*)0xCFCFCFCF;
    CPPUNIT_ASSERT(!_tpmValidator->apply(*_mRoute));
  }
  void testApplyLHRGRU()
  { // pass
    loadRouteLHRGRU();
    _mRoute->dataHandle() = (DataHandle*)0xCFCFCFCF;
    CPPUNIT_ASSERT(_tpmValidator->apply(*_mRoute));
  }
  void testMatchSector1_Directionality_FROM_Match()
  {
    loadRouteLHRGRU();
    _tpm->directionality() = FROM;
    _tpm->sec1Loc1Type() = IATA_AREA;
    _tpm->sec1Loc1() = "2";
    _tpm->sec1Loc2Type() = IATA_AREA;
    _tpm->sec1Loc2() = "1";
    CPPUNIT_ASSERT(_tpmValidator->matchSector1(*_mRoute, *_tpm, _sec1));
  }
  void testMatchSector1_Directionality_FROM_NotMatch()
  {
    loadRouteLHRGRU();
    _tpm->directionality() = FROM;
    _tpm->sec1Loc1Type() = IATA_AREA;
    _tpm->sec1Loc1() = "2";
    _tpm->sec1Loc2Type() = IATA_AREA;
    _tpm->sec1Loc2() = "4";
    CPPUNIT_ASSERT(!_tpmValidator->matchSector1(*_mRoute, *_tpm, _sec1));
  }
  void testMatchSector1_Directionality_FROM_NotMatchOppositeDir()
  {
    loadRouteLHRGRU();
    _tpm->directionality() = FROM;
    _tpm->sec1Loc1Type() = IATA_AREA;
    _tpm->sec1Loc1() = "1";
    _tpm->sec1Loc2Type() = IATA_AREA;
    _tpm->sec1Loc2() = "2";
    CPPUNIT_ASSERT(!_tpmValidator->matchSector1(*_mRoute, *_tpm, _sec1));
  }
  void testMatchSector1_Directionality_BETWEEN_Match()
  {
    loadRouteLHRGRU();
    _tpm->directionality() = BETWEEN;
    _tpm->sec1Loc1Type() = IATA_AREA;
    _tpm->sec1Loc1() = "2";
    _tpm->sec1Loc2Type() = IATA_AREA;
    _tpm->sec1Loc2() = "1";
    CPPUNIT_ASSERT(_tpmValidator->matchSector1(*_mRoute, *_tpm, _sec1));
  }
  void testMatchSector1_Directionality_BETWEEN_NotMatch()
  {
    loadRouteLHRGRU();
    _tpm->directionality() = BETWEEN;
    _tpm->sec1Loc1Type() = IATA_AREA;
    _tpm->sec1Loc1() = "10";
    _tpm->sec1Loc2Type() = IATA_AREA;
    _tpm->sec1Loc2() = "20";
    CPPUNIT_ASSERT(!_tpmValidator->matchSector1(*_mRoute, *_tpm, _sec1));
  }
  void testMatchSector1_Directionality_BETWEEN_MatchOppositeDir()
  {
    loadRouteLHRGRU();
    _tpm->directionality() = BETWEEN;
    _tpm->sec1Loc1Type() = IATA_AREA;
    _tpm->sec1Loc1() = "1";
    _tpm->sec1Loc2Type() = IATA_AREA;
    _tpm->sec1Loc2() = "2";
    CPPUNIT_ASSERT(_tpmValidator->matchSector1(*_mRoute, *_tpm, _sec1));
  }
  void testMatchSector2_Directionality_FROM_Match()
  {
    loadRouteLHRGRU();
    _tpm->directionality() = FROM;
    _tpm->sec2Loc1Type() = IATA_AREA;
    _tpm->sec2Loc1() = "2";
    _tpm->sec2Loc2Type() = IATA_AREA;
    _tpm->sec2Loc2() = "1";
    CPPUNIT_ASSERT(_tpmValidator->matchSector2(*_mRoute, *_tpm, _sec1));
  }
  void testMatchSector2_Directionality_FROM_NotMatch()
  {
    loadRouteLHRGRU();
    _tpm->directionality() = FROM;
    _tpm->sec2Loc1Type() = IATA_AREA;
    _tpm->sec2Loc1() = "2";
    _tpm->sec2Loc2Type() = IATA_AREA;
    _tpm->sec2Loc2() = "4";
    CPPUNIT_ASSERT(!_tpmValidator->matchSector2(*_mRoute, *_tpm, _sec1));
  }
  void testMatchSector2_Directionality_FROM_NotMatchOppositeDir()
  {
    loadRouteLHRGRU();
    _tpm->directionality() = FROM;
    _tpm->sec2Loc1Type() = IATA_AREA;
    _tpm->sec2Loc1() = "1";
    _tpm->sec2Loc2Type() = IATA_AREA;
    _tpm->sec2Loc2() = "2";
    CPPUNIT_ASSERT(!_tpmValidator->matchSector2(*_mRoute, *_tpm, _sec1));
  }
  void testMatchSector2_Directionality_BETWEEN_Match()
  {
    loadRouteLHRGRU();
    _tpm->directionality() = BETWEEN;
    _tpm->sec2Loc1Type() = IATA_AREA;
    _tpm->sec2Loc1() = "2";
    _tpm->sec2Loc2Type() = IATA_AREA;
    _tpm->sec2Loc2() = "1";
    CPPUNIT_ASSERT(_tpmValidator->matchSector2(*_mRoute, *_tpm, _sec1));
  }
  void testMatchSector2_Directionality_BETWEEN_NotMatch()
  {
    loadRouteLHRGRU();
    _tpm->directionality() = BETWEEN;
    _tpm->sec2Loc1Type() = IATA_AREA;
    _tpm->sec2Loc1() = "10";
    _tpm->sec2Loc2Type() = IATA_AREA;
    _tpm->sec2Loc2() = "20";
    CPPUNIT_ASSERT(!_tpmValidator->matchSector2(*_mRoute, *_tpm, _sec1));
  }
  void testMatchSector2_Directionality_BETWEEN_MatchOppositeDir()
  {
    loadRouteLHRGRU();
    _tpm->directionality() = BETWEEN;
    _tpm->sec2Loc1Type() = IATA_AREA;
    _tpm->sec2Loc1() = "1";
    _tpm->sec2Loc2Type() = IATA_AREA;
    _tpm->sec2Loc2() = "2";
    CPPUNIT_ASSERT(_tpmValidator->matchSector2(*_mRoute, *_tpm, _sec1));
  }
  void testMarkSectors()
  {
    createMileageRouteVect(3, true);
    _tpmValidator->markSectors(*_mRoute, true, _sec1, _sec2);
    CPPUNIT_ASSERT(_mRoute->mileageRouteItems()[1].southAtlanticExclusion());
    CPPUNIT_ASSERT(_mRoute->mileageRouteItems()[2].southAtlanticExclusion());
  }
  void testMatchViaPointOfIntrestr_T_Asc_SameLocations()
  {
    createMileageRouteVect(2, true);
    _tpm->viaPointRest() = 'T';
    _sec1->city2() = TestLocFactory::create("data/MileageRoute_Loc_JFK.xml");
    _sec2->city1() = TestLocFactory::create("data/MileageRoute_Loc_JFK.xml");
    CPPUNIT_ASSERT(_tpmValidator->matchViaPointOfIntrestr(*_mRoute, true, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_T_Asc_NotSameLocations()
  {
    createMileageRouteVect(2, true);
    _tpm->viaPointRest() = 'T';
    _sec1->city2() = TestLocFactory::create("data/MileageRoute_Loc_JFK.xml");
    _sec2->city1() = TestLocFactory::create("data/MileageRoute_Loc_GRU.xml");
    CPPUNIT_ASSERT(!_tpmValidator->matchViaPointOfIntrestr(*_mRoute, true, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_T_Desc_SameLocations()
  {
    createMileageRouteVect(2, false);
    _tpm->viaPointRest() = 'T';
    _sec2->city2() = TestLocFactory::create("data/MileageRoute_Loc_JFK.xml");
    _sec1->city1() = TestLocFactory::create("data/MileageRoute_Loc_JFK.xml");
    CPPUNIT_ASSERT(_tpmValidator->matchViaPointOfIntrestr(*_mRoute, false, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_T_Desc_NotSameLocations()
  {
    createMileageRouteVect(2, false);
    _tpm->viaPointRest() = 'T';
    _sec2->city2() = TestLocFactory::create("data/MileageRoute_Loc_JFK.xml");
    _sec1->city1() = TestLocFactory::create("data/MileageRoute_Loc_GRU.xml");
    CPPUNIT_ASSERT(!_tpmValidator->matchViaPointOfIntrestr(*_mRoute, false, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_G_Asc_2Segments()
  {
    createMileageRouteVect(2, true);
    _tpm->viaPointRest() = 'G';
    CPPUNIT_ASSERT(!_tpmValidator->matchViaPointOfIntrestr(*_mRoute, true, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_G_Asc_Surface()
  {
    createMileageRouteVect(3, true);
    _tpm->viaPointRest() = 'G';
    _mRoute->mileageRouteItems()[1].segmentCarrier() = "YY";
    _mRoute->mileageRouteItems()[1].isSurface() = true;
    CPPUNIT_ASSERT(_tpmValidator->matchViaPointOfIntrestr(*_mRoute, true, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_G_Asc_SameCarrier()
  {
    createMileageRouteVect(3, true);
    _tpm->viaPointRest() = 'G';
    _mRoute->mileageRouteItems()[1].segmentCarrier() = "UA";
    CPPUNIT_ASSERT(_tpmValidator->matchViaPointOfIntrestr(*_mRoute, true, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_G_Asc_DiffCarrier()
  {
    createMileageRouteVect(3, true);
    _tpm->viaPointRest() = 'G';
    _mRoute->mileageRouteItems()[1].segmentCarrier() = "LH";
    CPPUNIT_ASSERT(!_tpmValidator->matchViaPointOfIntrestr(*_mRoute, true, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_G_Desc_2Segments()
  {
    createMileageRouteVect(2, false);
    _tpm->viaPointRest() = 'G';
    CPPUNIT_ASSERT(!_tpmValidator->matchViaPointOfIntrestr(*_mRoute, false, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_G_Desc_Surface()
  {
    createMileageRouteVect(3, false);
    _tpm->viaPointRest() = 'G';
    _mRoute->mileageRouteItems()[1].segmentCarrier() = "YY";
    _mRoute->mileageRouteItems()[1].isSurface() = true;
    CPPUNIT_ASSERT(_tpmValidator->matchViaPointOfIntrestr(*_mRoute, false, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_G_Desc_SameCarrier()
  {
    createMileageRouteVect(3, false);
    _tpm->viaPointRest() = 'G';
    _mRoute->mileageRouteItems()[1].segmentCarrier() = "UA";
    CPPUNIT_ASSERT(_tpmValidator->matchViaPointOfIntrestr(*_mRoute, false, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_G_Desc_DiffCarrier()
  {
    createMileageRouteVect(3, false);
    _tpm->viaPointRest() = 'G';
    _mRoute->mileageRouteItems()[1].segmentCarrier() = "LH";
    CPPUNIT_ASSERT(!_tpmValidator->matchViaPointOfIntrestr(*_mRoute, false, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_A_Asc_2Segments()
  {
    createMileageRouteVect(2, true);
    _tpm->viaPointRest() = 'A';
    CPPUNIT_ASSERT(!_tpmValidator->matchViaPointOfIntrestr(*_mRoute, true, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_A_Asc_Surface()
  {
    createMileageRouteVect(3, true);
    _tpm->viaPointRest() = 'A';
    _mRoute->mileageRouteItems()[1].isSurface() = true;
    CPPUNIT_ASSERT(_tpmValidator->matchViaPointOfIntrestr(*_mRoute, true, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_A_Asc_NotSurface()
  {
    createMileageRouteVect(3, true);
    _tpm->viaPointRest() = 'A';
    _mRoute->mileageRouteItems()[1].segmentCarrier() = "LH";
    CPPUNIT_ASSERT(_tpmValidator->matchViaPointOfIntrestr(*_mRoute, true, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_A_Desc_2Segments()
  {
    createMileageRouteVect(2, false);
    _tpm->viaPointRest() = 'A';
    CPPUNIT_ASSERT(!_tpmValidator->matchViaPointOfIntrestr(*_mRoute, false, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_A_Desc_Surface()
  {
    createMileageRouteVect(3, false);
    _tpm->viaPointRest() = 'A';
    _mRoute->mileageRouteItems()[1].isSurface() = true;
    CPPUNIT_ASSERT(_tpmValidator->matchViaPointOfIntrestr(*_mRoute, false, *_tpm, _sec1, _sec2));
  }
  void testMatchViaPointOfIntrestr_A_Desc_NotSurface()
  {
    createMileageRouteVect(3, false);
    _tpm->viaPointRest() = 'A';
    _mRoute->mileageRouteItems()[1].segmentCarrier() = "LH";
    CPPUNIT_ASSERT(_tpmValidator->matchViaPointOfIntrestr(*_mRoute, false, *_tpm, _sec1, _sec2));
  }
  void testMatchSectorAppl_D_Sec1()
  {
    createMileageRouteVect(2, true);
    _tpm->sec1Appl() = 'D';
    CPPUNIT_ASSERT(_tpmValidator->matchSectorAppl(*_mRoute, *_tpm, _sec1, true));
  }
  void testMatchSectorAppl_D_Sec2()
  {
    createMileageRouteVect(2, true);
    _tpm->sec2Appl() = 'D';
    CPPUNIT_ASSERT(_tpmValidator->matchSectorAppl(*_mRoute, *_tpm, _sec2, false));
  }
  void testMatchSectorAppl_N_Sec1_NoHideStop()
  {
    createMileageRouteVect(2, true);
    _tpm->sec1Appl() = 'N';
    CPPUNIT_ASSERT(_tpmValidator->matchSectorAppl(*_mRoute, *_tpm, _sec1, true));
  }
  void testMatchSectorAppl_N_Sec1_HideStop()
  {
    createMileageRouteVect(2, true);
    _tpm->sec1Appl() = 'N';
    _sec1->hiddenLocs().push_back(TestLocFactory::create("data/MileageRoute_Loc_JFK.xml"));
    CPPUNIT_ASSERT(!_tpmValidator->matchSectorAppl(*_mRoute, *_tpm, _sec1, true));
  }
  void testMatchSectorAppl_N_Sec2_NoHideStop()
  {
    createMileageRouteVect(2, true);
    _tpm->sec2Appl() = 'N';
    CPPUNIT_ASSERT(_tpmValidator->matchSectorAppl(*_mRoute, *_tpm, _sec2, false));
  }
  void testMatchSectorAppl_N_Sec2_HideStop()
  {
    createMileageRouteVect(2, true);
    _tpm->sec2Appl() = 'N';
    _sec2->hiddenLocs().push_back(TestLocFactory::create("data/MileageRoute_Loc_JFK.xml"));
    CPPUNIT_ASSERT(!_tpmValidator->matchSectorAppl(*_mRoute, *_tpm, _sec2, false));
  }
  void testMultiCityOrig_MultiTransportOriginNull_CityNotEmpty()
  {
    _mItem->city1() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml", true);
    CPPUNIT_ASSERT_EQUAL(LocCode("JFK"), SouthAtlanticTPMExclusion::multiCityOrig(*_mItem));
  }
  void testMultiCityOrig_MultiTransportOriginNotNull_CityEmpty()
  {
    _mItem->multiTransportOrigin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    CPPUNIT_ASSERT_EQUAL(LocCode("NYC"), SouthAtlanticTPMExclusion::multiCityOrig(*_mItem));
  }
  void testMultiCityOrig_MultiTransportOriginNotNull_CityNotEmpty()
  {
    _mItem->city1() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    _mItem->multiTransportOrigin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    CPPUNIT_ASSERT_EQUAL(LocCode("NYC"), SouthAtlanticTPMExclusion::multiCityOrig(*_mItem));
  }
  void testMultiCityDest_MultiTransportDestinationNull_CityNotEmpty()
  {
    _mItem->city2() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml", true);
    CPPUNIT_ASSERT_EQUAL(LocCode("JFK"), SouthAtlanticTPMExclusion::multiCityDest(*_mItem));
  }
  void testMultiCityDest_MultiTransportDestinationNotNull_CityEmpty()
  {
    _mItem->multiTransportDestination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    CPPUNIT_ASSERT_EQUAL(LocCode("NYC"), SouthAtlanticTPMExclusion::multiCityDest(*_mItem));
  }
  void testMultiCityDest_MultiTransportDestinationNotNull_CityNotEmpty()
  {
    _mItem->city2() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    _mItem->multiTransportDestination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    CPPUNIT_ASSERT_EQUAL(LocCode("NYC"), SouthAtlanticTPMExclusion::multiCityDest(*_mItem));
  }

  void createMileageRouteVect(int items, bool asc)
  {
    _mRoute = _memHandle.create<MileageRoute>();
    _mRoute->governingCarrier() = "UA";
    for (int i = 0; i < items; i++)
      _mRoute->mileageRouteItems().push_back(*_memHandle.create<MileageRouteItem>());
    _sec1 = asc ? _mRoute->mileageRouteItems().begin() : _mRoute->mileageRouteItems().end() - 1;
    _sec2 = asc ? _mRoute->mileageRouteItems().end() - 1 : _mRoute->mileageRouteItems().begin();
  }
  void loadRouteLHRGRU()
  {
    // LHR - BOS - DFW - GRU
    _mRoute = TestMileageRouteFactory::create("data/Mileage_LHRGRU.xml", true);
    _sec1 = _mRoute->mileageRouteItems().begin();
    _sec2 = _mRoute->mileageRouteItems().begin() + 2;
    _sec1->segmentCarrier() = "AA";
    _sec2->segmentCarrier() = "AA";
  }
  void loadRouteGRULHR()
  {
    // GRU - MIA || NYC - LHR
    _mRoute = TestMileageRouteFactory::create("data/Mileage_GRULHR.xml", true);
    _sec1 = _mRoute->mileageRouteItems().begin();
    _sec2 = _mRoute->mileageRouteItems().begin() + 2;
    _sec1->segmentCarrier() = "AA";
    _sec2->segmentCarrier() = "AA";
  }

  void testCloneMileageRouteGRU_NYC_FRA()
  {
    MileageRouteItem mit1, mit2;
    MileageRoute mo, md;
    DataHandle dh;
    std::vector<std::pair<uint8_t, uint8_t> > idxMap;
    mit1.city1() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocGRU.xml");
    mit1.city2() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    mit2.city1() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    mit2.city2() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");

    mo.mileageRouteItems() += mit1, mit2;

    _tpmValidator->cloneMileageRoute(mo, md, idxMap, dh);

    CPPUNIT_ASSERT_EQUAL(size_t(2), md.mileageRouteItems().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), idxMap.size());
    CPPUNIT_ASSERT((idxMap[0].first == 0) && (idxMap[0].second == 0));
    CPPUNIT_ASSERT((idxMap[1].first == 1) && (idxMap[1].second == 1));
  }
  void testCloneMileageRouteGRU_NYC_EWR_FRA()
  {
    MileageRouteItem mit1, mit2, mit3;
    MileageRoute mo, md;
    DataHandle dh;
    std::vector<std::pair<uint8_t, uint8_t> > idxMap;
    mit1.city1() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocGRU.xml");
    mit1.city2() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    mit2.city1() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    mit2.city2() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocEWR.xml");
    mit3.city1() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocEWR.xml");
    mit3.city2() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");

    mo.mileageRouteItems() += mit1, mit2, mit3;

    _tpmValidator->cloneMileageRoute(mo, md, idxMap, dh);

    CPPUNIT_ASSERT_EQUAL(size_t(2), md.mileageRouteItems().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), idxMap.size());
    CPPUNIT_ASSERT((idxMap[0].first == 0) && (idxMap[0].second == 0));
    CPPUNIT_ASSERT((idxMap[1].first == 2) && (idxMap[1].second == 1));
  }
  void testCloneMileageRouteGRU_NYC_EWR_JFK_FRA()
  {
    MileageRouteItem mit1, mit2, mit3, mit4;
    MileageRoute mo, md;
    DataHandle dh;
    std::vector<std::pair<uint8_t, uint8_t> > idxMap;
    mit1.city1() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocGRU.xml");
    mit1.city2() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    mit2.city1() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    mit2.city2() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocEWR.xml");
    mit3.city1() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocEWR.xml");
    mit3.city2() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    mit4.city1() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    mit4.city2() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");

    mo.mileageRouteItems() += mit1, mit2, mit3, mit4;

    _tpmValidator->cloneMileageRoute(mo, md, idxMap, dh);

    CPPUNIT_ASSERT_EQUAL(size_t(2), md.mileageRouteItems().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), idxMap.size());
    CPPUNIT_ASSERT((idxMap[0].first == 0) && (idxMap[0].second == 0));
    CPPUNIT_ASSERT((idxMap[1].first == 3) && (idxMap[1].second == 1));
  }

private:
  TestMemHandle _memHandle;
  const SouthAtlanticTPMExclusionMock* _tpmValidator;
  TPMExclusion* _tpm;
  MileageRoute* _mRoute;
  Diagnostic* _rootDiag;
  Diag452Collector* _diag;
  std::vector<MileageRouteItem>::iterator _sec1;
  std::vector<MileageRouteItem>::iterator _sec2;
  MileageRouteItem* _mItem;
};
CPPUNIT_TEST_SUITE_REGISTRATION(SouthAtlanticTPMExclusionTest);
}
