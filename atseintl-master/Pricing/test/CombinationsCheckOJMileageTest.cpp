#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/test/MockFareMarket.h"
#include "Pricing/test/MockLoc.h"
#include "DBAccess/Mileage.h"
#include "Pricing/Combinations.h"

namespace tse
{

class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    Mileage* mil = _memHandle(new Mileage);
    if ((origin == "AAA" && dest == "BBB") || (origin == "CCC" && dest == "DDD"))
    {
      mil->mileage() = 1000;
    }
    else
    {
      mil->mileage() = 1200;
    }
    return mil;
  }
};

class CombinationsCheckOJMileageTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CombinationsCheckOJMileageTest);
  /*
      CPPUNIT_TEST(testLongOJ);
      CPPUNIT_TEST(testShortOJ);
      CPPUNIT_TEST(testLongOJRestrDomestic);
      CPPUNIT_TEST(testShortOJRestrDomestic);
      CPPUNIT_TEST(testLongOJRestrDomesticNotSameNation);
      CPPUNIT_TEST(testShortOJRestrDomesticNotSameNation);
  */
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    trx = new PricingTrx();
    diag = new DiagCollector();
    pu = new PricingUnit();
    pu->geoTravelType() = GeoTravelType::Domestic;
    pu->puType() = PricingUnit::Type::OPENJAW;
    pu->puSubType() = PricingUnit::DOUBLE_OPENJAW;
    fu = new FareUsage();
    far = new PaxTypeFare();
    fm = new MockFareMarket();
    fu->paxTypeFare() = far;
    far->fareMarket() = fm;
    pu->fareUsage().push_back(fu);
    fu2 = new FareUsage();
    far2 = new PaxTypeFare();
    fm2 = new MockFareMarket();
    fu2->paxTypeFare() = far2;
    far2->fareMarket() = fm2;
    pu->fareUsage().push_back(fu2);

    loc1 = new MockLoc();
    loc2 = new MockLoc();
    loc3 = new MockLoc();
    loc4 = new MockLoc();
    loc5 = new MockLoc();

    loc1->setLoc("AAA");
    loc1->setNation("AA");
    loc2->setLoc("BBB");
    loc2->setNation("AA");
    loc3->setLoc("CCC");
    loc3->setNation("AA");
    loc4->setLoc("DDD");
    loc4->setNation("AA");
    loc5->setLoc("DDD");
    loc5->setNation("BB");

    fm->set_origin(loc1);
    fm->set_destination(loc2);
    fm2->set_origin(loc3);
    fm2->set_destination(loc4);

    _memHandle.create<MyDataHandle>();
  }

  void tearDown()
  {
    delete trx;
    delete diag;
    delete pu;
    delete fu;
    delete far;
    delete fu2;
    delete far2;
    _memHandle.clear();
  }

  void testLongOJ()
  {
    Combinations comb;
    CPPUNIT_ASSERT(!comb.checkOJMileage(*diag, *pu, false, true));
  }

  void testShortOJ()
  {
    fm->set_destination(loc4);
    fm2->set_destination(loc2);
    Combinations comb;
    CPPUNIT_ASSERT(comb.checkOJMileage(*diag, *pu, false, true));
  }

  void testLongOJRestrDomestic()
  {
    Combinations comb;
    CPPUNIT_ASSERT(comb.checkOJMileage(*diag, *pu, false, false));
  }

  void testShortOJRestrDomestic()
  {
    fm->set_destination(loc4);
    fm2->set_destination(loc2);
    Combinations comb;
    CPPUNIT_ASSERT(comb.checkOJMileage(*diag, *pu, false, false));
  }

  void testLongOJRestrDomesticNotSameNation()
  {
    fm2->set_destination(loc5);
    Combinations comb;
    CPPUNIT_ASSERT(comb.checkOJMileage(*diag, *pu, false, false));
  }

  void testShortOJRestrDomesticNotSameNation()
  {
    fm->set_destination(loc5);
    fm2->set_destination(loc2);
    Combinations comb;
    CPPUNIT_ASSERT(comb.checkOJMileage(*diag, *pu, false, false));
  }

protected:
  TestMemHandle _memHandle;
  PricingTrx* trx;
  DiagCollector* diag;
  PricingUnit* pu;
  FareUsage* fu;
  PaxTypeFare* far;
  MockFareMarket* fm;
  FareUsage* fu2;
  PaxTypeFare* far2;
  MockFareMarket* fm2;
  MockLoc* loc1;
  MockLoc* loc2;
  MockLoc* loc3;
  MockLoc* loc4;
  MockLoc* loc5;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CombinationsCheckOJMileageTest);
}
