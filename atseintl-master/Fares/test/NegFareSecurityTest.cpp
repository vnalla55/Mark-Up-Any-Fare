#include "test/include/CppUnitHelperMacros.h"

#include "Rules/RuleConst.h"

#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Agent.h"
#include "DBAccess/Loc.h"

#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/DCFactory.h"

#include "DBAccess/NegFareSecurityInfo.h"
#include "Fares/NegFareSecurity.h"

namespace tse
{
class NegFareSecurityTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NegFareSecurityTest);
  CPPUNIT_TEST(testMatch);
  CPPUNIT_TEST_SUITE_END();

  //-----------------------------------------------------------------
  // testMatch()
  //-----------------------------------------------------------------
  void testMatch()
  {
    bool ret = false;

    init();
    NegFareSecurity nfSec(&_nfInfo);

    ret = nfSec.isMatchGeo(_agent, DateTime::localTime());
    CPPUNIT_ASSERT_EQUAL(true, ret);

    ret = nfSec.isMatchWho(_trx, _trx.getRequest()->ticketingAgent());
    CPPUNIT_ASSERT_EQUAL(true, ret);

    nfSec.localeType() = RuleConst::CRS_CRX_DEPT_CODE;
    nfSec.crsCarrierDepartment() = "XYZ";
    ret = nfSec.isMatchWho(_trx, _trx.getRequest()->ticketingAgent());
    CPPUNIT_ASSERT_EQUAL(true, ret);

    nfSec.crsCarrierDepartment() = "SCLTR";
    ret = nfSec.isMatchWho(_trx, _trx.getRequest()->ticketingAgent());
    CPPUNIT_ASSERT_EQUAL(true, ret);

    nfSec.localeType() = RuleConst::TRAVEL_AGENCY; // reset

    ret = nfSec.isMatchWhat(_trx.getRequest()->isTicketEntry());
    CPPUNIT_ASSERT_EQUAL(true, ret);

    nfSec.agencyPCC() = "UGH";
    ret = nfSec.isMatch(_trx, _trx.getRequest()->ticketingAgent());
    CPPUNIT_ASSERT_EQUAL(false, ret);

    ret = nfSec.isPos();
    CPPUNIT_ASSERT_EQUAL(true, ret);

    nfSec.applInd() = RuleConst::NOT_ALLOWED;
    ret = nfSec.isPos();
    CPPUNIT_ASSERT_EQUAL(false, ret);
  }

  //-----------------------------------------------------------------
  // init()
  //-----------------------------------------------------------------
  void init()
  {
    // TRANSACTION INFO
    _locAgent.loc() = "BOS";
    _locAgent.nation() = ("US");
    _locAgent.area() = ("1");

    _agent.agentLocation() = &_locAgent;
    _agent.agentCity() = "DFW";
    _agent.tvlAgencyPCC() = "HDQ";
    _agent.mainTvlAgencyPCC() = "HDQ";
    _agent.tvlAgencyIATA() = "XYZ";
    _agent.homeAgencyIATA() = "XYZ";
    _agent.agentFunctions() = "XYZ";
    _agent.agentDuty() = "XYZ";
    _agent.airlineDept() = "XYZ";
    _agent.cxrCode() = "AA";
    _agent.currencyCodeAgent() = "USD";
    _agent.coHostID() = 9;
    _agent.agentCommissionType() = "PERCENT";
    _agent.agentCommissionAmount() = 10;
    _agent.officeDesignator() = "SCLTR";

    _req.ticketingAgent() = &_agent;
    _req.ticketEntry() = 'Y';

    _trx.diagnostic().diagnosticType() = Diagnostic335;
    _trx.diagnostic().activate();
    _trx.setRequest(&_req);

    // DATABASE INFO
    _nfInfo.loc1().locType() = LOCTYPE_NATION;
    _nfInfo.loc1().loc() = _locAgent.nation();
    _nfInfo.loc2().locType() = ' ';
    _nfInfo.tvlAgencyInd() = 'X';
    _nfInfo.localeType() = RuleConst::TRAVEL_AGENCY;
    _nfInfo.agencyPCC() = _agent.tvlAgencyPCC();
    _nfInfo.ticketInd() = 'Y';
    _nfInfo.sellInd() = 'Y';
  }

private:
  NegFareSecurityInfo _nfInfo;
  PricingTrx _trx;
  PricingRequest _req;
  Agent _agent;
  Loc _locAgent;
};
CPPUNIT_TEST_SUITE_REGISTRATION(NegFareSecurityTest);
}
