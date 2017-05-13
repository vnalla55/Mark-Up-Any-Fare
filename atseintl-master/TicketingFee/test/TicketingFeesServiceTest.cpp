#include "Common/Code.h"
#include "Common/Config/ConfigMan.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseConsts.h"
#include "Common/TypeConvert.h"
#include "Common/MetricsMan.h"
#include "Common/MetricsUtil.h"
#include "Common/Thread/PriorityQueueTimerTaskExecutor.h"
#include "Common/TrxUtil.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareCalcConfig.h"
#include "DataModel/Agent.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/TrxAborter.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/TktFeesRequest.h"
#include "TicketingFee/PseudoObjectsBuilder.h"
#include "TicketingFee/TicketingFeesService.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <map>
#include <string>

namespace tse
{
const std::string nameSvc = "TICKETING_FEES_SVC";
class TicketingFeesServiceStub : public TicketingFeesService
{
public:
  TicketingFeesServiceStub(TseServer& server) : TicketingFeesService(nameSvc, server) {}

  ~TicketingFeesServiceStub() {}
};

class TicketingFeesServiceTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TicketingFeesServiceTest);
  CPPUNIT_TEST(testPrintOBNotRequested);
  CPPUNIT_TEST(testSetFOPBinNumberNotEmpty);
  CPPUNIT_TEST(testSetFOPBinNumberEmpty);
  CPPUNIT_TEST(testSetFOPBinNumberThrowNoPtp);
  CPPUNIT_TEST(testSetDataInRequest);
  CPPUNIT_TEST(testProcess_MetrixTrx);
  CPPUNIT_TEST(testProcessOB_Not_Requested);
  CPPUNIT_TEST(testProcessOB_Not_Requested_Diag);
  CPPUNIT_TEST(testProcess_Not_Requested);
  CPPUNIT_TEST(testProcess_Not_Requested_Diag);
  CPPUNIT_TEST(testSetTimeOut);

  CPPUNIT_TEST(testCollectTicktingFeesReturnFalseForExchange);
  CPPUNIT_TEST(testCollectTicktingFeesReturnFalseWhenNotMIPOrPricingTrx);
  CPPUNIT_TEST(testCollectTicktingFeesReturnFalseForNoPnr);
  CPPUNIT_TEST(testCollectTicktingFeesReturnFalseForAxessUser);
  CPPUNIT_TEST(testCollectTicktingFeesReturnFalseForCarrierPartition);
  CPPUNIT_TEST(testCollectTicktingFeesReturnTrueForPricingTrx);
  CPPUNIT_TEST(testProcessTicketingFeesReturnTrueWhenNoDefaultValidatingCxr);

  CPPUNIT_TEST_SUITE_END();

