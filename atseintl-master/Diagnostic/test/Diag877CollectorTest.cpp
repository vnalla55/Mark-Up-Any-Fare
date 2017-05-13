#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseEnums.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SeatCabinCharacteristicInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/SvcFeesCurrencyInfo.h"
#include "DBAccess/SvcFeesCxrResultingFCLInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "Diagnostic/Diag877Collector.h"
#include "ServiceFees/CarrierStrategy.h"
#include "ServiceFees/OCFees.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{

class SpecificTestConfigInitializer : public TestConfigInitializer
{
public:
  SpecificTestConfigInitializer() {}

  ~SpecificTestConfigInitializer() {}

private:
};

class Diag877CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag877CollectorTest);
  CPPUNIT_TEST(testPrintS7CommonHeader);
  CPPUNIT_TEST(testPrintS7CommonHeader_NotActive);

  CPPUNIT_TEST(testPrintS7Banner);
  CPPUNIT_TEST(testPrintS7Banner_NotActive);

  CPPUNIT_TEST(testPrintS7NotFound_ATP);
  CPPUNIT_TEST(testPrintS7NotFound_MMGR);
  CPPUNIT_TEST(testPrintS7NotFound_NotActive);

  CPPUNIT_TEST(testPrintS7PortionOfTravel_Board_OffCities_presents);
  CPPUNIT_TEST(testPrintS7PortionOfTravel_Board_OffAirport_presents);
  CPPUNIT_TEST(testPrintS7PortionOfTravel_NotActive);

  CPPUNIT_TEST(testPrintS7GroupCxrService_Marketing);
  CPPUNIT_TEST(testPrintS7GroupCxrService_Operating);
  CPPUNIT_TEST(testPrintS7GroupCxrService_Partition);
  CPPUNIT_TEST(testPrintS7GroupCxrService_NotActive);

  CPPUNIT_TEST(testPrintS7OptionalServiceStatus_Pass);
  CPPUNIT_TEST(testPrintS7OptionalServiceStatus_NotActive);

  CPPUNIT_TEST(testDisplayStatus_Pass);
  CPPUNIT_TEST(testDisplayStatus_InputTvlDate);
  CPPUNIT_TEST(testDisplayStatus_TvlDate);
  CPPUNIT_TEST(testDisplayStatus_InputPsgType);
  CPPUNIT_TEST(testDisplayStatus_AccountCode);
  CPPUNIT_TEST(testDisplayStatus_InputTicketDesignator);
  CPPUNIT_TEST(testDisplayStatus_OutputTicketDesignator);
  CPPUNIT_TEST(testDisplayStatus_SecurityTable);
  CPPUNIT_TEST(testDisplayStatus_SectorPortion);
  CPPUNIT_TEST(testDisplayStatus_Upgrade);
  CPPUNIT_TEST(testDisplayStatus_FromToWithin);
  CPPUNIT_TEST(testDisplayStatus_ViaLoc);
  CPPUNIT_TEST(testDisplayStatus_StopCnxDest);
  CPPUNIT_TEST(testDisplayStatus_Cabin);
  CPPUNIT_TEST(testDisplayStatus_RBDTable);
  CPPUNIT_TEST(testDisplayStatus_ResultingFCLTable);
  CPPUNIT_TEST(testDisplayStatus_RuleTariffInd);
  CPPUNIT_TEST(testDisplayStatus_RuleTariff);
  CPPUNIT_TEST(testDisplayStatus_Rule);
  CPPUNIT_TEST(testDisplayStatus_FareInd);
  CPPUNIT_TEST(testDisplayStatus_TourCodeInd);
  CPPUNIT_TEST(testDisplayStatus_StartStopTimeInd);
  CPPUNIT_TEST(testDisplayStatus_DOWInd);
  CPPUNIT_TEST(testDisplayStatus_AdvPurInd);
  CPPUNIT_TEST(testDisplayStatus_SfcT170);
  CPPUNIT_TEST(testDisplayStatus_Fail_FF);
  CPPUNIT_TEST(testDisplayStatus_Fail_Equipment);
  CPPUNIT_TEST(testDisplayStatus_Fail_Service_NA);
  CPPUNIT_TEST(testDisplayStatus_PASS_Service_Free);
  CPPUNIT_TEST(testDisplayStatus_Fail_Mileage_Free);
  CPPUNIT_TEST(testDisplayStatus_Fail_T186);
  CPPUNIT_TEST(testDisplayStatus_Fail_COLLECT_SUBTRACT);
  CPPUNIT_TEST(testDisplayStatus_Fail_NET_SELL);
  CPPUNIT_TEST(testDisplayStatus_Fail_FF_MileageAppl);
  CPPUNIT_TEST(testDisplayStatus_Soft_Pass);
  CPPUNIT_TEST(testDisplayStatus_Fail_And_Or_Ind);
  CPPUNIT_TEST(testDisplayStatus_Fail_Baggage_Weight_Unit);
  CPPUNIT_TEST(testDisplayStatus_Fail_Fee_Application);
  CPPUNIT_TEST(testDisplayStatus_Fail_Baggage_Occurrence);
  CPPUNIT_TEST(testDisplayStatus_Fail_Free_Bag_Pieces);
  CPPUNIT_TEST(testDisplayStatus_Fail_Tax_Application);
  CPPUNIT_TEST(testDisplayStatus_Fail_Availability);
  CPPUNIT_TEST(testDisplayStatus_Fail_Rule_Buster_Rmfc);
  CPPUNIT_TEST(testDisplayStatus_Fail_Pax_Min_Max_Age);
  CPPUNIT_TEST(testDisplayStatus_Fail_Pax_Occurrence);
  CPPUNIT_TEST(testDisplayStatus_Unknown);
  CPPUNIT_TEST(testDisplayStatus_NotActive);
  CPPUNIT_TEST(testDisplayStatus_InterlineInd);

  CPPUNIT_TEST(testPrintSvcFeeCurTable170Header);
  CPPUNIT_TEST(testPrintSvcFeeCurTable170Header_NotActive);

  CPPUNIT_TEST(test_printStartStopTimeHeader_true);
  CPPUNIT_TEST(test_printStartStopTimeHeader_false);

  CPPUNIT_TEST(testPrintSvcFeeCurInfoDetail_PASS);
  CPPUNIT_TEST(testPrintSvcFeeCurInfoDetail_FAIL);
  CPPUNIT_TEST(testPrintSvcFeeCurInfoDetail_NotActive);

  CPPUNIT_TEST(testPrintSvcFeeCurInfo170Label_POS_Empty);
  CPPUNIT_TEST(testPrintSvcFeeCurInfo170Label_POS_First);
  CPPUNIT_TEST(testPrintSvcFeeCurInfo170Label_NotActive);

  CPPUNIT_TEST(testPrintRBDTable198Header);
  CPPUNIT_TEST(testPrintRBDTable198Header_NotActive);

  CPPUNIT_TEST(testPrintRBDTable198Info);
  CPPUNIT_TEST(testPrintRBDTable198InfoWhenDifferentMktAndOperCxrForOperPhase);
  CPPUNIT_TEST(testPrintRBDTable198InfoWhenFailsOnCxr);
  CPPUNIT_TEST(testPrintRBDTable198InfoWhenFailsOnBKGCode);
  CPPUNIT_TEST(testPrintRBDTable198Info_NotActive);

  CPPUNIT_TEST(testPrintResFCLTable171Header);
  CPPUNIT_TEST(testPrintResFCLTable171Header_NotActive);

  CPPUNIT_TEST(testPrintResFCLTable171Info_Pass);
  CPPUNIT_TEST(testPrintResFCLTable171Info_Pass_Fare_CXR);
  CPPUNIT_TEST(testPrintResFCLTable171Info_Fail_On_CXR);
  CPPUNIT_TEST(testPrintResFCLTable171Info_Fail_On_FareClass);
  CPPUNIT_TEST(testPrintResFCLTable171Info_Pass_On_FareClass_WhenOverridedBySpecifiedFCL);
  CPPUNIT_TEST(testPrintResFCLTable171Info_Fail_On_FareType);
  CPPUNIT_TEST(testPrintResFCLTable171Info_Soft_On_FareClass);
  CPPUNIT_TEST(testPrintResFCLTable171Info_Soft_On_FareType);
  CPPUNIT_TEST(testPrintResFCLTable171Info_NotActive);

  CPPUNIT_TEST(testPrintCarrierFlightT186Header);
  CPPUNIT_TEST(testPrintCarrierFlightT186Header_NotActive);

  CPPUNIT_TEST(testPrintCarrierFlightApplT186Info_AnyFlights_Pass);
  CPPUNIT_TEST(testPrintCarrierFlightApplT186Info_AnyFlights_Fail_Marketing);
  CPPUNIT_TEST(testPrintCarrierFlightApplT186Info_AnyFlights_Fail_Operating);
  CPPUNIT_TEST(testPrintCarrierFlightApplT186Info_Flight_Range);
  CPPUNIT_TEST(testPrintCarrierFlightApplT186Info_Flight_Range_SoftPass);
  CPPUNIT_TEST(testPrintCarrierFlightApplT186Info_1ST_Flight);
  CPPUNIT_TEST(testPrintCarrierFlightApplT186Info_NotActive);

  CPPUNIT_TEST(testPrintNoCxrFligtT186Data);
  CPPUNIT_TEST(testPrintNoCxrFligtT186Data_NotActive);

  CPPUNIT_TEST(testPrintS7OptionalFeeInfo_No_OCFees_ATP);
  CPPUNIT_TEST(testPrintS7OptionalFeeInfo_No_OCFees_MMGR);
  CPPUNIT_TEST(testPrintS7OptionalFeeInfo_OCFees_Free);
  CPPUNIT_TEST(testPrintS7OptionalFeeInfo_OCFees_Amount_Applied);
  CPPUNIT_TEST(testPrintS7OptionalFeeInfo_NotActive);

  CPPUNIT_TEST(testPrintS7DetailInfo_No_PartialTravelDates);
  CPPUNIT_TEST(testPrintS7DetailInfo_PartialTravelDates);
  CPPUNIT_TEST(testPrintS7DetailInfo_PartialTravelDates_Months_Only);
  CPPUNIT_TEST(testPrintS7DetailInfo_TktEmptyDates);
  CPPUNIT_TEST(testPrintS7DetailInfo_RuleTariff_NotDefined);
  CPPUNIT_TEST(testPrintS7DetailInfo_NotActive);

  CPPUNIT_TEST(testPrintS7DetailInfo_PADIS);

  CPPUNIT_TEST(testDisplayStopCnxDestInd_C_Connection);
  CPPUNIT_TEST(testDisplayStopCnxDestInd_F_NotFareBreak);
  CPPUNIT_TEST(testDisplayStopCnxDestInd_P_NotFareBreakOR_Stopover);
  CPPUNIT_TEST(testDisplayStopCnxDestInd_T_StopoverWithGeo);
  CPPUNIT_TEST(testDisplayStopCnxDestInd_NotActive);

  CPPUNIT_TEST(testDisplaySourceForFareCreated_1_Cat19);
  CPPUNIT_TEST(testDisplaySourceForFareCreated_2_Cat25);
  CPPUNIT_TEST(testDisplaySourceForFareCreated_3_Cat35);
  CPPUNIT_TEST(testDisplaySourceForFareCreated_NotActive);

  CPPUNIT_TEST(testDisplayNotAvailNoCharge_X);
  CPPUNIT_TEST(testDisplayNotAvailNoCharge_F);
  CPPUNIT_TEST(testDisplayNotAvailNoCharge_E);
  CPPUNIT_TEST(testDisplayNotAvailNoCharge_H);
  CPPUNIT_TEST(testDisplayNotAvailNoCharge_NotActive);

  CPPUNIT_TEST(testDisplayRefundReissue_Y);
  CPPUNIT_TEST(testDisplayRefundReissue_N);
  CPPUNIT_TEST(testDisplayRefundReissue_NotActive);

  CPPUNIT_TEST(testDisplayFFMileageAppl_1_PerOneWay);
  CPPUNIT_TEST(testDisplayFFMileageAppl_2_PerRoundTrip);
  CPPUNIT_TEST(testDisplayFFMileageAppl_3_PerItem);
  CPPUNIT_TEST(testDisplayFFMileageAppl_4_PerTravel);
  CPPUNIT_TEST(testDisplayFFMileageAppl_5_PerTicket);
  CPPUNIT_TEST(testDisplayFFMileageAppl_NotActive);

  CPPUNIT_TEST(testDisplayFormOfTheRefund_OriginalFOP);
  CPPUNIT_TEST(testDisplayFormOfTheRefund_NotActive);

  CPPUNIT_TEST(testPrintS7SoftMatchedFields_NoSoftMatch_Found);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_Cabin_Only);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_RBD_Only);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_RULETARIFF_Only);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_RULE_Only);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_RESULTING_FARE_Only);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_CXRFLIGHT_Only);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_EQUIPMENT_Only);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_STOPCNXDESTIND_Only);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_STOPOVERTIME_Only);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_STOPOVERUNIT_Only);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_STARTTIME_Only);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_STOPTIME_Only);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_STOPCNXDESTIND_STOPOVERTIME);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_STOPCNXDESTIND_STOPOVERUNIT);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_STOPOVERTIME_STOPOVERUNIT);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_STOPCNXDESTIND_STOPOVERTIME_STOPOVERUNIT);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_STARTTIME_STOPTIME);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_STOPCNXDESTIND_STOPOVERTIME_STOPOVERUNIT_STARTTIME);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_STOPCNXDESTIND_STOPOVERTIME_STOPOVERUNIT_STOPTIME);
  CPPUNIT_TEST(
      testPrintS7SoftMatchedFields_STOPCNXDESTIND_STOPOVERTIME_STOPOVERUNIT_STARTTIME_STOPTIME);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_Cabin_RBD);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_Cabin_RULETARIFF);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_Cabin_RULE);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_Cabin_RESULTING_FARE);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_Cabin_CXRFLIGHT);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_Cabin_EQUIPMENT);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_RESULTING_FARE_CXRFLIGHT);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_RESULTING_FARE_CXRFLIGHT_STOPCNXDESTIND);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_RESULTING_FARE_STOPCNXDESTIND_STOPOVERTIME);
  CPPUNIT_TEST(
      testPrintS7SoftMatchedFields_RESULTING_FARE_STOPCNXDESTIND_STOPOVERTIME_STOPOVERUNIT);
  CPPUNIT_TEST(testPrintS7SoftMatchedFields_RESULTING_FARE_RBD_CABIN_RULE);
  CPPUNIT_TEST(testPrintS7SoftMatchedField_Frequent_Flyer_Equipment);
  CPPUNIT_TEST(testPrint_All_S7SoftMatchedFields);

  CPPUNIT_TEST(testDisplayAmount_NoAvailNoCharges_X);

  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------------------
  // testConstructor()
  //---------------------------------------------------------------------
  void testConstructor()
  {
    try
    {
      Diag877Collector diag;
      CPPUNIT_ASSERT_EQUAL(std::string(""), diag.str());
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    try
    {
      _diagroot = _memHandle.insert(new Diagnostic(Diagnostic877));
      _diagroot->activate();
      _diag = _memHandle.insert(new Diag877Collector(*_diagroot));
      _diag->enable(Diagnostic877);
      _dataHandleMock = _memHandle.create<Diag877CollectorDataHandleMock>();
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void tearDown() { _memHandle.clear(); }
  void setTicketingAgent()
  {
    Loc loc;
    loc.loc() = "DFW";
    loc.subarea() = "1";
    loc.area() = "2";
    loc.nation() = "US";
    loc.state() = "TX";
    loc.cityInd() = true;
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->getOptions()->fareCalculationDisplay() = '1';
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _diag->trx() = _trx;
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx->getRequest()->ticketingAgent()->agentLocation() = &loc;
  }
  void testPrintS7CommonHeader()
  {
    _diag->printS7CommonHeader();
    CPPUNIT_ASSERT_EQUAL(std::string("V SCODE SERVICE   SEQ NUM  PAX    AMOUNT  STATUS\n"),
                         _diag->str());
  }

  void testPrintS7CommonHeader_NotActive()
  {
    _diag->_active = false;
    _diag->printS7CommonHeader();
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testPrintS7Banner()
  {
    _diag->printS7Banner();
    CPPUNIT_ASSERT_EQUAL(
        std::string("*************** OC OPTIONAL SERVICE ANALYSIS ******************\n"),
        _diag->str());
  }

  void testPrintS7Banner_NotActive()
  {
    _diag->_active = false;
    _diag->printS7Banner();
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testPrintS7NotFound_ATP()
  {
    SubCodeInfo sci;
    sci.vendor() = "ATP";
    sci.serviceSubTypeCode() = "0A0";
    sci.fltTktMerchInd() = FLIGHT_RELATED_SERVICE;

    _diag->printS7NotFound(sci);
    CPPUNIT_ASSERT_EQUAL(std::string("A  0A0  FLIGHT                            DATA NOT FOUND\n"),
                         _diag->str());
  }

  void testPrintS7NotFound_MMGR()
  {
    SubCodeInfo sci;
    sci.vendor() = "MMGR";
    sci.serviceSubTypeCode() = "0A0";
    sci.fltTktMerchInd() = FLIGHT_RELATED_SERVICE;

    _diag->printS7NotFound(sci);
    CPPUNIT_ASSERT_EQUAL(std::string("M  0A0  FLIGHT                            DATA NOT FOUND\n"),
                         _diag->str());
  }

  void testPrintS7NotFound_NotActive()
  {
    _diag->_active = false;
    _diag->printS7NotFound(SubCodeInfo());
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testPrintS7GroupCxrService_Marketing()
  {
    FarePath fP;
    PaxType pt;
    fP.paxType() = &pt;
    pt.paxType() = "ADT";
    MarketingCarrierStrategy carrierStrategy;
    _diag->printS7GroupCxrService(fP, "ML", "AB", carrierStrategy.getCarrierStrategyTypeFull());
    CPPUNIT_ASSERT_EQUAL(
        std::string("**************************************************************\n"
                    " GROUP    : ML \n CXR      : AB  - MARKETING       REQUESTED PAX TYPE : ADT\n"
                    "-------------------------------------------------------------- \n"
                    "                  S7 RECORDS DATA PROCESSING\n"),
        _diag->str());
  }

  void testPrintS7GroupCxrService_Operating()
  {
    FarePath fP;
    PaxType pt;
    fP.paxType() = &pt;
    pt.paxType() = "ADT";
    OperatingCarrierStrategy carrierStrategy;
    _diag->printS7GroupCxrService(fP, "ML", "AB", carrierStrategy.getCarrierStrategyTypeFull());
    CPPUNIT_ASSERT_EQUAL(
        std::string("**************************************************************\n"
                    " GROUP    : ML \n CXR      : AB  - OPERATING       REQUESTED PAX TYPE : ADT\n"
                    "-------------------------------------------------------------- \n"
                    "                  S7 RECORDS DATA PROCESSING\n"),
        _diag->str());
  }

  void testPrintS7GroupCxrService_Partition()
  {
    FarePath fP;
    PaxType pt;
    fP.paxType() = &pt;
    pt.paxType() = "ADT";
    PartitionCarrierStrategy carrierStrategy;
    _diag->printS7GroupCxrService(fP, "ML", "AB", carrierStrategy.getCarrierStrategyTypeFull());
    CPPUNIT_ASSERT_EQUAL(
        std::string("**************************************************************\n"
                    " GROUP    : ML \n CXR      : AB  - PARTITION       REQUESTED PAX TYPE : ADT\n"
                    "-------------------------------------------------------------- \n"
                    "                  S7 RECORDS DATA PROCESSING\n"),
        _diag->str());
  }

  void testPrintS7GroupCxrService_NotActive()
  {
    _diag->_active = false;
    OperatingCarrierStrategy carrierStrategy;
    _diag->printS7GroupCxrService(FarePath(), "", "", carrierStrategy.getCarrierStrategyTypeFull());
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testPrintS7OptionalServiceStatus_Pass()
  {
    _diag->printS7OptionalServiceStatus(PASS_S7);
    CPPUNIT_ASSERT_EQUAL(std::string(" \n                              S7 STATUS : PASS\n"),
                         _diag->str());
  }

  void testPrintS7OptionalServiceStatus_NotActive()
  {
    _diag->_active = false;
    _diag->printS7OptionalServiceStatus(PASS_S7);
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testDisplayStatus_Pass()
  {
    _diag->displayStatus(PASS_S7);
    CPPUNIT_ASSERT_EQUAL(std::string("PASS"), _diag->str());
  }

  void testDisplayStatus_InputTvlDate()
  {
    _diag->displayStatus(FAIL_S7_INPUT_TVL_DATE);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL IN TVL DATE"), _diag->str());
  }

  void testDisplayStatus_TvlDate()
  {
    _diag->displayStatus(FAIL_S7_TVL_DATE);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL TVL DATE"), _diag->str());
  }

  void testDisplayStatus_InputPsgType()
  {
    _diag->displayStatus(FAIL_S7_INPUT_PSG_TYPE);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL INPUT PSG TYPE"), _diag->str());
  }

  void testDisplayStatus_AccountCode()
  {
    _diag->displayStatus(FAIL_S7_ACCOUNT_CODE);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL ACCOUNT CODE"), _diag->str());
  }

  void testDisplayStatus_InputTicketDesignator()
  {
    _diag->displayStatus(FAIL_S7_INPUT_TKT_DESIGNATOR);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL INPUT TKT DESIGN"), _diag->str());
  }

  void testDisplayStatus_OutputTicketDesignator()
  {
    _diag->displayStatus(FAIL_S7_OUTPUT_TKT_DESIGNATOR);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL OUTPUT TKT DESIGN"), _diag->str());
  }

  void testDisplayStatus_SecurityTable()
  {
    _diag->displayStatus(FAIL_S7_SECUR_T183);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL SECUR T183"), _diag->str());
  }

  void testDisplayStatus_Upgrade()
  {
    _diag->displayStatus(FAIL_S7_UPGRADE);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL UPGRADE CHECK"), _diag->str());
  }

  void testDisplayStatus_SectorPortion()
  {
    _diag->displayStatus(FAIL_S7_SECTOR_PORTION);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL SECTOR PORTION"), _diag->str());
  }

  void testDisplayStatus_FromToWithin()
  {
    _diag->displayStatus(FAIL_S7_FROM_TO_WITHIN);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL GEO FTW IND"), _diag->str());
  }

  void testDisplayStatus_ViaLoc()
  {
    _diag->displayStatus(FAIL_S7_INTERMEDIATE_POINT);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL VIA LOC"), _diag->str());
  }

  void testDisplayStatus_StopCnxDest()
  {
    _diag->displayStatus(FAIL_S7_STOP_CNX_DEST);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL STOP CNX DEST"), _diag->str());
  }

  void testDisplayStatus_Cabin()
  {
    _diag->displayStatus(FAIL_S7_CABIN);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL CABIN"), _diag->str());
  }

  void testDisplayStatus_RBDTable()
  {
    _diag->displayStatus(FAIL_S7_RBD_T198);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL RBD T198"), _diag->str());
  }

  void testDisplayStatus_ResultingFCLTable()
  {
    _diag->displayStatus(FAIL_S7_RESULT_FC_T171);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL RESLT FC STATUS"), _diag->str());
  }

  void testDisplayStatus_RuleTariffInd()
  {
    _diag->displayStatus(FAIL_S7_RULE_TARIFF_IND);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL RULE TARIFF IND"), _diag->str());
  }

  void testDisplayStatus_RuleTariff()
  {
    _diag->displayStatus(FAIL_S7_RULE_TARIFF);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL RULE TARIFF"), _diag->str());
  }

  void testDisplayStatus_Rule()
  {
    _diag->displayStatus(FAIL_S7_RULE);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL RULE"), _diag->str());
  }

  void testDisplayStatus_FareInd()
  {
    _diag->displayStatus(FAIL_S7_FARE_IND);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL FARE IND"), _diag->str());
  }

  void testDisplayStatus_TourCodeInd()
  {
    _diag->displayStatus(FAIL_S7_TOURCODE);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL TOUR CODE"), _diag->str());
  }

  void testDisplayStatus_StartStopTimeInd()
  {
    _diag->displayStatus(FAIL_S7_START_STOP_TIME);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL START/STOP TIME"), _diag->str());
  }

  void testDisplayStatus_DOWInd()
  {
    _diag->displayStatus(FAIL_S7_DOW);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL DOW"), _diag->str());
  }

  void testDisplayStatus_AdvPurInd()
  {
    _diag->displayStatus(FAIL_S7_ADVPUR);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL ADV PUR"), _diag->str());
  }

  void testDisplayStatus_SfcT170()
  {
    _diag->displayStatus(FAIL_S7_SFC_T170);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL SVC FEE CUR T170"), _diag->str());
  }

  void testDisplayStatus_Fail_FF()
  {
    _diag->displayStatus(FAIL_S7_FREQ_FLYER_STATUS);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL FF STATUS"), _diag->str());
  }

  void testDisplayStatus_Fail_Equipment()
  {
    _diag->displayStatus(FAIL_S7_EQUIPMENT);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL EQUIPMENT"), _diag->str());
  }

  void testDisplayStatus_Fail_Service_NA()
  {
    _diag->displayStatus(FAIL_S7_NOT_AVAIL_NO_CHANGE);
    CPPUNIT_ASSERT_EQUAL(std::string("SERVICE NOT AVAILABLE"), _diag->str());
  }

  void testDisplayStatus_PASS_Service_Free()
  {
    _diag->displayStatus(PASS_S7_FREE_SERVICE);
    CPPUNIT_ASSERT_EQUAL(std::string("PASS SERVICE IS FREE"), _diag->str());
  }

  void testDisplayStatus_Fail_Mileage_Free()
  {
    _diag->displayStatus(FAIL_S7_MILEAGE_FEE);
    CPPUNIT_ASSERT_EQUAL(std::string("MILEAGE FEE NOT APPL"), _diag->str());
  }

  void testDisplayStatus_Fail_T186()
  {
    _diag->displayStatus(FAIL_S7_CXR_FLT_T186);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL CXR/FLT T186"), _diag->str());
  }

  void testDisplayStatus_Fail_COLLECT_SUBTRACT()
  {
    _diag->displayStatus(FAIL_S7_COLLECT_SUBTRACT);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL COLLECT/SUBTRACT"), _diag->str());
  }

  void testDisplayStatus_Fail_NET_SELL()
  {
    _diag->displayStatus(FAIL_S7_NET_SELL);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL NET/SELL"), _diag->str());
  }

  void testDisplayStatus_Fail_FF_MileageAppl()
  {
    _diag->displayStatus(FAIL_S7_FEE_APPL);
    CPPUNIT_ASSERT_EQUAL(std::string("FEE APPLICATION"), _diag->str());
  }

  void testDisplayStatus_Soft_Pass()
  {
    _diag->displayStatus(SOFT_PASS_S7);
    CPPUNIT_ASSERT_EQUAL(std::string("SOFT PASS"), _diag->str());
  }

  void testDisplayStatus_Fail_And_Or_Ind()
  {
    _diag->displayStatus(FAIL_S7_AND_OR_IND);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL AND/OR IND"), _diag->str());
  }

  void testDisplayStatus_Fail_Baggage_Weight_Unit()
  {
    _diag->displayStatus(FAIL_S7_BAGGAGE_WEIGHT_UNIT);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL BAG WEIGHT UNIT"), _diag->str());
  }

  void testDisplayStatus_Fail_Fee_Application()
  {
    _diag->displayStatus(FAIL_S7_FEE_APPLICATION);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL FEE APPLICATION"), _diag->str());
  }

  void testDisplayStatus_Fail_Baggage_Occurrence()
  {
    _diag->displayStatus(FAIL_S7_BAGGAGE_OCCURRENCE);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL BAG OCCURRENCE"), _diag->str());
  }

  void testDisplayStatus_Fail_Free_Bag_Pieces()
  {
    _diag->displayStatus(FAIL_S7_FREE_BAG_PIECES);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL FREE BAG PIECES"), _diag->str());
  }

  void testDisplayStatus_Fail_Tax_Application()
  {
    _diag->displayStatus(FAIL_S7_TAX_APPLICATION);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL TAX APPLICATION"), _diag->str());
  }

  void testDisplayStatus_Fail_Availability()
  {
    _diag->displayStatus(FAIL_S7_AVAILABILITY);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL AVAILABILITY"), _diag->str());
  }

  void testDisplayStatus_Fail_Rule_Buster_Rmfc()
  {
    _diag->displayStatus(FAIL_S7_RULE_BUSTER_RMFC);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL RULE BUSTER RMFC"), _diag->str());
  }

  void testDisplayStatus_Fail_Pax_Min_Max_Age()
  {
    _diag->displayStatus(FAIL_S7_PAX_MIN_MAX_AGE);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL PAX MIN MAX AGE"), _diag->str());
  }

  void testDisplayStatus_Fail_Pax_Occurrence()
  {
    _diag->displayStatus(FAIL_S7_PAX_OCCURRENCE);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL PAX OCCURRENCE"), _diag->str());
  }

  void testDisplayStatus_Unknown()
  {
    _diag->displayStatus((StatusS7Validation) - 1);
    CPPUNIT_ASSERT_EQUAL(std::string("UNKNOWN STATUS"), _diag->str());
  }

  void testDisplayStatus_NotActive()
  {
    _diag->_active = false;
    _diag->displayStatus(PASS_S7);
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testDisplayStatus_InterlineInd()
  {
    _diag->displayStatus(FAIL_S7_INTERLINE_IND);
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL INTERLINE IND"), _diag->str());
  }

  void testPrintSvcFeeCurTable170Header()
  {
    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printSvcFeeCurTable170Header(10101);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n"
                    " * SVC FEE CUR T170 ITEM NO : 10101   DETAIL INFO    * \n"
                    " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n\n"
                    " MATCH EXACT POS   : \n"
                    " V SEQ-NO  POSLTYPE POSLOC CUR FEE-AMNT     DEC    STATUS \n"),
        _diag->str());
  }

  void testPrintSvcFeeCurTable170Header_NotActive()
  {
    _diag->_active = false;
    _diag->printSvcFeeCurTable170Header(10101);
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void test_printStartStopTimeHeader_true()
  {
    _diag->printStartStopTimeHeader(true);
    CPPUNIT_ASSERT_EQUAL(std::string(" *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n"
                                     " * START TIME/STOP TIME VALIDATION                   * \n"
                                     " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n"
                                     "    FAILED DUE TO DOW VALIDATION \n\n"),
                         _diag->str());
  }

  void test_printStartStopTimeHeader_false()
  {
    _diag->printStartStopTimeHeader(false);
    CPPUNIT_ASSERT_EQUAL(std::string(" *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n"
                                     " * START TIME/STOP TIME VALIDATION                   * \n"
                                     " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n"
                                     "    FAILED DUE TO START TIME/STOP TIME VALIDATION \n\n"),
                         _diag->str());
  }

  void prepareSvcFeesCurrencyInfo(SvcFeesCurrencyInfo& info, LocKey& lk)
  {
    info.vendor() = "ATP";
    info.seqNo() = 111233;
    info.posLoc() = lk;
    info.currency() = "ZUB";
    info.noDec() = 3;
    info.feeAmount() = 3103;
  }

  void testPrintSvcFeeCurInfoDetail_PASS()
  {
    SvcFeesCurrencyInfo info;
    LocKey lk;
    lk.loc() = "AZN";
    lk.locType() = 'C';
    prepareSvcFeesCurrencyInfo(info, lk);

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printSvcFeeCurInfoDetail(info, true);
    CPPUNIT_ASSERT_EQUAL(std::string(" A 111233  C        AZN    ZUB 3103.000     3      PASS \n"),
                         _diag->str());
  }

  void testPrintSvcFeeCurInfoDetail_FAIL()
  {
    SvcFeesCurrencyInfo info;
    LocKey lk;
    lk.loc() = "AZN";
    lk.locType() = 'C';
    prepareSvcFeesCurrencyInfo(info, lk);
    info.vendor() = "MMGR";

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printSvcFeeCurInfoDetail(info, false);
    CPPUNIT_ASSERT_EQUAL(std::string(" M 111233  C        AZN    ZUB 3103.000     3      FAIL \n"),
                         _diag->str());
  }

  void testPrintSvcFeeCurInfoDetail_NotActive()
  {
    _diag->_active = false;
    _diag->printSvcFeeCurInfoDetail(SvcFeesCurrencyInfo(), false);
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testPrintSvcFeeCurInfo170Label_POS_Empty()
  {
    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printSvcFeeCurInfo170Label(true);
    CPPUNIT_ASSERT_EQUAL(std::string(" DEFAULT POS EMPTY : \n"), _diag->str());
  }

  void testPrintSvcFeeCurInfo170Label_POS_First()
  {
    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printSvcFeeCurInfo170Label(false);
    CPPUNIT_ASSERT_EQUAL(std::string(" DEFAULT POS FIRST : \n"), _diag->str());
  }

  void testPrintSvcFeeCurInfo170Label_NotActive()
  {
    _diag->_active = false;
    _diag->printSvcFeeCurInfo170Label(false);
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testPrintRBDTable198Header()
  {
    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printRBDTable198Header(99);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - - *- - - - - - - - - - - - - - - - - - - - - - -* \n"
                    " * SEG INFO   * RBD T198 ITEM NO : 99      DETAIL INFO      * \n"
                    " *- - - - - - *- - - - - - - - - - - - - - - - - - - - - - -* \n"
                    " ORIG DEST RBD SEQ NO MKT/OP CXR RBD 1 2 3 4 5  STATUS \n"),
        _diag->str());
  }

  void testPrintRBDTable198Header_NotActive()
  {
    _diag->_active = false;
    _diag->printRBDTable198Header(99);
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void setupSvcFeesResBkgDesigInfo(SvcFeesResBkgDesigInfo& info)
  {
    info.seqNo() = 1;
    info.mkgOperInd() = 'E';
    info.carrier() = "LH";
    info.bookingCode1() = 'A';
    info.bookingCode2() = 'B';
    info.bookingCode3() = 'G';
    info.bookingCode4() = 'Z';
    info.bookingCode5() = 'Y';
  }

  void testPrintRBDTable198Info()
  {
    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    BookingCode bookingCode('Z');
    LocCode origAirport = "FRA";
    LocCode destAirport = "CHI";
    SvcFeesResBkgDesigInfo info;
    StatusT198 status = PASS_T198;
    setupSvcFeesResBkgDesigInfo(info);

    _diag->printRBDTable198Info(bookingCode, origAirport, destAirport, &info, status);
    CPPUNIT_ASSERT_EQUAL(std::string(" FRA  CHI  Z   1        E    LH      A B G Z Y  PASS\n"),
                         _diag->str());
  }

  void testPrintRBDTable198InfoWhenDifferentMktAndOperCxrForOperPhase()
  {
    BookingCode bookingCode('Z');
    LocCode origAirport = "FRA";
    LocCode destAirport = "CHI";
    SvcFeesResBkgDesigInfo info;
    StatusT198 status = FAIL_RBD_NOT_PROCESSED;
    setupSvcFeesResBkgDesigInfo(info);

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printRBDTable198Info(bookingCode, origAirport, destAirport, &info, status);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" FRA  CHI  Z   1        E    LH      A B G Z Y  NOT PROCESSED\n"),
        _diag->str());
  }

  void testPrintRBDTable198InfoWhenFailsOnCxr()
  {
    BookingCode bookingCode('Z');
    LocCode origAirport = "FRA";
    LocCode destAirport = "CHI";
    SvcFeesResBkgDesigInfo info;
    StatusT198 status = FAIL_ON_RBD_CXR;
    setupSvcFeesResBkgDesigInfo(info);

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printRBDTable198Info(bookingCode, origAirport, destAirport, &info, status);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" FRA  CHI  Z   1        E    LH      A B G Z Y  FAIL ON CXR\n"), _diag->str());
  }

  void testPrintRBDTable198InfoWhenFailsOnBKGCode()
  {
    BookingCode bookingCode('Z');
    LocCode origAirport = "FRA";
    LocCode destAirport = "CHI";
    SvcFeesResBkgDesigInfo info;
    StatusT198 status = FAIL_ON_RBD_CODE;
    setupSvcFeesResBkgDesigInfo(info);

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printRBDTable198Info(bookingCode, origAirport, destAirport, &info, status);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" FRA  CHI  Z   1        E    LH      A B G Z Y  FAIL ON RBD\n"), _diag->str());
  }

  void testPrintRBDTable198Info_NotActive()
  {
    _diag->_active = false;
    _diag->printRBDTable198Info(BookingCode(), LocCode(), LocCode(), NULL, FAIL_ON_RBD_CODE);
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testPrintResFCLTable171Header()
  {
    _diag->trx() = _memHandle.create<PricingTrx>();

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printResultingFareClassTable171Header(88);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - - - - - - -*- - - - - - - - - - - - - - - - - - - - -* \n"
                    " * FARE INFO           * RBD T171 ITEM NO : 88      DETAIL INFO  * \n"
                    " *- - - - - - - - - - -*- - - - - - - - - - - - - - - - - - - - -* \n"
                    " CXR  TYPE  FARE BASIS  SEQ NO  CXR  FARE CLASS  FARE TYPE  STATUS \n"),
        _diag->str());
  }

  void testPrintResFCLTable171Header_NotActive()
  {
    _diag->_active = false;
    _diag->printResultingFareClassTable171Header(99);
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void prepareFCLData(PaxTypeFare& ptf,
                      Fare& fare,
                      FareInfo& fareInfo,
                      FareClassAppInfo& fCAInfo,
                      FareMarket& fm,
                      SvcFeesCxrResultingFCLInfo& info)
  {
    ptf.fareMarket() = &fm;
    ptf.fareClassAppInfo() = &fCAInfo;
    fare.setFareInfo(&fareInfo);

    fm.governingCarrier() = "UA";
    fCAInfo._fareType = "EU";
    fareInfo.fareClass() = "Y1CH";
    fareInfo.carrier() = "LT";
    ptf.setFare(&fare);
    info.seqNo() = 12;
    info.carrier() = "UA";
    info.resultingFCL() = "Y2CH";
    info.fareType() = "EU";
  }

  void testPrintResFCLTable171Info_Pass()
  {
    setTicketingAgent();
    PaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    FareClassAppInfo fCAInfo;
    FareMarket fm;
    SvcFeesCxrResultingFCLInfo info;

    prepareFCLData(ptf, fare, fareInfo, fCAInfo, fm, info);

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printResultingFareClassTable171Info(ptf, info, PASS_T171);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" UA   EU    Y1CH       12      UA   Y2CH        EU         PASS\n"),
        _diag->str());
  }

  void testPrintResFCLTable171Info_Pass_Fare_CXR()
  {
    setTicketingAgent();
    PaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    FareClassAppInfo fCAInfo;
    FareMarket fm;
    SvcFeesCxrResultingFCLInfo info;

    prepareFCLData(ptf, fare, fareInfo, fCAInfo, fm, info);

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printResultingFareClassTable171Info(ptf, info, PASS_T171, true);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" LT   EU    Y1CH       12      UA   Y2CH        EU         PASS\n"),
        _diag->str());
  }

  void testPrintResFCLTable171Info_Fail_On_CXR()
  {
    setTicketingAgent();
    PaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    FareClassAppInfo fCAInfo;
    FareMarket fm;
    SvcFeesCxrResultingFCLInfo info;

    prepareFCLData(ptf, fare, fareInfo, fCAInfo, fm, info);

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printResultingFareClassTable171Info(ptf, info, FAIL_ON_CXR);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" UA   EU    Y1CH       12      UA   Y2CH        EU         FAIL ON CXR\n"),
        _diag->str());
  }

  void testPrintResFCLTable171Info_Fail_On_FareClass()
  {
    setTicketingAgent();
    PaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    FareClassAppInfo fCAInfo;
    FareMarket fm;
    SvcFeesCxrResultingFCLInfo info;

    prepareFCLData(ptf, fare, fareInfo, fCAInfo, fm, info);

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printResultingFareClassTable171Info(ptf, info, FAIL_ON_FARE_CLASS);
    CPPUNIT_ASSERT_EQUAL(
        std::string(
            " UA   EU    Y1CH       12      UA   Y2CH        EU         FAIL ON FARE CLASS\n"),
        _diag->str());
  }

  void testPrintResFCLTable171Info_Pass_On_FareClass_WhenOverridedBySpecifiedFCL()
  {
    setTicketingAgent();
    PaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    FareClassAppInfo fCAInfo;
    FareMarket fm;
    SvcFeesCxrResultingFCLInfo info;
    AirSeg seg;
    fm.travelSeg().push_back(&seg);
    seg.specifiedFbc() = "Y2CH";

    prepareFCLData(ptf, fare, fareInfo, fCAInfo, fm, info);

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printResultingFareClassTable171Info(ptf, info, PASS_T171);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" UA   EU    Y2CH       12      UA   Y2CH        EU         PASS\n"),
        _diag->str());
  }

  void testPrintResFCLTable171Info_Fail_On_FareType()
  {
    setTicketingAgent();
    PaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    FareClassAppInfo fCAInfo;
    FareMarket fm;
    SvcFeesCxrResultingFCLInfo info;

    prepareFCLData(ptf, fare, fareInfo, fCAInfo, fm, info);

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printResultingFareClassTable171Info(ptf, info, FAIL_NO_FARE_TYPE);
    CPPUNIT_ASSERT_EQUAL(
        std::string(
            " UA   EU    Y1CH       12      UA   Y2CH        EU         FAIL ON FARE TYPE\n"),
        _diag->str());
  }

  void testPrintResFCLTable171Info_Soft_On_FareClass()
  {
    setTicketingAgent();
    PaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    FareClassAppInfo fCAInfo;
    ptf.fareClassAppInfo() = &fCAInfo;
    fCAInfo._fareType = "";

    fare.setFareInfo(&fareInfo);
    FareMarket fm;
    ptf.fareMarket() = &fm;
    fm.governingCarrier() = "CO";
    ptf.setFare(&fare);
    SvcFeesCxrResultingFCLInfo info;
    info.seqNo() = 1;
    info.carrier() = "CO";
    info.resultingFCL() = "H2";

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printResultingFareClassTable171Info(ptf, info, SOFTPASS_FARE_CLASS);
    CPPUNIT_ASSERT_EQUAL(
        std::string(
            " CO                    1       CO   H2                     SOFT ON FARE CLASS\n"),
        _diag->str());
  }

  void testPrintResFCLTable171Info_Soft_On_FareType()
  {
    setTicketingAgent();
    PaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    FareClassAppInfo fCAInfo;
    ptf.fareClassAppInfo() = &fCAInfo;
    fCAInfo._fareType = "";

    fare.setFareInfo(&fareInfo);
    fareInfo.fareClass() = "H2";
    FareMarket fm;
    ptf.fareMarket() = &fm;
    fm.governingCarrier() = "CO";
    ptf.setFare(&fare);
    SvcFeesCxrResultingFCLInfo info;
    info.seqNo() = 1;
    info.carrier() = "CO";
    info.resultingFCL() = "H2";
    info.fareType() = "XXX";

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printResultingFareClassTable171Info(ptf, info, SOFTPASS_FARE_TYPE);
    CPPUNIT_ASSERT_EQUAL(
        std::string(
            " CO         H2         1       CO   H2          XXX        SOFT ON FARE TYPE\n"),
        _diag->str());
  }

  void testPrintResFCLTable171Info_NotActive()
  {
    _diag->_active = false;
    PaxTypeFare ptf;
    _diag->printResultingFareClassTable171Info(
        ptf, SvcFeesCxrResultingFCLInfo(), FAIL_NO_FARE_TYPE);
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void setCFSinfo(CarrierFlightSeg& info)
  {
    info.orderNo() = 10;
    info.marketingCarrier() = "UA";
    info.operatingCarrier() = "AA";
    info.flt1() = 111;
    info.flt2() = 999;
  }

  void testPrintCarrierFlightT186Header()
  {
    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printCarrierFlightT186Header(186);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -* \n"
                    " * CXR/FLIGHT APPL T186 ITEM NO : 186     DETAIL INFO       * \n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -* \n"
                    " NO      MARKET CXR  OPERAT CXR  FLT1   FLT2   STATUS \n"),
        _diag->str());
  }

  void testPrintCarrierFlightT186Header_NotActive()
  {
    _diag->_active = false;
    _diag->printCarrierFlightT186Header(1);
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testPrintCarrierFlightApplT186Info_AnyFlights_Pass()
  {
    CarrierFlightSeg info;
    StatusT186 status = PASS_T186;

    setCFSinfo(info);
    info.flt1() = -1;

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printCarrierFlightApplT186Info(info, status);
    CPPUNIT_ASSERT_EQUAL(std::string(" 10          UA          AA      ****          PASS\n"),
                         _diag->str());
  }

  void testPrintCarrierFlightApplT186Info_AnyFlights_Fail_Marketing()
  {
    CarrierFlightSeg info;
    StatusT186 status = FAIL_ON_MARK_CXR;

    setCFSinfo(info);
    info.flt1() = -1;

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printCarrierFlightApplT186Info(info, status);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" 10          UA          AA      ****          FAIL ON MARK CXR\n"),
        _diag->str());
  }

  void testPrintCarrierFlightApplT186Info_AnyFlights_Fail_Operating()
  {
    CarrierFlightSeg info;
    StatusT186 status = FAIL_ON_OPER_CXR;

    setCFSinfo(info);
    info.flt1() = -1;

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printCarrierFlightApplT186Info(info, status);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" 10          UA          AA      ****          FAIL ON OPER CXR\n"),
        _diag->str());
  }

  void testPrintCarrierFlightApplT186Info_Flight_Range()
  {
    CarrierFlightSeg info;
    StatusT186 status = FAIL_ON_FLIGHT;
    setCFSinfo(info);

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printCarrierFlightApplT186Info(info, status);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" 10          UA          AA      111    999    FAIL ON FLIGHT NO\n"),
        _diag->str());
  }

  void testPrintCarrierFlightApplT186Info_Flight_Range_SoftPass()
  {
    CarrierFlightSeg info;
    StatusT186 status = SOFTPASS_FLIGHT;
    setCFSinfo(info);

    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printCarrierFlightApplT186Info(info, status);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" 10          UA          AA      111    999    SOFTPASS ON FLTNO\n"),
        _diag->str());
  }

  void testPrintCarrierFlightApplT186Info_1ST_Flight()
  {
    CarrierFlightSeg info;
    StatusT186 status = FAIL_ON_FLIGHT;
    setCFSinfo(info);
    info.flt2() = 0;
    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printCarrierFlightApplT186Info(info, status);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" 10          UA          AA      111           FAIL ON FLIGHT NO\n"),
        _diag->str());
  }

  void testPrintCarrierFlightApplT186Info_NotActive()
  {
    _diag->_active = false;
    _diag->printCarrierFlightApplT186Info(CarrierFlightSeg(), StatusT186());
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testPrintNoCxrFligtT186Data()
  {
    _diag->rootDiag()->diagParamMap()[Diagnostic::DISPLAY_DETAIL] = "INFO";
    _diag->_active = true;

    _diag->printNoCxrFligtT186Data();
    CPPUNIT_ASSERT_EQUAL(std::string("   CARRIER/FLIGHT REQUIREMENTS ARE NOT PROVIDED\n"),
                         _diag->str());
  }

  void testPrintNoCxrFligtT186Data_NotActive()
  {
    _diag->_active = false;
    _diag->printNoCxrFligtT186Data();
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testPrintS7PortionOfTravel_Board_OffCities_presents()
  {
    AirSeg seg1;
    AirSeg seg2;
    seg1.boardMultiCity() = "NYC";
    seg1.pnrSegment() = 1;
    seg2.offMultiCity() = "DFW";
    seg2.pnrSegment() = 1;
    FarePath fP;
    setTicketingAgent();
    _diag->printS7PortionOfTravel(seg1, seg2, fP, *_trx);
    CPPUNIT_ASSERT_EQUAL(
        std::string("------------ PORTION OF TRAVEL : NYC 1  - DFW 1  ------------\n"),
        _diag->str());
  }

  void testPrintS7PortionOfTravel_Board_OffAirport_presents()
  {
    AirSeg seg1;
    AirSeg seg2;
    seg1.origAirport() = "NYC";
    seg2.destAirport() = "DFW";
    FarePath fP;
    setTicketingAgent();
    _diag->printS7PortionOfTravel(seg1, seg2, fP, *_trx);
    CPPUNIT_ASSERT_EQUAL(
        std::string("------------ PORTION OF TRAVEL : NYC 0  - DFW 0  ------------\n"),
        _diag->str());
  }

  void testPrintS7PortionOfTravel_NotActive()
  {
    _diag->_active = false;
    FarePath fP;
    _diag->printS7PortionOfTravel(AirSeg(), AirSeg(), fP, *_trx);
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void
  prepareOptionalServicesInfo(OptionalServicesInfo& info, LocKey& lk1, LocKey& lk2, LocKey& lkVia)
  {
    info.vendor() = "ATP";
    info.serviceSubTypeCode() = "0AA";
    info.fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    info.seqNo() = 111233;
    info.psgType() = "CNN";
    info.effDate() = DateTime(2010, 1, 1);
    info.discDate() = DateTime(2020, 12, 31);
    info.ticketEffDate() = DateTime(2010, 1, 2);
    info.ticketDiscDate() = DateTime(2010, 12, 1);
    info.psgType() = "CNN";
    info.minAge() = 1;
    info.maxAge() = 12;
    info.firstOccurence() = 1;
    info.lastOccurence() = 5;
    info.frequentFlyerStatus() = 4;
    info.publicPrivateInd() = 'P';
    info.tourCode() = "CPP UNIT TEST";
    info.serviceFeesAccountCodeTblItemNo() = 4444;
    info.serviceFeesTktDesigTblItemNo() = 3333;
    info.serviceFeesSecurityTblItemNo() = 2222;
    info.sectorPortionInd() = 'P';
    info.fromToWithinInd() = ' ';
    info.loc1() = lk1;
    info.loc1ZoneTblItemNo() = "Z120";
    info.loc2() = lk2;
    info.loc2ZoneTblItemNo() = "Z340";
    info.viaLoc() = lkVia;
    info.viaLocZoneTblItemNo() = "Z919";
    info.stopCnxDestInd() = 'S';
    info.stopoverTime() = " ";
    info.stopoverUnit() = 'D';
    info.cabin() = 'F';
    info.serviceFeesResBkgDesigTblItemNo() = 55555;
    info.upgradeCabin() = 'R';
    info.upgrdServiceFeesResBkgDesigTblItemNo() = 99999;
    info.serviceFeesCxrResultingFclTblItemNo() = 666666;
    info.resultServiceFeesTktDesigTblItemNo() = 777;
    info.ruleTariffInd() = "PUB";
    info.ruleTariff() = 3;
    info.rule() = "4321";
    info.fareInd() = '1';
    info.timeApplication() = 'R';
    info.dayOfWeek() = "34567";
    info.startTime() = 360;
    info.stopTime() = 1200;
    info.advPurchPeriod() = "MON";
    info.advPurchUnit() = "33";
    info.advPurchTktIssue() = 'X';
    info.equipmentCode() = "777";
    info.carrierFltTblItemNo() = 878787;
    info.notAvailNoChargeInd() = 'G';
    info.serviceFeesCurrencyTblItemNo() = 313222;
    info.andOrInd() = ' ';
    info.applicationFee() = 299992;
    info.frequentFlyerMileageAppl() = '2';
    info.refundReissueInd() = 'R';
    info.formOfFeeRefundInd() = '2';
    info.commissionInd() = 'Y';
    info.interlineInd() = 'N';
    info.collectSubtractInd() = 'I';
    info.netSellingInd() = 'N';
    info.taxInclInd() = 'X';
    info.availabilityInd() = 'N';
    info.segCount() = 12;
    // typedef Code<8>     RuleBusterFcl;
    // info.ruleBusterFcl() = "NOT USED AT THIS TIME";
    info.ruleBusterFcl() = "NOT USED";
    info.taxTblItemNo() = 9;
    info.taxExemptInd() = 'N';
  }

  void populateLocs(LocKey& lk1, LocKey& lk2, LocKey& lkVia)
  {
    lk1.loc() = "DFW";
    lk1.locType() = 'C';
    lk2.loc() = "FRA";
    lk2.locType() = 'C';
    lkVia.loc() = "LON";
    lkVia.locType() = 'C';
  }

  void testPrintS7OptionalFeeInfo_No_OCFees_ATP()
  {
    OptionalServicesInfo info;
    LocKey lk1, lk2, lkVia;
    populateLocs(lk1, lk2, lkVia);
    prepareOptionalServicesInfo(info, lk1, lk2, lkVia);
    OCFees ocFees;
    _diag->printS7OptionalFeeInfo(&info, ocFees, PASS_S7);
    CPPUNIT_ASSERT_EQUAL(std::string("A  0AA  FLIGHT    111233   CNN            PASS\n"),
                         _diag->str());
  }

  void testPrintS7OptionalFeeInfo_No_OCFees_MMGR()
  {
    OptionalServicesInfo info;
    LocKey lk1, lk2, lkVia;
    populateLocs(lk1, lk2, lkVia);
    prepareOptionalServicesInfo(info, lk1, lk2, lkVia);
    info.vendor() = "MMGR";
    OCFees ocFees;
    _diag->printS7OptionalFeeInfo(&info, ocFees, PASS_S7);
    CPPUNIT_ASSERT_EQUAL(std::string("M  0AA  FLIGHT    111233   CNN            PASS\n"),
                         _diag->str());
  }

  void testPrintS7OptionalFeeInfo_OCFees_Free()
  {
    OptionalServicesInfo info;
    LocKey lk1, lk2, lkVia;
    populateLocs(lk1, lk2, lkVia);
    prepareOptionalServicesInfo(info, lk1, lk2, lkVia);
    OCFees ocFees;
    ocFees.optFee() = &info;
    _diag->printS7OptionalFeeInfo(&info, ocFees, PASS_S7);
    CPPUNIT_ASSERT_EQUAL(std::string("A  0AA  FLIGHT    111233   CNN       FREE PASS\n"),
                         _diag->str());
  }

  void testPrintS7OptionalFeeInfo_OCFees_Amount_Applied()
  {
    OptionalServicesInfo info;
    LocKey lk1, lk2, lkVia;
    populateLocs(lk1, lk2, lkVia);
    prepareOptionalServicesInfo(info, lk1, lk2, lkVia);
    OCFees ocFees;
    ocFees.optFee() = &info;
    ocFees.feeAmount() = 10.05;
    ocFees.feeCurrency() = "RUB";
    ocFees.feeNoDec() = 3;
    _diag->printS7OptionalFeeInfo(&info, ocFees, PASS_S7);
    CPPUNIT_ASSERT_EQUAL(std::string("A  0AA  FLIGHT    111233   CNN  10.050RUB PASS\n"),
                         _diag->str());
  }

  void testPrintS7OptionalFeeInfo_NotActive()
  {
    _diag->_active = false;
    _diag->printS7OptionalFeeInfo(NULL, OCFees(), PASS_S7);
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testPrintS7DetailInfo_No_PartialTravelDates()
  {
    OptionalServicesInfo info;
    LocKey lk1, lk2, lkVia;
    populateLocs(lk1, lk2, lkVia);
    prepareOptionalServicesInfo(info, lk1, lk2, lkVia);
    _diag->printS7DetailInfo(&info, *_trx);

    CPPUNIT_ASSERT_EQUAL(
        std::string("------------- OPTIONAL SERVICE S7 DETAILED INFO --------------\n"
                    " SUBCODE : 0AA            VENDOR : ATP     SERVICE : FLIGHT   \n"
                    " SEQ NBR : 111233 \n"
                    " SRV EFF DATE : 2010-01-01      TRVL EFF DATE  : 2010-01-02\n"
                    " SRV DISC DATE: 2020-12-31      TRVL DISC DATE : 2010-12-01\n"
                    "  \n"
                    " PAX TYPE : CNN\n"
                    "      AGE MIN  : 1       MAX : 12\n"
                    "  OCCUR FIRST  : 1       LAST: 5  \n"
                    "   FF STATUS   : 4\n"
                    " PRIVATE IND   : P    TOUR CODE   : CPP UNIT TEST\n"
                    " ACC CODE T172 : 4444   \n"
                    " TKT DSGN T173 : 3333   \n"
                    " SECURITY T183 : 2222   \n"
                    "SECTOR/PORTION : P    GEO FTW IND : \n"
                    "     LOC1 TYPE : C    LOC1 : DFW       LOC1 ZONE : Z120    \n"
                    "     LOC2 TYPE : C    LOC2 : FRA       LOC2 ZONE : Z340    \n"
                    "VIA  LOC  TYPE : C    LOC  : LON       LOC  ZONE : Z919    \n"
                    "STOP/CONN/DEST : S - STOPOVER\n"
                    " STOPOVER TIME :     D\n"
                    "         CABIN : F\n"
                    "      RBD T198 : 55555  \n"
                    " UPGRADE CABIN : R\n"
                    " UPGR RBD T198 : 99999  \n"
                    "CXR/RESULT FARE CLASS T171 : 666666 \n"
                    "FARE TICKT DESIGNATOR T173 : 777    \n"
                    " RULETARIFF IND : PUB\n"
                    "    RULE TARIFF : 3       RULE : 4321   FARE IND : 19/22\n"
                    "TRAVEL TIME IND : R        DOW : 34567\n"
                    "          START : 360     STOP : 1200\n"
                    "ADV/PURCHASE RSV: MON 33             ADV/TICKET ISSUE : X\n"
                    "      EQUIPMENT : 777\n"
                    "CXR/FLIGHT APPL T186 : 878787 \n"
                    " NOT AVAIL/NO CHARGE : G- FREE SERVICE - NO EMD AND NO BKG REQ\n"
                    "       CURRENCY T170 : 313222 \n"
                    "         AND/OR : \n"
                    "FF MILEAGE FEE  : 299992  \n"
                    "FEE APPLICATION : 2 - PER ROUND TRIP\n"
                    " REFUND/REISSUE : R - REUSE TOWARDS FUTURE\n"
                    " FORM OF REFUND : ELECTRONIC VOUCHER\n"
                    "     COMMISSION : Y\n"
                    "      INTERLINE : N\n"
                    "   COLLECT/SUBSTRACT : I   NET/SELL : N\n"
                    "            TAX APPL : X                 AVAIL CHECK : N\n"
                    "      TAX EXEMPT IND : N\n"
                    "RULE BUSTER SEGMENTS : 12      RULE BUSTER FARE CLASS: NOT USED\n"
                    "TEXT T196 : 9      \n"),
        _diag->str());
  }

  void testPrintS7DetailInfo_PartialTravelDates()
  {
    OptionalServicesInfo info;
    LocKey lk1, lk2, lkVia;
    populateLocs(lk1, lk2, lkVia);
    prepareOptionalServicesInfo(info, lk1, lk2, lkVia);
    info.tvlStartYear() = 2009;
    info.tvlStartMonth() = 9;
    info.tvlStartDay() = 9;
    info.tvlStopYear() = 2011;
    info.tvlStopMonth() = 11;
    info.tvlStopDay() = 11;
    _diag->printS7DetailInfo(&info, *_trx);
    CPPUNIT_ASSERT_EQUAL(
        std::string("------------- OPTIONAL SERVICE S7 DETAILED INFO --------------\n"
                    " SUBCODE : 0AA            VENDOR : ATP     SERVICE : FLIGHT   \n"
                    " SEQ NBR : 111233 \n"
                    " SRV EFF DATE : 2010-01-01      TVL DATE START : 2009-09-09\n"
                    " SRV DISC DATE: 2020-12-31      TVL DATE STOP  : 2011-11-11\n"
                    "  \n"
                    " PAX TYPE : CNN\n"
                    "      AGE MIN  : 1       MAX : 12\n"
                    "  OCCUR FIRST  : 1       LAST: 5  \n"
                    "   FF STATUS   : 4\n"
                    " PRIVATE IND   : P    TOUR CODE   : CPP UNIT TEST\n"
                    " ACC CODE T172 : 4444   \n"
                    " TKT DSGN T173 : 3333   \n"
                    " SECURITY T183 : 2222   \n"
                    "SECTOR/PORTION : P    GEO FTW IND : \n"
                    "     LOC1 TYPE : C    LOC1 : DFW       LOC1 ZONE : Z120    \n"
                    "     LOC2 TYPE : C    LOC2 : FRA       LOC2 ZONE : Z340    \n"
                    "VIA  LOC  TYPE : C    LOC  : LON       LOC  ZONE : Z919    \n"
                    "STOP/CONN/DEST : S - STOPOVER\n"
                    " STOPOVER TIME :     D\n"
                    "         CABIN : F\n"
                    "      RBD T198 : 55555  \n"
                    " UPGRADE CABIN : R\n"
                    " UPGR RBD T198 : 99999  \n"
                    "CXR/RESULT FARE CLASS T171 : 666666 \n"
                    "FARE TICKT DESIGNATOR T173 : 777    \n"
                    " RULETARIFF IND : PUB\n"
                    "    RULE TARIFF : 3       RULE : 4321   FARE IND : 19/22\n"
                    "TRAVEL TIME IND : R        DOW : 34567\n"
                    "          START : 360     STOP : 1200\n"
                    "ADV/PURCHASE RSV: MON 33             ADV/TICKET ISSUE : X\n"
                    "      EQUIPMENT : 777\n"
                    "CXR/FLIGHT APPL T186 : 878787 \n"
                    " NOT AVAIL/NO CHARGE : G- FREE SERVICE - NO EMD AND NO BKG REQ\n"
                    "       CURRENCY T170 : 313222 \n"
                    "         AND/OR : \n"
                    "FF MILEAGE FEE  : 299992  \n"
                    "FEE APPLICATION : 2 - PER ROUND TRIP\n"
                    " REFUND/REISSUE : R - REUSE TOWARDS FUTURE\n"
                    " FORM OF REFUND : ELECTRONIC VOUCHER\n"
                    "     COMMISSION : Y\n"
                    "      INTERLINE : N\n"
                    "   COLLECT/SUBSTRACT : I   NET/SELL : N\n"
                    "            TAX APPL : X                 AVAIL CHECK : N\n"
                    "      TAX EXEMPT IND : N\n"
                    "RULE BUSTER SEGMENTS : 12      RULE BUSTER FARE CLASS: NOT USED\n"
                    "TEXT T196 : 9      \n"),
        _diag->str());
  }

  void testPrintS7DetailInfo_PartialTravelDates_Months_Only()
  {
    OptionalServicesInfo info;
    LocKey lk1, lk2, lkVia;
    populateLocs(lk1, lk2, lkVia);
    prepareOptionalServicesInfo(info, lk1, lk2, lkVia);
    info.tvlStartMonth() = 9;
    info.tvlStopMonth() = 11;
    _diag->printS7DetailInfo(&info, *_trx);
    CPPUNIT_ASSERT_EQUAL(
        std::string("------------- OPTIONAL SERVICE S7 DETAILED INFO --------------\n"
                    " SUBCODE : 0AA            VENDOR : ATP     SERVICE : FLIGHT   \n"
                    " SEQ NBR : 111233 \n"
                    " SRV EFF DATE : 2010-01-01      TVL DATE START : XXXX-09-XX\n"
                    " SRV DISC DATE: 2020-12-31      TVL DATE STOP  : XXXX-11-XX\n"
                    "  \n"
                    " PAX TYPE : CNN\n"
                    "      AGE MIN  : 1       MAX : 12\n"
                    "  OCCUR FIRST  : 1       LAST: 5  \n"
                    "   FF STATUS   : 4\n"
                    " PRIVATE IND   : P    TOUR CODE   : CPP UNIT TEST\n"
                    " ACC CODE T172 : 4444   \n"
                    " TKT DSGN T173 : 3333   \n"
                    " SECURITY T183 : 2222   \n"
                    "SECTOR/PORTION : P    GEO FTW IND : \n"
                    "     LOC1 TYPE : C    LOC1 : DFW       LOC1 ZONE : Z120    \n"
                    "     LOC2 TYPE : C    LOC2 : FRA       LOC2 ZONE : Z340    \n"
                    "VIA  LOC  TYPE : C    LOC  : LON       LOC  ZONE : Z919    \n"
                    "STOP/CONN/DEST : S - STOPOVER\n"
                    " STOPOVER TIME :     D\n"
                    "         CABIN : F\n"
                    "      RBD T198 : 55555  \n"
                    " UPGRADE CABIN : R\n"
                    " UPGR RBD T198 : 99999  \n"
                    "CXR/RESULT FARE CLASS T171 : 666666 \n"
                    "FARE TICKT DESIGNATOR T173 : 777    \n"
                    " RULETARIFF IND : PUB\n"
                    "    RULE TARIFF : 3       RULE : 4321   FARE IND : 19/22\n"
                    "TRAVEL TIME IND : R        DOW : 34567\n"
                    "          START : 360     STOP : 1200\n"
                    "ADV/PURCHASE RSV: MON 33             ADV/TICKET ISSUE : X\n"
                    "      EQUIPMENT : 777\n"
                    "CXR/FLIGHT APPL T186 : 878787 \n"
                    " NOT AVAIL/NO CHARGE : G- FREE SERVICE - NO EMD AND NO BKG REQ\n"
                    "       CURRENCY T170 : 313222 \n"
                    "         AND/OR : \n"
                    "FF MILEAGE FEE  : 299992  \n"
                    "FEE APPLICATION : 2 - PER ROUND TRIP\n"
                    " REFUND/REISSUE : R - REUSE TOWARDS FUTURE\n"
                    " FORM OF REFUND : ELECTRONIC VOUCHER\n"
                    "     COMMISSION : Y\n"
                    "      INTERLINE : N\n"
                    "   COLLECT/SUBSTRACT : I   NET/SELL : N\n"
                    "            TAX APPL : X                 AVAIL CHECK : N\n"
                    "      TAX EXEMPT IND : N\n"
                    "RULE BUSTER SEGMENTS : 12      RULE BUSTER FARE CLASS: NOT USED\n"
                    "TEXT T196 : 9      \n"),
        _diag->str());
  }

  void testPrintS7DetailInfo_TktEmptyDates()
  {
    OptionalServicesInfo info;
    LocKey lk1, lk2, lkVia;
    populateLocs(lk1, lk2, lkVia);
    prepareOptionalServicesInfo(info, lk1, lk2, lkVia);
    info.ticketEffDate() = DateTime::emptyDate();
    info.ticketDiscDate() = DateTime::emptyDate();

    _diag->printS7DetailInfo(&info, *_trx);
    CPPUNIT_ASSERT_EQUAL(
        std::string("------------- OPTIONAL SERVICE S7 DETAILED INFO --------------\n"
                    " SUBCODE : 0AA            VENDOR : ATP     SERVICE : FLIGHT   \n"
                    " SEQ NBR : 111233 \n"
                    " SRV EFF DATE : 2010-01-01      TRVL EFF DATE  : 1980-01-01\n"
                    " SRV DISC DATE: 2020-12-31      TRVL DISC DATE : 1980-01-01\n"
                    "  \n"
                    " PAX TYPE : CNN\n"
                    "      AGE MIN  : 1       MAX : 12\n"
                    "  OCCUR FIRST  : 1       LAST: 5  \n"
                    "   FF STATUS   : 4\n"
                    " PRIVATE IND   : P    TOUR CODE   : CPP UNIT TEST\n"
                    " ACC CODE T172 : 4444   \n"
                    " TKT DSGN T173 : 3333   \n"
                    " SECURITY T183 : 2222   \n"
                    "SECTOR/PORTION : P    GEO FTW IND : \n"
                    "     LOC1 TYPE : C    LOC1 : DFW       LOC1 ZONE : Z120    \n"
                    "     LOC2 TYPE : C    LOC2 : FRA       LOC2 ZONE : Z340    \n"
                    "VIA  LOC  TYPE : C    LOC  : LON       LOC  ZONE : Z919    \n"
                    "STOP/CONN/DEST : S - STOPOVER\n"
                    " STOPOVER TIME :     D\n"
                    "         CABIN : F\n"
                    "      RBD T198 : 55555  \n"
                    " UPGRADE CABIN : R\n"
                    " UPGR RBD T198 : 99999  \n"
                    "CXR/RESULT FARE CLASS T171 : 666666 \n"
                    "FARE TICKT DESIGNATOR T173 : 777    \n"
                    " RULETARIFF IND : PUB\n"
                    "    RULE TARIFF : 3       RULE : 4321   FARE IND : 19/22\n"
                    "TRAVEL TIME IND : R        DOW : 34567\n"
                    "          START : 360     STOP : 1200\n"
                    "ADV/PURCHASE RSV: MON 33             ADV/TICKET ISSUE : X\n"
                    "      EQUIPMENT : 777\n"
                    "CXR/FLIGHT APPL T186 : 878787 \n"
                    " NOT AVAIL/NO CHARGE : G- FREE SERVICE - NO EMD AND NO BKG REQ\n"
                    "       CURRENCY T170 : 313222 \n"
                    "         AND/OR : \n"
                    "FF MILEAGE FEE  : 299992  \n"
                    "FEE APPLICATION : 2 - PER ROUND TRIP\n"
                    " REFUND/REISSUE : R - REUSE TOWARDS FUTURE\n"
                    " FORM OF REFUND : ELECTRONIC VOUCHER\n"
                    "     COMMISSION : Y\n"
                    "      INTERLINE : N\n"
                    "   COLLECT/SUBSTRACT : I   NET/SELL : N\n"
                    "            TAX APPL : X                 AVAIL CHECK : N\n"
                    "      TAX EXEMPT IND : N\n"
                    "RULE BUSTER SEGMENTS : 12      RULE BUSTER FARE CLASS: NOT USED\n"
                    "TEXT T196 : 9      \n"),
        _diag->str());
  }

  void testPrintS7DetailInfo_RuleTariff_NotDefined()
  {
    OptionalServicesInfo info;
    LocKey lk1, lk2, lkVia;
    populateLocs(lk1, lk2, lkVia);
    prepareOptionalServicesInfo(info, lk1, lk2, lkVia);
    info.ticketEffDate() = DateTime::emptyDate();
    info.ticketDiscDate() = DateTime::emptyDate();
    info.ruleTariff() = (uint16_t) - 1;

    _diag->printS7DetailInfo(&info, *_trx);
    CPPUNIT_ASSERT_EQUAL(
        std::string("------------- OPTIONAL SERVICE S7 DETAILED INFO --------------\n"
                    " SUBCODE : 0AA            VENDOR : ATP     SERVICE : FLIGHT   \n"
                    " SEQ NBR : 111233 \n"
                    " SRV EFF DATE : 2010-01-01      TRVL EFF DATE  : 1980-01-01\n"
                    " SRV DISC DATE: 2020-12-31      TRVL DISC DATE : 1980-01-01\n"
                    "  \n"
                    " PAX TYPE : CNN\n"
                    "      AGE MIN  : 1       MAX : 12\n"
                    "  OCCUR FIRST  : 1       LAST: 5  \n"
                    "   FF STATUS   : 4\n"
                    " PRIVATE IND   : P    TOUR CODE   : CPP UNIT TEST\n"
                    " ACC CODE T172 : 4444   \n"
                    " TKT DSGN T173 : 3333   \n"
                    " SECURITY T183 : 2222   \n"
                    "SECTOR/PORTION : P    GEO FTW IND : \n"
                    "     LOC1 TYPE : C    LOC1 : DFW       LOC1 ZONE : Z120    \n"
                    "     LOC2 TYPE : C    LOC2 : FRA       LOC2 ZONE : Z340    \n"
                    "VIA  LOC  TYPE : C    LOC  : LON       LOC  ZONE : Z919    \n"
                    "STOP/CONN/DEST : S - STOPOVER\n"
                    " STOPOVER TIME :     D\n"
                    "         CABIN : F\n"
                    "      RBD T198 : 55555  \n"
                    " UPGRADE CABIN : R\n"
                    " UPGR RBD T198 : 99999  \n"
                    "CXR/RESULT FARE CLASS T171 : 666666 \n"
                    "FARE TICKT DESIGNATOR T173 : 777    \n"
                    " RULETARIFF IND : PUB\n"
                    "    RULE TARIFF :         RULE : 4321   FARE IND : 19/22\n"
                    "TRAVEL TIME IND : R        DOW : 34567\n"
                    "          START : 360     STOP : 1200\n"
                    "ADV/PURCHASE RSV: MON 33             ADV/TICKET ISSUE : X\n"
                    "      EQUIPMENT : 777\n"
                    "CXR/FLIGHT APPL T186 : 878787 \n"
                    " NOT AVAIL/NO CHARGE : G- FREE SERVICE - NO EMD AND NO BKG REQ\n"
                    "       CURRENCY T170 : 313222 \n"
                    "         AND/OR : \n"
                    "FF MILEAGE FEE  : 299992  \n"
                    "FEE APPLICATION : 2 - PER ROUND TRIP\n"
                    " REFUND/REISSUE : R - REUSE TOWARDS FUTURE\n"
                    " FORM OF REFUND : ELECTRONIC VOUCHER\n"
                    "     COMMISSION : Y\n"
                    "      INTERLINE : N\n"
                    "   COLLECT/SUBSTRACT : I   NET/SELL : N\n"
                    "            TAX APPL : X                 AVAIL CHECK : N\n"
                    "      TAX EXEMPT IND : N\n"
                    "RULE BUSTER SEGMENTS : 12      RULE BUSTER FARE CLASS: NOT USED\n"
                    "TEXT T196 : 9      \n"),
        _diag->str());
  }

  SvcFeesResBkgDesigInfo* getPadis(BookingCode bookingCode1,
                                   BookingCode bookingCode2,
                                   BookingCode bookingCode3,
                                   BookingCode bookingCode4,
                                   BookingCode bookingCode5,
                                   int seqNo)
  {
    SvcFeesResBkgDesigInfo* padis = _memHandle.create<SvcFeesResBkgDesigInfo>();
    if (bookingCode1 != "")
      padis->bookingCode1() = bookingCode1;
    if (bookingCode2 != "")
      padis->bookingCode2() = bookingCode2;
    if (bookingCode3 != "")
      padis->bookingCode3() = bookingCode3;
    if (bookingCode4 != "")
      padis->bookingCode4() = bookingCode4;
    if (bookingCode5 != "")
      padis->bookingCode5() = bookingCode5;
    padis->seqNo() = seqNo;

    return padis;
  }

  TravelSeg* getTravelSeg(const LocCode& originLocCode, const LocCode& destinationLocCode)
  {
    TravelSeg* travelSeg = _memHandle.create<AirSeg>();
    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = originLocCode;
    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = destinationLocCode;

    travelSeg->origin() = origin;
    travelSeg->destination() = destination;

    return travelSeg;
  }

  void testPrintS7DetailInfo_PADIS()
  {
    _diag->trx() = _memHandle.create<PricingTrx>();

    OCFees ocFees;
    ocFees.travelStart() = getTravelSeg("KRK", "ARN");
    ocFees.travelEnd() = getTravelSeg("ARN", "NYO");

    OptionalServicesInfo info;
    LocKey lk1, lk2, lkVia;
    populateLocs(lk1, lk2, lkVia);
    prepareOptionalServicesInfo(info, lk1, lk2, lkVia);
    std::vector<SvcFeesResBkgDesigInfo*> padisData;
    DateTime travelDate;

    // Create five SvcFeesResBkgDesigInfo objects with five booking codes and one seqNo.
    padisData.push_back(getPadis("A", "B", "", "", "", 133));
    padisData.push_back(getPadis("AA", "BA", "", "", "", 43));
    padisData.push_back(getPadis("AB", "BB", "", "", "", 42));
    padisData.push_back(getPadis("AC", "", "", "", "", 7));
    padisData.push_back(getPadis("AD", "", "", "", "", 52));

    _diag->printS7DetailInfoPadis(&info, ocFees, padisData, travelDate);
    std::string expectedResult = "*- - - - - - - - - - - - - - - - - - - - - - - - -  *\n"
                                 "* UPGRD RBD T198 ITEM NO : 99999     DETAIL INFO    *\n"
                                 "*- - - - - - - - - - - - - - - - - - - - - - - - - -*\n"
                                 "O/D      SEQ   PADIS TRANSLATION                     \n"
                                 "KRK-NYO  133   A     Aisle Seat DISP /AISLE/ \n"
                                 "               B     UNKNOWN\n"
                                 "KRK-NYO  43    AA    UNKNOWN\n"
                                 "               BA    UNKNOWN\n"
                                 "KRK-NYO  42    AB    CREW SEAT DISP /CREW/ \n"
                                 "               BB    UNKNOWN\n"
                                 "KRK-NYO  7     AC    WINDOW DISP /WINDO/ \n"
                                 "KRK-NYO  52    AD    WINDOW DISP /WINDO/ \n"
                                 "*- - - - - - - - - - - - - - - - - - - - - - - - -  *\n";
    CPPUNIT_ASSERT_EQUAL(expectedResult, _diag->str());
  }

  void testPrintS7DetailInfo_NotActive()
  {
    _diag->_active = false;
    _diag->printS7DetailInfo(NULL, *_trx);
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testDisplayStopCnxDestInd_C_Connection()
  {
    _diag->displayStopCnxDestInd('C');
    CPPUNIT_ASSERT_EQUAL(std::string("C - CONNECTION"), _diag->str());
  }

  void testDisplayStopCnxDestInd_F_NotFareBreak()
  {
    _diag->displayStopCnxDestInd('F');
    CPPUNIT_ASSERT_EQUAL(std::string("F - NOT FARE BREAK"), _diag->str());
  }

  void testDisplayStopCnxDestInd_P_NotFareBreakOR_Stopover()
  {
    _diag->displayStopCnxDestInd('P');
    CPPUNIT_ASSERT_EQUAL(std::string("P - NOT FARE BREAK OR STOPOVER"), _diag->str());
  }

  void testDisplayStopCnxDestInd_T_StopoverWithGeo()
  {
    _diag->displayStopCnxDestInd('T');
    CPPUNIT_ASSERT_EQUAL(std::string("T - STOPOVER WITH GEO"), _diag->str());
  }

  void testDisplayStopCnxDestInd_NotActive()
  {
    _diag->_active = false;
    _diag->displayStopCnxDestInd('X');
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testDisplaySourceForFareCreated_1_Cat19()
  {
    _diag->displaySourceForFareCreated('1');
    CPPUNIT_ASSERT_EQUAL(std::string("19/22\n"), _diag->str());
  }

  void testDisplaySourceForFareCreated_2_Cat25()
  {
    _diag->displaySourceForFareCreated('2');
    CPPUNIT_ASSERT_EQUAL(std::string("25\n"), _diag->str());
  }

  void testDisplaySourceForFareCreated_3_Cat35()
  {
    _diag->displaySourceForFareCreated('3');
    CPPUNIT_ASSERT_EQUAL(std::string("35\n"), _diag->str());
  }

  void testDisplaySourceForFareCreated_NotActive()
  {
    _diag->_active = false;
    _diag->displaySourceForFareCreated('X');
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testDisplayNotAvailNoCharge_X()
  {
    _diag->displayNotAvailNoCharge('X');
    CPPUNIT_ASSERT_EQUAL(std::string("X- SERVICE CODE IS NOT AVAILABLE\n"), _diag->str());
  }

  void testDisplayNotAvailNoCharge_F()
  {
    _diag->displayNotAvailNoCharge('F');
    CPPUNIT_ASSERT_EQUAL(std::string("F- FREE SERVICE - NO EMD ISSUED\n"), _diag->str());
  }

  void testDisplayNotAvailNoCharge_E()
  {
    _diag->displayNotAvailNoCharge('E');
    CPPUNIT_ASSERT_EQUAL(std::string("E- FREE SERVICE - EMD ISSUED\n"), _diag->str());
  }

  void testDisplayNotAvailNoCharge_H()
  {
    _diag->displayNotAvailNoCharge('H');
    CPPUNIT_ASSERT_EQUAL(std::string("H- FREE SERVICE - EMD ISSUED AND NO BKG REQ\n"),
                         _diag->str());
  }

  void testDisplayNotAvailNoCharge_NotActive()
  {
    _diag->_active = false;
    _diag->displayNotAvailNoCharge('X');
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testDisplayRefundReissue_Y()
  {
    _diag->displayRefundReissue('Y');
    CPPUNIT_ASSERT_EQUAL(std::string("Y - REFUNDABLE\n"), _diag->str());
  }

  void testDisplayRefundReissue_N()
  {
    _diag->displayRefundReissue('N');
    CPPUNIT_ASSERT_EQUAL(std::string("N - NON-REFUNDABLE\n"), _diag->str());
  }

  void testDisplayRefundReissue_NotActive()
  {
    _diag->_active = false;
    _diag->displayRefundReissue('X');
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testDisplayFFMileageAppl_1_PerOneWay()
  {
    _diag->displayFFMileageAppl('1');
    CPPUNIT_ASSERT_EQUAL(std::string("1 - PER ONE WAY"), _diag->str());
  }

  void testDisplayFFMileageAppl_2_PerRoundTrip()
  {
    _diag->displayFFMileageAppl('2');
    CPPUNIT_ASSERT_EQUAL(std::string("2 - PER ROUND TRIP"), _diag->str());
  }

  void testDisplayFFMileageAppl_3_PerItem()
  {
    _diag->displayFFMileageAppl('3');
    CPPUNIT_ASSERT_EQUAL(std::string("3 - PER ITEM"), _diag->str());
  }

  void testDisplayFFMileageAppl_4_PerTravel()
  {
    _diag->displayFFMileageAppl('4');
    CPPUNIT_ASSERT_EQUAL(std::string("4 - PER TRAVEL"), _diag->str());
  }

  void testDisplayFFMileageAppl_5_PerTicket()
  {
    _diag->displayFFMileageAppl('5');
    CPPUNIT_ASSERT_EQUAL(std::string("5 - PER TICKET"), _diag->str());
  }

  void testDisplayFFMileageAppl_NotActive()
  {
    _diag->_active = false;
    _diag->displayFFMileageAppl('X');
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testDisplayFormOfTheRefund_OriginalFOP()
  {
    _diag->displayFormOfTheRefund('1');
    CPPUNIT_ASSERT_EQUAL(std::string("ORIGINAL FOP\n"), _diag->str());
  }

  void testDisplayFormOfTheRefund_NotActive()
  {
    _diag->_active = false;
    _diag->displayFormOfTheRefund('X');
    CPPUNIT_ASSERT(_diag->str().empty());
  }

  void testPrintS7SoftMatchedFields_NoSoftMatch_Found()
  {
    OptionalServicesInfo info;
    OCFees ocFees;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : N/A\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_Cabin_Only()
  {
    OptionalServicesInfo info;
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_CABIN_SOFT, true);

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * CABIN\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_RBD_Only()
  {
    OptionalServicesInfo infoS;
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_RBD_SOFT, true);

    SvcFeesResBkgDesigInfo info;
    ocFees.softMatchRBDT198().push_back(&info);

    _diag->printS7SoftMatchedFields(&infoS, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * RBD T198\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_RULETARIFF_Only()
  {
    OptionalServicesInfo info;
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_RULETARIFF_SOFT, true);

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * RULE TARIFF\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }
  void testPrintS7SoftMatchedFields_RULE_Only()
  {
    OptionalServicesInfo info;
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_RULE_SOFT, true);

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * RULE\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_RESULTING_FARE_Only()
  {
    OptionalServicesInfo infoS;
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_RESULTING_FARE_SOFT, true);

    SvcFeesCxrResultingFCLInfo info;
    ocFees.softMatchResultingFareClassT171().push_back(&info);

    _diag->printS7SoftMatchedFields(&infoS, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * CXR/RESULT FARE CLASS\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_CXRFLIGHT_Only()
  {
    OptionalServicesInfo infoS;
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_CARRIER_FLIGHT_SOFT, true);

    CarrierFlightSeg info;
    setCFSinfo(info);
    ocFees.softMatchCarrierFlightT186().push_back(&info);

    _diag->printS7SoftMatchedFields(&infoS, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * CXR/FLIGHT APPL T186\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_EQUIPMENT_Only()
  {
    OptionalServicesInfo info;
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_EQUIPMENT_SOFT, true);

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * EQUIPMENT\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void prepareOCFees(OCFees& ocFees, OptionalServicesInfo& info)
  {
    LocKey lk1, lk2, lkVia;
    populateLocs(lk1, lk2, lkVia);
    prepareOptionalServicesInfo(info, lk1, lk2, lkVia);
  }

  void testPrintS7SoftMatchedFields_STOPCNXDESTIND_Only()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.stopoverTime() = "";
    info.stopoverUnit() = ' ';
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;
    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * STOP/CONN/DEST\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_STOPOVERTIME_Only()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.stopCnxDestInd() = ' ';
    info.stopoverUnit() = ' ';
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * STOPOVER TIME\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_STOPOVERUNIT_Only()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.stopCnxDestInd() = ' ';
    info.stopoverTime() = "";
    info.startTime() = 0;
    info.stopTime() = 0;

    ocFees.optFee() = &info;
    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * STOPOVER UNIT\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_STARTTIME_Only()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.stopCnxDestInd() = ' ';
    info.stopoverUnit() = ' ';
    info.stopoverTime() = "";
    info.stopTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * START TIME\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_STOPTIME_Only()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.stopCnxDestInd() = ' ';
    info.stopoverUnit() = ' ';
    info.stopoverTime() = "";
    info.startTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * STOP TIME\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_STOPCNXDESTIND_STOPOVERTIME()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.stopoverUnit() = ' ';
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;
    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * STOP/CONN/DEST * STOPOVER TIME\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_STOPCNXDESTIND_STOPOVERUNIT()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.stopoverTime() = "";
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;
    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * STOP/CONN/DEST * STOPOVER UNIT\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_STOPOVERTIME_STOPOVERUNIT()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.stopCnxDestInd() = ' ';
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;
    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * STOPOVER TIME * STOPOVER UNIT\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_STOPCNXDESTIND_STOPOVERTIME_STOPOVERUNIT()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;
    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * STOP/CONN/DEST * STOPOVER TIME * STOPOVER UNIT\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_STARTTIME_STOPTIME()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.stopCnxDestInd() = ' ';
    info.stopoverUnit() = ' ';
    info.stopoverTime() = "";
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * START TIME * STOP TIME\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_STOPCNXDESTIND_STOPOVERTIME_STOPOVERUNIT_STARTTIME()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.stopTime() = 0;
    ocFees.optFee() = &info;
    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * STOP/CONN/DEST * STOPOVER TIME * STOPOVER UNIT\n"
                    "              * START TIME\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_STOPCNXDESTIND_STOPOVERTIME_STOPOVERUNIT_STOPTIME()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.startTime() = 0;
    ocFees.optFee() = &info;
    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * STOP/CONN/DEST * STOPOVER TIME * STOPOVER UNIT\n"
                    "              * STOP TIME\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_STOPCNXDESTIND_STOPOVERTIME_STOPOVERUNIT_STARTTIME_STOPTIME()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    ocFees.optFee() = &info;
    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * STOP/CONN/DEST * STOPOVER TIME * STOPOVER UNIT\n"
                    "              * START TIME * STOP TIME\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_Cabin_RBD()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_CABIN_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_RBD_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);

    SvcFeesResBkgDesigInfo sfrbd;
    ocFees.softMatchRBDT198().push_back(&sfrbd);

    info.stopCnxDestInd() = ' ';
    info.stopoverUnit() = ' ';
    info.stopoverTime() = "";
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * CABIN * RBD T198\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_Cabin_RULETARIFF()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_CABIN_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_RULETARIFF_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.stopCnxDestInd() = ' ';
    info.stopoverUnit() = ' ';
    info.stopoverTime() = "";
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * CABIN * RULE TARIFF\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_Cabin_RULE()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_CABIN_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_RULE_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.stopCnxDestInd() = ' ';
    info.stopoverUnit() = ' ';
    info.stopoverTime() = "";
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * CABIN * RULE\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_Cabin_RESULTING_FARE()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_CABIN_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_RESULTING_FARE_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    SvcFeesCxrResultingFCLInfo sfcr;
    ocFees.softMatchResultingFareClassT171().push_back(&sfcr);

    info.stopCnxDestInd() = ' ';
    info.stopoverUnit() = ' ';
    info.stopoverTime() = "";
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * CABIN * CXR/RESULT FARE CLASS\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_Cabin_CXRFLIGHT()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_CABIN_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_CARRIER_FLIGHT_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    CarrierFlightSeg cfs;
    ocFees.softMatchCarrierFlightT186().push_back(&cfs);
    info.stopCnxDestInd() = ' ';
    info.stopoverUnit() = ' ';
    info.stopoverTime() = "";
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * CABIN * CXR/FLIGHT APPL T186\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_Cabin_EQUIPMENT()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_CABIN_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_EQUIPMENT_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.stopCnxDestInd() = ' ';
    info.stopoverUnit() = ' ';
    info.stopoverTime() = "";
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * CABIN * EQUIPMENT\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_RESULTING_FARE_CXRFLIGHT()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_CARRIER_FLIGHT_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_RESULTING_FARE_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    SvcFeesCxrResultingFCLInfo sfcr;
    ocFees.softMatchResultingFareClassT171().push_back(&sfcr);
    CarrierFlightSeg cfs;
    ocFees.softMatchCarrierFlightT186().push_back(&cfs);

    info.stopCnxDestInd() = ' ';
    info.stopoverUnit() = ' ';
    info.stopoverTime() = "";
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * CXR/RESULT FARE CLASS * CXR/FLIGHT APPL T186\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_RESULTING_FARE_CXRFLIGHT_STOPCNXDESTIND()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_CARRIER_FLIGHT_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_RESULTING_FARE_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    SvcFeesCxrResultingFCLInfo sfcr;
    ocFees.softMatchResultingFareClassT171().push_back(&sfcr);
    CarrierFlightSeg cfs;
    ocFees.softMatchCarrierFlightT186().push_back(&cfs);

    info.stopoverUnit() = ' ';
    info.stopoverTime() = "";
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * STOP/CONN/DEST * CXR/RESULT FARE CLASS\n"
                    "              * CXR/FLIGHT APPL T186\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_RESULTING_FARE_STOPCNXDESTIND_STOPOVERTIME()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_RESULTING_FARE_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    SvcFeesCxrResultingFCLInfo sfcr;
    ocFees.softMatchResultingFareClassT171().push_back(&sfcr);

    info.stopoverUnit() = ' ';
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * STOP/CONN/DEST * STOPOVER TIME\n"
                    "              * CXR/RESULT FARE CLASS\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_RESULTING_FARE_STOPCNXDESTIND_STOPOVERTIME_STOPOVERUNIT()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_RESULTING_FARE_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    SvcFeesCxrResultingFCLInfo sfcr;
    ocFees.softMatchResultingFareClassT171().push_back(&sfcr);

    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * STOP/CONN/DEST * STOPOVER TIME * STOPOVER UNIT\n"
                    "              * CXR/RESULT FARE CLASS\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedFields_RESULTING_FARE_RBD_CABIN_RULE()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_RESULTING_FARE_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_RBD_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_CABIN_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_RULE_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    SvcFeesCxrResultingFCLInfo sfcr;
    ocFees.softMatchResultingFareClassT171().push_back(&sfcr);
    SvcFeesResBkgDesigInfo sfrbd;
    ocFees.softMatchRBDT198().push_back(&sfrbd);

    info.stopCnxDestInd() = ' ';
    info.stopoverUnit() = ' ';
    info.stopoverTime() = "";
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * CABIN * RBD T198 * CXR/RESULT FARE CLASS\n"
                    "              * RULE\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrintS7SoftMatchedField_Frequent_Flyer_Equipment()
  {
    OCFees ocFees;
    ocFees.softMatchS7Status().set(OCFees::S7_FREQFLYER_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_EQUIPMENT_SOFT, true);
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.stopCnxDestInd() = ' ';
    info.stopoverUnit() = ' ';
    info.stopoverTime() = "";
    info.startTime() = 0;
    info.stopTime() = 0;
    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * FF STATUS * EQUIPMENT\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testPrint_All_S7SoftMatchedFields()
  {
    OCFees ocFees;
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    ocFees.softMatchS7Status().set(OCFees::S7_FREQFLYER_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_RESULTING_FARE_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_CARRIER_FLIGHT_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_RULETARIFF_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_EQUIPMENT_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_RBD_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_CABIN_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_RULE_SOFT, true);
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT, true);
    SvcFeesCxrResultingFCLInfo sfcr;
    ocFees.softMatchResultingFareClassT171().push_back(&sfcr);
    CarrierFlightSeg cfs;
    ocFees.softMatchCarrierFlightT186().push_back(&cfs);
    SvcFeesResBkgDesigInfo sfrbd;
    ocFees.softMatchRBDT198().push_back(&sfrbd);

    ocFees.optFee() = &info;

    _diag->printS7SoftMatchedFields(&info, ocFees);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"
                    " SOFT MATCH : * FF STATUS * STOP/CONN/DEST * STOPOVER TIME\n"
                    "              * STOPOVER UNIT * CABIN * RBD T198\n"
                    "              * CXR/RESULT FARE CLASS * RULE TARIFF * RULE\n"
                    "              * START TIME * STOP TIME * EQUIPMENT\n"
                    "              * CXR/FLIGHT APPL T186\n"
                    " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n"),
        _diag->str());
  }

  void testDisplayAmount_NoAvailNoCharges_X()
  {
    OCFees ocFees;
    OptionalServicesInfo info;
    prepareOCFees(ocFees, info);
    info.notAvailNoChargeInd() = 'X';
    ocFees.optFee() = &info;

    _diag->displayAmount(ocFees);
    CPPUNIT_ASSERT_EQUAL(std::string("   NOTAVAIL "), _diag->str());
  }

private:
  class Diag877CollectorDataHandleMock : public DataHandleMock
  {
    TestMemHandle _memHandle;

    SeatCabinCharacteristicInfo*
    getSeatCabinCharacteristic(const CarrierCode& carrier,
                               const Indicator& codeType,
                               const SeatCabinCode& seatCabinCode,
                               const std::string& codeDescription,
                               const std::string& displayDescription,
                               const std::string& abbreviatedDescription)
    {
      SeatCabinCharacteristicInfo* seatCabinCharacteristic =
          _memHandle.create<SeatCabinCharacteristicInfo>();
      seatCabinCharacteristic->carrier() = carrier;
      seatCabinCharacteristic->codeType() = codeType;
      seatCabinCharacteristic->seatCabinCode() = seatCabinCode;
      seatCabinCharacteristic->codeDescription() = codeDescription;
      seatCabinCharacteristic->displayDescription() = displayDescription;
      seatCabinCharacteristic->abbreviatedDescription() = abbreviatedDescription;
      return seatCabinCharacteristic;
    }

  public:
    const std::vector<SeatCabinCharacteristicInfo*>&
    getSeatCabinCharacteristicInfo(const CarrierCode& carrier,
                                   const Indicator& codeType,
                                   const DateTime& travelDate)
    {
      std::vector<SeatCabinCharacteristicInfo*>* result =
          _memHandle.create<std::vector<SeatCabinCharacteristicInfo*> >();

      result->push_back(
          getSeatCabinCharacteristic("2P", 'S', "A", "WINDOW", "WINDOW DISP", "WCHR"));
      result->push_back(
          getSeatCabinCharacteristic("2P", 'S', "AB", "CREW SEAT", "CREW SEAT DISP", "CREW"));
      result->push_back(
          getSeatCabinCharacteristic("2P", 'S', "AC", "WINDOW", "WINDOW DISP", "WINDO"));
      result->push_back(
          getSeatCabinCharacteristic("2P", 'S', "AD", "WINDOW", "WINDOW DISP", "WINDO"));
      result->push_back(
          getSeatCabinCharacteristic("**", 'S', "A", "Aisle Seat", "Aisle Seat DISP", "AISLE"));
      return *result;
    }
  };

  Diag877Collector* _diag;
  Diagnostic* _diagroot;
  TestMemHandle _memHandle;
  Diag877CollectorDataHandleMock* _dataHandleMock;
  PricingTrx* _trx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag877CollectorTest);

} // tse
