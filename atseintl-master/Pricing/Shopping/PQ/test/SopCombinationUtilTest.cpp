#include <boost/optional.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/Diversity.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/LegsBuilder.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{

namespace
{

// =================================
// LEGS DATA
// =================================

DateTime obDate = DateTime(2013, 06, 01);
DateTime ibDate = DateTime(2013, 06, 02);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment segments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "AA", "JFK", "DFW", "LH", DT(obDate, 6), DT(obDate, 7) }, // 1h
  { 0, 1, "LH", "JFK", "DFW", "LH", DT(obDate, 5), DT(obDate, 7) }, // 2h
  { 0, 2, "AA", "JFK", "SYD", "LH", DT(obDate, 5), DT(obDate, 6) }, // 1h
  { 0, 2, "AA", "SYD", "DFW", "LH", DT(obDate, 7), DT(obDate, 8) }, // 1h
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 12), DT(ibDate, 13) } // 1h
};
#undef DT

} // anonymous namespace

class SopCombinationUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SopCombinationUtilTest);

  CPPUNIT_TEST(testDetectCarrierOneWay);
  CPPUNIT_TEST(testDetectCarrierRoundTrip);
  CPPUNIT_TEST(testDetectCarrierRoundTripInterline);

  CPPUNIT_TEST(testDetectCarrierPairOneWay);
  CPPUNIT_TEST(testDetectCarrierPairRoundTrip);

  CPPUNIT_TEST(testGetSopsOneWay);
  CPPUNIT_TEST(testGetSopsRoundTrip);

  CPPUNIT_TEST(testGetDurationOneWayOneHour);
  CPPUNIT_TEST(testGetDurationOneWayTwoHours);
  CPPUNIT_TEST(testGetDurationRoundTripTwoHours);
  CPPUNIT_TEST(testGetDurationRoundTripThreeHours);

  CPPUNIT_TEST(testDetectNonStopOneWay);
  CPPUNIT_TEST(testDetectNonStopOneWayNotANonStop);
  CPPUNIT_TEST(testDetectNonStopRoundTripOnline);
  CPPUNIT_TEST(testDetectNonStopRoundTripInterline);
  CPPUNIT_TEST(testDetectNonStopRoundTripNotANonStop);

  CPPUNIT_TEST(testGetDiagNonStopTypeOnline);
  CPPUNIT_TEST(testGetDiagNonStopTypeInterline);
  CPPUNIT_TEST(testGetDiagNonStopTypeNotANonStop);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = TestShoppingTrxFactory::create(
        "/vobs/atseintl/Pricing/Shopping/PQ/test/testdata/ShoppingNGSTrx.xml", true);
    TSE_ASSERT(_trx);

    // init legs
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(segments, boost::size(segments));
    builder.endBuilding();
  }

  void tearDown() { _memHandle.clear(); }

  void testDetectCarrierOneWay()
  {
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"),
                         SopCombinationUtil::detectCarrier(&_trx->legs()[0].sop()[0], 0));

    shpq::SopIdxVec sopVec(1);
    sopVec[0] = 0;

    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), SopCombinationUtil::detectCarrier(*_trx, sopVec));

    boost::optional<CarrierCode> optCarrierNotInitialized(boost::none);
    boost::optional<CarrierCode> optCarrierAA("AA");
    boost::optional<CarrierCode> optCarrierLH("LH");

    SopCombinationUtil::detectCarrier(&_trx->legs()[0].sop()[0], 0, optCarrierNotInitialized);

    SopCombinationUtil::detectCarrier(&_trx->legs()[0].sop()[0], 0, optCarrierAA);

    SopCombinationUtil::detectCarrier(&_trx->legs()[0].sop()[0], 0, optCarrierLH);

    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), optCarrierNotInitialized.get_value_or("--"));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), optCarrierAA.get());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), optCarrierLH.get());
  }

  void testDetectCarrierRoundTrip()
  {
    CPPUNIT_ASSERT_EQUAL(
        CarrierCode("LH"),
        SopCombinationUtil::detectCarrier(&_trx->legs()[0].sop()[1], &_trx->legs()[1].sop()[0]));

    shpq::SopIdxVec sopVec(2);
    sopVec[0] = 1;
    sopVec[1] = 0;

    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), SopCombinationUtil::detectCarrier(*_trx, sopVec));

    boost::optional<CarrierCode> optCarrierNotInitialized(boost::none);
    boost::optional<CarrierCode> optCarrierAA("AA");
    boost::optional<CarrierCode> optCarrierLH("LH");

    SopCombinationUtil::detectCarrier(
        &_trx->legs()[0].sop()[1], &_trx->legs()[1].sop()[0], optCarrierNotInitialized);

    SopCombinationUtil::detectCarrier(
        &_trx->legs()[0].sop()[1], &_trx->legs()[1].sop()[0], optCarrierAA);

    SopCombinationUtil::detectCarrier(
        &_trx->legs()[0].sop()[1], &_trx->legs()[1].sop()[0], optCarrierLH);

    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), optCarrierNotInitialized.get_value_or("--"));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), optCarrierAA.get());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), optCarrierLH.get());
  }

  void testDetectCarrierRoundTripInterline()
  {
    CPPUNIT_ASSERT_EQUAL(
        Diversity::INTERLINE_CARRIER,
        SopCombinationUtil::detectCarrier(&_trx->legs()[0].sop()[0], &_trx->legs()[1].sop()[0]));

    shpq::SopIdxVec sopVec(2);
    sopVec[0] = 0;
    sopVec[1] = 0;

    CPPUNIT_ASSERT_EQUAL(Diversity::INTERLINE_CARRIER,
                         SopCombinationUtil::detectCarrier(*_trx, sopVec));

    boost::optional<CarrierCode> optCarrierNotInitialized(boost::none);
    boost::optional<CarrierCode> optCarrierAA("AA");
    boost::optional<CarrierCode> optCarrierLH("LH");

    SopCombinationUtil::detectCarrier(
        &_trx->legs()[0].sop()[0], &_trx->legs()[1].sop()[0], optCarrierNotInitialized);

    SopCombinationUtil::detectCarrier(
        &_trx->legs()[0].sop()[0], &_trx->legs()[1].sop()[0], optCarrierAA);

    SopCombinationUtil::detectCarrier(
        &_trx->legs()[0].sop()[0], &_trx->legs()[1].sop()[0], optCarrierLH);

    CPPUNIT_ASSERT_EQUAL(Diversity::INTERLINE_CARRIER, optCarrierNotInitialized.get_value_or("--"));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), optCarrierAA.get());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), optCarrierLH.get());
  }

  void testDetectCarrierPairOneWay()
  {
    shpq::SopIdxVec sopVec(1);
    sopVec[0] = 0;

    std::pair<CarrierCode, CarrierCode> carrierPair =
        SopCombinationUtil::detectCarrierPair(*_trx, sopVec);

    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), carrierPair.first);
    CPPUNIT_ASSERT_EQUAL(Diversity::INTERLINE_CARRIER, carrierPair.second);
  }

  void testDetectCarrierPairRoundTrip()
  {
    shpq::SopIdxVec sopVec(2);
    sopVec[0] = 0;
    sopVec[1] = 0;

    std::pair<CarrierCode, CarrierCode> carrierPair =
        SopCombinationUtil::detectCarrierPair(*_trx, sopVec);

    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), carrierPair.first);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), carrierPair.second);
  }

  void testGetSopsOneWay()
  {
    const ShoppingTrx::SchedulingOption* outbound = 0;
    const ShoppingTrx::SchedulingOption* inbound = 0;

    shpq::SopIdxVec sopVec(1);
    sopVec[0] = 0;

    SopCombinationUtil::getSops(*_trx, sopVec, &outbound, &inbound);

    CPPUNIT_ASSERT(&_trx->legs()[0].sop()[0] == outbound);
    CPPUNIT_ASSERT(0 == inbound);
  }

  void testGetSopsRoundTrip()
  {
    const ShoppingTrx::SchedulingOption* outbound = 0;
    const ShoppingTrx::SchedulingOption* inbound = 0;

    shpq::SopIdxVec sopVec(1);
    sopVec[0] = 0;
    sopVec[1] = 0;

    SopCombinationUtil::getSops(*_trx, sopVec, &outbound, &inbound);

    CPPUNIT_ASSERT(&_trx->legs()[0].sop()[0] == outbound);
    CPPUNIT_ASSERT(&_trx->legs()[1].sop()[0] == inbound);
  }

  void testGetDurationOneWayOneHour()
  {
    CPPUNIT_ASSERT_EQUAL(int32_t(60),
                         SopCombinationUtil::getDuration(&_trx->legs()[0].sop()[0], 0));
  }

  void testGetDurationOneWayTwoHours()
  {
    CPPUNIT_ASSERT_EQUAL(int32_t(120),
                         SopCombinationUtil::getDuration(&_trx->legs()[0].sop()[1], 0));
  }

  void testGetDurationRoundTripTwoHours()
  {
    CPPUNIT_ASSERT_EQUAL(
        int32_t(120),
        SopCombinationUtil::getDuration(&_trx->legs()[0].sop()[0], &_trx->legs()[1].sop()[0]));
  }

  void testGetDurationRoundTripThreeHours()
  {
    CPPUNIT_ASSERT_EQUAL(
        int32_t(180),
        SopCombinationUtil::getDuration(&_trx->legs()[0].sop()[1], &_trx->legs()[1].sop()[0]));
  }

  void testDetectNonStopOneWay()
  {
    CPPUNIT_ASSERT_EQUAL(SopCombinationUtil::ONLINE_NON_STOP,
                         SopCombinationUtil::detectNonStop(&_trx->legs()[0].sop()[1], 0));

    shpq::SopIdxVec sopVec(1);
    sopVec[0] = 1;

    CPPUNIT_ASSERT_EQUAL(SopCombinationUtil::ONLINE_NON_STOP,
                         SopCombinationUtil::detectNonStop(*_trx, sopVec));
  }

  void testDetectNonStopOneWayNotANonStop()
  {
    CPPUNIT_ASSERT_EQUAL(SopCombinationUtil::NOT_A_NON_STOP,
                         SopCombinationUtil::detectNonStop(&_trx->legs()[0].sop()[2], 0));

    shpq::SopIdxVec sopVec(1);
    sopVec[0] = 2;

    CPPUNIT_ASSERT_EQUAL(SopCombinationUtil::NOT_A_NON_STOP,
                         SopCombinationUtil::detectNonStop(*_trx, sopVec));
  }

  void testDetectNonStopRoundTripOnline()
  {
    CPPUNIT_ASSERT_EQUAL(
        SopCombinationUtil::ONLINE_NON_STOP,
        SopCombinationUtil::detectNonStop(&_trx->legs()[0].sop()[1], &_trx->legs()[1].sop()[0]));

    shpq::SopIdxVec sopVec(2);
    sopVec[0] = 1;
    sopVec[1] = 0;

    CPPUNIT_ASSERT_EQUAL(SopCombinationUtil::ONLINE_NON_STOP,
                         SopCombinationUtil::detectNonStop(*_trx, sopVec));
  }

  void testDetectNonStopRoundTripInterline()
  {
    CPPUNIT_ASSERT_EQUAL(
        SopCombinationUtil::INTERLINE_NON_STOP,
        SopCombinationUtil::detectNonStop(&_trx->legs()[0].sop()[0], &_trx->legs()[1].sop()[0]));

    shpq::SopIdxVec sopVec(2);
    sopVec[0] = 0;
    sopVec[1] = 0;

    CPPUNIT_ASSERT_EQUAL(SopCombinationUtil::INTERLINE_NON_STOP,
                         SopCombinationUtil::detectNonStop(*_trx, sopVec));
  }

  void testDetectNonStopRoundTripNotANonStop()
  {
    CPPUNIT_ASSERT_EQUAL(
        SopCombinationUtil::NOT_A_NON_STOP,
        SopCombinationUtil::detectNonStop(&_trx->legs()[0].sop()[2], &_trx->legs()[1].sop()[0]));

    shpq::SopIdxVec sopVec(2);
    sopVec[0] = 2;
    sopVec[1] = 0;

    CPPUNIT_ASSERT_EQUAL(SopCombinationUtil::NOT_A_NON_STOP,
                         SopCombinationUtil::detectNonStop(*_trx, sopVec));
  }

  void testGetDiagNonStopTypeOnline()
  {
    CPPUNIT_ASSERT_EQUAL(
        SopCombinationUtil::DIAG_ONLINE_NON_STOP,
        SopCombinationUtil::getDiagNonStopType(SopCombinationUtil::ONLINE_NON_STOP));

    CPPUNIT_ASSERT_EQUAL(SopCombinationUtil::DIAG_ONLINE_NON_STOP,
                         SopCombinationUtil::getDiagNonStopType(&_trx->legs()[0].sop()[1], 0));

    CPPUNIT_ASSERT_EQUAL(SopCombinationUtil::DIAG_ONLINE_NON_STOP,
                         SopCombinationUtil::getDiagNonStopType(&_trx->legs()[0].sop()[1],
                                                                &_trx->legs()[1].sop()[0]));
  }

  void testGetDiagNonStopTypeInterline()
  {
    CPPUNIT_ASSERT_EQUAL(
        SopCombinationUtil::DIAG_INTERLINE_NON_STOP,
        SopCombinationUtil::getDiagNonStopType(SopCombinationUtil::INTERLINE_NON_STOP));

    CPPUNIT_ASSERT_EQUAL(SopCombinationUtil::DIAG_INTERLINE_NON_STOP,
                         SopCombinationUtil::getDiagNonStopType(&_trx->legs()[0].sop()[0],
                                                                &_trx->legs()[1].sop()[0]));
  }

  void testGetDiagNonStopTypeNotANonStop()
  {
    CPPUNIT_ASSERT_EQUAL(
        SopCombinationUtil::DIAG_NOT_A_NON_STOP,
        SopCombinationUtil::getDiagNonStopType(SopCombinationUtil::NOT_A_NON_STOP));

    CPPUNIT_ASSERT_EQUAL(SopCombinationUtil::DIAG_NOT_A_NON_STOP,
                         SopCombinationUtil::getDiagNonStopType(&_trx->legs()[0].sop()[2], 0));

    CPPUNIT_ASSERT_EQUAL(SopCombinationUtil::DIAG_NOT_A_NON_STOP,
                         SopCombinationUtil::getDiagNonStopType(&_trx->legs()[0].sop()[2],
                                                                &_trx->legs()[1].sop()[0]));
  }

private:
  TestMemHandle _memHandle;
  ShoppingTrx* _trx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SopCombinationUtilTest);

} // tse
