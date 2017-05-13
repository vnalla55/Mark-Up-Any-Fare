// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include <memory>
#include "Rules/TaxApplicationLimitProcessor.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tax
{

class TaxApplicationLimitProcessorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxApplicationLimitProcessorTest);
  CPPUNIT_TEST(testAllOnceForItin);
  CPPUNIT_TEST(testAllContinousJourney_1);
  CPPUNIT_TEST(testAllContinousJourney_2);
  CPPUNIT_TEST(testAllSingleJourney_1);
  CPPUNIT_TEST(testAllSingleJourney_2);
  CPPUNIT_TEST(testAllUsRoundTrip_1);
  CPPUNIT_TEST(testAllUsRoundTrip_2);
  CPPUNIT_TEST(testContinousJourneyAndItineraryInLimitedJourney);
  CPPUNIT_TEST(testSingleJourneyAndItineraryInLimitedJourney);
  CPPUNIT_TEST(testUsRoundTripAndItineraryInLimitedJourney);
  CPPUNIT_TEST(testItineraryAndNoRestrictions);
  CPPUNIT_TEST(testContinousJourneyAndNoRestrictions);
  CPPUNIT_TEST(testSingleJourneyAndNoRestrictions);
  CPPUNIT_TEST(testUsRoundTripAndNoRestrictions);
  CPPUNIT_TEST(testCreateLimitedJourney);
  CPPUNIT_TEST(testTaxPointMap);
  CPPUNIT_TEST(testTaxPointMap_1);
  CPPUNIT_TEST_SUITE_END();

  void testAllOnceForItin()
  {
    std::vector<TaxLimitInfo> journey;
    journey.push_back(TaxLimitInfo::onceForItin(0, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::onceForItin(1, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::onceForItin(2, type::MoneyAmount(10, 1)));

    TaxPointMap taxPointMap;
    std::shared_ptr<LimitedJourneyInfo> limitedJourney(LimitedJourneyInfo::create<ItineraryInfo>(
        journey.begin(), journey.end(), journey.begin(), journey.end()));

    std::vector<type::Index> passedRules = limitedJourney->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(1), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 2) != passedRules.end());
  }

  void testAllContinousJourney_1()
  {
    std::vector<TaxLimitInfo> journey;
    journey.push_back(TaxLimitInfo::continousJourney(0, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::continousJourney(1, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::continousJourney(2, type::MoneyAmount(10, 1)));

    TaxPointMap taxPointMap;
    std::shared_ptr<LimitedJourneyInfo> limitedJourney(
        LimitedJourneyInfo::create<ContinuousJourneyInfo>(
            journey.begin(), journey.end(), journey.begin(), journey.end()));

    std::vector<type::Index> passedRules = limitedJourney->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(2), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 0) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 1) != passedRules.end());
  }

  void testAllContinousJourney_2()
  {
    std::vector<TaxLimitInfo> journey;
    journey.push_back(TaxLimitInfo::continousJourney(0, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::continousJourney(1, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::continousJourney(2, type::MoneyAmount(10, 1)));
    journey.push_back(TaxLimitInfo::continousJourney(3, type::MoneyAmount(14, 1)));
    journey.push_back(TaxLimitInfo::continousJourney(4, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::continousJourney(5, type::MoneyAmount(31, 1)));

    TaxPointMap taxPointMap;
    std::shared_ptr<LimitedJourneyInfo> limitedJourney1(
        LimitedJourneyInfo::create<ContinuousJourneyInfo>(
            journey.begin(), journey.end(), journey.begin(), journey.begin() + 3));
    std::shared_ptr<LimitedJourneyInfo> limitedJourney2(
        LimitedJourneyInfo::create<ContinuousJourneyInfo>(
            journey.begin(), journey.end(), journey.begin() + 3, journey.end()));

    std::vector<type::Index> passedRules = limitedJourney1->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(2), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 0) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 1) != passedRules.end());

    passedRules = limitedJourney2->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(2), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 3) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 4) != passedRules.end());
  }

  void testAllSingleJourney_1()
  {
    std::vector<TaxLimitInfo> journey;
    journey.push_back(TaxLimitInfo::singleJourney(0, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::singleJourney(1, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::singleJourney(2, type::MoneyAmount(10, 1)));

    TaxPointMap taxPointMap;
    std::shared_ptr<LimitedJourneyInfo> limitedJourney(
        LimitedJourneyInfo::create<SingleJourneyInfo>(
            journey.begin(), journey.end(), journey.begin(), journey.end()));

    std::vector<type::Index> passedRules = limitedJourney->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(1), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 0) != passedRules.end());
  }

  void testAllSingleJourney_2()
  {
    std::vector<TaxLimitInfo> journey;
    journey.push_back(TaxLimitInfo::singleJourney(0, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::singleJourney(1, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::singleJourney(2, type::MoneyAmount(10, 1)));
    journey.push_back(TaxLimitInfo::singleJourney(3, type::MoneyAmount(2, 1)));
    journey.push_back(TaxLimitInfo::singleJourney(4, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::singleJourney(5, type::MoneyAmount(8, 1)));

    TaxPointMap taxPointMap;
    std::shared_ptr<LimitedJourneyInfo> limitedJourney1(
        LimitedJourneyInfo::create<SingleJourneyInfo>(
            journey.begin(), journey.end(), journey.begin(), journey.begin() + 3));
    std::shared_ptr<LimitedJourneyInfo> limitedJourney2(
        LimitedJourneyInfo::create<SingleJourneyInfo>(
            journey.begin(), journey.end(), journey.begin() + 3, journey.end()));

    std::vector<type::Index> passedRules = limitedJourney1->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(1), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 0) != passedRules.end());

    passedRules = limitedJourney2->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(1), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 3) != passedRules.end());
  }

  void testAllUsRoundTrip_1()
  {
    std::vector<TaxLimitInfo> journey;
    journey.push_back(TaxLimitInfo::usRoundTrip(0, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(1, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(2, type::MoneyAmount(10, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(3, type::MoneyAmount(3, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(4, type::MoneyAmount(13, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(5, type::MoneyAmount(1, 1)));

    TaxPointMap taxPointMap;
    std::shared_ptr<LimitedJourneyInfo> limitedJourney(LimitedJourneyInfo::create<UsRoundTripInfo>(
        journey.begin(), journey.end(), journey.begin(), journey.end()));

    std::vector<type::Index> passedRules = limitedJourney->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(2), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 0) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 1) != passedRules.end());
  }

  void testAllUsRoundTrip_2()
  {
    std::vector<TaxLimitInfo> journey;
    journey.push_back(TaxLimitInfo::usRoundTrip(0, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(1, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(2, type::MoneyAmount(10, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(3, type::MoneyAmount(3, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(4, type::MoneyAmount(13, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(5, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(6, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(7, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(8, type::MoneyAmount(10, 1)));

    TaxPointMap taxPointMap;
    std::shared_ptr<LimitedJourneyInfo> limitedJourney1(LimitedJourneyInfo::create<UsRoundTripInfo>(
        journey.begin(), journey.end(), journey.begin(), journey.begin() + 3));
    std::shared_ptr<LimitedJourneyInfo> limitedJourney2(LimitedJourneyInfo::create<UsRoundTripInfo>(
        journey.begin(), journey.end(), journey.begin() + 3, journey.end()));

    std::vector<type::Index> passedRules = limitedJourney1->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(2), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 0) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 1) != passedRules.end());

    passedRules = limitedJourney2->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(2), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 3) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 4) != passedRules.end());
  }

  void testContinousJourneyAndItineraryInLimitedJourney()
  {
    std::vector<TaxLimitInfo> journey;
    journey.push_back(TaxLimitInfo::onceForItin(0, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::continousJourney(1, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::continousJourney(2, type::MoneyAmount(10, 1)));

    TaxPointMap taxPointMap;

    std::shared_ptr<LimitedJourneyInfo> limitedJourney1(
        LimitedJourneyInfo::create<ContinuousJourneyInfo>(
            journey.begin(), journey.end(), journey.begin() + 1, journey.end()));

    std::shared_ptr<LimitedJourneyInfo> limitedJourney2(LimitedJourneyInfo::create<ItineraryInfo>(
        journey.begin(), journey.end(), journey.begin(), journey.end()));

    taxPointMap.addJourney(std::vector<Trip>(1, std::make_pair(1, 2)));
    limitedJourney2->setTaxPointMap(taxPointMap);

    std::vector<type::Index> passedRules = limitedJourney1->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(2), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 1) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 2) != passedRules.end());

    passedRules = limitedJourney2->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(1), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 0) != passedRules.end());
  }

  void testSingleJourneyAndItineraryInLimitedJourney()
  {
    std::vector<TaxLimitInfo> journey;
    journey.push_back(TaxLimitInfo::onceForItin(0, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::singleJourney(1, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::singleJourney(2, type::MoneyAmount(10, 1)));

    TaxPointMap taxPointMap;

    std::shared_ptr<LimitedJourneyInfo> limitedJourney1(
        LimitedJourneyInfo::create<SingleJourneyInfo>(
            journey.begin(), journey.end(), journey.begin() + 1, journey.begin() + 3));

    std::shared_ptr<LimitedJourneyInfo> limitedJourney2(LimitedJourneyInfo::create<ItineraryInfo>(
        journey.begin(), journey.end(), journey.begin(), journey.end()));

    taxPointMap.addJourney(std::vector<Trip>(1, std::make_pair(1, 2)));
    limitedJourney2->setTaxPointMap(taxPointMap);

    std::vector<type::Index> passedRules = limitedJourney1->findPassedRules();
    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(1), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 2) != passedRules.end());

    passedRules = limitedJourney2->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(1), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 0) != passedRules.end());
  }

  void testUsRoundTripAndItineraryInLimitedJourney()
  {
    std::vector<TaxLimitInfo> journey;
    journey.push_back(TaxLimitInfo::onceForItin(0, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(1, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(2, type::MoneyAmount(10, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(3, type::MoneyAmount(7, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(4, type::MoneyAmount(12, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(5, type::MoneyAmount(3, 1)));

    TaxPointMap taxPointMap;

    std::shared_ptr<LimitedJourneyInfo> limitedJourney1(LimitedJourneyInfo::create<UsRoundTripInfo>(
        journey.begin(), journey.end(), journey.begin() + 1, journey.end()));

    std::shared_ptr<LimitedJourneyInfo> limitedJourney2(LimitedJourneyInfo::create<ItineraryInfo>(
        journey.begin(), journey.end(), journey.begin(), journey.end()));

    taxPointMap.addJourney(std::vector<Trip>(1, std::make_pair(1, 5)));
    limitedJourney2->setTaxPointMap(taxPointMap);

    std::vector<type::Index> passedRules = limitedJourney1->findPassedRules();
    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(2), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 1) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 2) != passedRules.end());

    passedRules = limitedJourney2->findPassedRules();
    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(1), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 0) != passedRules.end());
  }

  void testItineraryAndNoRestrictions()
  {
    std::vector<TaxLimitInfo> journey;
    journey.push_back(TaxLimitInfo::noRestrictions(0, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::onceForItin(1, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::onceForItin(2, type::MoneyAmount(10, 1)));
    journey.push_back(TaxLimitInfo::noRestrictions(3, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::noRestrictions(4, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::onceForItin(5, type::MoneyAmount(19, 1)));

    TaxPointMap taxPointMap;
    std::shared_ptr<LimitedJourneyInfo> limitedJourney(LimitedJourneyInfo::create<ItineraryInfo>(
        journey.begin(), journey.end(), journey.begin(), journey.end()));

    std::vector<type::Index> passedRules = limitedJourney->findPassedRules();
    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(4), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 0) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 3) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 4) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 5) != passedRules.end());
  }

  void testContinousJourneyAndNoRestrictions()
  {
    std::vector<TaxLimitInfo> journey;
    journey.push_back(TaxLimitInfo::noRestrictions(0, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::continousJourney(1, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::continousJourney(2, type::MoneyAmount(10, 1)));
    journey.push_back(TaxLimitInfo::noRestrictions(3, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::noRestrictions(4, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::continousJourney(5, type::MoneyAmount(19, 1)));

    TaxPointMap taxPointMap;
    std::shared_ptr<LimitedJourneyInfo> limitedJourney1(LimitedJourneyInfo::create<ItineraryInfo>(
        journey.begin(), journey.end(), journey.begin(), journey.end()));

    std::shared_ptr<LimitedJourneyInfo> limitedJourney2(
        LimitedJourneyInfo::create<ContinuousJourneyInfo>(
            journey.begin(), journey.end(), journey.begin() + 1, journey.end()));

    std::vector<type::Index> passedRules = limitedJourney1->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(3), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 0) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 3) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 4) != passedRules.end());

    passedRules = limitedJourney2->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(4), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 1) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 2) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 3) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 4) != passedRules.end());
  }

  void testSingleJourneyAndNoRestrictions()
  {
    std::vector<TaxLimitInfo> journey;
    journey.push_back(TaxLimitInfo::noRestrictions(0, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::singleJourney(1, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::singleJourney(2, type::MoneyAmount(10, 1)));
    journey.push_back(TaxLimitInfo::noRestrictions(3, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::noRestrictions(4, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::singleJourney(5, type::MoneyAmount(19, 1)));

    TaxPointMap taxPointMap;
    std::shared_ptr<LimitedJourneyInfo> limitedJourney1(
        LimitedJourneyInfo::create<SingleJourneyInfo>(
            journey.begin(), journey.end(), journey.begin() + 3, journey.end()));

    std::shared_ptr<LimitedJourneyInfo> limitedJourney2(
        LimitedJourneyInfo::create<SingleJourneyInfo>(
            journey.begin(), journey.end(), journey.begin(), journey.begin() + 4));

    std::vector<type::Index> passedRules = limitedJourney1->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(3), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 3) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 4) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 5) != passedRules.end());

    passedRules = limitedJourney2->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(3), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 0) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 2) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 3) != passedRules.end());
  }

  void testUsRoundTripAndNoRestrictions()
  {
    std::vector<TaxLimitInfo> journey;
    journey.push_back(TaxLimitInfo::noRestrictions(0, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(1, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(2, type::MoneyAmount(10, 1)));
    journey.push_back(TaxLimitInfo::noRestrictions(3, type::MoneyAmount(1, 1)));
    journey.push_back(TaxLimitInfo::noRestrictions(4, type::MoneyAmount(5, 1)));
    journey.push_back(TaxLimitInfo::usRoundTrip(5, type::MoneyAmount(19, 1)));

    TaxPointMap taxPointMap;

    std::shared_ptr<LimitedJourneyInfo> limitedJourney1(LimitedJourneyInfo::create<UsRoundTripInfo>(
        journey.begin(), journey.end(), journey.begin() + 3, journey.end()));

    std::shared_ptr<LimitedJourneyInfo> limitedJourney2(LimitedJourneyInfo::create<UsRoundTripInfo>(
        journey.begin(), journey.end(), journey.begin(), journey.begin() + 5));

    std::vector<type::Index> passedRules = limitedJourney1->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(2), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 3) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 4) != passedRules.end());

    passedRules = limitedJourney2->findPassedRules();

    CPPUNIT_ASSERT_EQUAL(std::vector<type::Index>::size_type(4), passedRules.size());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 0) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 1) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 3) != passedRules.end());
    CPPUNIT_ASSERT(std::find(passedRules.begin(), passedRules.end(), 4) != passedRules.end());
  }

  void testCreateLimitedJourney()
  {
    Trip trip1(0, 7);
    Trip trip2(8, 11);

    GeoPath geoPath;
    std::vector<FlightUsage> flightUsages;

    TaxApplicationLimitProcessor processor(geoPath, flightUsages);

    std::vector<TaxLimitInfo> taxLimitInfos;

    taxLimitInfos.push_back(TaxLimitInfo::continousJourney(0, type::MoneyAmount(5, 1)));
    taxLimitInfos.push_back(TaxLimitInfo::continousJourney(2, type::MoneyAmount(5, 1)));
    taxLimitInfos.push_back(TaxLimitInfo::continousJourney(8, type::MoneyAmount(5, 1)));
    taxLimitInfos.push_back(TaxLimitInfo::continousJourney(10, type::MoneyAmount(5, 1)));

    TaxPointMap taxPointMap;
    std::shared_ptr<LimitedJourneyInfo> limitedJourneyInfo1(
        LimitedJourneyInfo::create<ContinuousJourneyInfo>(
            taxLimitInfos.begin(), taxLimitInfos.end(), trip1));

    CPPUNIT_ASSERT_EQUAL(type::Index(0), limitedJourneyInfo1->getFirst()->getId());
    CPPUNIT_ASSERT_EQUAL(type::Index(8), limitedJourneyInfo1->getLast()->getId());

    std::shared_ptr<LimitedJourneyInfo> limitedJourneyInfo2(
        LimitedJourneyInfo::create<ContinuousJourneyInfo>(
            taxLimitInfos.begin(), taxLimitInfos.end(), trip2));

    CPPUNIT_ASSERT_EQUAL(type::Index(8), limitedJourneyInfo2->getFirst()->getId());
  }

  void testTaxPointMap()
  {
    Trip trip1(0, 3);
    Trip trip2(8, 11);

    std::vector<Trip> journey;
    journey.push_back(trip1);
    journey.push_back(trip2);

    TaxPointMap taxPointMap;
    taxPointMap.addJourney(journey);

    CPPUNIT_ASSERT(!taxPointMap.isOnlyInItinerary(type::Index(0)));
    CPPUNIT_ASSERT(!taxPointMap.isOnlyInItinerary(type::Index(3)));
    CPPUNIT_ASSERT(taxPointMap.isOnlyInItinerary(type::Index(4)));
    CPPUNIT_ASSERT(taxPointMap.isOnlyInItinerary(type::Index(7)));
    CPPUNIT_ASSERT(!taxPointMap.isOnlyInItinerary(type::Index(8)));
  }

  void testTaxPointMap_1()
  {
    Trip trip1(1, 2);
    std::vector<Trip> journey;
    journey.push_back(trip1);
    TaxPointMap taxPointMap;
    taxPointMap.addJourney(journey);

    CPPUNIT_ASSERT(taxPointMap.isOnlyInItinerary(type::Index(0)));
    CPPUNIT_ASSERT(!taxPointMap.isOnlyInItinerary(type::Index(1)));
    CPPUNIT_ASSERT(!taxPointMap.isOnlyInItinerary(type::Index(2)));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxApplicationLimitProcessorTest);
}
