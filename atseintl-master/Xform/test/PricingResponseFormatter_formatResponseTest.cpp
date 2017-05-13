#include "test/include/CppUnitHelperMacros.h"
#include "Xform/PricingResponseFormatter.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{

namespace
{
class MockPricingResponseFormatter : public PricingResponseFormatter
{
protected:
  virtual void formatPricingResponse(XMLConstruct& construct,
                                     const std::string& responseString,
                                     bool displayOnly,
                                     PricingTrx& pricingTrx,
                                     FareCalcCollector* fareCalcCollector,
                                     ErrorResponseException::ErrorResponseCode tktErrCode,
                                     bool prepareAgentAndBilling = true)
  {
    construct.openElement("formatPricingResponse");
    construct.closeElement();
  }

  void formatResponseForFullRefund(XMLConstruct& construct,
                                   const std::string& responseString,
                                   RefundPricingTrx& trx,
                                   ErrorResponseException::ErrorResponseCode tktErrCode) override
  {
    construct.openElement("formatResponseForFullRefund");
    construct.closeElement();
  }

  void formatResponseForFullRefund(XMLConstruct& construct,
                                   const std::string& responseString,
                                   RexBaseTrx& rexTrx,
                                   ErrorResponseException::ErrorResponseCode tktErrCode,
                                   bool taxInfoRequest) override
  {
    construct.openElement("formatResponseForFullRefund");
    construct.closeElement();
  }

  virtual void formatResponseIfRedirected(XMLConstruct& construct,
                                          const std::string& responseString,
                                          RexBaseTrx& rexTrx,
                                          FareCalcCollector* fareCalcCollector,
                                          ErrorResponseException::ErrorResponseCode tktErrCode)
  {
    construct.openElement("formatResponseIfRedirected");
    construct.closeElement();
  }
};
}

class PricingResponseFormatter_formatResponseTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingResponseFormatter_formatResponseTest);

  CPPUNIT_TEST(testFormatResponse_PricingTrx_DisplayOnly);
  CPPUNIT_TEST(testFormatResponse_PricingTrx);

  CPPUNIT_TEST(testFormatResponse_MePricingTrx_DisplayOnly);
  CPPUNIT_TEST(testFormatResponse_MePricingTrx);

  CPPUNIT_TEST(testFormatResponse_ExchangePricingTrx);

  CPPUNIT_TEST(testFormatResponse_RefundTrx);
  CPPUNIT_TEST(testFormatResponse_RefundTrx_FullRefund);
  CPPUNIT_TEST(testFormatResponse_RefundTrx_Redirected);

  CPPUNIT_TEST_SUITE_END();

  ErrorResponseException::ErrorResponseCode noError;
  PricingResponseFormatter* _formater;
  TestMemHandle _memH;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _formater = _memH.create<MockPricingResponseFormatter>();
  }

  void tearDown() { _memH.clear(); }

  enum
  {
    notDisplayOnly = 0,
    displayOnly = 1
  };

  PricingTrx& createPricingTrx()
  {
    PricingTrx* trx = _memH.create<PricingTrx>();
    trx->setRequest(_memH.create<PricingRequest>());
    trx->getRequest()->ticketingAgent() = _memH.create<Agent>();
    trx->billing() = _memH.create<Billing>();
    trx->itin().push_back(_memH.create<Itin>());
    return *trx;
  }

  ExchangePricingTrx& createExchangeTrx()
  {
    ExchangePricingTrx* trx = _memH.create<ExchangePricingTrx>();
    trx->setRequest(_memH.create<PricingRequest>());
    trx->getRequest()->ticketingAgent() = _memH.create<Agent>();
    trx->billing() = _memH.create<Billing>();
    trx->itin().push_back(_memH.create<Itin>());
    trx->reqType() = "FE";
    return *trx;
  }

  RefundPricingTrx& createRefundTrx()
  {
    RefundPricingTrx* trx = _memH.create<RefundPricingTrx>();
    trx->setRequest(_memH.create<PricingRequest>());
    trx->getRequest()->ticketingAgent() = _memH.create<Agent>();
    trx->itin().push_back(_memH.create<Itin>());
    trx->redirected() = false;
    trx->fullRefund() = false;
    trx->secondaryExcReqType() = "AM";
    return *trx;
  }

  void testFormatResponse_PricingTrx_DisplayOnly()
  {
    PricingTrx& trx = createPricingTrx();
    CPPUNIT_ASSERT_EQUAL(std::string("<PricingDisplayResponse>"
                                     "<formatPricingResponse/>"
                                     "</PricingDisplayResponse>"),
                         _formater->formatResponse("", displayOnly, trx, 0, noError));
  }

  void testFormatResponse_PricingTrx()
  {
    PricingTrx& trx = createPricingTrx();
    CPPUNIT_ASSERT_EQUAL(std::string("<PricingResponse>"
                                     "<formatPricingResponse/>"
                                     "</PricingResponse>"),
                         _formater->formatResponse("", notDisplayOnly, trx, 0, noError));
  }

  void testFormatResponse_MePricingTrx_DisplayOnly()
  {
    PricingTrx& trx = createPricingTrx();
    trx.setExcTrxType(PricingTrx::ME_DIAG_TRX);
    CPPUNIT_ASSERT_EQUAL(std::string("<formatPricingResponse/>"),
                         _formater->formatResponse("", displayOnly, trx, 0, noError));
  }

  void testFormatResponse_MePricingTrx()
  {
    PricingTrx& trx = createPricingTrx();
    trx.setExcTrxType(PricingTrx::ME_DIAG_TRX);
    CPPUNIT_ASSERT_EQUAL(std::string("<formatPricingResponse/>"),
                         _formater->formatResponse("", notDisplayOnly, trx, 0, noError));
  }

  void testFormatResponse_ExchangePricingTrx()
  {
    ExchangePricingTrx& trx = createExchangeTrx();
    CPPUNIT_ASSERT_EQUAL(std::string("<RexPricingResponse S96=\"FE\">"
                                     "<formatPricingResponse/>"
                                     "</RexPricingResponse>"),
                         _formater->formatResponse("", notDisplayOnly, trx, 0, noError));
  }

  void testFormatResponse_RefundTrx()
  {
    RefundPricingTrx& trx = createRefundTrx();
    CPPUNIT_ASSERT_EQUAL(std::string("<RexPricingResponse S96=\"AF\">"
                                     "<formatPricingResponse/>"
                                     "</RexPricingResponse>"),
                         _formater->formatResponse("", notDisplayOnly, trx, 0, noError));
  }

  void testFormatResponse_RefundTrx_FullRefund()
  {
    RefundPricingTrx& trx = createRefundTrx();
    trx.fullRefund() = true;
    CPPUNIT_ASSERT_EQUAL(std::string("<RexPricingResponse S96=\"AF\">"
                                     "<formatResponseForFullRefund/>"
                                     "</RexPricingResponse>"),
                         _formater->formatResponse("", notDisplayOnly, trx, 0, noError));
  }

  void testFormatResponse_RefundTrx_Redirected()
  {
    RefundPricingTrx& trx = createRefundTrx();
    trx.redirected() = true;
    CPPUNIT_ASSERT_EQUAL(std::string("<RexPricingResponse S96=\"AF\" SA8=\"AM\">"
                                     "<formatResponseIfRedirected/>"
                                     "</RexPricingResponse>"),
                         _formater->formatResponse("", notDisplayOnly, trx, 0, noError));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PricingResponseFormatter_formatResponseTest);
}
