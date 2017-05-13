#include "RexPricing/OptimizationMapper.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/ExcItin.h"
#include "DataModel/Itin.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>
#include <vector>

namespace tse
{
namespace
{
auto fcMatchingCheck = [](bool value) { return value; };
}

class OptimizationMapperTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _mHandle(new TestConfigInitializer);
  }

  void TearDown()
  {
    _mHandle.clear();
  }

protected:
  AirSeg* createSeg(LocCode origin, LocCode destination)
  {
    AirSeg* seg = _mHandle(new AirSeg());
    seg->origin() = getLoc(origin);
    seg->destination() = getLoc(destination);

    return seg;
  }

  FareCompInfo* createFareCompInfo(const std::vector<TravelSeg*> tvlSegments)
  {
    FareCompInfo* fc = _mHandle(new FareCompInfo());
    FareMarket* fm = _mHandle(new FareMarket());
    fm->origin() = tvlSegments.front()->origin();
    fm->destination() = tvlSegments.back()->destination();
    fm->travelSeg() = tvlSegments;
    fc->fareMarket() = fm;

    return fc;
  }

  ExcItin* createExcItin()
  {
    std::vector<TravelSeg*> segs;
    segs.push_back(createSeg("PAR", "MAD"));
    segs.push_back(createSeg("MAD", "LON"));
    segs.push_back(createSeg("LON", "MAD"));
    segs.push_back(createSeg("MAD", "PAR"));

    setPnrSegment(segs);

    FareCompInfo* firstFC = createFareCompInfo({segs[0], segs[1]});
    FareCompInfo* secondFC = createFareCompInfo({segs[2], segs[3]});

    ExcItin* itin = _mHandle(new ExcItin());
    itin->fareComponent() = {firstFC, secondFC};
    itin->furthestPointSegmentOrder() = 2;

    return itin;
  }

  Itin* createItin(const std::vector<TravelSeg*>& tvlSegments, uint32_t furthestPoint)
  {
    Itin* itin = _mHandle(new Itin());
    itin->travelSeg() = tvlSegments;
    itin->furthestPointSegmentOrder() = furthestPoint;

    return itin;
  }

  void setPnrSegment(std::vector<TravelSeg*>& segs)
  {
    uint32_t index = 0;
    for(TravelSeg* seg : segs)
    {
      seg->pnrSegment() = ++index;
    }
  }

  Loc* getLoc(LocCode locCode)
  {
    Loc* loc = _mHandle(new Loc());
    if (locCode == "PAR")
    {
      loc->loc() = locCode;
      loc->nation() = "FR";
      loc->subarea() = 21;
      loc->area() = 2;
      return loc;
    }
    else if (locCode == "MAD")
    {
      loc->loc() = locCode;
      loc->nation() = "ES";
      loc->subarea() = 21;
      loc->area() = 2;
      return loc;
    }
    else if (locCode == "LON")
    {
      loc->loc() = locCode;
      loc->nation() = "GB";
      loc->subarea() = 21;
      loc->area() = 2;
      return loc;
    }
    else if (locCode == "BCN")
    {
      loc->loc() = locCode;
      loc->nation() = "ES";
      loc->subarea() = 21;
      loc->area() = 2;
      return loc;
    }
    else if (locCode == "BER")
    {
      loc->loc() = locCode;
      loc->nation() = "DE";
      loc->subarea() = 21;
      loc->area() = 2;
      return loc;
    }
    else if (locCode == "AXZ")
    {
      loc->loc() = locCode;
      loc->nation() = "SA";
      loc->subarea() = 22;
      loc->area() = 2;
      return loc;
    }
    else
    {
      throw ErrorResponseException::INVALID_CITY_AIRPORT_CODE;
    }
  }

  std::vector<bool> getMatchedFCs(const OptimizationMapper& mapper)
  {
    return mapper._matchedFCs;
  }

  TestMemHandle _mHandle;
};

TEST_F(OptimizationMapperTest, cityMatchTest)
{
  ExcItin* excItin = createExcItin();

  std::vector<TravelSeg*> segs;
  segs.push_back(createSeg("PAR", "MAD"));
  segs.push_back(createSeg("MAD", "LON"));
  segs.push_back(createSeg("LON", "MAD"));
  segs.push_back(createSeg("MAD", "PAR"));

  setPnrSegment(segs);
  Itin* itin = createItin(segs, 2);

  OptimizationMapper mapper;
  mapper.processMapping(*excItin, *itin, nullptr);

  std::vector<bool> matchedFCs = getMatchedFCs(mapper);
  ASSERT_TRUE(std::all_of(matchedFCs.begin(), matchedFCs.end(), fcMatchingCheck));
}

TEST_F(OptimizationMapperTest, countryMatchTest)
{
  ExcItin* excItin = createExcItin();

  std::vector<TravelSeg*> newItinSegs;
  newItinSegs.push_back(createSeg("PAR", "MAD"));
  newItinSegs.push_back(createSeg("MAD", "BCN"));
  newItinSegs.push_back(createSeg("BCN", "MAD"));
  newItinSegs.push_back(createSeg("MAD", "PAR"));

  setPnrSegment(newItinSegs);

  Itin* itin = createItin(newItinSegs, 2);

  OptimizationMapper mapper;
  mapper.processMapping(*excItin, *itin, nullptr);

  std::vector<bool> matchedFCs = getMatchedFCs(mapper);
  ASSERT_TRUE(std::all_of(matchedFCs.begin(), matchedFCs.end(), fcMatchingCheck));
}

TEST_F(OptimizationMapperTest, subareaMatchTest)
{
  ExcItin* excItin = createExcItin();

  std::vector<TravelSeg*> newItinSegs;
  newItinSegs.push_back(createSeg("PAR", "MAD"));
  newItinSegs.push_back(createSeg("MAD", "BER"));
  newItinSegs.push_back(createSeg("BER", "MAD"));
  newItinSegs.push_back(createSeg("MAD", "PAR"));

  setPnrSegment(newItinSegs);

  Itin* itin = createItin(newItinSegs, 2);

  OptimizationMapper mapper;
  mapper.processMapping(*excItin, *itin, nullptr);

  std::vector<bool> matchedFCs = getMatchedFCs(mapper);
  ASSERT_TRUE(std::all_of(matchedFCs.begin(), matchedFCs.end(), fcMatchingCheck));
}

TEST_F(OptimizationMapperTest, unmappedTest)
{
  ExcItin* excItin = createExcItin();

  std::vector<TravelSeg*> newItinSegs;
  newItinSegs.push_back(createSeg("PAR", "AXZ"));
  newItinSegs.push_back(createSeg("AXZ", "PAR"));

  setPnrSegment(newItinSegs);

  Itin* itin = createItin(newItinSegs, 1);

  OptimizationMapper mapper;
  mapper.processMapping(*excItin, *itin, nullptr);

  std::vector<bool> matchedFCs = getMatchedFCs(mapper);
  ASSERT_FALSE(std::all_of(matchedFCs.begin(), matchedFCs.end(), fcMatchingCheck));
}


}
