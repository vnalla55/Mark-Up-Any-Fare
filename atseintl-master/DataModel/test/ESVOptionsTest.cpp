#ifndef ESVOPTIONS_UTIL_TEST_H
#define ESVOPTIONS_UTIL_TEST_H

#include <log4cxx/helpers/objectptr.h>
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

#include <iostream>

#include "DataModel/FareMarket.h"
#include "DataModel/ShoppingTrx.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ESVOptions.h"
#include "DataModel/AirSeg.h"
#include "Common/DateTime.h"

namespace tse
{

class ESVOptionsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ESVOptionsTest);
  CPPUNIT_TEST(testESVOptionsAccessors);
  CPPUNIT_TEST(testTotalPenaltiesWhenNoTravSegments);
  CPPUNIT_TEST(testTotalPenaltiesWhenCoupleTravSegments);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    sOption = new ShoppingTrx::SchedulingOption(new Itin(), 53);
    esvOptions = new ESVOptions();
  }
  void tearDown()
  {
    delete sOption;
    delete esvOptions;
  }
  void testESVOptionsAccessors()
  {
    // must price
    int expected_no_must_price_online = 41;
    int expected_no_must_price_interline = 42;
    int expected_no_must_price_nonstop_online = 43;
    int expected_no_must_price_nonstop_interline = 44;
    int expected_no_must_price_onestop_online = 45;
    int expected_no_must_price_nonandone_online = 46;
    int expected_no_must_price_nonandone_interline = 47;
    float expected_percent_factor = 0.11; // Q68
    int expected_flight_option_reuse_limit = 48;
    float expected_upper_bound_factor_nonstop = 3.6; // Q6S
    float expected_upper_bound_factor_not_nonstop = 3.7; // Q6A
    // penalties
    int expected_stop_penalty = 49;
    int expected_dur_penalty = 51;
    int expected_dev_penalty = 52;
    // low fare
    int expected_no_esv_lowfare = 53;
    float expected_esv_percent = 2.4;
    int expected_min_online_per_carrier = 54;
    float expected_online_percent = 2.5;
    int expected_lo_max_per_options = 55;
    int expected_hi_max_per_options = 56;

    float expected_upper_bound_factor_lfs = 3.8; // Q6S

    //------------------------------------------------------

    int no_must_price_online = esvOptions->noOfMustPriceOnlineSolutions();
    int no_must_price_interline = esvOptions->noOfMustPriceInterlineSolutions();
    int no_must_price_nonstop_online = esvOptions->noOfMustPriceNonstopOnlineSolutions();
    int no_must_price_nonstop_interline = esvOptions->noOfMustPriceNonstopInterlineSolutions();
    int no_must_price_onestop_online = esvOptions->noOfMustPriceSingleStopOnlineSolutions();
    int no_must_price_nonandone_online = esvOptions->noOfMustPriceNonAndOneStopOnlineSolutions();
    int no_must_price_nonandone_interline =
        esvOptions->noOfMustPriceNonAndOneStopInterlineSolutions();
    float percent_factor = esvOptions->percentFactor();
    int flight_option_reuse_limit = esvOptions->flightOptionReuseLimit();
    float upper_bound_factor_nonstop = esvOptions->upperBoundFactorForNonstop();
    float upper_bound_factor_not_nonstop = esvOptions->upperBoundFactorForNotNonstop();

    int stop_penalty = esvOptions->perStopPenalty();
    int dur_penalty = esvOptions->travelDurationPenalty();
    int dev_penalty = esvOptions->departureTimeDeviationPenalty();

    int no_esv_lowfare = esvOptions->noOfESVLowFareSolutionsReq();
    float esv_percent = esvOptions->esvPercent();
    int min_online_per_carrier = esvOptions->noOfMinOnlinePerCarrier();
    float online_percent = esvOptions->onlinePercent();
    int lo_max_per_options = esvOptions->loMaximumPerOption();
    int hi_max_per_options = esvOptions->hiMaximumPerOption();

    float upper_bound_factor_lfs = esvOptions->upperBoundFactorForLFS();

    //-----------------------------------------------------------------------------------
    int default_no_must_price_online = ESVOptions::DEFAULT_NUM_MUST_PRICE_ONLINE;
    int default_no_must_price_interline = ESVOptions::DEFAULT_NUM_MUST_PRICE_INTERLINE;
    int default_no_must_price_nonstop_online = ESVOptions::DEFAULT_NUM_MUST_PRICE_NONSTOP_ONLINE;
    int default_no_must_price_nonstop_interline =
        ESVOptions::DEFAULT_NUM_MUST_PRICE_NONSTOP_INTERLINE;
    int default_no_must_price_onestop_online = ESVOptions::DEFAULT_NUM_MUST_PRICE_ONESTOP_ONLINE;
    int default_no_must_price_nonandone_online =
        ESVOptions::DEFAULT_NUM_MUST_PRICE_NONANDONE_ONLINE;
    int default_no_must_price_nonandone_interline =
        ESVOptions::DEFAULT_NUM_MUST_PRICE_NONANDONE_INTERLINE;
    float default_percent_factor = ESVOptions::DEFAULT_MAX_ALLOWED_OVERAGE_PER_CARRIER;
    int default_flight_option_reuse_limit = ESVOptions::DEFAULT_FLIGHT_OPTION_REUSE_LIMIT;
    float default_upper_bound_factor_nonstop = ESVOptions::DEFAULT_UPPER_BOUND_FACTOR_NONSTOP;
    float default_upper_bound_factor_not_nonstop =
        ESVOptions::DEFAULT_UPPER_BOUND_FACTOR_NOT_NONSTOP;

    int default_stop_penalty = ESVOptions::DEFAULT_STOP_PENALTY;
    int default_trav_dur_penalty = ESVOptions::DEFAULT_TRAV_DUR_PENALTY;
    int default_dep_time_dev_penalty = ESVOptions::DEFAULT_DEP_TIME_DEV_PENALTY;

    int default_no_esv_lowfare = ESVOptions::DEFAULT_NUM_ESV_LOWFARE;
    float default_esv_percent = ESVOptions::DEFAULT_MAX_ALLOWED_OVERAGE_PER_CARRIER_LFS; // Q5T
    int default_min_online_per_carrier = ESVOptions::DEFAULT_NUM_MIN_ONLINE_PER_CARRIER; // Q5U
    float default_online_percent = ESVOptions::DEFAULT_MIN_PERCENT_OF_ONLINE; // Q5V
    int default_lo_max_per_options = ESVOptions::DEFAULT_FLIGHT_OPTION_REUSE_FOR_AVS; // Q5X
    int default_hi_max_per_options = ESVOptions::DEFAULT_FLIGHT_OPTION_REUSE_FOR_NONAVS; // Q5W

    float default_upper_bound_factor_lfs = ESVOptions::DEFAULT_UPPER_BOUND_FACTOR_LFS;

    //--------------------------------------------------------------------------------------
    CPPUNIT_ASSERT_EQUAL(default_no_must_price_online, no_must_price_online);
    CPPUNIT_ASSERT_EQUAL(default_no_must_price_interline, no_must_price_interline);
    CPPUNIT_ASSERT_EQUAL(default_no_must_price_nonstop_online, no_must_price_nonstop_online);
    CPPUNIT_ASSERT_EQUAL(default_no_must_price_nonstop_interline, no_must_price_nonstop_interline);
    CPPUNIT_ASSERT_EQUAL(default_no_must_price_onestop_online, no_must_price_onestop_online);
    CPPUNIT_ASSERT_EQUAL(default_no_must_price_nonandone_online, no_must_price_nonandone_online);
    CPPUNIT_ASSERT_EQUAL(default_no_must_price_nonandone_interline,
                         no_must_price_nonandone_interline);
    CPPUNIT_ASSERT_EQUAL(default_percent_factor, percent_factor);
    CPPUNIT_ASSERT_EQUAL(default_flight_option_reuse_limit, flight_option_reuse_limit);
    CPPUNIT_ASSERT_EQUAL(default_upper_bound_factor_nonstop, upper_bound_factor_nonstop);
    CPPUNIT_ASSERT_EQUAL(default_upper_bound_factor_not_nonstop, upper_bound_factor_not_nonstop);

    CPPUNIT_ASSERT_EQUAL(default_stop_penalty, stop_penalty);
    CPPUNIT_ASSERT_EQUAL(default_trav_dur_penalty, dur_penalty);
    CPPUNIT_ASSERT_EQUAL(default_dep_time_dev_penalty, dev_penalty);

    CPPUNIT_ASSERT_EQUAL(default_no_esv_lowfare, no_esv_lowfare);
    CPPUNIT_ASSERT_EQUAL(default_esv_percent, esv_percent);
    CPPUNIT_ASSERT_EQUAL(default_min_online_per_carrier, min_online_per_carrier);
    CPPUNIT_ASSERT_EQUAL(default_online_percent, online_percent);
    CPPUNIT_ASSERT_EQUAL(default_lo_max_per_options, lo_max_per_options);
    CPPUNIT_ASSERT_EQUAL(default_hi_max_per_options, hi_max_per_options);

    CPPUNIT_ASSERT_EQUAL(default_upper_bound_factor_lfs, upper_bound_factor_lfs);

    //---------------------------------------------------------------------------------------
    esvOptions->noOfMustPriceOnlineSolutions() = expected_no_must_price_online;
    esvOptions->noOfMustPriceInterlineSolutions() = expected_no_must_price_interline;
    esvOptions->noOfMustPriceNonstopOnlineSolutions() = expected_no_must_price_nonstop_online;
    esvOptions->noOfMustPriceNonstopInterlineSolutions() = expected_no_must_price_nonstop_interline;
    esvOptions->noOfMustPriceSingleStopOnlineSolutions() = expected_no_must_price_onestop_online;
    esvOptions->noOfMustPriceNonAndOneStopOnlineSolutions() =
        expected_no_must_price_nonandone_online;
    esvOptions->noOfMustPriceNonAndOneStopInterlineSolutions() =
        expected_no_must_price_nonandone_interline;
    esvOptions->percentFactor() = expected_percent_factor;
    esvOptions->flightOptionReuseLimit() = expected_flight_option_reuse_limit;
    esvOptions->upperBoundFactorForNonstop() = expected_upper_bound_factor_nonstop;
    esvOptions->upperBoundFactorForNotNonstop() = expected_upper_bound_factor_not_nonstop;

    esvOptions->perStopPenalty() = expected_stop_penalty;
    esvOptions->travelDurationPenalty() = expected_dur_penalty;
    esvOptions->departureTimeDeviationPenalty() = expected_dev_penalty;

    esvOptions->noOfESVLowFareSolutionsReq() = expected_no_esv_lowfare;
    esvOptions->esvPercent() = expected_esv_percent;
    esvOptions->noOfMinOnlinePerCarrier() = expected_min_online_per_carrier;
    esvOptions->onlinePercent() = expected_online_percent;
    esvOptions->loMaximumPerOption() = expected_lo_max_per_options;
    esvOptions->hiMaximumPerOption() = expected_hi_max_per_options;

    esvOptions->upperBoundFactorForLFS() = expected_upper_bound_factor_lfs;

    //-----------------------------------------------------------------------------------------
    no_must_price_online = esvOptions->noOfMustPriceOnlineSolutions();
    no_must_price_interline = esvOptions->noOfMustPriceInterlineSolutions();
    no_must_price_nonstop_online = esvOptions->noOfMustPriceNonstopOnlineSolutions();
    no_must_price_nonstop_interline = esvOptions->noOfMustPriceNonstopInterlineSolutions();
    no_must_price_onestop_online = esvOptions->noOfMustPriceSingleStopOnlineSolutions();
    no_must_price_nonandone_online = esvOptions->noOfMustPriceNonAndOneStopOnlineSolutions();
    no_must_price_nonandone_interline = esvOptions->noOfMustPriceNonAndOneStopInterlineSolutions();
    percent_factor = esvOptions->percentFactor();
    flight_option_reuse_limit = esvOptions->flightOptionReuseLimit();
    upper_bound_factor_nonstop = esvOptions->upperBoundFactorForNonstop();
    upper_bound_factor_not_nonstop = esvOptions->upperBoundFactorForNotNonstop();

    stop_penalty = esvOptions->perStopPenalty();
    dur_penalty = esvOptions->travelDurationPenalty();
    dev_penalty = esvOptions->departureTimeDeviationPenalty();

    no_esv_lowfare = esvOptions->noOfESVLowFareSolutionsReq();
    esv_percent = esvOptions->esvPercent();
    min_online_per_carrier = esvOptions->noOfMinOnlinePerCarrier();
    online_percent = esvOptions->onlinePercent();
    lo_max_per_options = esvOptions->loMaximumPerOption();
    hi_max_per_options = esvOptions->hiMaximumPerOption();

    upper_bound_factor_lfs = esvOptions->upperBoundFactorForLFS();

    //------------------------------------------------------------------------------------------------
    CPPUNIT_ASSERT_EQUAL(expected_no_must_price_online, no_must_price_online);
    CPPUNIT_ASSERT_EQUAL(expected_no_must_price_interline, no_must_price_interline);
    CPPUNIT_ASSERT_EQUAL(expected_no_must_price_nonstop_online, no_must_price_nonstop_online);
    CPPUNIT_ASSERT_EQUAL(expected_no_must_price_nonstop_interline, no_must_price_nonstop_interline);
    CPPUNIT_ASSERT_EQUAL(expected_no_must_price_onestop_online, no_must_price_onestop_online);
    CPPUNIT_ASSERT_EQUAL(expected_no_must_price_nonandone_online, no_must_price_nonandone_online);
    CPPUNIT_ASSERT_EQUAL(expected_no_must_price_nonandone_interline,
                         no_must_price_nonandone_interline);
    CPPUNIT_ASSERT_EQUAL(expected_percent_factor, percent_factor);
    CPPUNIT_ASSERT_EQUAL(expected_flight_option_reuse_limit, flight_option_reuse_limit);
    CPPUNIT_ASSERT_EQUAL(expected_upper_bound_factor_nonstop, upper_bound_factor_nonstop);
    CPPUNIT_ASSERT_EQUAL(expected_upper_bound_factor_not_nonstop, upper_bound_factor_not_nonstop);

    CPPUNIT_ASSERT_EQUAL(expected_stop_penalty, stop_penalty);
    CPPUNIT_ASSERT_EQUAL(expected_dur_penalty, dur_penalty);
    CPPUNIT_ASSERT_EQUAL(expected_dev_penalty, dev_penalty);

    CPPUNIT_ASSERT_EQUAL(expected_no_esv_lowfare, no_esv_lowfare);
    CPPUNIT_ASSERT_EQUAL(expected_esv_percent, esv_percent);
    CPPUNIT_ASSERT_EQUAL(expected_min_online_per_carrier, min_online_per_carrier);
    CPPUNIT_ASSERT_EQUAL(expected_online_percent, online_percent);
    CPPUNIT_ASSERT_EQUAL(expected_lo_max_per_options, lo_max_per_options);
    CPPUNIT_ASSERT_EQUAL(expected_hi_max_per_options, hi_max_per_options);

    CPPUNIT_ASSERT_EQUAL(expected_upper_bound_factor_lfs, upper_bound_factor_lfs);
  }
  void testTotalPenaltiesWhenNoTravSegments()
  {

    sOption->itin()->updateTotalPenalty(esvOptions, DateTime::localTime());
    double totalPenalty = sOption->itin()->totalPenalty();
    CPPUNIT_ASSERT_EQUAL(0.0, totalPenalty);
  }

  void testTotalPenaltiesWhenCoupleTravSegments()
  {

    DateTime oneHour = DateTime::localTime().addSeconds(SECONDS_PER_HOUR);
    DateTime twoHours = DateTime::localTime().addSeconds(2 * SECONDS_PER_HOUR);
    DateTime threeAndHalfHours =
        DateTime::localTime().addSeconds(3 * SECONDS_PER_HOUR + SECONDS_PER_HOUR / 2);

    AirSeg travelSegment1 = createOneSegment("LAX", "BOS", DateTime::localTime(), oneHour);
    AirSeg travelSegment2 = createOneSegment("BOS", "DFW", oneHour, twoHours);
    AirSeg travelSegment3 = createOneSegment("DFW", "KRK", twoHours, threeAndHalfHours);
    sOption->itin()->travelSeg().push_back(&travelSegment1);
    sOption->itin()->travelSeg().push_back(&travelSegment2);
    sOption->itin()->travelSeg().push_back(&travelSegment3);

    sOption->itin()->updateTotalPenalty(esvOptions, twoHours);

    float totalPenalty = sOption->itin()->totalPenalty();
    int stopPenalty = esvOptions->perStopPenalty() * 100;
    int durPenalty = esvOptions->travelDurationPenalty();
    int devPenalty = esvOptions->departureTimeDeviationPenalty();

    // the equation is total = (numberofhours* duration penalty) + (difference in expected departure
    // time*devPenalty) + (stop_penalty*number of stops)
    float expectedTotalPenalty = (3.5 * durPenalty + 2.0 * devPenalty + 2.0 * stopPenalty) / 100;
    CPPUNIT_ASSERT_EQUAL(expectedTotalPenalty, totalPenalty);
  }

private:
  ESVOptions* esvOptions;
  ShoppingTrx::SchedulingOption* sOption;

  AirSeg createOneSegment(std::string origincity, std::string destcity)
  {
    Loc* origin = new Loc();
    origin->loc() = origincity;
    origin->nation() = std::string("US");

    Loc* destination = new Loc();
    destination->loc() = destcity;
    destination->nation() = std::string("PE");

    AirSeg travelSeg;
    travelSeg.origin() = origin;
    travelSeg.destination() = destination;

    return travelSeg;
  }

  AirSeg createOneSegment(std::string origincity,
                          std::string destcity,
                          DateTime departure,
                          DateTime arrival)
  {
    AirSeg travSeg = createOneSegment(origincity, destcity);

    travSeg.departureDT() = departure;
    travSeg.arrivalDT() = arrival;

    return travSeg;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ESVOptionsTest);

} // tse

#endif
