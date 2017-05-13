//----------------------------------------------------------------------------
//
//  File   :  AtaeResponseHandlerTest.cpp
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

#include "ATAE/AtaeResponseHandler.h"
#include "ATAE/AtaeRequest.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseEnums.h"
#include "Common/ClassOfService.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/RBDByCabinInfo.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;
namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const Cabin*
  getCabin(const CarrierCode& carrier, const BookingCode& classOfService, const DateTime& date)
  {
    if (carrier == "AA")
    {
      Cabin* ret = _memHandle.create<Cabin>();
      ret->carrier() = carrier;
      ret->classOfService() = classOfService;
      BookingCode cos = classOfService;
      if (cos == "A" || cos == "F" || cos == "P")
        ret->cabin().setFirstClass();
      else if (cos == "C" || cos == "D" || cos == "I" || cos == "J")
        ret->cabin().setBusinessClass();
      else
        ret->cabin().setEconomyClass();
      return ret;
    }
    return DataHandleMock::getCabin(carrier, classOfService, date);
  }
  const vector<RBDByCabinInfo*>&
                                    getRBDByCabin(const VendorCode& vendor,
                                                  const CarrierCode& carrier)
  {
    if (carrier == "AA")
    {
      LocKey lk1, lk2;
      lk1.loc() = "DFW";
      lk1.locType() = 'C';
      lk2.loc() = "FRA";
      lk2.locType() = 'C';
      std::vector<RBDByCabinInfo*>* ret = _memHandle.create<vector<RBDByCabinInfo*> >();
      RBDByCabinInfo* info = _memHandle.create<RBDByCabinInfo>();
      info->vendor() = "ATP";
      info->carrier() = "AA";
      info->sequenceNo() = 111233;
      info->globalDir() = "";
      info->effDate() = DateTime(2010, 1, 1);
      info->discDate() = DateTime(2025, 12, 31);
      info->createDate() = DateTime(2010, 1, 1);
      info->expireDate() = DateTime(2025, 12, 31);
      info->locKey1() = lk1;
      info->locKey2() = lk2;
      info->flightNo1() = 0;
      info->flightNo2() = 0;
      info->equipmentType() = "";
      info->firstTicketDate() = DateTime(2010, 1, 2);
      info->lastTicketDate() = DateTime(2025, 12, 31);
      info->bookingCodeCabinMap().insert(std::make_pair("F ",'F'));
      info->bookingCodeCabinMap().insert(std::make_pair("A ",'F'));
      info->bookingCodeCabinMap().insert(std::make_pair("J ",'C'));
      info->bookingCodeCabinMap().insert(std::make_pair("C ",'C'));
      info->bookingCodeCabinMap().insert(std::make_pair("I ",'C'));
      info->bookingCodeCabinMap().insert(std::make_pair("W ",'Y'));
      info->bookingCodeCabinMap().insert(std::make_pair("T ",'Y'));
      info->bookingCodeCabinMap().insert(std::make_pair("Y ",'Y'));
      ret->push_back(info);
      return *ret;
    }
    return DataHandleMock::getRBDByCabin(vendor, carrier, DateTime(2010, 1, 2));
  }
};
}

class AtaeResponseHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AtaeResponseHandlerTest);
  CPPUNIT_TEST(testParse_Found);
  CPPUNIT_TEST(testParse_NotFound);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  AtaeResponseHandler* _ataeResponseHandler;
  AirSeg* _airSeg;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();

    PricingTrx* trx = _memHandle.create<PricingTrx>();
    Billing* billing = _memHandle.create<Billing>();
    PricingOptions* options = _memHandle.create<PricingOptions>();
    options->journeyActivatedForPricing() = true;
    PricingRequest* pRequest = _memHandle.create<PricingRequest>();
    Agent* agent = _memHandle.create<Agent>();
    pRequest->ticketingAgent() = agent;
    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = "DFW";
    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = "FRA";
    _airSeg = _memHandle.create<AirSeg>();
    _airSeg->pnrSegment() = 1;
    _airSeg->origAirport() = "DFW";
    _airSeg->destAirport() = "FRA";
    _airSeg->origin() = origin;
    _airSeg->destination() = destination;
    _airSeg->carrier() = "AA";
    _airSeg->flightNumber() = 2192;
    _airSeg->departureDT()= DateTime(2011, 1, 1);
    ClassOfService* cos = _memHandle.create<ClassOfService>();
    cos->bookingCode() = "F";
    _airSeg->classOfService().push_back(cos);
    Itin* itin = _memHandle.create<Itin>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->travelSeg().push_back(_airSeg);
    itin->travelSeg().push_back(_airSeg);
    itin->fareMarket().push_back(fm);
    trx->itin().push_back(itin);
    trx->setRequest(pRequest);
    trx->billing() = billing;
    trx->setOptions(options);

    _ataeResponseHandler = _memHandle.insert(new AtaeResponseHandler(*trx));
    _ataeResponseHandler->initialize();

    std::string str;
    AtaeRequest ataeRequest(*trx, false);
    ataeRequest.build(str);
    _ataeResponseHandler->fareMarketsSentToAtae() = ataeRequest.fareMarketsSentToAtae();
    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue(
            "ATPCO_RBDBYCABIN_ANSWER_TABLE_ACTIVATION_DATE", activationDate, "PRICING_SVC");
    DateTime dt(activationDate, 0);
    trx->ticketingDate() = dt.subtractDays(2);
  }

  void tearDown() { _memHandle.clear(); }

  void testParse_Found()
  {
    std::string request("<ATS Q3B=\"0\" S1I=\"OK\"><ASL><ASO S1J=\"1\" Q2Z=\"0\">"
                        "<SGS Q2X=\"1\" S1F=\"9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9\" "
                        "S1G=\"FAJCDIWTYBHKMRVNLSQO\" Q3F=\"0\" Q3G=\"3\" N1A=\"Y\" Q3C=\"0\"/>"
                        "</ASO></ASL></ATS>");

    _ataeResponseHandler->parse(request.c_str());
    CPPUNIT_ASSERT_EQUAL(9, (int)_airSeg->classOfService()[0]->numSeats());
  }

  void testParse_NotFound()
  {
    std::string request("<ATS Q3B=\"0\" S1I=\"OK\"><ASL><ASO S1J=\"1\" Q2Z=\"0\">"
                        "<SGS Q2X=\"1\" S1F=\"0 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9\" "
                        "S1G=\"XAJCDIWTYBHKMRVNLSQO\" Q3F=\"0\" Q3G=\"3\" N1A=\"Y\" Q3C=\"0\"/>"
                        "</ASO></ASL></ATS>");

    _ataeResponseHandler->parse(request.c_str());
    CPPUNIT_ASSERT_EQUAL(0, (int)_airSeg->classOfService()[0]->numSeats());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(AtaeResponseHandlerTest);
}
