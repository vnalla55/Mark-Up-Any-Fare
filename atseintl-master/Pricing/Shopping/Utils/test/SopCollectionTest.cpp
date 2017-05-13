//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2014
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include "test/include/GtestHelperMacros.h"
#include "Pricing/Shopping/Utils/SopCollection.h"

using namespace ::testing;
using namespace boost;

namespace tse
{

namespace utils
{

class SopCollectionTest: public Test
{
public:
  void SetUp(){}
  void TearDown(){}

  void prepare(unsigned int legCount)
  {
    _sopCollection.reset(new SopCollection(legCount));
  }

  void addSopPair(unsigned int s1, unsigned int s2)
  {
    ASSERT_TRUE(_sopCollection.use_count() > 0); // not empty
    _sopCollection->addSop(s1, s2);
    ASSERT_EQ(true, _sopCollection->containsSop(s1, s2));
  }

  static const unsigned int DUMMY_LEG_ID_1 = 0;
  static const unsigned int DUMMY_LEG_ID_2 = 1;
  static const unsigned int DUMMY_SOP_ID_1 = 91;
  static const unsigned int DUMMY_SOP_ID_2 = 0;

  std::shared_ptr<SopCollection> _sopCollection;
};

TEST_F(SopCollectionTest, testContainsAddedSop)
{
  prepare(1);
  addSopPair(DUMMY_LEG_ID_1, DUMMY_SOP_ID_1);
  ASSERT_EQ(false, _sopCollection->containsSop(DUMMY_LEG_ID_2, DUMMY_SOP_ID_2));
}

TEST_F(SopCollectionTest, testContainsTwoAddedSops)
{
  prepare(2);
  addSopPair(DUMMY_LEG_ID_1, DUMMY_SOP_ID_1);
  addSopPair(DUMMY_LEG_ID_2, DUMMY_SOP_ID_2);
}

TEST_F(SopCollectionTest, testCorrectlyCalculatesCardinalityIfEmpty)
{
  prepare(2);
  ASSERT_EQ(0, _sopCollection->getCartesianCardinality());
}

TEST_F(SopCollectionTest, testCorrectlyCalculatesCardinalityOneSop)
{
  prepare(1);
  addSopPair(DUMMY_LEG_ID_1, DUMMY_SOP_ID_1);
  ASSERT_EQ(1, _sopCollection->getCartesianCardinality());
}

TEST_F(SopCollectionTest, testCorrectlyCalculatesCardinalityOneSopPerLeg)
{
  prepare(2);
  addSopPair(DUMMY_LEG_ID_1, DUMMY_SOP_ID_1);
  addSopPair(DUMMY_LEG_ID_2, DUMMY_SOP_ID_2);
  ASSERT_EQ(1, _sopCollection->getCartesianCardinality());
}

TEST_F(SopCollectionTest, testCorrectlyCalculatesCardinalityMultipleSopsTwoLegs)
{
  prepare(2);
  addSopPair(0, 5);
  addSopPair(0, 10);
  addSopPair(1, 15);
  addSopPair(0, 23);
  addSopPair(1, 10);
  addSopPair(1, 65);
  addSopPair(0, 0);

  ASSERT_EQ(12, _sopCollection->getCartesianCardinality());
}

TEST_F(SopCollectionTest, testCorrectlyCalculatesCardinalityMultipleSopsThreeLegs)
{
  prepare(3);
  addSopPair(0, 5);
  addSopPair(0, 10);
  addSopPair(2, 7);
  addSopPair(1, 15);
  addSopPair(0, 23);
  addSopPair(1, 10);
  addSopPair(2, 12);
  addSopPair(1, 65);
  addSopPair(0, 0);

  ASSERT_EQ(24, _sopCollection->getCartesianCardinality());
}

TEST_F(SopCollectionTest, testZeroCardinalityWhenNoSopsOnFirstLeg)
{
  prepare(3);
  addSopPair(2, 7);
  addSopPair(1, 15);
  addSopPair(1, 10);
  addSopPair(2, 12);
  addSopPair(1, 65);

  ASSERT_EQ(0, _sopCollection->getCartesianCardinality());
}

TEST_F(SopCollectionTest, testZeroCardinalityWhenNoSopsOnMiddleLeg)
{
  prepare(3);
  addSopPair(0, 5);
  addSopPair(0, 10);
  addSopPair(2, 7);
  addSopPair(0, 23);
  addSopPair(2, 12);
  addSopPair(0, 0);

  ASSERT_EQ(0, _sopCollection->getCartesianCardinality());
}

TEST_F(SopCollectionTest, testZeroCardinalityWhenNoSopsOnLastLeg)
{
  prepare(3);
  addSopPair(0, 5);
  addSopPair(0, 10);
  addSopPair(1, 15);
  addSopPair(0, 23);
  addSopPair(1, 10);
  addSopPair(1, 65);
  addSopPair(0, 0);

  ASSERT_EQ(0, _sopCollection->getCartesianCardinality());
}

} // namespace utils

} // namespace tse
