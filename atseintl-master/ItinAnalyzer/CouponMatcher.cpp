//----------------------------------------------------------------------------
//  Copyright Sabre 2007
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

#include "Common/LocUtil.h"
#include "ItinAnalyzer/CouponMatcher.h"

#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"

namespace tse
{

namespace
{
typedef CouponMatcher::TravelSegments::const_iterator TravelSegConstIterator;
typedef CouponMatcher::TravelSegments::iterator TravelSegIterator;

class IsAirUnflownSegment
{
public:
  bool operator()(const TravelSeg* tvlSeg) const
  {
    return (tvlSeg->segmentType() == Air || tvlSeg->segmentType() == Open) && tvlSeg->unflown();
  }
};

template <const LocCode& (TravelSeg::*method)() const>
struct IsTheSameCity
{
  bool operator()(const TravelSeg& seg1, const TravelSeg& seg2) const
  {
    return (seg1.*method)() == (seg2.*method)();
  }
};

bool
usCa(const Loc& l)
{
  return l.nation() == CANADA || l.nation() == UNITED_STATES;
}
bool
vikingsLand(const Loc& l)
{
  return l.nation() == DENMARK || l.nation() == NORWAY || l.nation() == SWEDEN;
}

template <const Loc* (TravelSeg::*method)() const>
struct IsTheSameCountry
{
  bool operator()(const TravelSeg& seg1, const TravelSeg& seg2) const
  {
    const Loc& o = *(seg1.*method)();
    const Loc& d = *(seg2.*method)();

    if (o.nation() == d.nation())
      return true;

    if ((usCa(o) && usCa(d)) || (LocUtil::isRussianGroup(o) && LocUtil::isRussianGroup(d)))
      return true;

    if (vikingsLand(o) && vikingsLand(d))
      return true;

    return false;
  }
};

template <typename LocPred1, typename LocPred2>
struct HasSpecificLocation
{
  HasSpecificLocation(const TravelSeg& seg) : _seg(seg) {}
  bool operator()(const TravelSeg* seg) const { return _pred1(*seg, _seg) && _pred2(*seg, _seg); }

private:
  const TravelSeg& _seg;
  LocPred1 _pred1;
  LocPred2 _pred2;
};

template <GeoTravelType type1, GeoTravelType type2>
struct HasSpecificTravelType
{
  bool operator()(const TravelSeg* seg) const
  {
    return (seg->geoTravelType() == type1 || seg->geoTravelType() == type2);
  }
};

typedef IsTheSameCity<&TravelSeg::boardMultiCity> IsTheSameOriginCity;
typedef IsTheSameCity<&TravelSeg::offMultiCity> IsTheSameDestinationCity;

typedef IsTheSameCountry<&TravelSeg::origin> IsTheSameOriginCountry;
typedef IsTheSameCountry<&TravelSeg::destination> IsTheSameDestinationCountry;

typedef HasSpecificLocation<IsTheSameOriginCity, IsTheSameDestinationCity>
HasTheSameOriginCityAndDestinationCity;
typedef HasSpecificLocation<IsTheSameOriginCity, IsTheSameDestinationCountry>
HasTheSameOriginCityAndDestinationCountry;
typedef HasSpecificLocation<IsTheSameOriginCountry, IsTheSameDestinationCity>
HasTheSameOriginCountryAndDestinationCity;
typedef HasSpecificLocation<IsTheSameOriginCountry, IsTheSameDestinationCountry>
HasTheSameOriginCountryAndDestinationCountry;

typedef HasSpecificTravelType<GeoTravelType::International, GeoTravelType::Transborder> IsInternational;
typedef HasSpecificTravelType<GeoTravelType::Domestic, GeoTravelType::ForeignDomestic> IsDomestic;

} // namespace

bool
CouponMatcher::match(const TravelSegments& excSegs, const TravelSegments& newSegs) const
{
  TravelSegConstIterator excBegin = getFirstUnflownSegment(excSegs),
                         newBegin = getFirstUnflownSegment(newSegs);

  return matchUnflown(excBegin, excSegs.end(), newBegin, newSegs.end());
}

CouponMatcher::TravelSegConstIterator
CouponMatcher::getFirstUnflownSegment(const TravelSegments& segs) const
{
  return std::find_if(segs.begin(), segs.end(), IsAirUnflownSegment());
}

bool
CouponMatcher::matchUnflown(TravelSegConstIterator excBegin,
                            TravelSegConstIterator excEnd,
                            TravelSegConstIterator newBegin,
                            TravelSegConstIterator newEnd) const
{
  return (matchDomestic(excBegin, excEnd, newBegin, newEnd) ||
          matchInternational(excBegin, excEnd, newBegin, newEnd));
}

bool
CouponMatcher::matchDomestic(TravelSegConstIterator excBegin,
                             TravelSegConstIterator excEnd,
                             TravelSegConstIterator newBegin,
                             TravelSegConstIterator newEnd) const
{
  std::vector<TravelSeg*> changedSeg(newBegin, newEnd);
  TravelSegIterator end = changedSeg.end();

  IsDomestic isDomestic;
  for (; excBegin != excEnd; ++excBegin)
  {
    if (!(**excBegin).isAir())
      continue;

    if (isDomestic(*excBegin) &&
        !(identifySameOriginCityAndDestinationCity(**excBegin, changedSeg.begin(), end) ||
          identifySameOriginCityAndDestinationCountry(**excBegin, changedSeg.begin(), end) ||
          identifySameOriginCountryAndDestinationCity(**excBegin, changedSeg.begin(), end) ||
          identifySameOriginCountryAndDestinationCountry(**excBegin, changedSeg.begin(), end)))
      return false;
  }
  return true;
}

bool
CouponMatcher::matchInternational(TravelSegConstIterator excBegin,
                                  TravelSegConstIterator excEnd,
                                  TravelSegConstIterator newBegin,
                                  TravelSegConstIterator newEnd) const
{
  return (std::count_if(newBegin, newEnd, IsInternational()) <=
          std::count_if(excBegin, excEnd, IsInternational()));
}

bool
CouponMatcher::identifySameOriginCityAndDestinationCity(const TravelSeg& seg,
                                                        TravelSegIterator begin,
                                                        TravelSegIterator& end) const
{
  return removeFirst(begin, end, HasTheSameOriginCityAndDestinationCity(seg));
}

bool
CouponMatcher::identifySameOriginCityAndDestinationCountry(const TravelSeg& seg,
                                                           TravelSegIterator begin,
                                                           TravelSegIterator& end) const
{
  return removeFirst(begin, end, HasTheSameOriginCityAndDestinationCountry(seg));
}

bool
CouponMatcher::identifySameOriginCountryAndDestinationCity(const TravelSeg& seg,
                                                           TravelSegIterator begin,
                                                           TravelSegIterator& end) const
{
  return removeFirst(begin, end, HasTheSameOriginCountryAndDestinationCity(seg));
}

bool
CouponMatcher::identifySameOriginCountryAndDestinationCountry(const TravelSeg& seg,
                                                              TravelSegIterator begin,
                                                              TravelSegIterator& end) const
{
  return removeFirst(begin, end, HasTheSameOriginCountryAndDestinationCountry(seg));
}

} // tse
