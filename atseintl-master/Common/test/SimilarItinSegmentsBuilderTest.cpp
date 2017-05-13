//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------
#include "Common/SimilarItinSegmentsBuilder.h"

#include "DBAccess/Loc.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"

#include <boost/assign/std/vector.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/include/TestDataBuilders.h"
#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
namespace similaritin
{
using namespace boost::assign;

class SegmentsBuilderTest : public testing::Test
{
public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle(new PricingTrx);
    _motherItin = _memHandle.create<Itin>();
    _similarItin = _memHandle.create<Itin>();
  }

  void TearDown()
  {
    _memHandle.clear();
  }

protected:
  TravelSeg* createTravelSeg(const LocCode& from, const LocCode& to, uint16_t legId = 0)
  {
    return AirSegBuilder(_memHandle).withLocs(from, to).withLegId(legId).build();
  }

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  Itin* _motherItin;
  Itin* _similarItin;
};

TEST_F(SegmentsBuilderTest, testConstructByOnDSameLegs)
{
  SegmentsBuilder::TSVec& motherItinSegments(_motherItin->travelSeg());
  SegmentsBuilder::TSVec& similarItinSegments(_similarItin->travelSeg());

  SegmentsBuilder::TSVec originalSegments, adjustedSegments, expectedSegments;

  motherItinSegments += createTravelSeg("MIA", "LAX");
  TravelSeg* segment = createTravelSeg("LAX", "DFW");
  motherItinSegments += segment;
  originalSegments += segment;
  motherItinSegments += createTravelSeg("DFW", "LHR");
  motherItinSegments += createTravelSeg("LHR", "KRK");

  segment = createTravelSeg("LAX", "WAS");
  similarItinSegments += segment;
  expectedSegments += segment;
  segment = createTravelSeg("WAS", "DFW");
  similarItinSegments += segment;
  expectedSegments += segment;
  similarItinSegments += createTravelSeg("DFW", "KRK");

  SegmentsBuilder builder(*_trx, *_motherItin, *_similarItin);
  SegmentsBuilder::TSVec result = builder.constructByOriginAndDestination(originalSegments);

  ASSERT_EQ(result, expectedSegments);
}

TEST_F(SegmentsBuilderTest, testConstructByOnDDifferentLegs)
{
  SegmentsBuilder::TSVec& motherItinSegments(_motherItin->travelSeg());
  SegmentsBuilder::TSVec& similarItinSegments(_similarItin->travelSeg());

  SegmentsBuilder::TSVec originalSegments, adjustedSegments, expectedSegments;

  motherItinSegments += createTravelSeg("MIA", "LAX", 0);
  TravelSeg* segment = createTravelSeg("LAX", "DFW", 0);
  motherItinSegments += segment;
  originalSegments += segment;
  segment = createTravelSeg("DFW", "LHR", 1);
  motherItinSegments += segment;
  originalSegments += segment;
  segment = createTravelSeg("LHR", "KRK", 1);
  motherItinSegments += segment;
  originalSegments += segment;
  segment = createTravelSeg("KRK", "LAX", 2);
  motherItinSegments += segment;
  originalSegments += segment;
  motherItinSegments += createTravelSeg("LAX", "MIA", 2);

  segment = createTravelSeg("LAX", "WAS", 0);
  similarItinSegments += segment;
  expectedSegments += segment;
  segment = createTravelSeg("WAS", "DFW", 0);
  similarItinSegments += segment;
  expectedSegments += segment;
  segment = createTravelSeg("DFW", "KRK", 1);
  similarItinSegments += segment;
  expectedSegments += segment;
  segment = createTravelSeg("KRK", "WAS", 2);
  similarItinSegments += segment;
  expectedSegments += segment;
  segment = createTravelSeg("WAS", "LAX", 2);
  similarItinSegments += segment;
  expectedSegments += segment;

  SegmentsBuilder builder(*_trx, *_motherItin, *_similarItin);
  SegmentsBuilder::TSVec result = builder.constructByOriginAndDestination(originalSegments);

  ASSERT_EQ(result, expectedSegments);
}

TEST_F(SegmentsBuilderTest, testSimilarItinGeoConsistent_DifferentCities)
{
  SegmentsBuilder::TSVec& motherSegs(_motherItin->travelSeg());
  SegmentsBuilder::TSVec& similarSegs(_similarItin->travelSeg());

  AirSegBuilder airbld(_memHandle);

  motherSegs = {airbld.withLocs("AAA", "BBB").withLegId(0).build(),
                airbld.withLocs("BBB", "CCC").withLegId(1).build()};
  similarSegs = {airbld.withLocs("AAA", "BBB").withLegId(0).build(),
                 airbld.withLocs("BBB", "DDD").withLegId(1).build()};

  SegmentsBuilder builder(*_trx, *_motherItin, *_similarItin);
  ASSERT_FALSE(builder.similarItinGeoConsistent());
}

TEST_F(SegmentsBuilderTest, testSimilarItinGeoConsistent_Arunk)
{
  SegmentsBuilder::TSVec& motherSegs(_motherItin->travelSeg());
  SegmentsBuilder::TSVec& similarSegs(_similarItin->travelSeg());

  AirSegBuilder airbld(_memHandle);
  ArunkSegBuilder arunkbld(_memHandle);

  motherSegs = {airbld.withLocs("AAA", "BBB").withLegId(0).build(),
                airbld.withLocs("BBB", "CCC").withLegId(1).build()};
  similarSegs = {airbld.withLocs("AAA", "BBB").withLegId(0).build(),
                 arunkbld.withLocs("BBB", "CCC").withLegId(1).build()};

  SegmentsBuilder builder(*_trx, *_motherItin, *_similarItin);
  ASSERT_FALSE(builder.similarItinGeoConsistent());
}

TEST_F(SegmentsBuilderTest, testSimilarItinGeoConsistent_True)
{
  SegmentsBuilder::TSVec& motherSegs(_motherItin->travelSeg());
  SegmentsBuilder::TSVec& similarSegs(_similarItin->travelSeg());

  AirSegBuilder airbld(_memHandle);
  ArunkSegBuilder arunkbld(_memHandle);

  motherSegs = {airbld.withLocs("AAA", "BBB").withLegId(0).build(),
                arunkbld.withLocs("BBB", "CCC").withLegId(1).build(),
                airbld.withLocs("CCC", "DDD").withLegId(1).build()};
  similarSegs = {airbld.withLocs("AAA", "BBB").withLegId(0).build(),
                 arunkbld.withLocs("BBB", "CCC").withLegId(1).build(),
                 airbld.withLocs("CCC", "DDD").withLegId(1).build()};

  SegmentsBuilder builder(*_trx, *_motherItin, *_similarItin);
  ASSERT_TRUE(builder.similarItinGeoConsistent());
}
}
}
