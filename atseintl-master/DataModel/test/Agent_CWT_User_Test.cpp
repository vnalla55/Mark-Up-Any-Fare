#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/Agent.h"
#include "DBAccess/Customer.h"
#include "Common/TrxUtil.h"

using namespace tse;

class Agent_CWT_User_Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Agent_CWT_User_Test);
  CPPUNIT_TEST(testCWTAgent);
  CPPUNIT_TEST(testNonCWTAgent);
  CPPUNIT_TEST_SUITE_END();

protected:
  Agent* _agent;
  Customer* _customer;

public:
  void setUp()
  {
    _agent = new Agent;
    _customer = new Customer;
  }

  void tearDown()
  {
    delete _agent;
    delete _customer;
  }

  void testCWTAgent()
  {
    _customer->ssgGroupNo() = Agent::CWT_GROUP_NUMBER;
    _agent->agentTJR() = _customer;

    CPPUNIT_ASSERT(_customer->ssgGroupNo() == Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(_agent->cwtUser() == true);
  }

  void testNonCWTAgent()
  {
    CPPUNIT_ASSERT(_agent->cwtUser() == false);

    _agent->agentTJR() = NULL;
    CPPUNIT_ASSERT(_agent->cwtUser() == false);

    _customer->ssgGroupNo() = 0;
    CPPUNIT_ASSERT(_customer->ssgGroupNo() == 0);

    _agent->agentTJR() = _customer;
    CPPUNIT_ASSERT(_agent->cwtUser() == false);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Agent_CWT_User_Test);
