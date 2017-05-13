#include "Common/DateTime.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Response.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxRestrictionTransit.h"
#include "Diagnostic/Diag804Collector.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxSW1_new.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class TaxSW1_newTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(TaxSW1_newTest);
  CPPUNIT_TEST(testValidateTransitNoTransit);
  CPPUNIT_TEST(testValidateTransitForcedTransit);
  CPPUNIT_TEST(testValidateTransitForcedStopOver);
  CPPUNIT_TEST(testValidateTransitForcedTransitWithRestrictionTransit);
  CPPUNIT_TEST(testValidateTransitForcedStopOverWithRestrictionTransit);
  CPPUNIT_TEST(testValidateLocRestrictions);
  CPPUNIT_TEST(testValidateTripTypes);
  CPPUNIT_TEST(testValidateTransitIfNoHiddenStop1);
  CPPUNIT_TEST(testValidateTransitIfNoHiddenStop2);
  CPPUNIT_TEST(testValidateTransitIfNoHiddenStop3);

  CPPUNIT_TEST_SUITE_END();

public:
  void testValidateTransitNoTransit();
  void testValidateTransitForcedTransit();
  void testValidateTransitForcedStopOver();
  void testValidateTransitForcedTransitWithRestrictionTransit();
  void testValidateTransitForcedStopOverWithRestrictionTransit();
  void testValidateLocRestrictions();
  void testValidateTripTypes();
  void testValidateTransitIfNoHiddenStop1();
  void testValidateTransitIfNoHiddenStop2();
  void testValidateTransitIfNoHiddenStop3();

  void setUp();
  void tearDown();

private:
  PricingTrx* _trx;
  Agent* _agent;
  PricingRequest* _request;
  Loc* _origin;
  Loc* _destination;
  AirSeg* _travelSeg;
  Itin* _itin;
  FarePath* _farePath;
  Loc* _origin2;
  Loc* _destination2;
  AirSeg* _travelSeg2;
  TaxResponse* _taxResponse;
  Diagnostic* _diagroot;
  Diag804Collector* _diag;
  TaxCodeReg* _taxCodeReg;
  TaxRestrictionTransit* _taxRestrictionTransit;
  TransitValidator* _transitValidator;
  TaxSW1_new* _tax;

  TravelSeg* _travelSegFrom;
  TravelSeg* _travelSegTo;
  TestMemHandle _memHandle;
};
}

using namespace tse;
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(TaxSW1_newTest);

