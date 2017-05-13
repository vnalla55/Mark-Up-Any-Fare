#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FcDispFarePath.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <string>

using namespace std;
namespace tse
{
class FcDispFarePathTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FcDispFarePathTest);
  CPPUNIT_TEST(testDisplayMinFarePlusUp_noHRT);
  CPPUNIT_TEST(testDisplayMinFarePlusUp_HRT_exist);
  CPPUNIT_TEST_SUITE_END();

private:
  PricingTrx _trx;
  PricingRequest _request;
  Agent _agent;
  FareCalcConfig _fcConfig;
  FarePath _fp;
  PricingUnit _pu1, _pu2;
  FareCalcCollector _fcCollector;
  CalcTotals _calcTotals;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fp.pricingUnit().push_back(&_pu1);
    _fp.pricingUnit().push_back(&_pu2);
    _request.ticketingDT() = DateTime(2010, 10, 1);
    _request.ticketingAgent() = &_agent;
    _trx.setRequest(&_request);
    _trx.setOptions(_memHandle.create<PricingOptions>());
  }

  void tearDown() { _memHandle.clear(); }

  void testDisplayMinFarePlusUp_noHRT()
  {
    FcDispFarePath fcDisp(_trx, _fp, _fcConfig, _fcCollector, &_calcTotals);

    FareCalc::FcStream fcStream;
    fcDisp.displayMinFarePlusUp(fcStream);
    CPPUNIT_ASSERT(fcStream.str().empty());
  }

  void testDisplayMinFarePlusUp_HRT_exist()
  {
    FcDispFarePath fcDisp(_trx, _fp, _fcConfig, _fcCollector, &_calcTotals);

    MinFarePlusUpItem hrtPlusUp;
    hrtPlusUp.plusUpAmount = 15.00;
    _pu1.minFarePlusUp().addItem(HRT, &hrtPlusUp);
    FareCalc::FcStream fcStream;
    fcDisp.displayMinFarePlusUp(fcStream);
    CPPUNIT_ASSERT(fcStream.str().find("15.00") != string::npos);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FcDispFarePathTest);
};
