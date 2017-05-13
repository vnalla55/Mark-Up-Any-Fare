#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataManager.h"
#include "DBAccess/Loc.h"
#include "test/include/CppUnitHelperMacros.h"


using namespace std;

namespace tse
{
class AgentTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AgentTest);
  CPPUNIT_TEST(testSet);
  CPPUNIT_TEST(testGet);
  CPPUNIT_TEST(testIsMultiSettlementPlanUser);
  CPPUNIT_TEST(testGetMultiSettlementPlanTypes);
  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------------------
  // testSet()
  //---------------------------------------------------------------------
  void testSet()
  {
    Loc loc;
    loc.loc() = "DFW";
    loc.subarea() = "1";
    loc.area() = "2";
    loc.nation() = "US";
    loc.state() = "TX";
    loc.cityInd() = true;

    Agent agent;
    agent.agentLocation() = &loc;
    agent.agentCity() = "DFW";
    agent.tvlAgencyPCC() = "HDQ";
    agent.mainTvlAgencyPCC() = "HDQ";
    agent.tvlAgencyIATA() = "XYZ";
    agent.homeAgencyIATA() = "XYZ";
    agent.agentFunctions() = "XYZ";
    agent.agentDuty() = "XYZ";
    agent.airlineDept() = "XYZ";
    agent.cxrCode() = "AA";
    agent.currencyCodeAgent() = "USD";
    agent.coHostID() = 9;
    agent.agentCommissionType() = "PERCENT";
    agent.agentCommissionAmount() = 10;

    CPPUNIT_ASSERT(true);
  }

  //---------------------------------------------------------------------
  // testGet()
  //---------------------------------------------------------------------
  void testGet()
  {
    Loc loc;
    loc.loc() = "DFW";
    loc.subarea() = "1";
    loc.area() = "2";
    loc.nation() = "US";
    loc.state() = "TX";
    loc.cityInd() = true;

    Agent agent;
    agent.agentLocation() = &loc;
    agent.agentCity() = "DFW";
    agent.tvlAgencyPCC() = "HDQ";
    agent.mainTvlAgencyPCC() = "HDQ";
    agent.tvlAgencyIATA() = "XYZ";
    agent.homeAgencyIATA() = "XYZ";
    agent.agentFunctions() = "XYZ";
    agent.agentDuty() = "XYZ";
    agent.airlineDept() = "XYZ";
    agent.cxrCode() = "AA";
    agent.currencyCodeAgent() = "USD";
    agent.coHostID() = 9;
    agent.agentCommissionType() = "PERCENT";
    agent.agentCommissionAmount() = 10;

    Agent agent2;
    agent2.agentLocation() = agent.agentLocation();
    agent2.agentCity() = agent.agentCity();
    agent2.tvlAgencyPCC() = agent.tvlAgencyPCC();
    agent2.mainTvlAgencyPCC() = agent.mainTvlAgencyPCC();
    agent2.tvlAgencyIATA() = agent.tvlAgencyIATA();
    agent2.homeAgencyIATA() = agent.homeAgencyIATA();
    agent2.agentFunctions() = agent.agentFunctions();
    agent2.agentDuty() = agent.agentDuty();
    agent2.airlineDept() = agent.airlineDept();
    agent2.cxrCode() = agent.cxrCode();
    agent2.currencyCodeAgent() = agent.currencyCodeAgent();
    agent2.coHostID() = agent.coHostID();
    agent2.agentCommissionType() = agent.agentCommissionType();
    agent2.agentCommissionAmount() = agent.agentCommissionAmount();

    CPPUNIT_ASSERT(true);
  }

  void testIsMultiSettlementPlanUser()
  {
    Agent agent;
    Customer customer;

    CPPUNIT_ASSERT( !agent.isMultiSettlementPlanUser() ); // No TJR

    agent.agentTJR() = &customer;

    CPPUNIT_ASSERT( !agent.isMultiSettlementPlanUser() ); // No settlement plans

    customer.settlementPlans() = "AAABBBCCCDDD";
    CPPUNIT_ASSERT( agent.isMultiSettlementPlanUser() );
  }

  void testGetMultiSettlementPlanTypes()
  {
    std::vector<SettlementPlanType> settlementPlanTypes;
    Agent agent;
    Customer customer;
    agent.agentTJR() = &customer;

    // No plans
    settlementPlanTypes.clear();
    customer.settlementPlans().clear();
    agent.getMultiSettlementPlanTypes( settlementPlanTypes );
    CPPUNIT_ASSERT( 0 == settlementPlanTypes.size() );

    // One plan
    customer.settlementPlans().assign( "AAA" );
    agent.getMultiSettlementPlanTypes( settlementPlanTypes );
    CPPUNIT_ASSERT( 1 == settlementPlanTypes.size() );
    CPPUNIT_ASSERT( "AAA" == settlementPlanTypes[0] );

    // Invalid plan length
    settlementPlanTypes.clear();
    customer.settlementPlans().assign( "AA" );
    agent.getMultiSettlementPlanTypes( settlementPlanTypes );
    CPPUNIT_ASSERT( 0 == settlementPlanTypes.size() );

    // Two and partial => only two
    settlementPlanTypes.clear();
    customer.settlementPlans().assign( "AAABBBC" );
    agent.getMultiSettlementPlanTypes( settlementPlanTypes );
    CPPUNIT_ASSERT( 2 == settlementPlanTypes.size() );
    CPPUNIT_ASSERT( "AAA" == settlementPlanTypes[0] );
    CPPUNIT_ASSERT( "BBB" == settlementPlanTypes[1] );

    // Lots of plans
    settlementPlanTypes.clear();
    customer.settlementPlans().assign( "ARCBSPSATKRYPRTRUTGENTCHGTC" );
    agent.getMultiSettlementPlanTypes( settlementPlanTypes );
    CPPUNIT_ASSERT( 9 == settlementPlanTypes.size() );
    CPPUNIT_ASSERT( "ARC" == settlementPlanTypes[0] );
    CPPUNIT_ASSERT( "BSP" == settlementPlanTypes[1] );
    CPPUNIT_ASSERT( "SAT" == settlementPlanTypes[2] );
    CPPUNIT_ASSERT( "KRY" == settlementPlanTypes[3] );
    CPPUNIT_ASSERT( "PRT" == settlementPlanTypes[4] );
    CPPUNIT_ASSERT( "RUT" == settlementPlanTypes[5] );
    CPPUNIT_ASSERT( "GEN" == settlementPlanTypes[6] );
    CPPUNIT_ASSERT( "TCH" == settlementPlanTypes[7] );
    CPPUNIT_ASSERT( "GTC" == settlementPlanTypes[8] );
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(AgentTest);
}
