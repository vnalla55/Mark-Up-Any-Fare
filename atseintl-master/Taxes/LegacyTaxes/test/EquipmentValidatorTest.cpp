#include <string>
#include <time.h>
#include <iostream>

#include "Taxes/LegacyTaxes/EquipmentValidator.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag804Collector.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

using namespace std;
namespace tse
{
class EquipmentValidatorTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(EquipmentValidatorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testValidateEquipment_inclusive);
  CPPUNIT_TEST(testValidateEquipment_exclusive);
  CPPUNIT_TEST(testValidateEquipment_noExemptEQP);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor()
  {
    try { EquipmentValidator equipmentValidator; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  // Do ValidateEquipment
  void testValidateEquipment_inclusive()
  {
    TaxCodeReg taxCodeReg;
    std::string equipmentCode = "TRN";
    taxCodeReg.equipmentCode().push_back(equipmentCode);
    taxCodeReg.exempequipExclInd() = 'N';

    uint16_t travelSegIndex = 0;

    EquipmentValidator equipmentValidator;

    bool rc =
        equipmentValidator.validateEquipment(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testValidateEquipment_exclusive()
  {
    TaxCodeReg taxCodeReg;
    std::string equipmentCode = "TRN";
    taxCodeReg.equipmentCode().push_back(equipmentCode);
    taxCodeReg.exempequipExclInd() = 'Y';

    uint16_t travelSegIndex = 0;

    EquipmentValidator equipmentValidator;

    bool rc =
        equipmentValidator.validateEquipment(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testValidateEquipment_noExemptEQP()
  {
    TaxCodeReg taxCodeReg;
    uint16_t travelSegIndex = 0;
    EquipmentValidator equipmentValidator;

    bool rc =
        equipmentValidator.validateEquipment(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void setUp()
  {
    // Set up the PricingTrx
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
    origin->loc() = string("NYC");
    origin->nation() = string("US");

    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = string("LIM");
    destination->nation() = string("PE");

    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->origin() = origin;
    travelSeg->destination() = destination;
    travelSeg->equipmentType() = "TRN";

    // Done setting up travel segment

    // Set up the fare path

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;
    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    diagroot->activate();
    Diag804Collector* diag804 = _memHandle.insert(new Diag804Collector(*diagroot));

    _taxResponse->diagCollector() = diag804;
  }

  void tearDown() { _memHandle.clear(); }

private:
  tse::PricingTrx* _trx;
  tse::TaxResponse* _taxResponse;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(EquipmentValidatorTest);
}
