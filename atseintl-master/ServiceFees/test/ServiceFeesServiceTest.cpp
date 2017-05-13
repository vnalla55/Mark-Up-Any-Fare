#include "test/include/CppUnitHelperMacros.h"

#include "Common/Config/ConfigMan.h"
#include "Common/MetricsMan.h"
#include "Common/MetricsUtil.h"
#include "Common/Thread/PriorityQueueTimerTaskExecutor.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AncRequest.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/Customer.h"
#include "Diagnostic/Diag878Collector.h"
#include "Diagnostic/Diag879Collector.h"
#include "ServiceFees/ServiceFeesService.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockTseServer.h"
#include "test/include/MockGlobal.h"

namespace tse
{
namespace
{
const std::string nameSvc = "SERVICE_FEES_SVC";

class ServiceFeesServiceMock : public ServiceFeesService
{
public:
  ServiceFeesServiceMock(TseServer& server) : ServiceFeesService(nameSvc, server) {}

  void processOCFee(OptionalFeeCollector& ocCollector) const {}
  void processAncillaryFee(OptionalFeeCollector& ocCollector) const {}
};
}

class ServiceFeesServiceTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ServiceFeesServiceTest);
  CPPUNIT_TEST(testProcess_Metrics);
  CPPUNIT_TEST(testProcess_ExchangePricing);

  CPPUNIT_TEST(testProcess_Pricing_Skip);
  CPPUNIT_TEST(testProcess_Pricing_SkipWithDiag878);
  CPPUNIT_TEST(testProcess_Pricing_SkipWithDiag879);
  CPPUNIT_TEST(testProcess_Pricing_NotSkip);
  CPPUNIT_TEST(testProcess_Pricing_NotSkipWithDiag878);
  CPPUNIT_TEST(testProcess_Pricing_NotSkipWithDiag879);

  CPPUNIT_TEST(testCollectOCFeesReturnFalseForFareX);
  CPPUNIT_TEST(testCollectOCFeesReturnTrueForInfiniUser);
  CPPUNIT_TEST(testCollectOCFeesReturnFalseForExchange);
  CPPUNIT_TEST(testCollectOCFeesReturnFalseWhenNotMIPOrPricingTrx);
  CPPUNIT_TEST(testCollectOCFeesReturnFalseForWPA);
  CPPUNIT_TEST(testCollectOCFeesReturnFalseForNoPnr);
  CPPUNIT_TEST(testCollectOCFeesReturnTrueForAbacusUser);
  CPPUNIT_TEST(testCollectOCFeesReturnTrueForAxessUser);
  CPPUNIT_TEST(testCollectOCFeesReturnFalseForCarrierPartition);
  CPPUNIT_TEST(testCollectOCFeesReturnTrueForPricingTrx);
  CPPUNIT_TEST(testCollectOCFeesReturnFalseForTicketingRequest);
  CPPUNIT_TEST(testCheckTimeOutForNotPricingTrx);
  CPPUNIT_TEST(testCheckTimeOutForPricingTrx_NoTimeOut);
  CPPUNIT_TEST(testCheckTimeOutForWP_NoTimeOut);
  CPPUNIT_TEST(testCheckTimeOutForWPEqualLess2secs_TimeOut);
  CPPUNIT_TEST(testCheckTimeOutForWPAE_EqualLess2secs_TimeOut);
  CPPUNIT_TEST(testCheckTimeOutForWPAE_NoTimeOut);
  CPPUNIT_TEST(testAdjustAncillaryTimeOut_NoTimeOut);
  CPPUNIT_TEST(testAdjustAncillaryTimeOut_TimeOutExists);

  CPPUNIT_TEST(testProcess_AncillaryPricing_M70);
  CPPUNIT_TEST(testProcess_AncillaryPricing_WP_Display_AE);
  CPPUNIT_TEST(testProcess_AncillaryPricing_R7);
  CPPUNIT_TEST(testDiagActiveOrNot_Diag875);
  CPPUNIT_TEST(testDiagActiveOrNot_Diag199);

  CPPUNIT_TEST(testProcess_AltDates);
  CPPUNIT_TEST(testProcess_Other);
  CPPUNIT_TEST(testCheckTimeOutForShopping);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _myServer = _memHandle.create<MockTseServer>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _request->collectOCFees() = 'T';
    _request->validatingCarrier() = "AA";
    const DateTime tktDate = DateTime(2009, 3, 23, 8, 15, 0);
    _request->ticketingDT() = tktDate;
    _trx->setRequest(_request);
    Customer* customer = _memHandle.create<Customer>();
    customer->doNotApplyObTktFees() = NO;
    Agent* agent = _memHandle.create<Agent>();
    agent->agentTJR() = customer;
    _request->ticketingAgent() = agent;
    _options = _memHandle.create<PricingOptions>();
    _trx->setOptions(_options);
    _itin = _memHandle.create<Itin>();
    _trx->itin().push_back(_itin);

    _trx->billing() = _memHandle.create<Billing>();

    _sfs = _memHandle.insert(new ServiceFeesServiceMock(*_myServer));

    tse::MetricsMan* metricsMan = _memHandle.create<tse::MetricsMan>();
    metricsMan->initialize(tse::MetricsUtil::MAX_METRICS_ENUM);
    MockGlobal::setMetricsMan(metricsMan);
    TimerTaskExecutor::setInstance(new PriorityQueueTimerTaskExecutor);
  }

  void tearDown()
  {
    _memHandle.clear();
    MockGlobal::clear();
    TimerTaskExecutor::destroyInstance();
  }

