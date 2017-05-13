#include <string>
#include <time.h>
#include <iostream>

#include "Taxes/LegacyTaxes/TaxGB.h"
#include "Taxes/LegacyTaxes/TaxSP9202.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/PricingTrx.h"
#include "Common/DateTime.h"
#include "DBAccess/TaxCodeCabin.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Common/TseEnums.h"

#include "test/include/MockTseServer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"

using namespace std;

namespace tse
{

class TaxSP9202Mock : public TaxSP9202
{

public:
  TaxSP9202Mock() { _journeyBreak = 0; }
  virtual ~TaxSP9202Mock() {}

  virtual bool setJourneyBreakEndIndex(PricingTrx& trx,
                                       TaxResponse& taxResponse,
                                       TaxCodeReg& taxCodeReg,
                                       uint16_t& startIndex,
                                       uint16_t& endIndex)
  {
    return true;
  }
  virtual uint16_t findJourneyBreak(PricingTrx& trx,
                                    TaxResponse& taxResponse,
                                    TaxCodeReg& taxCodeReg,
                                    uint16_t startIndex)
  {
    _journeyBreak++;
    return _journeyBreak;
  }

  /*bool applyPortionOfTravel (PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg,
   uint16_t& startIndex, uint16_t& endIndex);
   bool validateLoc1( const Loc* & loc, TaxCodeReg& taxCodeReg);
   bool validateLoc2( const Loc* & loc, TaxCodeReg& taxCodeReg);
   bool isGeoMatch( const Loc &aLoc, LocTypeCode &locType, LocCode &loc, Indicator &locExclInd);
   bool isDomSeg( const TravelSeg* ts) const;
   uint16_t findMirrorImage( PricingTrx& trx, TaxResponse& taxResponse){return 0;}*/

  uint16_t _journeyBreak;
};

class TaxZQTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(TaxZQTest);

  CPPUNIT_TEST(testIsDomSeg_Dom);
  CPPUNIT_TEST(testIsDomSeg_Int);

  CPPUNIT_TEST(testApplyPortionOfTravelWrongStartIndex1);
  CPPUNIT_TEST(testApplyPortionOfTravelWrongStartIndex2);

  CPPUNIT_TEST_SUITE_END();

public:
  TaxZQTest() {}

  void testIsDomSeg_Dom()
  {
    Loc* origin1 = _memHandle.create<Loc>();
    origin1->loc() = string("LHR");
    origin1->nation() = string("GB");

    Loc* destination1 = _memHandle.create<Loc>();
    destination1->loc() = string("MAN");
    destination1->nation() = string("GB");

    AirSeg* travelSeg1 = _memHandle.create<AirSeg>();
    travelSeg1->origin() = origin1;
    travelSeg1->destination() = destination1;

    CPPUNIT_ASSERT(_taxZQ.isDomSeg(travelSeg1));
  }

  void testIsDomSeg_Int()
  {
    CPPUNIT_ASSERT(!_taxZQ.isDomSeg(_taxResponse->farePath()->itin()->travelSeg()[0]));
  }

  void testApplyPortionOfTravelWrongStartIndex1()
  {
    TaxSP9202Mock zqMock;
    zqMock._journeyBreak = 0; // first break is on last segment
    zqMock.preparePortionOfTravelIndexes(*_trx, *_taxResponse, _taxCodeReg);

    uint16_t startIndex = 1;
    uint16_t endIndex = 3;

    CPPUNIT_ASSERT(
        !zqMock.applyPortionOfTravel(*_trx, *_taxResponse, _taxCodeReg, startIndex, endIndex));
  }

  void testApplyPortionOfTravelWrongStartIndex2()
  {
    TaxSP9202Mock zqMock;
    zqMock._journeyBreak = 4; // first break is on last segment
    zqMock.preparePortionOfTravelIndexes(*_trx, *_taxResponse, _taxCodeReg);

    uint16_t startIndex = 1;
    uint16_t endIndex = 3;

    CPPUNIT_ASSERT(
        !zqMock.applyPortionOfTravel(*_trx, *_taxResponse, _taxCodeReg, startIndex, endIndex));
  }

  void setUp()
  {

    _trx = _memHandle.create<PricingTrx>();

    Loc* agentLocation = _memHandle.create<Loc>();
    agentLocation->loc() = string("LON");
    agentLocation->nation() = string("GB");

    Agent* agent = _memHandle.create<Agent>();
    agent->agentLocation() = agentLocation;

    PricingRequest* request = _memHandle.create<PricingRequest>();
    request->ticketingAgent() = agent;

    _trx->setRequest(request);

    Loc* _origin = _memHandle.create<Loc>();
    _origin->loc() = string("LHR");
    _origin->nation() = string("GB");

    Loc* _destination = _memHandle.create<Loc>();
    _destination->loc() = string("CDG");
    _destination->nation() = string("FR");

    AirSeg* _travelSeg = _memHandle.create<AirSeg>();
    _travelSeg->origin() = _origin;
    _travelSeg->destination() = _destination;
    _travelSeg->carrier() = "BA";
    _travelSeg->setBookingCode("F");

    // Set up the fare path

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(_travelSeg);
    itin->travelSeg().push_back(_travelSeg);
    itin->travelSeg().push_back(_travelSeg);
    itin->travelSeg().push_back(_travelSeg);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();

    _taxResponse->farePath() = farePath;

    Loc* _origin1 = _memHandle.create<Loc>();
    _origin1->loc() = string("LHR");
    _origin1->nation() = string("GB");

    Loc* _destination1 = _memHandle.create<Loc>();
    _destination1->loc() = string("MAN");
    _destination1->nation() = string("GB");

    AirSeg* _travelSeg1 = _memHandle.create<AirSeg>();
    _travelSeg1->origin() = _origin1;
    _travelSeg1->destination() = _destination1;
  }

  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  TaxResponse* _taxResponse;
  TaxSP9202 _taxZQ;

  Loc* _origin;
  Loc* _destination;
  AirSeg* _travelSeg;

  Loc* _origin1;
  Loc* _destination1;
  AirSeg* _travelSeg1;
  TaxCodeReg _taxCodeReg;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxZQTest);
}
