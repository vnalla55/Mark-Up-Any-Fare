//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "Common/ErrorResponseException.h"

#include "Pricing/Shopping/IBF/IbfRequirementsEstimator.h"

using namespace std;

namespace tse
{

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class IbfRequirementsEstimatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(IbfRequirementsEstimatorTest);
  CPPUNIT_TEST(getValuesNoEstimation);
  CPPUNIT_TEST(estimateNoInput);
  CPPUNIT_TEST(estimatePartialInput);
  CPPUNIT_TEST(getValuesFullInputNoEstimation);
  CPPUNIT_TEST(badInputTest1);
  CPPUNIT_TEST(badInputTest2);
  CPPUNIT_TEST(dataLogicallyInvalid1);
  CPPUNIT_TEST(dataLogicallyInvalid2);
  CPPUNIT_TEST(correctUsageTwoLegsA);
  CPPUNIT_TEST(correctUsageTwoLegsB);
  CPPUNIT_TEST(correctUsageOneLeg);
  CPPUNIT_TEST(correctUsageThreeLegs);
  CPPUNIT_TEST_SUITE_END();

public:
  void getValuesNoEstimation()
  {
    IbfRequirementsEstimator e;
    CPPUNIT_ASSERT_THROW(e.getEstimatedRemainingRco(), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(e.getEstimatedOptionsCountToCoverAllSops(), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(e.getEstimatedRcoMax(), ErrorResponseException);
  }

  void estimateNoInput()
  {
    IbfRequirementsEstimator e;
    CPPUNIT_ASSERT_THROW(e.estimateRemainingRcoCount(), ErrorResponseException);
  }

  void estimatePartialInput()
  {
    IbfRequirementsEstimator e;
    e.setLegsCount(2);
    CPPUNIT_ASSERT_THROW(e.estimateRemainingRcoCount(), ErrorResponseException);
    e.setRequiredSolutionsCount(400);
    CPPUNIT_ASSERT_THROW(e.getEstimatedRcoMax(), ErrorResponseException);
    e.setDirectFosCount(120);
    CPPUNIT_ASSERT_THROW(e.estimateRemainingRcoCount(), ErrorResponseException);
    e.setRcoDirectFosCount(75);

    e.setUnusedSopsCount(0, 110);
    CPPUNIT_ASSERT_THROW(e.getEstimatedOptionsCountToCoverAllSops(), ErrorResponseException);
    e.setUnusedSopsCount(1, 90);

    e.setRcoSopsCount(0, 15);
    CPPUNIT_ASSERT_THROW(e.estimateRemainingRcoCount(), ErrorResponseException);
    e.setRcoSopsCount(1, 16);

    // Now all input is done - no errors should appear
    e.estimateRemainingRcoCount();
  }

  void getValuesFullInputNoEstimation()
  {
    IbfRequirementsEstimator e;
    e.setLegsCount(2);
    e.setRequiredSolutionsCount(400);
    e.setDirectFosCount(120);
    e.setRcoDirectFosCount(75);
    e.setUnusedSopsCount(0, 110);
    e.setUnusedSopsCount(1, 90);
    e.setRcoSopsCount(0, 15);
    e.setRcoSopsCount(1, 16);
    CPPUNIT_ASSERT_THROW(e.getEstimatedRemainingRco(), ErrorResponseException);
  }

  void badInputTest1()
  {
    IbfRequirementsEstimator e;
    const unsigned int UNK = static_cast<unsigned int>(-1);
    CPPUNIT_ASSERT_THROW(e.setRequiredSolutionsCount(0), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(e.setDirectFosCount(UNK), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(e.setRcoDirectFosCount(UNK), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(e.setLegsCount(0), ErrorResponseException);
  }

  void badInputTest2()
  {
    IbfRequirementsEstimator e;
    const unsigned int UNK = static_cast<unsigned int>(-1);
    e.setLegsCount(2);

    // Bad legId
    CPPUNIT_ASSERT_THROW(e.setUnusedSopsCount(2, 9), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(e.setRcoSopsCount(3, 100), ErrorResponseException);

    // UNK values
    CPPUNIT_ASSERT_THROW(e.setUnusedSopsCount(0, UNK), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(e.setRcoSopsCount(1, UNK), ErrorResponseException);
  }

  // DF > Q
  void dataLogicallyInvalid1()
  {
    IbfRequirementsEstimator e;

    e.setLegsCount(2);
    e.setRequiredSolutionsCount(400);
    e.setDirectFosCount(405); // too much!
    e.setRcoDirectFosCount(75);
    e.setUnusedSopsCount(0, 110);
    e.setUnusedSopsCount(1, 90);
    e.setRcoSopsCount(0, 15);
    e.setRcoSopsCount(1, 16);

    CPPUNIT_ASSERT_THROW(e.estimateRemainingRcoCount(), ErrorResponseException);
  }

  // DFrco <= DF
  void dataLogicallyInvalid2()
  {
    IbfRequirementsEstimator e;

    e.setLegsCount(2);
    e.setRequiredSolutionsCount(400);
    e.setDirectFosCount(120);
    e.setRcoDirectFosCount(121); // // too much!
    e.setUnusedSopsCount(0, 110);
    e.setUnusedSopsCount(1, 90);
    e.setRcoSopsCount(0, 15);
    e.setRcoSopsCount(1, 16);
  }

  void correctUsageTwoLegsA()
  {
    IbfRequirementsEstimator e;
    e.setLegsCount(2);
    e.setRequiredSolutionsCount(400);
    e.setDirectFosCount(120);
    e.setRcoDirectFosCount(75);
    e.setUnusedSopsCount(0, 110);
    e.setUnusedSopsCount(1, 90);
    e.setRcoSopsCount(0, 15);
    e.setRcoSopsCount(1, 16);
    e.estimateRemainingRcoCount();
    // RcoMax - DFrco = 240 - 75 = 165
    // Q - DF - AS = 400 - 120 - 110 = 170
    // RRco = 165
    CPPUNIT_ASSERT_EQUAL(240u, e.getEstimatedRcoMax());
    CPPUNIT_ASSERT_EQUAL(110u, e.getEstimatedOptionsCountToCoverAllSops());
    CPPUNIT_ASSERT_EQUAL(165u, e.getEstimatedRemainingRco());
  }

  void correctUsageTwoLegsB()
  {
    IbfRequirementsEstimator e;
    e.setLegsCount(2);
    e.setRequiredSolutionsCount(400);
    e.setDirectFosCount(120);
    e.setRcoDirectFosCount(75);
    e.setUnusedSopsCount(0, 110);
    e.setUnusedSopsCount(1, 90);
    e.setRcoSopsCount(0, 15);
    e.setRcoSopsCount(1, 20);
    e.estimateRemainingRcoCount();
    // RcoMax - DFrco = 300 - 75 = 225
    // Q - DF - AS = 400 - 120 - 110 = 170
    // RRco = 170
    CPPUNIT_ASSERT_EQUAL(300u, e.getEstimatedRcoMax());
    CPPUNIT_ASSERT_EQUAL(110u, e.getEstimatedOptionsCountToCoverAllSops());
    CPPUNIT_ASSERT_EQUAL(170u, e.getEstimatedRemainingRco());
  }

  void correctUsageOneLeg()
  {
    IbfRequirementsEstimator e;
    e.setLegsCount(1);
    e.setRequiredSolutionsCount(300);
    e.setDirectFosCount(150);
    e.setRcoDirectFosCount(60);
    e.setUnusedSopsCount(0, 170);
    e.setRcoSopsCount(0, 120);
    e.estimateRemainingRcoCount();
    // RcoMax - DFrco = 120 - 60 = 60
    // Q - DF - AS = 300 - 150 - 170 = 0 (no capacity!)
    // RRco = 0
    CPPUNIT_ASSERT_EQUAL(120u, e.getEstimatedRcoMax());
    CPPUNIT_ASSERT_EQUAL(170u, e.getEstimatedOptionsCountToCoverAllSops());
    CPPUNIT_ASSERT_EQUAL(0u, e.getEstimatedRemainingRco());
  }

  void correctUsageThreeLegs()
  {
    IbfRequirementsEstimator e;
    e.setLegsCount(3);
    e.setRequiredSolutionsCount(500);
    e.setDirectFosCount(230);
    e.setRcoDirectFosCount(160);
    e.setUnusedSopsCount(0, 10);
    e.setUnusedSopsCount(1, 220);
    e.setUnusedSopsCount(2, 165);
    e.setRcoSopsCount(0, 10);
    e.setRcoSopsCount(1, 5);
    e.setRcoSopsCount(2, 15);
    e.estimateRemainingRcoCount();
    // RcoMax - DFrco = 750 - 160 = 590
    // Q - DF - AS = 500 - 230 - 220 = 50
    // RRco = 50
    CPPUNIT_ASSERT_EQUAL(750u, e.getEstimatedRcoMax());
    CPPUNIT_ASSERT_EQUAL(220u, e.getEstimatedOptionsCountToCoverAllSops());
    CPPUNIT_ASSERT_EQUAL(50u, e.getEstimatedRemainingRco());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(IbfRequirementsEstimatorTest);

} // namespace tse