protected:
  PricingTrx* _trx;
  PricingRequest* _request;
  ServiceFeesService* _sfs;
  TestMemHandle _memHandle;
  TseServer* _myServer;
  Itin* _itin;
  PricingOptions* _options;

  void createDiag(DiagnosticTypes diagType)
  {
    _trx->diagnostic().diagnosticType() = diagType;
    if (diagType != DiagnosticNone)
    {
      _trx->diagnostic().activate();
    }
  }

  // TESTS
public:
  void testProcess_Metrics()
  {
    MetricsTrx metricsTrx;
    CPPUNIT_ASSERT(_sfs->process(metricsTrx));
  }

  void testProcess_ExchangePricing()
  {
    ExchangePricingTrx excTrx;
    CPPUNIT_ASSERT(_sfs->process(excTrx));
  }

  void testProcess_Pricing_Skip()
  {
    _request->collectOCFees() = 'F';
    CPPUNIT_ASSERT(_sfs->process(*_trx));
  }

  void testProcess_Pricing_SkipWithDiag878()
  {
    _request->collectOCFees() = 'F';
    createDiag(Diagnostic878);

    _sfs->process(*_trx);
    CPPUNIT_ASSERT(_trx->diagnostic().toString().find("878 : SERVICE FEES - OC FEES SORTING") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_trx->diagnostic().toString().find(
                       "OC FEES COLLECTOR SKIPPED, REASON: \nFEES NOT REQUESTED") !=
                   std::string::npos);
  }

  void testProcess_Pricing_SkipWithDiag879()
  {
    _request->collectOCFees() = 'F';
    createDiag(Diagnostic879);
    _sfs->process(*_trx);
    CPPUNIT_ASSERT(_trx->diagnostic().toString().empty());
  }

  void testProcess_Pricing_NotSkip()
  {
    _request->collectOCFees() = 'T';
    CPPUNIT_ASSERT(_sfs->process(*_trx));
  }

  void testProcess_Pricing_NotSkipWithDiag878()
  {
    _request->collectOCFees() = 'T';
    createDiag(Diagnostic878);
    _sfs->process(*_trx);
    CPPUNIT_ASSERT(_trx->diagnostic().toString().find("878 : SERVICE FEES - OC FEES SORTING") !=
                   std::string::npos);
    CPPUNIT_ASSERT_EQUAL(std::string::npos,
                         _trx->diagnostic().toString().find(
                             "OC FEES COLLECTOR SKIPPED, REASON: \nFEES NOT REQUESTED"));
  }

  void testProcess_Pricing_NotSkipWithDiag879()
  {
    _request->collectOCFees() = 'T';
    createDiag(Diagnostic879);
    _sfs->process(*_trx);
    CPPUNIT_ASSERT(_trx->diagnostic().toString().find(
                       "879 : SERVICE FEES - OC FEES CURRENCY CONVERSION") != std::string::npos);
  }

  void testCollectOCFeesReturnFalseForFareX()
  {
    _trx->getOptions()->fareX() = true;
    CPPUNIT_ASSERT(!_sfs->collectOCFee(*_trx));
  }

  void testCollectOCFeesReturnTrueForInfiniUser()
  {
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = INFINI_MULTIHOST_ID;
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = INFINI_USER;
    CPPUNIT_ASSERT(_sfs->collectOCFee(*_trx));
  }

  void testCollectOCFeesReturnFalseForExchange()
  {
    ExchangePricingTrx excTrx;
    excTrx.setRequest(_request);
    CPPUNIT_ASSERT(!_sfs->collectOCFee(excTrx));
  }

  void testCollectOCFeesReturnFalseWhenNotMIPOrPricingTrx()
  {
    _trx->setTrxType(PricingTrx::IS_TRX);
    CPPUNIT_ASSERT(!_sfs->collectOCFee(*_trx));
  }

  void testCollectOCFeesReturnFalseForWPA()
  {
    AltPricingTrx wpaTrx;
    wpaTrx.setRequest(_request);
    CPPUNIT_ASSERT(!_sfs->collectOCFee(wpaTrx));
  }

  void testCollectOCFeesReturnFalseForNoPnr()
  {
    NoPNRPricingTrx noPnrTrx;
    noPnrTrx.setRequest(_request);
    CPPUNIT_ASSERT(!_sfs->collectOCFee(noPnrTrx));
  }

  void testCollectOCFeesReturnTrueForAbacusUser()
  {
    TrxUtil::enableAbacus();
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = ABACUS_MULTIHOST_ID;
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = ABACUS_USER;
    CPPUNIT_ASSERT(_sfs->collectOCFee(*_trx));
  }

  void testCollectOCFeesReturnTrueForAxessUser()
  {
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = AXESS_USER;
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = AXESS_MULTIHOST_ID;
    CPPUNIT_ASSERT(_sfs->collectOCFee(*_trx));
  }

  void testCollectOCFeesReturnFalseForCarrierPartition()
  {
    _trx->getRequest()->ticketingAgent()->agentTJR() = 0;
    _trx->billing()->partitionID() = "F9";
    _trx->billing()->aaaCity() = "DFW";
    CPPUNIT_ASSERT(!_sfs->collectOCFee(*_trx));
  }

  void testCollectOCFeesReturnFalseForTicketingRequest()
  {
    _trx->getRequest()->ticketEntry() = 'T';
    CPPUNIT_ASSERT(!_sfs->collectOCFee(*_trx));
  }

  void testCollectOCFeesReturnTrueForPricingTrx() { CPPUNIT_ASSERT(_sfs->collectOCFee(*_trx)); }

  void testCheckTimeOutForNotPricingTrx()
  {
    _trx->setTrxType(PricingTrx::MIP_TRX);
    CPPUNIT_ASSERT(_sfs->checkTimeOut(*_trx));
  }

  void testCheckTimeOutForPricingTrx_NoTimeOut()
  {
    _trx->setTrxType(PricingTrx::MIP_TRX);
    CPPUNIT_ASSERT(_sfs->checkTimeOut(*_trx));
    CPPUNIT_ASSERT(!_trx->aborter());
  }

  void testCheckTimeOutForWP_NoTimeOut()
  {
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    std::string cfgTime = "100";
    TestConfigInitializer::setValue("TRX_TIMEOUT", cfgTime, "TSE_SERVER");
    cfgTime = "2";
    TestConfigInitializer::setValue("TIME_TO_COMPLETE_TRANS", cfgTime, "SERVICE_FEES_SVC");
    TrxUtil::createTrxAborter(*_trx);

    CPPUNIT_ASSERT(_sfs->checkTimeOut(*_trx));
    TrxAborter* aborter = _trx->aborter();

    CPPUNIT_ASSERT_EQUAL((int)100, aborter->timeout());

    int timeToCompleteTrans = 0;
    Global::config().getValue("TIME_TO_COMPLETE_TRANS", timeToCompleteTrans, "SERVICE_FEES_SVC");
    int percentTime = ((aborter->getTimeOutAt() - time(0) - timeToCompleteTrans) * 10) / 100;

    CPPUNIT_ASSERT(aborter->getHurryAt() < (time(0) + percentTime));
    CPPUNIT_ASSERT(aborter->getHurryAt() < aborter->getTimeOutAt());
  }

  void testCheckTimeOutForWPEqualLess2secs_TimeOut()
  {
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    std::string cfgTime = "2";
    TestConfigInitializer::setValue("TRX_TIMEOUT", cfgTime, "TSE_SERVER");
    TrxUtil::createTrxAborter(*_trx);
    TestConfigInitializer::setValue("TIME_TO_COMPLETE_TRANS", cfgTime, "SERVICE_FEES_SVC");

    CPPUNIT_ASSERT(!_sfs->checkTimeOut(*_trx));
    TrxAborter* aborter = _trx->aborter();

    CPPUNIT_ASSERT_EQUAL((int)2, aborter->timeout());
    CPPUNIT_ASSERT(!_trx->itin().front()->timeOutForExceeded());
  }

  void testCheckTimeOutForWPAE_EqualLess2secs_TimeOut()
  {
    _itin->timeOutOCFForWP() = false;
    _itin->timeOutForExceeded() = false;
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    _trx->getOptions()->isProcessAllGroups() = true;
    std::string cfgTime = "2";
    TestConfigInitializer::setValue("TRX_TIMEOUT", cfgTime, "TSE_SERVER");
    TrxUtil::createTrxAborter(*_trx);
    TestConfigInitializer::setValue("TIME_TO_COMPLETE_TRANS", cfgTime, "SERVICE_FEES_SVC");

    CPPUNIT_ASSERT(!_sfs->checkTimeOut(*_trx));
    TrxAborter* aborter = _trx->aborter();

    CPPUNIT_ASSERT_EQUAL((int)2, aborter->timeout());
    CPPUNIT_ASSERT(_trx->itin().front()->timeOutForExceeded());
    CPPUNIT_ASSERT(!_trx->itin().front()->timeOutOCFForWP());
  }

  void testCheckTimeOutForWPAE_NoTimeOut()
  {
    _itin->timeOutOCFForWP() = false;
    _itin->timeOutForExceeded() = false;
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    _trx->getOptions()->isProcessAllGroups() = true;
    std::string cfgTime = "100";
    TestConfigInitializer::setValue("TRX_TIMEOUT", cfgTime, "TSE_SERVER");
    TrxUtil::createTrxAborter(*_trx);
    cfgTime = "2";
    TestConfigInitializer::setValue("TIME_TO_COMPLETE_TRANS", cfgTime, "SERVICE_FEES_SVC");

    CPPUNIT_ASSERT(_sfs->checkTimeOut(*_trx));
    TrxAborter* aborter = _trx->aborter();

    CPPUNIT_ASSERT_EQUAL((int)100, aborter->timeout());
    CPPUNIT_ASSERT(aborter->getHurryAt() < aborter->getTimeOutAt());
    CPPUNIT_ASSERT(!_trx->itin().front()->timeOutForExceeded());
    CPPUNIT_ASSERT(!_trx->itin().front()->timeOutOCFForWP());
  }

  void testAdjustAncillaryTimeOut_NoTimeOut()
  {
    AncillaryPricingTrx& trx = static_cast<AncillaryPricingTrx&>(*_trx);
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    _sfs->adjustTimeOut(trx);
    CPPUNIT_ASSERT(!trx.aborter());
  }

  void testAdjustAncillaryTimeOut_TimeOutExists()
  {
    AncillaryPricingTrx& trx = static_cast<AncillaryPricingTrx&>(*_trx);
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    AncRequest req;
    req.ancRequestType() = AncRequest::M70Request;
    trx.setRequest(&req);
    std::string cfgTime = "10";
    TestConfigInitializer::setValue("TRX_TIMEOUT", cfgTime, "TSE_SERVER");
    cfgTime = "20";
    TestConfigInitializer::setValue("ANCILLARY_TIMEOUT", cfgTime, "SERVICE_FEES_SVC");
    TrxUtil::createTrxAborter(*_trx);
    TrxAborter* aborter = _trx->aborter();

    CPPUNIT_ASSERT_EQUAL((int)10, aborter->timeout());

    _sfs->adjustTimeOut(trx);

    CPPUNIT_ASSERT_EQUAL((int)20, aborter->timeout());
  }

  void testDiagActiveOrNot_Diag875()
  {
    createDiag(Diagnostic875);
    CPPUNIT_ASSERT(_sfs->diagActive(*_trx));
  }

  void testDiagActiveOrNot_Diag199()
  {
    createDiag(Diagnostic199);
    CPPUNIT_ASSERT(!_sfs->diagActive(*_trx));
  }

  void testProcess_AncillaryPricing_M70()
  {
    AncRequest req;
    req.ancRequestType() = AncRequest::M70Request;
    AncillaryPricingTrx ancTrx;
    ancTrx.setRequest(&req);
    CPPUNIT_ASSERT(_sfs->process(ancTrx));
  }

  void testProcess_AncillaryPricing_WP_Display_AE()
  {
    AncRequest req;
    req.ancRequestType() = AncRequest::WPAERequest;
    AncillaryPricingTrx ancTrx;
    ancTrx.setRequest(&req);
    CPPUNIT_ASSERT(_sfs->process(ancTrx));
  }

  void testProcess_AncillaryPricing_R7()
  {
    AncRequest req;
    req.ancRequestType() = AncRequest::PostTktRequest;
    AncillaryPricingTrx ancTrx;
    ancTrx.setRequest(&req);
    CPPUNIT_ASSERT(_sfs->process(ancTrx));
  }

  void testProcess_AltDates()
  {
    _trx->setAltDates(true);
    createDiag(Diagnostic878);

    _sfs->process(*_trx);
    CPPUNIT_ASSERT(_trx->diagnostic().toString().find(
                       "OC FEES COLLECTOR SKIPPED, REASON: \nALTERNATE DATES REQUEST") !=
                   std::string::npos);
  }

  void testProcess_Other()
  {
    _trx->setTrxType(PricingTrx::REPRICING_TRX);
    createDiag(Diagnostic878);
    _sfs->process(*_trx);
    CPPUNIT_ASSERT(_trx->diagnostic().toString().find(
                       "OC FEES COLLECTOR SKIPPED, REASON: \nOTHER") != std::string::npos);
  }

  void testCheckTimeOutForShopping()
  {
    _trx->setTrxType(PricingTrx::MIP_TRX);
    std::string cfgTime = "2";
    TestConfigInitializer::setValue("TRX_TIMEOUT", cfgTime, "TSE_SERVER");
    TrxUtil::createTrxAborter(*_trx);
    TestConfigInitializer::setValue("TIME_TO_COMPLETE_TRANS_SHP", cfgTime, "SERVICE_FEES_SVC");

    CPPUNIT_ASSERT(_sfs->checkTimeOut(*_trx));
    TrxAborter* aborter = _trx->aborter();

    CPPUNIT_ASSERT_EQUAL((int)2, aborter->timeout());
    CPPUNIT_ASSERT(!_trx->itin().front()->timeOutForExceeded());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(ServiceFeesServiceTest);
}
