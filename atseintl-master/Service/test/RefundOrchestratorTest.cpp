#include "test/include/CppUnitHelperMacros.h"
#include "Service/RefundOrchestrator.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RefundPricingRequest.h"
#include "DBAccess/DiskCache.h"
#include "Service/TransactionOrchestrator.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/ExcItin.h"
#include "Service/test/FakeService.h"
#include "Service/test/FakeServer.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/RexPricingOptions.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

class RefundOrchestratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RefundOrchestratorTest);

  CPPUNIT_TEST(testProcessWhenItinAnalyzerReturnsFalse);
  CPPUNIT_TEST(testProcessWhenFareCollectorReturnsFalse);
  CPPUNIT_TEST(testProcessWhenRexFareSelectorReturnsFalse);
  CPPUNIT_TEST(testProcessWhenInternalReturnsFalse);
  CPPUNIT_TEST(testProcessWhenDiag199);
  CPPUNIT_TEST(testProcessWhenDiag600AndPricingFailForExc);
  CPPUNIT_TEST(testProcessWhenDiag600AndPricingPassForExc);

  CPPUNIT_TEST(testDetermineDiagnosticQualifier_none);
  CPPUNIT_TEST(testDetermineDiagnosticQualifier_194);
  CPPUNIT_TEST(testDetermineDiagnosticQualifier_233);
  CPPUNIT_TEST(testDetermineDiagnosticQualifier_333);
  CPPUNIT_TEST(testDetermineDiagnosticQualifier_688);
  CPPUNIT_TEST(testDetermineDiagnosticQualifier_689);
  CPPUNIT_TEST(testDetermineDiagnosticQualifier_MINinternal);
  CPPUNIT_TEST(testDetermineDiagnosticQualifier_MAXinternal);
  CPPUNIT_TEST(testDetermineDiagnosticQualifier_EXC);
  CPPUNIT_TEST(testDetermineDiagnosticQualifier_ALL);
  CPPUNIT_TEST(testDetermineDiagnosticQualifier_RED);
  CPPUNIT_TEST(testDetermineDiagnosticQualifier_other);
  CPPUNIT_TEST(testDetermineDiagnosticQualifier_fullRefund_689);
  CPPUNIT_TEST(testDetermineDiagnosticQualifier_fullRefund_690);

  CPPUNIT_TEST(testInternalDiagnostic_MIN);
  CPPUNIT_TEST(testInternalDiagnostic_MAX);
  CPPUNIT_TEST(testInternalDiagnostic_MINless);
  CPPUNIT_TEST(testInternalDiagnostic_MAXmore);

  CPPUNIT_TEST(testProcessNewItin);

  CPPUNIT_TEST(testIsEnforceRedirection_pass);
  CPPUNIT_TEST(testIsEnforceRedirection_fullRefund_fail);
  CPPUNIT_TEST(testIsEnforceRedirection_noSecondaryExcReqType_fail);
  CPPUNIT_TEST(testIsEnforceRedirection_notSuitableException_fail);

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

protected:
  FakeServer* _fakeServer;
  TransactionOrchestrator::RefundOrchestrator* _refundOrch;
  RefundPricingTrx* _refundTrx;
  TransactionOrchestrator* _to;
  FakeService* _itinAnalyzerServiceReturnsFalse;
  FakeService* _itinAnalyzerServiceReturnsTrue;
  FakeService* _fareCollectorReturnsFalse;
  FakeService* _fareCollectorReturnsTrue;
  FakeService* _rexFareSelectorServiceReturnsFalse;
  FakeService* _rexFareSelectorServiceReturnsTrue;
  FakeService* _internalServiceReturnsFalse;
  FakeService* _internalServiceReturnsTrue;
  FakeService* _pricingServiceReturnsFalse;
  FakeService* _pricingServiceReturnsTrue;
  FakeService* _taxServiceReturnsTrue;
  Itin* _itin;
  ExcItin* _excItin;
  TestMemHandle _memH;

