#include <boost/assign/std/vector.hpp>
#include <list>
#include <utility>

#include "Common/ErrorResponseException.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/CsoPricingTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/DiskCache.h"
#include "Diagnostic/DiagManager.h"
#include "Service/test/FakeServer.h"
#include "Service/test/FakeService.h"
#include "Service/TransactionOrchestrator.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestItinFactory.h"
#include "test/testdata/TestTravelSegFactory.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

using boost::assign::operator+=;

class RexPricingTrxMock : public RexPricingTrx
{
public:
  RexPricingTrxMock() : _excTrxCreateFail(false)
  {
    setRequest(&_request_);
    setOptions(&_options_);
    newItin().push_back(&_itin_);
    exchangeItin().push_back(&_excItin_);
  }

  virtual void createPricingTrxForCSO(bool forDiagnostic) { _pricingTrxForCSO = &_csoTrx_; }

  virtual void createExchangePricingTrxForRedirect(bool forDiagnostic)
  {
    _exchangePricingTrxForRedirect = _excTrxCreateFail ? NULL : &_excTrx_;
  }

  PricingRequest _request_;
  PricingOptions _options_;
  Itin _itin_, _csoItin_;
  ExcItin _excItin_;
  ExchangePricingTrx _excTrx_;
  CsoPricingTrx _csoTrx_;
  bool _excTrxCreateFail;
};

class TransactionOrchestratorMock : public TransactionOrchestrator
{
public:
  TransactionOrchestratorMock(const std::string& name, TseServer& server) :
    TransactionOrchestrator(name, server),
    _mockInvokeServices(false)
  {

  }

  virtual bool invokeServices(Trx& trx, const uint64_t& serviceBits)
  {
    if (_mockInvokeServices)
    {
      _calls.push_back(std::make_pair(&trx, serviceBits));
      return true;
    }
    else
      return this->TransactionOrchestrator::invokeServices(trx, serviceBits);
  }

  bool _mockInvokeServices;
  std::list< std::pair<const Trx*, uint64_t> > _calls;
};

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

class TransactionOrchestratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TransactionOrchestratorTest);

  CPPUNIT_TEST(testBlankInit);
  CPPUNIT_TEST(testBlankInitFirst);
  CPPUNIT_TEST(testOneValid);
  CPPUNIT_TEST(testInvokeServices);

  CPPUNIT_TEST(testRedirectToPortExchangeWhenUnkErr);
  CPPUNIT_TEST(testRedirectToPortExchangeWhenNoCombinableErr);

  CPPUNIT_TEST(testDiagnosticQualifierProcess_NONE);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A3EXC);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A3NEW);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A3ALL);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A3UFL);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A2);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A3Diag194);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A3Diag231);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A3Diag331);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A3Diag688);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A3Diag689);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A2Diag194);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A2Diag231);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A2Diag331);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A2Diag688);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_A2Diag689);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_InternalExc);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_InternalNew);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_InternalUfl);
  CPPUNIT_TEST(testDiagnosticQualifierProcess_InternalRed);

  CPPUNIT_TEST(testDetermineAndExecuteRedirect_noRedirect);
  CPPUNIT_TEST(testDetermineAndExecuteRedirect_throwRedirectedNoFare);
  CPPUNIT_TEST(testDetermineAndExecuteRedirect_throwRedirectedNoCombinable);
  CPPUNIT_TEST(testDetermineAndExecuteRedirect_throwRedirected);
  CPPUNIT_TEST(testDetermineAndExecuteRedirect_notHandledException);
  CPPUNIT_TEST(testDetermineAndExecuteRedirect_notHandledExceptionEmptySecExcReq);

  CPPUNIT_TEST(testProcessExcItinNoThrow);
  CPPUNIT_TEST(testProcessExcItinErThrow);
  CPPUNIT_TEST(testProcessExcItinStdThrow);
  CPPUNIT_TEST(testProcessExcItinOtherThrow);

  CPPUNIT_TEST(testProcessNewItinNoThrow);
  CPPUNIT_TEST(testProcessNewItinErThrow);
  CPPUNIT_TEST(testProcessNewItinStdThrow);
  CPPUNIT_TEST(testProcessNewItinOtherThrow);

  CPPUNIT_TEST(testProcessUflItinNoThrow);
  CPPUNIT_TEST(testProcessUflItinErThrow);
  CPPUNIT_TEST(testProcessUflItinStdThrow);
  CPPUNIT_TEST(testProcessUflItinOtherThrow);

  CPPUNIT_TEST(testProcessEftItinNoThrow);
  CPPUNIT_TEST(testProcessEftItinErThrow);
  CPPUNIT_TEST(testProcessEftItinStdThrow);
  CPPUNIT_TEST(testProcessEftItinOtherThrow);
  CPPUNIT_TEST(testProcessEftItinNoThrowCreationFail);

  CPPUNIT_TEST(testRexPricingMainProcess_ExcFail_CsoCalled_noRedirect_noSecTypeTrx);
  CPPUNIT_TEST(testRexPricingMainProcess_ExcFail_CsoCalled_noRedirect1);
  CPPUNIT_TEST(testRexPricingMainProcess_ExcFail_CsoCalled_noRedirect2);
  CPPUNIT_TEST(testRexPricingMainProcess_ExcFail_noCsoCalled_Redirect);
  CPPUNIT_TEST(testRexPricingMainProcess_noExcFail_CsoCalled_noRedirect);

  CPPUNIT_TEST(testIsErrorEnforceRedirection_ReissueRulesFail);
  CPPUNIT_TEST(testIsErrorEnforceRedirection_UnknownException);
  CPPUNIT_TEST(testBuildSubItinVectorsOfDifferentCarriers);
  CPPUNIT_TEST(testBuildSubItinVectorsOfSameCarriers);

  CPPUNIT_TEST(testCallPricingSvcSecondTimeEvenException);

  CPPUNIT_TEST(testProcessAncillaryTrx_NotACS);
  CPPUNIT_TEST(testProcessAncillaryTrx_WrongSchemaVersion);
  CPPUNIT_TEST(testProcessAncillaryTrx_EmptyOptions);
  CPPUNIT_TEST(testProcessAncillaryTrx_NoAcsServiceGroups);
  CPPUNIT_TEST(testProcessAncillaryTrx_v3_NoDataRequested);
  CPPUNIT_TEST(testProcessAncillaryTrx_v3_AncillaryDataRequested);
  CPPUNIT_TEST(testProcessAncillaryTrx_v3_noAB240_NoDataRequested);
  CPPUNIT_TEST(testProcessAncillaryTrx_v3_DisclosureDataRequested);
  CPPUNIT_TEST(testProcessAncillaryTrx_v3_CatalogDataRequested);
  CPPUNIT_TEST(testProcessAncillaryTrx_AcsServiceGroup_BG);
  CPPUNIT_TEST(testProcessAncillaryTrx_AcsServiceGroup_PT);

  CPPUNIT_TEST_SUITE_END();

