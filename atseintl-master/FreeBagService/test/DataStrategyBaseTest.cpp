#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/Diag852Collector.h"
#include "Diagnostic/test/Diag852ParsedParamsTester.h"
#include "FreeBagService/DataStrategyBase.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "test/include/TestMemHandle.h"

#include <boost/assign/std/vector.hpp>

namespace tse
{
using boost::assign::operator+=;

namespace
{
// MOCKS
class TestDataStrategy : public DataStrategyBase
{
public:
  TestDataStrategy(PricingTrx& trx) : DataStrategyBase(trx) {};

  virtual void processBaggageTravel(BaggageTravel* /*baggageTravels*/,
                                    const BaggageTravelInfo& /*bgIndex*/,
                                    const CheckedPoint& /*furthestCheckedPoint*/,
                                    BaggageTripType /* btt*/,
                                    Diag852Collector* /*dc*/) const {};
};

class TestDiagCollector : public Diag852Collector
{
public:
  TestDiagCollector() : _paramsTester(Diag852Collector::_params)
  {
    rootDiag() = _memHandle.create<Diagnostic>();
  }

  void addParam(const std::string& key, const std::string& value)
  {
    rootDiag()->diagParamMap().insert(std::make_pair(key, value));
    _paramsTester.forceInitialization();
  }

private:
  TestMemHandle _memHandle;
  Diag852ParsedParamsTester _paramsTester;
};
}

class DataStrategyBaseTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DataStrategyBaseTest);

  CPPUNIT_TEST(testCheckFareLineAndCheckedPortion_True_NoCp);
  CPPUNIT_TEST(testCheckFareLineAndCheckedPortion_True_Cp);
  CPPUNIT_TEST(testCheckFareLineAndCheckedPortion_False_NoDiag);
  CPPUNIT_TEST(testCheckFareLineAndCheckedPortion_False_Fl);
  CPPUNIT_TEST(testCheckFareLineAndCheckedPortion_False_Cp);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _itin = _memHandle.create<Itin>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->itin() += _itin;
    _strategy = _memHandle.insert(new TestDataStrategy(*_trx));
  }

  void tearDown() { _memHandle.clear(); }

  BaggageTravel* createBaggageTravel(FarePath* fp = 0)
  {
    if (!fp)
    {
      fp = _memHandle.create<FarePath>();
      _itin->farePath() += fp;
      fp->itin() = _itin;
    }

    return BaggageTravelBuilder(&_memHandle).withFarePath(fp).build();
  }

  std::vector<BaggageTravel*>* create1ItemBaggageTravelVector()
  {
    std::vector<BaggageTravel*>* bagTvls = _memHandle.create<std::vector<BaggageTravel*> >();
    *bagTvls += createBaggageTravel();
    return bagTvls;
  }

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  Itin* _itin;
  DataStrategyBase* _strategy;

public:
  // TESTS
  void testCheckFareLineAndCheckedPortion_True_NoCp()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "01");
    std::vector<BaggageTravel*>* bagTvls = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(
        _strategy->checkFareLineAndCheckedPortion((*bagTvls)[0], BaggageTravelInfo(0, 0), &diag));
  }

  void testCheckFareLineAndCheckedPortion_True_Cp()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "01");
    diag.addParam("CP", "01");
    std::vector<BaggageTravel*>* bagTvls = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(
        _strategy->checkFareLineAndCheckedPortion((*bagTvls)[0], BaggageTravelInfo(0, 0), &diag));
  }

  void testCheckFareLineAndCheckedPortion_False_NoDiag()
  {
    std::vector<BaggageTravel*>* bagTvls = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(
        !_strategy->checkFareLineAndCheckedPortion((*bagTvls)[0], BaggageTravelInfo(0, 0), 0));
  }

  void testCheckFareLineAndCheckedPortion_False_Fl()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "02");
    std::vector<BaggageTravel*>* bagTvls = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(
        !_strategy->checkFareLineAndCheckedPortion((*bagTvls)[0], BaggageTravelInfo(0, 0), &diag));
  }

  void testCheckFareLineAndCheckedPortion_False_Cp()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "01");
    diag.addParam("CP", "02");
    std::vector<BaggageTravel*>* bagTvls = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(
        !_strategy->checkFareLineAndCheckedPortion((*bagTvls)[0], BaggageTravelInfo(0, 0), &diag));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(DataStrategyBaseTest);
} // tse
