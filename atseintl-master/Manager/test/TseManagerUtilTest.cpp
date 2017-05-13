#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

#include "Common/CustomerActivationUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/Agent.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/CurrencyTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/Trx.h"
#include "DBAccess/Customer.h"
#include "Manager/TseManagerUtil.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{

class TseManagerUtilTest : public CppUnit::TestFixture
{
  class MyDataHandle : public DataHandleMock
  {
  public:
    const std::vector<CustomerActivationControl*>&
    getCustomerActivationControl(const std::string& projectCode)
    {
      const DateTime today = DateTime::localTime();
      const DateTime discDate = today.addDays(90);
      std::vector<CustomerActivationControl*>* cacList =
          _memHandle.create<std::vector<CustomerActivationControl*> >();

      if ("GSA" == projectCode)
      {
        CustomerActivationControl* cac = createCustomerActivationControl(
            10, today, discDate, "A0B0", false);
        cacList->push_back(cac);
        cac = createCustomerActivationControl(20, today, discDate, "", true);
        cacList->push_back(cac);
      }

      return *cacList;
    }

    CustomerActivationControl*
    createCustomerActivationControl(int64_t seqNo,
                                    const DateTime& effDate,
                                    const DateTime& discDate,
                                    const PseudoCityCode& pcc,
                                    bool isActivated)
    {
      static const Indicator NOT_ACTIVE = ' ';
      static const Indicator ACTIVE     = 'X';

      CustomerActivationControl* cac = _memHandle.create<CustomerActivationControl>();
      cac->seqNo()       = seqNo;
      cac->effDate()     = effDate;
      cac->discDate()    = discDate;
      cac->pseudoCity()  = pcc;
      cac->projActvInd() = isActivated ? ACTIVE : NOT_ACTIVE;
      return cac;
    }

  private:
    TestMemHandle _memHandle;
  };


  CPPUNIT_TEST_SUITE(TseManagerUtilTest);
    CPPUNIT_TEST(test_setTrxActivationFlags_ValidatingCxrGsa_NoRequest);
    CPPUNIT_TEST(test_setTrxActivationFlags_ValidatingCxrGsa_NoAgent);
    CPPUNIT_TEST(test_setTrxActivationFlags_ValidatingCxrGsa_HostAgent);
    CPPUNIT_TEST(test_setTrxActivationFlags_ValidatingCxrGsa_NonApplicableTrxType);
    /// CPPUNIT_TEST(test_setTrxActivationFlags_ValidatingCxrGsa_InactiveAgency);
    CPPUNIT_TEST(test_setTrxActivationFlags_ValidatingCxrGsa_ApplicableTrxType);
    CPPUNIT_TEST(test_setTrxActivationFlags_ValidatingCxrGsa_NotPricingTrx);
    /// CPPUNIT_TEST(test_setTrxActivationFlags_ValidatingCxrGsa_RexPricingTrx_Active);
    /// CPPUNIT_TEST(test_setTrxActivationFlags_ValidatingCxrGsa_RexDateIgnored);
    CPPUNIT_TEST(test_setTrxActivationFlags_ValidatingCxrGsa_BaggageTrx);
    CPPUNIT_TEST(test_setTrxActivationFlags_ValidatingCxrGsa_AncillaryTrx);
    CPPUNIT_TEST(test_setTrxActivationFlags_ValidatingCxrGsa_TktFeesTrx);

    CPPUNIT_TEST(test_setTrxActivationFlags_ValidatingCxrGsa_TJR_Y_ON);
    CPPUNIT_TEST(test_setTrxActivationFlags_ValidatingCxrGsa_TJR_N_OFF);

