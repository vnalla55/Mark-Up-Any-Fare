#include <string>
#include <time.h>
#include <iostream>
#include <vector>
#include <unistd.h>

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxRestrictionPsg.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Response.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/Diag804Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "Server/TseServer.h"
#include "Taxes/LegacyTaxes/TaxApply.h"
#include "Taxes/LegacyTaxes/TaxCodeValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"


using namespace std;

namespace tse
{
class TaxCodeValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxCodeValidatorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxCodeValidatorCase0);
  CPPUNIT_TEST(testTaxCodeValidatorCase1);
  CPPUNIT_TEST(testTaxCodeValidatorCase2);
  CPPUNIT_TEST(testTaxCodeValidatorCase3);
  CPPUNIT_TEST(testTaxCodeValidatorCase4);
  CPPUNIT_TEST(testTaxCodeValidatorCase5);
  CPPUNIT_TEST(testTaxCodeValidatorCase6);
  CPPUNIT_TEST(testTaxCodeValidatorCase7);
  CPPUNIT_TEST(testTaxCodeValidatorCase8);
  CPPUNIT_TEST(testTaxCodeValidatorCase9);
  CPPUNIT_TEST(testTaxCodeValidatorCase10);
  CPPUNIT_TEST(testTaxCodeValidatorCase11);
  CPPUNIT_TEST(testTaxCodeValidatorCase12);
  CPPUNIT_TEST(testTaxCodeValidatorCase13);
  CPPUNIT_TEST(testTaxCodeValidator_SegmentFee);
  CPPUNIT_TEST(testTaxCodeValidator_SegmentFee1);
  CPPUNIT_TEST(testTaxCodeValidator_SegmentFee2);
  CPPUNIT_TEST(testTaxCodeValidator_FormOfPayment);
  CPPUNIT_TEST(testTaxCodeValidator_FormOfPayment1);
  CPPUNIT_TEST(testTaxCodeValidator_FormOfPayment2);
  CPPUNIT_TEST(testTaxCodeValidator_FormOfPayment3);
  CPPUNIT_TEST(testTaxCodeValidator_FormOfPayment4);
  CPPUNIT_TEST(testTaxCodeValidator_CarrierRestrictions);
  CPPUNIT_TEST(testTaxCodeValidator_CarrierRestrictions1);
  CPPUNIT_TEST(testTaxCodeValidator_CarrierRestrictions2);
  CPPUNIT_TEST(testTaxCodeValidator_CarrierRestrictions3);
  CPPUNIT_TEST(testTaxCodeValidator_ticketExemptions);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor()
  {
    try { TaxCodeValidatorTest taxCodeValidatorTest; }
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
    _agent->currencyCodeAgent() = "USD";

    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);

    _trx->getRequest()->ticketingAgent() = _agent;
    _trx->getRequest()->ticketingDT() = DateTime(2010, 1, 1); // 10;

    _origin = _memHandle.create<Loc>();
    _destination = _memHandle.create<Loc>();

    _origin->loc() = std::string("DFW");
    _origin->nation() = std::string("US");

    _destination->loc() = std::string("LON");
    _destination->nation() = std::string("GB");
    _trx->getRequest()->ticketingAgent()->agentLocation() = _destination;

    _travelSeg = _memHandle.create<AirSeg>();
    _travelSeg->geoTravelType() = GeoTravelType::Domestic;

    _travelSeg->origin() = _origin;
    _travelSeg->destination() = _destination;
    _travelSeg->departureDT() = DateTime(2010, 1, 1); // 10;

    _itin = _memHandle.create<Itin>();

    _itin->tripCharacteristics().set(Itin::OneWay, true);
    _trx->itin().push_back(_itin);

    _farePath = _memHandle.create<FarePath>();

    _farePath->itin() = _itin;

    _farePath->itin()->travelSeg().push_back(_travelSeg);

    _itin->farePath().push_back(_farePath);

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = _farePath;
    _taxResponse->paxTypeCode() = std::string("ADT");

    _diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    _diag = _memHandle.insert(new Diag804Collector(*_diagroot));

    _taxResponse->diagCollector() = _diag;

    _paxType = _memHandle.create<PaxType>();
    _paxType->paxType() = "ADT";
    _taxResponse->farePath()->paxType() = _paxType;

    _taxCodeReg = _memHandle.create<TaxCodeReg>();
    _taxCodeReg->expireDate() = DateTime(2093, 1, 1); // 999;
    _taxCodeReg->loc1Type() = tse::LocType('N');
    _taxCodeReg->loc2Type() = tse::LocType('N');
    _taxCodeReg->loc1() = tse::LocCode("MY");
    _taxCodeReg->loc1ExclInd() = tse::Indicator('Y');
    _taxCodeReg->loc2() = tse::LocCode("MY");
    _taxCodeReg->loc2ExclInd() = tse::Indicator('N');
    _taxCodeReg->taxCode() = std::string("MY");
    _taxCodeReg->effDate() = DateTime(2001, 1, 1); // 1;
    _taxCodeReg->discDate() = DateTime(2093, 1, 1); // 999;
    _taxCodeReg->firstTvlDate() = DateTime(2005, 1, 1); // 5;
    _taxCodeReg->lastTvlDate() = DateTime(2020, 1, 1); // 20;
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

    _travelSeg2 = _memHandle.create<AirSeg>();
    _origin2 = _memHandle.create<Loc>();
    _destination2 = _memHandle.create<Loc>();

    _options = _memHandle.create<PricingOptions>();
  }

  void setUp2()
  {
    _trx->setOptions(_options);

    _origin->loc() = std::string("NYC");
    _travelSeg->geoTravelType() = GeoTravelType::International;

    _itin->tripCharacteristics().set(Itin::RoundTrip, true);

    _farePath->setTotalNUCAmount(0.0);

    _origin2->loc() = std::string("LON");
    _origin2->nation() = std::string("GB");

    _destination2->loc() = std::string("NYC");
    _destination2->nation() = std::string("US");

    _travelSeg2->geoTravelType() = GeoTravelType::International;

    _travelSeg2->origin() = _origin2;
    _travelSeg2->destination() = _destination2;
    _travelSeg2->departureDT() = DateTime(2094, 1, 2); // 10000;

    _farePath->itin()->travelSeg().push_back(_travelSeg2);

    _taxCodeReg->loc1() = tse::LocCode("");
    _taxCodeReg->loc1ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    _taxCodeReg->loc2() = tse::LocCode("");
    _taxCodeReg->taxCode() = std::string("YC");
    _taxCodeReg->nation() = std::string("US");
    _taxCodeReg->travelType() = tse::Indicator('I');
    _taxCodeReg->itineraryType() = tse::Indicator(' ');
  }

  void tearDown() { _memHandle.clear(); }

  void testTaxCodeValidatorCase0()
  {
    TaxCodeValidator taxCodeValidator;

    bool rc = taxCodeValidator.validateTaxCode(*_trx, *_taxResponse, *_taxCodeReg);

    CPPUNIT_ASSERT(rc == true);
  }

  void testTaxCodeValidatorCase1()
  {
    TaxCodeValidator taxCodeValidator;

    _taxCodeReg->itineraryType() = tse::Indicator('R');

    bool rc = taxCodeValidator.validateTaxCode(*_trx, *_taxResponse, *_taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidatorCase2()
  {
    setUp2();

    TaxCodeValidator taxCodeValidator;

    bool rc = taxCodeValidator.validateTaxCode(*_trx, *_taxResponse, *_taxCodeReg);

    CPPUNIT_ASSERT(rc == true);
  }

  void testTaxCodeValidatorCase3()
  {
    setUp2();

    TaxCodeValidator taxCodeValidator;

    _taxCodeReg->expireDate() = DateTime(2001, 1, 1); // 1;

    bool rc = taxCodeValidator.validateTaxCode(*_trx, *_taxResponse, *_taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidatorCase4()
  {
    setUp2();

    TaxCodeValidator taxCodeValidator;

    _taxCodeReg->expireDate() = DateTime(2096, 1, 1); // 999999;

    _taxCodeReg->effDate() = DateTime(2005, 1, 1); // 99999;
    _taxCodeReg->discDate() = DateTime(2097, 1, 1); // 9999999;
    _taxCodeReg->firstTvlDate() = DateTime(2005, 1, 1); // 5;
    _taxCodeReg->lastTvlDate() = DateTime(2020, 1, 1); // 20;

    bool rc = taxCodeValidator.validateTaxCode(*_trx, *_taxResponse, *_taxCodeReg);

    CPPUNIT_ASSERT(rc == true);
  }

  void testTaxCodeValidatorCase5()
  {

    setUp2();

    TaxCodeValidator taxCodeValidator;

    _taxCodeReg->expireDate() = DateTime(2096, 1, 1); // 999999;
    _taxCodeReg->effDate() = DateTime(2009, 1, 1); // 9;
    _taxCodeReg->discDate() = DateTime(2092, 1, 1); // 99;
    _taxCodeReg->firstTvlDate() = DateTime(2095, 1, 1); // 99999;
    _taxCodeReg->lastTvlDate() = DateTime(2097, 1, 1); // 9999999;

    bool rc = taxCodeValidator.validateTaxCode(*_trx, *_taxResponse, *_taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidatorCase6()
  {

    setUp2();

    TaxCodeValidator taxCodeValidator;

    _taxCodeReg->expireDate() = DateTime(2096, 1, 1); // 999999;
    _taxCodeReg->effDate() = DateTime(2001, 1, 1); // 1;
    _taxCodeReg->discDate() = DateTime(2093, 1, 1); // 999;
    _taxCodeReg->firstTvlDate() = DateTime(2005, 1, 1); // 5;
    _taxCodeReg->lastTvlDate() = DateTime(2020, 1, 1); // 20;
    _taxCodeReg->itineraryType() = tse::Indicator('O');

    bool rc = taxCodeValidator.validateTaxCode(*_trx, *_taxResponse, *_taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidatorCase7()
  {

    setUp2();

    TaxCodeValidator taxCodeValidator;

    _taxCodeReg->expireDate() = DateTime(2096, 1, 1); // 999999;
    _taxCodeReg->effDate() = DateTime(2001, 1, 1); // 1;
    _taxCodeReg->discDate() = DateTime(2093, 1, 1); // 999;
    _taxCodeReg->firstTvlDate() = DateTime(2005, 1, 1); // 5;
    _taxCodeReg->lastTvlDate() = DateTime(2020, 1, 1); // 20;
    _taxCodeReg->itineraryType() = tse::Indicator('O');
    _taxCodeReg->freeTktexempt() = tse::Indicator('Y');

    bool rc = taxCodeValidator.validateTaxCode(*_trx, *_taxResponse, *_taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidatorCase8()
  {

    setUp2();

    TaxCodeValidator taxCodeValidator;

    _taxCodeReg->expireDate() = DateTime(2096, 1, 1); // 999999;
    _taxCodeReg->effDate() = DateTime(2001, 1, 1); // 1
    _taxCodeReg->discDate() = DateTime(2093, 1, 1); // 999;
    _taxCodeReg->firstTvlDate() = DateTime(2005, 1, 1); // 5;
    _taxCodeReg->lastTvlDate() = DateTime(2020, 1, 1); // 20;
    _taxCodeReg->occurrence() = tse::Indicator('C');
    _taxCodeReg->freeTktexempt() = tse::Indicator('N');
    _taxCodeReg->itineraryType() = tse::Indicator(' ');

    _taxCodeReg->posExclInd() = tse::Indicator('N');
    _taxCodeReg->posLocType() = tse::LocType('C');
    _taxCodeReg->posLoc() = tse::LocCode("DFW");

    bool rc = taxCodeValidator.validateTaxCode(*_trx, *_taxResponse, *_taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidatorCase9()
  {

    setUp2();

    TaxCodeValidator taxCodeValidator;

    _taxCodeReg->expireDate() = DateTime(2096, 1, 1); // 999999;
    _taxCodeReg->effDate() = DateTime(2001, 1, 1); // 1;
    _taxCodeReg->discDate() = DateTime(2093, 1, 1); // 999;
    _taxCodeReg->firstTvlDate() = DateTime(2005, 1, 1); // 5;
    _taxCodeReg->lastTvlDate() = DateTime(2020, 1, 1); // 20;
    _taxCodeReg->occurrence() = tse::Indicator('C');
    _taxCodeReg->freeTktexempt() = tse::Indicator('N');
    _taxCodeReg->itineraryType() = tse::Indicator(' ');
    _taxCodeReg->poiExclInd() = tse::Indicator('N');
    _taxCodeReg->poiLocType() = tse::LocType('C');
    _taxCodeReg->poiLoc() = tse::LocCode("DFW");
    _taxCodeReg->posLoc() = tse::LocCode("");

    bool rc = taxCodeValidator.validateTaxCode(*_trx, *_taxResponse, *_taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidatorCase10()
  {

    setUp2();

    TaxCodeValidator taxCodeValidator;

    _taxCodeReg->expireDate() = DateTime(2096, 1, 1); // 999999;
    _taxCodeReg->effDate() = DateTime(2001, 1, 1); // 1;
    _taxCodeReg->discDate() = DateTime(2093, 1, 1); // 999;
    _taxCodeReg->firstTvlDate() = DateTime(2005, 1, 1); // 5;
    _taxCodeReg->lastTvlDate() = DateTime(2020, 1, 1); // 20;
    _taxCodeReg->occurrence() = tse::Indicator('C');
    _taxCodeReg->freeTktexempt() = tse::Indicator('N');
    _taxCodeReg->itineraryType() = tse::Indicator(' ');
    _taxCodeReg->poiExclInd() = tse::Indicator('N');
    _taxCodeReg->poiLocType() = tse::LocType('C');
    _taxCodeReg->posLoc() = tse::LocCode("");
    _taxCodeReg->originLocExclInd() = tse::Indicator('N');
    _taxCodeReg->originLocType() = tse::LocType('C');
    _taxCodeReg->originLoc() = tse::LocCode("DFW");
    _taxCodeReg->poiLoc() = tse::LocCode("");

    bool rc = taxCodeValidator.validateTaxCode(*_trx, *_taxResponse, *_taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidatorCase11()
  {

    setUp2();

    TaxCodeValidator taxCodeValidator;

    _taxCodeReg->expireDate() = DateTime(2096, 1, 1); // 999999;
    _taxCodeReg->effDate() = DateTime(2001, 1, 1); // 1;
    _taxCodeReg->discDate() = DateTime(2093, 1, 1); // 999;
    _taxCodeReg->firstTvlDate() = DateTime(2005, 1, 1); // 5;
    _taxCodeReg->lastTvlDate() = DateTime(2020, 1, 1); // 20;
    _taxCodeReg->occurrence() = tse::Indicator('C');
    _taxCodeReg->freeTktexempt() = tse::Indicator('N');
    _taxCodeReg->itineraryType() = tse::Indicator(' ');
    _taxCodeReg->poiExclInd() = tse::Indicator('N');
    _taxCodeReg->poiLocType() = tse::LocType('C');
    _taxCodeReg->posLoc() = tse::LocCode("");
    _taxCodeReg->originLocExclInd() = tse::Indicator('N');
    _taxCodeReg->originLocType() = tse::LocType('C');
    _taxCodeReg->poiLoc() = tse::LocCode("");
    _taxCodeReg->originLoc() = tse::LocCode("");
    _taxCodeReg->psgrExclInd() = tse::Indicator('Y');

    TaxRestrictionPsg taxRestrictionPsg;
    _taxCodeReg->restrictionPsg().clear();
    taxRestrictionPsg.psgType() = std::string("ADT");
    taxRestrictionPsg.minAge() = 0;
    taxRestrictionPsg.maxAge() = 0;

    _taxCodeReg->restrictionPsg().push_back(taxRestrictionPsg);

    bool rc = taxCodeValidator.validateTaxCode(*_trx, *_taxResponse, *_taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidatorCase12()
  {

    setUp2();

    TaxCodeValidator taxCodeValidator;

    _taxCodeReg->expireDate() = DateTime(2096, 1, 1); // 999999;
    _taxCodeReg->effDate() = DateTime(2001, 1, 1); // 1;
    _taxCodeReg->discDate() = DateTime(2093, 1, 1); // 999;
    _taxCodeReg->firstTvlDate() = DateTime(2005, 1, 1); // 5;
    _taxCodeReg->lastTvlDate() = DateTime(2020, 1, 1); // 20;
    _taxCodeReg->occurrence() = tse::Indicator('C');
    _taxCodeReg->freeTktexempt() = tse::Indicator('N');
    _taxCodeReg->itineraryType() = tse::Indicator(' ');
    _taxCodeReg->poiExclInd() = tse::Indicator('N');
    _taxCodeReg->poiLocType() = tse::LocType('C');
    _taxCodeReg->posLoc() = tse::LocCode("");
    _taxCodeReg->originLocExclInd() = tse::Indicator('N');
    _taxCodeReg->originLocType() = tse::LocType('C');
    _taxCodeReg->poiLoc() = tse::LocCode("");
    _taxCodeReg->originLoc() = tse::LocCode("");
    _taxCodeReg->psgrExclInd() = tse::Indicator('Y');

    TaxRestrictionPsg taxRestrictionPsg;

    taxRestrictionPsg.psgType() = std::string("ADT");
    taxRestrictionPsg.minAge() = 0;
    taxRestrictionPsg.maxAge() = 0;

    _taxCodeReg->restrictionPsg().push_back(taxRestrictionPsg);
    _taxCodeReg->psgrExclInd() = tse::Indicator('N');

    bool rc = taxCodeValidator.validateTaxCode(*_trx, *_taxResponse, *_taxCodeReg);

    CPPUNIT_ASSERT(rc == true);
  }

  void testTaxCodeValidatorCase13()
  {

    setUp2();

    TaxCodeValidator taxCodeValidator;

    _taxCodeReg->expireDate() = DateTime(2096, 1, 1); // 999999;
    _taxCodeReg->effDate() = DateTime(2001, 1, 1); // 1;
    _taxCodeReg->discDate() = DateTime(2093, 1, 1); // 999;
    _taxCodeReg->firstTvlDate() = DateTime(2005, 1, 1); // 5;
    _taxCodeReg->lastTvlDate() = DateTime(2020, 1, 1); // 20;
    _taxCodeReg->occurrence() = tse::Indicator('C');
    _taxCodeReg->freeTktexempt() = tse::Indicator('N');
    _taxCodeReg->itineraryType() = tse::Indicator(' ');
    _taxCodeReg->poiExclInd() = tse::Indicator('N');
    _taxCodeReg->poiLocType() = tse::LocType('C');
    _taxCodeReg->posLoc() = tse::LocCode("");
    _taxCodeReg->originLocExclInd() = tse::Indicator('N');
    _taxCodeReg->originLocType() = tse::LocType('C');
    _taxCodeReg->poiLoc() = tse::LocCode("");
    _taxCodeReg->originLoc() = tse::LocCode("");
    _taxCodeReg->psgrExclInd() = tse::Indicator('Y');

    TaxRestrictionPsg taxRestrictionPsg;

    taxRestrictionPsg.psgType() = std::string("ADT");
    taxRestrictionPsg.minAge() = 5;
    taxRestrictionPsg.maxAge() = 30;

    _taxCodeReg->restrictionPsg().push_back(taxRestrictionPsg);

    bool rc = taxCodeValidator.validateTaxCode(*_trx, *_taxResponse, *_taxCodeReg);

    CPPUNIT_ASSERT(rc == true);
  }

  void testTaxCodeValidator_SegmentFee()
  {
    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;

    TaxResponse taxResponse;

    TaxCodeReg taxCodeReg;

    taxCodeReg.specialProcessNo() = 64;

    Customer* customer = _memHandle.create<Customer>();
    customer->doNotApplySegmentFee() = 'N';

    trx.getRequest()->ticketingAgent()->agentTJR() = customer;

    TaxCodeValidator taxCodeValidator;

    bool rc = taxCodeValidator.validateSegmentFee(trx, taxResponse, taxCodeReg);

    CPPUNIT_ASSERT(rc == true);
  }

  void testTaxCodeValidator_SegmentFee1()
  {
    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;

    TaxResponse taxResponse;

    TaxCodeReg taxCodeReg;

    taxCodeReg.specialProcessNo() = 64;

    Customer* customer = _memHandle.create<Customer>();
    customer->doNotApplySegmentFee() = 'Y';

    trx.getRequest()->ticketingAgent()->agentTJR() = customer;

    TaxCodeValidator taxCodeValidator;

    bool rc = taxCodeValidator.validateSegmentFee(trx, taxResponse, taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidator_SegmentFee2()
  {
    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;

    TaxResponse taxResponse;

    TaxCodeReg taxCodeReg;

    taxCodeReg.specialProcessNo() = 64;

    TaxCodeValidator taxCodeValidator;
    trx.getRequest()->ticketingAgent()->agentTJR() = 0;

    bool rc = taxCodeValidator.validateSegmentFee(trx, taxResponse, taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidator_FormOfPayment()
  {
    PricingTrx trx;

    PricingRequest request;
    trx.setRequest(&request);

    TaxCodeReg taxCodeReg;

    TaxCodeValidator taxCodeValidator;

    bool rc = taxCodeValidator.validateFormOfPayment(trx, taxCodeReg);

    CPPUNIT_ASSERT(rc == true);
  }

  void testTaxCodeValidator_FormOfPayment1()
  {
    PricingTrx trx;

    PricingRequest request;
    trx.setRequest(&request);

    TaxCodeReg taxCodeReg;

    TaxCodeValidator taxCodeValidator;

    taxCodeReg.formOfPayment() = 'A';

    bool rc = taxCodeValidator.validateFormOfPayment(trx, taxCodeReg);

    CPPUNIT_ASSERT(rc == true);
  }

  void testTaxCodeValidator_FormOfPayment2()
  {
    PricingTrx trx;

    PricingRequest request;
    trx.setRequest(&request);

    TaxCodeReg taxCodeReg;

    TaxCodeValidator taxCodeValidator;
    taxCodeReg.formOfPayment() = 'C';
    request.formOfPaymentCash() = 'A';

    bool rc = taxCodeValidator.validateFormOfPayment(trx, taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidator_FormOfPayment3()
  {
    PricingTrx trx;

    PricingRequest request;
    trx.setRequest(&request);

    TaxCodeReg taxCodeReg;

    TaxCodeValidator taxCodeValidator;
    taxCodeReg.formOfPayment() = 'K';
    request.formOfPaymentCash() = 'C';

    bool rc = taxCodeValidator.validateFormOfPayment(trx, taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidator_FormOfPayment4()
  {
    PricingTrx trx;

    PricingRequest request;
    trx.setRequest(&request);

    TaxCodeReg taxCodeReg;

    TaxCodeValidator taxCodeValidator;
    taxCodeReg.formOfPayment() = 'R';
    request.formOfPaymentCash() = 'K';

    bool rc = taxCodeValidator.validateFormOfPayment(trx, taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidator_CarrierRestrictions()
  {
    PricingTrx trx;

    PricingRequest request;
    trx.setRequest(&request);

    Itin itin;

    CarrierCode crx = "LH";

    itin.validatingCarrier() = crx;

    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.setTotalNUCAmount(0.0);
    farePath.itin() = &itin;

    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;
    taxResponse.paxTypeCode() = std::string("ADT");

    TaxCodeReg taxCodeReg;

    taxCodeReg.restrictionValidationCxr().push_back(crx);

    TaxCodeValidator taxCodeValidator;

    bool rc = taxCodeValidator.validateCarrierRestrictions(trx, taxResponse, taxCodeReg);

    CPPUNIT_ASSERT(rc == true);
  }

  void testTaxCodeValidator_CarrierRestrictions1()
  {
    PricingTrx trx;

    PricingRequest request;
    trx.setRequest(&request);

    Itin itin;

    CarrierCode crx = "LH";

    itin.validatingCarrier() = crx;

    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.setTotalNUCAmount(0.0);
    farePath.itin() = &itin;

    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;
    taxResponse.paxTypeCode() = std::string("ADT");

    TaxCodeReg taxCodeReg;

    taxCodeReg.restrictionValidationCxr().push_back(crx);

    TaxCodeValidator taxCodeValidator;
    taxCodeReg.valcxrExclInd() = 'Y';

    bool rc = taxCodeValidator.validateCarrierRestrictions(trx, taxResponse, taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidator_CarrierRestrictions2()
  {
    PricingTrx trx;

    PricingRequest request;
    trx.setRequest(&request);

    Itin itin;

    CarrierCode crx = "LH";

    itin.validatingCarrier() = crx;

    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.setTotalNUCAmount(0.0);
    farePath.itin() = &itin;

    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;
    taxResponse.paxTypeCode() = std::string("ADT");

    TaxCodeReg taxCodeReg;

    taxCodeReg.restrictionValidationCxr().push_back(crx);

    TaxCodeValidator taxCodeValidator;
    taxCodeReg.valcxrExclInd() = 'N';
    CarrierCode crx2 = "LR";
    itin.validatingCarrier() = crx2;

    bool rc = taxCodeValidator.validateCarrierRestrictions(trx, taxResponse, taxCodeReg);

    CPPUNIT_ASSERT(rc == false);
  }

  void testTaxCodeValidator_CarrierRestrictions3()
  {
    PricingTrx trx;

    PricingRequest request;
    trx.setRequest(&request);

    Itin itin;

    CarrierCode crx = "LH";

    itin.validatingCarrier() = crx;

    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.setTotalNUCAmount(0.0);
    farePath.itin() = &itin;

    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;
    taxResponse.paxTypeCode() = std::string("ADT");

    TaxCodeReg taxCodeReg;

    taxCodeReg.restrictionValidationCxr().push_back(crx);
    CarrierCode crx2 = "LR";
    itin.validatingCarrier() = crx2;

    TaxCodeValidator taxCodeValidator;
    taxCodeReg.valcxrExclInd() = 'Y';

    bool rc = taxCodeValidator.validateCarrierRestrictions(trx, taxResponse, taxCodeReg);

    CPPUNIT_ASSERT(rc == true);
  }

  void testTaxCodeValidator_ticketExemptions()
  {
    TaxCodeReg taxCodeReg;
    PricingTrx trx;
    PricingRequest request;
    trx.setRequest(&request);

    Itin itin;
    trx.itin().push_back(&itin);

    FarePath farePath;
    farePath.itin() = &itin;

    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;

    TaxCodeValidator taxCodeValidator;

    //
    farePath.setTotalNUCAmount(0.0);
    taxCodeReg.freeTktexempt() = 'N';
    request.equivAmountOverride() = 0;
    bool rc = taxCodeValidator.validateFreeTktExemption(trx, taxResponse, taxCodeReg);
    CPPUNIT_ASSERT(rc == true);
    //
    farePath.setTotalNUCAmount(0.0);
    taxCodeReg.freeTktexempt() = 'Y';
    request.equivAmountOverride() = 0;
    rc = taxCodeValidator.validateFreeTktExemption(trx, taxResponse, taxCodeReg);
    CPPUNIT_ASSERT(rc == false);
    //
    farePath.setTotalNUCAmount(EPSILON); // 1.0;
    taxCodeReg.freeTktexempt() = 'Y';
    request.equivAmountOverride() = 0;
    rc = taxCodeValidator.validateFreeTktExemption(trx, taxResponse, taxCodeReg);
    CPPUNIT_ASSERT(rc == true);
    //
    farePath.setTotalNUCAmount(EPSILON);
    taxCodeReg.freeTktexempt() = 'Y';
    request.equivAmountOverride() = EPSILON;
    rc = taxCodeValidator.validateFreeTktExemption(trx, taxResponse, taxCodeReg);
    CPPUNIT_ASSERT(rc == true);
  }

private:
  PricingTrx* _trx;
  Agent* _agent;
  PricingRequest* _request;
  PricingOptions* _options;

  Loc* _origin;
  Loc* _destination;
  Loc* _origin2;
  Loc* _destination2;

  AirSeg* _travelSeg;
  AirSeg* _travelSeg2;
  Itin* _itin;
  FarePath* _farePath;
  TaxResponse* _taxResponse;
  PaxType* _paxType;
  TaxCodeReg* _taxCodeReg;
  Diagnostic* _diagroot;
  Diag804Collector* _diag;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxCodeValidatorTest);
}
