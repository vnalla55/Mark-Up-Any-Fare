#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "Diagnostic/Diag804Collector.h"
#include "Taxes/LegacyTaxes/CabinValidator.h"
#include "test/include/TestMemHandle.h"

#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestXMLHelper.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

class CabinValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CabinValidatorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testCabinValidator);
  CPPUNIT_TEST(testCabinValidatorCase1);
  CPPUNIT_TEST(testCabinValidatorCase2);
  CPPUNIT_TEST(testCabinValidatorCase3);
  CPPUNIT_TEST(testCabinValidatorCase4);
  CPPUNIT_TEST(testCabinValidatorCase5);
  CPPUNIT_TEST(testCabinValidatorCase6);
  CPPUNIT_TEST(testCabinValidatorCase7);
  CPPUNIT_TEST(testCabinValidatorCase8);
  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:
  void testConstructor()
  {
    try { CabinValidator cabinValidator; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  //-----------------------------------------------------------------
  // setUp()
  //-----------------------------------------------------------------
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();

    _agent = _memHandle.create<Agent>();
    _request = _memHandle.create<PricingRequest>();
    _loc = _memHandle.create<Loc>();
    _options = _memHandle.create<PricingOptions>();

    _itin = _memHandle.create<Itin>();
    _farePath = _memHandle.create<FarePath>();
    _taxResponse = _memHandle.create<TaxResponse>();

    _pricingUnit = _memHandle.create<PricingUnit>();
    _fareUsage = _memHandle.create<FareUsage>();
    _fare = _memHandle.create<Fare>();
    _paxTypeFare = _memHandle.create<PaxTypeFare>();

    _fareMarket = _memHandle.create<FareMarket>();
    _fareInfo = _memHandle.create<FareInfo>();
    _tariffRefInfo = _memHandle.create<TariffCrossRefInfo>();
    _paxType = _memHandle.create<PaxType>();
    _pti = _memHandle.create<PaxTypeInfo>();

    _agent->currencyCodeAgent() = "USD";

    _trx->setRequest(_request);
    _trx->getRequest()->ticketingAgent() = _agent;

    DateTime dt;
    _trx->getRequest()->ticketingDT() = dt.localTime();

    _loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    _request->ticketingAgent()->agentLocation() = _loc;

    _trx->setOptions(_options);

    _airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegJFK_DFW.xml");
    _airSeg2 = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_ORD.xml");

    _itin->originationCurrency() = "USD";
    _itin->calculationCurrency() = "USD";

    _trx->itin().push_back(_itin);

    _farePath->setTotalNUCAmount(500.00);
    _farePath->itin() = _itin;

    _farePath->itin()->travelSeg().push_back(_airSeg);
    _farePath->itin()->travelSeg().push_back(_airSeg2);

    _itin->farePath().push_back(_farePath);

    _taxResponse->farePath() = _farePath;

    _diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    _diag = _memHandle.insert(new Diag804Collector(*_diagroot));

    _taxResponse->diagCollector() = _diag;

    _fareMarket->travelSeg().push_back(_airSeg);
    _fareMarket->travelSeg().push_back(_airSeg2);

    _fare->nucFareAmount() = 1000.00;

    _fareInfo->_fareAmount = 1000.00;
    _fareInfo->_currency = "USD";
    _fareInfo->_fareTariff = 0;
    _fareInfo->_fareClass = "AA";

    _tariffRefInfo->_fareTariff = 0;

    _fare->initialize(Fare::FS_International, _fareInfo, *_fareMarket, _tariffRefInfo);

    _paxType->paxType() = std::string("ADT");
    _paxType->paxTypeInfo() = _pti;
    _itin->farePath()[0]->paxType() = _paxType;

    _paxTypeFare->paxType() = _paxType;

    _fareUsage->paxTypeFare() = _paxTypeFare;
    _fareUsage->paxTypeFare()->setFare(_fare);

    _fareUsage->travelSeg().push_back(_airSeg);
    _fareUsage->travelSeg().push_back(_airSeg2);

    _taxResponse->farePath()->pricingUnit().push_back(_pricingUnit);
    _taxResponse->farePath()->pricingUnit()[0]->fareUsage().push_back(_fareUsage);
  }

  //-----------------------------------------------------------------
  // tearDown()
  //-----------------------------------------------------------------
  void tearDown() { _memHandle.clear(); }

  void testCabinValidator()
  {
    std::vector<TravelSeg*>::const_iterator tsI;
    tsI = _farePath->itin()->travelSeg().begin();

    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_ZP.xml");
    taxCodeReg->cabins().clear();

    bool rc = CabinValidator().validateCabinRestriction(*_trx, *_taxResponse, *taxCodeReg, *tsI);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testCabinValidatorCase1()
  {
    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_ZP.xml");
    taxCodeReg->cabins().clear();

    TaxCodeCabin taxCodeCabin;

    taxCodeCabin.carrier() = "AA";
    taxCodeCabin.classOfService() = "Y";
    taxCodeReg->cabins().push_back(&taxCodeCabin);

    std::vector<TravelSeg*>::const_iterator tsI;
    tsI = _farePath->itin()->travelSeg().begin();

    bool rc = CabinValidator().validateCabinRestriction(*_trx, *_taxResponse, *taxCodeReg, *tsI);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testCabinValidatorCase2()
  {
    std::vector<TravelSeg*>::const_iterator tsI;
    tsI = _farePath->itin()->travelSeg().begin();

    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_ZP.xml");
    taxCodeReg->cabins().clear();

    TaxCodeCabin taxCodeCabin;

    taxCodeCabin.classOfService() = "*";
    taxCodeCabin.carrier() = "";
    taxCodeReg->cabins().push_back(&taxCodeCabin);

    bool rc = CabinValidator().validateCabinRestriction(*_trx, *_taxResponse, *taxCodeReg, *tsI);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testCabinValidatorCase3()
  {
    std::vector<TravelSeg*>::const_iterator tsI;
    tsI = _farePath->itin()->travelSeg().begin();

    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_ZP.xml");
    taxCodeReg->cabins().clear();

    TaxCodeCabin taxCodeCabin;

    _airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegYYC_DFW_AA.xml");

    taxCodeCabin.classOfService() = _airSeg->getBookingCode();
    taxCodeCabin.carrier() = "";
    taxCodeReg->cabins().push_back(&taxCodeCabin);

    bool rc = CabinValidator().validateCabinRestriction(*_trx, *_taxResponse, *taxCodeReg, *tsI);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testCabinValidatorCase4()
  {
    std::vector<TravelSeg*>::const_iterator tsI;
    tsI = _farePath->itin()->travelSeg().begin();

    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_SQ.xml");
    taxCodeReg->cabins().clear();

    TaxCodeCabin taxCodeCabin;

    _airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegYYC_DFW_AA.xml");

    _farePath->itin()->travelSeg().clear();
    _fareUsage->travelSeg().clear();
    _fareMarket->travelSeg().clear();

    _farePath->itin()->travelSeg().push_back(_airSeg);
    _fareUsage->travelSeg().push_back(_airSeg);
    _fareMarket->travelSeg().push_back(_airSeg);

    taxCodeCabin.classOfService() = _airSeg->getBookingCode();
    taxCodeCabin.carrier() = "BA";
    taxCodeReg->cabins().push_back(&taxCodeCabin);

    bool rc = CabinValidator().validateCabinRestriction(*_trx, *_taxResponse, *taxCodeReg, *tsI);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testCabinValidatorCase5()
  {
    std::vector<TravelSeg*>::const_iterator tsI;
    tsI = _farePath->itin()->travelSeg().begin();

    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_SQ.xml");
    taxCodeReg->cabins().clear();

    TaxCodeCabin taxCodeCabin;

    _airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegYYC_DFW_AA.xml");

    taxCodeCabin.classOfService() = _airSeg->getBookingCode();
    taxCodeCabin.carrier() = "BA";
    taxCodeReg->cabins().push_back(&taxCodeCabin);

    _trx->getRequest()->lowFareRequested() = 'T';
    _trx->getRequest()->lowFareNoAvailability() = 'T';

    bool rc = CabinValidator().validateCabinRestriction(*_trx, *_taxResponse, *taxCodeReg, *tsI);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testCabinValidatorCase6()
  {
    std::vector<TravelSeg*>::const_iterator tsI;
    tsI = _farePath->itin()->travelSeg().begin();

    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_ZP.xml");
    taxCodeReg->cabins().clear();

    TaxCodeCabin taxCodeCabin;

    _airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegYYC_DFW_AA.xml");

    taxCodeCabin.classOfService() = _airSeg->getBookingCode();
    taxCodeCabin.carrier() = "BA";
    taxCodeReg->cabins().push_back(&taxCodeCabin);

    bool rc = CabinValidator().validateCabinRestriction(*_trx, *_taxResponse, *taxCodeReg, *tsI);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testCabinValidatorCase7()
  {
    std::vector<TravelSeg*>::const_iterator tsI;
    tsI = _farePath->itin()->travelSeg().begin();

    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_SQ.xml");
    taxCodeReg->cabins().clear();

    TaxCodeCabin taxCodeCabin;

    _airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegYYC_DFW_AA.xml");

    taxCodeCabin.classOfService() = _airSeg->getBookingCode();
    taxCodeCabin.carrier() = "BA";
    taxCodeCabin.exceptInd() = 'Y';
    taxCodeReg->cabins().push_back(&taxCodeCabin);

    bool rc = CabinValidator().validateCabinRestriction(*_trx, *_taxResponse, *taxCodeReg, *tsI);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testCabinValidatorCase8()
  {
    std::vector<TravelSeg*>::const_iterator tsI;
    tsI = _farePath->itin()->travelSeg().begin();

    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_SQ.xml");
    taxCodeReg->cabins().clear();

    TaxCodeCabin taxCodeCabin;

    taxCodeCabin.carrier() = "AA";
    taxCodeCabin.classOfService() = "*";
    taxCodeCabin.directionalInd() = BETWEEN;
    taxCodeCabin.exceptInd() = 'Y';
    taxCodeReg->cabins().push_back(&taxCodeCabin);

    bool rc = CabinValidator().validateCabinRestriction(*_trx, *_taxResponse, *taxCodeReg, *tsI);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

private:
  PricingTrx* _trx;

  Agent* _agent;
  PricingRequest* _request;
  Loc* _loc;
  PricingOptions* _options;
  AirSeg* _airSeg;
  AirSeg* _airSeg2;

  Itin* _itin;
  FarePath* _farePath;
  TaxResponse* _taxResponse;

  Diagnostic* _diagroot;
  Diag804Collector* _diag;

  PricingUnit* _pricingUnit;
  FareUsage* _fareUsage;
  Fare* _fare;
  PaxTypeFare* _paxTypeFare;

  FareMarket* _fareMarket;
  FareInfo* _fareInfo;
  TariffCrossRefInfo* _tariffRefInfo;
  PaxType* _paxType;
  PaxTypeInfo* _pti;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(CabinValidatorTest);
}