    CPPUNIT_TEST(test_setShoppingGsaActivationFlags_ValidatingCxrGsa_NotActive);
    CPPUNIT_TEST(test_setShoppingGsaActivationFlags_ValidatingCxrGsa_NoRequest);
    CPPUNIT_TEST(test_setShoppingGsaActivationFlags_ValidatingCxrGsa_NonApplicableTrxType);
    CPPUNIT_TEST(test_setShoppingGsaActivationFlags_ValidatingCxrGsa_ApplicableTrxType);
    CPPUNIT_TEST(test_setShoppingGsaActivationFlags_ValidatingCxrGsa_RequestFalse);
    CPPUNIT_TEST(test_setTrxActivationFlags_FareSelection_NoRequest);
    CPPUNIT_TEST(test_setTrxActivationFlags_FareSelection_NotYetActivated);
    CPPUNIT_TEST(test_setTrxActivationFlags_FareSelection_ActivatedAndApplicable);
    CPPUNIT_TEST(test_setTrxActivationFlags_FareSelection_ActivatedAndApplicable_Shopping);
    CPPUNIT_TEST(test_setTrxActivationFlags_FareSelection_ActivatedAndNotApplicable_Shopping);
    CPPUNIT_TEST(test_setTrxActivationFlags_FareSelection_ActivatedButNotApplicable);
  CPPUNIT_TEST_SUITE_END();

private:
  MyDataHandle* _mdh;
  TestMemHandle _memHandle;
  Trx*          _trx;
  PricingTrx*   _pTrx;

public:
  void setUp()
  {
    _mdh = _memHandle.create<MyDataHandle>();
    _memHandle.create<TestConfigInitializer>();
    _pTrx = _memHandle.create<PricingTrx>();
    _trx = _pTrx;
  }

  void tearDown() { _memHandle.clear(); }

  void setPricingRequestAndAgent(const PseudoCityCode& agency = "ABCD")
  {
    PricingRequest* request = _memHandle.create<PricingRequest>();
    Agent*          agent   = _memHandle.create<Agent>();
    agent->tvlAgencyPCC() = agency;
    request->ticketingAgent() = agent;

    Customer *tjr = _memHandle.create<Customer>();
    tjr->crsCarrier() = SABRE_MULTIHOST_ID;
    tjr->hostName() = "SABR";
    tjr->pricingApplTag5() = 'Y';
    request->ticketingAgent()->agentTJR() = tjr;
    _pTrx->setRequest(request);
  }

  void setTicketingDate(PricingTrx* trx, bool active)
  {
    DateTime tktDate = DateTime::localTime();
    tktDate = active ? tktDate.addDays(2) : tktDate.subtractDays(2);
    trx->ticketingDate() = tktDate;
    trx->dataHandle().setTicketDate(tktDate);
  }

  void test_setTrxActivationFlags_ValidatingCxrGsa_NoRequest()
  {
    setPricingRequestAndAgent();
    setTicketingDate(_pTrx, true);
    _pTrx->setRequest(0);
    TseManagerUtil::setTrxActivationFlags(_pTrx);
    CPPUNIT_ASSERT(!_pTrx->isValidatingCxrGsaApplicable());
  }

  void test_setTrxActivationFlags_ValidatingCxrGsa_NoAgent()
  {
    setPricingRequestAndAgent();
    setTicketingDate(_pTrx, true);
    _pTrx->getRequest()->ticketingAgent() = 0;
    TseManagerUtil::setTrxActivationFlags(_pTrx);
    CPPUNIT_ASSERT(!_pTrx->isValidatingCxrGsaApplicable());
  }

  void test_setTrxActivationFlags_ValidatingCxrGsa_HostAgent()
  {
    const PseudoCityCode hostAgent = "";
    setPricingRequestAndAgent(hostAgent);
    setTicketingDate(_pTrx, true);
    TseManagerUtil::setTrxActivationFlags(_pTrx);
    CPPUNIT_ASSERT(!_pTrx->isValidatingCxrGsaApplicable());
  }

  void test_setTrxActivationFlags_ValidatingCxrGsa_NonApplicableTrxType()
  {
    setPricingRequestAndAgent();
    setTicketingDate(_pTrx, true);
    _pTrx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
    TseManagerUtil::setTrxActivationFlags(_pTrx);
    CPPUNIT_ASSERT(!_pTrx->isValidatingCxrGsaApplicable());
  }

  void test_setTrxActivationFlags_ValidatingCxrGsa_InactiveAgency()
  {
    //fallback::value::fallbackValidatingCxrTJROptOut.set(true);
    setPricingRequestAndAgent("A0B0"); // Inactive agency
    setTicketingDate(_pTrx, true);
    _pTrx->setTrxType(PricingTrx::PRICING_TRX);
    TseManagerUtil::setTrxActivationFlags(_pTrx);
    CPPUNIT_ASSERT(!_pTrx->isValidatingCxrGsaApplicable());
  }

