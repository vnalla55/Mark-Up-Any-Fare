//-------------------------------------------------------------------
//
//  Authors:     Andrzej Fediuk
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

#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"

#include "DataModel/IbfAvailabilityTools.h"

#include <memory>

using namespace ::testing;

namespace tse
{

class IbfAvailabilityToolsTest: public Test
{
public:

  void SetUp()
  {
    _seg1.reset(new AirSeg());
    _seg1->legId() = 1;
    _seg2.reset(new AirSeg());
    _seg2->legId() = 1;
    _seg3.reset(new AirSeg());
    _seg3->legId() = 2;
    _seg4.reset(new AirSeg());
    _seg4->legId() = 2;

    _fm1.reset(new FareMarket());
    _fm1->forceStatusForBrand(DUMMY_BRAND, Direction::BOTHWAYS, STATUS_A);
    _fm1->travelSeg().push_back(_seg1.get());
    _fm2.reset(new FareMarket());
    _fm2->forceStatusForBrand(DUMMY_BRAND, Direction::BOTHWAYS, STATUS_O);
    _fm2->travelSeg().push_back(_seg2.get());
    _fm2->travelSeg().push_back(_seg3.get());
    _fm3.reset(new FareMarket());
    _fm3->forceStatusForBrand(DUMMY_BRAND, Direction::BOTHWAYS, STATUS_F);
    _fm3->travelSeg().push_back(_seg3.get());
    _fm3->travelSeg().push_back(_seg4.get());

    _itin.reset(new Itin());
    _itin->fareMarket().push_back(_fm1.get());
    _itin->fareMarket().push_back(_fm2.get());
    _itin->fareMarket().push_back(_fm3.get());
  }

  std::shared_ptr<FareMarket> _fm1;
  std::shared_ptr<FareMarket> _fm2;
  std::shared_ptr<FareMarket> _fm3;

  std::shared_ptr<AirSeg> _seg1;
  std::shared_ptr<AirSeg> _seg2;
  std::shared_ptr<AirSeg> _seg3;
  std::shared_ptr<AirSeg> _seg4;

  std::shared_ptr<Itin> _itin;

  static const IbfErrorMessage STATUS_A;
  static const IbfErrorMessage STATUS_O;
  static const IbfErrorMessage STATUS_F;

  static const BrandCode DUMMY_BRAND;
};

const IbfErrorMessage IbfAvailabilityToolsTest::STATUS_A = IbfErrorMessage::IBF_EM_NOT_AVAILABLE;
const IbfErrorMessage IbfAvailabilityToolsTest::STATUS_O = IbfErrorMessage::IBF_EM_NOT_OFFERED;
const IbfErrorMessage IbfAvailabilityToolsTest::STATUS_F = IbfErrorMessage::IBF_EM_NO_FARE_FOUND;

const BrandCode IbfAvailabilityToolsTest::DUMMY_BRAND = "ORANGE";

TEST_F(IbfAvailabilityToolsTest, testCalculatesSegmentsStatusesCorrectly)
{
  // FM1: status A, segment 1, leg 1
  // FM2: status O, segment 2+3, leg 1+2
  // FM3: status F, segment 3+4, leg 2

  IbfErrorMessage expetedStatusForSeg1 = STATUS_A;
  IbfErrorMessage expetedStatusForSeg2 = STATUS_O;
  IbfErrorMessage expetedStatusForSeg3 = STATUS_O;
  IbfErrorMessage expetedStatusForSeg3Update = STATUS_F;
  IbfErrorMessage expetedStatusForSeg4 = STATUS_F;

  std::map<const TravelSeg*, IbfErrorMessageChoiceAcc> result;

  IbfAvailabilityTools::calculateStatusesForFareMarketTravelSegments(_fm1.get(),
    DUMMY_BRAND, result);

  ASSERT_TRUE(result.find(_seg1.get()) != result.end());
  ASSERT_EQ(expetedStatusForSeg1, result.at(_seg1.get()).getStatus());

  IbfAvailabilityTools::calculateStatusesForFareMarketTravelSegments(_fm2.get(),
    DUMMY_BRAND, result);

  ASSERT_TRUE(result.find(_seg1.get()) != result.end());
  ASSERT_TRUE(result.find(_seg2.get()) != result.end());
  ASSERT_TRUE(result.find(_seg3.get()) != result.end());

  ASSERT_EQ(expetedStatusForSeg1, result.at(_seg1.get()).getStatus());
  ASSERT_EQ(expetedStatusForSeg2, result.at(_seg2.get()).getStatus());
  ASSERT_EQ(expetedStatusForSeg3, result.at(_seg3.get()).getStatus());

  IbfAvailabilityTools::calculateStatusesForFareMarketTravelSegments(_fm3.get(),
    DUMMY_BRAND, result);

  ASSERT_TRUE(result.find(_seg1.get()) != result.end());
  ASSERT_TRUE(result.find(_seg2.get()) != result.end());
  ASSERT_TRUE(result.find(_seg3.get()) != result.end());
  ASSERT_TRUE(result.find(_seg4.get()) != result.end());

  ASSERT_EQ(expetedStatusForSeg1, result.at(_seg1.get()).getStatus());
  ASSERT_EQ(expetedStatusForSeg2, result.at(_seg2.get()).getStatus());
  ASSERT_EQ(expetedStatusForSeg3Update, result.at(_seg3.get()).getStatus());
  ASSERT_EQ(expetedStatusForSeg4, result.at(_seg4.get()).getStatus());
}

TEST_F(IbfAvailabilityToolsTest, testRaisesCorrectlyOnImproperTravelSegment)
{
  _fm1->travelSeg().push_back(static_cast<AirSeg*>(0));
  std::map<const TravelSeg*, IbfErrorMessageChoiceAcc> result;

  ASSERT_THROW(IbfAvailabilityTools::calculateStatusesForFareMarketTravelSegments(
    _fm1.get(), DUMMY_BRAND, result), ErrorResponseException);
}

TEST_F(IbfAvailabilityToolsTest, testCalculatesItinAndLegStatusCorrectly)
{
  // FM1: status A, segment 1, leg 1
  // FM2: status O, segment 2+3, leg 1+2
  // FM3: status F, segment 3+4, leg 2

  IbfErrorMessage expected = STATUS_O;

  std::map<LegId, IbfErrorMessage> resultLegStatuses;
  std::map<LegId, IbfErrorMessage> expectedLegStatuses;
  expectedLegStatuses[1] = STATUS_O;
  expectedLegStatuses[2] = STATUS_F;

  ASSERT_EQ(expected, IbfAvailabilityTools::calculateAllStatusesForItinBrand(
    _itin.get(), DUMMY_BRAND, resultLegStatuses));
  ASSERT_EQ(expectedLegStatuses, resultLegStatuses);
}

TEST_F(IbfAvailabilityToolsTest, testRaisesCorrectlyOnImproperItin)
{
  std::map<LegId, IbfErrorMessage> resultLegStatuses;

  ASSERT_THROW(IbfAvailabilityTools::calculateAllStatusesForItinBrand(
    0, DUMMY_BRAND, resultLegStatuses), ErrorResponseException);
}

TEST_F(IbfAvailabilityToolsTest, testRaisesCorrectlyOnImproperFareMarket)
{
  std::map<LegId, IbfErrorMessage> resultLegStatuses;
  _itin->fareMarket().push_back(static_cast<FareMarket*>(0));

  ASSERT_THROW(IbfAvailabilityTools::calculateAllStatusesForItinBrand(
    _itin.get(), DUMMY_BRAND, resultLegStatuses), ErrorResponseException);
}

} //namespace tse
