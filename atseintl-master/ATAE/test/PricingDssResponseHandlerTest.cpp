//----------------------------------------------------------------------------
//
//  File   :  PricingDssResponseHandlerTest.cpp
//
//  Author :  Kul Shekhar
//
//  Copyright Sabre 2005
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s) have
//          been supplied.
//
//----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "ATAE/test/MockObjects.h"
#include "ATAE/PricingDssRequest.h"
#include "ATAE/PricingDssResponseHandler.h"
#include "ATAE/PricingDssFlightMapBuilder.h"
#include "Common/ClassOfService.h"
#include "Common/DateTime.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "DBAccess/DiskCache.h"


namespace tse
{

namespace
{
class HiddenStopDetailsComparator
{
public:
  bool operator()(const HiddenStopDetails& r1, const HiddenStopDetails& r2) const
  {
    return r1.airport() == r2.airport() && r1.equipment() == r2.equipment() &&
           r1.departureDate() == r2.departureDate() && r1.arrivalDate() == r2.arrivalDate();
  }
};

class PricingDssFlightComparator
{
public:
  bool operator()(const PricingDssFlight& r1, const PricingDssFlight& r2) const
  {
    return r1._origAirport == r2._origAirport && r1._destAirport == r2._destAirport &&
      r1._marketingCarrierCode == r2._marketingCarrierCode &&
      r1._marketingFlightNumber == r2._marketingFlightNumber &&
      r1._operatingCarrierCode == r2._operatingCarrierCode &&
      r1._operatingFlightNumber == r2._operatingFlightNumber &&
      r1._equipmentCode == r2._equipmentCode &&
      r1._bbrCarrier == r2._bbrCarrier &&
      r1._arrivalDayAdjust == r2._arrivalDayAdjust &&
      r1._localArrivalTime == r2._localArrivalTime &&
      r1._equipTypeFirstLeg == r2._equipTypeFirstLeg &&
      r1._equipTypeLastLeg == r2._equipTypeLastLeg &&
      (r1._offeredBookingCodes.size() == r2._offeredBookingCodes.size() &&
        std::equal(r1._offeredBookingCodes.begin(), r1._offeredBookingCodes.end(),
                   r2._offeredBookingCodes.begin())) &&
      (r1._hiddenStops.size() == r2._hiddenStops.size() &&
        std::equal(r1._hiddenStops.begin(), r1._hiddenStops.end(),
                   r2._hiddenStops.begin())) &&
      (r1._hiddenStopsDetails.size() == r2._hiddenStopsDetails.size() &&
        std::equal(r1._hiddenStopsDetails.begin(), r1._hiddenStopsDetails.end(),
                   r2._hiddenStopsDetails.begin(), HiddenStopDetailsComparator()));
  }
};
std::ostream& operator<<(std::ostream & stream, const HiddenStopDetails& hiddenStop)
{
  stream << "{" << hiddenStop.airport() << "," << hiddenStop.equipment() << ","
         << hiddenStop.departureDate().toIsoString() << "," << hiddenStop.arrivalDate().toIsoString() << "}";
  return stream;
}
}

class PricingDssResponseHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingDssResponseHandlerTest);

  CPPUNIT_TEST(testNoSegments);
  CPPUNIT_TEST(test_1_segment);
  CPPUNIT_TEST(test_2_segments);
  CPPUNIT_TEST(test_1_segments_2_hiddenStops);
  CPPUNIT_TEST(test_2_segments_2_hiddenStops_first);
  CPPUNIT_TEST(test_2_segments_2_hiddenStops_each);
  CPPUNIT_TEST(test_1_segment_flown);
  CPPUNIT_TEST(test_is_validating_cxr_gsa);
  CPPUNIT_TEST(test_is_historical);
  CPPUNIT_TEST(test_exception_in_MER);
  CPPUNIT_TEST(testHiddenStopDetails_HiddenStopDetails);
  CPPUNIT_TEST(testHiddenStopDetails_2HiddenStops);
  CPPUNIT_TEST(testHiddenStopDetails_0HiddenStops);

  CPPUNIT_TEST_SUITE_END();