  void test_setTrxActivationFlags_ValidatingCxrGsa_ApplicableTrxType()
  {
    setPricingRequestAndAgent();
    setTicketingDate(_pTrx, true);
    _pTrx->setTrxType(PricingTrx::PRICING_TRX);
    _pTrx->setValidatingCxrGsaApplicable(false);
    TseManagerUtil::setTrxActivationFlags(_pTrx);
    CPPUNIT_ASSERT(_pTrx->isValidatingCxrGsaApplicable());

    _pTrx->setTrxType(PricingTrx::MIP_TRX);
    _pTrx->setValidatingCxrGsaApplicable(false);
    TseManagerUtil::setTrxActivationFlags(_pTrx);
    CPPUNIT_ASSERT(_pTrx->isValidatingCxrGsaApplicable());

    _pTrx->setTrxType(PricingTrx::IS_TRX);
    _pTrx->setValidatingCxrGsaApplicable(false);
    TseManagerUtil::setTrxActivationFlags(_pTrx);
    CPPUNIT_ASSERT(_pTrx->isValidatingCxrGsaApplicable());

    _pTrx->setTrxType(PricingTrx::FF_TRX);
    _pTrx->setValidatingCxrGsaApplicable(false);
    TseManagerUtil::setTrxActivationFlags(_pTrx);
    CPPUNIT_ASSERT(_pTrx->isValidatingCxrGsaApplicable());
  }

  void test_setTrxActivationFlags_ValidatingCxrGsa_NotPricingTrx()
  {
    setPricingRequestAndAgent();
    setTicketingDate(_pTrx, true);
    CurrencyTrx trx;
    TseManagerUtil::setTrxActivationFlags(&trx);
    CPPUNIT_ASSERT(!_pTrx->isValidatingCxrGsaApplicable());
  }

  void test_setTrxActivationFlags_ValidatingCxrGsa_RexPricingTrx_Active()
  {
    //fallback::value::fallbackValidatingCxrTJROptOut.set(true);
    RexPricingTrx rex;
    PricingRequest request;
    Agent agent;
    agent.tvlAgencyPCC() = "ABCD";
    request.ticketingAgent() = &agent;
    rex.setRequest(&request);
    setTicketingDate(&rex, true);
    const DateTime today = DateTime::localTime();
    rex.currentTicketingDT() = today.addDays(2); // CAC active date
    rex.setValidatingCxrGsaApplicable(false);
    TseManagerUtil::setTrxActivationFlags(&rex);
    CPPUNIT_ASSERT(rex.isValidatingCxrGsaApplicable());
  }

  void test_setTrxActivationFlags_ValidatingCxrGsa_RexDateIgnored()
  {
    //fallback::value::fallbackValidatingCxrTJROptOut.set(true);
    RexPricingTrx rex;
    PricingRequest request;
    Agent agent;
    agent.tvlAgencyPCC() = "ABCD";
    request.ticketingAgent() = &agent;
    rex.setRequest(&request);
    setTicketingDate(&rex, true);
    const DateTime today = DateTime::localTime();
    // CAC inactive date, but this date is ignored
    rex.currentTicketingDT() = today.subtractDays(20);
    rex.setValidatingCxrGsaApplicable(false);
    TseManagerUtil::setTrxActivationFlags(&rex);
    CPPUNIT_ASSERT(rex.isValidatingCxrGsaApplicable());
  }

  void test_setTrxActivationFlags_ValidatingCxrGsa_BaggageTrx()
  {
    BaggageTrx bagTrx;
    PricingRequest request;
    Agent agent;
    agent.tvlAgencyPCC() = "ABCD";
    request.ticketingAgent() = &agent;
    bagTrx.setRequest( &request );
    bagTrx.setTrxType(PricingTrx::PRICING_TRX);
    setTicketingDate( &bagTrx, true );
    bagTrx.setValidatingCxrGsaApplicable(false);
    TseManagerUtil::setTrxActivationFlags( &bagTrx );
    CPPUNIT_ASSERT( !bagTrx.isValidatingCxrGsaApplicable() );
  }

