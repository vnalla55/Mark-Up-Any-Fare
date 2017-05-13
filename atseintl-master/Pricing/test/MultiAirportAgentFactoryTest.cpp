#include "DataModel/AirSeg.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/MultiAirportAgentFactory.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tse
{

class MultiAirportAgentFactoryTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MultiAirportAgentFactoryTest);
  CPPUNIT_TEST(mapGeneration);
  CPPUNIT_SKIP_TEST(creatingAgent);
  CPPUNIT_TEST_SUITE_END();

  void mapGeneration()
  {
    // given
    const string s = "LON*LGW|NYC*LGA";
    MultiAirportAgentFactory factory;

    // when
    factory.generateCityMap(s);

    // then
    CPPUNIT_ASSERT_EQUAL(factory._cityAirportContainer.size(), static_cast<size_t>(2));
    CPPUNIT_ASSERT_EQUAL(factory._cityAirportContainer[0].first, string("LON"));
    CPPUNIT_ASSERT_EQUAL(factory._cityAirportContainer[0].second, string("LGW"));
    CPPUNIT_ASSERT_EQUAL(factory._cityAirportContainer[1].first, string("NYC"));
    CPPUNIT_ASSERT_EQUAL(factory._cityAirportContainer[1].second, string("LGA"));
  }

  void creatingAgent()
  {
    // given
    MultiAirportAgentFactory factory;
    AirSeg as;
    as.boardMultiCity() = "LON";
    as.offMultiCity() = "DFW";
    Itin itin;
    itin.travelSeg().push_back(&as);
    ShoppingTrx::SchedulingOption sop(&itin, 0, false, false);
    ShoppingTrx trx; // test fail here
    ShoppingTrx::Leg leg;
    leg.sop().push_back(sop);
    trx.legs().push_back(leg);
    trx.setSimpleTrip(true);

    // when
    MultiAirportAgent* t = factory.getAgent("LON*LGW|NYC*LGA", trx);

    // then
    CPPUNIT_ASSERT(t != 0);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MultiAirportAgentFactoryTest);
}
