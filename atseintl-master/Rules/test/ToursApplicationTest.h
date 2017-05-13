#ifndef __TOURS_APPLICATION_TEST_H__
#define __TOURS_APPLICATION_TEST_H__

#include "DataModel/PaxTypeFare.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <vector>
#include <cppunit/TestFixture.h>

using std::string;

namespace tse
{

class FareUsage;
class ToursApplication;

class ToursApplicationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ToursApplicationTest);
  CPPUNIT_TEST(testValidateEmptyFareUsageVec);
  CPPUNIT_TEST(testValidateSingleFareUsageEmptyTourCode);
  CPPUNIT_TEST(testValidateSingleFareUsageWithTourCode);
  CPPUNIT_TEST(testValidateTwoFareUsagesNoTourCodesSameCarrier);
  CPPUNIT_TEST(testValidateTwoFareUsagesNoTourCodesDiffCarriers);
  CPPUNIT_TEST(testValidateTwoFareUsagesWithSameTourCodeSameCarrier);
  CPPUNIT_TEST(testValidateTwoFareUsagesWithSameTourCodeDiffCarriers);
  CPPUNIT_TEST(testValidateTwoFareUsagesWithDiffTourCodesSameCarrier);
  CPPUNIT_TEST(testValidateTwoFareUsagesWithDiffTourCodesDiffCarriers);
  CPPUNIT_TEST(testValidateTwoFareUsagesWithOneEmptyTourCodeFirstSameCarrier);
  CPPUNIT_TEST(testValidateTwoFareUsagesWithOneEmptyTourCodeFirstDiffCarriers);
  CPPUNIT_TEST(testValidateTwoFareUsagesWithOneEmptyTourCodeLastSameCarrier);
  CPPUNIT_TEST(testValidateTwoFareUsagesWithOneEmptyTourCodeLastDiffCarriers);
  CPPUNIT_TEST(testValidateThreeFareUsagesNoTourCodesSameCarrier);
  CPPUNIT_TEST(testValidateThreeFareUsagesNoTourCodesDiffCarriers);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithFirstTourCodeSameCarrier);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithFirstTourCodeDiffCarriers);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithMiddleTourCodeSameCarrier);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithMiddleTourCodeDiffCarriers);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithLastTourCodeSameCarrier);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithLastTourCodeDiffCarriers);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithNoFirstTourCodeSameCarrier);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithNoFirstTourCodeDiffCarriers);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithNoFirstMixTourCodesSameCarrier);
  CPPUNIT_TEST(testValidateThreeFareUsagesNoWithFirstMixTourCodesDiffCarriers);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithNoMiddleTourCodeSameCarrier);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithNoMiddleTourCodeDiffCarriers);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithNoMiddleMixTourCodesSameCarrier);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithNoMiddleMixTourCodesDiffCarriers);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithNoLastTourCodeSameCarrier);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithNoLastTourCodeDiffCarriers);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithNoLastMixTourCodesSameCarrier);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithNoLastMixTourCodesDiffCarriers);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithSameTourCodeSameCarrier);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithSameTourCodeDiffCarrier);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithDiffTourCodesSameCarrier);
  CPPUNIT_TEST(testValidateThreeFareUsagesWithDiffTourCodesDiffCarriers);
  CPPUNIT_TEST(testValidateCanBeSkippedForSoloCarnival);
  CPPUNIT_TEST(testValidatePricingUnitNotValid);
  CPPUNIT_TEST(testValidatePricingUnitFail);
  CPPUNIT_TEST(testValidatePricingUnitPass);
  CPPUNIT_TEST(testValidateFarePathNotValid);
  CPPUNIT_TEST(testValidateFarePathEmpty);
  CPPUNIT_TEST(testValidateFarePathOnePricingUnitOneFareUsage);
  CPPUNIT_TEST(testValidateFarePathOnePricingUnitTwoFareUsage);
  CPPUNIT_TEST(testValidateFarePathTwoPricingUnitOneFareUsage);
  CPPUNIT_TEST(testValidateFarePathTwoPricingUnitTwoFareUsage);
  CPPUNIT_TEST(testValidatePricingUnitCmdPricingInvalidCombination);
  CPPUNIT_TEST(testValidateFarePathCmdPricingInvalidCombination);
  CPPUNIT_TEST(testGetCarrierIndustryCarrier);
  CPPUNIT_TEST(testGetCarrierNoIndustryCarrier);
  CPPUNIT_TEST(testIsNegotiatedFareWithTourCodeForNonNegotiatedFare);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testValidateEmptyFareUsageVec();
  void testValidateSingleFareUsageEmptyTourCode();
  void testValidateSingleFareUsageWithTourCode();
  void testValidateTwoFareUsagesNoTourCodesSameCarrier();
  void testValidateTwoFareUsagesNoTourCodesDiffCarriers();
  void testValidateTwoFareUsagesWithSameTourCodeSameCarrier();
  void testValidateTwoFareUsagesWithSameTourCodeDiffCarriers();
  void testValidateTwoFareUsagesWithDiffTourCodesSameCarrier();
  void testValidateTwoFareUsagesWithDiffTourCodesDiffCarriers();
  void testValidateTwoFareUsagesWithOneEmptyTourCodeFirstSameCarrier();
  void testValidateTwoFareUsagesWithOneEmptyTourCodeFirstDiffCarriers();
  void testValidateTwoFareUsagesWithOneEmptyTourCodeLastSameCarrier();
  void testValidateTwoFareUsagesWithOneEmptyTourCodeLastDiffCarriers();
  void testValidateThreeFareUsagesNoTourCodesSameCarrier();
  void testValidateThreeFareUsagesNoTourCodesDiffCarriers();
  void testValidateThreeFareUsagesWithFirstTourCodeSameCarrier();
  void testValidateThreeFareUsagesWithFirstTourCodeDiffCarriers();
  void testValidateThreeFareUsagesWithMiddleTourCodeSameCarrier();
  void testValidateThreeFareUsagesWithMiddleTourCodeDiffCarriers();
  void testValidateThreeFareUsagesWithLastTourCodeSameCarrier();
  void testValidateThreeFareUsagesWithLastTourCodeDiffCarriers();
  void testValidateThreeFareUsagesWithNoFirstTourCodeSameCarrier();
  void testValidateThreeFareUsagesWithNoFirstTourCodeDiffCarriers();
  void testValidateThreeFareUsagesWithNoFirstMixTourCodesSameCarrier();
  void testValidateThreeFareUsagesNoWithFirstMixTourCodesDiffCarriers();
  void testValidateThreeFareUsagesWithNoMiddleTourCodeSameCarrier();
  void testValidateThreeFareUsagesWithNoMiddleTourCodeDiffCarriers();
  void testValidateThreeFareUsagesWithNoMiddleMixTourCodesSameCarrier();
  void testValidateThreeFareUsagesWithNoMiddleMixTourCodesDiffCarriers();
  void testValidateThreeFareUsagesWithNoLastTourCodeSameCarrier();
  void testValidateThreeFareUsagesWithNoLastTourCodeDiffCarriers();
  void testValidateThreeFareUsagesWithNoLastMixTourCodesSameCarrier();
  void testValidateThreeFareUsagesWithNoLastMixTourCodesDiffCarriers();
  void testValidateThreeFareUsagesWithSameTourCodeSameCarrier();
  void testValidateThreeFareUsagesWithSameTourCodeDiffCarrier();
  void testValidateThreeFareUsagesWithDiffTourCodesSameCarrier();
  void testValidateThreeFareUsagesWithDiffTourCodesDiffCarriers();
  void testValidateCanBeSkippedForSoloCarnival();
  void testValidatePricingUnitNotValid();
  void testValidatePricingUnitFail();
  void testValidatePricingUnitPass();
  void testValidateFarePathNotValid();
  void testValidateFarePathEmpty();
  void testValidateFarePathOnePricingUnitOneFareUsage();
  void testValidateFarePathOnePricingUnitTwoFareUsage();
  void testValidateFarePathTwoPricingUnitOneFareUsage();
  void testValidateFarePathTwoPricingUnitTwoFareUsage();
  void testValidatePricingUnitCmdPricingInvalidCombination();
  void testValidateFarePathCmdPricingInvalidCombination();
  void testGetCarrierIndustryCarrier();
  void testGetCarrierNoIndustryCarrier();
  void testIsNegotiatedFareWithTourCodeForNonNegotiatedFare();

  void addTourCode(PaxTypeFare& fare, const string& tourCode);
  FareUsage* createFareUsage(const string& tourCode, const CarrierCode& carrier);
  PaxTypeFare* createPaxTypeFare(CarrierCode carrier, CarrierCode governing);

  ToursApplication* _toursApplication;
  TestMemHandle _memHandle;
  PricingTrx* _pricingTrx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ToursApplicationTest);

}
#endif //__TOURS_APPLICATION_TEST_H__
