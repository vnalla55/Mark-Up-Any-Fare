#include <string>
#include <time.h>
#include <iostream>

#include "Taxes/LegacyTaxes/FareClassValidator.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DataModel/PaxType.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "Diagnostic/Diag804Collector.h"
#include "DataModel/PaxTypeFare.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
class FareClassValidatorTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(FareClassValidatorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testValidateFareType_positive);
  CPPUNIT_TEST(testValidateFareType_negative);
  CPPUNIT_TEST(testValidateFareClass_positive);
  CPPUNIT_TEST(testValidateFareClass_negative);
  CPPUNIT_TEST(testValidateFareClass_noFT_FC);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor()
  {
    try { FareClassValidator fareClassValidator; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  // Do ValidateFareType
  void testValidateFareType_positive()
  {
    TaxCodeReg taxCodeReg;
    FareType restrictionFareType;
    FareType fareType = "B";
    restrictionFareType = fareType;
    taxCodeReg.restrictionFareType().push_back(restrictionFareType);
    taxCodeReg.fareTypeExclInd() = 'N';

    uint16_t travelSegIndex = 0;

    FareClassValidator fareClassValidator;

    bool rc = fareClassValidator.validateFareClassRestriction(
        *_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testValidateFareType_negative()
  {
    TaxCodeReg taxCodeReg;
    FareType restrictionFareType;
    FareType fareType = "M";
    restrictionFareType = fareType;
    taxCodeReg.restrictionFareType().push_back(restrictionFareType);
    taxCodeReg.fareTypeExclInd() = 'N';

    uint16_t travelSegIndex = 0;

    FareClassValidator fareClassValidator;

    bool rc = fareClassValidator.validateFareClassRestriction(
        *_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  // Do ValidateFareClass
  void testValidateFareClass_positive()
  {
    TaxCodeReg taxCodeReg;
    FareClassCode restrictionFareClass;
    FareClassCode fareClass = "BAP1M";
    restrictionFareClass = fareClass;
    taxCodeReg.restrictionFareClass().push_back(restrictionFareClass);
    taxCodeReg.fareclassExclInd() = 'N';

    uint16_t travelSegIndex = 0;

    FareClassValidator fareClassValidator;

    bool rc = fareClassValidator.validateFareClassRestriction(
        *_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testValidateFareClass_negative()
  {
    TaxCodeReg taxCodeReg;
    FareClassCode restrictionFareClass;
    FareClassCode fareClass = "MAP1M";
    restrictionFareClass = fareClass;
    taxCodeReg.restrictionFareClass().push_back(restrictionFareClass);
    taxCodeReg.fareclassExclInd() = 'N';

    uint16_t travelSegIndex = 0;

    FareClassValidator fareClassValidator;

    bool rc = fareClassValidator.validateFareClassRestriction(
        *_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testValidateFareClass_noFT_FC()
  {
    TaxCodeReg taxCodeReg;
    uint16_t travelSegIndex = 0;
    FareClassValidator fareClassValidator;

    bool rc = fareClassValidator.validateFareClassRestriction(
        *_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
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

    _trx->travelSeg().push_back(travelSeg);

    FareMarket* faremkt = _memHandle.create<FareMarket>();
    faremkt->origin() = origin;
    faremkt->destination() = destination;
    faremkt->travelSeg().push_back(travelSeg);
    // Done setting up travel segment

    // Set up the fare path

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Set up the PaxTypeFare

    PaxType* paxType = _memHandle.create<PaxType>();
    PaxTypeCode paxTypeCode = "ADT";
    paxType->paxType() = paxTypeCode;

    PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();
    paxTypeFare->paxType() = paxType;

    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->_fareClass = "BAP1M";
    fare->setFareInfo(fareInfo);

    FareClassAppInfo* fareClassAppInfo = _memHandle.create<FareClassAppInfo>();
    fareClassAppInfo->_fareType = "B";
    paxTypeFare->fareClassAppInfo() = fareClassAppInfo;
    paxTypeFare->setFare(fare);

    // No need PaxTypeBucket as dealing with FU not the faremarket
    //    PaxTypeBucket* paxTypeCortege = new PaxTypeBucket();
    //    paxTypeCortege->requestedPaxType() = paxType;
    //    paxTypeCortege->paxTypeFare().push_back(paxTypeFare);
    //    faremkt->paxTypeCortege().push_back(*paxTypeCortege);

    PricingUnit* pu = _memHandle.create<PricingUnit>();
    farePath->pricingUnit().push_back(pu);

    FareUsage* fu = _memHandle.create<FareUsage>();
    fu->paxTypeFare() = paxTypeFare;
    farePath->pricingUnit()[0]->fareUsage().push_back(fu);

    //    farePath->pricingUnit()[0]->fareUsage().paxTypeFare().push_back(&paxTypeFare);
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
CPPUNIT_TEST_SUITE_REGISTRATION(FareClassValidatorTest);
}
