
#pragma once

#include <vector>

#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Itin.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class TestLeg
{
public:
  TestLeg(int legId, TestMemHandle& handle) : _id(legId), _memHandle(&handle) {}

  void populateVector(std::vector<TravelSeg*>& v) const
  {
    for (std::vector<TravelSeg*>::const_iterator iter = _segments.begin(); iter != _segments.end();
         ++iter)
    {
      v.push_back(*iter);
    }
  }

  void populateItin(Itin& itin) const { populateVector(itin.travelSeg()); }

  TravelSeg* front() const
  {
    CPPUNIT_ASSERT(_segments.size() > 0);
    return _segments[0];
  }

  TravelSeg* back() const
  {
    CPPUNIT_ASSERT(_segments.size() > 0);
    return _segments[_segments.size() - 1];
  }

  void addSegment(bool air = true)
  {
    TravelSeg* segment = 0;
    if (air)
    {
      segment = _memHandle->create<AirSeg>();
    }
    else
    {
      segment = _memHandle->create<ArunkSeg>();
    }
    segment->legId() = _id;
    _segments.push_back(segment);
  }

private:
  int _id;
  std::vector<TravelSeg*> _segments;
  TestMemHandle* _memHandle;
};

class TestItin
{
public:
  TestItin(TestMemHandle& handle) : _memHandle(handle) {}

  void addLeg(int id, int segmentsNr)
  {
    CPPUNIT_ASSERT(segmentsNr > 0);
    TestLeg tl(id, _memHandle);
    for (int i = 0; i < segmentsNr; ++i)
    {
      tl.addSegment();
    }
    _legs.push_back(tl);
  }

  // true - air segment
  // false - arunk segment
  void addLeg(int id, const std::vector<bool>& airness)
  {
    CPPUNIT_ASSERT(airness.size() > 0);
    TestLeg tl(id, _memHandle);
    for (std::vector<bool>::const_iterator iter = airness.begin(); iter != airness.end(); ++iter)
    {
      if (true == *iter)
      {
        tl.addSegment(true);
      }
      else
      {
        tl.addSegment(false);
      }
    }
    _legs.push_back(tl);
  }

  void populateItin(Itin& itin) const
  {
    for (std::vector<TestLeg>::const_iterator iter = _legs.begin(); iter != _legs.end(); ++iter)
    {
      iter->populateItin(itin);
    }

    for (std::vector<FareMarket*>::const_iterator iter = _fareMarkets.begin();
         iter != _fareMarkets.end();
         ++iter)
    {
      itin.fareMarket().push_back(*iter);
    }
  }

  unsigned int getLegsNumber() const { return _legs.size(); }

  const TestLeg& getLegAtIndex(unsigned int index) const
  {
    CPPUNIT_ASSERT(index >= 0);
    CPPUNIT_ASSERT(index < _legs.size());
    return _legs[index];
  }

  // build FareMarket containing segments from
  // startSeg to endSeg, inclusive
  // segments are zero-based
  FareMarket* buildFareMarket(int startSeg, int endSeg) const
  {
    // Dump segments in linear fashion
    std::vector<TravelSeg*> v;
    for (std::vector<TestLeg>::const_iterator iter = _legs.begin(); iter != _legs.end(); ++iter)
    {
      iter->populateVector(v);
    }

    FareMarket* fm = _memHandle.create<FareMarket>();
    for (int i = startSeg; i <= endSeg; ++i)
    {
      fm->travelSeg().push_back(v.at(i));
    }
    return fm;
  }

  // build fare market and store it internally
  // to place it into an itin in populateItin()
  FareMarket* addFareMarket(int startSeg, int endSeg)
  {
    FareMarket* fm = buildFareMarket(startSeg, endSeg);
    _fareMarkets.push_back(fm);
    return fm;
  }

private:
  std::vector<TestLeg> _legs;
  std::vector<FareMarket*> _fareMarkets;
  TestMemHandle& _memHandle;
};
} // namespace tsetest