  void test_setTrxActivationFlags_ValidatingCxrGsa_AncillaryTrx()
  {
    AncillaryPricingTrx ancTrx;
    PricingRequest request;
    Agent agent;
    agent.tvlAgencyPCC() = "ABCD";
    request.ticketingAgent() = &agent;
    ancTrx.setRequest( &request );
    ancTrx.setTrxType(PricingTrx::PRICING_TRX);
    setTicketingDate( &ancTrx, true );
    ancTrx.setValidatingCxrGsaApplicable(false);
    TseManagerUtil::setTrxActivationFlags( &ancTrx );
    CPPUNIT_ASSERT( !ancTrx.isValidatingCxrGsaApplicable() );
  }

  void test_setTrxActivationFlags_ValidatingCxrGsa_TktFeesTrx()
  {
    TktFeesPricingTrx tktTrx;
    PricingRequest request;
    Agent agent;
    agent.tvlAgencyPCC() = "ABCD";
    request.ticketingAgent() = &agent;
    tktTrx.setRequest( &request );
    tktTrx.setTrxType(PricingTrx::PRICING_TRX);
    setTicketingDate( &tktTrx, true );
    tktTrx.setValidatingCxrGsaApplicable(false);
    TseManagerUtil::setTrxActivationFlags( &tktTrx );
    CPPUNIT_ASSERT( !tktTrx.isValidatingCxrGsaApplicable() );
  }

  void test_setTrxActivationFlags_ValidatingCxrGsa_TJR_Y_ON()
  {
    setPricingRequestAndAgent();
    setTicketingDate(_pTrx, true);
    _pTrx->setTrxType(PricingTrx::PRICING_TRX);

    PricingRequest* request = _pTrx->getRequest();
    CPPUNIT_ASSERT(request != 0 &&
        request->ticketingAgent() &&
        request->ticketingAgent()->agentTJR());
    request->ticketingAgent()->agentTJR()->pricingApplTag5() = 'Y';

    TseManagerUtil::setTrxActivationFlags(_pTrx);
    CPPUNIT_ASSERT(_pTrx->isValidatingCxrGsaApplicable());
  }

  void test_setTrxActivationFlags_ValidatingCxrGsa_TJR_N_OFF()
  {
    setPricingRequestAndAgent();
    setTicketingDate(_pTrx, true);
    _pTrx->setTrxType(PricingTrx::PRICING_TRX);

    PricingRequest* request = _pTrx->getRequest();
    CPPUNIT_ASSERT(request != 0 &&
        request->ticketingAgent() &&
        request->ticketingAgent()->agentTJR());
    request->ticketingAgent()->agentTJR()->pricingApplTag5() = 'N';

    TseManagerUtil::setTrxActivationFlags(_pTrx);
    CPPUNIT_ASSERT(!_pTrx->isValidatingCxrGsaApplicable());
  }

  void test_setShoppingGsaActivationFlags_ValidatingCxrGsa_NotActive()
  {
    setPricingRequestAndAgent();
    setTicketingDate(_pTrx, false);
    TseManagerUtil::setShoppingGsaActivationFlags(_pTrx);
    CPPUNIT_ASSERT(!_pTrx->isValidatingCxrGsaApplicable());
  }

  void test_setShoppingGsaActivationFlags_ValidatingCxrGsa_NoRequest()
  {
    setPricingRequestAndAgent();
    setTicketingDate(_pTrx, true);
    _pTrx->setRequest(0);
    TseManagerUtil::setShoppingGsaActivationFlags(_pTrx);
    CPPUNIT_ASSERT(!_pTrx->isValidatingCxrGsaApplicable());
  }

  void test_setShoppingGsaActivationFlags_ValidatingCxrGsa_NonApplicableTrxType()
  {
    setPricingRequestAndAgent();
    setTicketingDate(_pTrx, true);
    _pTrx->setTrxType(PricingTrx::PRICING_TRX);
    TseManagerUtil::setShoppingGsaActivationFlags(_pTrx);
    CPPUNIT_ASSERT(!_pTrx->isValidatingCxrGsaApplicable());
  }

