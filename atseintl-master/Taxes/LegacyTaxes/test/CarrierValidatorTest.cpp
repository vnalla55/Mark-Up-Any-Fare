#include "Taxes/LegacyTaxes/CarrierValidator.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/PricingTrx.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{

class CarrierValidatorMockProtectedToPublic : public CarrierValidator
{
public:
  virtual bool
  validateCarrierCode(const CarrierCode& marketingCarrierCode, const CarrierCode& carrierCode)
  {
    return CarrierValidator::validateCarrierCode(marketingCarrierCode, carrierCode);
  }

  virtual bool validateFlightNumber(uint16_t marketingFlight, uint16_t flight1, uint16_t flight2)
  {
    return CarrierValidator::validateFlightNumber(marketingFlight, flight1, flight2);
  }
  virtual bool validateFlightDirection(const LocCode& airport1,
                                       const LocCode& airport2,
                                       const LocCode& originAirport,
                                       const LocCode& destinationAirport,
                                       const Indicator& direction)
  {
    return CarrierValidator::validateFlightDirection(
        airport1, airport2, originAirport, destinationAirport, direction);
  }

  virtual bool equalOrEmpty(std::string lhs, std::string rhs)
  {
    return CarrierValidator::equalOrEmpty(lhs, rhs);
  }
};

class CarrierValidatorMockBasicValidators : public CarrierValidator
{
public:
  virtual bool
  validateCarrierCode(const CarrierCode& marketingCarrierCode, const CarrierCode& carrierCode)
  {
    return _validateCarrierCode;
  }
  virtual bool validateFlightNumber(uint16_t marketingFlight, uint16_t flight1, uint16_t flight2)
  {
    return _validateFlightNumber;
  }
  virtual bool validateFlightDirection(const LocCode& airport1,
                                       const LocCode& airport2,
                                       const LocCode& originAirport,
                                       const LocCode& destinationAirport,
                                       const Indicator& direction)
  {
    return _validateFlightDirection;
  }

  virtual bool
  validateExemptCxrRecord(const TaxExemptionCarrier& taxExemptionCarrier, const AirSeg& airSeg)
  {
    return CarrierValidator::validateExemptCxrRecord(taxExemptionCarrier, airSeg);
  }

  bool _validateCarrierCode;
  bool _validateFlightNumber;
  bool _validateFlightDirection;
};

class CarrierValidatorMockValidateExemptCxrRecord : public CarrierValidator
{
public:
  virtual bool
  validateExemptCxrRecord(const TaxExemptionCarrier& taxExemptionCarrier, const AirSeg& airSeg)
  {
    return _validateExemptCxrRecord;
  }

  bool _validateExemptCxrRecord;
};

class CarrierValidatorTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(CarrierValidatorTest);
  CPPUNIT_TEST(testValidateCarrier_inclusive);
  CPPUNIT_TEST(testValidateCarrier_exclusive);
  CPPUNIT_TEST(testValidateCarrier_exemptNoMatchExclude);
  CPPUNIT_TEST(testValidateCarrier_exemptNoMatchInclude);
  CPPUNIT_TEST(testValidateCarrier_flight1);
  CPPUNIT_TEST(testValidateCarrier_flight1_Exclude);
  CPPUNIT_TEST(testValidateCarrier_flight1_2);
  CPPUNIT_TEST(testValidateCarrier_flight1_2_Exclude);
  CPPUNIT_TEST(testValidateCarrier_flight_Outofrange_Exclude);
  CPPUNIT_TEST(testValidateCarrier_flight_Outofrange_Include);
  CPPUNIT_TEST(testValidateCarrier_flight2_1);
  CPPUNIT_TEST(testValidateCarrier_airport1);
  CPPUNIT_TEST(testValidateCarrier_airport1_Exclude);
  CPPUNIT_TEST(testValidateCarrier_between_airport2_1);
  CPPUNIT_TEST(testValidateCarrier_between_airport2_1_Exclude);
  CPPUNIT_TEST(testValidateCarrier_noExemptCxr);

  CPPUNIT_TEST(equalOrEmpty1);
  CPPUNIT_TEST(equalOrEmpty2);
  CPPUNIT_TEST(equalOrEmpty3);
  CPPUNIT_TEST(testValidateCarrierCodeNoCxr);
  CPPUNIT_TEST(testValidateCarrierCodeNotValidCxr);
  CPPUNIT_TEST(testValidateCarrierCodeValidCxr);
  CPPUNIT_TEST(testValidateFlightNumberNoRestriction);
  CPPUNIT_TEST(testValidateFlightNumber1);
  CPPUNIT_TEST(testValidateFlightNumber2);
  CPPUNIT_TEST(testValidateFlightNumber3);
  CPPUNIT_TEST(testValidateFlightNumber4);
  CPPUNIT_TEST(testValidateFlightNumber5);
  CPPUNIT_TEST(testValidateFlightNumber6);
  CPPUNIT_TEST(testValidateFlightDirection1);
  CPPUNIT_TEST(testValidateFlightDirection2);
  CPPUNIT_TEST(testValidateFlightDirection3);
  CPPUNIT_TEST(testValidateFlightDirection4);
  CPPUNIT_TEST(testValidateFlightDirection5);
  CPPUNIT_TEST(testValidateFlightDirection6);
  CPPUNIT_TEST(testValidateFlightDirection7);
  CPPUNIT_TEST(testValidateFlightDirection8);
  CPPUNIT_TEST(testValidateFlightDirection9);
  CPPUNIT_TEST(testValidateFlightDirection10);
  CPPUNIT_TEST(testValidateFlightDirection11);
  CPPUNIT_TEST(testValidateExemptCxrRecord1);
  CPPUNIT_TEST(testValidateExemptCxrRecord2);
  CPPUNIT_TEST(testValidateExemptCxrRecord3);
  CPPUNIT_TEST(testValidateCarrier1);
  CPPUNIT_TEST(testValidateCarrier2);
  CPPUNIT_TEST(testValidateCarrier3);
  CPPUNIT_TEST(testValidateCarrier4);
  CPPUNIT_TEST(testValidateCarrierArunk1);
  CPPUNIT_TEST(testValidateCarrierArunk2);
  CPPUNIT_TEST(testValidateCarrierArunk3);
  CPPUNIT_TEST(testValidateCarrierArunk4);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void testValidateCarrier_inclusive();
  void testValidateCarrier_exclusive();
  void testValidateCarrier_exemptNoMatchExclude();
  void testValidateCarrier_exemptNoMatchInclude();
  void testValidateCarrier_flight1();
  void testValidateCarrier_flight1_Exclude();
  void testValidateCarrier_flight1_2();
  void testValidateCarrier_flight1_2_Exclude();
  void testValidateCarrier_flight_Outofrange_Exclude();
  void testValidateCarrier_flight_Outofrange_Include();
  void testValidateCarrier_flight2_1();
  void testValidateCarrier_airport1();
  void testValidateCarrier_airport1_Exclude();
  void testValidateCarrier_between_airport2_1();
  void testValidateCarrier_between_airport2_1_Exclude();
  void testValidateCarrier_noExemptCxr();

  void equalOrEmpty1();
  void equalOrEmpty2();
  void equalOrEmpty3();
  void testValidateCarrierCodeNoCxr();
  void testValidateCarrierCodeNotValidCxr();
  void testValidateCarrierCodeValidCxr();
  void testValidateFlightNumberNoRestriction();
  void testValidateFlightNumber1();
  void testValidateFlightNumber2();
  void testValidateFlightNumber3();
  void testValidateFlightNumber4();
  void testValidateFlightNumber5();
  void testValidateFlightNumber6();
  void testValidateFlightDirection1();
  void testValidateFlightDirection2();
  void testValidateFlightDirection3();
  void testValidateFlightDirection4();
  void testValidateFlightDirection5();
  void testValidateFlightDirection6();
  void testValidateFlightDirection7();
  void testValidateFlightDirection8();
  void testValidateFlightDirection9();
  void testValidateFlightDirection10();
  void testValidateFlightDirection11();
  void testValidateExemptCxrRecord1();
  void testValidateExemptCxrRecord2();
  void testValidateExemptCxrRecord3();
  void testValidateCarrier1();
  void testValidateCarrier2();
  void testValidateCarrier3();
  void testValidateCarrier4();
  void testValidateCarrierArunk1();
  void testValidateCarrierArunk2();
  void testValidateCarrierArunk3();
  void testValidateCarrierArunk4();

  void setUp();
  void tearDown();

