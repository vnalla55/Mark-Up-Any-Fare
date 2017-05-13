//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#include <string>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/include/GtestHelperMacros.h"
#include "Xform/DataModelMap.cpp"

namespace tse
{
using namespace ::testing;
class DataModelMapTest : public ::testing::Test
{
public:
  DataModelMapTest()
  {}

  void SetUp()
  {}

  void TearDown()
  {}
};

TEST_F(DataModelMapTest, testPurgeBookingCodeOfNonAlpha)
{
  const std::string p1 ("A"),
                    p2 ("A-"),
                    p3 ("AC"),
                    p4 ("ABC"),
                    p5 ("A1C");


  EXPECT_EQ("A", DataModelMap::purgeBookingCodeOfNonAlpha(p1));
  EXPECT_EQ("A", DataModelMap::purgeBookingCodeOfNonAlpha(p2));
  EXPECT_EQ("AC", DataModelMap::purgeBookingCodeOfNonAlpha(p3));
  EXPECT_EQ("AB", DataModelMap::purgeBookingCodeOfNonAlpha(p4));
  EXPECT_EQ("A", DataModelMap::purgeBookingCodeOfNonAlpha(p5));
}
}