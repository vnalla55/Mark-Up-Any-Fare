//----------------------------------------------------------------------------
//
//  File:        TravelSegUtil.h
//  Copyright Sabre 2004
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
#pragma once

#include "Common/LocUtil.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"

#include <functional>
#include <vector>

namespace tse
{

class AirSeg;
class TravelSeg;
class DataHandle;
/**
 * Utility for AigSeg, TravelSeg
 **/
namespace TravelSegUtil
{

struct OriginNationEqual : public std::binary_function<TravelSeg*, NationCode, bool>
{
  bool operator()(const TravelSeg* t, const NationCode& nation) const
  {
    return t && t->origin() && t->origin()->nation() == nation;
  }
};

struct DestinationNationEqual : public std::binary_function<TravelSeg*, NationCode, bool>
{
  bool operator()(const TravelSeg* t, const NationCode& nation) const
  {
    return t && t->destination() && t->destination()->nation() == nation;
  }
};

struct NationEqual : public std::binary_function<TravelSeg*, NationCode, bool>
{
  bool operator()(const TravelSeg* t, const NationCode& nation) const
  {
    return t && ((t->origin() && t->origin()->nation() == nation) ||
                 (t->destination() && t->destination()->nation() == nation));
  }
};

struct OrigEqual
{
  OrigEqual(bool checkCity = false) : _checkCity(checkCity) {}
  OrigEqual(const Loc* loc, bool checkCity = false) : _loc(loc), _checkCity(checkCity) {}

  bool operator()(const TravelSeg* ts) { return operator()(ts, _loc); }
  bool operator()(const TravelSeg* ts, const Loc* loc)
  {
    return (_checkCity ? (ts && LocUtil::isSamePoint(
                                    *ts->origin(), ts->boardMultiCity(), *loc, loc->city()))
                       : (ts && ts->origin() == loc));
  }

private:
  const Loc* _loc = nullptr;
  bool _checkCity = false;
};

struct DestEqual
{
  DestEqual(bool checkCity = false) : _checkCity(checkCity) {}
  DestEqual(const Loc* loc, bool checkCity = false) : _loc(loc), _checkCity(checkCity) {}

  bool operator()(const TravelSeg* ts) { return operator()(ts, _loc); }

  bool operator()(const TravelSeg* ts, const Loc* loc)
  {
    return (_checkCity ? (ts && LocUtil::isSamePoint(
                                    *ts->destination(), ts->offMultiCity(), *loc, loc->city()))
                       : (ts && ts->destination() == loc));
  }

private:
  const Loc* _loc = nullptr;
  bool _checkCity = false;
};

struct ContainsLoc
{
  ContainsLoc(const Loc* loc) : _loc(loc) {}
  bool operator()(const TravelSeg* ts)
  {
    return (ts && (ts->origin() == _loc || ts->destination() == _loc));
  }

private:
  const Loc* _loc;
};

struct OriginNationEquals
{
  OriginNationEquals(const NationCode& nation) : _nation(nation) {}
  bool operator()(const TravelSeg* ts) { return (ts && (ts->origin()->nation() == _nation)); }

private:
  const NationCode& _nation;
};

const AirSeg*
firstAirSeg(const std::vector<TravelSeg*>& tvlSeg);
const AirSeg*
lastAirSeg(const std::vector<TravelSeg*>& tvlSeg);
const TravelSeg*
firstNoArunkSeg(const std::vector<TravelSeg*>& tvlSeg);
GeoTravelType
getGeoTravelType(const std::vector<TravelSeg*>& tvlSeg, DataHandle& dh);

uint32_t
hashCode(const std::vector<TravelSeg*>& tvlSeg);

void
setGeoTrvlTypeAndMltCity(AirSeg*& seg);

void
setupItinerarySegment(DataHandle& dataHandle,
                      AirSeg*& segment,
                      const DateTime& outboundDate,
                      const LocCode& boardCity,
                      const LocCode& offCity,
                      const CarrierCode& cxrCode,
                      int16_t pnrSegment);

std::vector<bool>
calculateStopOvers(const std::vector<TravelSeg*>& tvlSeg,
                   GeoTravelType geoTravelType,
                   TravelSeg::Application application = TravelSeg::OTHER,
                   bool shortSO = false);

std::vector<bool>
calculateStopOversForMultiTkt(const std::vector<TravelSeg*>& tvlSeg,
                              TravelSeg::Application application = TravelSeg::OTHER,
                              bool shortSO = false);

ArunkSeg*
buildArunk(DataHandle& dataHandle, TravelSeg* leftSeg, TravelSeg* rightSeg);
bool
isTravelSegVecOnline(const std::vector<TravelSeg*>& tSegs);
bool
isNotUSCanadaOrCanadaUS(const TravelSeg* segment);

void
setSegmentAttributes(std::vector<TravelSeg*>&, std::vector<SegmentAttributes>&);

std::vector<SegmentAttributes>
calcSegmentAttributes(const std::vector<TravelSeg*>&);
}

} // end tse namespace