  void test_setShoppingGsaActivationFlags_ValidatingCxrGsa_ApplicableTrxType()
  {
    setPricingRequestAndAgent();
    _pTrx->getRequest()->setValidatingCarrierRequest(true);
    _pTrx->setValidatingCxrGsaApplicable(true);

    _pTrx->setTrxType(PricingTrx::MIP_TRX);
    TseManagerUtil::setShoppingGsaActivationFlags(_pTrx);
    CPPUNIT_ASSERT(_pTrx->isValidatingCxrGsaApplicable());

    _pTrx->setTrxType(PricingTrx::IS_TRX);
    TseManagerUtil::setShoppingGsaActivationFlags(_pTrx);
    CPPUNIT_ASSERT(_pTrx->isValidatingCxrGsaApplicable());

    _pTrx->setTrxType(PricingTrx::FF_TRX);
    TseManagerUtil::setShoppingGsaActivationFlags(_pTrx);
    CPPUNIT_ASSERT(_pTrx->isValidatingCxrGsaApplicable());
  }

  void test_setShoppingGsaActivationFlags_ValidatingCxrGsa_RequestFalse()
  {
    setPricingRequestAndAgent();
    _pTrx->getRequest()->setValidatingCarrierRequest(false);
    _pTrx->setValidatingCxrGsaApplicable(true);

    _pTrx->setTrxType(PricingTrx::MIP_TRX);
    TseManagerUtil::setShoppingGsaActivationFlags(_pTrx);
    CPPUNIT_ASSERT(!_pTrx->isValidatingCxrGsaApplicable());

    _pTrx->setTrxType(PricingTrx::IS_TRX);
    TseManagerUtil::setShoppingGsaActivationFlags(_pTrx);
    CPPUNIT_ASSERT(!_pTrx->isValidatingCxrGsaApplicable());

    _pTrx->setTrxType(PricingTrx::FF_TRX);
    TseManagerUtil::setShoppingGsaActivationFlags(_pTrx);
    CPPUNIT_ASSERT(!_pTrx->isValidatingCxrGsaApplicable());
  }

  void test_setTrxActivationFlags_FareSelection_NoRequest()
  {
    setPricingRequestAndAgent();
    _pTrx->setRequest(0);

    _pTrx->setTrxType( PricingTrx::PRICING_TRX );
    TseManagerUtil::setTrxActivationFlags( _pTrx );
    CPPUNIT_ASSERT( !_pTrx->isIataFareSelectionApplicable() );
  }

  void test_setTrxActivationFlags_FareSelection_NotYetActivated()
  {
    setPricingRequestAndAgent();

    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue(
        "IATA_FARE_SELECTION_ACTIVATION_DATE", activationDate, "PRICING_SVC" );
    DateTime dt( activationDate, 0 );
    _pTrx->ticketingDate() = dt.subtractDays(2);

    _pTrx->setTrxType( PricingTrx::PRICING_TRX );
    TseManagerUtil::setTrxActivationFlags( _pTrx );
    CPPUNIT_ASSERT( !_pTrx->isIataFareSelectionApplicable() );
  }

  void test_setTrxActivationFlags_FareSelection_ActivatedAndApplicable()
  {
    setPricingRequestAndAgent();

    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue(
        "IATA_FARE_SELECTION_ACTIVATION_DATE", activationDate, "PRICING_SVC", true );
    DateTime dt( activationDate, 0 );
    _pTrx->ticketingDate() = dt.addDays( 2 );

 
    CPPUNIT_ASSERT( TrxUtil::isIataFareSelectionActivated( *_pTrx ) );
    _pTrx->setTrxType( PricingTrx::PRICING_TRX );
    _pTrx->setValidatingCxrGsaApplicable(false);
    TseManagerUtil::setTrxActivationFlags( _pTrx );
    CPPUNIT_ASSERT( TrxUtil::isIataFareSelectionActivated( *_pTrx ) );
    CPPUNIT_ASSERT( _pTrx->isIataFareSelectionApplicable() );
  }

