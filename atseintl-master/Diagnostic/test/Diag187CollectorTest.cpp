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

#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/BookingCodeExceptionSegment.h"
#include "DBAccess/RBDByCabinInfo.h"
#include "DBAccess/ZoneInfo.h"
#include "Diagnostic/Diag187Collector.h"


using namespace std;

namespace tse
{

namespace
{
class Diag187CollectorTestMock : public Diag187Collector
{
public:
  const ZoneInfo* getZone(const PricingTrx& trx,
                          const VendorCode& vendor,
                          const Zone& zoneNo)
  {
    return &_zoneInfo;
  }

  const vector<class RBDByCabinInfo*>
                                    getRBDByCabin(PricingTrx& trx,
                                                  const VendorCode& vendor,
                                                  CarrierCode carrier)
  {
    return _cabinInfos;

  }
ZoneInfo _zoneInfo;
vector<class RBDByCabinInfo*> _cabinInfos;
};
} // anon NS

class Diag187CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag187CollectorTest);

  CPPUNIT_TEST(testPrintHeader_PRICING_RQ);
  CPPUNIT_TEST(testPrintHeader_ITIN_SHP_PYA);
  CPPUNIT_TEST(testPrintHeader_CONTENT_SVC);
  CPPUNIT_TEST(testPrintHeader_CONTENT_SVC_FAKE);
  CPPUNIT_TEST(testPrintHeader_DSS_RSP);
  CPPUNIT_TEST(testPrintHeader_AVAIL_RSP);
  CPPUNIT_TEST(testPrintHeader_T999_VAL);
  CPPUNIT_TEST(testPrintHeader_RBD_CAT31);
  CPPUNIT_TEST(testPrintHeader_OPTIONAL_SVC);
  CPPUNIT_TEST(testPrintHeader_ANC_BAG_RQ);
  CPPUNIT_TEST(testPrintHeader_SHP_HANDLER);
  CPPUNIT_TEST(testPrintHeader_PREFERRED_CABIN);
  CPPUNIT_TEST(testPrintHeader_SHP_WPNI);
  CPPUNIT_TEST(testPrintHeader_RBD_VAL);
  CPPUNIT_TEST(testPrintHeader_ITIN_SHP_SVC);
  CPPUNIT_TEST(testPrintHeader_SOLD_OUT);
  CPPUNIT_TEST(testPrintHeader_FAMILY_LOGIC);
  CPPUNIT_TEST(testPrintHeader_UNKNOWN_CALL);

  CPPUNIT_TEST(testPrintHeader_Cxr_And_CityPair);
  CPPUNIT_TEST(testPrintHeader_Cxr_Only);
  CPPUNIT_TEST(testPrintHeader_Cxr_ALL);

  CPPUNIT_TEST(testPrintRecordHeader);
  CPPUNIT_TEST(testPrintRecordHeaderShort);
  CPPUNIT_TEST(testPrintCurrentRBD);
  CPPUNIT_TEST(testPrintReqCabinNotMatch);
  CPPUNIT_TEST(testPrintCabinNotFoundForRBD);
  CPPUNIT_TEST(testPrintRBDinRQNotMatchRBDinSeg);
  CPPUNIT_TEST(testPrintAncBagRBDNotFiledInCxrRbdByCabin);
  CPPUNIT_TEST(testPrintDefaultEconomyCabin);
  CPPUNIT_TEST(testPrintCabinFoundForRBD);
  CPPUNIT_TEST(testPrintFailFoundCabinForRBD);
  CPPUNIT_TEST(testPrintRBDByCabinNotFoundForCarrier);
  CPPUNIT_TEST(testPrintFailStatus_FAIL_TRAVEL_DATE);
  CPPUNIT_TEST(testPrintFailStatus_FAIL_TICKET_DATE);
  CPPUNIT_TEST(testPrintFailStatus_FAIL_GEO_LOC);
  CPPUNIT_TEST(testPrintFailStatus_FAIL_GLOBAL_IND);
  CPPUNIT_TEST(testPrintFailStatus_FAIL_EQUP);
  CPPUNIT_TEST(testPrintFailStatus_FAIL_FLIGHT);
  CPPUNIT_TEST(testPrintFailStatusShort_FAIL_FLIGHT);
  CPPUNIT_TEST(testPrintFailStatus_UNKNOWN);

  CPPUNIT_TEST(testPrintRbdByCabinInfo_NoZone_NoCabin_Selected);
  CPPUNIT_TEST(testPrintRbdByCabinInfo_Wrong_Cabin_Selected);
  CPPUNIT_TEST(testPrintRbdByCabinInfo_Wrong_Cabin_Coded);
  CPPUNIT_TEST(testPrintRbdByCabinInfo_Cabin_Not_Coded);
  CPPUNIT_TEST(testDisplayLocType_City);
  CPPUNIT_TEST(testDisplayLocType_State);
  CPPUNIT_TEST(testDisplayLocType_Nation);
  CPPUNIT_TEST(testDisplayLocType_SubArea);
  CPPUNIT_TEST(testDisplayLocType_Area);
  CPPUNIT_TEST(testDisplayLocType_Zone);
  CPPUNIT_TEST(testDisplayLocType_Airport);
  CPPUNIT_TEST(testDisplayLocType_Pcc);
  CPPUNIT_TEST(testDisplayLocType_PccArc);
  CPPUNIT_TEST(testDisplayLocType_User);
  CPPUNIT_TEST(testDisplayLocType_FmsZone);
  CPPUNIT_TEST(testDisplayLocType_None);
  CPPUNIT_TEST(testDisplayDirectionalInd_Between);
  CPPUNIT_TEST(testDisplayDirectionalInd_And);
  CPPUNIT_TEST(testDisplayDirectionalInd_To);
  CPPUNIT_TEST(testDisplayDirectionalInd_From);
  CPPUNIT_TEST(testDisplayDirectionalInd_None);
  CPPUNIT_TEST(testDisplayDirectionalInd_Unknown);
  CPPUNIT_TEST(testDisplayExclIncl_Include);
  CPPUNIT_TEST(testDisplayExclIncl_Exclude);
  CPPUNIT_TEST(testDisplayExclIncl_Unknown);
  CPPUNIT_TEST(testDisplayCabin);
  CPPUNIT_TEST(testDisplayZoneDetail);
  CPPUNIT_TEST(testDisplayZoneDetail_Zone_Not_Found);
  CPPUNIT_TEST(testPrintRbdByCabinInfo_With_Zone);
  CPPUNIT_TEST(testPrintRbdByCabinInfo_With_Zone_Detail);
  CPPUNIT_TEST(testPrintFooter);

  CPPUNIT_TEST(testPrintRbdByCabinInfoShort);
  CPPUNIT_TEST(testPrintAirSegData_With_SingleRBD_Flignt_Segment);
  CPPUNIT_TEST(testPrintAirSegData_With_SingleRBD_Open_Segment);
  CPPUNIT_TEST(testPrintAirSegData_With_SingleRBD_NoGEO_Segment);
  CPPUNIT_TEST(testPrintAirSegData_With_SingleRBD_partial_GEO_Segment);
  CPPUNIT_TEST(testPrintAirSegData_With_SegmentDataOnly);
  CPPUNIT_TEST(testPrintAirSegData_With_SegmentData_Bkc_vector);
  CPPUNIT_TEST(testPrintAirSegData_With_Ptfare_Bkc_vector);
  CPPUNIT_TEST(testPrintAirSegData_With_WPNCS_Ptfare);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _collector = _memHandle.create<Diag187Collector>();
    _collector->activate();
    _collectorM = _memHandle.insert(new Diag187CollectorTestMock());
    _collectorM->activate();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->ticketingDate() = DateTime(2015, 7, 12);
    _locDFW = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _locLON = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testPrintHeader_PRICING_RQ()
  {
    _collector->printHeader(PRICING_RQ);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "         GET CABIN FOR THE RBD IN ITINERARY REQUEST\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintHeader_ITIN_SHP_PYA()
  {
    _collector->printHeader(ITIN_SHP_PYA);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "        GET CABIN FOR SHOPPING ORIGIN BASED RT PRICING\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintHeader_CONTENT_SVC()
  {
    _collector->printHeader(CONTENT_SVC);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "          GET CABIN FOR ALL RBDS FOR OPEN SEGMENT\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintHeader_CONTENT_SVC_FAKE()
  {
    _collector->printHeader(CONTENT_SVC_FAKE);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "            GET CABIN FOR RBD FOR AVFAKE REQUEST\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }
  void testPrintHeader_DSS_RSP()
  {
    _collector->printHeader(DSS_RSP);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "            GET CABINS FOR RBD RETURNED BY DSS\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }
  void testPrintHeader_AVAIL_RSP()
  {
    _collector->printHeader(AVAIL_RSP);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "          GET CABINS FOR RBD RETRIEVED FROM ASV2\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }
  void testPrintHeader_T999_VAL()
  {
    _collector->printHeader(T999_VAL);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "         GET CABIN FOR T999 REBOOKED RBD FOR WPNCS\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }
  void testPrintHeader_RBD_CAT31()
  {
    _collector->printHeader(RBD_CAT31);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "       GET CABIN FOR CAT31 RBD HIERARCHY PROCESSING\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }
  void testPrintHeader_OPTIONAL_SVC()
  {
    _collector->printHeader(OPTIONAL_SVC);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "     GET CABIN FOR T198 RBD IN ANCILLARY S7 VALIDATION\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }
  void testPrintHeader_ANC_BAG_RQ()
  {
    _collector->printHeader(ANC_BAG_RQ);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "    GET CABIN FOR FLIGHT RBD IN ANCILLARY/BAGGAGE REQUEST\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }
  void testPrintHeader_SHP_HANDLER()
  {
    _collector->printHeader(SHP_HANDLER);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "        GET CABIN FOR FLIGHT RBD IN SHOPPING REQUEST\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }
  void testPrintHeader_PREFERRED_CABIN()
  {
    _collector->printHeader(PREFERRED_CABIN);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "      GET CABIN FOR PREFERRED RBD IN SHOPPING REQUEST\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }
  void testPrintHeader_SHP_WPNI()
  {
    _collector->printHeader(SHP_WPNI);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "          GET CABIN FOR ALL RBD IN SHOPPING WPNI.C\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }
  void testPrintHeader_RBD_VAL()
  {
    _collector->printHeader(RBD_VAL);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "          GET CABIN FOR WPNC/WPNCS IN PRICE BY CABIN\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintHeader_ITIN_SHP_SVC()
  {
    _collector->printHeader(ITIN_SHP_SVC);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "         GET CABIN FOR SOP PREMIUM IN ITINANALYZER\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }
  void testPrintHeader_SOLD_OUT()
  {
    _collector->printHeader(SOLD_OUT);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "          GET CABIN FOR SOLD OUT IN ITINANALYZER\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }
  void testPrintHeader_FAMILY_LOGIC()
  {
    _collector->printHeader(FAMILY_LOGIC);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "        GET CABIN FOR FAMILY LOGIC IN ITINANALYZER\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }
  void testPrintHeader_UNKNOWN_CALL()
  {
    _collector->printHeader(NO_CALL);
    std::stringstream expectedDiag;
    expectedDiag
    << "*************************************************************\n"
    << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
    << "             GET ALL VALID RBD CABIN FOR CARRIER\n"
    << "*************************************************************\n";
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintHeader_Cxr_And_CityPair()
  {
    const std::string cxr = "AA";
    const std::string cityPair = "DFWMOW";
    DateTime date = DateTime(2015, 9, 19);
    std::stringstream expectedDiag;
    expectedDiag << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
                 << "*-----------------------------------------------------------*\n"
                 << " RBD BY CABIN FOR CARRIER: AA   CITY PAIR: DFW-MOW\n"
                 << " TRANSACTION DATE: 19SEP2015 \n"
                 << "*-----------------------------------------------------------*\n"
                 << " AVAILABLE QUALIFIERS:\n"
                 << " VN - VENDOR  CX - CARRIER    SQ - SEQUENCE\n"
                 << " CB - CABIN   CP - CITY PAIR  DDZONE - ZONE DETAILED INFO\n"
                 << "*************************************************************\n"
                 << " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n"
                 << "                              DISC          LAST\n"
                 << "-------------------------------------------------------------\n";

    _collector->printHeader(cxr, cityPair, date);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintHeader_Cxr_Only()
  {
    const std::string cxr = "AA";
    const std::string cityPair;
    DateTime date = DateTime(2015, 9, 19);

    std::stringstream expectedDiag;
    expectedDiag  << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
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
                  << "-------------------------------------------------------------\n";
    _collector->printHeader(cxr, cityPair, date);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintHeader_Cxr_ALL()
  {
    const std::string cxr;
    const std::string cityPair;
    DateTime date = DateTime(2015, 9, 19);
    std::stringstream expectedDiag;
    expectedDiag << "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n"
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
                 << "-------------------------------------------------------------\n";

    _collector->printHeader(cxr, cityPair, date);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintRecordHeader()
  {
    _collector->printRecordHeader();
    std::stringstream expectedDiag;
    expectedDiag
    << "-------------------------------------------------------------\n"
    << " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n"
    << "                              DISC          LAST\n"
    << "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintRecordHeaderShort()
  {
    _collector->printRecordHeaderShort();
    std::stringstream expectedDiag;
    expectedDiag 
    << "-------------------------------------------------------------\n"
    << " VN   CXR  SEQ      GI  EFF DATE  FIRST TKT DATE    STATUS\n"
    << "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintCurrentRBD()
  {
    BookingCode bks = "AZ";
    _collector->printCurrentRBD(bks);
    std::stringstream expectedDiag;
    expectedDiag 
    << "-------------------------------------------------------------\n"
    << "     GET CABIN FOR RBD:  AZ\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintReqCabinNotMatch()
  {
    BookingCode bks = "AZ";
    Indicator cabin = 'Z';
    _collector->printReqCabinNotMatch(cabin, bks);
    std::stringstream expectedDiag;
    expectedDiag 
    << "\n  *** RBD: AZ  IS NOT IN THE REQUESTED CABIN: Z\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintCabinNotFoundForRBD()
  {
    BookingCode bks = "AZ";
    _collector->printCabinNotFoundForRBD(bks);
    std::stringstream expectedDiag;
    expectedDiag 
    << "\n  *** SEQUENCE MATCHED FIXED DATA: RBD - AZ NOT FOUND\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintRBDinRQNotMatchRBDinSeg()
  {
    BookingCode bksRq = "AZ";
    BookingCode bksSeg = "ZA";
    _collector->printRBDinRQNotMatchRBDinSeg(bksRq, bksSeg);
    std::stringstream expectedDiag;
    expectedDiag 
    << "-------------------------------------------------------------\n"
    << "  REQUESTED RBD: AZ  NOT MATCH PROCESSED RBD: ZA\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintAncBagRBDNotFiledInCxrRbdByCabin()
  {
    BookingCode bks = "AZ";
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateZoneLocs(lk1, lk2);

    prepareRBDByCabinInfo(info, lk1, lk2);

    _collector->printAncBagRBDNotFiledInCxrRbdByCabin(info, bks);
    std::stringstream expectedDiag;
    expectedDiag 
    << "  PROCESSED RBD: AZ"
    << "\n  NOT FILED IN ATPCO RBDBYCABIN FOR CXR: AA  IN SEQ: 111233\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintDefaultEconomyCabin()
  {
    _collector->printDefaultEconomyCabin();
    std::stringstream expectedDiag;
    expectedDiag 
    << "\n DEFAULT ECONOMY CABIN ASSIGNED\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintCabinFoundForRBD()
  {
    BookingCode bks = "AZ";
    Indicator cabin = 'W';
    CarrierCode cxr = "ZZ";

    _collector->printCabinFoundForRBD(bks, cabin, cxr);
    std::stringstream expectedDiag;
    expectedDiag 
    << " ** RESULT: FOUND CABIN: W - PREMIUM ECONOMY CABIN FOR RBD: AZ **\n"
    << " **         "
    << "  \n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFailFoundCabinForRBD()
  {
    BookingCode bks = "AZ";
    CarrierCode cxr = "ZZ";

    _collector->printFailFoundCabinForRBD(bks, cxr);
    std::stringstream expectedDiag;
    expectedDiag 
    << "\n  *** CABIN NOT FOUND FOR RBD : AZ  CARRIER : ZZ\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintRBDByCabinNotFoundForCarrier()
  {
    CarrierCode cxr = "ZZ";

    _collector->printRBDByCabinNotFoundForCarrier(cxr);
    std::stringstream expectedDiag;
    expectedDiag 
    << "-------------------------------------------------------------\n"
    << "  *** CABIN RECORD NOT FOUND FOR CARRIER : ZZ\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFailStatus_FAIL_TRAVEL_DATE()
  {
    _collector->printFailStatus(FAIL_TRAVEL_DATE);
    std::stringstream expectedDiag;
    expectedDiag 
    << "                                     *** STATUS: FAIL TVL\n"
    << "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFailStatus_FAIL_TICKET_DATE()
  {
    _collector->printFailStatus(FAIL_TICKET_DATE);
    std::stringstream expectedDiag;
    expectedDiag 
    << "                                     *** STATUS: FAIL TKT\n"
    << "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFailStatus_FAIL_GEO_LOC()
  {
    _collector->printFailStatus(FAIL_GEO_LOC);
    std::stringstream expectedDiag;
    expectedDiag 
    << "                                     *** STATUS: FAIL GEO\n"
    << "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFailStatus_FAIL_GLOBAL_IND()
  {
    _collector->printFailStatus(FAIL_GLOBAL_IND);
    std::stringstream expectedDiag;
    expectedDiag 
    << "                                     *** STATUS: FAIL GI\n"
    << "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFailStatus_FAIL_EQUP()
  {
    _collector->printFailStatus(FAIL_EQUP);
    std::stringstream expectedDiag;
    expectedDiag 
    << "                                     *** STATUS: FAIL EQUP\n"
    << "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFailStatus_FAIL_FLIGHT()
  {
    _collector->printFailStatus(FAIL_FLIGHT);
    std::stringstream expectedDiag;
    expectedDiag 
    << "                                     *** STATUS: FAIL FLIGHT\n"
    << "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFailStatusShort_FAIL_FLIGHT()
  {
    _collector->printFailStatusShort(FAIL_FLIGHT);
    std::stringstream expectedDiag;
    expectedDiag << "FAIL FLIGHT\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFailStatus_UNKNOWN()
  {
    _collector->printFailStatus(UNKNOWN_STATUS);
    std::stringstream expectedDiag;
    expectedDiag 
    << "                                     *** STATUS: UNKNOWN\n"
    << "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }


  void testDisplayLocType_City()
  {
    const Indicator locType = 'C';
    std::stringstream expectedDiag;
    expectedDiag << "CITY   ";
    _collector->displayLocType(locType);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayLocType_State()
  {
    const Indicator locType = 'S';
    std::stringstream expectedDiag;
    expectedDiag << "STATE  ";
    _collector->displayLocType(locType);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayLocType_Nation()
  {
    const Indicator locType = 'N';
    std::stringstream expectedDiag;
    expectedDiag << "NATION ";
    _collector->displayLocType(locType);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayLocType_SubArea()
  {
    const Indicator locType = '*';
    std::stringstream expectedDiag;
    expectedDiag << "SUBAREA";
    _collector->displayLocType(locType);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayLocType_Area()
  {
    const Indicator locType = 'A';
    std::stringstream expectedDiag;
    expectedDiag << "AREA   ";
    _collector->displayLocType(locType);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayLocType_Zone()
  {
    const Indicator locType = 'Z';
    std::stringstream expectedDiag;
    expectedDiag << "ZONE   ";
    _collector->displayLocType(locType);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayLocType_Airport()
  {
    const Indicator locType = 'P';
    std::stringstream expectedDiag;
    expectedDiag << "AIRPORT";
    _collector->displayLocType(locType);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayLocType_Pcc()
  {
    const Indicator locType = 'X';
    std::stringstream expectedDiag;
    expectedDiag << "PCC    ";
    _collector->displayLocType(locType);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayLocType_PccArc()
  {
    const Indicator locType = 'Y';
    std::stringstream expectedDiag;
    expectedDiag << "PCC ARC";
    _collector->displayLocType(locType);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayLocType_User()
  {
    const Indicator locType = 'U';
    std::stringstream expectedDiag;
    expectedDiag << "USER   ";
    _collector->displayLocType(locType);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayLocType_FmsZone()
  {
    const Indicator locType = '1';
    std::stringstream expectedDiag;
    expectedDiag << "FMSZONE";
    _collector->displayLocType(locType);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayLocType_None()
  {
    const Indicator locType = ' ';
    std::stringstream expectedDiag;
    expectedDiag << "NONE   ";
    _collector->displayLocType(locType);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayDirectionalInd_Between()
  {
    const Indicator dir = 'B';
    std::stringstream expectedDiag;
    expectedDiag << "BETWEEN";
    _collector->displayDirectionalInd(dir);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayDirectionalInd_And()
  {
    const Indicator dir = 'A';
    std::stringstream expectedDiag;
    expectedDiag << "AND    ";
    _collector->displayDirectionalInd(dir);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayDirectionalInd_To()
  {
    const Indicator dir = 'T';
    std::stringstream expectedDiag;
    expectedDiag << "TO     ";
    _collector->displayDirectionalInd(dir);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayDirectionalInd_From()
  {
    const Indicator dir = 'F';
    std::stringstream expectedDiag;
    expectedDiag << "FROM   ";
    _collector->displayDirectionalInd(dir);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayDirectionalInd_None()
  {
    const Indicator dir = 'N';
    std::stringstream expectedDiag;
    expectedDiag << "NONE   ";
    _collector->displayDirectionalInd(dir);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayDirectionalInd_Unknown()
  {
    const Indicator dir = ' ';
    std::stringstream expectedDiag;
    expectedDiag << "UNKWN  ";
    _collector->displayDirectionalInd(dir);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayExclIncl_Include()
  {
    const Indicator excIncl = 'I';
    std::stringstream expectedDiag;
    expectedDiag << "INCLUDE\n";
    _collector->displayExclIncl(excIncl);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayExclIncl_Exclude()
  {
    const Indicator excIncl = 'E';
    std::stringstream expectedDiag;
    expectedDiag << "EXCLUDE\n";
    _collector->displayExclIncl(excIncl);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayExclIncl_Unknown()
  {
    const Indicator excIncl = 'A';
    std::stringstream expectedDiag;
    expectedDiag << "UNKWN  \n";
    _collector->displayExclIncl(excIncl);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testDisplayCabin()
  {
   Indicator cabin = 'W'; 
   std::vector<BookingCode> cxr =
   {"A ","B ","C ","D ","E ","F ","G ","H ","I ","J ","K ","L ","M ","N ","O ","P ","Q ","R ","S ","T ",
    "U ","V ","W ","X ","Y ","Z ","AA","AB","AC","AD","AE","AF","AG","AH","AI","AJ","AK","AL","AM","AN",
    "AO","AP","AQ","AR","AS","AT","AU","AV","AW","AX","AY","AZ","BA","CA","DA","EA","FA","GA","HA","IA",
    "JA","KA","LA","MA","NA"};

    std::stringstream expectedDiag;
    expectedDiag << " CABIN: W - PREMIUM ECONOMY CABIN \n"
                 << "   7"
                 << "        A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  \n"
             << "            Q  R  S  T  U  V  W  X  Y  Z  AA AB AC AD AE AF \n"
             << "            AG AH AI AJ AK AL AM AN AO AP AQ AR AS AT AU AV \n"
             << "            AW AX AY AZ BA CA DA EA FA GA HA IA JA KA LA MA \n"
             << "            NA \n";

    _collector->displayCabin(cabin, cxr);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFooter()
  {
    std::stringstream expectedDiag;
    expectedDiag << "*********************** END   DIAG 187 ***********************\n";
    _collector->printFooter();
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintRbdByCabinInfoShort()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);

    prepareRBDByCabinInfo(info, lk1, lk2);

    _collector->printRbdByCabinInfoShort(&info);

    std::stringstream expectedDiag;
    expectedDiag << " ATP  AA   111233   AT  01JAN2010  02JAN2010      ";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collector->str());
  }

  void testPrintRbdByCabinInfo_NoZone_NoCabin_Selected()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);

    prepareRBDByCabinInfo(info, lk1, lk2);

    _collector->printRbdByCabinInfo(*_trx, &info);

// "*************************************************************\n"
// " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n"
// "                              DISC          LAST\n"
// "-------------------------------------------------------------\n";
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

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collector->str());
  }


  void testPrintRbdByCabinInfo_Wrong_Cabin_Selected()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);

    prepareRBDByCabinInfo(info, lk1, lk2);
    _trx->diagnostic().diagParamMap().insert(std::make_pair("CB", "A"));

    _collector->printRbdByCabinInfo(*_trx, &info);

// "*************************************************************\n"
// " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n"
// "                              DISC          LAST\n"
// "-------------------------------------------------------------\n";
    std::stringstream expectedDiag;
    expectedDiag
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1:         EQUIPMENT: \n"
<< " FLIGHT 2: \n"
<< " GEO LOC1: C DFW \n"
<< " GEO LOC2: C FRA \n"
<< "  \n"
<< " CABIN A REQUESTED - CABIN NOT FOUND FOR CARRIER AA\n"
<< "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collector->str());
  }

  void testPrintRbdByCabinInfo_Wrong_Cabin_Coded()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);

    prepareRBDByCabinInfo(info, lk1, lk2);
    info.bookingCodeCabinMap().insert(std::make_pair("BB",'Z'));

    _collector->printRbdByCabinInfo(*_trx, &info);

// "*************************************************************\n"
// " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n"
// "                              DISC          LAST\n"
// "-------------------------------------------------------------\n";
    std::stringstream expectedDiag;
    expectedDiag
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1:         EQUIPMENT: \n"
<< " FLIGHT 2: \n"
<< " GEO LOC1: C DFW \n"
<< " GEO LOC2: C FRA \n"
<< "  \n"
<< " INVALID CABIN - Z - FOUND FOR CARRIER AA\n"
<< "  \n"
<< " CABIN: W - PREMIUM ECONOMY CABIN \n"
<< "   7        A1 B2 C3 D4 E5 \n"
<< "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collector->str());
  }

  void testPrintRbdByCabinInfo_Cabin_Not_Coded()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);

    prepareRBDByCabinInfo(info, lk1, lk2);
    info.bookingCodeCabinMap().clear();

    _collector->printRbdByCabinInfo(*_trx, &info);

// "*************************************************************\n"
// " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n"
// "                              DISC          LAST\n"
// "-------------------------------------------------------------\n";
    std::stringstream expectedDiag;
    expectedDiag
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1:         EQUIPMENT: \n"
<< " FLIGHT 2: \n"
<< " GEO LOC1: C DFW \n"
<< " GEO LOC2: C FRA \n"
<< "  \n"
<< " NO CABIN AND RBD DATA FOUND FOR CARRIER AA\n"
<< "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collector->str());
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

  void populateLocs(LocKey& lk1, LocKey& lk2)
  {
    lk1.loc() = "DFW";
    lk1.locType() = 'C';
    lk2.loc() = "FRA";
    lk2.locType() = 'C';
  }

  void populateZoneLocs(LocKey& lk1, LocKey& lk2)
  {
    lk1.loc() = "112322";
    lk1.locType() = 'Z';
    lk2.loc() = "112322";
    lk2.locType() = 'Z';
  }

  void testDisplayZoneDetail()
  {
    prepareZoneTable();
    std::string zone = "112322";
    _collectorM->displayZoneDetail(*_trx, ATPCO_VENDOR_CODE, zone);

// "*************************************************************\n"
// " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n"
// "                              DISC          LAST\n"
// "-------------------------------------------------------------\n";
    std::stringstream expectedDiag;
    expectedDiag << "                - - - - - - - - - - - - - - - - - - - - - -\n"
                 << "                VENDOR LOCTYPE  LOCATION  DIRECTN INCL/EXCL\n"
                 << "                - - - - - - - - - - - - - - - - - - - - - -\n"
                 << "                 ATP   CITY     DFW       NONE     INCLUDE\n"
                 << "                 ATP   NATION   AR        NONE     INCLUDE\n"
                 << "                 ATP   STATE    TX        NONE     EXCLUDE\n"
                 << "                 ATP   SUBAREA  21        NONE     INCLUDE\n"
                 << "                 ATP   FMSZONE  5KAD      NONE     EXCLUDE\n"
                 << "                 ATP   PCC ARC  34567     NONE     INCLUDE\n"
                 << "                 ATP   AREA     1         NONE     INCLUDE\n"
                 << "                 ATP   AIRPORT  ANK       NONE     INCLUDE\n"
                 << "                 ATP   ZONE     101       NONE     INCLUDE\n"
                 << "                 ATP   PCC      B4T0      NONE     INCLUDE\n"
                 << "                 ATP   USER     10101     NONE     INCLUDE\n"
                 << "                 ATP   NONE               NONE     INCLUDE\n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collectorM->str());
  }

  void testDisplayZoneDetail_Zone_Not_Found()
  {
    prepareZoneTable();
    std::string zone = "112322";
    _collector->displayZoneDetail(*_trx, ATPCO_VENDOR_CODE, zone);

// "*************************************************************\n"
// " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n"
// "                              DISC          LAST\n"
// "-------------------------------------------------------------\n";
    std::stringstream expectedDiag;
    expectedDiag << "  ZONE: 112322    VENDOR: ATP   NOT FOUND\n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collector->str());
  }


  void testPrintRbdByCabinInfo_With_Zone()
  {
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateZoneLocs(lk1, lk2);

    prepareRBDByCabinInfo(info, lk1, lk2);
    prepareZoneTable();
    std::string zone = "112322";

    _collectorM->printRbdByCabinInfo(*_trx, &info);

// "*************************************************************\n"
// " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n"
// "                              DISC          LAST\n"
// "-------------------------------------------------------------\n";
    std::stringstream expectedDiag;
    expectedDiag
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1:         EQUIPMENT: \n"
<< " FLIGHT 2: \n"
<< " GEO LOC1: Z 112322   \n"
<< " GEO LOC2: Z 112322   \n"
<< "  \n"
<< " CABIN: W - PREMIUM ECONOMY CABIN \n"
<< "   7        A1 B2 C3 D4 E5 \n"
<< "-------------------------------------------------------------\n";
//std::cout << "\nExpected length - " << expectedDiag.str().size() << std::endl;
//std::cout << "Actual  length - " << _collectorM->str().size() << std::endl;

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collectorM->str());
  }

  void testPrintRbdByCabinInfo_With_Zone_Detail()
  {
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "ZONE"));
    RBDByCabinInfo info;
    LocKey lk1, lk2;
    populateZoneLocs(lk1, lk2);

    prepareRBDByCabinInfo(info, lk1, lk2);
    prepareZoneTable();
    std::string zone = "112322";

    _collectorM->printRbdByCabinInfo(*_trx, &info);

// "*************************************************************\n"
// " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n"
// "                              DISC          LAST\n"
// "-------------------------------------------------------------\n";
    std::stringstream expectedDiag;
    expectedDiag
<< " ATP    AA    111233     AT   01JAN2010     02JAN2010 \n"
<< "                              31DEC2025     31DEC2025 \n"
<< " FLIGHT 1:         EQUIPMENT: \n"
<< " FLIGHT 2: \n"
<< " GEO LOC1: Z 112322   \n"
<< "                - - - - - - - - - - - - - - - - - - - - - -\n"
<< "                VENDOR LOCTYPE  LOCATION  DIRECTN INCL/EXCL\n"
<< "                - - - - - - - - - - - - - - - - - - - - - -\n"
<< "                 ATP   CITY     DFW       NONE     INCLUDE\n"
<< "                 ATP   NATION   AR        NONE     INCLUDE\n"
<< "                 ATP   STATE    TX        NONE     EXCLUDE\n"
<< "                 ATP   SUBAREA  21        NONE     INCLUDE\n"
<< "                 ATP   FMSZONE  5KAD      NONE     EXCLUDE\n"
<< "                 ATP   PCC ARC  34567     NONE     INCLUDE\n"
<< "                 ATP   AREA     1         NONE     INCLUDE\n"
<< "                 ATP   AIRPORT  ANK       NONE     INCLUDE\n"
<< "                 ATP   ZONE     101       NONE     INCLUDE\n"
<< "                 ATP   PCC      B4T0      NONE     INCLUDE\n"
<< "                 ATP   USER     10101     NONE     INCLUDE\n"
<< "                 ATP   NONE               NONE     INCLUDE\n"
<< " GEO LOC2: Z 112322   \n"
<< "                SAME ZONE AS ABOVE\n"
<< "  \n"
<< " CABIN: W - PREMIUM ECONOMY CABIN \n"
<< "   7        A1 B2 C3 D4 E5 \n"
<< "-------------------------------------------------------------\n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collectorM->str());
  }

  void testPrintAirSegData_With_SingleRBD_Flignt_Segment()
  {
    AirSeg seg;
    CarrierCode cxr = "BB";
    createAirSegment(seg, cxr);
    BookingCode bkc = "A ";
    std::vector<BookingCode> bks;
    bool open = false;

    _collector->printAirSegData(seg, cxr, bkc, seg.departureDT(), seg.departureDT(), open, bks);

    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: FLIGHT SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: BB  MARKET: DFW-LON  TRAVEL ON: 19SEP2015 \n"
    << "    RBD: A \n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collector->str());
  }

  void testPrintAirSegData_With_SingleRBD_Open_Segment()
  {
    AirSeg seg;
    CarrierCode cxr = "BB";
    createAirSegment(seg, cxr);
    seg.segmentType() = Open;
    BookingCode bkc = "A ";
    std::vector<BookingCode> bks;
    bool open = false;

    _collector->printAirSegData(seg, cxr, bkc, seg.departureDT(), seg.departureDT(), open, bks);

    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: OPEN SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: BB  MARKET: DFW-LON  TRAVEL ON: 19SEP2015 \n"
    << "    RBD: A \n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collector->str());
  }

  void testPrintAirSegData_With_SingleRBD_NoGEO_Segment()
  {
    AirSeg seg;
    CarrierCode cxr = "BB";
    seg.departureDT() = DateTime(2015, 9, 19);

    BookingCode bkc = "A ";
    std::vector<BookingCode> bks;
    bool open = false;

    _collector->printAirSegData(seg, cxr, bkc, seg.departureDT(), seg.departureDT(), open, bks);

    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: FLIGHT SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: BB  MARKET:  EMPTY   TRAVEL ON: 19SEP2015 \n"
    << "    RBD: A \n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collector->str());
  }

  void testPrintAirSegData_With_SingleRBD_partial_GEO_Segment()
  {
    AirSeg seg;
    CarrierCode cxr = "BB";
    seg.departureDT() = DateTime(2015, 9, 19);
    seg.origin() = _locDFW;

    BookingCode bkc = "A ";
    std::vector<BookingCode> bks;
    bool open = false;

    _collector->printAirSegData(seg, cxr, bkc, seg.departureDT(), seg.departureDT(), open, bks);

    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: FLIGHT SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: BB  MARKET: DFW-     TRAVEL ON: 19SEP2015 \n"
    << "    RBD: A \n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collector->str());
  }

  void testPrintAirSegData_With_SegmentDataOnly()
  {
    AirSeg seg;
    CarrierCode cxr = "BB";
    createAirSegment(seg, cxr);
    BookingCode bkc = "A ";
    seg.setBookingCode(bkc);
    bool open = false;

    _collector->printAirSegData(seg, seg.departureDT(), seg.departureDT(), open);

    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: FLIGHT SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: BB  MARKET: DFW-LON  TRAVEL ON: 19SEP2015 \n"
    << "    RBD: A \n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collector->str());
  }

  void testPrintAirSegData_With_SegmentData_Bkc_vector()
  {
    AirSeg seg;
    CarrierCode cxr = "BB";
    createAirSegment(seg, cxr);
    BookingCode bkc = "A ";
    seg.setBookingCode(bkc);
    std::vector<BookingCode> bks;
    bks.push_back("Z1");
    bks.push_back("Y1");

    _collector->printAirSegData(seg, seg.departureDT(), bks, seg.departureDT());

    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: FLIGHT SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: BB  MARKET: DFW-LON  TRAVEL ON: 19SEP2015 \n"
    << "    RBD: Z1 Y1 \n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collector->str());
  }

  void testPrintAirSegData_With_Ptfare_Bkc_vector()
  {
    AirSeg seg;
    PaxTypeFare ptf;
    FareMarket mkt;
    mkt.travelSeg().push_back(&seg);
    ptf.fareMarket() = &mkt;
    mkt.governingCarrier() = "BB";
    CarrierCode cxr = "BB";
    createAirSegment(seg, cxr);
    BookingCode bkc = "A ";
    seg.setBookingCode(bkc);

    FareInfo fareInfo;
    fareInfo.fareClass() = "Y26";
    Fare fare;

    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, mkt, &tCRInfo);
    ptf.setFare(&fare);


    std::vector<BookingCode> bks;
    bks.push_back("Z1");
    bks.push_back("Y1");

    _collector->printAirSegData(ptf, seg.departureDT(), bks, seg.departureDT());

    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: FLIGHT SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: BB  MARKET: DFW-LON  TRAVEL ON: 19SEP2015 \n"
    << "    RBD: Z1 Y1   FARE CLASS: Y26     \n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collector->str());
  }

  void testPrintAirSegData_With_WPNCS_Ptfare()
  {
    AirSeg seg;
    PaxTypeFare ptf;
    FareMarket mkt;
    mkt.travelSeg().push_back(&seg);
    ptf.fareMarket() = &mkt;
    mkt.governingCarrier() = "BB";
    CarrierCode cxr = "BB";
    createAirSegment(seg, cxr);
    BookingCode bkc = "A ";
    seg.setBookingCode(bkc);

    FareInfo fareInfo;
    fareInfo.fareClass() = "Y26";
    Fare fare;

    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, mkt, &tCRInfo);
    ptf.setFare(&fare);

    uint32_t item = 1;
    BookingCodeExceptionSegment bce;
    bce.segNo() = 1001;
    bce.restrictionTag() = 'R';
    bool open = false;

    _collector->printAirSegData(seg, bkc, seg.departureDT(), ptf, item, bce, seg.departureDT(), open);

    std::stringstream expectedDiag;
    expectedDiag
    << " SOURCE DATA: FLIGHT SEGMENT   TRANSACTION DATE: 19SEP2015 \n"
    << "    CXR: BB  MARKET: DFW-LON  TRAVEL ON: 19SEP2015 \n"
    << "    RBD: A \n"
    << "    FARE CLASS: Y26         FARE MARKET: DFW-LON\n"
    << "    T999 ITEM: 1   SEQ: 1001      RESTR TAG: R\n";

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _collector->str());
  }


  void createAirSegment(AirSeg& air, CarrierCode cxr)
  {
    air.origin() = _locDFW;
    air.origAirport() = _locDFW->loc();
    air.boardMultiCity() = _locDFW->loc();
    air.origAirport() = _locDFW->loc();

    air.destination() = _locLON;
    air.destAirport() = _locLON->loc();
    air.offMultiCity()= _locLON->loc();
    air.destAirport() = _locLON->loc();

    air.departureDT() = DateTime(2015, 9, 19);
    air.carrier() = cxr;
    air.setOperatingCarrierCode(cxr);
    air.setMarketingCarrierCode(cxr);
  }

  void prepareZoneTable()
  {
    _collectorM->_zoneInfo.vendor() = "ATP";

    ZoneInfo::ZoneSeg zs1;
    ZoneInfo::ZoneSeg zs2;
    ZoneInfo::ZoneSeg zs3;
    ZoneInfo::ZoneSeg zs4;
    ZoneInfo::ZoneSeg zs11;
    ZoneInfo::ZoneSeg zs21;
    ZoneInfo::ZoneSeg zs31;
    ZoneInfo::ZoneSeg zs41;
    ZoneInfo::ZoneSeg zs12;
    ZoneInfo::ZoneSeg zs22;
    ZoneInfo::ZoneSeg zs32;
    ZoneInfo::ZoneSeg zs42;

    zs1.locType() = 'C';
    zs1.loc() = "DFW";
    zs1.directionalQualifier() = 'N';
    zs1.inclExclInd() = 'I';

    zs2.locType() = 'N';
    zs2.loc() = "AR";
    zs2.directionalQualifier() = 'N';
    zs2.inclExclInd() = 'I';

    zs3.locType() = 'A';
    zs3.loc() = "1";
    zs3.directionalQualifier() = 'N';
    zs3.inclExclInd() = 'I';

    zs4.locType() = 'P';
    zs4.loc() = "ANK";
    zs4.directionalQualifier() = 'N';
    zs4.inclExclInd() = 'I';

    zs11.locType() = 'S';
    zs11.loc() = "TX";
    zs11.directionalQualifier() = 'N';
    zs11.inclExclInd() = 'E';

    zs21.locType() = '*';
    zs21.loc() = "21";
    zs21.directionalQualifier() = 'N';
    zs21.inclExclInd() = 'I';

    zs31.locType() = 'Z';
    zs31.loc() = "101";
    zs31.directionalQualifier() = 'N';
    zs31.inclExclInd() = 'I';

    zs41.locType() = 'X';
    zs41.loc() = "B4T0";
    zs41.directionalQualifier() = 'N';
    zs41.inclExclInd() = 'I';

    zs12.locType() = '1';
    zs12.loc() = "5KAD";
    zs12.directionalQualifier() = 'N';
    zs12.inclExclInd() = 'E';

    zs22.locType() = 'Y';
    zs22.loc() = "34567";
    zs22.directionalQualifier() = 'N';
    zs22.inclExclInd() = 'I';

    zs32.locType() = 'U';
    zs32.loc() = "10101";
    zs32.directionalQualifier() = 'N';
    zs32.inclExclInd() = 'I';

    zs42.locType() = ' ';
    zs42.loc() = "";
    zs42.directionalQualifier() = 'N';
    zs42.inclExclInd() = 'I';

    std::vector<ZoneInfo::ZoneSeg> zsv1;
    std::vector<ZoneInfo::ZoneSeg> zsv2;

    zsv1.push_back(zs1);
    zsv1.push_back(zs2);
    zsv1.push_back(zs11);
    zsv1.push_back(zs21);
    zsv1.push_back(zs12);
    zsv1.push_back(zs22);
    zsv2.push_back(zs3);
    zsv2.push_back(zs4);
    zsv2.push_back(zs31);
    zsv2.push_back(zs41);
    zsv2.push_back(zs32);
    zsv2.push_back(zs42);

    _collectorM->_zoneInfo.sets().push_back(zsv1);
    _collectorM->_zoneInfo.sets().push_back(zsv2);
  }

protected:
  Diag187Collector* _collector;
  Diag187CollectorTestMock* _collectorM;
  TestMemHandle _memHandle;
  PricingTrx* _trx;

  const Loc*  _locDFW;
  const Loc*  _locLON;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag187CollectorTest);
} // NS tse