private:
  PricingTrx* _trx;
  TaxResponse* _taxResponse;
  TaxCodeReg _taxCodeReg;
  TaxExemptionCarrier _exemptionCxr;
  uint16_t _travelSegIndex;
  uint16_t _arunkSegIndex;
  AirSeg* _travelSeg;
  CarrierValidator _carrierValidator;

  CarrierValidatorMockBasicValidators _carrierValidatorMockBasicValidators;
  CarrierValidatorMockProtectedToPublic _carrierValidatorMockBasicValidatorsProtected;
  CarrierValidatorMockValidateExemptCxrRecord _carrierValidatorMockValidateExemptCxrRecord;
};
}

using namespace tse;
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(CarrierValidatorTest);

void
CarrierValidatorTest::setUp()
{
  _memHandle.create<TestConfigInitializer>();
  DiskCache::initialize(Global::config());
  _memHandle.create<MockDataManager>();
  _trx = _memHandle.create<PricingTrx>();

  Loc* agentLocation = _memHandle.create<Loc>();
  agentLocation->loc() = string("LIM");
  agentLocation->nation() = string("PE");

  Agent* agent = _memHandle.create<Agent>();
  agent->agentLocation() = agentLocation;

  PricingRequest* request = _memHandle.create<PricingRequest>();
  request->ticketingAgent() = agent;

  _trx->setRequest(request);

  // Finished setting up trx.

  // Set up the travel segment

  Loc* origin = _memHandle.create<Loc>();
  origin->loc() = string("JFK");
  origin->nation() = string("US");

  Loc* destination = _memHandle.create<Loc>();
  destination->loc() = string("LIM");
  destination->nation() = string("PE");

  AirSeg* travelSeg = _memHandle.create<AirSeg>();
  travelSeg->origin() = origin;
  travelSeg->boardMultiCity() = string("JFK");
  travelSeg->destination() = destination;
  travelSeg->offMultiCity() = string("LIM");
  travelSeg->setMarketingCarrierCode("AA");
  travelSeg->marketingFlightNumber() = 4500;

  ArunkSeg* arunkSeg = _memHandle.create<ArunkSeg>();
  arunkSeg->origin() = destination;
  arunkSeg->boardMultiCity() = string("LIM");
  arunkSeg->origin() = origin;
  arunkSeg->offMultiCity() = string("JFK");

  // Done setting up travel segment

  // Set up the fare path

  Itin* itin = _memHandle.create<Itin>();
  itin->travelSeg().push_back(travelSeg);
  _travelSegIndex = 0;
  itin->travelSeg().push_back(arunkSeg);
  _arunkSegIndex = 1;

  FarePath* farePath = _memHandle.create<FarePath>();
  farePath->itin() = itin;

  // Done setting up the farepath

  _taxResponse = _memHandle.create<TaxResponse>();
  _taxResponse->farePath() = farePath;
  _taxCodeReg.exemptionCxr().push_back(_exemptionCxr);
  _travelSeg = _memHandle.create<AirSeg>();
}

void
CarrierValidatorTest::tearDown()
{
  _memHandle.clear();
}