public:

  void setUp()
  {
    _memHandle.create<DataHandleMock>();

    _trx = _memHandle.create<PricingTrx>();
    Billing* billing = _memHandle.create<Billing>();
    PricingRequest* pRequest = _memHandle.create<PricingRequest>();
    Agent* agent = _memHandle.create<Agent>();

    Customer* cust = _memHandle.create<Customer>();
    cust->crsCarrier() = "1F";
    cust->hostName() = "INFI";

    agent->agentTJR() = cust;
    pRequest->ticketingAgent() = agent;

    _trx->setRequest(pRequest);
    _trx->billing() = billing;

    flight1._origAirport = "DFW";
    flight1._destAirport = "LGW";
    flight1._marketingCarrierCode = "BA";
    flight1._marketingFlightNumber = 2192;
    flight1._operatingCarrierCode = "BA";
    flight1._operatingFlightNumber = 2192;
    flight1._equipmentCode = "777";
    flight1._bbrCarrier = 0;
    flight1._arrivalDayAdjust = 1;
    flight1._localArrivalTime = "07:20";
    flight1._equipTypeFirstLeg = "777";
    flight1._equipTypeLastLeg = "777";
    flight1._offeredBookingCodes = {"F","A","J","C","D","I","W","T","Y","B","H","K","M","R","V","N","L","S","Q","O"};
    flight1._hiddenStops = {"WAW", "LAX", "ORD"};

    flight2._origAirport = "KRK";
    flight2._destAirport = "WAW";
    flight2._marketingCarrierCode = "LO";
    flight2._marketingFlightNumber = 2193;
    flight2._operatingCarrierCode = "LX";
    flight2._operatingFlightNumber = 2192;
    flight2._equipmentCode = "747";
    flight2._bbrCarrier = 0;
    flight2._arrivalDayAdjust = 1;
    flight2._localArrivalTime = "07:20";
    flight2._equipTypeFirstLeg = "787";
    flight2._equipTypeLastLeg = "777";
    flight2._offeredBookingCodes = {"K","M","R","V","N","L","S","Q","O"};
    flight2._hiddenStops = {"FRA", "TXL"};
  }

  void tearDown() { _memHandle.clear(); }

  void testNoSegments()
  {
    PricingDssResponseHandler* resHandler = _memHandle.insert(new PricingDssResponseHandler(*_trx, true));
    resHandler->initialize();

    std::string response =
      "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\">  <FLL>  </FLL>  <PTM D83=\"0.000000\" "
      "D84=\"0.000373\" D85=\"0.000000\" D86=\"0.000240\" D87=\"0.000000\" D88=\"0.000072\"/></DSS>";

    resHandler->parse(response.c_str());
    CPPUNIT_ASSERT(resHandler->_dssFlights.empty());
  }

  void test_1_segment()
  {
    PricingDssResponseHandler* resHandler = _memHandle.insert(new PricingDssResponseHandler(*_trx, true));
    resHandler->initialize();

    std::string response =
      "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\">  <FLL>    <ASG DDA=\"13018\" "
      "BBR=\"false\" NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" "
      "ORG=\"DFW\" DST=\"LGW\" MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" "
      "ED2=\"2005-10-28\" ED3=\"SMTWTFS\" SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" "
      "OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" DSA=\"07:20\" DGA=\"60\" STC=\"0\" "
      "DTC=\"N\" IDA=\"0\" CXC=\"F A J C D I W T Y B H K M R V N L S Q O\" "
      "MX2=\"M M M M\" OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" TRS=\"II\" "
      "TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
      "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" "
      "DQ1=\"777\" DQT=\"W\" LOF=\"WAW LAX ORD\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" "
      "DS2=\"\" ED4=\"SMTWTFS\" TMS=\"2005-07-18 16:20:36\"/>  </FLL>  <PTM "
      "D83=\"0.000000\" D84=\"0.000373\" D85=\"0.000000\" D86=\"0.000240\" D87=\"0.000000\" "
      "D88=\"0.000072\"/></DSS>";

    resHandler->parse(response.c_str());
    CPPUNIT_ASSERT(resHandler->_dssFlights.size() == 1);
    CPPUNIT_ASSERT(PricingDssFlightComparator()(flight1, resHandler->_dssFlights.at(0)));
  }

  void test_2_segments()
  {
    PricingDssResponseHandler* resHandler = _memHandle.insert(new PricingDssResponseHandler(*_trx, true));
    resHandler->initialize();

    std::string response =
      "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\">  <FLL>    <ASG "
      "DDA=\"13018\" BBR=\"false\" NPR=\"false\" ICD=\"false\" "
      "CTT=\"false\" SSA=\"false\" BCC=\"\" ORG=\"DFW\" DST=\"LGW\" "
      "MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" "
      "ED2=\"2005-10-28\" ED3=\"SMTWTFS\" SCH=\"1656480267\" ODA=\"0\" "
      "ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" "
      "DSA=\"07:20\" DGA=\"60\" STC=\"0\" DTC=\"N\" IDA=\"0\" "
      "CXC=\"F A J C D I W T Y B H K M R V N L S Q O\" MX2=\"M M M M\" OCX=\"BA\" "
      "OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" TRS=\"II\" TRX=\" \" "
      "TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
      "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" "
      "EQT=\"W\" DQ1=\"777\" DQT=\"W\" LOF=\"WAW LAX ORD\" ORC=\"US\" "
      "DSC=\"GB\" ORS=\"TX\" DS2=\"\" ED4=\"SMTWTFS\" "
      "TMS=\"2005-07-18 16:20:36\"/>    <ASG DDA=\"13018\" BBR=\"false\" "
      "NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" "
      "ORG=\"KRK\" DST=\"WAW\" MXC=\"LO\" FLT=\"2193\" MFS=\"\" "
      "ED1=\"2005-07-23\" ED2=\"2005-10-28\" ED3=\"SMTWTFS\" "
      "SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" "
      "OTC=\"D\" DD2=\"1\" DSA=\"07:20\" DGA=\"60\" STC=\"0\" DTC=\"N\" "
      "IDA=\"0\" CXC=\"K M R V N L S Q O\" MX2=\"M M M M\" OCX=\"LX\" "
      "OFN=\"2192\" OPC=\"LX\" MXX=\"\" ALX=\"\" TRS=\"II\" TRX=\" \" "
      "TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
      "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"747\" EQ1=\"787\" "
      "EQT=\"W\" DQ1=\"777\" DQT=\"W\" LOF=\"FRA TXL\" ORC=\"US\" DSC=\"GB\" "
      "ORS=\"TX\" DS2=\"\" ED4=\"SMTWTFS\" TMS=\"2005-07-18 16:20:36\"/>  "
      "</FLL>  <PTM D83=\"0.000000\" D84=\"0.000373\" D85=\"0.000000\" "
      "D86=\"0.000240\" D87=\"0.000000\" D88=\"0.000072\"/></DSS>";

    resHandler->parse(response.c_str());
    CPPUNIT_ASSERT(resHandler->_dssFlights.size() == 2);
    CPPUNIT_ASSERT(PricingDssFlightComparator()(flight1, resHandler->_dssFlights.at(0)));
    CPPUNIT_ASSERT(PricingDssFlightComparator()(flight2, resHandler->_dssFlights.at(1)));
  }

  void test_1_segments_2_hiddenStops()
  {
    PricingDssResponseHandler* resHandler = _memHandle.insert(new PricingDssResponseHandler(*_trx, true));
    resHandler->initialize();

    std::string response =
      "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\">  <FLL>    <ASG DDA=\"13018\" "
      "BBR=\"false\" NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" "
      "BCC=\"\" ORG=\"DFW\" DST=\"LGW\" MXC=\"BA\" FLT=\"2192\" MFS=\"\" "
      "ED1=\"2005-07-23\" ED2=\"2005-10-28\" ED3=\"SMTWTFS\" SCH=\"1656480267\" "
      "ODA=\"0\" ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" "
      "DSA=\"07:20\" DGA=\"60\" STC=\"0\" DTC=\"N\" IDA=\"0\" "
      "CXC=\"F A J C D I W T Y B H K M R V N L S Q O\" MX2=\"M M M M\" "
      "OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" TRS=\"II\" TRX=\" \" "
      "TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" SMK=\"false\" "
      "CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" DQ1=\"777\" "
      "DQT=\"W\" LOF=\"WAW LAX ORD\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" DS2=\"\" "
      "ED4=\"SMTWTFS\" TMS=\"2005-07-18 16:20:36\">      <HSG A00=\"PHX\" "
      "B40=\"320\" D03=\"2014-05-20\" D04=\"2014-05-21\" D31=\"09:05\" "
      "D32=\"08:08\" D64=\"57\" D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>      "
      "<HSG A00=\"PH2\" B40=\"420\" D03=\"2015-05-20\" D04=\"2015-05-21\" "
      "D31=\"10:05\" D32=\"07:08\" D64=\"57\" D65=\"323\" D80=\"-420\" "
      "Q0M=\"1788\"/>    </ASG>  </FLL>  <PTM D83=\"0.000000\" D84=\"0.000373\" "
      "D85=\"0.000000\" D86=\"0.000240\" D87=\"0.000000\" D88=\"0.000072\"/></DSS>";
    resHandler->parse(response.c_str());
    CPPUNIT_ASSERT(resHandler->_dssFlights.size() == 1);

    HiddenStopDetails details1;
    details1.airport() = "PHX";
    details1.equipment() = "320";
    details1.departureDate() = boost::posix_time::from_iso_string("20140520T090500");
    details1.arrivalDate() = boost::posix_time::from_iso_string("20140521T080800");

    flight1._hiddenStopsDetails.push_back(details1);

    HiddenStopDetails details2;
    details2.airport() = "PH2";
    details2.equipment() = "420";
    details2.departureDate() = boost::posix_time::from_iso_string("20150520T100500");
    details2.arrivalDate() = boost::posix_time::from_iso_string("20150521T070800");

    flight1._hiddenStopsDetails.push_back(details2);

    CPPUNIT_ASSERT(PricingDssFlightComparator()(flight1, resHandler->_dssFlights.at(0)));
  }

  void test_2_segments_2_hiddenStops_first()
  {
    PricingDssResponseHandler* resHandler = _memHandle.insert(new PricingDssResponseHandler(*_trx, true));
    resHandler->initialize();

    std::string response =
      "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\">  <FLL>    <ASG DDA=\"13018\" "
      "BBR=\"false\" NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" "
      "ORG=\"DFW\" DST=\"LGW\" MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" "
      "ED2=\"2005-10-28\" ED3=\"SMTWTFS\" SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" "
      "OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" DSA=\"07:20\" DGA=\"60\" STC=\"0\" "
      "DTC=\"N\" IDA=\"0\" CXC=\"F A J C D I W T Y B H K M R V N L S Q O\" "
      "MX2=\"M M M M\" OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" TRS=\"II\" "
      "TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
      "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" "
      "DQ1=\"777\" DQT=\"W\" LOF=\"WAW LAX ORD\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" "
      "DS2=\"\" ED4=\"SMTWTFS\" TMS=\"2005-07-18 16:20:36\">      <HSG A00=\"PHX\" "
      "B40=\"320\" D03=\"2014-05-20\" D04=\"2014-05-21\" D31=\"09:05\" D32=\"08:08\" "
      "D64=\"57\" D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>      <HSG A00=\"PH2\" "
      "B40=\"420\" D03=\"2015-05-20\" D04=\"2015-05-21\" D31=\"10:05\" D32=\"07:08\" "
      "D64=\"57\" D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>    </ASG>    "
      "<ASG DDA=\"13018\" BBR=\"false\" NPR=\"false\" ICD=\"false\" CTT=\"false\" "
      "SSA=\"false\" BCC=\"\" ORG=\"KRK\" DST=\"WAW\" MXC=\"LO\" FLT=\"2193\" MFS=\"\" "
      "ED1=\"2005-07-23\" ED2=\"2005-10-28\" ED3=\"SMTWTFS\" SCH=\"1656480267\" ODA=\"0\" "
      "ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" DSA=\"07:20\" DGA=\"60\" "
      "STC=\"0\" DTC=\"N\" IDA=\"0\" CXC=\"K M R V N L S Q O\" MX2=\"M M M M\" OCX=\"LX\" "
      "OFN=\"2192\" OPC=\"LX\" MXX=\"\" ALX=\"\" TRS=\"II\" TRX=\" \" TRA=\" \" ONT=\"\" "
      "DOT=\"false\" LGS=\"false\" ETX=\"true\" SMK=\"false\" CHT=\"false\" FNL=\"false\" "
      "EQP=\"747\" EQ1=\"787\" EQT=\"W\" DQ1=\"777\" DQT=\"W\" LOF=\"FRA TXL\" ORC=\"US\" "
      "DSC=\"GB\" ORS=\"TX\" DS2=\"\" ED4=\"SMTWTFS\" TMS=\"2005-07-18 16:20:36\">   "
      "</ASG>  </FLL>  <PTM D83=\"0.000000\" D84=\"0.000373\" D85=\"0.000000\" "
      "D86=\"0.000240\" D87=\"0.000000\" D88=\"0.000072\"/></DSS>";

    resHandler->parse(response.c_str());

    HiddenStopDetails details1;
    details1.airport() = "PHX";
    details1.equipment() = "320";
    details1.departureDate() = boost::posix_time::from_iso_string("20140520T090500");
    details1.arrivalDate() = boost::posix_time::from_iso_string("20140521T080800");

    flight1._hiddenStopsDetails.push_back(details1);

    HiddenStopDetails details2;
    details2.airport() = "PH2";
    details2.equipment() = "420";
    details2.departureDate() = boost::posix_time::from_iso_string("20150520T100500");
    details2.arrivalDate() = boost::posix_time::from_iso_string("20150521T070800");

    flight1._hiddenStopsDetails.push_back(details2);

    CPPUNIT_ASSERT(resHandler->_dssFlights.size() == 2);
    CPPUNIT_ASSERT(PricingDssFlightComparator()(flight1, resHandler->_dssFlights.at(0)));
    CPPUNIT_ASSERT(PricingDssFlightComparator()(flight2, resHandler->_dssFlights.at(1)));
  }

  void test_2_segments_2_hiddenStops_each()
  {
    PricingDssResponseHandler* resHandler = _memHandle.insert(new PricingDssResponseHandler(*_trx, true));
    resHandler->initialize();

    std::string response =
      "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\">  <FLL>    <ASG DDA=\"13018\" "
      "BBR=\"false\" NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" "
      "ORG=\"DFW\" DST=\"LGW\" MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" "
      "ED2=\"2005-10-28\" ED3=\"SMTWTFS\" SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" "
      "OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" DSA=\"07:20\" DGA=\"60\" STC=\"0\" "
      "DTC=\"N\" IDA=\"0\" CXC=\"F A J C D I W T Y B H K M R V N L S Q O\" "
      "MX2=\"M M M M\" OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" TRS=\"II\" "
      "TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
      "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" "
      "DQ1=\"777\" DQT=\"W\" LOF=\"WAW LAX ORD\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" "
      "DS2=\"\" ED4=\"SMTWTFS\" TMS=\"2005-07-18 16:20:36\">      <HSG A00=\"PHX\" "
      "B40=\"320\" D03=\"2014-05-20\" D04=\"2014-05-21\" D31=\"09:05\" D32=\"08:08\" "
      "D64=\"57\" D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>      <HSG A00=\"PH2\" "
      "B40=\"420\" D03=\"2015-05-20\" D04=\"2015-05-21\" D31=\"10:05\" D32=\"07:08\" "
      "D64=\"57\" D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>    </ASG>    "
      "<ASG DDA=\"13018\" BBR=\"false\" NPR=\"false\" ICD=\"false\" CTT=\"false\" "
      "SSA=\"false\" BCC=\"\" ORG=\"KRK\" DST=\"WAW\" MXC=\"LO\" FLT=\"2193\" MFS=\"\" "
      "ED1=\"2005-07-23\" ED2=\"2005-10-28\" ED3=\"SMTWTFS\" SCH=\"1656480267\" "
      "ODA=\"0\" ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" DSA=\"07:20\" "
      "DGA=\"60\" STC=\"0\" DTC=\"N\" IDA=\"0\" CXC=\"K M R V N L S Q O\" "
      "MX2=\"M M M M\" OCX=\"LX\" OFN=\"2192\" OPC=\"LX\" MXX=\"\" ALX=\"\" TRS=\"II\" "
      "TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
      "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"747\" EQ1=\"787\" EQT=\"W\" "
      "DQ1=\"777\" DQT=\"W\" LOF=\"FRA TXL\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" DS2=\"\" "
      "ED4=\"SMTWTFS\" TMS=\"2005-07-18 16:20:36\">      <HSG A00=\"PH3\" B40=\"320\" "
      "D03=\"2014-05-20\" D04=\"2014-05-21\" D31=\"09:05\" D32=\"08:08\" D64=\"57\" "
      "D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>      <HSG A00=\"PH4\" B40=\"420\" "
      "D03=\"2015-05-20\" D04=\"2015-05-21\" D31=\"10:05\" D32=\"07:08\" D64=\"57\" "
      "D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>    </ASG>\t\t  </FLL>  "
      "<PTM D83=\"0.000000\" D84=\"0.000373\" D85=\"0.000000\" D86=\"0.000240\" "
      "D87=\"0.000000\" D88=\"0.000072\"/></DSS>";

    resHandler->parse(response.c_str());
    CPPUNIT_ASSERT(!resHandler->_dssFlights.empty());

    HiddenStopDetails details1;
    details1.airport() = "PHX";
    details1.equipment() = "320";
    details1.departureDate() = boost::posix_time::from_iso_string("20140520T090500");
    details1.arrivalDate() = boost::posix_time::from_iso_string("20140521T080800");

    flight1._hiddenStopsDetails.push_back(details1);
    details1.airport() = "PH3";
    flight2._hiddenStopsDetails.push_back(details1);

    HiddenStopDetails details2;
    details2.airport() = "PH2";
    details2.equipment() = "420";
    details2.departureDate() = boost::posix_time::from_iso_string("20150520T100500");
    details2.arrivalDate() = boost::posix_time::from_iso_string("20150521T070800");

    flight1._hiddenStopsDetails.push_back(details2);
    details2.airport() = "PH4";
    flight2._hiddenStopsDetails.push_back(details2);

    CPPUNIT_ASSERT(resHandler->_dssFlights.size() == 2);
    CPPUNIT_ASSERT(PricingDssFlightComparator()(flight1, resHandler->_dssFlights.at(0)));
    CPPUNIT_ASSERT(PricingDssFlightComparator()(flight2, resHandler->_dssFlights.at(1)));
  }

  void test_1_segment_flown()
  {
    PricingDssResponseHandler* resHandler = _memHandle.insert(new PricingDssResponseHandler(*_trx, false));
    resHandler->initialize();

    std::string response =
      "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\">  <FLL>    <ASG DDA=\"13018\" "
      "BBR=\"false\" NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" "
      "ORG=\"DFW\" DST=\"LGW\" MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" "
      "ED2=\"2005-10-28\" ED3=\"SMTWTFS\" SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" "
      "OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" DSA=\"07:20\" DGA=\"60\" STC=\"0\" "
      "DTC=\"N\" IDA=\"0\" CXC=\"F A J C D I W T Y B H K M R V N L S Q O\" "
      "MX2=\"M M M M\" OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" TRS=\"II\" "
      "TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
      "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" "
      "DQ1=\"777\" DQT=\"W\" LOF=\"WAW LAX ORD\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" "
      "DS2=\"\" ED4=\"SMTWTFS\" TMS=\"2005-07-18 16:20:36\"/>  </FLL>  <PTM "
      "D83=\"0.000000\" D84=\"0.000373\" D85=\"0.000000\" D86=\"0.000240\" D87=\"0.000000\" "
      "D88=\"0.000072\"/></DSS>";

    resHandler->parse(response.c_str());
    CPPUNIT_ASSERT(resHandler->_dssFlights.size() == 1);
    CPPUNIT_ASSERT(PricingDssFlightComparator()(flight1, resHandler->_dssFlights.at(0)));
  }

  void test_is_validating_cxr_gsa()
  {
    _trx->setValidatingCxrGsaApplicable(true);
    PricingDssResponseHandler* resHandler = _memHandle.insert(new PricingDssResponseHandler(*_trx, false));
    resHandler->initialize();

    std::string response =
        "<DSS VER=\"2.0\" COR=\"    \" TXN=\"7584100014683620320\">"
        "<MER COD=\"1\" MSG=\"No such flight\" SEI=\"-1\" CXR=\"KL\" FLN=\"3000\"/> "
        "<PTM D83=\"0.000000\" D84=\"0.000013\" D85=\"0.000000\" D86=\"0.000309\" D87=\"0.000000\" D88=\"0.000020\"/>"
        "</DSS>";

    CPPUNIT_ASSERT_NO_THROW(resHandler->parse(response.c_str()));
  }

  void test_is_historical()
  {
    _trx->ticketingDate() = DateTime(2014, 5, 21);
    PricingDssResponseHandler* resHandler = _memHandle.insert(new PricingDssResponseHandler(*_trx, true));
    resHandler->initialize();

    std::string response =
        "<DSS VER=\"2.0\" COR=\"    \" TXN=\"7584100014683620320\">"
        "<MER COD=\"1\" MSG=\"No such flight\" SEI=\"-1\" CXR=\"KL\" FLN=\"3000\"/> "
        "<PTM D83=\"0.000000\" D84=\"0.000013\" D85=\"0.000000\" D86=\"0.000309\" D87=\"0.000000\" D88=\"0.000020\"/>"
        "</DSS>";

    CPPUNIT_ASSERT_NO_THROW(resHandler->parse(response.c_str()));
  }

  void test_exception_in_MER()
  {
    _trx->setValidatingCxrGsaApplicable(true);
    _trx->ticketingDate() = DateTime(2014, 5, 21);

    PricingDssResponseHandler* resHandler = _memHandle.insert(new PricingDssResponseHandler(*_trx, true));
    resHandler->initialize();

    std::string response =
        "<DSS VER=\"2.0\" COR=\"    \" TXN=\"7584100014683620320\">"
        "<MER COD=\"1\" MSG=\"No such flight\" SEI=\"-1\" CXR=\"KL\" FLN=\"3000\"/> "
        "<PTM D83=\"0.000000\" D84=\"0.000013\" D85=\"0.000000\" D86=\"0.000309\" D87=\"0.000000\" D88=\"0.000020\"/>"
        "</DSS>";

    CPPUNIT_ASSERT_THROW(resHandler->parse(response.c_str()), std::string);
  }
  void testHiddenStopDetails_0HiddenStops()
  {
    PricingDssResponseHandler* resHandler = _memHandle.insert(new PricingDssResponseHandler(*_trx, false));
    resHandler->initialize();

    std::string response =
        "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\">"
        "<FLL>"
        "<ASG DDA=\"13018\" BBR=\"false\" "
        "NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" ORG=\"DFW\" DST=\"LGW\" "
        "MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" ED2=\"2005-10-28\" ED3=\"SMTWTFS\" "
        "SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" "
        "DSA=\"07:20\" DGA=\"60\" STC=\"0\" DTC=\"N\" IDA=\"0\" CXC=\"F A J C D I W T Y B H K M R "
        "V N L S Q O\" MX2=\"M M M M\" OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" "
        "TRS=\"II\" TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
        "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" DQ1=\"777\" "
        "DQT=\"W\" LOF=\"\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" DS2=\"\" ED4=\"SMTWTFS\" "
        "TMS=\"2005-07-18 16:20:36\">"
        "</ASG>"
        "</FLL>"
        "<PTM D83=\"0.000000\" D84=\"0.000373\" "
        "D85=\"0.000000\" D86=\"0.000240\" D87=\"0.000000\" D88=\"0.000072\"/></DSS>";

    CPPUNIT_ASSERT_NO_THROW(resHandler->parse(response.c_str()));
    CPPUNIT_ASSERT(resHandler->_dssFlights.size() == 1);
    CPPUNIT_ASSERT_EQUAL(size_t(0), resHandler->_dssFlights.front()._hiddenStopsDetails.size());

    //hidden stop[0] == hidden deteils[0].loc
  }

  void testHiddenStopDetails_2HiddenStops()
  {
    PricingDssResponseHandler* resHandler = _memHandle.insert(new PricingDssResponseHandler(*_trx, false));
    resHandler->initialize();

    std::string response =
        "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\">"
        "<FLL>"
        "<ASG DDA=\"13018\" BBR=\"false\" "
        "NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" ORG=\"DFW\" DST=\"LGW\" "
        "MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" ED2=\"2005-10-28\" ED3=\"SMTWTFS\" "
        "SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" "
        "DSA=\"07:20\" DGA=\"60\" STC=\"0\" DTC=\"N\" IDA=\"0\" CXC=\"F A J C D I W T Y B H K M R "
        "V N L S Q O\" MX2=\"M M M M\" OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" "
        "TRS=\"II\" TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
        "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" DQ1=\"777\" "
        "DQT=\"W\" LOF=\"\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" DS2=\"\" ED4=\"SMTWTFS\" "
        "TMS=\"2005-07-18 16:20:36\">"

        "<HSG A00=\"PHX\" B40=\"320\" D03=\"2014-05-21\" D04=\"2014-05-21\" D31=\"09:05\" "
          "D32=\"08:08\" D64=\"57\" D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>"

        "<HSG A00=\"PH2\" B40=\"320\" D03=\"2014-05-21\" D04=\"2014-05-21\" D31=\"09:05\" "
          "D32=\"08:08\" D64=\"57\" D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>"

        "</ASG>"
        "</FLL>"
        "<PTM D83=\"0.000000\" D84=\"0.000373\" "
        "D85=\"0.000000\" D86=\"0.000240\" D87=\"0.000000\" D88=\"0.000072\"/></DSS>";

    CPPUNIT_ASSERT_NO_THROW(resHandler->parse(response.c_str()));
    CPPUNIT_ASSERT(resHandler->_dssFlights.size() == 1);
    CPPUNIT_ASSERT_EQUAL(size_t(2), resHandler->_dssFlights.front()._hiddenStopsDetails.size());
  }

  void testHiddenStopDetails_HiddenStopDetails()
  {
    _trx->ticketingDate() = DateTime(2014, 5, 21);
    PricingDssResponseHandler* resHandler = _memHandle.insert(new PricingDssResponseHandler(*_trx, false));
    resHandler->initialize();

    std::string response =
        "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\">"
        "<FLL>"
        "<ASG DDA=\"13018\" BBR=\"false\" "
        "NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" ORG=\"DFW\" DST=\"LGW\" "
        "MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" ED2=\"2005-10-28\" ED3=\"SMTWTFS\" "
        "SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" "
        "DSA=\"07:20\" DGA=\"60\" STC=\"0\" DTC=\"N\" IDA=\"0\" CXC=\"F A J C D I W T Y B H K M R "
        "V N L S Q O\" MX2=\"M M M M\" OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" "
        "TRS=\"II\" TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
        "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" DQ1=\"777\" "
        "DQT=\"W\" LOF=\"\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" DS2=\"\" ED4=\"SMTWTFS\" "
        "TMS=\"2005-07-18 16:20:36\">"

        "<HSG A00=\"PHX\" B40=\"320\" D03=\"2014-05-20\" D04=\"2014-05-21\" D31=\"09:05\" "
          "D32=\"08:08\" D64=\"57\" D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>"

        "<HSG A00=\"PH2\" B40=\"420\" D03=\"2015-05-20\" D04=\"2015-05-21\" D31=\"10:05\" "
          "D32=\"07:08\" D64=\"57\" D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>"

        "</ASG>"
        "</FLL>"
        "<PTM D83=\"0.000000\" D84=\"0.000373\" "
        "D85=\"0.000000\" D86=\"0.000240\" D87=\"0.000000\" D88=\"0.000072\"/></DSS>";

    CPPUNIT_ASSERT_NO_THROW(resHandler->parse(response.c_str()));
    CPPUNIT_ASSERT(resHandler->_dssFlights.size() == 1);
    CPPUNIT_ASSERT_EQUAL(size_t(2), resHandler->_dssFlights.at(0)._hiddenStopsDetails.size());

    const HiddenStopDetails& details = resHandler->_dssFlights.at(0)._hiddenStopsDetails.at(1);

    CPPUNIT_ASSERT_EQUAL(LocCode("PH2"), details.airport());
    CPPUNIT_ASSERT_EQUAL(EquipmentType("420"), details.equipment());
    CPPUNIT_ASSERT_EQUAL(DateTime(2015, 5, 20, 10, 5), details.departureDate());
    CPPUNIT_ASSERT_EQUAL(DateTime(2015, 5, 21, 7, 8), details.arrivalDate());
  }



private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingDssFlight flight1, flight2;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PricingDssResponseHandlerTest);
}
