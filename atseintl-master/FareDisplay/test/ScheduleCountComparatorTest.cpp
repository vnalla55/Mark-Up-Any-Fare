#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/Comparator.h"
#include "FareDisplay/ScheduleCountComparator.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "test/include/TestMemHandle.h"
#include "DSS/FlightCount.h"

using namespace std;

namespace tse
{
class ScheduleCountComparatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ScheduleCountComparatorTest);
  CPPUNIT_TEST(testCompare_NoFlightCount);
  CPPUNIT_TEST(testCompare_AAMoreNonstop);
  CPPUNIT_TEST(testCompare_BBMoreNonstop);
  CPPUNIT_TEST(testCompare_AAMoreDirect);
  CPPUNIT_TEST(testCompare_BBMoreDirect);
  CPPUNIT_TEST(testCompare_AAMoreOnline);
  CPPUNIT_TEST(testCompare_BBMoreOnline);
  CPPUNIT_TEST(testCompare_SameAABB);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _trx = _memHandle.create<FareDisplayTrx>();
    _trx->fdResponse() = _memHandle.create<FareDisplayResponse>();
    _comparator = _memHandle.create<ScheduleCountComparator>();
    Fare* f = _memHandle.create<Fare>();
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->carrier() = "AA";
    f->setFareInfo(fi);
    _ptfAA.setFare(f);
    f = _memHandle.create<Fare>();
    fi = _memHandle.create<FareInfo>();
    fi->carrier() = "BB";
    f->setFareInfo(fi);
    _ptfBB.setFare(f);
    _fliAA = _memHandle.create<FlightCount>();
    _fliBB = _memHandle.create<FlightCount>();
    _fliAA->_carrier = "AA";
    _fliBB->_carrier = "BB";
    _fliAA->_nonStop = _fliBB->_nonStop = 1;
    _fliAA->_direct = _fliBB->_direct = 2;
    _fliAA->_onlineConnection = _fliBB->_onlineConnection = 3;
  }
  void tearDown() { _memHandle.clear(); }
  void testCompare_NoFlightCount()
  {
    _fliAA = _fliBB = 0;
    setFlightCounts();
    CPPUNIT_ASSERT_EQUAL(Comparator::EQUAL, _comparator->compare(_ptfAA, _ptfBB));
  }
  void testCompare_AAMoreNonstop()
  {
    _fliAA->_nonStop = 10;
    setFlightCounts();
    CPPUNIT_ASSERT_EQUAL(Comparator::TRUE, _comparator->compare(_ptfAA, _ptfBB));
  }
  void testCompare_BBMoreNonstop()
  {
    _fliBB->_nonStop = 10;
    setFlightCounts();
    CPPUNIT_ASSERT_EQUAL(Comparator::FALSE, _comparator->compare(_ptfAA, _ptfBB));
  }
  void testCompare_AAMoreDirect()
  {
    _fliAA->_direct = 10;
    setFlightCounts();
    CPPUNIT_ASSERT_EQUAL(Comparator::TRUE, _comparator->compare(_ptfAA, _ptfBB));
  }
  void testCompare_BBMoreDirect()
  {
    _fliBB->_direct = 10;
    setFlightCounts();
    CPPUNIT_ASSERT_EQUAL(Comparator::FALSE, _comparator->compare(_ptfAA, _ptfBB));
  }
  void testCompare_AAMoreOnline()
  {
    _fliAA->_onlineConnection = 10;
    setFlightCounts();
    CPPUNIT_ASSERT_EQUAL(Comparator::TRUE, _comparator->compare(_ptfAA, _ptfBB));
  }
  void testCompare_BBMoreOnline()
  {
    _fliBB->_onlineConnection = 10;
    setFlightCounts();
    CPPUNIT_ASSERT_EQUAL(Comparator::FALSE, _comparator->compare(_ptfAA, _ptfBB));
  }
  void testCompare_SameAABB()
  {
    setFlightCounts();
    CPPUNIT_ASSERT_EQUAL(Comparator::EQUAL, _comparator->compare(_ptfAA, _ptfBB));
  }

private:
  void setFlightCounts()
  {
    _trx->fdResponse()->scheduleCounts().push_back(_fliAA);
    _trx->fdResponse()->scheduleCounts().push_back(_fliBB);
    _comparator->prepare(*_trx);
  }

  FareDisplayTrx* _trx;
  TestMemHandle _memHandle;
  Comparator* _comparator;
  PaxTypeFare _ptfAA;
  PaxTypeFare _ptfBB;
  FlightCount* _fliAA;
  FlightCount* _fliBB;
};
CPPUNIT_TEST_SUITE_REGISTRATION(ScheduleCountComparatorTest);
}
