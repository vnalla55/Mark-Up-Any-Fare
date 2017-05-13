//----------------------------------------------------------------------------
//
// Copyright Sabre 2014
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"
#include "Routing/MileageUtil.h"

namespace tse
{
class MileageUtilTest : public ::testing::Test
{
};

TEST_F(MileageUtilTest, test_getEMS_zeroMPM)
{
  ASSERT_EQ(uint16_t(0), MileageUtil::getEMS(0, 0));
  ASSERT_EQ(uint16_t(0), MileageUtil::getEMS(123, 0));
  ASSERT_EQ(uint16_t(0), MileageUtil::getEMS(65535, 0));
}

TEST_F(MileageUtilTest, test_getEMS_zeroTPM)
{
  ASSERT_EQ(uint16_t(0), MileageUtil::getEMS(0, 1234));
  ASSERT_EQ(uint16_t(0), MileageUtil::getEMS(0, 65535));
}

TEST_F(MileageUtilTest, test_getEMS_TPM_lte_MPM)
{
  ASSERT_EQ(uint16_t(0), MileageUtil::getEMS(1, 2));
  ASSERT_EQ(uint16_t(0), MileageUtil::getEMS(123, 1234));
  ASSERT_EQ(uint16_t(0), MileageUtil::getEMS(1234, 1234));
  ASSERT_EQ(uint16_t(0), MileageUtil::getEMS(65534, 65535));
  ASSERT_EQ(uint16_t(0), MileageUtil::getEMS(1000, 65535));
}

TEST_F(MileageUtilTest, test_getEMS_5)
{
  ASSERT_EQ(uint16_t(5), MileageUtil::getEMS(1050, 1000));
  ASSERT_EQ(uint16_t(5), MileageUtil::getEMS(6744, 6743));
  ASSERT_EQ(uint16_t(5), MileageUtil::getEMS(8764, 8400));
  ASSERT_EQ(uint16_t(5), MileageUtil::getEMS(2462, 2345));
  ASSERT_EQ(uint16_t(5), MileageUtil::getEMS(65535, 62415));
}

TEST_F(MileageUtilTest, test_getEMS_10)
{
  ASSERT_EQ(uint16_t(10), MileageUtil::getEMS(1100, 1000));
  ASSERT_EQ(uint16_t(10), MileageUtil::getEMS(3676, 3500));
  ASSERT_EQ(uint16_t(10), MileageUtil::getEMS(9123, 8400));
  ASSERT_EQ(uint16_t(10), MileageUtil::getEMS(2463, 2345));
  ASSERT_EQ(uint16_t(10), MileageUtil::getEMS(2579, 2345));
  ASSERT_EQ(uint16_t(10), MileageUtil::getEMS(65535, 59578));
}

TEST_F(MileageUtilTest, test_getEMS_15)
{
  ASSERT_EQ(uint16_t(15), MileageUtil::getEMS(1150, 1000));
  ASSERT_EQ(uint16_t(15), MileageUtil::getEMS(6205, 5640));
  ASSERT_EQ(uint16_t(15), MileageUtil::getEMS(9456, 8400));
  ASSERT_EQ(uint16_t(15), MileageUtil::getEMS(2580, 2345));
  ASSERT_EQ(uint16_t(15), MileageUtil::getEMS(2696, 2345));
  ASSERT_EQ(uint16_t(15), MileageUtil::getEMS(65535, 56987));
}

TEST_F(MileageUtilTest, test_getEMS_20)
{
  ASSERT_EQ(uint16_t(20), MileageUtil::getEMS(1200, 1000));
  ASSERT_EQ(uint16_t(20), MileageUtil::getEMS(9017, 7840));
  ASSERT_EQ(uint16_t(20), MileageUtil::getEMS(10000, 8400));
  ASSERT_EQ(uint16_t(20), MileageUtil::getEMS(2697, 2345));
  ASSERT_EQ(uint16_t(20), MileageUtil::getEMS(2814, 2346));
  ASSERT_EQ(uint16_t(20), MileageUtil::getEMS(65535, 54613));
}

TEST_F(MileageUtilTest, test_getEMS_25)
{
  ASSERT_EQ(uint16_t(25), MileageUtil::getEMS(1250, 1000));
  ASSERT_EQ(uint16_t(25), MileageUtil::getEMS(1609, 1340));
  ASSERT_EQ(uint16_t(25), MileageUtil::getEMS(10100, 8400));
  ASSERT_EQ(uint16_t(25), MileageUtil::getEMS(2815, 2344));
  ASSERT_EQ(uint16_t(25), MileageUtil::getEMS(2931, 2345));
  ASSERT_EQ(uint16_t(25), MileageUtil::getEMS(65535, 52428));
}

TEST_F(MileageUtilTest, test_getEMS_30)
{
  ASSERT_EQ(uint16_t(30), MileageUtil::getEMS(1251, 1000));
  ASSERT_EQ(uint16_t(30), MileageUtil::getEMS(4321, 1234));
  ASSERT_EQ(uint16_t(30), MileageUtil::getEMS(2932, 2345));
  ASSERT_EQ(uint16_t(30), MileageUtil::getEMS(65535, 52427));
  ASSERT_EQ(uint16_t(30), MileageUtil::getEMS(65535, 1));
}
}