void
TaxSW1_newTest::setUp()
{
  _memHandle.create<TestConfigInitializer>();
  _trx = new PricingTrx();
  _agent = new Agent();
  _request = new PricingRequest();
  _origin = new Loc();
  _destination = new Loc();
  _travelSeg = new AirSeg();
  _itin = new Itin();
  _farePath = new FarePath();
  _origin2 = new Loc();
  _destination2 = new Loc();
  _travelSeg2 = new AirSeg();
  _taxResponse = new TaxResponse();
  _diagroot = new Diagnostic();
  _diag = new Diag804Collector();
  _taxCodeReg = new TaxCodeReg();
  _taxRestrictionTransit = new TaxRestrictionTransit();
  _transitValidator = new TransitValidator();
  _tax = new TaxSW1_new();

  _agent->currencyCodeAgent() = "USD";
  _trx->setRequest(_request);
  _trx->getRequest()->ticketingAgent() = _agent;
  _trx->getRequest()->ticketingDT() = DateTime(2004, 7, 14, 11, 0, 0);
  _origin->loc() = std::string("DFW");
  _origin->nation() = std::string("US");
  _destination->loc() = std::string("MIA");
  _destination->nation() = std::string("US");
  _trx->getRequest()->ticketingAgent()->agentLocation() = _destination;
  _travelSeg->geoTravelType() = GeoTravelType::Domestic;
  _travelSeg->origin() = _origin;
  _travelSeg->destination() = _destination;
  _travelSeg->departureDT() = DateTime(2004, 7, 14, 4, 0, 0);
  _travelSeg->arrivalDT() = DateTime(2004, 7, 14, 5, 0, 0);
  _itin->tripCharacteristics().set(Itin::OneWay, true);
  _trx->itin().push_back(_itin);
  _farePath->itin() = _itin;
  _farePath->itin()->travelSeg().push_back(_travelSeg);
  _origin2->loc() = std::string("MIA");
  _origin2->nation() = std::string("US");
  _destination2->loc() = std::string("LHR");
  _destination2->nation() = std::string("GB");
  _travelSeg2->origin() = _origin2;
  _travelSeg2->destination() = _destination2;
  _travelSeg2->departureDT() = DateTime(2004, 7, 14, 9, 0, 0);
  _travelSeg2->arrivalDT() = DateTime(2004, 7, 14, 11, 0, 0);
  _farePath->itin()->travelSeg().push_back(_travelSeg2);
  _itin->farePath().push_back(_farePath);
  _diagroot = new Diagnostic(LegacyTaxDiagnostic24);
  _diag = new Diag804Collector(*_diagroot);
  _taxResponse->diagCollector() = _diag;
  _taxResponse->farePath() = _farePath;
  _taxResponse->paxTypeCode() = std::string("ADT");
  _taxCodeReg->expireDate() = DateTime(2004, 7, 14, 11, 0, 0);
  _taxCodeReg->loc1Type() = tse::LocType('N');
  _taxCodeReg->loc2Type() = tse::LocType('N');
  _taxCodeReg->loc1() = tse::LocCode("US");
  _taxCodeReg->loc1ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
  _taxCodeReg->loc2() = tse::LocCode("GB");
  _taxCodeReg->loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
  _taxCodeReg->taxCode() = std::string("YC");
  _taxCodeReg->effDate() = (DateTime)1;
  _taxCodeReg->discDate() = (DateTime)999;
  _taxCodeReg->firstTvlDate() = (DateTime)5;
  _taxCodeReg->lastTvlDate() = (DateTime)20;
  _taxCodeReg->nation() = std::string("MY");
  _taxCodeReg->taxfullFareInd() = tse::Indicator('N');
  _taxCodeReg->taxequivAmtInd() = tse::Indicator('N');
  _taxCodeReg->taxexcessbagInd() = tse::Indicator('N');
  _taxCodeReg->tvlDateasoriginInd() = tse::Indicator('N');
  _taxCodeReg->displayonlyInd() = tse::Indicator('N');
  _taxCodeReg->feeInd() = tse::Indicator('N');
  _taxCodeReg->interlinableTaxInd() = tse::Indicator('N');
  _taxCodeReg->showseparateInd() = tse::Indicator('N');
  _taxCodeReg->posExclInd() = tse::Indicator('N');
  _taxCodeReg->posLocType() = tse::LocType(' ');
  _taxCodeReg->posLoc() = tse::LocCode("");
  _taxCodeReg->poiExclInd() = tse::Indicator('N');
  _taxCodeReg->poiLocType() = tse::LocType(' ');
  _taxCodeReg->poiLoc() = tse::LocCode("");
  _taxCodeReg->sellCurExclInd() = tse::Indicator('N');
  _taxCodeReg->sellCur() = tse::CurrencyCode("");
  _taxCodeReg->occurrence() = tse::Indicator(' ');
  _taxCodeReg->freeTktexempt() = tse::Indicator(' ');
  _taxCodeReg->idTvlexempt() = tse::Indicator(' ');
  _taxCodeReg->taxCur() = tse::CurrencyCode("");
  _taxCodeReg->fareclassExclInd() = tse::Indicator(' ');
  _taxCodeReg->tktdsgExclInd() = tse::Indicator(' ');
  _taxCodeReg->valcxrExclInd() = tse::Indicator(' ');
  _taxCodeReg->exempequipExclInd() = tse::Indicator(' ');
  _taxCodeReg->psgrExclInd() = tse::Indicator(' ');
  _taxCodeReg->fareTypeExclInd() = tse::Indicator(' ');
  _taxCodeReg->originLocExclInd() = tse::Indicator('N');
  _taxCodeReg->originLocType() = tse::LocType(' ');
  _taxCodeReg->originLoc() = tse::LocCode("");
  _taxCodeReg->originLocExclInd() = tse::Indicator('N');
  _taxCodeReg->originLocType() = tse::LocType(' ');
  _taxCodeReg->originLoc() = tse::LocCode("");
  _taxCodeReg->loc1Appl() = tse::Indicator(' ');
  _taxCodeReg->loc2Appl() = tse::Indicator(' ');
  _taxCodeReg->tripType() = tse::Indicator(' ');
  _taxCodeReg->travelType() = tse::Indicator('D');
  _taxCodeReg->itineraryType() = tse::Indicator('O');
  _taxCodeReg->formOfPayment() = tse::Indicator(' ');
  _taxCodeReg->taxOnTaxExcl() = tse::Indicator(' ');
  _taxRestrictionTransit->transitTaxonly() = tse::Indicator('N');
  _taxRestrictionTransit->transitHours() = 48;
  _taxRestrictionTransit->transitMinutes() = 0;
  _taxRestrictionTransit->sameFlight() = NO;
  _taxRestrictionTransit->sameDayInd() = NO;
  _taxRestrictionTransit->transitDomDom() = NO;
  _taxRestrictionTransit->transitDomIntl() = NO;
  _taxRestrictionTransit->transitIntlDom() = NO;
  _taxRestrictionTransit->transitIntlIntl() = NO;
  _taxRestrictionTransit->transitSurfDom() = NO;
  _taxRestrictionTransit->transitSurfIntl() = NO;
  _taxRestrictionTransit->transitOfflineCxr() = NO;

  _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

  _travelSegFrom = new AirSeg();
  _travelSegTo = new AirSeg();
}

