//-------------------------------------------------------------------
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
#include <string>
#include <gtest/gtest.h>
#include "test/include/GtestHelperMacros.h"

#include "Common/TseConsts.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FareCalcHelper.h"

#include <memory>

namespace tse
{
class SpanishFamilyDiscountDesignatorTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _agentLocation.nation() = SPAIN;

    _trvlSeg.origin() = &_agentLocation;
    _trvlSeg.destination() = &_agentLocation;
    _travelSegs.push_back(&_trvlSeg);

    _designator = "F1";
    _dfn = SLFUtil::DiscountLevel::LEVEL_1;

    _appender.reset(new SpanishFamilyDiscountDesignator(_dfn,
                                                        _agentLocation,
                                                        _travelSegs,
                                                        FareCalcConsts::MAX_FARE_BASIS_SIZE));
  }

  void TearDown() {}

  Loc _agentLocation;
  AirSeg _trvlSeg;
  std::vector<TravelSeg*> _travelSegs;
  std::string _designator;
  SLFUtil::DiscountLevel _dfn;
  std::shared_ptr<SpanishFamilyDiscountDesignator> _appender;
};

TEST_F(SpanishFamilyDiscountDesignatorTest, wholeTravelNotInSpainTest)
{
  Loc destination;
  destination.nation() = JAPAN;
  AirSeg trvlSeg;
  trvlSeg.origin() = &_agentLocation;
  trvlSeg.destination() = &destination;
  _travelSegs.push_back(&trvlSeg);
  std::string fareBasis = "YDRT";
  const std::string expectedFareBasis = fareBasis;
  (*_appender)(fareBasis);
  ASSERT_EQ(expectedFareBasis, fareBasis);
}

TEST_F(SpanishFamilyDiscountDesignatorTest, agentLocationNotInSpainTest)
{
  _agentLocation.nation() = "PL";
  std::string fareBasis = "YDRT";
  const std::string expectedFareBasis = fareBasis;
  (*_appender)(fareBasis);
  ASSERT_EQ(expectedFareBasis, fareBasis);
}

TEST_F(SpanishFamilyDiscountDesignatorTest, basicTest)
{
  std::string fareBasis = "YDRT";
  const std::string expectedFareBasis = fareBasis + "/" + _designator;
  (*_appender)(fareBasis);
  ASSERT_EQ(expectedFareBasis, fareBasis);
}

TEST_F(SpanishFamilyDiscountDesignatorTest, fareBasis8CharsTest)
{
  std::string fareBasis = "YD8CHARS";
  const std::string expectedFareBasis = fareBasis + "/" + _designator;
  (*_appender)(fareBasis);
  ASSERT_EQ(expectedFareBasis, fareBasis);
}

TEST_F(SpanishFamilyDiscountDesignatorTest, fareBasisWithSlashTest)
{
  std::string fareBasis = "YDRT/RAD";
  const std::string expectedFareBasis = fareBasis + _designator;
  (*_appender)(fareBasis);
  ASSERT_EQ(expectedFareBasis, fareBasis);
}

TEST_F(SpanishFamilyDiscountDesignatorTest, fareBasisWithSlash13CharsTest)
{
  std::string fareBasis = "YDRTT/13CHARS";
  const std::string expectedFareBasis = fareBasis + _designator;
  (*_appender)(fareBasis);
  ASSERT_EQ(expectedFareBasis, fareBasis);
}

TEST_F(SpanishFamilyDiscountDesignatorTest, fareBasisWithSlash14CharsTest)
{
  std::string fareBasis = "YDRTTT/14CHARS";
  const std::string expectedFareBasis = fareBasis + 'F';
  (*_appender)(fareBasis);
  ASSERT_EQ(expectedFareBasis, fareBasis);
}

TEST_F(SpanishFamilyDiscountDesignatorTest, fareBasisWithSlashMaxFareBasisSizeTest)
{
  std::string fareBasis = "YDRTTTT/15CHARS";
  const std::string expectedFareBasis = fareBasis;
  (*_appender)(fareBasis);
  ASSERT_EQ(expectedFareBasis, fareBasis);
}

}  // namespace tse
