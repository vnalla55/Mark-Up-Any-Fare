#include "Taxes/LegacyTaxes/test/TaxUHTest.h"
#include "test/include/TestConfigInitializer.h"

#include "Taxes/LegacyTaxes/TaxUH.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
#include "Diagnostic/DCFactory.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/PricingTrx.h"

#include <string>

using namespace tse;
using namespace std;

void
TaxUHTest::testConstructor()
{
  try
  {
    TaxUH taxUH;
    return;
  }

  catch (...)
  {
    // If any error occured at all, then fail.
    CPPUNIT_ASSERT(false);
  }
}

void
TaxUHTest::testTaxUH_FromSVONoSU()
{
  // Set up the travel segment
  Loc* origin1 = _memHandle.create<Loc>();
  origin1->loc() = string("SVO");
  origin1->nation() = string("RU");

  Loc* destination1 = _memHandle.create<Loc>();
  destination1->loc() = string("FRA");
  destination1->nation() = string("FR");

  AirSeg* travelSeg1 = _memHandle.create<AirSeg>();
  travelSeg1->origin() = origin1;
  travelSeg1->destination() = destination1;
  travelSeg1->carrier() = "LH";
  travelSeg1->flightNumber() = 3189;
  travelSeg1->setBookingCode("H");

  Loc* origin2 = _memHandle.create<Loc>();
  origin2->loc() = string("FRA");
  origin2->nation() = string("FR");

  Loc* destination2 = _memHandle.create<Loc>();
  destination2->loc() = string("SVO");
  destination2->nation() = string("RU");

  AirSeg* travelSeg2 = _memHandle.create<AirSeg>();
  travelSeg2->origin() = origin2;
  travelSeg2->destination() = destination2;
  travelSeg2->carrier() = "LH";
  travelSeg1->flightNumber() = 3184;
  travelSeg2->setBookingCode("H");

  Itin* itin = _memHandle.create<Itin>();
  itin->travelSeg().push_back(travelSeg1);
  itin->travelSeg().push_back(travelSeg2);

  FarePath* farePath = _memHandle.create<FarePath>();
  farePath->itin() = itin;

  // Done setting up the farepath

  _taxResponse = _memHandle.create<TaxResponse>();
  DCFactory* factory = DCFactory::instance();
  DiagCollector& diagCollector = *(factory->create(*_trx));

  _taxResponse->diagCollector() = &diagCollector;

  _taxResponse->farePath() = farePath;

  // Do validation

  TaxUH taxUH;
  TaxCodeReg taxCodeReg;

  taxCodeReg.loc1Type() = tse::LocType('N');
  taxCodeReg.loc2Type() = tse::LocType('N');
  taxCodeReg.loc1() = tse::LocCode("CA");
  taxCodeReg.loc1ExclInd() = tse::Indicator('Y');
  taxCodeReg.loc2() = tse::LocCode("US");
  taxCodeReg.loc2ExclInd() = tse::Indicator('N');
  taxCodeReg.tripType() = tse::Indicator('F');
  taxCodeReg.specialProcessNo() = 52;
  taxCodeReg.tripType() = TripTypesValidator::TAX_FROM_TO;

  uint16_t startIndex = 0;
  uint16_t endIndex = 1; // it will fail the 2nd item

  bool rc = taxUH.validateTripTypes(*_trx, *_taxResponse, taxCodeReg, startIndex, endIndex);

  CPPUNIT_ASSERT_EQUAL(false, rc);
}

