//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2015
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
#include "test/include/GtestHelperMacros.h"

#include <set>
#include <vector>

#include "Common/ErrorResponseException.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"

using namespace ::testing;
using namespace boost;


namespace tse
{

namespace utils
{

class ShoppingUtilsTest: public Test
{
public:

  void SetUp()
  {
    createDummyIntegerSet();
    createDummyDoubleVector();
    ZEROES.push_back(0.0);
    ZEROES.push_back(0.0);
  }

  void TearDown(){}

  void createDummyIntegerSet()
  {
    DUMMY_INTEGER_SET.insert(18);
    DUMMY_INTEGER_SET.insert(27);
    DUMMY_INTEGER_SET.insert(6);
    DUMMY_INTEGER_SET.insert(100);
    DUMMY_INTEGER_SET.insert(1);
  }

  void createDummyDoubleVector()
  {
    DUMMY_DOUBLE_VECTOR.push_back(1.23);
    DUMMY_DOUBLE_VECTOR.push_back(456.7);
    DUMMY_DOUBLE_VECTOR.push_back(8.9101112);
    DUMMY_DOUBLE_VECTOR.push_back(1.31415);
    DUMMY_DOUBLE_VECTOR.push_back(1617.1819);
    DUMMY_DOUBLE_VECTOR.push_back(20.2122);
    DUMMY_DOUBLE_VECTOR.push_back(2.32425);
  }

  std::set<unsigned int> DUMMY_INTEGER_SET;
  std::vector<double> DUMMY_DOUBLE_VECTOR;
  std::vector<double> ZEROES;
  std::vector<double> EMPTY;
};

TEST_F(ShoppingUtilsTest, testEqual)
{
  ASSERT_TRUE(utils::equal(0.5, 0.5));
  ASSERT_TRUE(utils::equal(4*0.5, 100.0/50));
  ASSERT_FALSE(utils::equal(0.5, 0.5001));
}


TEST_F(ShoppingUtilsTest, testMean)
{
  ASSERT_TRUE(utils::equal(30.4, utils::mean(DUMMY_INTEGER_SET)));
  ASSERT_TRUE(utils::equal(301.1246587, utils::mean(DUMMY_DOUBLE_VECTOR)));
  ASSERT_TRUE(utils::equal(0, utils::mean(ZEROES)));
  ASSERT_THROW(utils::mean(EMPTY), ErrorResponseException);
}

TEST_F(ShoppingUtilsTest, testVariance)
{
  ASSERT_TRUE(utils::equal(1293.84, utils::variance(DUMMY_INTEGER_SET)));
  ASSERT_TRUE(utils::equal(312802.3332, utils::variance(DUMMY_DOUBLE_VECTOR)));
  ASSERT_TRUE(utils::equal(0, utils::variance(ZEROES)));
  ASSERT_THROW(utils::variance(EMPTY), ErrorResponseException);
}

TEST_F(ShoppingUtilsTest, testStdDeviation)
{
  ASSERT_TRUE(utils::equal(35.96998749,
                           utils::standard_deviation(DUMMY_INTEGER_SET)));
  ASSERT_TRUE(utils::equal(559.287344,
                           utils::standard_deviation(DUMMY_DOUBLE_VECTOR)));
  ASSERT_TRUE(utils::equal(0, utils::standard_deviation(ZEROES)));
  ASSERT_THROW(utils::standard_deviation(EMPTY), ErrorResponseException);
}

TEST_F(ShoppingUtilsTest, testCov)
{
  ASSERT_TRUE(utils::equal(1.183223273,
                           utils::coefficient_of_variation(DUMMY_INTEGER_SET)));
  ASSERT_TRUE(utils::equal(
      1.857328278, utils::coefficient_of_variation(DUMMY_DOUBLE_VECTOR)));
  ASSERT_THROW(utils::coefficient_of_variation(EMPTY), ErrorResponseException);
  ASSERT_THROW(utils::coefficient_of_variation(ZEROES), ErrorResponseException);
}


} // namespace utils

} // namespace tse

