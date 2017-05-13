//----------------------------------------------------------------------------
//
//  File   :  PricingDssRequestTest.cpp
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
#include "ATAE/PricingDssRequest.h"

#include "ATAE/PricingDssFlightMapBuilder.h"
#include "ATAE/test/MockObjects.h"
#include "Common/DateTime.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "Xray/JsonMessage.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <boost/range/adaptors.hpp>

#include <iostream>
#include <fstream>

using namespace std;

namespace tse
{
class PricingDssRequestTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingDssRequestTest);
  CPPUNIT_TEST(testBuild);
  CPPUNIT_TEST_SUITE_END();

public:
  //-----------------------------------------------------------------
  // setUp()
  //-----------------------------------------------------------------
  void setUp() {}

  //-----------------------------------------------------------------
  // tearDown()
  //-----------------------------------------------------------------
  void tearDown() { _memHandle.clear(); }

  //---------------------------------------------------------------------
  // testbuildRequest()
  //---------------------------------------------------------------------
  void testBuild()
  {
    PricingTrx trx;
    trx.assignXrayJsonMessage(xray::JsonMessagePtr(new xray::JsonMessage("mid", "cid")));
    trx.getXrayJsonMessage()->setId("id");
    Billing* billing = _memHandle.create<Billing>();
    PricingRequest* pRequest = _memHandle.create<PricingRequest>();
    Agent* agent = _memHandle.create<Agent>();
    pRequest->ticketingAgent() = agent;
    trx.setRequest(pRequest);

    trx.billing() = billing;

    AirSeg* airSeg = _memHandle.create<AirSeg>();
    airSeg->origAirport() = "DFW";
    airSeg->destAirport() = "LGW";
    airSeg->carrier() = "BA";
    airSeg->flightNumber() = 2192;
    std::vector<AirSeg*> airSegments;
    airSegments.push_back(airSeg);

    CarrierSwapUtilMock carrierSwapMock;
    PricingDssFlightMapBuilder<CarrierSwapUtilMock, CurrentTimeMock> builder(
        carrierSwapMock, MethodGetFlownSchedule::NORMAL_GET_FLOWN);
    PricingDssFlightMap flightMap;
    builder.populateFlightMap(airSegments.begin(), airSegments.end(), flightMap);
    PricingDssRequest dssReq(trx);
    std::string request;

    CPPUNIT_ASSERT_NO_THROW(dssReq.build(flightMap | boost::adaptors::map_keys, request));
    CPPUNIT_ASSERT(request.find("XRA") != std::string::npos);
    CPPUNIT_ASSERT(request.find("CID=\"cid.id\"") != std::string::npos);
    CPPUNIT_ASSERT(request.find("MID") != std::string::npos);
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PricingDssRequestTest);
}
