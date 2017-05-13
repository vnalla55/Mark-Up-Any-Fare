#include "Taxes/LegacyTaxes/CarrierValidatorOC.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/Loc.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/PricingTrx.h"
#include "ServiceFees/OCFees.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class CarrierValidatorOCMockBasicValidators : public CarrierValidatorOC
{
public:
  virtual bool
  validateCarrierCode(const CarrierCode& marketingCarrierCode, const CarrierCode& carrierCode)
  {
    return _validateCarrierCode;
  }

  virtual bool validateExemptCxrRecord(const TaxExemptionCarrier& taxExemptionCarrier)
  {
    return CarrierValidatorOC::validateExemptCxrRecord(taxExemptionCarrier);
  }

  bool _validateCarrierCode;
};

class CarrierValidatorOCMockValidateExemptCxrRecord : public CarrierValidatorOC
{
public:
  virtual bool validateCarrier(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t travelSegIndex)

  {
    return CarrierValidatorOC::validateCarrier(trx, taxResponse, taxCodeReg, travelSegIndex);
  }

  virtual bool
  validateExemptCxrRecord(const TaxExemptionCarrier& taxExemptionCarrier, const AirSeg* airSeg)
  {
    return _validateExemptCxrRecord;
  }

  void setOcFees(OCFees* ocFees) { CarrierValidatorOC::setOcFees(ocFees); }

  bool _validateExemptCxrRecord;
};

class CarrierValidatorOCTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(CarrierValidatorOCTest);

  CPPUNIT_TEST(testValidateCarrier1);
  CPPUNIT_TEST(testValidateCarrier2);
  CPPUNIT_TEST(testValidateCarrier3);
  CPPUNIT_TEST(testValidateCarrier4);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void testValidateCarrier1();
  void testValidateCarrier2();
  void testValidateCarrier3();
  void testValidateCarrier4();

  void setUp();
  void tearDown();

private:
  PricingTrx* _trx;
  TaxResponse* _taxResponse;
  TaxCodeReg _taxCodeReg;
  TaxExemptionCarrier _exemptionCxr;
  uint16_t _travelSegIndex;
  AirSeg* _travelSeg;
  CarrierValidatorOC _carrierValidator;

  CarrierValidatorOCMockBasicValidators _carrierValidatorMockBasicValidators;
  CarrierValidatorOCMockValidateExemptCxrRecord _carrierValidatorMockValidateExemptCxrRecord;
  OCFees _ocFees;
};
}

using namespace tse;
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(CarrierValidatorOCTest);

void
CarrierValidatorOCTest::setUp()
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

  // Done setting up travel segment

  // Set up the fare path

  Itin* itin = _memHandle.create<Itin>();
  itin->travelSeg().push_back(travelSeg);

  FarePath* farePath = _memHandle.create<FarePath>();
  farePath->itin() = itin;

  // Done setting up the farepath

  _taxResponse = _memHandle.create<TaxResponse>();
  _taxResponse->farePath() = farePath;
  _taxCodeReg.exemptionCxr().push_back(_exemptionCxr);
  _travelSegIndex = 0;
  _travelSeg = _memHandle.create<AirSeg>();

  _ocFees.carrierCode() = "BA";
  _carrierValidatorMockValidateExemptCxrRecord.setOcFees(&_ocFees);
}

void
CarrierValidatorOCTest::tearDown()
{
  _memHandle.clear();
}

void
CarrierValidatorOCTest::testValidateCarrier1()
{
  TaxCodeReg taxCodeReg;

  CPPUNIT_ASSERT(_carrierValidatorMockValidateExemptCxrRecord.validateCarrier(
      *_trx, *_taxResponse, taxCodeReg, _travelSegIndex));
}
void
CarrierValidatorOCTest::testValidateCarrier2()
{
  _taxCodeReg.exempcxrExclInd() = 'Y';
  _carrierValidatorMockValidateExemptCxrRecord._validateExemptCxrRecord = false;

  CPPUNIT_ASSERT(!_carrierValidatorMockValidateExemptCxrRecord.validateCarrier(
      *_trx, *_taxResponse, _taxCodeReg, _travelSegIndex));
}

void
CarrierValidatorOCTest::testValidateCarrier3()
{
  _carrierValidatorMockValidateExemptCxrRecord._validateExemptCxrRecord = true;
  _taxCodeReg.exempcxrExclInd() = 'N';

  CPPUNIT_ASSERT(_carrierValidatorMockValidateExemptCxrRecord.validateCarrier(
      *_trx, *_taxResponse, _taxCodeReg, _travelSegIndex));
}

void
CarrierValidatorOCTest::testValidateCarrier4()
{
  _carrierValidatorMockValidateExemptCxrRecord._validateExemptCxrRecord = true;
  _taxCodeReg.exempcxrExclInd() = 'Y';

  CPPUNIT_ASSERT(!_carrierValidatorMockValidateExemptCxrRecord.validateCarrier(
      *_trx, *_taxResponse, _taxCodeReg, _travelSegIndex));
}