public:
  static const std::string BAD_SERVICE;

  FakeServer* _myServer;
  TransactionOrchestratorMock* _to;
  FakeService* _successService;
  FakeService* _failService;
  RexPricingTrx* _rexTrx;
  PricingTrx* _pricingTrx;
  AncillaryPricingTrx* _ancillaryPricingTrx;
  ExchangePricingTrx* _excTrx;
  PricingRequest* _request;
  PricingOptions* _options;
  Itin* _itin;
  std::vector<Itin*> _itinFirstCxr;
  std::vector<Itin*> _itinSecondCxr;
  ExcItin* _excItin;
  Billing* _billing;
  AncRequest* _ancRequest;
  TestMemHandle _memH;

  template <typename F, typename T>
  T* fabricate(const std::string& data)
  {
    return F::create("/vobs/atseintl/test/testdata/data/" + data);
  }

  void setUpSnapItinerary_diff_cxr()
  {
    _pricingTrx->itin() += fabricate<TestItinFactory, Itin>("ItinSFO_GDL_SFO.xml");

    _itinFirstCxr.clear();
    _itinSecondCxr.clear();

    Itin* itin1 = _memH.create<Itin>();
    Itin* itin2 = _memH.create<Itin>();

    itin1->travelSeg() += fabricate<TestTravelSegFactory, TravelSeg>("AirSegSFO_LAX.xml"),
        fabricate<TestTravelSegFactory, TravelSeg>("AirSegLAX_SFO.xml");
    itin2->travelSeg() += fabricate<TestTravelSegFactory, TravelSeg>("AirSegLAX_GDL.xml"),
        fabricate<TestTravelSegFactory, TravelSeg>("AirSegGDL_LAX.xml");

    _itinFirstCxr.push_back(itin1);
    _itinSecondCxr.push_back(itin2);
  }

  void setUpSnapItinerary_same_cxr()
  {
    _pricingTrx->itin() += fabricate<TestItinFactory, Itin>("ItinSFO_GDL_SFO_WN.xml");

    _itinFirstCxr.clear();
    _itinSecondCxr.clear();

    Itin* itin1 = _memH.create<Itin>();

    itin1->travelSeg() += fabricate<TestTravelSegFactory, TravelSeg>("AirSegSFO_LAX.xml"),
        fabricate<TestTravelSegFactory, TravelSeg>("AirSegLAX_GDL_WN.xml"),
        fabricate<TestTravelSegFactory, TravelSeg>("AirSegGDL_LAX_WN.xml"),
        fabricate<TestTravelSegFactory, TravelSeg>("AirSegLAX_SFO.xml");

    _itinFirstCxr.push_back(itin1);
  }

  void setUp()
  {
    _memH.create<SpecificTestConfigInitializer>();
    _memH.create<RootLoggerGetOff>();
    _myServer = _memH.create<FakeServer>();

    _to = new TransactionOrchestratorMock("TO_SVC", *_myServer);

    const std::string DUMMY_SERVICE = "DUMMY_SVC";
    const bool SUCCESS = true, FAIL = false;

    _successService = _memH.create<FakeService>(DUMMY_SERVICE, *_myServer, SUCCESS);
    _failService = _memH.create<FakeService>(DUMMY_SERVICE, *_myServer, FAIL);

    _rexTrx = _memH.create<RexPricingTrxMock>();
    _pricingTrx = _memH.create<PricingTrx>();
    _ancillaryPricingTrx = _memH.create<AncillaryPricingTrx>();
    _excTrx = _memH.create<ExchangePricingTrx>();
    _request = _memH.create<PricingRequest>();
    _options = _memH.create<PricingOptions>();
    _itin = _memH.create<Itin>();
    _excItin = _memH.create<ExcItin>();
    _billing = _memH.create<Billing>();
    _ancRequest = _memH.create<AncRequest>();

    _excTrx->setRequest(_request);
    _excTrx->setOptions(_options);
    _excTrx->newItin().push_back(_itin);
    _excTrx->exchangeItin().push_back(_excItin);
    _myServer->initializeGlobalConfigMan();
    _myServer->initializeGlobal();
    _to->initialize();
    _rexTrx->billing() = _billing;

    _ancillaryPricingTrx->billing() = _billing;
    _ancillaryPricingTrx->setOptions(_options);
    _ancillaryPricingTrx->setRequest(_ancRequest);
  }

  void tearDown()
  {
    _memH.clear();
    delete _to;
  }

  void testBlankInit()
  {
    // explicitly check each service, on init is invalid
    CPPUNIT_ASSERT(!_to->validateServicePointers(TransactionOrchestrator::ITIN_ANALYZER));
    CPPUNIT_ASSERT(!_to->validateServicePointers(TransactionOrchestrator::FARE_COLLECTOR));
    CPPUNIT_ASSERT(!_to->validateServicePointers(TransactionOrchestrator::FARE_VALIDATOR));
    CPPUNIT_ASSERT(!_to->validateServicePointers(TransactionOrchestrator::TAXES));
    CPPUNIT_ASSERT(!_to->validateServicePointers(TransactionOrchestrator::PRICING));
    CPPUNIT_ASSERT(!_to->validateServicePointers(TransactionOrchestrator::FARE_CALC));
    CPPUNIT_ASSERT(!_to->validateServicePointers(TransactionOrchestrator::CURRENCY));
    CPPUNIT_ASSERT(!_to->validateServicePointers(TransactionOrchestrator::MILEAGE));
    CPPUNIT_ASSERT(!_to->validateServicePointers(TransactionOrchestrator::SHOPPING));
    CPPUNIT_ASSERT(!_to->validateServicePointers(TransactionOrchestrator::SERVICE_FEES));
    CPPUNIT_ASSERT(!_to->validateServicePointers(TransactionOrchestrator::FREE_BAG));
    CPPUNIT_ASSERT(!_to->validateServicePointers(TransactionOrchestrator::TICKETING_FEES));
  }

  void testBlankInitFirst()
  {
    // check all services, on init is invalid (fails on first service)
    CPPUNIT_ASSERT(!_to->validateServicePointers(TransactionOrchestrator::ALL_SERVICES));
  }

  void testOneValid()
  {
    _to->_itinAnalyzerService = _successService;
    CPPUNIT_ASSERT(_to->validateServicePointers(TransactionOrchestrator::ITIN_ANALYZER));
    // verify all others still not set
    CPPUNIT_ASSERT(!_to->validateServicePointers(TransactionOrchestrator::ALL_SERVICES));
  }

  void testInvokeServices()
  {
    PricingTrx trx;

    populateDefaultServices(_successService);

    // It shouldnt find the service yet
    CPPUNIT_ASSERT(_to->invokeServices(trx, TransactionOrchestrator::ITIN_ANALYZER));
    _to->_itinAnalyzerService = 0;
    CPPUNIT_ASSERT(!_to->invokeServices(trx, TransactionOrchestrator::ITIN_ANALYZER));

    CPPUNIT_ASSERT(_to->invokeServices(trx, TransactionOrchestrator::FARE_COLLECTOR));
    _to->_faresCollectionService = 0;
    CPPUNIT_ASSERT(!_to->invokeServices(trx, TransactionOrchestrator::FARE_COLLECTOR));

    CPPUNIT_ASSERT(_to->invokeServices(trx, TransactionOrchestrator::FARE_VALIDATOR));
    _to->_faresValidationService = 0;
    CPPUNIT_ASSERT(!_to->invokeServices(trx, TransactionOrchestrator::FARE_VALIDATOR));

    CPPUNIT_ASSERT(_to->invokeServices(trx, TransactionOrchestrator::TAXES));
    _to->_taxService = 0;
    CPPUNIT_ASSERT(!_to->invokeServices(trx, TransactionOrchestrator::TAXES));

    CPPUNIT_ASSERT(_to->invokeServices(trx, TransactionOrchestrator::PRICING));
    _to->_pricingService = 0;
    CPPUNIT_ASSERT(!_to->invokeServices(trx, TransactionOrchestrator::PRICING));

    CPPUNIT_ASSERT(_to->invokeServices(trx, TransactionOrchestrator::FARE_CALC));
    _to->_fareCalcService = 0;
    CPPUNIT_ASSERT(!_to->invokeServices(trx, TransactionOrchestrator::FARE_CALC));

    CPPUNIT_ASSERT(_to->invokeServices(trx, TransactionOrchestrator::CURRENCY));
    _to->_currencyService = 0;
    CPPUNIT_ASSERT(!_to->invokeServices(trx, TransactionOrchestrator::CURRENCY));

    CPPUNIT_ASSERT(_to->invokeServices(trx, TransactionOrchestrator::MILEAGE));
    _to->_mileageService = 0;
    CPPUNIT_ASSERT(!_to->invokeServices(trx, TransactionOrchestrator::MILEAGE));

    CPPUNIT_ASSERT(_to->invokeServices(trx, TransactionOrchestrator::INTERNAL));
    _to->_internalService = 0;
    CPPUNIT_ASSERT(!_to->invokeServices(trx, TransactionOrchestrator::INTERNAL));

    CPPUNIT_ASSERT(_to->invokeServices(trx, TransactionOrchestrator::SERVICE_FEES));
    _to->_serviceFeesService = 0;
    CPPUNIT_ASSERT(!_to->invokeServices(trx, TransactionOrchestrator::SERVICE_FEES));

    CPPUNIT_ASSERT(_to->invokeServices(trx, TransactionOrchestrator::FREE_BAG));
    _to->_freeBagService = 0;
    CPPUNIT_ASSERT(!_to->invokeServices(trx, TransactionOrchestrator::FREE_BAG));

    CPPUNIT_ASSERT(_to->invokeServices(trx, TransactionOrchestrator::TICKETING_FEES));
    _to->_ticketingFeesService = 0;
    CPPUNIT_ASSERT(!_to->invokeServices(trx, TransactionOrchestrator::TICKETING_FEES));

    _to->_itinAnalyzerService = _successService;
    _to->_faresCollectionService = _successService;
    _to->_faresValidationService = _successService;
    _to->_taxService = _successService;
    _to->_pricingService = _successService;
    _to->_shoppingService = _successService;
    _to->_fareCalcService = _successService;
    _to->_currencyService = _successService;
    _to->_mileageService = _successService;
    _to->_internalService = _successService;
    _to->_serviceFeesService = _successService;
    _to->_ticketingFeesService = _successService;
    _to->_freeBagService = _successService;
    CPPUNIT_ASSERT(_to->invokeServices(trx, TransactionOrchestrator::ALL_SERVICES));

    // Now make sure the CONTINUE on fail doesnt die when one of the services is null
    _to->_taxService = 0;
    CPPUNIT_ASSERT(_to->invokeServices(trx, TransactionOrchestrator::ALL_SERVICES));

    // Now turn the CONTINUE off and set one of them to invalid and make sure it fails
    uint64_t allServices =
        TransactionOrchestrator::ALL_SERVICES ^ TransactionOrchestrator::CONTINUE_ON_FAILURE;

    CPPUNIT_ASSERT(!_to->invokeServices(trx, allServices));

    // Now make one return false and  make sure it still fails
    _to->_taxService = _failService;
    CPPUNIT_ASSERT(!_to->invokeServices(trx, allServices));
  }

  void testDiagnosticQualifierProcess_NONE()
  {
    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_NONE,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A3EXC()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic500;
    _rexTrx->diagnostic().diagParamMap()[Diagnostic::ITIN_TYPE] = "EXC";

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A3NEW()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic500;
    _rexTrx->diagnostic().diagParamMap()[Diagnostic::ITIN_TYPE] = "NEW";

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITNEW,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A3UFL()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic500;
    _rexTrx->diagnostic().diagParamMap()[Diagnostic::ITIN_TYPE] = "UFL";

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITUFL,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A3ALL()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic500;
    _rexTrx->diagnostic().diagParamMap()[Diagnostic::ITIN_TYPE] = "ALL";

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITALL,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A2()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic500;
    _rexTrx->diagnostic().diagParamMap()[Diagnostic::ITIN_TYPE] = "RED";

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEFT,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A3Diag194()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic194;

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A3Diag231()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic231;

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A3Diag331()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic331;

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A3Diag688()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic688;

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A3Diag689()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic689;

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITNEW,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A2Diag194()
  {
    _rexTrx->diagnostic().diagParamMap()[Diagnostic::ITIN_TYPE] = "RED";
    _rexTrx->diagnostic().diagnosticType() = Diagnostic194;

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEFT,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A2Diag231()
  {
    _rexTrx->diagnostic().diagParamMap()[Diagnostic::ITIN_TYPE] = "RED";
    _rexTrx->diagnostic().diagnosticType() = Diagnostic231;

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEFT,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A2Diag331()
  {
    _rexTrx->diagnostic().diagParamMap()[Diagnostic::ITIN_TYPE] = "RED";
    _rexTrx->diagnostic().diagnosticType() = Diagnostic331;

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEFT,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A2Diag688()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic688;
    _rexTrx->diagnostic().diagParamMap()[Diagnostic::ITIN_TYPE] = "RED";

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEFT,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_A2Diag689()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic689;
    _rexTrx->diagnostic().diagParamMap()[Diagnostic::ITIN_TYPE] = "RED";

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEFT,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_InternalExc()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic150;
    _rexTrx->diagnostic().diagParamMap()[Diagnostic::ITIN_TYPE] = "EXC";

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_InternalNew()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic150;
    _rexTrx->diagnostic().diagParamMap()[Diagnostic::ITIN_TYPE] = "NEW";

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_InternalUfl()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic150;
    _rexTrx->diagnostic().diagParamMap()[Diagnostic::ITIN_TYPE] = "UFL";

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITUFL,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void testDiagnosticQualifierProcess_InternalRed()
  {
    _rexTrx->diagnostic().diagnosticType() = Diagnostic150;
    _rexTrx->diagnostic().diagParamMap()[Diagnostic::ITIN_TYPE] = "RED";

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEFT,
                         _to->diagnosticQualifierProcess(*_rexTrx));
  }

  void populateDefaultServices(Service* serv, const std::string& name = "default")
  {
    _to->_faresCollectionService = _to->_faresValidationService = _to->_pricingService =
        _to->_itinAnalyzerService = _to->_taxService = _to->_fareCalcService =
            _to->_currencyService = _to->_mileageService = _to->_shoppingService =
                _to->_internalService = _to->_fareDisplayService = _to->_fareSelectorService =
                    _to->_freeBagService = _to->_serviceFeesService = _to->_ticketingFeesService =
                        _to->_rexFareSelectorService = serv;

    _to->_faresCollectionServiceName = _to->_faresValidationServiceName = _to->_pricingServiceName =
        _to->_shoppingServiceName = _to->_itinAnalyzerServiceName = _to->_taxServiceName =
            _to->_fareCalcServiceName = _to->_currencyServiceName = _to->_mileageServiceName =
                _to->_internalServiceName = _to->_fareDisplayServiceName =
                    _to->_fareSelectorServiceName = _to->_freeBagServiceName =
                        _to->_serviceFeesServiceName = _to->_ticketingFeesServiceName =
                            _to->_rexFareSelectorServiceName = name;
  }

  void testRedirectToPortExchangeWhenUnkErr()
  {
    const char* const MESSAGE = "INVALID TRANSACTION FOR REDIRECT TO PORT EXCHANGE";
    populateDefaultServices(_successService);

    FakeServiceThrowEREx serv(
        BAD_SERVICE,
        *_myServer,
        ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION, MESSAGE));

    _to->_pricingService = &serv;
    std::string actionCode = "WFR";
    _rexTrx->billing()->actionCode() = actionCode;
    _rexTrx->secondaryExcReqType() = FULL_EXCHANGE;

    CPPUNIT_ASSERT_THROW_EQUAL(
        _to->processEftItin(*_rexTrx,
                            (TransactionOrchestrator::FARE_VALIDATOR |
                             TransactionOrchestrator::PRICING | TransactionOrchestrator::TAXES),
                            false),
        ErrorResponseException,
        ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION, MESSAGE));

    CPPUNIT_ASSERT_EQUAL(actionCode + FULL_EXCHANGE, _rexTrx->billing()->actionCode());
  }

  void testRedirectToPortExchangeWhenNoCombinableErr()
  {
    populateDefaultServices(_successService);
    FakeServiceThrowEREx serv(
        BAD_SERVICE,
        *_myServer,
        ErrorResponseException(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS));
    _to->_pricingService = &serv;

    CPPUNIT_ASSERT_THROW_EQUAL(
        _to->processEftItin(*_rexTrx,
                            (TransactionOrchestrator::FARE_VALIDATOR |
                             TransactionOrchestrator::PRICING | TransactionOrchestrator::TAXES),
                            false),
        ErrorResponseException,
        ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
  }

  void testDetermineAndExecuteRedirect_noRedirect()
  {
    populateDefaultServices(_successService);

    _rexTrx->secondaryExcReqType() = FULL_EXCHANGE;

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(result = _to->rexPricingMainProcess(*_rexTrx));

    CPPUNIT_ASSERT(!_rexTrx->redirected());
    CPPUNIT_ASSERT(result);
  }

  void testDetermineAndExecuteRedirect_throwRedirectedNoFare()
  {
    populateDefaultServices(_successService);

    FakeServiceThrowEREx serv(
        BAD_SERVICE, *_myServer, ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    _to->_rexFareSelectorService = &serv;

    _rexTrx->secondaryExcReqType() = FULL_EXCHANGE;

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(result = _to->rexPricingMainProcess(*_rexTrx));

    CPPUNIT_ASSERT(_rexTrx->redirected());
    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::NO_FARE_FOR_CLASS_USED,
                         _rexTrx->redirectReasonError().code());
    CPPUNIT_ASSERT(result);
  }

  void testDetermineAndExecuteRedirect_throwRedirectedNoCombinable()
  {
    populateDefaultServices(_successService);
    FakeServiceThrowEREx serv(
        BAD_SERVICE, *_myServer, ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
    _to->_rexFareSelectorService = &serv;

    _rexTrx->secondaryExcReqType() = FULL_EXCHANGE;

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(result = _to->rexPricingMainProcess(*_rexTrx));

    CPPUNIT_ASSERT(_rexTrx->redirected());
    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS,
                         _rexTrx->redirectReasonError().code());
    CPPUNIT_ASSERT(result);
  }

  void testDetermineAndExecuteRedirect_throwRedirected()
  {
    populateDefaultServices(_successService);
    FakeServiceThrowEREx serv(
        BAD_SERVICE, *_myServer, ErrorResponseException::UNABLE_TO_MATCH_REISSUE_RULES);
    _to->_rexFareSelectorService = &serv;

    _rexTrx->secondaryExcReqType() = FULL_EXCHANGE;

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(result = _to->rexPricingMainProcess(*_rexTrx));

    CPPUNIT_ASSERT(_rexTrx->redirected());
    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::UNABLE_TO_MATCH_REISSUE_RULES,
                         _rexTrx->redirectReasonError().code());
    CPPUNIT_ASSERT(result);
  }

  void testDetermineAndExecuteRedirect_notHandledException()
  {
    populateDefaultServices(_successService);
    FakeServiceThrowEREx serv(BAD_SERVICE, *_myServer, ErrorResponseException::UNKNOWN_EXCEPTION);
    _to->_rexFareSelectorService = &serv;

    _rexTrx->secondaryExcReqType() = FULL_EXCHANGE;

    bool result = false;
    CPPUNIT_ASSERT_THROW_EQUAL(result = _to->rexPricingMainProcess(*_rexTrx),
                               ErrorResponseException,
                               ErrorResponseException::UNKNOWN_EXCEPTION);

    CPPUNIT_ASSERT(!_rexTrx->redirected());
    CPPUNIT_ASSERT(!result);
  }

  void testDetermineAndExecuteRedirect_notHandledExceptionEmptySecExcReq()
  {
    populateDefaultServices(_successService);
    FakeServiceThrowEREx serv(
        BAD_SERVICE, *_myServer, ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    _to->_rexFareSelectorService = &serv;

    _rexTrx->secondaryExcReqType() = "";

    bool result = false;
    CPPUNIT_ASSERT_THROW_EQUAL(result = _to->rexPricingMainProcess(*_rexTrx),
                               ErrorResponseException,
                               ErrorResponseException::NO_FARE_FOR_CLASS_USED);

    CPPUNIT_ASSERT(!_rexTrx->redirected());
    CPPUNIT_ASSERT(!result);
  }

  void testProcessExcItinNoThrow()
  {
    populateDefaultServices(_successService);
    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(result =
                                _to->processExcItin(*_rexTrx,
                                                    (TransactionOrchestrator::FARE_COLLECTOR |
                                                     TransactionOrchestrator::REX_FARE_SELECTOR |
                                                     TransactionOrchestrator::PRICING)));

    CPPUNIT_ASSERT(result);
  }

  void testProcessExcItinErThrow()
  {
    populateDefaultServices(_successService);
    FakeServiceThrowEREx serv(
        BAD_SERVICE, *_myServer, ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    _to->_rexFareSelectorService = &serv;

    bool result = false;
    CPPUNIT_ASSERT_THROW_EQUAL(result =
                                   _to->processExcItin(*_rexTrx,
                                                       (TransactionOrchestrator::FARE_COLLECTOR |
                                                        TransactionOrchestrator::REX_FARE_SELECTOR |
                                                        TransactionOrchestrator::PRICING)),
                               ErrorResponseException,
                               ErrorResponseException::NO_FARE_FOR_CLASS_USED);

    CPPUNIT_ASSERT(!result);
  }

  void testProcessExcItinStdThrow()
  {
    populateDefaultServices(_successService);
    FakeServiceThrowStdEx serv(BAD_SERVICE, *_myServer);
    _to->_rexFareSelectorService = &serv;

    bool result = false;
    CPPUNIT_ASSERT_THROW(result = _to->processExcItin(*_rexTrx,
                                                      (TransactionOrchestrator::FARE_COLLECTOR |
                                                       TransactionOrchestrator::REX_FARE_SELECTOR |
                                                       TransactionOrchestrator::PRICING)),
                         std::bad_cast);

    CPPUNIT_ASSERT(!result);
  }

  void testProcessExcItinOtherThrow()
  {
    populateDefaultServices(_successService);
    FakeServiceThrowOtherEx serv(BAD_SERVICE, *_myServer);
    _to->_rexFareSelectorService = &serv;

    bool result = false;
    CPPUNIT_ASSERT_THROW_EQUAL(result =
                                   _to->processExcItin(*_rexTrx,
                                                       (TransactionOrchestrator::FARE_COLLECTOR |
                                                        TransactionOrchestrator::REX_FARE_SELECTOR |
                                                        TransactionOrchestrator::PRICING)),
                               int,
                               FakeServiceThrowOtherEx::ERROR_CODE);

    CPPUNIT_ASSERT(!result);
  }

  void testProcessNewItinNoThrow()
  {
    populateDefaultServices(_successService);

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(
        result = _to->processNewItin(
            *_rexTrx,
            (TransactionOrchestrator::FARE_COLLECTOR | TransactionOrchestrator::FARE_VALIDATOR |
             TransactionOrchestrator::PRICING | TransactionOrchestrator::TAXES |
             TransactionOrchestrator::FARE_CALC)));

    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::NO_ERROR, _rexTrx->reissuePricingErrorCode());
  }

  void testProcessNewItinErThrow()
  {
    populateDefaultServices(_successService);
    ErrorResponseException e(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
    FakeServiceThrowEREx serv(BAD_SERVICE, *_myServer, e);
    _to->_pricingService = &serv;

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(
        result = _to->processNewItin(
            *_rexTrx,
            (TransactionOrchestrator::FARE_COLLECTOR | TransactionOrchestrator::FARE_VALIDATOR |
             TransactionOrchestrator::PRICING | TransactionOrchestrator::TAXES |
             TransactionOrchestrator::FARE_CALC)));

    CPPUNIT_ASSERT(!result);
    CPPUNIT_ASSERT_EQUAL(e.code(), _rexTrx->reissuePricingErrorCode());
  }

  void testProcessNewItinStdThrow()
  {
    populateDefaultServices(_successService);
    FakeServiceThrowStdEx serv(BAD_SERVICE, *_myServer);
    _to->_pricingService = &serv;

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(
        result = _to->processNewItin(
            *_rexTrx,
            (TransactionOrchestrator::FARE_COLLECTOR | TransactionOrchestrator::FARE_VALIDATOR |
             TransactionOrchestrator::PRICING | TransactionOrchestrator::TAXES |
             TransactionOrchestrator::FARE_CALC)));
    CPPUNIT_ASSERT(!result);
    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::UNKNOWN_EXCEPTION,
                         _rexTrx->reissuePricingErrorCode());
  }

  void testProcessNewItinOtherThrow()
  {
    populateDefaultServices(_successService);
    FakeServiceThrowOtherEx serv(BAD_SERVICE, *_myServer);
    _to->_pricingService = &serv;

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(
        result = _to->processNewItin(
            *_rexTrx,
            (TransactionOrchestrator::FARE_COLLECTOR | TransactionOrchestrator::FARE_VALIDATOR |
             TransactionOrchestrator::PRICING | TransactionOrchestrator::TAXES |
             TransactionOrchestrator::FARE_CALC)));

    CPPUNIT_ASSERT(!result);
    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::UNKNOWN_EXCEPTION,
                         _rexTrx->reissuePricingErrorCode());
  }

  void testProcessUflItinNoThrow()
  {
    populateDefaultServices(_successService);

    _rexTrx->createPricingTrxForCSO(false);

    CPPUNIT_ASSERT(_rexTrx->pricingTrxForCSO());

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(
        result = _to->processUflItin(
            *_rexTrx,
            (TransactionOrchestrator::ITIN_ANALYZER | TransactionOrchestrator::FARE_COLLECTOR |
             TransactionOrchestrator::FARE_VALIDATOR | TransactionOrchestrator::PRICING |
             TransactionOrchestrator::TAXES | TransactionOrchestrator::FARE_CALC)));

    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::NO_ERROR, _rexTrx->csoPricingErrorCode());
  }

  void testProcessUflItinErThrow()
  {
    populateDefaultServices(_successService);
    ErrorResponseException e(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
    FakeServiceThrowEREx serv(BAD_SERVICE, *_myServer, e);
    _to->_pricingService = &serv;

    _rexTrx->createPricingTrxForCSO(false);

    CPPUNIT_ASSERT(_rexTrx->pricingTrxForCSO());

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(
        result = _to->processUflItin(
            *_rexTrx,
            (TransactionOrchestrator::ITIN_ANALYZER | TransactionOrchestrator::FARE_COLLECTOR |
             TransactionOrchestrator::FARE_VALIDATOR | TransactionOrchestrator::PRICING |
             TransactionOrchestrator::TAXES | TransactionOrchestrator::FARE_CALC)));

    CPPUNIT_ASSERT(!result);
    CPPUNIT_ASSERT_EQUAL(e.code(), _rexTrx->csoPricingErrorCode());
  }

  void testProcessUflItinStdThrow()
  {
    populateDefaultServices(_successService);
    FakeServiceThrowStdEx serv(BAD_SERVICE, *_myServer);
    _to->_pricingService = &serv;

    _rexTrx->createPricingTrxForCSO(false);

    CPPUNIT_ASSERT(_rexTrx->pricingTrxForCSO());

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(
        result = _to->processUflItin(
            *_rexTrx,
            (TransactionOrchestrator::ITIN_ANALYZER | TransactionOrchestrator::FARE_COLLECTOR |
             TransactionOrchestrator::FARE_VALIDATOR | TransactionOrchestrator::PRICING |
             TransactionOrchestrator::TAXES | TransactionOrchestrator::FARE_CALC)));

    CPPUNIT_ASSERT(!result);
    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::UNKNOWN_EXCEPTION, _rexTrx->csoPricingErrorCode());
  }

  void testProcessUflItinOtherThrow()
  {
    populateDefaultServices(_successService);
    FakeServiceThrowOtherEx serv(BAD_SERVICE, *_myServer);
    _to->_pricingService = &serv;

    _rexTrx->createPricingTrxForCSO(false);

    CPPUNIT_ASSERT(_rexTrx->pricingTrxForCSO());

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(
        result = _to->processUflItin(
            *_rexTrx,
            (TransactionOrchestrator::ITIN_ANALYZER | TransactionOrchestrator::FARE_COLLECTOR |
             TransactionOrchestrator::FARE_VALIDATOR | TransactionOrchestrator::PRICING |
             TransactionOrchestrator::TAXES | TransactionOrchestrator::FARE_CALC)));

    CPPUNIT_ASSERT(!result);
    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::UNKNOWN_EXCEPTION, _rexTrx->csoPricingErrorCode());
  }

  void testProcessEftItinNoThrow()
  {
    populateDefaultServices(_successService);

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(
        result = _to->processEftItin(
            *_rexTrx,
            (TransactionOrchestrator::ITIN_ANALYZER | TransactionOrchestrator::FARE_COLLECTOR |
             TransactionOrchestrator::FARE_VALIDATOR | TransactionOrchestrator::PRICING |
             TransactionOrchestrator::TAXES | TransactionOrchestrator::FARE_CALC),
            false));

    CPPUNIT_ASSERT(result);
  }

  void testProcessEftItinErThrow()
  {
    populateDefaultServices(_successService);
    ErrorResponseException e(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
    FakeServiceThrowEREx serv(BAD_SERVICE, *_myServer, e);
    _to->_pricingService = &serv;

    bool result = false;
    CPPUNIT_ASSERT_THROW_EQUAL(
        result = _to->processEftItin(
            *_rexTrx,
            (TransactionOrchestrator::ITIN_ANALYZER | TransactionOrchestrator::FARE_COLLECTOR |
             TransactionOrchestrator::FARE_VALIDATOR | TransactionOrchestrator::PRICING |
             TransactionOrchestrator::TAXES | TransactionOrchestrator::FARE_CALC),
            false),
        ErrorResponseException,
        ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);

    CPPUNIT_ASSERT(!result);
  }

  void testProcessEftItinStdThrow()
  {
    populateDefaultServices(_successService);
    FakeServiceThrowStdEx serv(BAD_SERVICE, *_myServer);
    _to->_pricingService = &serv;

    bool result = false;
    CPPUNIT_ASSERT_THROW(
        result = _to->processEftItin(
            *_rexTrx,
            (TransactionOrchestrator::ITIN_ANALYZER | TransactionOrchestrator::FARE_COLLECTOR |
             TransactionOrchestrator::FARE_VALIDATOR | TransactionOrchestrator::PRICING |
             TransactionOrchestrator::TAXES | TransactionOrchestrator::FARE_CALC),
            false),
        std::bad_cast);

    CPPUNIT_ASSERT(!result);
  }

  void testProcessEftItinOtherThrow()
  {
    populateDefaultServices(_successService);
    FakeServiceThrowOtherEx serv(BAD_SERVICE, *_myServer);
    _to->_pricingService = &serv;

    bool result = false;
    CPPUNIT_ASSERT_THROW_EQUAL(
        result = _to->processEftItin(
            *_rexTrx,
            (TransactionOrchestrator::ITIN_ANALYZER | TransactionOrchestrator::FARE_COLLECTOR |
             TransactionOrchestrator::FARE_VALIDATOR | TransactionOrchestrator::PRICING |
             TransactionOrchestrator::TAXES | TransactionOrchestrator::FARE_CALC),
            false),
        int,
        FakeServiceThrowOtherEx::ERROR_CODE);

    CPPUNIT_ASSERT(!result);
  }

  void testProcessEftItinNoThrowCreationFail()
  {
    populateDefaultServices(_successService);

    static_cast<RexPricingTrxMock*>(_rexTrx)->_excTrxCreateFail = true;

    bool result = false;
    CPPUNIT_ASSERT_THROW_EQUAL(
        result = _to->processEftItin(
            *_rexTrx,
            (TransactionOrchestrator::ITIN_ANALYZER | TransactionOrchestrator::FARE_COLLECTOR |
             TransactionOrchestrator::FARE_VALIDATOR | TransactionOrchestrator::PRICING |
             TransactionOrchestrator::TAXES | TransactionOrchestrator::FARE_CALC),
            false),
        ErrorResponseException,
        ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION,
                               "INVALID TRANSACTION FOR REDIRECT TO PORT EXCHANGE"));

    CPPUNIT_ASSERT(!result);
  }

  void testRexPricingMainProcess_ExcFail_CsoCalled_noRedirect_noSecTypeTrx()
  {
    populateDefaultServices(_successService);

    FakeService lastServ("LastServ", *_myServer);
    _to->_fareCalcService = &lastServ;
    lastServ._ran = false;

    FakeServiceThrowOtherEx serv(BAD_SERVICE, *_myServer);
    _to->_rexFareSelectorService = &serv;

    _rexTrx->secondaryExcReqType() = "";

    bool result = false;
    CPPUNIT_ASSERT_THROW_EQUAL(
        result = _to->rexPricingMainProcess(*_rexTrx), int, FakeServiceThrowOtherEx::ERROR_CODE);

    CPPUNIT_ASSERT(!result);
    CPPUNIT_ASSERT(_rexTrx->pricingTrxForCSO());
    CPPUNIT_ASSERT(_rexTrx->csoTransSuccessful());
    CPPUNIT_ASSERT(!_rexTrx->exchangePricingTrxForRedirect());
    CPPUNIT_ASSERT(!_rexTrx->redirectResult());
    CPPUNIT_ASSERT(lastServ._ran);
  }

  void testRexPricingMainProcess_ExcFail_CsoCalled_noRedirect1()
  {
    populateDefaultServices(_successService);

    FakeService lastServ("LastServ", *_myServer);
    _to->_fareCalcService = &lastServ;
    lastServ._ran = false;

    FakeServiceThrowEREx serv(BAD_SERVICE, *_myServer, ErrorResponseException::UNKNOWN_EXCEPTION);
    _to->_rexFareSelectorService = &serv;

    _rexTrx->secondaryExcReqType() = FULL_EXCHANGE;

    bool result = false;
    CPPUNIT_ASSERT_THROW_EQUAL(result = _to->rexPricingMainProcess(*_rexTrx),
                               ErrorResponseException,
                               ErrorResponseException::UNKNOWN_EXCEPTION);

    CPPUNIT_ASSERT(!result);
    CPPUNIT_ASSERT(_rexTrx->pricingTrxForCSO());
    CPPUNIT_ASSERT(_rexTrx->csoTransSuccessful());
    CPPUNIT_ASSERT(!_rexTrx->exchangePricingTrxForRedirect());
    CPPUNIT_ASSERT(!_rexTrx->redirectResult());
    CPPUNIT_ASSERT(lastServ._ran);
  }

  void testRexPricingMainProcess_ExcFail_CsoCalled_noRedirect2()
  {
    populateDefaultServices(_successService);

    FakeService lastServ("LastServ", *_myServer);
    _to->_fareCalcService = &lastServ;
    lastServ._ran = false;

    FakeServiceThrowOtherEx serv(BAD_SERVICE, *_myServer);
    _to->_rexFareSelectorService = &serv;

    _rexTrx->secondaryExcReqType() = FULL_EXCHANGE;

    bool result = false;
    CPPUNIT_ASSERT_THROW_EQUAL(
        result = _to->rexPricingMainProcess(*_rexTrx), int, FakeServiceThrowOtherEx::ERROR_CODE);

    CPPUNIT_ASSERT(!result);
    CPPUNIT_ASSERT(_rexTrx->pricingTrxForCSO());
    CPPUNIT_ASSERT(_rexTrx->csoTransSuccessful());
    CPPUNIT_ASSERT(!_rexTrx->exchangePricingTrxForRedirect());
    CPPUNIT_ASSERT(!_rexTrx->redirectResult());
    CPPUNIT_ASSERT(lastServ._ran);
  }

  void testRexPricingMainProcess_ExcFail_noCsoCalled_Redirect()
  {
    populateDefaultServices(_successService);

    FakeService lastServ("LastServ", *_myServer);
    _to->_fareCalcService = &lastServ;
    lastServ._ran = false;

    FakeServiceThrowEREx serv(
        BAD_SERVICE, *_myServer, ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    _to->_rexFareSelectorService = &serv;

    _rexTrx->secondaryExcReqType() = FULL_EXCHANGE;

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(result = _to->rexPricingMainProcess(*_rexTrx));

    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT(_rexTrx->pricingTrxForCSO());
    CPPUNIT_ASSERT(!_rexTrx->csoTransSuccessful());
    CPPUNIT_ASSERT(_rexTrx->exchangePricingTrxForRedirect());
    CPPUNIT_ASSERT(_rexTrx->redirectResult());
    CPPUNIT_ASSERT(lastServ._ran);
  }

  void testRexPricingMainProcess_noExcFail_CsoCalled_noRedirect()
  {
    populateDefaultServices(_successService);

    FakeService lastServ("LastServ", *_myServer);
    _to->_fareCalcService = &lastServ;
    lastServ._ran = false;

    _rexTrx->secondaryExcReqType() = FULL_EXCHANGE;

    bool result = false;
    CPPUNIT_ASSERT_NO_THROW(result = _to->rexPricingMainProcess(*_rexTrx));

    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT(_rexTrx->pricingTrxForCSO());
    CPPUNIT_ASSERT(_rexTrx->csoTransSuccessful());
    CPPUNIT_ASSERT(!_rexTrx->exchangePricingTrxForRedirect());
    CPPUNIT_ASSERT(!_rexTrx->redirectResult());
    CPPUNIT_ASSERT(lastServ._ran);
  }

  void testIsErrorEnforceRedirection_ReissueRulesFail()
  {
    ErrorResponseException err(ErrorResponseException::REISSUE_RULES_FAIL);

    CPPUNIT_ASSERT(!_to->isErrorEnforceRedirection(err));
  }

  void testIsErrorEnforceRedirection_UnknownException()
  {
    ErrorResponseException err(ErrorResponseException::UNKNOWN_EXCEPTION);

    CPPUNIT_ASSERT(!_to->isErrorEnforceRedirection(err));
  }

  void compareItin(std::vector<Itin*>& firstItinVec, std::vector<Itin*>& secondItinVec)
  {
    std::vector<Itin*>::iterator itinVecFirstIt = firstItinVec.begin();
    std::vector<Itin*>::iterator itinVecFirstItEnd = firstItinVec.end();

    std::vector<Itin*>::iterator itinVecSecondIt = secondItinVec.begin();
    std::vector<Itin*>::iterator itinVecSecondItEnd = secondItinVec.end();

    for (; itinVecFirstIt != itinVecFirstItEnd || itinVecSecondIt != itinVecSecondItEnd;
         ++itinVecFirstIt, ++itinVecSecondIt)
    {
      std::vector<TravelSeg*>::iterator travelSegIter = (*itinVecFirstIt)->travelSeg().begin();
      std::vector<TravelSeg*>::iterator travelSegIterEnd = (*itinVecFirstIt)->travelSeg().end();

      std::vector<TravelSeg*>::iterator compareTravelSegIter =
          (*itinVecSecondIt)->travelSeg().begin();
      std::vector<TravelSeg*>::iterator compareTravelSegIterEnd =
          (*itinVecSecondIt)->travelSeg().end();

      for (; travelSegIter != travelSegIterEnd || compareTravelSegIter != compareTravelSegIterEnd;
           ++travelSegIter, ++compareTravelSegIter)
      {
        TravelSeg* travelSeg = (*travelSegIter);
        AirSeg* airSeg = dynamic_cast<AirSeg*>(travelSeg);
        TravelSeg* travelSegComp = (*compareTravelSegIter);
        AirSeg* airSegComp = dynamic_cast<AirSeg*>(travelSegComp);

        if (airSeg != NULL && airSegComp != NULL)
        {
          CPPUNIT_ASSERT_EQUAL(airSegComp->carrier(), airSeg->carrier());
          CPPUNIT_ASSERT_EQUAL(airSegComp->flightNumber(), airSeg->flightNumber());
        }
      }
    }
  }

  void testBuildSubItinVectorsOfDifferentCarriers()
  {
    setUpSnapItinerary_diff_cxr();
    _to->buildSubItinVectors(*_pricingTrx);

    compareItin(_pricingTrx->subItinVecFirstCxr(), _itinFirstCxr);
    compareItin(_pricingTrx->subItinVecSecondCxr(), _itinSecondCxr);
  }

  void testBuildSubItinVectorsOfSameCarriers()
  {
    setUpSnapItinerary_same_cxr();
    _to->buildSubItinVectors(*_pricingTrx);

    compareItin(_pricingTrx->subItinVecFirstCxr(), _itinFirstCxr);
    CPPUNIT_ASSERT(!_pricingTrx->subItinVecSecondCxr().size());
  }

  class TOStub : public TransactionOrchestrator
  {
  public:
    TOStub(TseServer& server) : TransactionOrchestrator("TO_SVC", server), _hitCounter(0) {}

    unsigned _hitCounter;

  protected:
    virtual bool invokeServices(Trx&, const uint64_t& serviceBits)
    {
      ++_hitCounter;
      throw ErrorResponseException(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
      return false;
    }
  };

  void testCallPricingSvcSecondTimeEvenException()
  {
    ReissueSequence rs;
    rs.processingInd() = KEEP_THE_FARES;
    rs.stopInd() = ProcessTagPermutation::STOP_TRY_KEEP;

    ProcessTagInfo pti;
    pti.reissueSequence()->orig() = &rs;

    ProcessTagPermutation ptp;
    ptp.processTags().push_back(&pti);

    _rexTrx->insert(ptp);

    TOStub tostub(*_myServer);

    tostub.processNewItin(*_rexTrx, TransactionOrchestrator::PRICING);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned>(2), tostub._hitCounter);
  }

  void testProcessAncillaryTrx_NotACS()
  {
    populateDefaultServices(_successService);
    _to->_freeBagService = 0;
    _billing->requestPath() = PSS_PO_ATSE_PATH;

    CPPUNIT_ASSERT(_to->process(*_ancillaryPricingTrx));
  }

  void testProcessAncillaryTrx_WrongSchemaVersion()
  {
    populateDefaultServices(_successService);
    _to->_freeBagService = 0;
    _billing->requestPath() = ACS_PO_ATSE_PATH;
    _ancRequest->majorSchemaVersion() = 1;

    CPPUNIT_ASSERT(_to->process(*_ancillaryPricingTrx));
  }

  void testProcessAncillaryTrx_EmptyOptions()
  {
    populateDefaultServices(_successService);
    _to->_freeBagService = 0;
    _billing->requestPath() = ACS_PO_ATSE_PATH;
    _ancRequest->majorSchemaVersion() = 2;

    CPPUNIT_ASSERT(!_to->process(*_ancillaryPricingTrx));
  }

  void testProcessAncillaryTrx_NoAcsServiceGroups()
  {
    populateDefaultServices(_successService);
    _to->_freeBagService = 0;
    _billing->requestPath() = ACS_PO_ATSE_PATH;
    _ancRequest->majorSchemaVersion() = 2;
    RequestedOcFeeGroup group;
    group.groupCode() = "AB";
    _options->serviceGroupsVec() += group;

    CPPUNIT_ASSERT(_to->process(*_ancillaryPricingTrx));
  }

  void testProcessAncillaryTrx_AcsServiceGroup_BG()
  {
    populateDefaultServices(_successService);
    _to->_freeBagService = 0;
    _billing->requestPath() = ACS_PO_ATSE_PATH;
    _ancRequest->majorSchemaVersion() = 2;
    RequestedOcFeeGroup group;
    group.groupCode() = "BG";
    _options->serviceGroupsVec() += group;

    CPPUNIT_ASSERT(!_to->process(*_ancillaryPricingTrx));
  }

  void testProcessAncillaryTrx_v3_Init(bool isAB240, RequestedOcFeeGroup::RequestedInformation requestedInformation)
  {
    populateDefaultServices(_successService);
    _to->_freeBagService = 0;
    _to->_mockInvokeServices = true;
    _to->_calls.clear();
    _billing->requestPath() = ACS_PO_ATSE_PATH;
    _ancRequest->majorSchemaVersion() = 3;
    RequestedOcFeeGroup group;
    group.groupCode() = "BG";
    group.setRequestedInformation(requestedInformation);
    _options->serviceGroupsVec() += group;
    _ancillaryPricingTrx->modifiableActivationFlags().setAB240(isAB240);
  }

  void testProcessAncillaryTrx_v3_AncillaryDataRequested()
  {
    testProcessAncillaryTrx_v3_Init(true, RequestedOcFeeGroup::AncillaryData);

    CPPUNIT_ASSERT_EQUAL(false, _options->isProcessAllGroups());
    CPPUNIT_ASSERT_EQUAL(true, _ancillaryPricingTrx->activationFlags().isAB240());

    CPPUNIT_ASSERT(_to->process(*_ancillaryPricingTrx));
    CPPUNIT_ASSERT_EQUAL((size_t)1, _to->_calls.size());
    CPPUNIT_ASSERT_EQUAL((uint64_t)(TransactionOrchestrator::ITIN_ANALYZER |
                                    TransactionOrchestrator::SERVICE_FEES |
                                    TransactionOrchestrator::TAXES),
                         _to->_calls.front().second);
  }

  void testProcessAncillaryTrx_v3_noAB240_NoDataRequested()
  {
    testProcessAncillaryTrx_v3_Init(false, RequestedOcFeeGroup::NoData);

    CPPUNIT_ASSERT_EQUAL(false, _options->isProcessAllGroups());
    CPPUNIT_ASSERT_EQUAL(false, _ancillaryPricingTrx->activationFlags().isAB240());

    CPPUNIT_ASSERT(_to->process(*_ancillaryPricingTrx));
    CPPUNIT_ASSERT_EQUAL((size_t)1, _to->_calls.size());
    CPPUNIT_ASSERT_EQUAL((uint64_t)(TransactionOrchestrator::ITIN_ANALYZER |
                                    TransactionOrchestrator::SERVICE_FEES |
                                    TransactionOrchestrator::FREE_BAG |
                                    TransactionOrchestrator::TAXES),
                         _to->_calls.front().second);
  }

  void testProcessAncillaryTrx_v3_NoDataRequested()
  {
    testProcessAncillaryTrx_v3_Init(true, RequestedOcFeeGroup::NoData);

    CPPUNIT_ASSERT_EQUAL(false, _options->isProcessAllGroups());
    CPPUNIT_ASSERT_EQUAL(true, _ancillaryPricingTrx->activationFlags().isAB240());

    CPPUNIT_ASSERT(!_to->process(*_ancillaryPricingTrx));
  }

  void testProcessAncillaryTrx_v3_DisclosureDataRequested()
  {
    testProcessAncillaryTrx_v3_Init(true, RequestedOcFeeGroup::DisclosureData);

    CPPUNIT_ASSERT_EQUAL(false, _options->isProcessAllGroups());
    CPPUNIT_ASSERT_EQUAL(true, _ancillaryPricingTrx->activationFlags().isAB240());

    CPPUNIT_ASSERT(_to->process(*_ancillaryPricingTrx));
    CPPUNIT_ASSERT_EQUAL((size_t)1, _to->_calls.size());
    CPPUNIT_ASSERT_EQUAL((uint64_t)(TransactionOrchestrator::ITIN_ANALYZER |
                                    TransactionOrchestrator::FREE_BAG |
                                    TransactionOrchestrator::TAXES),
                         _to->_calls.front().second);
  }

  void testProcessAncillaryTrx_v3_CatalogDataRequested()
  {
    testProcessAncillaryTrx_v3_Init(true, RequestedOcFeeGroup::CatalogData);

    CPPUNIT_ASSERT_EQUAL(false, _options->isProcessAllGroups());
    CPPUNIT_ASSERT_EQUAL(true, _ancillaryPricingTrx->activationFlags().isAB240());

    CPPUNIT_ASSERT(_to->process(*_ancillaryPricingTrx));
    CPPUNIT_ASSERT_EQUAL((size_t)1, _to->_calls.size());
    CPPUNIT_ASSERT_EQUAL((uint64_t)(TransactionOrchestrator::ITIN_ANALYZER |
                                    TransactionOrchestrator::FREE_BAG |
                                    TransactionOrchestrator::TAXES),
                         _to->_calls.front().second);
  }

  void testProcessAncillaryTrx_AcsServiceGroup_PT()
  {
    populateDefaultServices(_successService);
    _to->_freeBagService = 0;
    _billing->requestPath() = ACS_PO_ATSE_PATH;
    _ancRequest->majorSchemaVersion() = 2;
    RequestedOcFeeGroup group;
    group.groupCode() = "PT";
    _options->serviceGroupsVec() += group;

    CPPUNIT_ASSERT(!_to->process(*_ancillaryPricingTrx));
  }
};

const std::string TransactionOrchestratorTest::BAD_SERVICE = "BAD_SERVICE";

CPPUNIT_TEST_SUITE_REGISTRATION(TransactionOrchestratorTest);

std::ostream& operator<<(std::ostream& os, const tse::ErrorResponseException& e)
{
  return os << e.code() << ":" << e.message();
}

bool operator==(const ErrorResponseException& lhs, const ErrorResponseException& rhs)
{
  return lhs.code() == rhs.code() && lhs.message() == rhs.message();
}

} // tse
