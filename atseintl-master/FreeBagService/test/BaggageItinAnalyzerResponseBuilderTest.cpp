#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "FreeBagService/BaggageItinAnalyzer.h"
#include "FreeBagService/BaggageItinAnalyzerResponseBuilder.h"
#include "FreeBagService/test/AirSegBuilder.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestItinFactory.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
using boost::assign::operator+=;

class BaggageItinAnalyzerResponseBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageItinAnalyzerResponseBuilderTest);

  CPPUNIT_TEST(test_printCheckedPoint_AtOrigin);
  CPPUNIT_TEST(test_printCheckedPoint_AtDestination);
  CPPUNIT_TEST(test_printFurthestCheckedPoint);
  CPPUNIT_TEST(test_printOriginAndDestination);
  CPPUNIT_TEST(test_printOriginAndDestination_Arunks);
  CPPUNIT_TEST(test_printBaggageTravel);
  CPPUNIT_TEST(test_printBaggageTravels);

  CPPUNIT_TEST_SUITE_END();

private:
  std::string _ut_data_path;
  TestMemHandle _memHandle;
  AncillaryPricingTrx* _trx;
  Itin* _itin;
  FarePath* _farePath;
  BaggageItinAnalyzer* _analyzer;
  BaggageItinAnalyzerResponseBuilder* _builder;
  std::vector<TravelSeg*> _segments;

public:
  BaggageItinAnalyzerResponseBuilderTest() { _ut_data_path = "/vobs/atseintl/test/testdata/data/"; }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<AncillaryPricingTrx>();
    _itin = TestItinFactory::create(_ut_data_path + "ItinGDL_SFO.xml");
    _farePath = _memHandle.create<FarePath>();
    _farePath->itin() = _itin;
    _itin->farePath() += _farePath;
    _analyzer = _memHandle.create<BaggageItinAnalyzer>(*_trx, *_itin);
    _builder = _memHandle.create<BaggageItinAnalyzerResponseBuilder>(*_analyzer);
    _segments.push_back(TestAirSegFactory::create(_ut_data_path + "AirSegDFW_LAX.xml"));
    _segments.push_back(TestAirSegFactory::create(_ut_data_path + "AirSegLAX_SFO.xml"));
  }

  void tearDown() { _memHandle.clear(); }

private:
  void test_printCheckedPoint_AtOrigin()
  {
    std::vector<TravelSeg*> segments;
    segments.push_back(TestAirSegFactory::create(_ut_data_path + "AirSegDFW_LAX.xml"));

    CheckedPoint cp;
    cp.first = segments.begin();
    cp.second = CP_AT_ORIGIN;

    _builder->printCheckedPoint(cp);
    CPPUNIT_ASSERT_EQUAL(std::string("DFW"), _builder->_output.str());
  }

  void test_printCheckedPoint_AtDestination()
  {
    std::vector<TravelSeg*> segments;
    segments.push_back(TestAirSegFactory::create(_ut_data_path + "AirSegDFW_LAX.xml"));

    CheckedPoint cp;
    cp.first = segments.begin();
    cp.second = CP_AT_DESTINATION;

    _builder->printCheckedPoint(cp);
    CPPUNIT_ASSERT_EQUAL(std::string("LAX"), _builder->_output.str());
  }

  void test_printFurthestCheckedPoint()
  {
    std::vector<TravelSeg*> segments;
    segments.push_back(TestAirSegFactory::create(_ut_data_path + "AirSegDFW_LAX.xml"));

    CheckedPoint cp;
    cp.first = segments.begin();
    cp.second = CP_AT_DESTINATION;
    CheckedPointVector cpv;

    _builder->printFurthestCheckedPoint(cpv, cp);
    CPPUNIT_ASSERT_EQUAL(std::string("LAX\n"), _builder->_output.str());
  }

  void test_printOriginAndDestination()
  {
    Itin* itin = TestItinFactory::create(_ut_data_path + "ItinGDL_SFO.xml");
    _builder->printOriginAndDestination(*itin);

    CPPUNIT_ASSERT_EQUAL(std::string("ORIGIN : GDL\nDESTINATION : SFO\n"), _builder->_output.str());
  }

  void test_printOriginAndDestination_Arunks()
  {
    Itin* itin = TestItinFactory::create(_ut_data_path + "ItinDFW_LAX_Arunk_02.xml");
    _builder->printOriginAndDestination(*itin);

    CPPUNIT_ASSERT_EQUAL(std::string("ORIGIN : DFW\nDESTINATION : DFW\n"), _builder->_output.str());
  }

  void test_printBaggageTravel()
  {
    BaggageTravel* bt = BaggageTravelBuilder(&_memHandle)
                            .withFarePath(_farePath)
                            .withTravelSegMore(_segments)
                            .withTrx(_trx)
                            .build();
    CheckedPointVector cpv;

    _builder->printBaggageTravel(bt, cpv, true, 1);

    std::string expected = "01 BAGGAGE TRAVEL : DFW-LAX-SFO\n"
                           "MSC MARKETING : AA\n"
                           "MSC OPERATING : \n"
                           "FCIC MARKETING : Y4\n"
                           "FCIC OPERATING : \n"
                           "STOPOVER POINTS : \n";

    CPPUNIT_ASSERT_EQUAL(expected, _builder->_output.str());
  }

  void test_printBaggageTravels()
  {
    std::vector<BaggageTravel*> bts;
    bts.push_back(BaggageTravelBuilder(&_memHandle)
                      .withFarePath(_farePath)
                      .withTravelSegMore(_segments)
                      .withTrx(_trx)
                      .build());
    bts.push_back(BaggageTravelBuilder(&_memHandle)
                      .withFarePath(_farePath)
                      .withTravelSegMore(_segments)
                      .withTrx(_trx)
                      .build());
    CheckedPointVector cpv;

    _builder->printBaggageTravels(bts, cpv, true);

    std::string expected = "01 BAGGAGE TRAVEL : DFW-LAX-SFO\n"
                           "MSC MARKETING : AA\n"
                           "MSC OPERATING : \n"
                           "FCIC MARKETING : Y4\n"
                           "FCIC OPERATING : \n"
                           "STOPOVER POINTS : \n"
                           "--------------------------------------------------------\n"
                           "02 BAGGAGE TRAVEL : DFW-LAX-SFO\n"
                           "MSC MARKETING : AA\n"
                           "MSC OPERATING : \n"
                           "FCIC MARKETING : Y4\n"
                           "FCIC OPERATING : \n"
                           "STOPOVER POINTS : \n";

    CPPUNIT_ASSERT_EQUAL(expected, _builder->_output.str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BaggageItinAnalyzerResponseBuilderTest);

} // tse
