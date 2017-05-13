#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag875Collector.h"

#include <time.h>
#include <iostream>
#include <utility>

#include "DataModel/PricingTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/ItinHelperStructs.h"
#include "ServiceFees/CarrierStrategy.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/EmdInterlineAgreementInfo.h"

#include "Diagnostic/Diagnostic.h"
#include "Common/TseEnums.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include <unistd.h>
#include "Common/Config/ConfigMan.h"

using namespace std;

namespace tse
{
class Diag875CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag875CollectorTest);
  CPPUNIT_TEST(testPrintS5CommonHeader);
  CPPUNIT_TEST(testPrintCanNotCollectOC_NOT_ACTIVE_YET);
  CPPUNIT_TEST(testPrintPccIsDeactivated);
  CPPUNIT_TEST(testPrintCanNotCollectANCILLARY_NOT_ACTIVE);
  CPPUNIT_TEST(testPrintCanNotCollectANCILLARY_WP_DISPLAY_NOT_ACTIVE);
  CPPUNIT_TEST(testPrintS5NotFound_Marketing_Cxr);
  CPPUNIT_TEST(testPrintS5NotFound_Operating_Cxr);
  CPPUNIT_TEST(testPrintS5NotFound_Partition_Cxr);
  CPPUNIT_TEST(testPrintNoGroupCodeProvided);
  CPPUNIT_TEST(testPrintJourneyDestination);
  CPPUNIT_TEST(testDisplayActiveCxrGroupHeader);
  CPPUNIT_TEST(testDisplayCxrNoMAdata);
  CPPUNIT_TEST(testDisplayActiveCxrGroup_Not_Active);
  CPPUNIT_TEST(testDisplayActiveCxrGroup_Active);
  CPPUNIT_TEST(testDisplayNoActiveCxrGroupForOC);
  CPPUNIT_TEST(testDisplayMrktDrivenCxrHeader);
  CPPUNIT_TEST(testDisplayMrktDrivenNoMCdata);
  CPPUNIT_TEST(testDisplayMrktDrivenCxrData);
  CPPUNIT_TEST(testDisplayMrktDrivenCxrData_Alt_Prc_N);
  CPPUNIT_TEST(testPrintNoOptionsRequested);

  CPPUNIT_TEST(testPrintTravelPortionHeader_Cities);
  CPPUNIT_TEST(testPrintTravelPortionHeader_Airports);

  CPPUNIT_TEST(testPrintS5SubCodeStatus_PASS);
  CPPUNIT_TEST(testPrintS5SubCodeStatus_Fail_Tkt_Date);
  CPPUNIT_TEST(testPrintS5SubCodeStatus_Fail_Service);
  CPPUNIT_TEST(testPrintS5SubCodeStatus_Fail_Industry_Ind);
  CPPUNIT_TEST(testPrintS5SubCodeStatus_Fail_Display_Cat);
  CPPUNIT_TEST(testPrintS5SubCodeStatus_Fail_EMD_Type);
  CPPUNIT_TEST(testPrintS5SubCodeStatus_Fail_S7);
  CPPUNIT_TEST(testDisplayStatus_Unknown);
  CPPUNIT_TEST(testDisplayConcurInd_Flight);
  CPPUNIT_TEST(testDisplayConcurInd_Rule);
  CPPUNIT_TEST(testDisplayConcurInd_Other);
  CPPUNIT_TEST(testDisplayBookingInd_SSR);
  CPPUNIT_TEST(testDisplayBookingInd_AUX);
  CPPUNIT_TEST(testDisplayBookingInd_ASK);
  CPPUNIT_TEST(testDisplayBookingInd_No_BKG);
  CPPUNIT_TEST(testDisplayBookingInd_Not_Defined);
  CPPUNIT_TEST(testDisplayVendor_ATP_xchar);
  CPPUNIT_TEST(testDisplayVendor_ATP_1char);
  CPPUNIT_TEST(testDisplayVendor_MMGR_xchar);
  CPPUNIT_TEST(testDisplayVendor_MMGR_1char);
  CPPUNIT_TEST(testDisplayVendor_OTHER_xchar);
  CPPUNIT_TEST(testDisplayVendor_OTHER_1char);
  CPPUNIT_TEST(testPrintS5SubCodeInfo_CXR_Marketing);
  CPPUNIT_TEST(testPrintS5SubCodeInfo_CXR_Operating);
  CPPUNIT_TEST(testPrintS5SubCodeInfo_CXR_Partition);
  CPPUNIT_TEST(testPrintDetailS5Info);
  CPPUNIT_TEST(testPrintGroupCodesInTheRequest_RT_INTL_GroupRequested);
  CPPUNIT_TEST(testPrintGroupCodesInTheRequest_OW_DOM_Group_NOT_Requested);
  CPPUNIT_TEST(testPrintGroupCodesInTheRequest_RT_INTL_GroupRequested_Historical_ON);
  CPPUNIT_TEST(testGetCarrierListAsString);
  CPPUNIT_TEST(testPrintDetailInterlineEmdProcessingS5Info);
  CPPUNIT_TEST(testPrintDetailInterlineEmdProcessingStatusS5Info);
  CPPUNIT_TEST(testPrintNoInterlineDataFoundInfo);
  CPPUNIT_TEST(testPrintDetailInterlineEmdAgreementInfo);
  CPPUNIT_TEST_SUITE_END();

