#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/Agent.h"
#include "DataModel/RexPricingOptions.h"

using namespace tse;

class RexPricingRequestTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexPricingRequestTest);
  CPPUNIT_TEST(testCurrentTicketAgentRetrievedWhenAnalyzingNewItin);
  CPPUNIT_TEST(testPreviousTicketAgentRetrievedWhenAnalyzingExchangeItin);
  CPPUNIT_TEST(testSetTicketingDT);
  CPPUNIT_TEST(testGetTicketingDT);
  CPPUNIT_TEST(testTicketingDTWithoutTrxAssociation);
  CPPUNIT_TEST(testTicketingDTWithTrxAssociationWhenAnalysingNewItin);
  CPPUNIT_TEST(testTicketingDTWithTrxAssociationWhenAnalysingExchangeItin);
  CPPUNIT_TEST(testTktDesignator_ExcItin);
  CPPUNIT_TEST(testTktDesignator_NotExcItin);
  CPPUNIT_TEST(testTktDesignator_PricingRequest);
  CPPUNIT_TEST(testTktDesignator_segmentOrder);
  CPPUNIT_TEST(testIsTktDesignatorEntry);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    request = new RexPricingRequest();
    request->setTrx(&trx);
    request->ticketingAgent() = &agent;
    request->prevTicketIssueAgent() = &previousTicketAgent;

    trx.setRequest(request);
    trx.setOptions(new RexPricingOptions);
  }

  void tearDown()
  {
    delete request;
    delete trx.getOptions();
  }

  void testCurrentTicketAgentRetrievedWhenAnalyzingNewItin()
  {
    trx.setAnalyzingExcItin(false);
    CPPUNIT_ASSERT(trx.getRequest()->ticketingAgent() == &agent);
  }

  void testPreviousTicketAgentRetrievedWhenAnalyzingExchangeItin()
  {
    trx.setAnalyzingExcItin(true);
    CPPUNIT_ASSERT(trx.getRequest()->ticketingAgent() == &previousTicketAgent);
  }

  void testSetTicketingDT()
  {
    DateTime ticketingDT;
    ticketingDT = DateTime(2007, 10, 10);
    request->setTicketingDT(ticketingDT);
    CPPUNIT_ASSERT(request->getTicketingDT() == ticketingDT);
  }

  void testGetTicketingDT()
  {
    DateTime ticketingDT;
    ticketingDT = DateTime(2007, 10, 10);
    request->setTicketingDT(ticketingDT);
    CPPUNIT_ASSERT(request->getTicketingDT() == ticketingDT);
  }

  void testTicketingDTWithoutTrxAssociation()
  {
    PricingRequest* priRequest = 0;

    priRequest = request;
    request->setTrx(0);
    DateTime ticketingDT;
    ticketingDT = DateTime(2007, 10, 10);

    request->setTicketingDT(ticketingDT);
    CPPUNIT_ASSERT(request->getTicketingDT() == ticketingDT);
    CPPUNIT_ASSERT(request->ticketingDT() == ticketingDT);
    CPPUNIT_ASSERT(priRequest->ticketingDT() == ticketingDT);
  }

  void testTicketingDTWithTrxAssociationWhenAnalysingNewItin()
  {
    PricingRequest* priRequest = 0;

    priRequest = request;
    trx.currentTicketingDT() = DateTime(2007, 5, 10);
    trx.setOriginalTktIssueDT() = DateTime(2007, 4, 10);

    DateTime ticketingDT;
    ticketingDT = DateTime(2007, 10, 10);
    request->setTicketingDT(ticketingDT);

    trx.setAnalyzingExcItin(false);

    CPPUNIT_ASSERT(request->getTicketingDT() == ticketingDT);
    CPPUNIT_ASSERT(request->ticketingDT() == trx.fareApplicationDT());
    CPPUNIT_ASSERT(request->ticketingDT() == trx.currentTicketingDT());
    CPPUNIT_ASSERT(request->ticketingDT() == trx.dataHandle().ticketDate());

    CPPUNIT_ASSERT(priRequest->ticketingDT() == trx.fareApplicationDT());
    CPPUNIT_ASSERT(priRequest->ticketingDT() == trx.currentTicketingDT());
    CPPUNIT_ASSERT(priRequest->ticketingDT() == trx.dataHandle().ticketDate());
  }

  void testTicketingDTWithTrxAssociationWhenAnalysingExchangeItin()
  {
    PricingRequest* priRequest = 0;

    priRequest = request;
    trx.currentTicketingDT() = DateTime(2007, 5, 10);
    trx.setOriginalTktIssueDT() = DateTime(2007, 4, 10);

    DateTime ticketingDT;
    ticketingDT = DateTime(2007, 10, 10);
    request->setTicketingDT(ticketingDT);

    trx.setAnalyzingExcItin(true);

    CPPUNIT_ASSERT(request->getTicketingDT() == ticketingDT);
    CPPUNIT_ASSERT(request->ticketingDT() == trx.fareApplicationDT());
    CPPUNIT_ASSERT(request->ticketingDT() == trx.originalTktIssueDT());
    CPPUNIT_ASSERT(request->ticketingDT() == trx.dataHandle().ticketDate());

    CPPUNIT_ASSERT(priRequest->ticketingDT() == trx.fareApplicationDT());
    CPPUNIT_ASSERT(priRequest->ticketingDT() == trx.originalTktIssueDT());
    CPPUNIT_ASSERT(priRequest->ticketingDT() == trx.dataHandle().ticketDate());
  }

  void testTktDesignator_ExcItin()
  {
    const TktDesignator memphis = "MEMPHIS";
    trx.setAnalyzingExcItin(true);
    request->tktDesignator().insert(std::make_pair(1, memphis));

    const TktDesignator check = request->tktDesignator()[1];

    CPPUNIT_ASSERT_EQUAL(check, memphis);

    trx.setAnalyzingExcItin(false);
    CPPUNIT_ASSERT(!request->tktDesignator().size());
  }

  void testTktDesignator_NotExcItin()
  {
    const TktDesignator memphis = "MEMPHIS";
    trx.setAnalyzingExcItin(false);
    request->tktDesignator().insert(std::make_pair(1, memphis));

    const TktDesignator check = request->tktDesignator()[1];

    CPPUNIT_ASSERT_EQUAL(check, memphis);

    trx.setAnalyzingExcItin(true);
    CPPUNIT_ASSERT(!request->tktDesignator().size());
  }

  void testTktDesignator_PricingRequest()
  {
    PricingRequest request;
    const TktDesignator memphis = "MEMPHIS";
    request.tktDesignator().insert(std::make_pair(1, memphis));

    // we suppose to use base class method so no matter what phase is and no trx is needed
    const TktDesignator check = request.tktDesignator()[1];

    trx.setAnalyzingExcItin(false);
    CPPUNIT_ASSERT_EQUAL(check, memphis);
    trx.setAnalyzingExcItin(true);
    CPPUNIT_ASSERT_EQUAL(check, memphis);
  }

  void testTktDesignator_segmentOrder()
  {
    const TktDesignator memphis = "MEMPHIS";
    trx.setAnalyzingExcItin(true);
    request->tktDesignator().insert(std::make_pair(1, memphis));

    const TktDesignator check = request->tktDesignator(1);

    CPPUNIT_ASSERT_EQUAL(check, memphis);

    trx.setAnalyzingExcItin(false);
    CPPUNIT_ASSERT(!request->tktDesignator().size());
  }

  void testIsTktDesignatorEntry()
  {
    const TktDesignator memphis = "MEMPHIS";
    trx.setAnalyzingExcItin(true);
    request->tktDesignator().insert(std::make_pair(1, memphis));

    const TktDesignator check = request->tktDesignator()[1];

    CPPUNIT_ASSERT(request->isTktDesignatorEntry());

    trx.setAnalyzingExcItin(false);
    CPPUNIT_ASSERT(!request->isTktDesignatorEntry());
  }

protected:
  tse::RexPricingTrx trx;
  tse::Agent agent;
  tse::Agent previousTicketAgent;
  RexPricingRequest* request;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RexPricingRequestTest);
