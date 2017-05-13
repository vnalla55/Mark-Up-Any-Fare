//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#include "Fares/AvailabilityChecker.h"

#include "Common/ClassOfService.h"
#include "DataModel/AirSeg.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>

#include <vector>

namespace tse
{
class AvailabilityCheckerTest : public testing::Test
{
public:
  void TearDown() override
  {
    _segments.clear();
    _classes.clear();
  }

protected:
  std::vector<TravelSeg*> _segments;
  std::vector<std::vector<ClassOfService*>*> _classes;
};

TEST_F(AvailabilityCheckerTest, estCheckAvailability_Pass_EmptyTravelSeg)
{
  ASSERT_TRUE(AvailabilityChecker().checkAvailability(1, "A", _classes, _segments));
}

TEST_F(AvailabilityCheckerTest, estCheckAvailability_Fail_EmptyClassOfServiceVec)
{
  AirSeg airSeg;
  _segments.push_back(&airSeg);
  ASSERT_FALSE(AvailabilityChecker().checkAvailability(1, "A", _classes, _segments));
}

TEST_F(AvailabilityCheckerTest, estCheckAvailability_Fail_NoClassOfService)
{
  AirSeg airSeg;
  _segments.push_back(&airSeg);
  std::vector<ClassOfService*> cosVec;
  _classes.push_back(&cosVec);

  ASSERT_FALSE(AvailabilityChecker().checkAvailability(1, "A", _classes, _segments));
}

TEST_F(AvailabilityCheckerTest, estCheckAvailability_Fail_NoMatchAvail)
{
  AirSeg airSeg;
  _segments.push_back(&airSeg);
  std::vector<ClassOfService*> cosVec;
  _classes.push_back(&cosVec);
  ClassOfService cos;
  cosVec.push_back(&cos);

  ASSERT_FALSE(AvailabilityChecker().checkAvailability(1, "A", _classes, _segments));
}

TEST_F(AvailabilityCheckerTest, estCheckAvailability_Pass)
{
  _segments.clear();
  AirSeg airSeg;
  _segments.push_back(&airSeg);
  std::vector<ClassOfService*> cosVec;
  _classes.push_back(&cosVec);
  ClassOfService cos;
  cos.bookingCode() = "A";
  cos.numSeats() = 3;
  cosVec.push_back(&cos);

  ASSERT_TRUE(AvailabilityChecker().checkAvailability(1, "A", _classes, _segments));
}

TEST_F(AvailabilityCheckerTest, estCheckAvailability_Fail_notEnoughSeats)
{
  _segments.clear();
  AirSeg airSeg;
  _segments.push_back(&airSeg);
  std::vector<ClassOfService*> cosVec;
  _classes.push_back(&cosVec);
  ClassOfService cos;
  cos.bookingCode() = "A";
  cos.numSeats() = 3;
  cosVec.push_back(&cos);

  ASSERT_FALSE(AvailabilityChecker().checkAvailability(5, "A", _classes, _segments));
}
}