public:
  FakeService* createDummyService(bool flag)
  {
    return _memH.insert(new FakeService("DUMMY_SERVICE", *_fakeServer, flag));
  }

  void setUp()
  {
    _memH.create<SpecificTestConfigInitializer>();
    _fakeServer = _memH.insert(new FakeServer);
    _refundTrx = _memH.insert(new RefundPricingTrx);

    _to = new TransactionOrchestrator("DUMMY_SERVICE", *_fakeServer);

    _refundOrch = _memH.insert(new TransactionOrchestrator::RefundOrchestrator(*_refundTrx, *_to));

    _itinAnalyzerServiceReturnsFalse = createDummyService(false);
    _itinAnalyzerServiceReturnsTrue = createDummyService(true);
    _fareCollectorReturnsFalse = createDummyService(false);
    _fareCollectorReturnsTrue = createDummyService(true);
    _rexFareSelectorServiceReturnsFalse = createDummyService(false);
    _rexFareSelectorServiceReturnsTrue = createDummyService(true);
    _internalServiceReturnsFalse = createDummyService(false);
    _internalServiceReturnsTrue = createDummyService(true);
    _pricingServiceReturnsFalse = createDummyService(false);
    _pricingServiceReturnsTrue = createDummyService(true);
    _taxServiceReturnsTrue = createDummyService(true);

    _excItin = _memH.insert(new ExcItin);
    _itin = _memH.insert(new Itin);
    _fakeServer->initializeGlobalConfigMan();
    _fakeServer->initializeGlobal();

    std::string srvName = "default";

    _to->_faresCollectionServiceName = srvName;
    _to->_faresValidationServiceName = srvName;
    _to->_pricingServiceName = srvName;
    _to->_shoppingServiceName = srvName;
    _to->_itinAnalyzerServiceName = srvName;
    _to->_taxServiceName = srvName;
    _to->_fareCalcServiceName = srvName;
    _to->_currencyServiceName = srvName;
    _to->_mileageServiceName = srvName;
    _to->_internalServiceName = srvName;
    _to->_fareDisplayServiceName = srvName;
    _to->_fareSelectorServiceName = srvName;
    _to->_rexFareSelectorServiceName = srvName;
    _refundTrx->setOptions(_memH.insert(new RexPricingOptions));
    _refundTrx->newItin().push_back(_itin);
    _refundTrx->exchangeItin().push_back(_excItin);

    _refundTrx->setAnalyzingExcItin(true);
  }

  void tearDown()
  {
    _memH.clear();
    delete _to;
  }

  void populateDiagnostic(DiagnosticTypes diag, const std::string& itinLabel = "")
  {
    _refundTrx->diagnostic().diagnosticType() = diag;
    if (!itinLabel.empty())
      _refundTrx->diagnostic().diagParamMap().insert(make_pair(Diagnostic::ITIN_TYPE, itinLabel));
  }

  void testProcessWhenItinAnalyzerReturnsFalse()
  {
    populateDiagnostic(AllFareDiagnostic, "EXC");

    _to->_itinAnalyzerService = _itinAnalyzerServiceReturnsFalse;
    _to->_faresCollectionService = _fareCollectorReturnsTrue;
    _to->_rexFareSelectorService = _rexFareSelectorServiceReturnsTrue;
    _to->_internalService = _internalServiceReturnsTrue;
    _to->_pricingService = _pricingServiceReturnsTrue;

    CPPUNIT_ASSERT(!_refundOrch->process());
    CPPUNIT_ASSERT(_excItin == _refundTrx->itin().front());
    CPPUNIT_ASSERT(_itinAnalyzerServiceReturnsFalse->_ran);
    CPPUNIT_ASSERT(!_fareCollectorReturnsTrue->_ran);
    CPPUNIT_ASSERT(!_rexFareSelectorServiceReturnsTrue->_ran);
    CPPUNIT_ASSERT(!_internalServiceReturnsTrue->_ran);
    CPPUNIT_ASSERT(!_pricingServiceReturnsTrue->_ran);
  }

  void testProcessWhenFareCollectorReturnsFalse()
  {
    populateDiagnostic(Diagnostic233);

    _to->_itinAnalyzerService = _itinAnalyzerServiceReturnsTrue;
    _to->_faresCollectionService = _fareCollectorReturnsFalse;
    _to->_rexFareSelectorService = _rexFareSelectorServiceReturnsTrue;
    _to->_internalService = _internalServiceReturnsTrue;
    _to->_pricingService = _pricingServiceReturnsTrue;

    CPPUNIT_ASSERT(!_refundOrch->process());
    CPPUNIT_ASSERT(_refundTrx->trxPhase() == RexBaseTrx::REPRICE_EXCITIN_PHASE);
    CPPUNIT_ASSERT(_excItin == _refundTrx->itin().front());
    CPPUNIT_ASSERT(_itinAnalyzerServiceReturnsTrue->_ran);
    CPPUNIT_ASSERT(_fareCollectorReturnsFalse->_ran);
    CPPUNIT_ASSERT(!_rexFareSelectorServiceReturnsTrue->_ran);
    CPPUNIT_ASSERT(!_internalServiceReturnsTrue->_ran);
    CPPUNIT_ASSERT(!_pricingServiceReturnsTrue->_ran);
  }

  void testProcessWhenRexFareSelectorReturnsFalse()
  {
    populateDiagnostic(Diagnostic233);

    _to->_itinAnalyzerService = _itinAnalyzerServiceReturnsTrue;
    _to->_faresCollectionService = _fareCollectorReturnsTrue;
    _to->_rexFareSelectorService = _rexFareSelectorServiceReturnsFalse;
    _to->_internalService = _internalServiceReturnsTrue;
    _to->_pricingService = _pricingServiceReturnsTrue;

    CPPUNIT_ASSERT(!_refundOrch->process());
    CPPUNIT_ASSERT(_refundTrx->trxPhase() == RexBaseTrx::REPRICE_EXCITIN_PHASE);
    CPPUNIT_ASSERT(_excItin == _refundTrx->itin().front());
    CPPUNIT_ASSERT(_itinAnalyzerServiceReturnsTrue->_ran);
    CPPUNIT_ASSERT(_fareCollectorReturnsTrue->_ran);
    CPPUNIT_ASSERT(_rexFareSelectorServiceReturnsFalse->_ran);
    CPPUNIT_ASSERT(!_internalServiceReturnsTrue->_ran);
    CPPUNIT_ASSERT(!_pricingServiceReturnsTrue->_ran);
  }

  void testProcessWhenInternalReturnsFalse()
  {
    populateDiagnostic(Diagnostic198);

    _to->_itinAnalyzerService = _itinAnalyzerServiceReturnsTrue;
    _to->_faresCollectionService = _fareCollectorReturnsFalse;
    _to->_rexFareSelectorService = _rexFareSelectorServiceReturnsFalse;
    _to->_internalService = _internalServiceReturnsFalse;
    _to->_pricingService = _pricingServiceReturnsTrue;

    CPPUNIT_ASSERT(!_refundOrch->process());
    CPPUNIT_ASSERT(_refundTrx->trxPhase() == RexBaseTrx::REPRICE_EXCITIN_PHASE);
    CPPUNIT_ASSERT(_itin == _refundTrx->itin().front());
    CPPUNIT_ASSERT(_itinAnalyzerServiceReturnsTrue->_ran);
    CPPUNIT_ASSERT(!_fareCollectorReturnsFalse->_ran);
    CPPUNIT_ASSERT(!_rexFareSelectorServiceReturnsFalse->_ran);
    CPPUNIT_ASSERT(_internalServiceReturnsFalse->_ran);
    CPPUNIT_ASSERT(!_pricingServiceReturnsTrue->_ran);
  }

  void testProcessWhenDiag199()
  {
    populateDiagnostic(Diagnostic199);

    _to->_itinAnalyzerService = _itinAnalyzerServiceReturnsTrue;
    _to->_faresCollectionService = _fareCollectorReturnsFalse;
    _to->_rexFareSelectorService = _rexFareSelectorServiceReturnsFalse;
    _to->_internalService = _internalServiceReturnsTrue;
    _to->_pricingService = _pricingServiceReturnsTrue;

    CPPUNIT_ASSERT(_refundOrch->process());
    CPPUNIT_ASSERT(_refundTrx->trxPhase() == RexBaseTrx::REPRICE_EXCITIN_PHASE);
    CPPUNIT_ASSERT(_itin == _refundTrx->itin().front());
    CPPUNIT_ASSERT(_itinAnalyzerServiceReturnsTrue->_ran);
    CPPUNIT_ASSERT(!_fareCollectorReturnsFalse->_ran);
    CPPUNIT_ASSERT(!_rexFareSelectorServiceReturnsFalse->_ran);
    CPPUNIT_ASSERT(_internalServiceReturnsTrue->_ran);
    CPPUNIT_ASSERT(!_pricingServiceReturnsTrue->_ran);
  }

  void testProcessWhenDiag600AndPricingFailForExc()
  {
    populateDiagnostic(Diagnostic600, "EXC");
    _refundTrx->arePenaltiesAndFCsEqualToSumFromFareCalc() = true;
    _to->_itinAnalyzerService = _itinAnalyzerServiceReturnsTrue;
    _to->_faresCollectionService = _fareCollectorReturnsTrue;
    _to->_rexFareSelectorService = _rexFareSelectorServiceReturnsTrue;
    _to->_internalService = _internalServiceReturnsFalse;
    _to->_pricingService = _pricingServiceReturnsFalse;
    _to->_taxService = _taxServiceReturnsTrue;

    CPPUNIT_ASSERT(!_refundOrch->process());
    CPPUNIT_ASSERT(_refundTrx->trxPhase() == RexBaseTrx::REPRICE_EXCITIN_PHASE);
    CPPUNIT_ASSERT(_excItin == _refundTrx->itin().front());
    CPPUNIT_ASSERT(_itinAnalyzerServiceReturnsTrue->_ran);
    CPPUNIT_ASSERT(_fareCollectorReturnsTrue->_ran);
    CPPUNIT_ASSERT(_rexFareSelectorServiceReturnsTrue->_ran);
    CPPUNIT_ASSERT(!_internalServiceReturnsFalse->_ran);
    CPPUNIT_ASSERT(_pricingServiceReturnsFalse->_ran);
  }

  void testProcessWhenDiag600AndPricingPassForExc()
  {
    populateDiagnostic(Diagnostic600, "EXC");

    _refundTrx->arePenaltiesAndFCsEqualToSumFromFareCalc() = true;
    _to->_itinAnalyzerService = _itinAnalyzerServiceReturnsTrue;
    _to->_faresCollectionService = _fareCollectorReturnsTrue;
    _to->_rexFareSelectorService = _rexFareSelectorServiceReturnsTrue;
    _to->_internalService = _internalServiceReturnsFalse;
    _to->_pricingService = _pricingServiceReturnsTrue;
    _to->_taxService = _taxServiceReturnsTrue;

    CPPUNIT_ASSERT(_refundOrch->process());
    CPPUNIT_ASSERT(_refundTrx->trxPhase() == RexBaseTrx::MATCH_EXC_RULE_PHASE);
    CPPUNIT_ASSERT(_excItin == _refundTrx->itin().front());
    CPPUNIT_ASSERT(_itinAnalyzerServiceReturnsTrue->_ran);
    CPPUNIT_ASSERT(_fareCollectorReturnsTrue->_ran);
    CPPUNIT_ASSERT(_rexFareSelectorServiceReturnsTrue->_ran);
    CPPUNIT_ASSERT(!_internalServiceReturnsFalse->_ran);
    CPPUNIT_ASSERT(_pricingServiceReturnsTrue->_ran);
  }

  void testDetermineDiagnosticQualifier_none()
  {
    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_NONE,
                         _refundOrch->determineDiagnosticQualifier());
  }

  void testDetermineDiagnosticQualifier_194()
  {
    populateDiagnostic(Diagnostic194);

    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _refundOrch->determineDiagnosticQualifier());
  }
  void testDetermineDiagnosticQualifier_233()
  {
    populateDiagnostic(Diagnostic233);
    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _refundOrch->determineDiagnosticQualifier());
  }

  void testDetermineDiagnosticQualifier_333()
  {
    populateDiagnostic(Diagnostic333);
    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _refundOrch->determineDiagnosticQualifier());
  }

  void testDetermineDiagnosticQualifier_688()
  {
    populateDiagnostic(Diagnostic688);
    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _refundOrch->determineDiagnosticQualifier());
  }

  void testDetermineDiagnosticQualifier_689()
  {
    populateDiagnostic(Diagnostic689);
    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITNEW,
                         _refundOrch->determineDiagnosticQualifier());
  }

  void testDetermineDiagnosticQualifier_MINinternal()
  {
    populateDiagnostic(Diagnostic150);
    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _refundOrch->determineDiagnosticQualifier());
  }

  void testDetermineDiagnosticQualifier_MAXinternal()
  {
    populateDiagnostic(Diagnostic199);
    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _refundOrch->determineDiagnosticQualifier());
  }

  void testDetermineDiagnosticQualifier_EXC()
  {
    populateDiagnostic(Diagnostic663, "EXC");
    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _refundOrch->determineDiagnosticQualifier());
  }

  void testDetermineDiagnosticQualifier_ALL()
  {
    populateDiagnostic(Diagnostic663, "ALL");
    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITALL,
                         _refundOrch->determineDiagnosticQualifier());
  }

  void testDetermineDiagnosticQualifier_RED()
  {
    populateDiagnostic(Diagnostic690, "RED");
    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEFT,
                         _refundOrch->determineDiagnosticQualifier());
  }

  void testDetermineDiagnosticQualifier_other()
  {
    populateDiagnostic(Diagnostic663);
    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITNEW,
                         _refundOrch->determineDiagnosticQualifier());
  }

  void testDetermineDiagnosticQualifier_fullRefund_689()
  {
    populateDiagnostic(Diagnostic689);
    _refundTrx->fullRefund() = true;
    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _refundOrch->determineDiagnosticQualifier());
  }

  void testDetermineDiagnosticQualifier_fullRefund_690()
  {
    populateDiagnostic(Diagnostic690);
    _refundTrx->fullRefund() = true;
    CPPUNIT_ASSERT_EQUAL(TransactionOrchestrator::DQ_ITEXC,
                         _refundOrch->determineDiagnosticQualifier());
  }

  void testInternalDiagnostic_MIN()
  {
    populateDiagnostic(Diagnostic150);
    CPPUNIT_ASSERT(_refundOrch->internalDiagnostic());
  }

  void testInternalDiagnostic_MAX()
  {
    populateDiagnostic(Diagnostic199);
    CPPUNIT_ASSERT(_refundOrch->internalDiagnostic());
  }

  void testInternalDiagnostic_MINless()
  {
    populateDiagnostic(Diagnostic85);
    CPPUNIT_ASSERT(!_refundOrch->internalDiagnostic());
  }

  void testInternalDiagnostic_MAXmore()
  {
    populateDiagnostic(AllFareDiagnostic);
    CPPUNIT_ASSERT(!_refundOrch->internalDiagnostic());
  }

  void testProcessNewItin()
  {
    RefundPricingRequest request;
    _refundTrx->setRequest(&request);
    CPPUNIT_ASSERT(_refundOrch->processNewItin(0));
    CPPUNIT_ASSERT_EQUAL('Y', _refundTrx->getRequest()->lowFareRequested());
    CPPUNIT_ASSERT_EQUAL(RexBaseTrx::PRICE_NEWITIN_PHASE, _refundTrx->trxPhase());
    CPPUNIT_ASSERT(!_refundTrx->isAnalyzingExcItin());
  }

  void testIsEnforceRedirection_pass()
  {
    _refundTrx->fullRefund() = false;
    _refundTrx->secondaryExcReqType() = "AM";
    ErrorResponseException e(ErrorResponseException::REFUND_RULES_FAIL);
    CPPUNIT_ASSERT(_refundOrch->isEnforceRedirection(e));
  }

  void testIsEnforceRedirection_fullRefund_fail()
  {
    _refundTrx->fullRefund() = true;
    _refundTrx->secondaryExcReqType() = "AM";
    ErrorResponseException e(ErrorResponseException::UNABLE_TO_MATCH_REFUND_RULES);
    CPPUNIT_ASSERT(!_refundOrch->isEnforceRedirection(e));
  }

  void testIsEnforceRedirection_noSecondaryExcReqType_fail()
  {
    _refundTrx->fullRefund() = false;
    _refundTrx->secondaryExcReqType() = "";
    ErrorResponseException e(ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    CPPUNIT_ASSERT(!_refundOrch->isEnforceRedirection(e));
  }

  void testIsEnforceRedirection_notSuitableException_fail()
  {
    _refundTrx->fullRefund() = false;
    _refundTrx->secondaryExcReqType() = "";
    ErrorResponseException e(ErrorResponseException::UNKNOWN_EXCEPTION);
    CPPUNIT_ASSERT(!_refundOrch->isEnforceRedirection(e));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RefundOrchestratorTest);
}