void
TaxSW1_newTest::tearDown()
{
  delete _agent;
  delete _request;
  delete _origin;
  delete _destination;
  delete _travelSeg;
  delete _itin;
  delete _farePath;
  delete _origin2;
  delete _destination2;
  delete _travelSeg2;
  delete _taxResponse;
  delete _diagroot;
  delete _diag;
  delete _taxCodeReg;
  delete _taxRestrictionTransit;
  delete _transitValidator;
  delete _trx;
  delete _tax;
  delete _travelSegFrom;
  delete _travelSegTo;
  _memHandle.clear();
}

void
TaxSW1_newTest::testValidateTransitNoTransit()
{
  CPPUNIT_ASSERT(!_tax->validateTransit(*_trx, *_taxResponse, *_taxCodeReg, 1));
}

void
TaxSW1_newTest::testValidateTransitForcedTransit()
{

  _travelSeg->forcedConx() = tse::Indicator('Y');
  _travelSeg->forcedStopOver() = tse::Indicator('N');
  CPPUNIT_ASSERT(!_tax->validateTransit(*_trx, *_taxResponse, *_taxCodeReg, 1));
}

void
TaxSW1_newTest::testValidateTransitForcedStopOver()
{
  _travelSeg->forcedStopOver() = tse::Indicator('Y');
  _travelSeg->forcedConx() = tse::Indicator('N');
  CPPUNIT_ASSERT(_tax->validateTransit(*_trx, *_taxResponse, *_taxCodeReg, 1));
}

void
TaxSW1_newTest::testValidateTransitForcedStopOverWithRestrictionTransit()
{
  _taxCodeReg->restrictionTransit().front().transitTaxonly() = tse::Indicator('Y');
  _travelSeg->forcedConx() = tse::Indicator('Y');
  _travelSeg->forcedStopOver() = tse::Indicator('N');
  CPPUNIT_ASSERT(_tax->validateTransit(*_trx, *_taxResponse, *_taxCodeReg, 1));
}

void
TaxSW1_newTest::testValidateTransitForcedTransitWithRestrictionTransit()
{
  _taxCodeReg->restrictionTransit().front().transitTaxonly() = tse::Indicator('Y');
  _travelSeg->forcedStopOver() = tse::Indicator('Y');
  _travelSeg->forcedConx() = tse::Indicator('N');
  CPPUNIT_ASSERT(!_tax->validateTransit(*_trx, *_taxResponse, *_taxCodeReg, 1));
}

void
TaxSW1_newTest::testValidateLocRestrictions()
{
  uint16_t startIndex = 1;
  uint16_t endIndex = 2;
  CPPUNIT_ASSERT(
      _tax->validateLocRestrictions(*_trx, *_taxResponse, *_taxCodeReg, startIndex, endIndex));
}

void
TaxSW1_newTest::testValidateTripTypes()
{

  uint16_t startIndex = 1;
  uint16_t endIndex = 2;
  CPPUNIT_ASSERT(_tax->validateTripTypes(*_trx, *_taxResponse, *_taxCodeReg, startIndex, endIndex));
}

void
TaxSW1_newTest::testValidateTransitIfNoHiddenStop1()
{
  uint16_t startIndex = 1;
  CPPUNIT_ASSERT(
      !_tax->validateTransitIfNoHiddenStop(*_trx, *_taxResponse, *_taxCodeReg, startIndex));
}
void
TaxSW1_newTest::testValidateTransitIfNoHiddenStop2()
{
  uint16_t startIndex = 1;
  _travelSeg->forcedStopOver() = tse::Indicator('Y');
  _travelSeg->forcedConx() = tse::Indicator('N');

  CPPUNIT_ASSERT(
      _tax->validateTransitIfNoHiddenStop(*_trx, *_taxResponse, *_taxCodeReg, startIndex));
}
void
TaxSW1_newTest::testValidateTransitIfNoHiddenStop3()
{
  uint16_t startIndex = 0;
  _travelSeg->forcedStopOver() = tse::Indicator('Y');
  _travelSeg->forcedConx() = tse::Indicator('N');
  CPPUNIT_ASSERT(
      _tax->validateTransitIfNoHiddenStop(*_trx, *_taxResponse, *_taxCodeReg, startIndex));
}