public:
  TicketingFeesServiceTest() {}

  void setUp()
  {
    _myServer = _memHandle.create<MockTseServer>();
    _tfpTrx = _memHandle.insert(new TktFeesPricingTrx);
    _itin = _memHandle.insert(new Itin);
    _tktFeesReq = _memHandle.insert(new TktFeesRequest);
    _tfpTrx->setRequest(_tktFeesReq);
    _tfpTrx->itin().push_back(_itin);

    _agent = _memHandle.insert(new Agent);
    _billing = _memHandle.insert(new Billing);
    _tktFeesReq->ticketingAgent() = _agent;
    _customer = _memHandle.insert(new Customer);
    _agent->agentTJR() = _customer;
    _tfpTrx->billing() = _billing;

    _memHandle.get(_ptp);
    _memHandle.get(_ppi);
    _memHandle.get(_fop);
    _tfs = _memHandle.insert(new TicketingFeesServiceStub(*_myServer));

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

  void createDiag(DiagnosticTypes diagType)
  {
    _tfpTrx->diagnostic().diagnosticType() = diagType;
    if (diagType != DiagnosticNone)
    {
      _tfpTrx->diagnostic().activate();
    }
  }

  void testPrintOBNotRequested()
  {
    TicketingFeesServiceStub builder(*_myServer);
    createDiag(Diagnostic870);

    builder.printOBNotRequested(*_tfpTrx);
    CPPUNIT_ASSERT_EQUAL(
        std::string("******************* SERVICE FEE DIAGNOSTIC ********************\n"
                    "*** NOT PROCESSED - OB FEES NOT REQUESTED  \n"),
        _tfpTrx->diagnostic().toString());
  }

  void testSetFOPBinNumberNotEmpty()
  {
    TicketingFeesServiceStub builder(*_myServer);

    _fop->fopBinNumber() = "123456";
    _ppi->fopVector().push_back(_fop);
    _ptp->ppiV().push_back(_ppi);

    TktFeesRequest* req = static_cast<TktFeesRequest*>(_tfpTrx->getRequest());
    req->paxTypePaymentPerItin()[_itin] = _ptp;

    CPPUNIT_ASSERT_NO_THROW(builder.setFOPBinNumber(*_tfpTrx, _itin));
    CPPUNIT_ASSERT_EQUAL(FopBinNumber("123456"), _tfpTrx->getRequest()->formOfPayment());
  }
  void testSetFOPBinNumberEmpty()
  {
    TicketingFeesServiceStub builder(*_myServer);

    _fop->fopBinNumber() = "";
    _ppi->fopVector().push_back(_fop);
    _ptp->ppiV().push_back(_ppi);

    TktFeesRequest* req = static_cast<TktFeesRequest*>(_tfpTrx->getRequest());
    req->paxTypePaymentPerItin()[_itin] = _ptp;

    CPPUNIT_ASSERT_NO_THROW(builder.setFOPBinNumber(*_tfpTrx, _itin));
    CPPUNIT_ASSERT_EQUAL(FopBinNumber(""), _tfpTrx->getRequest()->formOfPayment());
  }

  void testSetFOPBinNumberThrowNoPtp()
  {
    TicketingFeesServiceStub builder(*_myServer);
    CPPUNIT_ASSERT_THROW(builder.setFOPBinNumber(*_tfpTrx, _itin), ErrorResponseException);
  }

  void testSetDataInRequest()
  {
    TicketingFeesServiceStub builder(*_myServer);
    dynamic_cast<TktFeesRequest*>(_tfpTrx->getRequest())->tktDesignatorPerItin()[_itin].insert(
        std::pair<int16_t, TktDesignator>(0, "TKTDSG1"));
    dynamic_cast<TktFeesRequest*>(_tfpTrx->getRequest())->corpIdPerItin()[_itin].push_back("CRP01");
    dynamic_cast<TktFeesRequest*>(_tfpTrx->getRequest())->accountCodeIdPerItin()[_itin].push_back(
        "ACC11");
    _fop->fopBinNumber() = "";
    _ppi->fopVector().push_back(_fop);
    _ptp->ppiV().push_back(_ppi);
    _ptp->tktOverridePoint() = "DFW";
    TktFeesRequest* req = static_cast<TktFeesRequest*>(_tfpTrx->getRequest());
    req->paxTypePaymentPerItin()[_itin] = _ptp;

    CPPUNIT_ASSERT_NO_THROW(builder.setDataInRequest(*_tfpTrx, _itin));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _tfpTrx->getRequest()->tktDesignator().size());
    CPPUNIT_ASSERT_EQUAL(TktDesignator("TKTDSG1"), _tfpTrx->getRequest()->tktDesignator()[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(1), _tfpTrx->getRequest()->corpIdVec().size());
    CPPUNIT_ASSERT_EQUAL(std::string("CRP01"), _tfpTrx->getRequest()->corpIdVec()[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(1), _tfpTrx->getRequest()->accCodeVec().size());
    CPPUNIT_ASSERT_EQUAL(std::string("ACC11"), _tfpTrx->getRequest()->accCodeVec()[0]);
    CPPUNIT_ASSERT(_tfpTrx->getRequest()->isMultiAccCorpId());
    CPPUNIT_ASSERT_EQUAL(FopBinNumber(""), _tfpTrx->getRequest()->formOfPayment());
    CPPUNIT_ASSERT_EQUAL(LocCode("DFW"), _tfpTrx->getRequest()->ticketPointOverride());
  }

  void testProcessOB_Not_Requested()
  {
    TicketingFeesServiceStub builder(*_myServer);
    _tktFeesReq->collectOBFee() = 'N';
    CPPUNIT_ASSERT(builder.process(*_tfpTrx));
  }

  void testProcessOB_Not_Requested_Diag()
  {
    TicketingFeesServiceStub builder(*_myServer);
    _tktFeesReq->collectOBFee() = 'N';
    createDiag(Diagnostic870);
    CPPUNIT_ASSERT(builder.process(*_tfpTrx));
    CPPUNIT_ASSERT_EQUAL(
        std::string("******************* SERVICE FEE DIAGNOSTIC ********************\n"
                    "*** NOT PROCESSED - OB FEES NOT REQUESTED  \n"),
        _tfpTrx->diagnostic().toString());
  }

  void testProcess_Not_Requested()
  {
    TicketingFeesServiceStub builder(*_myServer);
    _tktFeesReq->collectOBFee() = 'N';
    CPPUNIT_ASSERT(builder.process(*_tfpTrx));
  }

  void testProcess_Not_Requested_Diag()
  {
    TicketingFeesServiceStub builder(*_myServer);
    _tktFeesReq->collectOBFee() = 'N';
    createDiag(Diagnostic870);
    CPPUNIT_ASSERT(builder.process(*_tfpTrx));
    CPPUNIT_ASSERT_EQUAL(
        std::string("******************* SERVICE FEE DIAGNOSTIC ********************\n"
                    "*** NOT PROCESSED - OB FEES NOT REQUESTED  \n"),
        _tfpTrx->diagnostic().toString());
  }

  void testSetTimeOut()
  {
    TicketingFeesServiceStub builder(*_myServer);
    std::string cfgTime = "60";
    std::string cfgTimeOB = "6";
    TestConfigInitializer::setValue("TRX_TIMEOUT", cfgTime, "TSE_SERVER");
    TestConfigInitializer::setValue("OB_FEES_TIMEOUT", cfgTimeOB, "TICKETING_FEES_SVC");

    TrxUtil::createTrxAborter(*_tfpTrx);
    TrxAborter* aborter = _tfpTrx->aborter();

    CPPUNIT_ASSERT_EQUAL((int)60, aborter->timeout());
    builder.setTimeOut(*_tfpTrx);
    CPPUNIT_ASSERT_EQUAL((int)6, aborter->timeout());
  }

  void testProcess_MetrixTrx()
  {
    MetricsTrx metricsTrx;
    TicketingFeesServiceStub builder(*_myServer);
    CPPUNIT_ASSERT(builder.process(metricsTrx));
  }

  void testCollectTicktingFeesReturnFalseForExchange()
  {
    TicketingFeesServiceStub builder(*_myServer);
    ExchangePricingTrx excTrx;
    excTrx.setRequest(_tktFeesReq);
    CPPUNIT_ASSERT(!builder.collectTicketingFees(excTrx));
  }

  void testCollectTicktingFeesReturnFalseWhenNotMIPOrPricingTrx()
  {
    TicketingFeesServiceStub builder(*_myServer);
    _tfpTrx->setTrxType(PricingTrx::IS_TRX);
    CPPUNIT_ASSERT(!builder.collectTicketingFees(*_tfpTrx));
  }

  void testCollectTicktingFeesReturnFalseForNoPnr()
  {
    TicketingFeesServiceStub builder(*_myServer);
    NoPNRPricingTrx noPnrTrx;
    noPnrTrx.noPNRPricing() = true;
    noPnrTrx.setRequest(_tktFeesReq);
    CPPUNIT_ASSERT(!builder.collectTicketingFees(noPnrTrx));
  }

  void testCollectTicktingFeesReturnFalseForAxessUser()
  {
    TicketingFeesServiceStub builder(*_myServer);
    _tfpTrx->getRequest()->ticketingAgent()->agentTJR()->hostName() = AXESS_USER;
    _tfpTrx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = AXESS_MULTIHOST_ID;
    CPPUNIT_ASSERT(!builder.collectTicketingFees(*_tfpTrx));
  }

  void testCollectTicktingFeesReturnFalseForInfiniUser()
  {
    TicketingFeesServiceStub builder(*_myServer);
    _tfpTrx->getRequest()->ticketingAgent()->agentTJR()->hostName() = INFINI_USER;
    _tfpTrx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = INFINI_MULTIHOST_ID;
    CPPUNIT_ASSERT(!builder.collectTicketingFees(*_tfpTrx));
  }

  void testCollectTicktingFeesReturnFalseForCarrierPartition()
  {
    TicketingFeesServiceStub builder(*_myServer);
    _tfpTrx->getRequest()->ticketingAgent()->agentTJR() = 0;
    _tfpTrx->billing()->partitionID() = "AA";
    _tfpTrx->billing()->aaaCity() = "DFW";
    CPPUNIT_ASSERT(!builder.collectTicketingFees(*_tfpTrx));
  }

  void testCollectTicktingFeesReturnTrueForPricingTrx()
  {
    TicketingFeesServiceStub builder(*_myServer);
    CPPUNIT_ASSERT(builder.collectTicketingFees(*_tfpTrx));
  }

  void testProcessTicketingFeesReturnTrueWhenNoDefaultValidatingCxr()
  {
    TicketingFeesServiceStub builder(*_myServer);
    FarePath fp;
    _itin->farePath().push_back(&fp);
    _tfpTrx->setValidatingCxrGsaApplicable(true);
    CPPUNIT_ASSERT(builder.processTicketingFees(*_tfpTrx));
  }

private:
  TseServer* _myServer;
  TicketingFeesService* _tfs;
  TestMemHandle _memHandle;
  Trx* _trx;
  DataHandle _dataHandle;
  TktFeesRequest* _tktFeesReq;
  TktFeesPricingTrx* _tfpTrx;
  Agent* _agent;
  Customer* _customer;
  Billing* _billing;

  TktFeesRequest::TktFeesFareBreakAssociation* _fba;
  TktFeesRequest::PaxTypePayment* _ptp;
  TktFeesRequest::PassengerPaymentInfo* _ppi;
  TktFeesRequest::FormOfPayment* _fop;

  Itin* _itin;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TicketingFeesServiceTest);
} // tse
