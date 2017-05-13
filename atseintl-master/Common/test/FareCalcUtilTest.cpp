#include "Common/FareCalcUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareCalcConfig.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <string>

using namespace std;

namespace tse
{
class FareCalcUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareCalcUtilTest);
  CPPUNIT_TEST(testDoubleToStringTruncate);

  CPPUNIT_TEST(testGetUserApplMultihostAA);
  CPPUNIT_TEST(testGetUserApplMultihostNotAA);
  CPPUNIT_TEST(testGetUserApplForAxessUser);
  CPPUNIT_TEST(testGetUserApplForAbacusUser);
  CPPUNIT_TEST(testGetUserApplForInfiniUser);
  CPPUNIT_TEST(testGetUserApplForSabreUser);
  CPPUNIT_TEST(testGetUserApplForInfiniHelpDeskUser);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FareCalcConfig* _fcConfig;
  PricingRequest* _request;
  Agent* _agent;

public:
  void setUp()
  {
    _trx = _memHandle.create<PricingTrx>();
    _fcConfig = _memHandle.create<FareCalcConfig>();
    _fcConfig->warningMessages() = 'Y';
    _trx->fareCalcConfig() = _fcConfig;

    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);
    _agent = _memHandle.create<Agent>();
    _request->ticketingAgent() = _agent;

  }

  void testDoubleToStringTruncate()
  {
    string result;

    FareCalcUtil::doubleToStringTruncate(0.0, result, 0);
    CPPUNIT_ASSERT_EQUAL(string("0"), result);

    FareCalcUtil::doubleToStringTruncate(0.0, result, 1);
    CPPUNIT_ASSERT_EQUAL(string("0.0"), result);

    FareCalcUtil::doubleToStringTruncate(0.0, result, 2);
    CPPUNIT_ASSERT_EQUAL(string("0.00"), result);

    FareCalcUtil::doubleToStringTruncate(15.123456, result, 1);
    CPPUNIT_ASSERT_EQUAL(string("15.1"), result);

    FareCalcUtil::doubleToStringTruncate(1234.25, result, 1);
    CPPUNIT_ASSERT_EQUAL(string("1234.2"), result);

    FareCalcUtil::doubleToStringTruncate(0.999, result, 1);
    CPPUNIT_ASSERT_EQUAL(string("1.0"), result);

    FareCalcUtil::doubleToStringTruncate(0.999, result, 2);
    CPPUNIT_ASSERT_EQUAL(string("0.99"), result);

    FareCalcUtil::doubleToStringTruncate(0.123456789, result, 8);
    CPPUNIT_ASSERT_EQUAL(string("0.12345678"), result);
  }

  void testGetUserApplMultihostAA()
  {
    Indicator userApplType = CRS_USER_APPL;
    std::string userAppl;
    _agent->hostCarrier() = "AA";
    FareCalcUtil::getUserAppl(*_trx, *_agent, userApplType, userAppl);

    CPPUNIT_ASSERT("AA" == userAppl);
    CPPUNIT_ASSERT(MULTIHOST_USER_APPL == userApplType);
  }
  void testGetUserApplMultihostNotAA()
  {
    Indicator userApplType = CRS_USER_APPL;
    std::string userAppl;
    _agent->hostCarrier() = "XX";
    FareCalcUtil::getUserAppl(*_trx, *_agent, userApplType, userAppl);

    CPPUNIT_ASSERT("XX" == userAppl);
    CPPUNIT_ASSERT(MULTIHOST_USER_APPL == userApplType);
  }
  void testGetUserApplForSabreUser()
  {
    Indicator userApplType = CRS_USER_APPL;
    std::string userAppl;
    _agent->hostCarrier() = "XX";
    _agent->vendorCrsCode() = SABRE_MULTIHOST_ID;
    FareCalcUtil::getUserAppl(*_trx, *_agent, userApplType, userAppl);

    CPPUNIT_ASSERT(SABRE_USER == userAppl);
    CPPUNIT_ASSERT(CRS_USER_APPL == userApplType);
  }
  void testGetUserApplForAxessUser()
  {
    Indicator userApplType = CRS_USER_APPL;
    std::string userAppl;
    _agent->hostCarrier() = "XX";
    _agent->vendorCrsCode() = AXESS_MULTIHOST_ID;
    FareCalcUtil::getUserAppl(*_trx, *_agent, userApplType, userAppl);

    CPPUNIT_ASSERT(AXESS_USER == userAppl);
    CPPUNIT_ASSERT(CRS_USER_APPL == userApplType);
  }
  void testGetUserApplForAbacusUser()
  {
    Indicator userApplType = CRS_USER_APPL;
    std::string userAppl;
    _agent->hostCarrier() = "XX";
    _agent->vendorCrsCode() = ABACUS_MULTIHOST_ID;
    FareCalcUtil::getUserAppl(*_trx, *_agent, userApplType, userAppl);

    CPPUNIT_ASSERT(ABACUS_USER == userAppl);
    CPPUNIT_ASSERT(CRS_USER_APPL == userApplType);
  }
  void testGetUserApplForInfiniUser()
  {
    Indicator userApplType = CRS_USER_APPL;
    std::string userAppl;
    _agent->hostCarrier() = "XX";
    _agent->vendorCrsCode() = INFINI_MULTIHOST_ID;
    FareCalcUtil::getUserAppl(*_trx, *_agent, userApplType, userAppl);

    CPPUNIT_ASSERT(INFINI_USER == userAppl);
    CPPUNIT_ASSERT(CRS_USER_APPL == userApplType);
  }
  void testGetUserApplForInfiniHelpDeskUser()
  {
    Indicator userApplType = CRS_USER_APPL;
    std::string userAppl;
    _agent->hostCarrier() = "AA";
    _agent->vendorCrsCode() = INFINI_MULTIHOST_ID;
    FareCalcUtil::getUserAppl(*_trx, *_agent, userApplType, userAppl);

    CPPUNIT_ASSERT(INFINI_USER == userAppl);
    CPPUNIT_ASSERT(CRS_USER_APPL == userApplType);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareCalcUtilTest);
}
