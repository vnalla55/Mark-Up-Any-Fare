//----------------------------------------------------------------------------
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

#pragma once

#include <algorithm>
#include <vector>

namespace tse
{

class TravelSeg;

class CouponMatcher
{
  friend class CouponMatcherTest;

public:
  typedef std::vector<TravelSeg*> TravelSegments;

  bool match(const TravelSegments& excSegs, const TravelSegments& newSegs) const;

private:
  typedef TravelSegments::const_iterator TravelSegConstIterator;
  typedef TravelSegments::iterator TravelSegIterator;

  TravelSegConstIterator getFirstUnflownSegment(const TravelSegments& segs) const;

  bool matchUnflown(TravelSegConstIterator excBegin,
                    TravelSegConstIterator excEnd,
                    TravelSegConstIterator newBegin,
                    TravelSegConstIterator newEnd) const;

  bool matchDomestic(TravelSegConstIterator excBegin,
                     TravelSegConstIterator excEnd,
                     TravelSegConstIterator newBegin,
                     TravelSegConstIterator newEnd) const;

  bool matchInternational(TravelSegConstIterator excBegin,
                          TravelSegConstIterator excEnd,
                          TravelSegConstIterator newBegin,
                          TravelSegConstIterator newEnd) const;

  template <typename I, typename P>
  bool removeFirst(I begin, I& end, P pred) const
  {
    I it = std::find_if(begin, end, pred);
    if (it == end)
      return false;

    end = std::copy(it + 1, end, it);
    return true;
  }

  bool identifySameOriginCityAndDestinationCity(const TravelSeg& seg,
                                                TravelSegIterator begin,
                                                TravelSegIterator& end) const;
  bool identifySameOriginCityAndDestinationCountry(const TravelSeg& seg,
                                                   TravelSegIterator begin,
                                                   TravelSegIterator& end) const;
  bool identifySameOriginCountryAndDestinationCity(const TravelSeg& seg,
                                                   TravelSegIterator begin,
                                                   TravelSegIterator& end) const;
  bool identifySameOriginCountryAndDestinationCountry(const TravelSeg& seg,
                                                      TravelSegIterator begin,
                                                      TravelSegIterator& end) const;
};

} // tse