void
TaxUHTest::testTaxUH_FromSVOWithSU()
{
  // Set up the travel segment
  Loc* origin = _memHandle.create<Loc>();
  origin->loc() = string("SVO");
  origin->nation() = string("RU");

  Loc* destination = _memHandle.create<Loc>();
  destination->loc() = string("LED");
  destination->nation() = string("RU");

  AirSeg* travelSeg = _memHandle.create<AirSeg>();
  travelSeg->origin() = origin;
  travelSeg->destination() = destination;
  travelSeg->carrier() = "SU";
  travelSeg->flightNumber() = 779;
  travelSeg->setBookingCode("Y");

  Itin* itin = _memHandle.create<Itin>();
  itin->travelSeg().push_back(travelSeg);

  FarePath* farePath = _memHandle.create<FarePath>();
  farePath->itin() = itin;

  // Done setting up the farepath

  _taxResponse = _memHandle.create<TaxResponse>();
  DCFactory* factory = DCFactory::instance();
  DiagCollector& diagCollector = *(factory->create(*_trx));

  _taxResponse->diagCollector() = &diagCollector;

  _taxResponse->farePath() = farePath;

  // DO Validation

  uint16_t startIndex = 0;
  uint16_t endIndex = 0;

  TaxUH taxUH;
  TaxCodeReg taxCodeReg;

  taxCodeReg.loc1Type() = tse::LocType('N');
  taxCodeReg.loc2Type() = tse::LocType('N');
  taxCodeReg.loc1() = tse::LocCode("RU");
  taxCodeReg.loc1ExclInd() = tse::Indicator('N');
  taxCodeReg.loc2() = tse::LocCode("RU");
  taxCodeReg.loc2ExclInd() = tse::Indicator('N');
  taxCodeReg.specialProcessNo() = 52;
  taxCodeReg.tripType() = TripTypesValidator::TAX_FROM_TO;

  bool rc = taxUH.validateTripTypes(*_trx, *_taxResponse, taxCodeReg, startIndex, endIndex);

  CPPUNIT_ASSERT_EQUAL(true, rc);
}

void
TaxUHTest::testTaxUH_NotSVO()
{
  // Set up the travel segment

  Loc* origin = _memHandle.create<Loc>();
  origin->loc() = string("JFK");
  origin->nation() = string("US");

  Loc* destination = _memHandle.create<Loc>();
  destination->loc() = string("SVO");
  destination->nation() = string("RU");

  AirSeg* travelSeg = _memHandle.create<AirSeg>();
  travelSeg->origin() = origin;
  travelSeg->destination() = destination;
  travelSeg->carrier() = "SU";
  travelSeg->flightNumber() = 316;
  travelSeg->setBookingCode("F");

  Itin* itin = _memHandle.create<Itin>();
  itin->travelSeg().push_back(travelSeg);

  FarePath* farePath = _memHandle.create<FarePath>();
  farePath->itin() = itin;

  // Done setting up the farepath

  _taxResponse = _memHandle.create<TaxResponse>();
  DCFactory* factory = DCFactory::instance();
  DiagCollector& diagCollector = *(factory->create(*_trx));

  _taxResponse->diagCollector() = &diagCollector;

  _taxResponse->farePath() = farePath;

  // DO Validation

  uint16_t startIndex = 0;
  uint16_t endIndex = 0;

  TaxUH taxUH;
  TaxCodeReg taxCodeReg;

  taxCodeReg.loc1Type() = tse::LocType('N');
  taxCodeReg.loc2Type() = tse::LocType('N');
  taxCodeReg.loc1() = tse::LocCode("CA");
  taxCodeReg.loc1ExclInd() = tse::Indicator('N');
  taxCodeReg.loc2() = tse::LocCode("US");
  taxCodeReg.loc2ExclInd() = tse::Indicator('N');
  taxCodeReg.specialProcessNo() = 52;
  taxCodeReg.tripType() = TripTypesValidator::TAX_FROM_TO;

  bool rc = taxUH.validateTripTypes(*_trx, *_taxResponse, taxCodeReg, startIndex, endIndex);

  CPPUNIT_ASSERT_EQUAL(false, rc);
}