private:
  Diag875Collector* _diag;
  Diagnostic* _diagroot;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest* _request;
  PricingOptions* _pOptions;
  Itin* _itin;
  SubCodeInfo* _sci;
  ServiceFeesGroup* _sfg;
  MerchCarrierPreferenceInfo* _mcpInfo;

public:
  //---------------------------------------------------------------------
  // testConstructor()
  //---------------------------------------------------------------------
  void testConstructor()
  {
    try
    {
      Diag875Collector diag;
      CPPUNIT_ASSERT_EQUAL(string(""), diag.str());
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void setUp()
  {
    try
    {
      _memHandle.create<TestConfigInitializer>();
      _diagroot = _memHandle.insert(new Diagnostic(Diagnostic875));
      _diagroot->activate();
      _diag = _memHandle.insert(new Diag875Collector(*_diagroot));
      _diag->enable(Diagnostic875);
      _trx = _memHandle.create<PricingTrx>();
      _itin = _memHandle.create<Itin>();
      _trx->itin().push_back(_itin);
      _request = _memHandle.create<PricingRequest>();
      _trx->setRequest(_request);
      _request->ticketingDT() = DateTime(2010, 5, 1);
      _pOptions = _memHandle.insert(new PricingOptions);
      _pOptions->currencyOverride() = "USD";
      _trx->setOptions(_pOptions);
      _sfg = _memHandle.create<ServiceFeesGroup>();
      _sci = _memHandle.create<SubCodeInfo>();
      _mcpInfo = _memHandle.create<MerchCarrierPreferenceInfo>();
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }
  void tearDown() { _memHandle.clear(); }

  void createSubCodeInfo()
  {
    _sci->vendor() = "ATP";
    _sci->carrier() = "AB";
    _sci->fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    _sci->industryCarrierInd() = 'I';
    _sci->serviceGroup() = "UU";
    _sci->serviceSubGroup() = "XX";
    _sci->serviceSubTypeCode() = "0A0";
    _sci->effDate() = DateTime(2010, 1, 1);
    _sci->discDate() = DateTime(2020, 1, 1);
    _sci->description1() = "UN";
    _sci->concur() = 'X';
    _sci->rfiCode() = 'A';
    _sci->ssrCode() = "SSR";
    _sci->displayCat() = '1';
    _sci->bookingInd() = "04";
    _sci->ssimCode() = 'N';
    _sci->emdType() = '4';
    _sci->commercialName() = "CPPUNIT TEST";
    _sci->taxTextTblItemNo() = 101010;
    _sci->pictureNo() = 202;
    _sci->consumptionInd() = 'Y';
  }

  void createmerchCxrPrefInfo()
  {
    _mcpInfo->carrier() = "US";
    _mcpInfo->prefVendor() = "USOC";
    _mcpInfo->altProcessInd() = 'Y';
    _mcpInfo->groupCode() = "SA";
    _mcpInfo->sectorPortionInd() = 'S';
    _mcpInfo->concurrenceInd() = 'N';
  }

  void createmerchCxrPrefInfo_AltPrc_N()
  {
    _mcpInfo->carrier() = "US";
    _mcpInfo->prefVendor() = "USOC";
    _mcpInfo->altProcessInd() = 'N';
    _mcpInfo->groupCode() = "SA";
    _mcpInfo->sectorPortionInd() = 'S';
    _mcpInfo->concurrenceInd() = 'N';
  }

  void createServiceFeesGroup()
  {
    _sfg->groupCode() = "ML";
    _itin->ocFeesGroup().push_back(_sfg);
  }

  void testPrintS5CommonHeader()
  {
    _diag->printS5CommonHeader();
    CPPUNIT_ASSERT_EQUAL(string("V CXR   SERVICE  IND GRP  SUBGRP SUBCODE STATUS\n"), _diag->str());
  }

  void testPrintCanNotCollectOC_NOT_ACTIVE_YET()
  {
    _diag->printCanNotCollect(OC_NOT_ACTIVE_YET);
    CPPUNIT_ASSERT_EQUAL(string(" *** NOT PROCESSED - TICKETING DATE BEFORE OC ACTIVATION DATE\n"),
                         _diag->str());
  }

  void testPrintPccIsDeactivated()
  {
    PseudoCityCode pcc = "B4T0";
    _diag->printPccIsDeactivated(pcc);
    CPPUNIT_ASSERT_EQUAL(string(" *** NOT PROCESSED - AGENCY PCC B4T0 IS NOT ACTIVE ***\n"),
                         _diag->str());
  }

  void testPrintCanNotCollectANCILLARY_NOT_ACTIVE()
  {
    _diag->printCanNotCollect(ANCILLARY_NOT_ACTIVE);
    CPPUNIT_ASSERT_EQUAL(string(" *** NOT PROCESSED - ANCILLARY FEES ARE NOT ACTIVATED\n"),
                         _diag->str());
  }

  void testPrintCanNotCollectANCILLARY_WP_DISPLAY_NOT_ACTIVE()
  {
    _diag->printCanNotCollect(ANCILLARY_WP_DISPLAY_NOT_ACTIVE);
    CPPUNIT_ASSERT_EQUAL(string(" *** NOT PROCESSED - WP*AE ANCILLARY FEES ARE NOT ACTIVATED\n"),
                         _diag->str());
  }

  void testPrintS5NotFound_Marketing_Cxr()
  {
    ServiceGroup group = "BG";
    CarrierCode cxr = "KL";
    MarketingCarrierStrategy carrierStrategy;
    _diag->printS5NotFound(group, cxr, carrierStrategy.getCarrierStrategyTypeShort());
    CPPUNIT_ASSERT_EQUAL(string("  KL M               BG                    DATA NOT FOUND\n"),
                         _diag->str());
  }

  void testPrintS5NotFound_Operating_Cxr()
  {
    ServiceGroup group = "BG";
    CarrierCode cxr = "KL";
    OperatingCarrierStrategy carrierStrategy;
    _diag->printS5NotFound(group, cxr, carrierStrategy.getCarrierStrategyTypeShort());
    CPPUNIT_ASSERT_EQUAL(string("  KL O               BG                    DATA NOT FOUND\n"),
                         _diag->str());
  }

  void testPrintS5NotFound_Partition_Cxr()
  {
    ServiceGroup group = "BG";
    CarrierCode cxr = "KL";
    PartitionCarrierStrategy carrierStrategy;
    _diag->printS5NotFound(group, cxr, carrierStrategy.getCarrierStrategyTypeShort());
    CPPUNIT_ASSERT_EQUAL(string("  KL P               BG                    DATA NOT FOUND\n"),
                         _diag->str());
  }

  void testPrintNoGroupCodeProvided()
  {
    _diag->printNoGroupCodeProvided();
    CPPUNIT_ASSERT_EQUAL(string(" *** NOT PROCESSED - NO GROUP CODE PROVIDED ***\n"), _diag->str());
  }

  void testPrintJourneyDestination()
  {
    _diag->printJourneyDestination("DFW");
    CPPUNIT_ASSERT_EQUAL(string("-------------- JOURNEY DEST/TURNAROUND: DFW ------------------\n"),
                         _diag->str());
  }

  void testDisplayActiveCxrGroupHeader()
  {
    _diag->displayActiveCxrGroupHeader();
    CPPUNIT_ASSERT_EQUAL(string("\n-------------- CARRIER MERCH ACTIVATION DATA ----------------- "
                                "\n CXR  ACTIVE        ACTIVE GROUP\n"),
                         _diag->str());
  }

  void testDisplayCxrNoMAdata()
  {
    _diag->displayCxrNoMAdata("GG");
    CPPUNIT_ASSERT_EQUAL(string(" GG   NO MERCH ACTIVATION DATA \n"), _diag->str());
  }

  void testDisplayActiveCxrGroup_Not_Active()
  {
    _diag->displayActiveCxrGroup("GG", false, "BG ML IE");
    CPPUNIT_ASSERT_EQUAL(string(" GG     N           BG ML IE\n"), _diag->str());
  }

  void testDisplayActiveCxrGroup_Active()
  {
    _diag->displayActiveCxrGroup("GG", true, "BG ML IE");
    CPPUNIT_ASSERT_EQUAL(string(" GG     Y           BG ML IE\n"), _diag->str());
  }

  void testDisplayNoActiveCxrGroupForOC()
  {
    _diag->displayNoActiveCxrGroupForOC();
    CPPUNIT_ASSERT_EQUAL(
        string("\n------------NO CARRIERS ACTIVE FOR OC PROCESSING-------------- \n"),
        _diag->str());
  }

  void testDisplayMrktDrivenCxrHeader()
  {
    _diag->displayMrktDrivenCxrHeader();
    CPPUNIT_ASSERT_EQUAL(
        string("\n--------------- MERCH CARRIER PREFERENCE DATA ----------------\n CXR  VENDOR  "
               "ALTPROCESS  GROUP   SECTORPORTION  CONCURRENCE \n"),
        _diag->str());
  }

  void testDisplayMrktDrivenNoMCdata()
  {
    _diag->displayMrktDrivenCxrNoData();
    CPPUNIT_ASSERT_EQUAL(string(" NO DATA FOUND\n"), _diag->str());
  }

  void testDisplayMrktDrivenCxrData()
  {
    std::vector<MerchCarrierPreferenceInfo*> mCxrPrefVec;
    createmerchCxrPrefInfo();
    mCxrPrefVec.push_back(_mcpInfo);
    _diag->displayMrktDrivenCxrData(mCxrPrefVec);
    CPPUNIT_ASSERT_EQUAL(string(" US     USOC      Y         SA           S             N      \n"),
                         _diag->str());
  }

  void testDisplayMrktDrivenCxrData_Alt_Prc_N()
  {
    std::vector<MerchCarrierPreferenceInfo*> mCxrPrefVec;
    createmerchCxrPrefInfo_AltPrc_N();
    mCxrPrefVec.push_back(_mcpInfo);
    _diag->displayMrktDrivenCxrData(mCxrPrefVec);
    CPPUNIT_ASSERT_EQUAL(string(" US     USOC      N         SA           S             N      \n"),
                         _diag->str());
  }

  void testPrintNoOptionsRequested()
  {
    _diag->printNoOptionsRequested();
    CPPUNIT_ASSERT_EQUAL(string(" *** NOT PROCESSED - REQUESTED OPTIONS = 0 ***\n"), _diag->str());
  }

  void prepareCityAirports(AirSeg& seg1,
                           AirSeg& seg2,
                           std::set<tse::CarrierCode>& marketingCxr,
                           std::set<tse::CarrierCode>& operatingCxr,
                           FarePath& fP,
                           PaxType& pt)
  {
    seg1.boardMultiCity() = "NYC";
    seg1.origAirport() = "JFK";
    seg2.offMultiCity() = "DFW";
    seg2.destAirport() = "DFW";
    marketingCxr.insert("AD");
    operatingCxr.insert("DA");
    fP.paxType() = &pt;
    pt.paxType() = "ADT";
  }

  void testPrintTravelPortionHeader_Cities()
  {
    AirSeg seg1, seg2;
    std::set<tse::CarrierCode> marketingCxr, operatingCxr;
    FarePath fP;
    PaxType pt;
    prepareCityAirports(seg1, seg2, marketingCxr, operatingCxr, fP, pt);
    _diag->printTravelPortionHeader(seg1, seg2, marketingCxr, operatingCxr, fP, *_trx);
    CPPUNIT_ASSERT_EQUAL(
        string("------------ PORTION OF TRAVEL : NYC 0  - DFW 0  ------------\nMARKETING  CXR     "
               ": AD \nOPERATING  CXR     : DA \nREQUESTED PAX TYPE : "
               "ADT\n-------------------------------------------------------------- \n             "
               "     S5 RECORDS DATA PROCESSING\n"),
        _diag->str());
  }

  void testPrintTravelPortionHeader_Airports()
  {
    AirSeg seg1, seg2;
    std::set<tse::CarrierCode> marketingCxr, operatingCxr;
    FarePath fP;
    PaxType pt;
    prepareCityAirports(seg1, seg2, marketingCxr, operatingCxr, fP, pt);
    seg1.boardMultiCity() = "";
    seg2.offMultiCity() = "";

    _diag->printTravelPortionHeader(seg1, seg2, marketingCxr, operatingCxr, fP, *_trx);
    CPPUNIT_ASSERT_EQUAL(
        string("------------ PORTION OF TRAVEL : JFK 0  - DFW 0  ------------\nMARKETING  CXR     "
               ": AD \nOPERATING  CXR     : DA \nREQUESTED PAX TYPE : "
               "ADT\n-------------------------------------------------------------- \n             "
               "     S5 RECORDS DATA PROCESSING\n"),
        _diag->str());
  }

  void testPrintS5SubCodeStatus_PASS()
  {
    _diag->printS5SubCodeStatus(PASS_S5);
    CPPUNIT_ASSERT_EQUAL(string("                              S5 STATUS : PASS\n"), _diag->str());
  }

  void testPrintS5SubCodeStatus_Fail_Tkt_Date()
  {
    _diag->printS5SubCodeStatus(FAIL_S5_TKT_DATE);
    CPPUNIT_ASSERT_EQUAL(string("                              S5 STATUS : FAIL TKT DATE\n"),
                         _diag->str());
  }

  void testPrintS5SubCodeStatus_Fail_Service()
  {
    _diag->printS5SubCodeStatus(FAIL_S5_FLIGHT_RELATED);
    CPPUNIT_ASSERT_EQUAL(string("                              S5 STATUS : SVC NOT PROCESSED\n"),
                         _diag->str());
  }

  void testPrintS5SubCodeStatus_Fail_Industry_Ind()
  {
    _diag->printS5SubCodeStatus(FAIL_S5_INDUSTRY_INDICATOR);
    CPPUNIT_ASSERT_EQUAL(string("                              S5 STATUS : FAIL INDUSTRY IND\n"),
                         _diag->str());
  }

  void testPrintS5SubCodeStatus_Fail_Display_Cat()
  {
    _diag->printS5SubCodeStatus(FAIL_S5_DISPLAY_CATEGORY);
    CPPUNIT_ASSERT_EQUAL(string("                              S5 STATUS : FAIL DISPLAY CAT\n"),
                         _diag->str());
  }

  void testPrintS5SubCodeStatus_Fail_EMD_Type()
  {
    _diag->printS5SubCodeStatus(FAIL_S5_EMD_TYPE);
    CPPUNIT_ASSERT_EQUAL(string("                              S5 STATUS : FAIL EMD TYPE\n"),
                         _diag->str());
  }

  void testPrintS5SubCodeStatus_Fail_S7()
  {
    _diag->printS5SubCodeStatus(FAIL_S7);
    CPPUNIT_ASSERT_EQUAL(string("                              S5 STATUS : FAIL S7\n"),
                         _diag->str());
  }

  void testDisplayStatus_Unknown()
  {
    _diag->displayStatus((StatusS5Validation) - 1);
    CPPUNIT_ASSERT_EQUAL(string("UNKNOWN STATUS\n"), _diag->str());
  }

  void testDisplayConcurInd_Flight()
  {
    _diag->displayConcurInd('1');
    CPPUNIT_ASSERT_EQUAL(string("FLIGHT   "), _diag->str());
  }

  void testDisplayConcurInd_Rule()
  {
    _diag->displayConcurInd('2');
    CPPUNIT_ASSERT_EQUAL(string("RULE     "), _diag->str());
  }

  void testDisplayConcurInd_Other()
  {
    _diag->displayConcurInd('3');
    CPPUNIT_ASSERT_EQUAL(string("X        "), _diag->str());
  }

  void testDisplayBookingInd_SSR()
  {
    _diag->displayBookingInd("01");
    CPPUNIT_ASSERT_EQUAL(string("SSR      "), _diag->str());
  }

  void testDisplayBookingInd_AUX()
  {
    _diag->displayBookingInd("02");
    CPPUNIT_ASSERT_EQUAL(string("AUX SEGM "), _diag->str());
  }

  void testDisplayBookingInd_ASK()
  {
    _diag->displayBookingInd("03");
    CPPUNIT_ASSERT_EQUAL(string("ASK CXR  "), _diag->str());
  }

  void testDisplayBookingInd_No_BKG()
  {
    _diag->displayBookingInd("04");
    CPPUNIT_ASSERT_EQUAL(string("NO BKG RQ"), _diag->str());
  }

  void testDisplayBookingInd_Not_Defined()
  {
    _diag->displayBookingInd("05");
    CPPUNIT_ASSERT_EQUAL(string("         "), _diag->str());
  }

  void testDisplayVendor_ATP_xchar()
  {
    _diag->displayVendor("ATP", true);
    CPPUNIT_ASSERT_EQUAL(string("ATP "), _diag->str());
  }

  void testDisplayVendor_ATP_1char()
  {
    _diag->displayVendor("ATP", false);
    CPPUNIT_ASSERT_EQUAL(string("A"), _diag->str());
  }

  void testDisplayVendor_MMGR_xchar()
  {
    _diag->displayVendor("MMGR", true);
    CPPUNIT_ASSERT_EQUAL(string("MM  "), _diag->str());
  }

  void testDisplayVendor_MMGR_1char()
  {
    _diag->displayVendor("MMGR", false);
    CPPUNIT_ASSERT_EQUAL(string("M"), _diag->str());
  }

  void testDisplayVendor_OTHER_xchar()
  {
    _diag->displayVendor("SITA", true);
    CPPUNIT_ASSERT_EQUAL(string("    "), _diag->str());
  }

  void testDisplayVendor_OTHER_1char()
  {
    _diag->displayVendor("SITA", false);
    CPPUNIT_ASSERT_EQUAL(string(" "), _diag->str());
  }

  void testPrintS5SubCodeInfo_CXR_Marketing()
  {
    createSubCodeInfo();
    MarketingCarrierStrategy carrierStrategy;
    _diag->printS5SubCodeInfo(_sci, carrierStrategy.getCarrierStrategyTypeShort(), PASS_S5);
    CPPUNIT_ASSERT_EQUAL(string("A AB M  FLIGHT    I  UU     XX     0A0     PASS\n"), _diag->str());
  }

  void testPrintS5SubCodeInfo_CXR_Operating()
  {
    createSubCodeInfo();
    OperatingCarrierStrategy carrierStrategy;
    _diag->printS5SubCodeInfo(_sci, carrierStrategy.getCarrierStrategyTypeShort(), PASS_S5);
    CPPUNIT_ASSERT_EQUAL(string("A AB O  FLIGHT    I  UU     XX     0A0     PASS\n"), _diag->str());
  }

  void testPrintS5SubCodeInfo_CXR_Partition()
  {
    createSubCodeInfo();
    PartitionCarrierStrategy carrierStrategy;
    _diag->printS5SubCodeInfo(_sci, carrierStrategy.getCarrierStrategyTypeShort(), PASS_S5);
    CPPUNIT_ASSERT_EQUAL(string("A AB P  FLIGHT    I  UU     XX     0A0     PASS\n"), _diag->str());
  }

  void testPrintDetailS5Info()
  {
    createSubCodeInfo();
    MarketingCarrierStrategy carrierStrategy;
    _diag->printDetailS5Info(_sci, carrierStrategy.getCarrierStrategyTypeShort());
    CPPUNIT_ASSERT_EQUAL(
        string("------------------ SUB CODE DETAILED INFO --------------------\n SUB CODE : 0A0   "
               "VN : ATP  CXR : AB M  EFF DATE : 2010-01-01\n SERVICE  : FLIGHT                    "
               "  DISC DATE: 2020-01-01\n  \n GROUP    : UU     SUBGROUP : XX  DESCR1 : UN  DESCR2 "
               ":    \n  \n INDUSTRY/CXR : I    CONCUR : X         RFIC : A  SSR :SSR \n DISPLAY "
               "CAT  : 1   BOOKING : NO BKG RQ SSIM : N  EMD : 4\n COMMERCIAL : CPPUNIT TEST\n "
               "TEXT T196  : 101010  \n PICTURE NUM: 202     \n CONSUMPTION AT ISSUANCE : Y\n"),
        _diag->str());
  }

  void testPrintGroupCodesInTheRequest_RT_INTL_GroupRequested()
  {
    createServiceFeesGroup(); // create active group code 'ML'
    std::vector<ServiceGroup> input;
    std::vector<ServiceGroup> vecInv;
    std::vector<ServiceGroup> vecInvTkt;
    ItinBoolMap isInternational;
    ItinBoolMap isRoundTrip;
    input.push_back("BG");
    input.push_back("ML");
    input.push_back("MM");
    vecInv.push_back("MM");
    vecInvTkt.push_back("BG");
    isInternational.insert(std::make_pair(_trx->itin().front(), true));
    isRoundTrip.insert(std::make_pair(_trx->itin().front(), true));

    _diag->printGroupCodesInTheRequest(
        *_trx, input, vecInv, vecInvTkt, isInternational, isRoundTrip, false);

    CPPUNIT_ASSERT_EQUAL(
        string("*************** OC SUB CODE SERVICE ANALYSIS ******************\nGROUP REQUESTED : "
               "BG ML MM \nGROUP ACTIVE    : ML \nGROUP NOT ACTIVE: BG \nINVALID GROUP   : MM "
               "\nTKT DATE        : 2010-05-01\nJOURNEY TYPE    : RT INTL \n"),
        _diag->str());
  }

  void testPrintGroupCodesInTheRequest_OW_DOM_Group_NOT_Requested()
  {
    createServiceFeesGroup(); // create active group code 'ML'
    std::vector<ServiceGroup> input;
    std::vector<ServiceGroup> vecInv;
    std::vector<ServiceGroup> vecInvTkt;
    ItinBoolMap isInternational;
    ItinBoolMap isRoundTrip;
    vecInvTkt.push_back("BG");
    isInternational.insert(std::make_pair(_trx->itin().front(), false));
    isRoundTrip.insert(std::make_pair(_trx->itin().front(), false));

    _diag->printGroupCodesInTheRequest(
        *_trx, input, vecInv, vecInvTkt, isInternational, isRoundTrip, true);

    CPPUNIT_ASSERT_EQUAL(
        string("*************** OC SUB CODE SERVICE ANALYSIS ******************\nGROUP REQUESTED : "
               "NO GROUP CODE IN THE ENTRY\nGROUP ACTIVE    : ML \nGROUP NOT ACTIVE: BG \nTKT DATE "
               "       : 2010-05-01\nJOURNEY TYPE    : OW DOM  \n"),
        _diag->str());
  }

  void testPrintGroupCodesInTheRequest_RT_INTL_GroupRequested_Historical_ON()
  {
    createServiceFeesGroup(); // create active group code 'ML'
    std::vector<ServiceGroup> input;
    std::vector<ServiceGroup> vecInv;
    std::vector<ServiceGroup> vecInvTkt;
    ItinBoolMap isInternational;
    ItinBoolMap isRoundTrip;
    input.push_back("BG");
    input.push_back("ML");
    input.push_back("MM");
    vecInv.push_back("MM");
    vecInvTkt.push_back("BG");
    isInternational.insert(std::make_pair(_trx->itin().front(), true));
    isRoundTrip.insert(std::make_pair(_trx->itin().front(), true));
    _trx->getOptions()->isOCHistorical() = true;

    _diag->printGroupCodesInTheRequest(
        *_trx, input, vecInv, vecInvTkt, isInternational, isRoundTrip, false);

    CPPUNIT_ASSERT_EQUAL(
        string("*************** OC SUB CODE SERVICE ANALYSIS ******************\nGROUP REQUESTED : "
               "BG ML MM \nGROUP ACTIVE    : ML \nGROUP NOT ACTIVE: BG \nINVALID GROUP   : MM "
               "\nTKT DATE        : 2010-05-01                 PCC SAME DATE : N\nJOURNEY TYPE    "
               ": RT INTL \n"),
        _diag->str());
  }

  void testGetCarrierListAsString()
  {
    std::set<CarrierCode> codes;
    codes.insert("AA");

    CPPUNIT_ASSERT_EQUAL(std::string("AA"), _diag->getCarrierListAsString(codes, "|"));
    codes.insert("BB");
    CPPUNIT_ASSERT_EQUAL(std::string("AA|BB"), _diag->getCarrierListAsString(codes, "|"));
    codes.insert("C1");
    codes.insert("C2");
    codes.insert("C3");
    codes.insert("C4");
    codes.insert("C5");
    codes.insert("C6");
    codes.insert("C7");
    codes.insert("C8");
    codes.insert("C9");
    codes.insert("D1");
    codes.insert("D2");
    codes.insert("D3");
    codes.insert("D4");
    CPPUNIT_ASSERT_EQUAL(
      std::string("AA|BB|C1|C2|C3|C4|C5|C6|C7|C8|C9|D1|D2|\n                      D3|D4"),
      _diag->getCarrierListAsString(codes, "|"));
  }

  void testPrintDetailInterlineEmdProcessingStatusS5Info()
  {
    _diag->printDetailInterlineEmdProcessingStatusS5Info(true);
    CPPUNIT_ASSERT_EQUAL(std::string(" S5 STATUS : I-EMD PROCESSED\n"), _diag->str());

    _diag->clear();
    _diag->printDetailInterlineEmdProcessingStatusS5Info(false);
    CPPUNIT_ASSERT_EQUAL(std::string(" S5 STATUS : I-EMD NOT PROCESSED \n"), _diag->str());

    _diag->clear();
    std::set<CarrierCode> carrierCodes;
    carrierCodes.insert("AA");
    carrierCodes.insert("BB");
    _diag->printDetailInterlineEmdProcessingStatusS5Info(false, carrierCodes);
    CPPUNIT_ASSERT_EQUAL(std::string(" S5 STATUS : I-EMD NOT PROCESSED AA/BB\n"), _diag->str());
  }

  void testPrintDetailInterlineEmdProcessingS5Info()
  {
    std::set<tse::CarrierCode> marketingCarriers;
    marketingCarriers.insert("AA");
    marketingCarriers.insert("BB");
    std::set<tse::CarrierCode> operatingCarriers;
    operatingCarriers.insert("CC");
    operatingCarriers.insert("DD");

    _diag->printDetailInterlineEmdProcessingS5Info("NC", "GDS", "XX", marketingCarriers, operatingCarriers);
    CPPUNIT_ASSERT_EQUAL(std::string("--------- INTERLINE EMD AGREEMENT PROCESSING DETAILS ---------\n"
                                     " NATION             : NC\n"
                                     " GDS                : GDS\n"
                                     " VALIDATING CXR     : XX\n"
                                     " MARKETING  CXR     : AA BB\n"
                                     " OPERATING  CXR     : CC DD\n"), _diag->str());

    _diag->clear();
    marketingCarriers.insert("C1");
    marketingCarriers.insert("C2");
    marketingCarriers.insert("C3");
    marketingCarriers.insert("C4");
    marketingCarriers.insert("C5");
    marketingCarriers.insert("C6");
    marketingCarriers.insert("C7");
    marketingCarriers.insert("C8");
    marketingCarriers.insert("C9");
    marketingCarriers.insert("D1");
    marketingCarriers.insert("D2");
    marketingCarriers.insert("D3");
    marketingCarriers.insert("D4");
    _diag->printDetailInterlineEmdProcessingS5Info("NC", "GDS", "XX", marketingCarriers, operatingCarriers);
    CPPUNIT_ASSERT_EQUAL(std::string("--------- INTERLINE EMD AGREEMENT PROCESSING DETAILS ---------\n"
                                     " NATION             : NC\n"
                                     " GDS                : GDS\n"
                                     " VALIDATING CXR     : XX\n"
                                     " MARKETING  CXR     : AA BB C1 C2 C3 C4 C5 C6 C7 C8 C9 D1 D2 \n"
                                     "                      D3 D4\n"
                                     " OPERATING  CXR     : CC DD\n"), _diag->str());

  }

  void testPrintNoInterlineDataFoundInfo()
  {
    _diag->printNoInterlineDataFoundInfo();
    CPPUNIT_ASSERT_EQUAL(std::string("--------- NO INTERLINE EMD AGREEMENT DATA FOUND ---------\n"), _diag->str());
  }

  void testPrintDetailInterlineEmdAgreementInfo()
  {
    std::vector<tse::EmdInterlineAgreementInfo*> eiaList;

    tse::CarrierCode carrierCode;
    tse::EmdInterlineAgreementInfo emd[15];

    for (int i = 0; i < 3; ++i) {
        std::string tmp = "";
        tmp.push_back(char('A' + i));
        tmp.push_back(char('A' + i));
        carrierCode = tmp;
        tse::EmdInterlineAgreementInfo::dummyData(emd[i]);
        emd[i].setParticipatingCarrier(carrierCode);
        eiaList.push_back(&emd[i]);
    }
    _diag->printDetailInterlineEmdAgreementInfo(eiaList);
    CPPUNIT_ASSERT_EQUAL(std::string(" ALLOWED    CXR     : AA BB CC \n"), _diag->str());

    _diag->clear();

    for (int i = 3; i < 15; ++i) {
        std::string tmp = "";
        tmp.push_back(char('A' + i));
        tmp.push_back(char('A' + i));
        carrierCode = tmp;
        tse::EmdInterlineAgreementInfo::dummyData(emd[i]);
        emd[i].setParticipatingCarrier(carrierCode);
        eiaList.push_back(&emd[i]);
    }
    _diag->printDetailInterlineEmdAgreementInfo(eiaList);
    CPPUNIT_ASSERT_EQUAL(std::string(" ALLOWED    CXR     : AA BB CC DD EE FF GG HH II JJ KK LL MM \n"
                                     "                      NN OO \n"), _diag->str());
  }

  void printExpectedVsActualValues(string& expected, string& actual)
  {
    cout << std::endl;
    cout << "XXXXXX" << endl;
    cout << actual << endl;
    cout << "XXXXX" << endl;
    cout << "XXXXXX" << endl;
    cout << expected << endl;
    cout << "XXXXX" << endl;

    cout << "length of Expected: " << expected.size();
    cout << "length of Actual  : " << actual.size();
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag875CollectorTest);
} // tse
