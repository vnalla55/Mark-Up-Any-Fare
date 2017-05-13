//-----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "Common/ClassOfService.h"
#include "Common/RBDByCabinUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/BookingCodeExceptionSegment.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/RBDByCabinInfo.h"
#include "DBAccess/ZoneInfo.h"
#include "Diagnostic/Diag187Collector.h"

using namespace std;

namespace tse
{

namespace
{
class RBDByCabinUtilTestMock : public RBDByCabinUtil
{
public:
  RBDByCabinUtilTestMock(PricingTrx& trx, tse::RBDByCabinCall call, Diag187Collector* diag)
  : RBDByCabinUtil(trx, call, diag)
  {}

  const vector<RBDByCabinInfo*>& getRBDByCabin(const VendorCode& vendor,
                                               const CarrierCode& carrier,
                                               DateTime& tvlDate)
  {
    return _cabinInfos;

  }
  bool
  isInLoc(const VendorCode& vendor, const LocKey& locKey, const Loc& loc, CarrierCode carrier)
  {
    if (vendor == SITA_VENDOR_CODE)
    {
      return locKey.loc() == loc.loc();
    }
    else
      return !isdigit(loc.loc()[0]) && !isdigit(locKey.loc()[0]);
  }

  bool
  isInZone(const VendorCode& vendor, const LocCode& zone, const Loc& loc, CarrierCode carrier)
  {
    return isdigit(zone[0]);
  }

vector<class RBDByCabinInfo*> _cabinInfos;
};
} // anon NS

class RBDByCabinUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RBDByCabinUtilTest);

  CPPUNIT_TEST(testSetsUpSegInds);
  CPPUNIT_TEST(testPrintRBDByCabin);
  CPPUNIT_TEST(testPrintRBDByCabinWithFlt1);
  CPPUNIT_TEST(testPrintRBDByCabinWithFlt1Flt2);
  CPPUNIT_TEST(testPrintRBDByCabinWithFlt1Flt2Equipment);
  CPPUNIT_TEST(testPrintRBDByCabinWithFlt1Equipment);
  CPPUNIT_TEST(testPrintRBDByCabin_CityPair);
  CPPUNIT_TEST(testInvoke187Diagnostic_ALL_Carriers);
  CPPUNIT_TEST(testInvoke187Diagnostic_Selected_Carrier);
  CPPUNIT_TEST(testInvoke187Diagnostic_Selected_Carrier_And_CityPair);
  CPPUNIT_TEST(testCheckTravelDate_PASS);
  CPPUNIT_TEST(testCheckTravelDate_Fail_NoData);
  CPPUNIT_TEST(testCheckTicketDate_PASS);
  CPPUNIT_TEST(testCheckTicketDate_Fail);

  CPPUNIT_TEST(testCheckGlobalInd_GD_BLANK_PASS);
  CPPUNIT_TEST(testCheckGlobalInd_NotMatch_Fail);
  CPPUNIT_TEST(testCheckGlobalInd_Match_Pass);
  CPPUNIT_TEST(testCheckGlobalInd_NO_DIR_Match_Pass);

  CPPUNIT_TEST(testCheckLocations_Both_Locs_Blank_Pass);
  CPPUNIT_TEST(testCheckLocations_Loc1_Blank_Loc2_Coded_Fail);

  CPPUNIT_TEST(testCheckFlights_No_Flights_Pass);
  CPPUNIT_TEST(testCheckFlights_Flight_2_Only_Fail);
  CPPUNIT_TEST(testCheckFlights_Flight_1_Only_Not_Match_Fail);
  CPPUNIT_TEST(testCheckFlights_Flight_1_Only_Match_Pass);
  CPPUNIT_TEST(testCheckFlights_FlightRange_Match_Pass);
  CPPUNIT_TEST(testCheckFlights_FlightRange_Flight_Low_Fail);
  CPPUNIT_TEST(testCheckFlights_FlightRange_Flight_High_Fail);
  CPPUNIT_TEST(testCheckFlights_FlightRange_Flight1_Higher_Flight2_Fail);

  CPPUNIT_TEST(testCheckEquipment_Info_Empty_Pass);
  CPPUNIT_TEST(testCheckEquipment_Info_Has_Equipment_Match_Pass);
  CPPUNIT_TEST(testCheckEquipment_Info_Has_Equipment_No_Match_Fail);

  CPPUNIT_TEST(testSetsUpGeoTravelType_Domestic_Domestic);
  CPPUNIT_TEST(testSetsUpGeoTravelType_Domestic_TransBorder);
  CPPUNIT_TEST(testSetsUpGeoTravelType_International_International);
  CPPUNIT_TEST(testSetsUpGeoTravelType_International_ForeignDomestic);
  CPPUNIT_TEST(testSetsUpGeoTravelType_International_UnknownGeoTravelType);
  CPPUNIT_TEST(testSetsUpGeoTravelType_FM_UnknownGeoTravelType);

  CPPUNIT_TEST(testValidateLocation_origin_vs_RbdInfo_Pass);
  CPPUNIT_TEST(testValidateLocation_destination_vs_RbdInfo_Fail);
  CPPUNIT_TEST(testValidateLoc1_From_To_Loc1_Pass);
  CPPUNIT_TEST(testValidateLoc1_From_To_Fail);
  CPPUNIT_TEST(testValidateBothLocs_Fail);
  CPPUNIT_TEST(testValidateBothLocs_Pass);
  CPPUNIT_TEST(testValidateLocation_origin_vs_RbdInfo_Zone_Pass);

  CPPUNIT_TEST(testPrepareCabin_Blank);
  CPPUNIT_TEST(testPrepareCabin_Not_Blank);

  CPPUNIT_TEST(testGetRBDCabin_Not_Found_Return_BLANK);
  CPPUNIT_TEST(testGetRBDCabin_Match_Found);

  CPPUNIT_TEST(testValidateRBDByCabin_Match_RBD);
  CPPUNIT_TEST(testValidateRBDByCabin_Not_Match_RBD_NOT_YY_ReturnZero);
  CPPUNIT_TEST(testValidateRBDByCabin_Not_Match_RBD_Ancil_YY_ReturnEconomy);
  CPPUNIT_TEST(testValidateRBDByCabin_Not_Match_RBD_Not_Ancil_YY_ReturnEconomy);

  CPPUNIT_TEST(testFindCabinByRBD_DB_EMPTY);
  CPPUNIT_TEST(testGetCabinByRbdByType_InvalidClass);
  CPPUNIT_TEST(testGetCabinByRbdByType_InvalidClass_SegmentOpen);

  CPPUNIT_TEST(testGetCabinByRBD_InvalidClass_SegmentOpen);
  CPPUNIT_TEST(testGetCabinByRBD_InvalidClass_Air_Segment);

  CPPUNIT_TEST(testGetCabinByRBD);
  CPPUNIT_TEST(testGetCabinsByRbd_Ptf_InvalidClass);

  CPPUNIT_TEST(testGetCabinForAirseg_InvalidClass);
  CPPUNIT_TEST(testGetCabinByRBD_InvalidClass);
  CPPUNIT_TEST(testGetCabinsByRbd_InvalidClass);
  CPPUNIT_TEST(testGetCabinsByRbd_Cabin);
  CPPUNIT_TEST(testGetCabinsByRbd_Atae_Cabin);
  CPPUNIT_TEST(testIsProgramCallSelected_Empty);
  CPPUNIT_TEST(testIsProgramCallSelected_Not_Match);
  CPPUNIT_TEST(testIsProgramCallSelected_Match);

  CPPUNIT_TEST(testIsCpaSelected_Empty);
  CPPUNIT_TEST(testIsCpaSelected_Not_Match);
  CPPUNIT_TEST(testIsCpaSelected_Match);
  CPPUNIT_TEST(testIsCpaSelected_FM_Not_Match);
  CPPUNIT_TEST(testIsCpaSelected_FM_Match);

  CPPUNIT_TEST(testIsCabinSelected_FAIL);
  CPPUNIT_TEST(testIsCabinSelected_PASS);
  CPPUNIT_TEST(testIsSequenceSelected_Empty);
  CPPUNIT_TEST(testIsSequenceSelected_Match);
  CPPUNIT_TEST(testIsSequenceSelected_Not_Match);
  CPPUNIT_TEST(testIsCxrSelected_Empty);
  CPPUNIT_TEST(testIsCxrSelected_Request_Match);
  CPPUNIT_TEST(testIsCxrSelected_Request_Not_Match);
  CPPUNIT_TEST(testIsCxrSelected_AirSeg_Match);
  CPPUNIT_TEST(testIsCxrSelected_AirSeg_Not_Match);
  CPPUNIT_TEST(testIsCxrSelected_FM_Match);
  CPPUNIT_TEST(testIsCxrSelected_FM_Not_Match);
  CPPUNIT_TEST(testIsBookingCodeSelected_Empty);
  CPPUNIT_TEST(testIsBookingCodeSelected_Request_Match);
  CPPUNIT_TEST(testIsBookingCodeSelected_Request_Not_Match);

  CPPUNIT_TEST(testPrintHeader);
  CPPUNIT_TEST(testPrintRecordHeader_Short);
  CPPUNIT_TEST(testPrintRecordHeader);
  CPPUNIT_TEST(testPrintCurrentRBD);
  CPPUNIT_TEST(testPrintRbdByCabinInfo);
  CPPUNIT_TEST(testPrintRbdByCabinInfo_Short);
  CPPUNIT_TEST(testPrintReqCabinNotMatch);
  CPPUNIT_TEST(testPrintReqCabinNotMatch_NothingToPrint);
  CPPUNIT_TEST(testPrintCabinNotFoundForRBD);
  CPPUNIT_TEST(testPrintCabinNotFoundForRBD_NothingToPrint);
  CPPUNIT_TEST(testPrintDefaultEconomyCabin);
  CPPUNIT_TEST(testPrintDefaultEconomyCabin_NothingToPrint);
  CPPUNIT_TEST(testPrintCabinFoundForRBD);
  CPPUNIT_TEST(testPrintCabinFoundForRBD_Short);
  CPPUNIT_TEST(testPrintFailFoundCabinForRBD);
  CPPUNIT_TEST(testPrintFailFoundCabinForRBD_NothingToPrint);
  CPPUNIT_TEST(testPrintRBDByCabinNotFoundForCarrier);
  CPPUNIT_TEST(testPrintFailStatus_Short);
  CPPUNIT_TEST(testPrintFailStatus);
  CPPUNIT_TEST(testPrintFailStatus_NothingToPrint);

  CPPUNIT_TEST(testPrintAirSegSearchData_AirSeg);
  CPPUNIT_TEST(testPrintAirSegSearchData);
  CPPUNIT_TEST(testPrintAirSegSearchData_Cxr_Rbd);
  CPPUNIT_TEST(testPrintAirSegSearchData_Rbd_List);
  CPPUNIT_TEST(testPrintAirSegSearchData_Cos_List);
  CPPUNIT_TEST(testPrintAirSegSearchData_Ptf);
  CPPUNIT_TEST(testPrintAirSegSearchData_Ptf_Bce);
  CPPUNIT_TEST(testSkipSequenceFlt1AndEquipNotCoded);
  CPPUNIT_TEST(testSkipSequenceFlt1Coded);
  CPPUNIT_TEST(testSkipSequenceEquipCoded);
  CPPUNIT_TEST(testSkipSequenceFlt1AndEquipBothCoded);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->dataHandle().setTicketDate(DateTime(2015, 9, 19));
    _diag = _memHandle.create<Diag187Collector>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _airSeg = _memHandle.create<AirSeg>();
    _trx->travelSeg().push_back(_airSeg);

    _cabinUtil = _memHandle.create<RBDByCabinUtil>(*_trx, _call, _diag);
    _cabinUtilM = _memHandle.insert(new RBDByCabinUtilTestMock(*_trx, _call, _diag));

    _locDFW = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _locLON = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");

    _airSeg->origin() = _locDFW;
    _airSeg->origAirport() = _locDFW->loc();
    _airSeg->boardMultiCity() = _locDFW->loc();
    _airSeg->origAirport() = _locDFW->loc();

    _airSeg->destination() = _locLON;
    _airSeg->destAirport() = _locLON->loc();
    _airSeg->offMultiCity()= _locLON->loc();
    _airSeg->destAirport() = _locLON->loc();

    _airSeg->departureDT() = DateTime(2015, 9, 19);
    _airSeg->carrier() = "AA";
    _airSeg->setOperatingCarrierCode("AA");
    _airSeg->setMarketingCarrierCode("AA");
    _fm = _memHandle.create<FareMarket>();
    _fm->travelDate() = DateTime(2015, 9, 19);
    _ptf = _memHandle.create<PaxTypeFare>();
    _ptf->fareMarket() = _fm;
    _trx->ticketingDate() = DateTime(2015, 7, 12);
  }

  enum DiagParam : uint8_t
  { CITY_PAIR = 1,
    CARRIER,
    CABIN,
    SEQUENCE,
    RBD,
    PTF,
    DDINFO,
    DDPASSED,
    PRG_CALL};

  void tearDown()
  {
    _memHandle.clear();
  }

  void testSetsUpSegInds()
  {
    _cabinUtil->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT( _cabinUtil->_segmentVSfareComp == true);
  }

  void testPrintRBDByCabin()
  {
    createActivateDiagnostic();
    CarrierCode carrier = "AA";
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtilM->_cabinInfos.push_back(&info);
    _cabinUtilM->printRBDByCabin(carrier);

    std::stringstream expectedDiag;
    expectedDiag
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1:         EQUIPMENT: \n"
<< " FLIGHT 2: \n"
<< " GEO LOC1: C DFW \n"
<< " GEO LOC2: C FRA \n"
<< "  \n"
<< " CABIN: W - PREMIUM ECONOMY CABIN \n"
<< "   7        A1 B2 C3 D4 E5 \n"
<< "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }

  void testPrintRBDByCabinWithFlt1()
  {
    createActivateDiagnostic();
    CarrierCode carrier = "AA";
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    info.flightNo1() = 323;
    _cabinUtilM->_cabinInfos.push_back(&info);
    _cabinUtilM->printRBDByCabin(carrier);

    std::stringstream expectedDiag;
    expectedDiag
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1: 323     EQUIPMENT: \n"
<< " FLIGHT 2: \n"
<< " GEO LOC1: C DFW \n"
<< " GEO LOC2: C FRA \n"
<< "  \n"
<< " CABIN: W - PREMIUM ECONOMY CABIN \n"
<< "   7        A1 B2 C3 D4 E5 \n"
<< "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }

  void testPrintRBDByCabinWithFlt1Flt2()
  {
    createActivateDiagnostic();
    CarrierCode carrier = "AA";
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    info.flightNo1() = 323;
    info.flightNo2() = 700;
    _cabinUtilM->_cabinInfos.push_back(&info);
    _cabinUtilM->printRBDByCabin(carrier);

    std::stringstream expectedDiag;
    expectedDiag
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1: 323     EQUIPMENT: \n"
<< " FLIGHT 2: 700 \n"
<< " GEO LOC1: C DFW \n"
<< " GEO LOC2: C FRA \n"
<< "  \n"
<< " CABIN: W - PREMIUM ECONOMY CABIN \n"
<< "   7        A1 B2 C3 D4 E5 \n"
<< "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }

  void testPrintRBDByCabinWithFlt1Flt2Equipment()
  {
    createActivateDiagnostic();
    CarrierCode carrier = "AA";
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    info.flightNo1() = 323;
    info.flightNo2() = 700;
    info.equipmentType() = "777";
    _cabinUtilM->_cabinInfos.push_back(&info);
    _cabinUtilM->printRBDByCabin(carrier);

    std::stringstream expectedDiag;
    expectedDiag
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1: 323     EQUIPMENT: 777\n"
<< " FLIGHT 2: 700 \n"
<< " GEO LOC1: C DFW \n"
<< " GEO LOC2: C FRA \n"
<< "  \n"
<< " CABIN: W - PREMIUM ECONOMY CABIN \n"
<< "   7        A1 B2 C3 D4 E5 \n"
<< "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }

  void testPrintRBDByCabinWithFlt1Equipment()
  {
    createActivateDiagnostic();
    CarrierCode carrier = "AA";
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    info.flightNo1() = 323;
    info.equipmentType() = "777";
    _cabinUtilM->_cabinInfos.push_back(&info);
    _cabinUtilM->printRBDByCabin(carrier);

    std::stringstream expectedDiag;
    expectedDiag
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1: 323     EQUIPMENT: 777\n"
<< " FLIGHT 2: \n"
<< " GEO LOC1: C DFW \n"
<< " GEO LOC2: C FRA \n"
<< "  \n"
<< " CABIN: W - PREMIUM ECONOMY CABIN \n"
<< "   7        A1 B2 C3 D4 E5 \n"
<< "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }

  void testPrintRBDByCabin_CityPair()
  {
    createActivateDiagnostic();
    createDiagnosticParams(CITY_PAIR);
    CarrierCode carrier = "AA";
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);

    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtilM->_cabinInfos.push_back(&info);

    _cabinUtilM->printRBDByCabin(carrier);

    std::stringstream expectedDiag;
    expectedDiag
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1:         EQUIPMENT: \n"
<< " FLIGHT 2: \n"
<< " GEO LOC1: C DFW \n"
<< " GEO LOC2: C FRA \n"
<< "  \n"
<< " CABIN: W - PREMIUM ECONOMY CABIN \n"
<< "   7        A1 B2 C3 D4 E5 \n"
<< "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }

  void testInvoke187Diagnostic_ALL_Carriers()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtilM->_cabinInfos.push_back(&info);

    createActivateDiagnostic();
    _cabinUtilM->invoke187Diagnostic();

    std::stringstream expectedDiag;
    expectedDiag
<< "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
<< "*-----------------------------------------------------------*\n"
<< " RBD BY CABIN FOR CARRIER: ALL\n"
<< " TRANSACTION DATE: 19SEP2015 \n"
<< "*-----------------------------------------------------------*\n"
<< " AVAILABLE QUALIFIERS:\n"
<< " VN - VENDOR  CX - CARRIER    SQ - SEQUENCE\n"
<< " CB - CABIN   CP - CITY PAIR  DDZONE - ZONE DETAILED INFO\n"
<< "*************************************************************\n"
<< " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n"
<< "                              DISC          LAST\n"
<< "-------------------------------------------------------------\n"
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1:         EQUIPMENT: \n"
<< " FLIGHT 2: \n"
<< " GEO LOC1: C DFW \n"
<< " GEO LOC2: C FRA \n"
<< "  \n"
<< " CABIN: W - PREMIUM ECONOMY CABIN \n"
<< "   7        A1 B2 C3 D4 E5 \n"
<< "-------------------------------------------------------------\n"
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1:         EQUIPMENT: \n"
<< " FLIGHT 2: \n"
<< " GEO LOC1: C DFW \n"
<< " GEO LOC2: C FRA \n"
<< "  \n"
<< " CABIN: W - PREMIUM ECONOMY CABIN \n"
<< "   7        A1 B2 C3 D4 E5 \n"
<< "-------------------------------------------------------------\n"
<< "  \n"
<< "*********************** END   DIAG 187 ***********************\n";
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }

  void testInvoke187Diagnostic_Selected_Carrier()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtilM->_cabinInfos.push_back(&info);

    createActivateDiagnostic();
    createDiagnosticParams(CARRIER);
    _cabinUtilM->invoke187Diagnostic();

    std::stringstream expectedDiag;
    expectedDiag
<< "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
<< "*-----------------------------------------------------------*\n"
<< " RBD BY CABIN FOR CARRIER: AA \n"
<< " TRANSACTION DATE: 19SEP2015 \n"
<< "*-----------------------------------------------------------*\n"
<< " AVAILABLE QUALIFIERS:\n"
<< " VN - VENDOR  CX - CARRIER    SQ - SEQUENCE\n"
<< " CB - CABIN   CP - CITY PAIR  DDZONE - ZONE DETAILED INFO\n"
<< "*************************************************************\n"
<< " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n"
<< "                              DISC          LAST\n"
<< "-------------------------------------------------------------\n"
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1:         EQUIPMENT: \n"
<< " FLIGHT 2: \n"
<< " GEO LOC1: C DFW \n"
<< " GEO LOC2: C FRA \n"
<< "  \n"
<< " CABIN: W - PREMIUM ECONOMY CABIN \n"
<< "   7        A1 B2 C3 D4 E5 \n"
<< "-------------------------------------------------------------\n"
<< "  \n"
<< "*********************** END   DIAG 187 ***********************\n";
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }

  void testInvoke187Diagnostic_Selected_Carrier_And_CityPair()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);

    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtilM->_cabinInfos.push_back(&info);

    createActivateDiagnostic();
    createDiagnosticParams(CITY_PAIR);
    createDiagnosticParams(CARRIER);
    _cabinUtilM->invoke187Diagnostic();

    std::stringstream expectedDiag;
    expectedDiag
<< "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
<< "*-----------------------------------------------------------*\n"
<< " RBD BY CABIN FOR CARRIER: AA   CITY PAIR: DFW-LON\n"
<< " TRANSACTION DATE: 19SEP2015 \n"
<< "*-----------------------------------------------------------*\n"
<< " AVAILABLE QUALIFIERS:\n"
<< " VN - VENDOR  CX - CARRIER    SQ - SEQUENCE\n"
<< " CB - CABIN   CP - CITY PAIR  DDZONE - ZONE DETAILED INFO\n"
<< "*************************************************************\n"
<< " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n"
<< "                              DISC          LAST\n"
<< "-------------------------------------------------------------\n"
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1:         EQUIPMENT: \n"
<< " FLIGHT 2: \n"
<< " GEO LOC1: C DFW \n"
<< " GEO LOC2: C FRA \n"
<< "  \n"
<< " CABIN: W - PREMIUM ECONOMY CABIN \n"
<< "   7        A1 B2 C3 D4 E5 \n"
<< "-------------------------------------------------------------\n"
<< "  \n"
<< "*********************** END   DIAG 187 ***********************\n";
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }

  void createActivateDiagnostic()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic187;
    _trx->diagnostic().activate();
    _cabinUtilM->_diag = _memHandle.insert(new Diag187Collector(_trx->diagnostic()));
    _cabinUtilM->_diag->activate();
  }

  void createDiagnosticParams(DiagParam param)
  {
    switch (param)
    {
    case CITY_PAIR:
      _trx->diagnostic().diagParamMap().insert(std::make_pair("CP", "DFWLON"));
      break;
    case CARRIER:
      _trx->diagnostic().diagParamMap().insert(std::make_pair("CX", "AA"));
      break;
    case CABIN:
      _trx->diagnostic().diagParamMap().insert(std::make_pair("CB", "W"));
      break;
    case SEQUENCE:
      _trx->diagnostic().diagParamMap().insert(std::make_pair("SQ", "1212121"));
      break;
    case RBD:
      _trx->diagnostic().diagParamMap().insert(std::make_pair("BK", "C3"));
      break;
    case PTF:
      _trx->diagnostic().diagParamMap().insert(std::make_pair("FC", "Y26"));
      break;
    case DDINFO:
      _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "INFO"));
      break;
    case DDPASSED:
      _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "PASSED"));
      break;
    case PRG_CALL:
      _trx->diagnostic().diagParamMap().insert(std::make_pair("PC", "SHPFAMILY"));
      break;
    default:
      break;
  }
}

  void populateLocs(LocKey& lk1, LocKey& lk2)
  {
    lk1.loc() = "DFW";
    lk1.locType() = 'C';
    lk2.loc() = "FRA";
    lk2.locType() = 'C';
  }

  void
  prepareRBDByCabinInfo(RBDByCabinInfo& info, LocKey& lk1, LocKey& lk2)
  {
    info.vendor() = "ATP";
    info.carrier() = "AA";
    info.sequenceNo() = 111233;
    info.globalDir() = "AT";
    info.effDate() = DateTime(2010, 1, 1);
    info.discDate() = DateTime(2025, 12, 31);
    info.createDate() = DateTime(2010, 1, 1);
    info.expireDate() = DateTime(2025, 12, 31);
    info.locKey1() = lk1;
    info.locKey2() = lk2;
    info.firstTicketDate() = DateTime(2010, 1, 2);
    info.lastTicketDate() = DateTime(2025, 12, 31);
    info.flightNo1() = 0;
    info.flightNo2() = 0;
    info.equipmentType() = "";
    info.bookingCodeCabinMap().insert(std::make_pair("A1",'W'));
    info.bookingCodeCabinMap().insert(std::make_pair("B2",'W'));
    info.bookingCodeCabinMap().insert(std::make_pair("C3",'W'));
    info.bookingCodeCabinMap().insert(std::make_pair("D4",'W'));
    info.bookingCodeCabinMap().insert(std::make_pair("E5",'W'));
  }

  void testCheckTravelDate_PASS()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtil->setsUpSegInds(*_airSeg);

    CPPUNIT_ASSERT(_cabinUtil->checkTravelDate(info, _airSeg->departureDT()));
  }

  void testCheckTravelDate_Fail_NoData()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    info.discDate() = DateTime::emptyDate();
    info.effDate() = DateTime::emptyDate();
    _cabinUtil->setsUpSegInds(*_airSeg);

    CPPUNIT_ASSERT(!_cabinUtil->checkTravelDate(info, _airSeg->departureDT()));
  }

  void testCheckTicketDate_PASS()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);

    CPPUNIT_ASSERT(_cabinUtil->checkTicketDate(info, _trx->ticketingDate()));
  }

  void testCheckTicketDate_Fail()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    info.lastTicketDate() = DateTime(2015, 7, 11);

    CPPUNIT_ASSERT(!_cabinUtil->checkTicketDate(info, _trx->ticketingDate()));
  }

  void testCheckGlobalInd_GD_BLANK_PASS()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    info.globalDir() = "";
    _cabinUtil->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(_cabinUtil->checkGlobalInd(info));
  }

  void testCheckGlobalInd_NotMatch_Fail()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    _airSeg->globalDirection() = GlobalDirection::TS;
    _cabinUtil->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(!_cabinUtil->checkGlobalInd(info));
  }

  void testCheckGlobalInd_Match_Pass()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    _airSeg->globalDirection() = GlobalDirection::AT;
    _cabinUtil->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(_cabinUtil->checkGlobalInd(info));
  }

  void testCheckGlobalInd_NO_DIR_Match_Pass()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtil->setsUpSegInds(*_airSeg);

    CPPUNIT_ASSERT(_cabinUtil->checkGlobalInd(info));
  }

  void testCheckLocations_Both_Locs_Blank_Pass()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtil->setsUpSegInds(*_airSeg);
    _cabinUtil->setsUpGeoTravelType();

    CPPUNIT_ASSERT(_cabinUtil->checkLocations(info));
  }

  void testCheckLocations_Loc1_Blank_Loc2_Coded_Fail()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    lk1.loc() = "";
    lk1.locType() = RBDByCabinUtil::CHAR_BLANK;
    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtil->setsUpSegInds(*_airSeg);
    _cabinUtil->setsUpGeoTravelType();

    CPPUNIT_ASSERT(!_cabinUtil->checkLocations(info));
  }

  void testCheckFlights_No_Flights_Pass()
  {
    RBDByCabinInfo info;
    CPPUNIT_ASSERT(_cabinUtil->checkFlights(info));
  }

  void testCheckFlights_Flight_2_Only_Fail()
  {
    RBDByCabinInfo info;
    info.flightNo2() = 200;
    CPPUNIT_ASSERT(!_cabinUtil->checkFlights(info));
  }

  void testCheckFlights_Flight_1_Only_Not_Match_Fail()
  {
    RBDByCabinInfo info;
    info.flightNo1() = 200;
    _airSeg->flightNumber() = 201;
    _cabinUtil->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(!_cabinUtil->checkFlights(info));
  }

  void testCheckFlights_Flight_1_Only_Match_Pass()
  {
    RBDByCabinInfo info;
    info.flightNo1() = 200;
    _airSeg->flightNumber() = 200;
    _cabinUtil->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(_cabinUtil->checkFlights(info));
  }

  void testCheckFlights_FlightRange_Match_Pass()
  {
    RBDByCabinInfo info;
    info.flightNo1() = 200;
    info.flightNo2() = 300;
    _airSeg->flightNumber() = 200;
    _cabinUtil->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(_cabinUtil->checkFlights(info));
  }

  void testCheckFlights_FlightRange_Flight_Low_Fail()
  {
    RBDByCabinInfo info;
    info.flightNo1() = 200;
    info.flightNo2() = 300;
    _airSeg->flightNumber() = 199;
    _cabinUtil->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(!_cabinUtil->checkFlights(info));
  }

  void testCheckFlights_FlightRange_Flight_High_Fail()
  {
    RBDByCabinInfo info;
    info.flightNo1() = 200;
    info.flightNo2() = 300;
    _airSeg->flightNumber() = 301;
    _cabinUtil->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(!_cabinUtil->checkFlights(info));
  }

  void testCheckFlights_FlightRange_Flight1_Higher_Flight2_Fail()
  {
    RBDByCabinInfo info;
    info.flightNo1() = 300;
    info.flightNo2() = 200;
    _airSeg->flightNumber() = 301;
    _cabinUtil->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(!_cabinUtil->checkFlights(info));
  }

  void testCheckEquipment_Info_Empty_Pass()
  {
    RBDByCabinInfo info;
    _cabinUtil->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(_cabinUtil->checkEquipment(info));
  }

  void testCheckEquipment_Info_Has_Equipment_Match_Pass()
  {
    RBDByCabinInfo info;
    info.equipmentType() = "300";
    _airSeg->equipmentType() = "300";
    _cabinUtil->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(_cabinUtil->checkEquipment(info));
  }

  void testCheckEquipment_Info_Has_Equipment_No_Match_Fail()
  {
    RBDByCabinInfo info;
    info.equipmentType() = "300";
    _cabinUtil->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(!_cabinUtil->checkEquipment(info));
  }


  void testSetsUpGeoTravelType_Domestic_Domestic()
  {
    _cabinUtil->setsUpSegInds(*_airSeg);
    _airSeg->geoTravelType() = GeoTravelType::Domestic;

    CPPUNIT_ASSERT(GeoTravelType::Domestic == _cabinUtil->setsUpGeoTravelType());
  }

  void testSetsUpGeoTravelType_Domestic_TransBorder()
  {
    _cabinUtil->setsUpSegInds(*_airSeg);
    _airSeg->geoTravelType() = GeoTravelType::Transborder;

    CPPUNIT_ASSERT(GeoTravelType::Domestic == _cabinUtil->setsUpGeoTravelType());
  }

  void testSetsUpGeoTravelType_International_International()
  {
    _cabinUtil->setsUpSegInds(*_airSeg);
    _airSeg->geoTravelType() = GeoTravelType::International;

    CPPUNIT_ASSERT(GeoTravelType::International == _cabinUtil->setsUpGeoTravelType());
  }

  void testSetsUpGeoTravelType_International_ForeignDomestic()
  {
    _cabinUtil->setsUpSegInds(*_airSeg);
    _airSeg->geoTravelType() = GeoTravelType::ForeignDomestic;

    CPPUNIT_ASSERT(GeoTravelType::International == _cabinUtil->setsUpGeoTravelType());
  }

  void testSetsUpGeoTravelType_International_UnknownGeoTravelType()
  {
    _cabinUtil->setsUpSegInds(*_airSeg);
    _airSeg->geoTravelType() = GeoTravelType::UnknownGeoTravelType;

    CPPUNIT_ASSERT(GeoTravelType::International == _cabinUtil->setsUpGeoTravelType());
  }

  void testSetsUpGeoTravelType_FM_UnknownGeoTravelType()
  {
    _cabinUtil->setsUpFMInds(*_ptf);

    CPPUNIT_ASSERT(GeoTravelType::UnknownGeoTravelType == _cabinUtil->setsUpGeoTravelType());
  }

  void testValidateLocation_LocNull_Fail()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    lk1.loc() = "";
    prepareRBDByCabinInfo(info, lk1, lk2);

    CPPUNIT_ASSERT(!_cabinUtil->validateLocation(info.vendor(), lk1, *(_airSeg->origin()), info.carrier()));
  }

  void testValidateLocation_origin_vs_RbdInfo_Pass()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    _airSeg->geoTravelType() = GeoTravelType::International;

    prepareRBDByCabinInfo(info, lk1, lk2);

    CPPUNIT_ASSERT(_cabinUtil->validateLocation(info.vendor(), info.locKey1(),
                                                *(_airSeg->origin()), info.carrier()));
  }

  void testValidateLocation_destination_vs_RbdInfo_Fail()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    _airSeg->geoTravelType() = GeoTravelType::International;

    prepareRBDByCabinInfo(info, lk1, lk2);

    CPPUNIT_ASSERT(!_cabinUtil->validateLocation(info.vendor(), info.locKey2(),
                                                *(_airSeg->origin()), info.carrier()));
  }

  void testValidateLoc1_From_To_Loc1_Pass()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    _airSeg->geoTravelType() = GeoTravelType::International;
    lk2.loc() = "";
    lk2.locType() = RBDByCabinUtil::CHAR_BLANK;
    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtil->setsUpSegInds(*_airSeg);
    _cabinUtil->setsUpGeoTravelType();

    CPPUNIT_ASSERT(_cabinUtil->checkLocations(info));
  }

  void testValidateLoc1_From_To_Fail()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    _airSeg->geoTravelType() = GeoTravelType::International;
    lk1.loc() = "FRA";
    lk2.loc() = "";
    lk2.locType() = RBDByCabinUtil::CHAR_BLANK;
    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtil->setsUpSegInds(*_airSeg);
    _cabinUtil->setsUpGeoTravelType();

    CPPUNIT_ASSERT(!_cabinUtil->checkLocations(info));
  }

  void testValidateBothLocs_Fail()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    _airSeg->geoTravelType() = GeoTravelType::International;
    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtil->setsUpSegInds(*_airSeg);
    _cabinUtil->setsUpGeoTravelType();

    CPPUNIT_ASSERT(!_cabinUtil->checkLocations(info));
  }

  void testValidateBothLocs_Pass()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    lk2.loc() = "LON";
    _airSeg->geoTravelType() = GeoTravelType::International;
    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtil->setsUpSegInds(*_airSeg);
    _cabinUtil->setsUpGeoTravelType();

    CPPUNIT_ASSERT(_cabinUtil->checkLocations(info));
  }

  void testValidateLocation_origin_vs_RbdInfo_Zone_Pass()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    _airSeg->geoTravelType() = GeoTravelType::International;
    lk1.loc() = "100";
    lk1.locType() = 'U';

    prepareRBDByCabinInfo(info, lk1, lk2);

    CPPUNIT_ASSERT(_cabinUtilM->validateLocation(info.vendor(), info.locKey1(),
                                                *(_airSeg->origin()), info.carrier()));
  }

  void testPrepareCabin_Blank()
  {
    BookingCode bkg = "CH";
    Indicator cab = RBDByCabinUtil::CHAR_BLANK;
    const Cabin* cabin = _cabinUtilM->prepareCabin(bkg, cab);
    CPPUNIT_ASSERT(cabin->cabin().isEconomyClass());
  }

  void testPrepareCabin_Not_Blank()
  {
    BookingCode bkg = "CH";
    Indicator cab = 'F';
    const Cabin* cabin = _cabinUtilM->prepareCabin(bkg, cab);
    CPPUNIT_ASSERT(cabin->cabin().isFirstClass());
  }

  void testGetRBDCabin_Not_Found_Return_BLANK()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    BookingCode bk = "Z";

    CPPUNIT_ASSERT_EQUAL(RBDByCabinUtil::CHAR_BLANK, _cabinUtilM->getRBDCabin(info, bk));
  }

  void testGetRBDCabin_Match_Found()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    BookingCode bk = "A1";

    CPPUNIT_ASSERT_EQUAL('W', _cabinUtil->getRBDCabin(info, bk));
  }

  void testValidateRBDByCabin_Match_RBD()
  {
    _cabinUtilM->setsUpSegInds(*_airSeg);
    _airSeg->geoTravelType() = GeoTravelType::UnknownGeoTravelType;
    _cabinUtilM->setsUpGeoTravelType();
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    BookingCode bk = "A1";
    std::vector<RBDByCabinInfo*> cabinInfos;
    cabinInfos.push_back(&info);

    const Cabin* cabin = _cabinUtilM->validateRBDByCabin(cabinInfos, bk,
                                               _airSeg->departureDT(), _airSeg->departureDT());
    CPPUNIT_ASSERT(cabin->cabin().isPremiumEconomyClass());
  }

  void testValidateRBDByCabin_Not_Match_RBD_NOT_YY_ReturnZero()
  {
    _cabinUtilM->setsUpSegInds(*_airSeg);
    _airSeg->geoTravelType() = GeoTravelType::UnknownGeoTravelType;
    _cabinUtilM->setsUpGeoTravelType();
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    BookingCode bk = "A";
    std::vector<RBDByCabinInfo*> cabinInfos;
    cabinInfos.push_back(&info);

    const Cabin* cabin = _cabinUtilM->validateRBDByCabin(cabinInfos, bk,
                                                    _airSeg->departureDT(), _airSeg->departureDT());
    CPPUNIT_ASSERT(!cabin);
  }

  void testValidateRBDByCabin_Not_Match_RBD_Ancil_YY_ReturnEconomy()
  {
    _cabinUtilM->setsUpSegInds(*_airSeg);
    _airSeg->geoTravelType() = GeoTravelType::UnknownGeoTravelType;
    _cabinUtilM->setsUpGeoTravelType();
    _cabinUtilM->_ancilBagServices = true;
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    info.carrier() = "YY";

    BookingCode bk = "A";
    std::vector<RBDByCabinInfo*> cabinInfos;
    cabinInfos.push_back(&info);

    const Cabin* cabin = _cabinUtilM->validateRBDByCabin(cabinInfos, bk,
                                           _airSeg->departureDT(), _airSeg->departureDT());
    CPPUNIT_ASSERT(cabin->cabin().isEconomyClass());
  }

  void testValidateRBDByCabin_Not_Match_RBD_Not_Ancil_YY_ReturnEconomy()
  {
    _cabinUtilM->setsUpSegInds(*_airSeg);
    _airSeg->geoTravelType() = GeoTravelType::UnknownGeoTravelType;
    _cabinUtilM->setsUpGeoTravelType();

    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    info.carrier() = "YY";

    BookingCode bk = "A";
    std::vector<RBDByCabinInfo*> cabinInfos;
    cabinInfos.push_back(&info);

    const Cabin* cabin = _cabinUtilM->validateRBDByCabin(cabinInfos, bk,
                                            _airSeg->departureDT(), _airSeg->departureDT());
    CPPUNIT_ASSERT(cabin->cabin().isEconomyClass());
  }

  void testFindCabinByRBD_DB_EMPTY()
  {
    CarrierCode cxr = "AA";
    BookingCode bk = "A";
    CPPUNIT_ASSERT(!_cabinUtilM->findCabinByRBD(cxr, bk, _airSeg->departureDT(), _airSeg->departureDT()));
  }

  void testGetCabinByRbdByType_InvalidClass()
  {
    BookingCode bk = "A";
    PaxTypeFare pTF;
    uint32_t itemNo = 1;
    BookingCodeExceptionSegment bceSegment;

    CabinType cb = _cabinUtilM->getCabinByRbdByType(*_airSeg, bk, pTF, itemNo, bceSegment);
    CPPUNIT_ASSERT(cb.isInvalidClass());
  }

  void testGetCabinByRbdByType_InvalidClass_SegmentOpen()
  {
    BookingCode bk = "A";
    _airSeg->segmentType() = Open;
    PaxTypeFare pTF;
    uint32_t itemNo = 1;
    BookingCodeExceptionSegment bceSegment;

    CabinType cb = _cabinUtilM->getCabinByRbdByType(*_airSeg, bk, pTF, itemNo, bceSegment);
    CPPUNIT_ASSERT(cb.isInvalidClass());
  }

  void testGetCabinByRBD_InvalidClass_SegmentOpen()
  {
    _airSeg->segmentType() = Open;
    _cabinUtilM->getCabinByRBD(*_airSeg);
    CPPUNIT_ASSERT(_airSeg->bookedCabin().isInvalidClass());
  }

  void testGetCabinByRBD_InvalidClass_Air_Segment()
  {
    _airSeg->segmentType() = Air;
    _cabinUtilM->getCabinByRBD(*_airSeg);
    CPPUNIT_ASSERT(_airSeg->bookedCabin().isInvalidClass());
  }

  void testGetCabinByRBD()
  {
    CarrierCode cxr = "AA";
    BookingCode bk = "A";
    CPPUNIT_ASSERT(!_cabinUtilM->getCabinByRBD(cxr, bk, *_airSeg));
  }

  void testGetCabinsByRbd_Ptf_InvalidClass()
  {
    FareMarket fm;
    PaxTypeFare ptf;
    ptf.fareMarket() = &fm;
    std::vector<ClassOfService> classOfService;
    ClassOfService cos;
    cos.bookingCode() = "Y";
    cos.numSeats() = 10;
    classOfService.push_back(cos);
    _cabinUtilM->getCabinsByRbd(ptf, classOfService);

    CPPUNIT_ASSERT(classOfService[0].cabin().isInvalidClass());
  }

  void testGetCabinForAirseg_InvalidClass()
  {
    CPPUNIT_ASSERT(!_cabinUtilM->getCabinForAirseg(*_airSeg));
  }

  void testGetCabinByRBD_InvalidClass()
  {
    CPPUNIT_ASSERT(!_cabinUtilM->getCabinByRBD(_airSeg->carrier(),
                                               _airSeg->getBookingCode(),
                                               _airSeg->departureDT()));
  }

  void testGetCabinsByRbd_InvalidClass()
  {
    std::vector<ClassOfService*> classOfService;
    ClassOfService cos;
    cos.bookingCode() = "Y";
    classOfService.push_back(&cos);
    _cabinUtilM->getCabinsByRbd(*_airSeg, classOfService);

    CPPUNIT_ASSERT(classOfService[0]->cabin().isInvalidClass());
  }

  void testGetCabinsByRbd_Cabin()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);

    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtilM->_cabinInfos.push_back(&info);
    std::vector<ClassOfService*> classOfService;
    ClassOfService cos;
    cos.bookingCode() = "C3";
    classOfService.push_back(&cos);
    _cabinUtilM->getCabinsByRbd(*_airSeg, classOfService);

    CPPUNIT_ASSERT(classOfService[0]->cabin().isPremiumEconomyClass());
  }

  void testGetCabinsByRbd_Atae_Cabin()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);

    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtilM->_cabinInfos.push_back(&info);
    std::vector<BookingCode> bks;
    bks.push_back("C3");
    _cabinUtilM->getCabinsByRbd(*_airSeg, bks);

    CPPUNIT_ASSERT(_airSeg->classOfService().front()->cabin().isPremiumEconomyClass());
  }

  void testIsProgramCallSelected_Empty()
  {
    createActivateDiagnostic();
    CPPUNIT_ASSERT(_cabinUtilM->isProgramCallSelected());
  }
  void testIsProgramCallSelected_Not_Match()
  {
    createActivateDiagnostic();
    createDiagnosticParams(PRG_CALL);
    CPPUNIT_ASSERT(!_cabinUtilM->isProgramCallSelected());
  }
  void testIsProgramCallSelected_Match()
  {
    createActivateDiagnostic();
    createDiagnosticParams(PRG_CALL);
    _cabinUtilM->_call = FAMILY_LOGIC;
    CPPUNIT_ASSERT(_cabinUtilM->isProgramCallSelected());
  }

  void testIsCpaSelected_Empty()
  {
    createActivateDiagnostic();
    CPPUNIT_ASSERT(_cabinUtilM->isCpaSelected());
  }
  void testIsCpaSelected_Not_Match()
  {
    createActivateDiagnostic();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("CP", "DFWLAX"));
    _cabinUtilM->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(!_cabinUtilM->isCpaSelected());
  }
  void testIsCpaSelected_Match()
  {
    createActivateDiagnostic();
    createDiagnosticParams(CITY_PAIR);
    _cabinUtilM->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(_cabinUtilM->isCpaSelected());
  }

  void testIsCpaSelected_FM_Not_Match()
  {
    createActivateDiagnostic();
    createDiagnosticParams(CITY_PAIR);
    _cabinUtilM->setsUpFMInds(*_ptf);
    CPPUNIT_ASSERT(!_cabinUtilM->isCpaSelected());
  }
  void testIsCpaSelected_FM_Match()
  {
    createActivateDiagnostic();
    createDiagnosticParams(CITY_PAIR);
    _cabinUtilM->setsUpFMInds(*_ptf);
    _fm->travelSeg().push_back(_airSeg);
    CPPUNIT_ASSERT(_cabinUtilM->isCpaSelected());
  }

  void testIsCabinSelected_FAIL()
  {
    createActivateDiagnostic();
    CPPUNIT_ASSERT(!_cabinUtilM->isCabinSelected());
  }
  void testIsCabinSelected_PASS()
  {
    createActivateDiagnostic();
    createDiagnosticParams(CABIN);
    CPPUNIT_ASSERT(_cabinUtilM->isCabinSelected());
  }

  void testIsSequenceSelected_Empty()
  {
    createActivateDiagnostic();
    RBDByCabinInfo info;
    CPPUNIT_ASSERT(_cabinUtilM->isSequenceSelected(&info));
  }
  void testIsSequenceSelected_Match()
  {
    createActivateDiagnostic();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("SQ", "111233"));
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    CPPUNIT_ASSERT(_cabinUtilM->isSequenceSelected(&info));
  }
  void testIsSequenceSelected_Not_Match()
  {
    createActivateDiagnostic();
    createDiagnosticParams(SEQUENCE);
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    CPPUNIT_ASSERT(!_cabinUtilM->isSequenceSelected(&info));
  }

  void testIsCxrSelected_Empty()
  {
    createActivateDiagnostic();
    CarrierCode cxr = CarrierCode();
    CPPUNIT_ASSERT(_cabinUtilM->isCxrSelected(cxr));
  }
  void testIsCxrSelected_Request_Match()
  {
    createActivateDiagnostic();
    createDiagnosticParams(CARRIER);
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT(_cabinUtilM->isCxrSelected(cxr));
  }
  void testIsCxrSelected_Request_Not_Match()
  {
    createActivateDiagnostic();
    createDiagnosticParams(CARRIER);
    CarrierCode cxr = "BB";
    CPPUNIT_ASSERT(!_cabinUtilM->isCxrSelected(cxr));
  }
  void testIsCxrSelected_AirSeg_Match()
  {
    createActivateDiagnostic();
    createDiagnosticParams(CARRIER);
    CarrierCode cxr = CarrierCode();
    _cabinUtilM->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(_cabinUtilM->isCxrSelected(cxr));
  }
  void testIsCxrSelected_AirSeg_Not_Match()
  {
    createActivateDiagnostic();
    createDiagnosticParams(CARRIER);
    CarrierCode cxr = CarrierCode();
    _airSeg->carrier() = "BB";
    _cabinUtilM->setsUpSegInds(*_airSeg);
    CPPUNIT_ASSERT(!_cabinUtilM->isCxrSelected(cxr));
  }

  void testIsCxrSelected_FM_Match()
  {
    createActivateDiagnostic();
    createDiagnosticParams(CARRIER);
    CarrierCode cxr = CarrierCode();
    _fm->governingCarrier() = "AA";
    _cabinUtilM->setsUpFMInds(*_ptf);
    CPPUNIT_ASSERT(_cabinUtilM->isCxrSelected(cxr));
  }
  void testIsCxrSelected_FM_Not_Match()
  {
    createActivateDiagnostic();
    createDiagnosticParams(CARRIER);
    CarrierCode cxr = CarrierCode();
    _fm->governingCarrier() = "BB";
    _cabinUtilM->setsUpFMInds(*_ptf);
    CPPUNIT_ASSERT(!_cabinUtilM->isCxrSelected(cxr));
  }

  void testIsBookingCodeSelected_Empty()
  {
    createActivateDiagnostic();
    BookingCode bc = "A";
    CPPUNIT_ASSERT(_cabinUtilM->isBookingCodeSelected(bc));
  }
  void testIsBookingCodeSelected_Request_Match()
  {
    createActivateDiagnostic();
    createDiagnosticParams(RBD);
    BookingCode bc = "C3";
    CPPUNIT_ASSERT(_cabinUtilM->isBookingCodeSelected(bc));
  }
  void testIsBookingCodeSelected_Request_Not_Match()
  {
    createActivateDiagnostic();
    createDiagnosticParams(RBD);
    BookingCode bc = "C";
    CPPUNIT_ASSERT(!_cabinUtilM->isBookingCodeSelected(bc));
  }

  void testPrintHeader()
  {
    createActivateDiagnostic();
    _cabinUtilM->_call = PRICING_RQ;
    _cabinUtilM->printHeader();
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "         GET CABIN FOR THE RBD IN ITINERARY REQUEST\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintRecordHeader_Short()
  {
    createActivateDiagnostic();
    _cabinUtilM->printRecordHeader();
    std::stringstream expectedDiag;
    expectedDiag
    << "-------------------------------------------------------------\n"
    << " VN   CXR  SEQ      GI  EFF DATE  FIRST TKT DATE    STATUS\n"
    << "-------------------------------------------------------------\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintRecordHeader()
  {
    createActivateDiagnostic();
    createDiagnosticParams(DDINFO);
    _cabinUtilM->printRecordHeader();
    std::stringstream expectedDiag;
    expectedDiag
    << "-------------------------------------------------------------\n"
    << " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n"
    << "                              DISC          LAST\n"
    << "-------------------------------------------------------------\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintCurrentRBD()
  {
    createActivateDiagnostic();
    BookingCode bc = "F";
    _cabinUtilM->printCurrentRBD(bc);
    std::stringstream expectedDiag;
    expectedDiag
    << "-------------------------------------------------------------\n"
    << "     GET CABIN FOR RBD:  F\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintRbdByCabinInfo()
  {
    createActivateDiagnostic();
    createDiagnosticParams(DDINFO);
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtilM->printRbdByCabinInfo(&info);
    std::stringstream expectedDiag;
    expectedDiag
    << " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
    << "                              31DEC2025     31DEC2025 \n"
    << " FLIGHT 1:         EQUIPMENT: \n"
    << " FLIGHT 2: \n"
    << " GEO LOC1: C DFW \n"
    << " GEO LOC2: C FRA \n"
    << "  \n"
    << " CABIN: W - PREMIUM ECONOMY CABIN \n"
    << "   7        A1 B2 C3 D4 E5 \n";
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str() );
  }
  void testPrintRbdByCabinInfo_Short()
  {
    createActivateDiagnostic();
    createDiagnosticParams(SEQUENCE);
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    _cabinUtilM->printRbdByCabinInfo(&info);
    std::stringstream expectedDiag;
    expectedDiag << " ATP  AA   111233   AT  01JAN2010  02JAN2010      ";
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }

  void testPrintReqCabinNotMatch()
  {
    createActivateDiagnostic();
    Indicator cabin = 'R';
    BookingCode bc = "F";
    _cabinUtilM->printReqCabinNotMatch(cabin, bc);
    std::stringstream expectedDiag;
    expectedDiag << "\n  *** RBD: F  IS NOT IN THE REQUESTED CABIN: R\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintReqCabinNotMatch_NothingToPrint()
  {
    createActivateDiagnostic();
    createDiagnosticParams(DDPASSED);
    Indicator cabin = 'R';
    BookingCode bc = "F";
    _cabinUtilM->printReqCabinNotMatch(cabin, bc);
    std::stringstream expectedDiag;
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintCabinNotFoundForRBD()
  {
    createActivateDiagnostic();
    BookingCode bc = "F";
    _cabinUtilM->printCabinNotFoundForRBD(bc);
    std::stringstream expectedDiag;
    expectedDiag << "\n  *** SEQUENCE MATCHED FIXED DATA: RBD - F NOT FOUND\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintCabinNotFoundForRBD_NothingToPrint()
  {
    createActivateDiagnostic();
    createDiagnosticParams(DDPASSED);
    BookingCode bc = "F";
    _cabinUtilM->printCabinNotFoundForRBD(bc);
    std::stringstream expectedDiag;
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintDefaultEconomyCabin()
  {
    createActivateDiagnostic();
    BookingCode bc = "F";
    _cabinUtilM->printDefaultEconomyCabin(bc);
    std::stringstream expectedDiag;
    expectedDiag << "\n DEFAULT ECONOMY CABIN ASSIGNED\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintDefaultEconomyCabin_NothingToPrint()
  {
    createActivateDiagnostic();
    createDiagnosticParams(DDPASSED);
    BookingCode bc = "F";
    _cabinUtilM->printDefaultEconomyCabin(bc);
    std::stringstream expectedDiag;
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintCabinFoundForRBD()
  {
    createActivateDiagnostic();
    createDiagnosticParams(DDINFO);
    BookingCode bc = "F";
    Indicator cabin = 'R';
    CarrierCode cxr = "AA";
    _cabinUtilM->printCabinFoundForRBD(bc, cabin, cxr);
    std::stringstream expectedDiag;
    expectedDiag 
    << " ** RESULT: FOUND CABIN: R - PREMIUM FIRST CABIN FOR RBD: F **\n"
    << " **         "
    << "  \n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintCabinFoundForRBD_Short()
  {
    createActivateDiagnostic();
    BookingCode bc = "F";
    Indicator cabin = 'R';
    CarrierCode cxr = "AA";
    _cabinUtilM->printCabinFoundForRBD(bc, cabin, cxr);
    std::stringstream expectedDiag;
    expectedDiag << "PASS\n"
    << " ** RESULT: FOUND CABIN: R - PREMIUM FIRST CABIN FOR RBD: F **\n"
    << " **         "
    << "  \n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintFailFoundCabinForRBD()
  {
    createActivateDiagnostic();
    BookingCode bc = "F";
    _cabinUtilM->printFailFoundCabinForRBD(bc, _airSeg->carrier());
    std::stringstream expectedDiag;
    expectedDiag << "\n  *** CABIN NOT FOUND FOR RBD : F  CARRIER : AA\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintFailFoundCabinForRBD_NothingToPrint()
  {
    createActivateDiagnostic();
    createDiagnosticParams(DDPASSED);
    BookingCode bc = "F";
    _cabinUtilM->printFailFoundCabinForRBD(bc, _airSeg->carrier());
    std::stringstream expectedDiag;
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintRBDByCabinNotFoundForCarrier()
  {
    createActivateDiagnostic();
    _cabinUtilM->printRBDByCabinNotFoundForCarrier(_airSeg->carrier());
    std::stringstream expectedDiag;
    expectedDiag
    << "-------------------------------------------------------------\n"
    << "  *** CABIN RECORD NOT FOUND FOR CARRIER : AA\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintFailStatus_Short()
  {
    createActivateDiagnostic();
    _cabinUtilM->printFailStatus( FAIL_FLIGHT, _airSeg->carrier());
    std::stringstream expectedDiag;
    expectedDiag
    << "FAIL FLIGHT\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }
  void testPrintFailStatus()
  {
    createActivateDiagnostic();
    createDiagnosticParams(DDINFO);
    _cabinUtilM->printFailStatus( FAIL_FLIGHT, _airSeg->carrier());
    std::stringstream expectedDiag;
    expectedDiag
    << "                                     *** STATUS: FAIL FLIGHT\n"
    << "-------------------------------------------------------------\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }
  void testPrintFailStatus_NothingToPrint()
  {
    createActivateDiagnostic();
    createDiagnosticParams(DDPASSED);
    _cabinUtilM->printFailStatus( FAIL_FLIGHT, _airSeg->carrier());
    std::stringstream expectedDiag;
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _cabinUtilM->_diag->str() );
  }

  void testPrintAirSegSearchData_AirSeg()
  {
    createActivateDiagnostic();
    _cabinUtilM->setsUpSegInds(*_airSeg);
    _airSeg->setBookingCode("F");
    _cabinUtilM->printAirSegSearchData(*_airSeg, _airSeg->departureDT());
    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: FLIGHT SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: AA  MARKET: DFW-LON  TRAVEL ON: 19SEP2015 \n"
    << "    RBD: F\n";
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }
  void testPrintAirSegSearchData()
  {
    createActivateDiagnostic();
    _cabinUtilM->setsUpSegInds(*_airSeg);
    _airSeg->setBookingCode("F");
    _cabinUtilM->printAirSegSearchData(_airSeg->departureDT());
    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: FLIGHT SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: AA  MARKET: DFW-LON  TRAVEL ON: 19SEP2015 \n"
    << "    RBD: F\n";
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }

  void testPrintAirSegSearchData_Cxr_Rbd()
  {
    createActivateDiagnostic();
    _cabinUtilM->setsUpSegInds(*_airSeg);
    BookingCode bc = "F";
    _cabinUtilM->printAirSegSearchData(_airSeg->carrier(), bc, _airSeg->departureDT());
    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: FLIGHT SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: AA  MARKET: DFW-LON  TRAVEL ON: 19SEP2015 \n"
    << "    RBD: F\n";
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }
  void testPrintAirSegSearchData_Rbd_List()
  {
    createActivateDiagnostic();
    _cabinUtilM->setsUpSegInds(*_airSeg);
    std::vector<BookingCode> bks;
    bks.push_back("C3");
    bks.push_back("D3");
    _cabinUtilM->printAirSegSearchData(bks, _airSeg->departureDT());
    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: FLIGHT SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: AA  MARKET: DFW-LON  TRAVEL ON: 19SEP2015 \n"
    << "    RBD: C3 D3 \n";
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }
  void testPrintAirSegSearchData_Cos_List()
  {
    createActivateDiagnostic();
    _cabinUtilM->setsUpSegInds(*_airSeg);
    std::vector<ClassOfService*> classOfService;
    ClassOfService cos;
    cos.bookingCode() = "Y";
    classOfService.push_back(&cos);
    _cabinUtilM->printAirSegSearchData(classOfService, _airSeg->departureDT());
    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: FLIGHT SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: AA  MARKET: DFW-LON  TRAVEL ON: 19SEP2015 \n"
    << "    RBD: Y \n";
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }
  void testPrintAirSegSearchData_Ptf()
  {
    createActivateDiagnostic();
    _fm->governingCarrier() = "AA";
    _cabinUtilM->setsUpFMInds(*_ptf);
    std::vector<ClassOfService> classOfService;
    ClassOfService cos;
    cos.bookingCode() = "Y";
    classOfService.push_back(cos);
    _fm->travelSeg().push_back(_airSeg);

    FareInfo fareInfo;
    fareInfo.fareClass() = "Y26";
    Fare fare;

    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm, &tCRInfo);
    _ptf->setFare(&fare);

    _cabinUtilM->printAirSegSearchData(*_ptf, classOfService);
    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: FLIGHT SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: AA  MARKET: DFW-LON  TRAVEL ON: 19SEP2015 \n"
    << "    RBD: Y   FARE CLASS: Y26     \n";
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }
  void testPrintAirSegSearchData_Ptf_Bce()
  {
    createActivateDiagnostic();
    _fm->governingCarrier() = "AA";
    _fm->travelSeg().push_back(_airSeg);
    _cabinUtilM->setsUpSegInds(*_airSeg);
     BookingCode bc = "Y";
    FareInfo fareInfo;
    fareInfo.fareClass() = "Y26";
    Fare fare;

    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm, &tCRInfo);
    _ptf->setFare(&fare);
    uint32_t item = 1;
    BookingCodeExceptionSegment bce;
    bce.segNo() = 1001;
    bce.restrictionTag() = 'R';

    _cabinUtilM->printAirSegSearchData(bc, *_ptf, item, bce, _ptf->fareMarket()->travelDate());
    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: FLIGHT SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: AA  MARKET: DFW-LON  TRAVEL ON: 19SEP2015 \n"
    << "    RBD: Y\n"
    << "    FARE CLASS: Y26         FARE MARKET: DFW-LON\n"
    << "    T999 ITEM: 1   SEQ: 1001      RESTR TAG: R\n";
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _cabinUtilM->_diag->str());
  }

//  isFlightNumberOrEquipmentCoded
  void testSkipSequenceFlt1AndEquipNotCoded()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);

    CPPUNIT_ASSERT(!_cabinUtil->isFlightNumberOrEquipmentCoded(&info));
  }

  void testSkipSequenceFlt1Coded()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    info.flightNo1() = 2222;

    CPPUNIT_ASSERT(_cabinUtil->isFlightNumberOrEquipmentCoded(&info));
  }
  void testSkipSequenceEquipCoded()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    info.equipmentType() = "777";

    CPPUNIT_ASSERT(_cabinUtil->isFlightNumberOrEquipmentCoded(&info));
  }
  void testSkipSequenceFlt1AndEquipBothCoded()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    prepareRBDByCabinInfo(info, lk1, lk2);
    info.flightNo1() = 2222;
    info.equipmentType() = "777";

    CPPUNIT_ASSERT(_cabinUtil->isFlightNumberOrEquipmentCoded(&info));
  }


protected:
  RBDByCabinUtil* _cabinUtil;
  RBDByCabinUtilTestMock* _cabinUtilM;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  AirSeg*     _airSeg;
  FareMarket* _fm;
  PaxTypeFare* _ptf;
  Diag187Collector* _diag;
  RBDByCabinCall _call;

  const Loc*  _locDFW;
  const Loc*  _locLON;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RBDByCabinUtilTest);
} // NS tse
