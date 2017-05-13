//----------------------------------------------------------------------------
//
//  File:        TseUtil.h
//  Created:     4/12/2004
//  Authors:     KS
//
//  Description: Common functions required for ATSE shopping/pricing.
//
//  Updates:
//
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

#include "Common/DateTime.h"
#include "Common/TseStringTypes.h"

#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

#include <string>
#include <vector>

namespace tse
{

class TravelSeg;
class PricingTrx;
class Loc;
class FlightFinderTrx;
class PaxTypeFare;

inline bool
isDigit(char ch)
{
  return ch >= '0' && ch <= '9';
}

namespace TseUtil
{
void
alert(const char* msg);
DateTime
getTravelDate(const std::vector<TravelSeg*>& travelSegVec);
DateTime
getBookingDate(const std::vector<TravelSeg*>& travelSegVec);

TravelSeg*
getFirstUnflownAirSeg(const std::vector<TravelSeg*>& travelSegVec, const DateTime& requestDT);
TravelSeg*
firstDatedSegBeforeDT(const std::vector<TravelSeg*>& tvlSegs,
                      const DateTime& refDT,
                      PricingTrx& trx);

uint32_t
greatCircleMiles(const Loc& p1, const Loc& p2);

template <class T, int size>
bool
isMember(const T& t, T (&arr)[size])
{
  return (std::find(&arr[0], &arr[size], t) != &arr[size]);
}

bool
isNotAirSeg(const TravelSeg* tvlSeg);
std::string
getGeoType(GeoTravelType geoTravelType);
GeoTravelType
getGeoType(char geoTravelType);

bool
boolValue(const std::string& parm);

class FFOwrtApplicabilityPred final
{
public:
  FFOwrtApplicabilityPred(FlightFinderTrx& trx) : _trx(trx) {}

  bool operator()(const PaxTypeFare* ptFare);

private:
  FlightFinderTrx& _trx;
};

struct Solution
{
  uint16_t _skipped;
  std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                        std::vector<TravelSeg*>::const_iterator>*> _routes;

  Solution(uint16_t skipped,
           const std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                                       std::vector<TravelSeg*>::const_iterator>*>& routes)
    : _skipped(skipped), _routes(routes)
  {
  }

  bool operator<(const Solution& s) const
  {
    return _skipped <= s._skipped && _routes.size() < s._routes.size();
  }
};

typedef boost::multi_index_container<
    Solution,
    boost::multi_index::indexed_by<boost::multi_index::ordered_non_unique<
        boost::multi_index::identity<Solution> > > > SolutionSet;

const std::string TPM_TO_MPM_RATIO = "1.2";

inline uint32_t
getTPMFromMPM(uint32_t mpm)
{
  return mpm * 5 / 6;
}

void
splitTextLine(const std::string& str, std::vector<std::string>& vec);

void
splitTextLine(const std::string& str,
              std::vector<std::string>& vec,
              size_t maxLineLen,
              const bool remSpacesAtBeg);

} //TseUtil

} // end tse namespace