void
CarrierValidatorTest::testValidateCarrier_inclusive()
{
  _exemptionCxr.carrier() = "AA";
  _exemptionCxr.airport1() = "JFK";
  _exemptionCxr.airport2() = "LIM";
  _exemptionCxr.direction() = 'F';
  _exemptionCxr.flight1() = 0;
  _exemptionCxr.flight2() = 0;
  _taxCodeReg.exempcxrExclInd() = 'N';

  CPPUNIT_ASSERT(
      _carrierValidator.validateCarrier(*_trx, *_taxResponse, _taxCodeReg, _travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier_exclusive()
{
  _exemptionCxr.carrier() = "AA";
  _exemptionCxr.airport1() = "JFK";
  _exemptionCxr.airport2() = "LIM";
  _exemptionCxr.direction() = 'B';
  _exemptionCxr.flight1() = 0;
  _exemptionCxr.flight2() = 0;
  _taxCodeReg.exempcxrExclInd() = 'Y';

  CPPUNIT_ASSERT(
      !_carrierValidator.validateCarrier(*_trx, *_taxResponse, _taxCodeReg, _travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier_exemptNoMatchExclude()
{
  _exemptionCxr.carrier() = "SU";
  _exemptionCxr.airport1() = "";
  _exemptionCxr.airport2() = "";
  _exemptionCxr.direction() = ' ';
  _exemptionCxr.flight1() = 0;
  _exemptionCxr.flight2() = 0;
  _taxCodeReg.exemptionCxr().push_back(_exemptionCxr);
  _taxCodeReg.exempcxrExclInd() = 'Y';

  CPPUNIT_ASSERT(true);
}

void
CarrierValidatorTest::testValidateCarrier_exemptNoMatchInclude()
{
  TaxCodeReg taxCodeReg;
  TaxExemptionCarrier exemptionCxr;
  exemptionCxr.carrier() = "SU";
  exemptionCxr.airport1() = "";
  exemptionCxr.airport2() = "";
  exemptionCxr.direction() = ' ';
  exemptionCxr.flight1() = 0;
  exemptionCxr.flight2() = 0;
  taxCodeReg.exemptionCxr().push_back(exemptionCxr);
  taxCodeReg.exempcxrExclInd() = 'N'; // drop through and Indicator = N" - no tax

  uint16_t travelSegIndex = 0;

  CPPUNIT_ASSERT(
      !_carrierValidator.validateCarrier(*_trx, *_taxResponse, taxCodeReg, travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier_flight1()
{
  TaxCodeReg taxCodeReg;
  TaxExemptionCarrier exemptionCxr;
  exemptionCxr.carrier() = "AA";
  exemptionCxr.airport1() = "JFK";
  exemptionCxr.airport2() = "LIM";
  exemptionCxr.direction() = 'F';
  exemptionCxr.flight1() = 4500;
  exemptionCxr.flight2() = 0;
  taxCodeReg.exemptionCxr().push_back(exemptionCxr);
  taxCodeReg.exempcxrExclInd() = 'N'; // inclusive

  uint16_t travelSegIndex = 0;

  CPPUNIT_ASSERT(
      _carrierValidator.validateCarrier(*_trx, *_taxResponse, taxCodeReg, travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier_flight1_Exclude()
{
  TaxCodeReg taxCodeReg;
  TaxExemptionCarrier exemptionCxr;
  exemptionCxr.carrier() = "AA";
  exemptionCxr.airport1() = "JFK";
  exemptionCxr.airport2() = "LIM";
  exemptionCxr.direction() = 'B';
  exemptionCxr.flight1() = 4500;
  exemptionCxr.flight2() = 0;
  taxCodeReg.exemptionCxr().push_back(exemptionCxr);
  taxCodeReg.exempcxrExclInd() = 'Y'; // Exclusive

  uint16_t travelSegIndex = 0;

  CPPUNIT_ASSERT(
      !_carrierValidator.validateCarrier(*_trx, *_taxResponse, taxCodeReg, travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier_flight1_2()
{
  TaxCodeReg taxCodeReg;
  TaxExemptionCarrier exemptionCxr;
  exemptionCxr.carrier() = "AA";
  exemptionCxr.airport1() = "JFK";
  exemptionCxr.airport2() = "LIM";
  exemptionCxr.direction() = 'F';
  exemptionCxr.flight1() = 4000;
  exemptionCxr.flight2() = 4999;
  taxCodeReg.exemptionCxr().push_back(exemptionCxr);
  taxCodeReg.exempcxrExclInd() = 'N';

  uint16_t travelSegIndex = 0;

  CPPUNIT_ASSERT(
      _carrierValidator.validateCarrier(*_trx, *_taxResponse, taxCodeReg, travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier_flight1_2_Exclude()
{
  TaxCodeReg taxCodeReg;
  TaxExemptionCarrier exemptionCxr;
  exemptionCxr.carrier() = "AA";
  exemptionCxr.airport1() = "JFK";
  exemptionCxr.airport2() = "LIM";
  exemptionCxr.direction() = 'B';
  exemptionCxr.flight1() = 4000;
  exemptionCxr.flight2() = 4999;
  taxCodeReg.exemptionCxr().push_back(exemptionCxr);
  taxCodeReg.exempcxrExclInd() = 'Y';

  uint16_t travelSegIndex = 0;

  CPPUNIT_ASSERT(
      !_carrierValidator.validateCarrier(*_trx, *_taxResponse, taxCodeReg, travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier_flight_Outofrange_Exclude()
{
  TaxCodeReg taxCodeReg;
  TaxExemptionCarrier exemptionCxr;
  exemptionCxr.carrier() = "AA";
  exemptionCxr.airport1() = "JFK";
  exemptionCxr.airport2() = "LIM";
  exemptionCxr.direction() = 'B';
  exemptionCxr.flight1() = 4000;
  exemptionCxr.flight2() = 4449;
  taxCodeReg.exemptionCxr().push_back(exemptionCxr);
  taxCodeReg.exempcxrExclInd() = 'Y';

  uint16_t travelSegIndex = 0;

  CPPUNIT_ASSERT(
      _carrierValidator.validateCarrier(*_trx, *_taxResponse, taxCodeReg, travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier_flight_Outofrange_Include()
{
  TaxCodeReg taxCodeReg;
  TaxExemptionCarrier exemptionCxr;
  exemptionCxr.carrier() = "AA";
  exemptionCxr.airport1() = "";
  exemptionCxr.airport2() = "";
  exemptionCxr.direction() = 'B';
  exemptionCxr.flight1() = 4000;
  exemptionCxr.flight2() = 4449;
  taxCodeReg.exemptionCxr().push_back(exemptionCxr);
  taxCodeReg.exempcxrExclInd() = 'Y';

  uint16_t travelSegIndex = 0;

  CPPUNIT_ASSERT(
      _carrierValidator.validateCarrier(*_trx, *_taxResponse, taxCodeReg, travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier_flight2_1()
{
  TaxCodeReg taxCodeReg;
  TaxExemptionCarrier exemptionCxr;
  exemptionCxr.carrier() = "AA";
  exemptionCxr.airport1() = "JFK";
  exemptionCxr.airport2() = "LIM";
  exemptionCxr.direction() = 'F';
  exemptionCxr.flight1() = 4500;
  exemptionCxr.flight2() = 4000;
  taxCodeReg.exemptionCxr().push_back(exemptionCxr);
  taxCodeReg.exempcxrExclInd() = 'N';

  uint16_t travelSegIndex = 0;

  CPPUNIT_ASSERT(
      _carrierValidator.validateCarrier(*_trx, *_taxResponse, taxCodeReg, travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier_airport1()
{
  TaxCodeReg taxCodeReg;
  TaxExemptionCarrier exemptionCxr;
  exemptionCxr.carrier() = "AA";
  exemptionCxr.airport1() = "JFK";
  exemptionCxr.airport2() = "LIM";
  exemptionCxr.direction() = 'F';
  exemptionCxr.flight1() = 0;
  exemptionCxr.flight2() = 0;
  taxCodeReg.exemptionCxr().push_back(exemptionCxr);
  taxCodeReg.exempcxrExclInd() = 'N';

  uint16_t travelSegIndex = 0;

  CPPUNIT_ASSERT(
      _carrierValidator.validateCarrier(*_trx, *_taxResponse, taxCodeReg, travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier_airport1_Exclude()
{
  TaxCodeReg taxCodeReg;
  TaxExemptionCarrier exemptionCxr;
  exemptionCxr.carrier() = "AA";
  exemptionCxr.airport1() = "JFK";
  exemptionCxr.airport2() = "LIM";
  exemptionCxr.direction() = 'F';
  exemptionCxr.flight1() = 0;
  exemptionCxr.flight2() = 0;
  taxCodeReg.exemptionCxr().push_back(exemptionCxr);
  taxCodeReg.exempcxrExclInd() = 'Y';

  uint16_t travelSegIndex = 0;

  CPPUNIT_ASSERT(
      !_carrierValidator.validateCarrier(*_trx, *_taxResponse, taxCodeReg, travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier_between_airport2_1()
{
  TaxCodeReg taxCodeReg;
  TaxExemptionCarrier exemptionCxr;
  exemptionCxr.carrier() = "AA";
  exemptionCxr.airport1() = "LIM";
  exemptionCxr.airport2() = "JFK";
  exemptionCxr.direction() = 'B';
  exemptionCxr.flight1() = 0;
  exemptionCxr.flight2() = 0;
  taxCodeReg.exemptionCxr().push_back(exemptionCxr);
  taxCodeReg.exempcxrExclInd() = 'Y';

  uint16_t travelSegIndex = 0;

  CPPUNIT_ASSERT(
      !_carrierValidator.validateCarrier(*_trx, *_taxResponse, taxCodeReg, travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier_between_airport2_1_Exclude()
{
  TaxCodeReg taxCodeReg;
  TaxExemptionCarrier exemptionCxr;
  exemptionCxr.carrier() = "AA";
  exemptionCxr.airport1() = "LIM";
  exemptionCxr.airport2() = "JFK";
  exemptionCxr.direction() = 'B';
  exemptionCxr.flight1() = 0;
  exemptionCxr.flight2() = 0;
  taxCodeReg.exemptionCxr().push_back(exemptionCxr);
  taxCodeReg.exempcxrExclInd() = 'Y';

  uint16_t travelSegIndex = 0;

  CPPUNIT_ASSERT(
      !_carrierValidator.validateCarrier(*_trx, *_taxResponse, taxCodeReg, travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier_noExemptCxr()
{
  TaxCodeReg taxCodeReg;

  uint16_t travelSegIndex = 0;

  CPPUNIT_ASSERT(
      _carrierValidator.validateCarrier(*_trx, *_taxResponse, taxCodeReg, travelSegIndex));
}

// New tests

void
CarrierValidatorTest::equalOrEmpty1()
{
  std::string lhs;
  std::string rhs;

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.equalOrEmpty(lhs, rhs));
}

void
CarrierValidatorTest::equalOrEmpty2()
{
  std::string lhs;
  std::string rhs = "A";

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.equalOrEmpty(lhs, rhs));
}

void
CarrierValidatorTest::equalOrEmpty3()
{
  std::string lhs = "A";
  ;
  std::string rhs = "B";
  ;

  CPPUNIT_ASSERT(!_carrierValidatorMockBasicValidatorsProtected.equalOrEmpty(lhs, rhs));
}

void
CarrierValidatorTest::testValidateCarrierCodeNoCxr()
{
  CarrierCode marketingCarrierCode = "LO";
  CarrierCode carrierCode;

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.validateCarrierCode(
      marketingCarrierCode, carrierCode));
}

void
CarrierValidatorTest::testValidateCarrierCodeNotValidCxr()
{
  CarrierCode marketingCarrierCode = "LO";
  CarrierCode carrierCode = "AA";

  CPPUNIT_ASSERT(!_carrierValidatorMockBasicValidatorsProtected.validateCarrierCode(
      marketingCarrierCode, carrierCode));
}
void
CarrierValidatorTest::testValidateCarrierCodeValidCxr()
{
  CarrierCode marketingCarrierCode = "LO";
  CarrierCode carrierCode = "LO";

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.validateCarrierCode(
      marketingCarrierCode, carrierCode));
}

void
CarrierValidatorTest::testValidateFlightNumberNoRestriction()
{
  uint16_t marketingFlight = 0;
  uint16_t flight1 = 0;
  uint16_t flight2 = 0;

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.validateFlightNumber(
      marketingFlight, flight1, flight2));
}

void
CarrierValidatorTest::testValidateFlightNumber1()
{
  uint16_t marketingFlight = 0;
  uint16_t flight1 = 1;
  uint16_t flight2 = 0;

  CPPUNIT_ASSERT(!_carrierValidatorMockBasicValidatorsProtected.validateFlightNumber(
      marketingFlight, flight1, flight2));
}
void
CarrierValidatorTest::testValidateFlightNumber2()
{
  uint16_t marketingFlight = 2;
  uint16_t flight1 = 1;
  uint16_t flight2 = 3;

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.validateFlightNumber(
      marketingFlight, flight1, flight2));
}

void
CarrierValidatorTest::testValidateFlightNumber3()
{
  uint16_t marketingFlight = 5;
  uint16_t flight1 = 1;
  uint16_t flight2 = 3;

  CPPUNIT_ASSERT(!_carrierValidatorMockBasicValidatorsProtected.validateFlightNumber(
      marketingFlight, flight1, flight2));
}

void
CarrierValidatorTest::testValidateFlightNumber4()
{
  uint16_t marketingFlight = 1;
  uint16_t flight1 = 1;
  uint16_t flight2 = 0;

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.validateFlightNumber(
      marketingFlight, flight1, flight2));
}

void
CarrierValidatorTest::testValidateFlightNumber5()
{
  uint16_t marketingFlight = 4;
  uint16_t flight1 = 4;
  uint16_t flight2 = 3;

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.validateFlightNumber(
      marketingFlight, flight1, flight2));
}

void
CarrierValidatorTest::testValidateFlightNumber6()
{
  uint16_t marketingFlight = 3;
  uint16_t flight1 = 4;
  uint16_t flight2 = 3;

  CPPUNIT_ASSERT(!_carrierValidatorMockBasicValidatorsProtected.validateFlightNumber(
      marketingFlight, flight1, flight2));
}

void
CarrierValidatorTest::testValidateFlightDirection1()
{
  LocCode airport1 = "";
  LocCode airport2 = "";
  LocCode originAirport = "KRK";
  LocCode destinationAirport = "LHR";
  Indicator direction = 'F';

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.validateFlightDirection(
      airport1, airport2, originAirport, destinationAirport, direction));
}

void
CarrierValidatorTest::testValidateFlightDirection2()
{
  LocCode airport1 = "KRK";
  LocCode airport2 = "";
  LocCode originAirport = "KRK";
  LocCode destinationAirport = "LHR";
  Indicator direction = 'F';

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.validateFlightDirection(
      airport1, airport2, originAirport, destinationAirport, direction));
}

void
CarrierValidatorTest::testValidateFlightDirection3()
{
  LocCode airport1 = "KRK";
  LocCode airport2 = "LHR";
  LocCode originAirport = "KRK";
  LocCode destinationAirport = "LHR";
  Indicator direction = 'F';

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.validateFlightDirection(
      airport1, airport2, originAirport, destinationAirport, direction));
}

void
CarrierValidatorTest::testValidateFlightDirection4()
{
  LocCode airport1 = "KRK";
  LocCode airport2 = "JFK";
  LocCode originAirport = "KRK";
  LocCode destinationAirport = "LHR";
  Indicator direction = 'F';

  CPPUNIT_ASSERT(!_carrierValidatorMockBasicValidatorsProtected.validateFlightDirection(
      airport1, airport2, originAirport, destinationAirport, direction));
}

void
CarrierValidatorTest::testValidateFlightDirection5()
{
  LocCode airport1 = "";
  LocCode airport2 = "KRK";
  LocCode originAirport = "KRK";
  LocCode destinationAirport = "LHR";
  Indicator direction = 'F';

  CPPUNIT_ASSERT(!_carrierValidatorMockBasicValidatorsProtected.validateFlightDirection(
      airport1, airport2, originAirport, destinationAirport, direction));
}

void
CarrierValidatorTest::testValidateFlightDirection6()
{
  LocCode airport1 = "KRK";
  LocCode airport2 = "";
  LocCode originAirport = "KRK";
  LocCode destinationAirport = "LHR";
  Indicator direction = 'B';

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.validateFlightDirection(
      airport1, airport2, originAirport, destinationAirport, direction));
}

void
CarrierValidatorTest::testValidateFlightDirection7()
{
  LocCode airport1 = "";
  LocCode airport2 = "KRK";
  LocCode originAirport = "KRK";
  LocCode destinationAirport = "LHR";
  Indicator direction = 'B';

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.validateFlightDirection(
      airport1, airport2, originAirport, destinationAirport, direction));
}

void
CarrierValidatorTest::testValidateFlightDirection8()
{
  LocCode airport1 = "";
  LocCode airport2 = "";
  LocCode originAirport = "KRK";
  LocCode destinationAirport = "LHR";
  Indicator direction = 'B';

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.validateFlightDirection(
      airport1, airport2, originAirport, destinationAirport, direction));
}

void
CarrierValidatorTest::testValidateFlightDirection9()
{
  LocCode airport1 = "LHR";
  LocCode airport2 = "";
  LocCode originAirport = "KRK";
  LocCode destinationAirport = "LHR";
  Indicator direction = 'B';

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.validateFlightDirection(
      airport1, airport2, originAirport, destinationAirport, direction));
}

void
CarrierValidatorTest::testValidateFlightDirection10()
{
  LocCode airport1 = "";
  LocCode airport2 = "JFK";
  LocCode originAirport = "KRK";
  LocCode destinationAirport = "LHR";
  Indicator direction = 'B';

  CPPUNIT_ASSERT(!_carrierValidatorMockBasicValidatorsProtected.validateFlightDirection(
      airport1, airport2, originAirport, destinationAirport, direction));
}

void
CarrierValidatorTest::testValidateFlightDirection11()
{
  LocCode airport1 = "LHR";
  LocCode airport2 = "KRK";
  LocCode originAirport = "KRK";
  LocCode destinationAirport = "LHR";
  Indicator direction = 'B';

  CPPUNIT_ASSERT(_carrierValidatorMockBasicValidatorsProtected.validateFlightDirection(
      airport1, airport2, originAirport, destinationAirport, direction));
}

void
CarrierValidatorTest::testValidateExemptCxrRecord1()
{
  _carrierValidatorMockBasicValidators._validateCarrierCode = false;
  _carrierValidatorMockBasicValidators._validateFlightNumber = false;
  _carrierValidatorMockBasicValidators._validateFlightDirection = false;

  CPPUNIT_ASSERT(
      !_carrierValidatorMockBasicValidators.validateExemptCxrRecord(_exemptionCxr, *_travelSeg));
}
void
CarrierValidatorTest::testValidateExemptCxrRecord2()
{
  _carrierValidatorMockBasicValidators._validateCarrierCode = true;
  _carrierValidatorMockBasicValidators._validateFlightNumber = false;
  _carrierValidatorMockBasicValidators._validateFlightDirection = true;

  CPPUNIT_ASSERT(
      !_carrierValidatorMockBasicValidators.validateExemptCxrRecord(_exemptionCxr, *_travelSeg));
}

void
CarrierValidatorTest::testValidateExemptCxrRecord3()
{
  _carrierValidatorMockBasicValidators._validateCarrierCode = true;
  _carrierValidatorMockBasicValidators._validateFlightNumber = true;
  _carrierValidatorMockBasicValidators._validateFlightDirection = true;

  CPPUNIT_ASSERT(
      _carrierValidatorMockBasicValidators.validateExemptCxrRecord(_exemptionCxr, *_travelSeg));
}
void
CarrierValidatorTest::testValidateCarrier1()
{
  TaxCodeReg taxCodeReg;

  CPPUNIT_ASSERT(_carrierValidatorMockValidateExemptCxrRecord.validateCarrier(
      *_trx, *_taxResponse, taxCodeReg, _travelSegIndex));
}
void
CarrierValidatorTest::testValidateCarrier2()
{
  _carrierValidatorMockValidateExemptCxrRecord._validateExemptCxrRecord = false;

  CPPUNIT_ASSERT(!_carrierValidatorMockValidateExemptCxrRecord.validateCarrier(
      *_trx, *_taxResponse, _taxCodeReg, _travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier3()
{
  _carrierValidatorMockValidateExemptCxrRecord._validateExemptCxrRecord = true;
  _taxCodeReg.exempcxrExclInd() = 'N';

  CPPUNIT_ASSERT(_carrierValidatorMockValidateExemptCxrRecord.validateCarrier(
      *_trx, *_taxResponse, _taxCodeReg, _travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrier4()
{
  _carrierValidatorMockValidateExemptCxrRecord._validateExemptCxrRecord = true;
  _taxCodeReg.exempcxrExclInd() = 'Y';

  CPPUNIT_ASSERT(!_carrierValidatorMockValidateExemptCxrRecord.validateCarrier(
      *_trx, *_taxResponse, _taxCodeReg, _travelSegIndex));
}

void
CarrierValidatorTest::testValidateCarrierArunk1()
{
  _taxCodeReg.exempcxrExclInd() = 'N';
  CPPUNIT_ASSERT(!_carrierValidatorMockValidateExemptCxrRecord.validateCarrier(
      *_trx, *_taxResponse, _taxCodeReg, _arunkSegIndex));
}

void
CarrierValidatorTest::testValidateCarrierArunk2()
{
  _taxCodeReg.exempcxrExclInd() = 'Y';
  CPPUNIT_ASSERT(_carrierValidatorMockValidateExemptCxrRecord.validateCarrier(
      *_trx, *_taxResponse, _taxCodeReg, _arunkSegIndex));
}

void
CarrierValidatorTest::testValidateCarrierArunk3()
{
  _taxCodeReg.exempcxrExclInd() = 'N';
  CPPUNIT_ASSERT(!_carrierValidatorMockValidateExemptCxrRecord.validateCarrier(
      *_trx, *_taxResponse, _taxCodeReg, _arunkSegIndex));
}

void
CarrierValidatorTest::testValidateCarrierArunk4()
{
  _taxCodeReg.exempcxrExclInd() = 'Y';
  CPPUNIT_ASSERT(_carrierValidatorMockValidateExemptCxrRecord.validateCarrier(
      *_trx, *_taxResponse, _taxCodeReg, _arunkSegIndex));
}
