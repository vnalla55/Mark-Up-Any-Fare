//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/COP.h"
#include "DBAccess/DiskCache.h"
#include "MinFares/COPMinimumFare.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "test/include/MockGlobal.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingRequest.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "MinFares/test/TestUtil.h"
#include "test/include/TestMemHandle.h"
#include "MinFares/test/MinFareDataHandleTest.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

class COPMinimumFareTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(COPMinimumFareTest);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testReorderPu);
  CPPUNIT_TEST(testIsCopLoc);
  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _memHandle.create<MinFareDataHandleTest>();
  }
  void tearDown() { _memHandle.clear(); }

  void testProcess()
  {
    PricingTrx trx;
    trx.setTravelDate(DateTime::localTime());
    trx.diagnostic().diagnosticType() = Diagnostic765;
    trx.diagnostic().activate();

    Loc loc1;
    loc1.loc() = "FRA";
    loc1.nation() = "DE";
    loc1.subarea() = EUROPE;

    Loc loc2;
    loc2.loc() = "PAR";
    loc2.nation() = "FR";
    loc2.subarea() = EUROPE;

    Loc loc3;
    loc3.loc() = "NYC";
    loc3.nation() = "US";
    loc3.subarea() = NORTH_AMERICA;

    // Build Agent
    Agent agent;
    Loc loca;
    loca.loc() = "LON";
    loca.nation() = "SG";

    agent.agentLocation() = &loca; // Sold in GP (Guadeloupe)

    // Build Request
    PricingRequest request;
    request.ticketingAgent() = &agent;
    trx.setRequest(&request);
    trx.setTravelDate(DateTime::localTime());

    AirSeg travelSeg1;
    travelSeg1.segmentOrder() = 0;
    travelSeg1.geoTravelType() = GeoTravelType::International;
    travelSeg1.origin() = &loc1;
    travelSeg1.destination() = &loc2;
    travelSeg1.carrier() = "AF";
    travelSeg1.departureDT() = DateTime::localTime();

    AirSeg travelSeg2;
    travelSeg2.segmentOrder() = 1;
    travelSeg2.geoTravelType() = GeoTravelType::International;
    travelSeg2.origin() = &loc2;
    travelSeg2.destination() = &loc3;
    travelSeg2.carrier() = "AF";

    AirSeg travelSeg3;
    travelSeg3.segmentOrder() = 2;
    travelSeg3.geoTravelType() = GeoTravelType::International;
    travelSeg3.origin() = &loc3;
    travelSeg3.destination() = &loc1;
    travelSeg3.carrier() = "AZ";

    AirSeg travelSeg4;
    travelSeg4.segmentOrder() = 3;
    travelSeg4.geoTravelType() = GeoTravelType::International;
    travelSeg4.origin() = &loc1;
    travelSeg4.destination() = &loc2;
    travelSeg4.carrier() = "AF";

    AirSeg travelSeg5;
    travelSeg5.segmentOrder() = 4;
    travelSeg5.geoTravelType() = GeoTravelType::International;
    travelSeg5.origin() = &loc2;
    travelSeg5.destination() = &loc1;
    travelSeg5.carrier() = "AF";

    // Build Itin
    Itin itin;
    itin.travelSeg().push_back(&travelSeg1);
    itin.travelSeg().push_back(&travelSeg2);
    itin.travelSeg().push_back(&travelSeg3);
    itin.travelSeg().push_back(&travelSeg4);
    itin.travelSeg().push_back(&travelSeg5);
    itin.intlSalesIndicator() = Itin::SITI;
    itin.ticketingCarrier() = "AF";

    // Build 1st pricing unit
    PricingUnit pu;
    pu.puType() = PricingUnit::Type::CIRCLETRIP;
    pu.puFareType() = PricingUnit::NL;
    pu.geoTravelType() = GeoTravelType::International;
    pu.travelSeg().push_back(&travelSeg1);
    pu.travelSeg().push_back(&travelSeg2);
    pu.travelSeg().push_back(&travelSeg3);

    // Build 2rd pricing unit
    PricingUnit pu1;
    pu1.puType() = PricingUnit::Type::ROUNDTRIP;
    pu1.puFareType() = PricingUnit::NL;
    pu1.geoTravelType() = GeoTravelType::International;
    pu1.travelSeg().push_back(&travelSeg4);
    pu1.travelSeg().push_back(&travelSeg5);

    // Build Fare Path
    FarePath farePath;
    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pu);
    // farePath.pricingUnit().push_back(&pu1); // Can not test it now. Zone validation will access
    // DataHandle.

    trx.travelSeg().push_back(&travelSeg1);

    COPMinimumFare cop(trx, farePath);

    CPPUNIT_ASSERT(cop.process() >= 0.0);

    const std::vector<CopMinimum*>& cops = cop.getCopInfo(agent.agentLocation()->nation());
    CPPUNIT_ASSERT(cops.size() >= 0);

    // std::string str = trx.diagnostic().toString();
    // std::cout << str << std::endl;
  }

  void testReorderPu()
  {
    PricingTrx trx;
    trx.setTravelDate(DateTime::localTime());
    trx.diagnostic().diagnosticType() = Diagnostic765;
    trx.diagnostic().activate();

    const Loc* HKG, *TYO, *LON, *JNB, *NYC, *OSA;
    HKG = trx.dataHandle().getLoc("HKG", DateTime::localTime());
    TYO = trx.dataHandle().getLoc("TYO", DateTime::localTime());
    LON = trx.dataHandle().getLoc("LON", DateTime::localTime());
    JNB = trx.dataHandle().getLoc("JNB", DateTime::localTime());
    NYC = trx.dataHandle().getLoc("NYC", DateTime::localTime());
    OSA = trx.dataHandle().getLoc("OSA", DateTime::localTime());

    Agent agent;
    agent.agentLocation() = TYO;

    PricingRequest request;
    request.ticketingAgent() = &agent;
    trx.setRequest(&request);
    trx.setTravelDate(DateTime::localTime());

    TravelSeg* tvlSeg[6];
    tvlSeg[0] = TestUtil::createAirSeg(0, HKG, TYO, "AF");
    tvlSeg[0]->departureDT() = DateTime::localTime();
    tvlSeg[1] = TestUtil::createAirSeg(1, TYO, LON, "AF");
    tvlSeg[2] = TestUtil::createAirSeg(2, LON, JNB, "AF");
    tvlSeg[3] = TestUtil::createAirSeg(3, JNB, NYC, "AF");
    tvlSeg[4] = TestUtil::createAirSeg(4, NYC, OSA, "AF");
    tvlSeg[5] = TestUtil::createAirSeg(4, OSA, HKG, "AF");

    Itin itin;
    itin.travelSeg().clear();
    itin.travelSeg().push_back(tvlSeg[0]);
    itin.travelSeg().push_back(tvlSeg[1]);
    itin.travelSeg().push_back(tvlSeg[2]);
    itin.travelSeg().push_back(tvlSeg[3]);
    itin.travelSeg().push_back(tvlSeg[4]);
    itin.travelSeg().push_back(tvlSeg[5]);

    PricingUnit pu;
    pu.travelSeg().clear();
    pu.travelSeg().push_back(tvlSeg[0]);
    pu.travelSeg().push_back(tvlSeg[1]);
    pu.travelSeg().push_back(tvlSeg[2]);
    pu.travelSeg().push_back(tvlSeg[3]);
    pu.travelSeg().push_back(tvlSeg[4]);
    pu.travelSeg().push_back(tvlSeg[5]);

    FarePath farePath;
    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pu);

    trx.travelSeg().push_back(tvlSeg[0]);

    COPMinimumFare cop(trx, farePath);
    std::vector<std::vector<TravelSeg*>> repriceTvlSegs;
    CPPUNIT_ASSERT(cop.getTvlSegFromCopPoint(pu, repriceTvlSegs));
    /*for (int i=0, n=repriceTvlSegs.size(); i<n; i++)
    {
      std::cout << "\nReprice TvlSeg:\n";
      COPMinimumFare::TravelSegVec& ts = repriceTvlSegs[i];
      for (COPMinimumFare::TravelSegVec::const_iterator i = ts.begin();
           i != ts.end();
           ++i)
      {
        std::cout << (*i)->origin()->loc()
             << "--"
             << (*i)->destination()->loc()
             << std::endl;
      }
    }*/
  }

  void testIsCopLoc()
  {
    PricingTrx trx;
    trx.setTravelDate(DateTime::localTime());
    trx.diagnostic().diagnosticType() = Diagnostic765;
    trx.diagnostic().activate();

    const Loc* PAR = trx.dataHandle().getLoc("PAR", DateTime::localTime());
    const Loc* MOW = trx.dataHandle().getLoc("MOW", DateTime::localTime());

    AirSeg airSeg;
    airSeg.departureDT() = DateTime::localTime();

    Agent agent;
    agent.agentLocation() = PAR;

    PricingRequest request;
    request.ticketingAgent() = &agent;
    trx.setRequest(&request);
    trx.setTravelDate(DateTime::localTime());

    Itin itin;
    PricingUnit pu;
    FarePath farePath;
    itin.setTravelDate(DateTime::localTime());
    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pu);
    trx.travelSeg().push_back(&airSeg);

    COPMinimumFare cop(trx, farePath);

    CPPUNIT_ASSERT(cop.isCOPLoc("MQ", PAR->nation()));
    CPPUNIT_ASSERT(cop.isCOPLoc("GP", PAR->nation()));
    CPPUNIT_ASSERT(cop.isCOPLoc("FR", PAR->nation()));
    CPPUNIT_ASSERT(cop.isCOPLoc("RE", PAR->nation()));

    CPPUNIT_ASSERT(cop.isCOPLoc("AZ", MOW->nation()));
    CPPUNIT_ASSERT(cop.isCOPLoc("UZ", MOW->nation()));
    CPPUNIT_ASSERT(cop.isCOPLoc("UA", MOW->nation()));
    CPPUNIT_ASSERT(cop.isCOPLoc("TM", MOW->nation()));
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(COPMinimumFareTest);
}
