#include <string>
#include <time.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxRestrictionTransit.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "DataModel/PricingRequest.h"
#include "DBAccess/Loc.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/PaxType.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxType.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "DataModel/Response.h"
#include "DataModel/Itin.h"
#include "DataModel/Agent.h"
#include "Common/DateTime.h"
#include "Diagnostic/Diag804Collector.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "test/include/MockTseServer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"

using namespace std;

namespace tse
{

class TransitValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TransitValidatorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTransitValidatorCase0);
  CPPUNIT_TEST(testTransitValidatorCase1);
  CPPUNIT_TEST(testTransitValidatorCase2);
  CPPUNIT_TEST(testTransitValidatorCase3);
  CPPUNIT_TEST(testTransitValidatorCase4);
  CPPUNIT_TEST(testTransitValidatorCase5);
  CPPUNIT_TEST(testTransitValidatorCase6);
  CPPUNIT_TEST(testTransitValidatorCase7);
  CPPUNIT_TEST(testTransitValidatorCase8);
  CPPUNIT_TEST(testTransitValidatorCase9);
  CPPUNIT_TEST(testTransitValidatorCase10);
  CPPUNIT_TEST(testTransitValidatorCase11);
  CPPUNIT_TEST(testTransitValidatorCase12);
  CPPUNIT_TEST(testTransitValidatorCase13);
  CPPUNIT_TEST(testTransitValidatorCase14);
  CPPUNIT_TEST(testTransitValidatorCase15);
  CPPUNIT_TEST(testTransitValidatorCase16);
  CPPUNIT_TEST(testTransitValidatorCase17);
  CPPUNIT_TEST(testTransitValidatorCase18);
  CPPUNIT_TEST(testTransitValidatorCase19);
  CPPUNIT_TEST(testTransitValidatorCase20);
  CPPUNIT_TEST(testTransitValidatorCase21);
  CPPUNIT_TEST(testTransitValidatorCase22);
  CPPUNIT_TEST(testTransitValidatorCase23);
  CPPUNIT_TEST(testTransitValidatorCase24);
  CPPUNIT_TEST(testTransitValidatorCase25);
  CPPUNIT_TEST(testTransitValidatorCase26);
  CPPUNIT_TEST(testTransitValidatorCase27);
  CPPUNIT_TEST(testTransitValidatorCase28);
  CPPUNIT_TEST(testTransitValidatorCase29);
  CPPUNIT_TEST(testTransitValidatorCase30);
  CPPUNIT_TEST(testTransitValidatorCase31);
  CPPUNIT_TEST(testTransitValidatorCase32);
  CPPUNIT_TEST(testTransitValidatorCase33);
  CPPUNIT_TEST(testTransitValidatorCase34);
  CPPUNIT_TEST(testTransitValidatorCase35);
  CPPUNIT_TEST(testTransitValidatorCase36);
  CPPUNIT_TEST(testTransitValidatorCase37);
  CPPUNIT_TEST(testTransitValidatorCase38);
  CPPUNIT_TEST(testTransitValidatorCase39);
  CPPUNIT_TEST(testTransitValidatorCase40);
  CPPUNIT_TEST(testTransitValidatorCase41);
  CPPUNIT_TEST(testTransitValidatorCase42);
  CPPUNIT_TEST(testTransitValidatorCase43);
  CPPUNIT_TEST(testTransitValidatorCase44);
  CPPUNIT_TEST(testTransitValidatorCase45);
  CPPUNIT_TEST(testTransitValidatorCase46);
  CPPUNIT_TEST(testTransitValidatorCase47);
  CPPUNIT_TEST(testTransitValidatorCase48);
  CPPUNIT_TEST(testTransitValidatorCase49);
  CPPUNIT_TEST(testTransitValidatorCase50);
  CPPUNIT_TEST(testTransitValidatorCase51);
  CPPUNIT_TEST(testTransitValidatorCase52);
  CPPUNIT_TEST(testTransitValidatorCase53);
  CPPUNIT_TEST(testTransitValidatorCase54);
  CPPUNIT_TEST(testTransitValidatorCase55);
  CPPUNIT_TEST(testTransitValidatorCase56);
  CPPUNIT_TEST(testTransitValidatorCase57);
  CPPUNIT_TEST(testTransitValidatorCase58);
  CPPUNIT_TEST(testTransitValidatorCase59);
  CPPUNIT_TEST(testTransitValidatorCase60);
  CPPUNIT_TEST(testTransitValidatorCase61);
  CPPUNIT_TEST(testTransitValidatorCase62);
  CPPUNIT_TEST(testTransitValidatorCase63);
  CPPUNIT_TEST(testTransitValidatorCase64);
  CPPUNIT_TEST(testTransitValidatorCase65);
  CPPUNIT_TEST(testTransitValidatorCase66);
  CPPUNIT_TEST(testTransitValidatorCase67);
  CPPUNIT_TEST(testTransitValidatorCase68);
  CPPUNIT_TEST(testTransitValidatorCase69);
  CPPUNIT_TEST(testTransitValidatorCase70);
  CPPUNIT_TEST(testTransitValidatorCase71);
  CPPUNIT_TEST(testTransitValidatorCase72);
  CPPUNIT_TEST(testTransitValidatorCase73);
  CPPUNIT_TEST(testTransitValidatorCase74);
  CPPUNIT_TEST(testTransitValidatorCase75);
  CPPUNIT_TEST(testTransitValidatorCase76);
  CPPUNIT_TEST(testTransitValidatorCase77);
  CPPUNIT_TEST(testTransitValidatorCase78);
  CPPUNIT_TEST(testTransitValidatorCase79);
  CPPUNIT_TEST(testTransitValidatorCase80);
  CPPUNIT_TEST_SUITE_END();