void
TaxUHTest::testTaxUH_Loc2Blank()
{
  // Set up the travel segment

  Loc* origin = _memHandle.create<Loc>();
  origin->loc() = string("JFK");
  origin->nation() = string("US");

  Loc* destination = _memHandle.create<Loc>();
  destination->loc() = string("SVO");
  destination->nation() = string("RU");

  AirSeg* travelSeg = _memHandle.create<AirSeg>();
  travelSeg->origin() = origin;
  travelSeg->destination() = destination;
  travelSeg->carrier() = "SU";
  travelSeg->flightNumber() = 316;
  travelSeg->setBookingCode("F");

  Itin* itin = _memHandle.create<Itin>();
  itin->travelSeg().push_back(travelSeg);

  FarePath* farePath = _memHandle.create<FarePath>();
  farePath->itin() = itin;

  // Done setting up the farepath

  _taxResponse = _memHandle.create<TaxResponse>();
  DCFactory* factory = DCFactory::instance();
  DiagCollector& diagCollector = *(factory->create(*_trx));

  _taxResponse->diagCollector() = &diagCollector;

  _taxResponse->farePath() = farePath;

  // DO Validation

  uint16_t startIndex = 0;
  uint16_t endIndex = 0;

  TaxUH taxUH;
  TaxCodeReg taxCodeReg;
  taxCodeReg.loc2Type() = tse::LocType(' ');
  taxCodeReg.loc1Type() = tse::LocType('N');
  taxCodeReg.loc1() = tse::LocCode("US");
  taxCodeReg.loc1ExclInd() = tse::Indicator('N');
  taxCodeReg.loc2() = tse::LocCode("US");
  taxCodeReg.loc2ExclInd() = tse::Indicator('N');
  taxCodeReg.specialProcessNo() = 52;
  taxCodeReg.tripType() = TripTypesValidator::TAX_FROM_TO;

  bool rc = taxUH.validateTripTypes(*_trx, *_taxResponse, taxCodeReg, startIndex, endIndex);

  CPPUNIT_ASSERT_EQUAL(true, rc);
}

void
TaxUHTest::setUp()
{
  _memHandle.create<TestConfigInitializer>();
  // Set up the PricingTrx
  _trx = _memHandle.create<PricingTrx>();

  Loc* agentLocation = _memHandle.create<Loc>();
  agentLocation->loc() = string("DFW");
  agentLocation->nation() = string("US");

  Agent* agent = _memHandle.create<Agent>();
  agent->agentLocation() = agentLocation;

  PricingRequest* request = _memHandle.create<PricingRequest>();
  request->ticketingAgent() = agent;

  _trx->setRequest(request);

  // Finished setting up trx.

  // Set up the travel segment

  Loc* origin1 = _memHandle.create<Loc>();
  origin1->loc() = string("SVO");
  origin1->nation() = string("RU");

  Loc* destination1 = _memHandle.create<Loc>();
  destination1->loc() = string("FRA");
  destination1->nation() = string("FR");

  AirSeg* travelSeg1 = _memHandle.create<AirSeg>();
  travelSeg1->origin() = origin1;
  travelSeg1->destination() = destination1;
  travelSeg1->carrier() = "LH";
  travelSeg1->flightNumber() = 3189;
  travelSeg1->setBookingCode("H");

  Loc* origin2 = _memHandle.create<Loc>();
  origin2->loc() = string("FRA");
  origin2->nation() = string("FR");

  Loc* destination2 = _memHandle.create<Loc>();
  destination2->loc() = string("SVO");
  destination2->nation() = string("RU");

  AirSeg* travelSeg2 = _memHandle.create<AirSeg>();
  travelSeg2->origin() = origin2;
  travelSeg2->destination() = destination2;
  travelSeg2->carrier() = "LH";
  travelSeg1->flightNumber() = 3184;
  travelSeg2->setBookingCode("H");

  // Set up the fare path

  Itin* itin = _memHandle.create<Itin>();
  itin->travelSeg().push_back(travelSeg1);
  itin->travelSeg().push_back(travelSeg2);

  FarePath* farePath = _memHandle.create<FarePath>();
  farePath->itin() = itin;

  // Done setting up the farepath

  _taxResponse = _memHandle.create<TaxResponse>();
  _taxResponse->farePath() = farePath;
}

void
TaxUHTest::tearDown()
{
  _memHandle.clear();
  _taxResponse = nullptr;
  _trx = nullptr;
}