  void test_setTrxActivationFlags_FareSelection_ActivatedAndNotApplicable_Shopping()
  {
    setPricingRequestAndAgent();

    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue(
        "IATA_FARE_SELECTION_ACTIVATION_DATE", activationDate, "PRICING_SVC", true );
    DateTime dt( activationDate, 0 );
    _pTrx->ticketingDate() = dt.addDays( 2 );

    CPPUNIT_ASSERT( TrxUtil::isIataFareSelectionActivated(*_pTrx) );
    _pTrx->setTrxType(PricingTrx::IS_TRX);
    _pTrx->setProjectActivationFlag(0);
    TseManagerUtil::setTrxActivationFlags( _pTrx );
    CPPUNIT_ASSERT(TrxUtil::isIataFareSelectionActivated(*_pTrx));
    CPPUNIT_ASSERT(!_pTrx->isIataFareSelectionApplicable());

    CPPUNIT_ASSERT( TrxUtil::isIataFareSelectionActivated(*_pTrx) );
    _pTrx->setTrxType(PricingTrx::IS_TRX);
    TseManagerUtil::setTrxActivationFlags( _pTrx );
    CPPUNIT_ASSERT(TrxUtil::isIataFareSelectionActivated(*_pTrx));
    CPPUNIT_ASSERT(!_pTrx->isIataFareSelectionApplicable());

    CPPUNIT_ASSERT( TrxUtil::isIataFareSelectionActivated(*_pTrx) );
    _pTrx->setTrxType(PricingTrx::MIP_TRX);
    TseManagerUtil::setTrxActivationFlags( _pTrx );
    CPPUNIT_ASSERT(TrxUtil::isIataFareSelectionActivated(*_pTrx));
    CPPUNIT_ASSERT(_pTrx->isIataFareSelectionApplicable());
  }

  void test_setTrxActivationFlags_FareSelection_ActivatedAndApplicable_Shopping()
  {
    setPricingRequestAndAgent();

    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue(
        "IATA_FARE_SELECTION_ACTIVATION_DATE", activationDate, "PRICING_SVC", true );
    DateTime dt( activationDate, 0 );
    _pTrx->ticketingDate() = dt.addDays( 2 );

    CPPUNIT_ASSERT( TrxUtil::isIataFareSelectionActivated(*_pTrx) );
    _pTrx->setTrxType(PricingTrx::IS_TRX);
    _pTrx->setProjectActivationFlag(1);
    TseManagerUtil::setTrxActivationFlags( _pTrx );
    CPPUNIT_ASSERT(TrxUtil::isIataFareSelectionActivated(*_pTrx));
    CPPUNIT_ASSERT(!_pTrx->isIataFareSelectionApplicable());

    CPPUNIT_ASSERT( TrxUtil::isIataFareSelectionActivated(*_pTrx) );
    _pTrx->setTrxType(PricingTrx::IS_TRX);
    _pTrx->setProjectActivationFlag(3);
    TseManagerUtil::setTrxActivationFlags( _pTrx );
    CPPUNIT_ASSERT(TrxUtil::isIataFareSelectionActivated(*_pTrx));
    CPPUNIT_ASSERT(!_pTrx->isIataFareSelectionApplicable());

    CPPUNIT_ASSERT( TrxUtil::isIataFareSelectionActivated(*_pTrx) );
    _pTrx->setTrxType(PricingTrx::MIP_TRX);
    TseManagerUtil::setTrxActivationFlags( _pTrx );
    CPPUNIT_ASSERT(TrxUtil::isIataFareSelectionActivated(*_pTrx));
    CPPUNIT_ASSERT(_pTrx->isIataFareSelectionApplicable());
  }

  void test_setTrxActivationFlags_FareSelection_ActivatedButNotApplicable()
  {
    AncillaryPricingTrx ancillaryTrx;
    PricingRequest request;
    Agent agent;
    request.ticketingAgent() = &agent;
    ancillaryTrx.setRequest( &request );

    const std::string activationDate = "2025-02-01";
    TestConfigInitializer::setValue(
        "IATA_FARE_SELECTION_ACTIVATION_DATE", activationDate, "PRICING_SVC" );
    DateTime dt( activationDate, 0 );
    ancillaryTrx.ticketingDate() = dt.addDays(2);

    TseManagerUtil::setTrxActivationFlags( &ancillaryTrx );
    CPPUNIT_ASSERT( !ancillaryTrx.isIataFareSelectionApplicable() );
  }

}; // class TseManagerUtilTest

CPPUNIT_TEST_SUITE_REGISTRATION(TseManagerUtilTest);

} // namespace tse