public:
  TransitValidatorTest() {}

  void testConstructor()
  {
    try { TransitValidatorTest transitValidatorTest; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _agent = _memHandle.create<Agent>();
    _request = _memHandle.create<PricingRequest>();
    _origin = _memHandle.create<Loc>();
    _destination = _memHandle.create<Loc>();
    _travelSeg = _memHandle.create<AirSeg>();
    _itin = _memHandle.create<Itin>();
    _farePath = _memHandle.create<FarePath>();
    _origin2 = _memHandle.create<Loc>();
    _destination2 = _memHandle.create<Loc>();
    _travelSeg2 = _memHandle.create<AirSeg>();
    _taxResponse = _memHandle.create<TaxResponse>();
    _diagroot = _memHandle.create<Diagnostic>();
    _diag = _memHandle.create<Diag804Collector>();
    _taxCodeReg = _memHandle.create<TaxCodeReg>();
    _taxRestrictionTransit = _memHandle.create<TaxRestrictionTransit>();
    _transitValidator = _memHandle.create<TransitValidator>();

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
    _diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    _diag = _memHandle.insert(new Diag804Collector(*_diagroot));
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
    _taxRestrictionTransit->transitHours() = 0;
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
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testTransitValidatorCase0()
  {
    TransitValidator transitValidator;

    bool mirrorImage = false;
    uint32_t travelSegIndex = 1;

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase1()
  {
    TransitValidator transitValidator;

    bool mirrorImage = true;
    uint32_t travelSegIndex = 1;

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase2()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase3()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool rc =
        transitValidator.validateTransitTime(*_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase4()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitDomDom() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase5()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitDomIntl() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase6()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitIntlDom() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase7()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitIntlIntl() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase8()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitSurfDom() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase9()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitOfflineCxr() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase10()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitOfflineCxr() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxRestrictionTransit->viaLoc() = std::string("US");
    _taxRestrictionTransit->viaLocType() = tse::LocType('N');
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase11()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitHours() = -1;
    _taxRestrictionTransit->transitMinutes() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase12()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitMinutes() = -1;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase13()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = false;
    _taxRestrictionTransit->transitMinutes() = -1;
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    _taxCodeReg->loc2Appl() = tse::Indicator(LocRestrictionValidator::TAX_DEPLANEMENT);
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase14()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = false;
    _taxCodeReg->restrictionTransit().clear();

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase15()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = false;
    _taxRestrictionTransit->sameFlight() = YES;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    _travelSeg2->marketingFlightNumber() = 1;
    _travelSeg->marketingFlightNumber() = 2;

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase16()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = false;
    _travelSeg2->segmentType() = Open;

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase17()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = false;
    _taxRestrictionTransit->sameDayInd() = YES;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    _travelSeg2->segmentType() = Open;

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase18()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = false;
    _taxRestrictionTransit->transitOfflineCxr() = YES;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase19()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitDomDom() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase20()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitDomIntl() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase21()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitIntlDom() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase22()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitIntlIntl() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase23()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitSurfDom() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase24()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitOfflineCxr() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase25()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitOfflineCxr() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxRestrictionTransit->viaLoc() = std::string("US");
    _taxRestrictionTransit->viaLocType() = tse::LocType('N');
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase26()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitHours() = -1;
    _taxRestrictionTransit->transitMinutes() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase27()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitMinutes() = -1;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase28()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitMinutes() = -1;
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    _taxCodeReg->loc1Appl() = tse::Indicator(LocRestrictionValidator::TAX_DEPLANEMENT);
    _taxCodeReg->loc2Appl() = tse::Indicator(LocRestrictionValidator::TAX_DEPLANEMENT);
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase29()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxCodeReg->restrictionTransit().clear();

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase30()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->sameFlight() = YES;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase31()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _travelSeg2->segmentType() = Open;

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase32()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->sameDayInd() = YES;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase33()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitOfflineCxr() = YES;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase34()
  {
    TransitValidator transitValidator;

    bool mirrorImage = true;
    uint32_t travelSegIndex = 0;

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase35()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase36()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool rc =
        transitValidator.validateTransitTime(*_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase37()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitDomDom() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase38()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitDomIntl() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase39()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitIntlDom() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase40()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitIntlIntl() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase41()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitSurfDom() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase42()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitOfflineCxr() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase43()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitOfflineCxr() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxRestrictionTransit->viaLoc() = std::string("US");
    _taxRestrictionTransit->viaLocType() = tse::LocType('N');
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase44()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitHours() = -1;
    _taxRestrictionTransit->transitMinutes() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase45()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = true;
    _taxRestrictionTransit->transitMinutes() = -1;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase46()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = false;
    _taxRestrictionTransit->transitMinutes() = -1;
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    _taxCodeReg->loc2Appl() = tse::Indicator(LocRestrictionValidator::TAX_DEPLANEMENT);
    _taxCodeReg->loc1Appl() = tse::Indicator(LocRestrictionValidator::TAX_DEPLANEMENT);
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase47()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = false;
    _taxCodeReg->restrictionTransit().clear();

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase48()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = false;
    _taxRestrictionTransit->sameFlight() = YES;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase49()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = false;
    _travelSeg2->segmentType() = Open;

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase50()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = false;
    _taxRestrictionTransit->sameDayInd() = YES;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase51()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    bool mirrorImage = false;
    _taxRestrictionTransit->transitOfflineCxr() = YES;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase52()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitDomDom() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase53()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitDomIntl() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase54()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitIntlDom() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase55()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitIntlIntl() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase56()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitSurfDom() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase57()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitOfflineCxr() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase58()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxRestrictionTransit->transitTaxonly() = tse::Indicator('Y');
    _taxRestrictionTransit->transitOfflineCxr() = tse::Indicator('Y');
    _taxRestrictionTransit->transitHours() = 30;
    _taxRestrictionTransit->viaLoc() = std::string("US");
    _taxRestrictionTransit->viaLocType() = tse::LocType('N');
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase59()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxRestrictionTransit->transitHours() = -1;
    _taxRestrictionTransit->transitMinutes() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase60()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxRestrictionTransit->transitMinutes() = -1;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase61()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxRestrictionTransit->transitMinutes() = -1;
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    _taxCodeReg->loc1Appl() = tse::Indicator(LocRestrictionValidator::TAX_DEPLANEMENT);
    _taxCodeReg->loc2Appl() = tse::Indicator(LocRestrictionValidator::TAX_DEPLANEMENT);
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase62()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxCodeReg->restrictionTransit().clear();

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase63()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxRestrictionTransit->sameFlight() = YES;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase64()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _travelSeg2->segmentType() = Open;

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase65()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxRestrictionTransit->sameDayInd() = YES;
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase66()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxRestrictionTransit->transitOfflineCxr() = YES;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase67()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxCodeReg->loc1Appl() = tse::Indicator(LocRestrictionValidator::TAX_DEPLANEMENT);
    _taxCodeReg->loc2Appl() = tse::Indicator(LocRestrictionValidator::TAX_DEPLANEMENT);
    bool rc =
        transitValidator.validateTransitTime(*_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase68()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    bool mirrorImage = false;
    _taxRestrictionTransit->transitOfflineCxr() = YES;
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    _taxCodeReg->loc1Appl() = tse::Indicator(LocRestrictionValidator::TAX_DEPLANEMENT);
    _taxCodeReg->loc2Appl() = tse::Indicator(LocRestrictionValidator::TAX_DEPLANEMENT);

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase69()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _taxRestrictionTransit->transitMinutes() = -1;
    _taxRestrictionTransit->transitHours() = 30;
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    _taxCodeReg->loc2Appl() = tse::Indicator(LocRestrictionValidator::TAX_DEPLANEMENT);
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    _travelSeg->segmentType() = Open;
    _travelSeg2->segmentType() = Open;

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase70()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 0;
    _travelSeg->segmentType() = Open;
    _travelSeg2->segmentType() = Open;

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase71()
  {
    TransitValidator transitValidator;

    bool mirrorImage = false;
    uint32_t travelSegIndex = 1;
    _travelSeg->forcedStopOver() = 'Y';

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase72()
  {
    TransitValidator transitValidator;

    bool mirrorImage = false;
    uint32_t travelSegIndex = 1;
    _travelSeg->forcedConx() = 'Y';

    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase73()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitHours() = 3;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    bool rc =
        transitValidator.validateTransitTime(*_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase74()
  {
    TransitValidator transitValidator;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitHours() = 3;
    _taxRestrictionTransit->sameDayInd() = YES;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    bool rc =
        transitValidator.validateTransitTime(*_trx, *_taxResponse, *_taxCodeReg, travelSegIndex);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase75()
  {
    TransitValidator transitValidator;
    bool mirrorImage = false;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->sameDayInd() = YES;
    _travelSeg2->segmentType() = Open;
    _travelSeg->segmentType() = Open;
    _taxRestrictionTransit->transitMinutes() = 3;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase76()
  {
    TransitValidator transitValidator;
    bool mirrorImage = false;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitHours() = 24;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase77()
  {
    TransitValidator transitValidator;
    bool mirrorImage = false;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitHours() = 24;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    _travelSeg->equipmentType() = TRAIN;
    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase78()
  {
    TransitValidator transitValidator;
    bool mirrorImage = false;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitHours() = 24;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    _travelSeg->equipmentType() = TRAIN;
    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, false);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTransitValidatorCase79()
  {
    TransitValidator transitValidator;
    bool mirrorImage = false;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitHours() = 24;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    _travelSeg->equipmentType() = TRS;
    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, true);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTransitValidatorCase80()
  {
    TransitValidator transitValidator;
    bool mirrorImage = false;
    uint32_t travelSegIndex = 1;
    _taxRestrictionTransit->transitHours() = 24;
    _taxCodeReg->restrictionTransit().clear();
    _taxCodeReg->restrictionTransit().push_back(*_taxRestrictionTransit);
    _travelSeg->equipmentType() = TRS;
    bool rc = transitValidator.validateTransitRestriction(
        *_trx, *_taxResponse, *_taxCodeReg, travelSegIndex, mirrorImage, false);
    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

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
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TransitValidatorTest);
}
