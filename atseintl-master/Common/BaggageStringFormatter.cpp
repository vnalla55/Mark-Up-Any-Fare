//-------------------------------------------------------------------
//  Copyright Sabre 2012
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
#include "Common/BaggageStringFormatter.h"

#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"

#include <boost/algorithm/string/join.hpp>
#include <iterator>

namespace tse
{
namespace BaggageStringFormatter
{

struct TravelLocation
{
  const LocCode& name;
  std::vector<int16_t> segmentsNumbersOccursIn;
};

using TravelLocations = std::vector<TravelLocation>;

TravelLocations
toTravelSegments(TravelSegPtrVecCI begin, TravelSegPtrVecCI end)
{
  TravelLocations travelLocations;
  for (auto segmentIter = begin; segmentIter != end; ++segmentIter)
  {
    auto segment = *segmentIter;
    travelLocations.push_back(TravelLocation{segment->origin()->loc(),      {segment->pnrSegment()}});
    travelLocations.push_back(TravelLocation{segment->destination()->loc(), {segment->pnrSegment()}});
  }
  return travelLocations;
}

TravelLocations
coalesceContiguousTravelLocations(TravelLocations travelLocations)
{
  TravelLocations coalescedTravelLocations;
  for (auto location : travelLocations)
  {
    if (coalescedTravelLocations.size() && coalescedTravelLocations.back().name == location.name)
      coalescedTravelLocations.back().segmentsNumbersOccursIn.push_back(location.segmentsNumbersOccursIn.back());
    else
      coalescedTravelLocations.push_back(TravelLocation{location.name, {location.segmentsNumbersOccursIn}});
  }
  return coalescedTravelLocations;
}

std::string
travelLocationToString(const TravelLocation& travelLocation)
{
  std::vector<std::string> segmentNumbers;
  for (auto segmentNumber : travelLocation.segmentsNumbersOccursIn)
    segmentNumbers.push_back(std::to_string(segmentNumber));

  return travelLocation.name + " " + boost::algorithm::join(segmentNumbers, ",");
}

std::string
getLocationName(const TravelLocation& travelLocation)
{
  return travelLocation.name;
}

std::string
printTravelSegments(const TravelLocations& travelLocations,
                    std::string (*travelLocationToString) (const TravelLocation&),
                    std::string separator)
{
  std::vector<std::string> locationsStrings;

  for (auto location : travelLocations)
    locationsStrings.push_back(travelLocationToString(location));

  return boost::algorithm::join(locationsStrings, separator);
}

std::string
printBaggageTravelSegmentsWithNumbering(TravelSegPtrVecCI begin, TravelSegPtrVecCI end)
{
  return printTravelSegments(coalesceContiguousTravelLocations(toTravelSegments(begin, end)), travelLocationToString, " - ");
}

std::string
printBaggageTravelSegmentsWithoutNumbering(TravelSegPtrVecCI begin, TravelSegPtrVecCI end)
{
  return printTravelSegments(coalesceContiguousTravelLocations(toTravelSegments(begin, end)), getLocationName, "-");
}

std::string
printBaggageTravelSegmentsWithNumbering(const BaggageTravel& bt)
{
  return printBaggageTravelSegmentsWithNumbering(bt.getTravelSegBegin(), bt.getTravelSegEnd());
}

std::string
printBaggageTravelSegmentsWithoutNumbering(const BaggageTravel& bt)
{
  return printBaggageTravelSegmentsWithoutNumbering(bt.getTravelSegBegin(), bt.getTravelSegEnd());
}

void
old_printBaggageTravelSegments(TravelSegPtrVecCI begin, TravelSegPtrVecCI end, std::ostringstream& output)
{
  LocCode previousLoc;

  for (; begin != end; ++begin)
  {
    const LocCode& origin = (*begin)->origin()->loc();
    const LocCode& destination = (*begin)->destination()->loc();

    if (origin != previousLoc)
      output << origin << "-";

    output << destination;

    if ((begin + 1) != end)
      output << "-";

    previousLoc = destination;
  }
}

void
old_printBaggageTravelSegments(const BaggageTravel& bt, std::ostringstream& output)
{
  old_printBaggageTravelSegments(bt.getTravelSegBegin(), bt.getTravelSegEnd(), output);
}

inline static bool
shouldDisplayMarketing(const PricingTrx& trx, const bool usDot, const bool defer)
{
  if (usDot || defer)
    return true;
  return TrxUtil::isIataReso302MandateActivated(trx);
}

inline static bool
shouldDisplayOperating(const PricingTrx& trx, const bool usDot, const bool defer)
{
  if (usDot)
    return false;
  if (defer)
    return true;
  return !TrxUtil::isIataReso302MandateActivated(trx);
}

void
printBtCarriers(const BaggageTravel& bt,
                const bool usDot,
                const bool defer,
                std::ostringstream& out)
{
  const bool displayMarketing = shouldDisplayMarketing(*bt._trx, usDot, defer);
  const bool displayOperating = shouldDisplayOperating(*bt._trx, usDot, defer);
  const AirSeg* mss = bt.getMsSeg(usDot)->toAirSeg();
  const AirSeg* fcis = bt.getFciSeg(usDot)->toAirSeg();

  out << "MSC MARKETING : ";

  if (mss && displayMarketing)
    out << mss->marketingCarrierCode();

  out << "\n";
  out << "MSC OPERATING : ";

  if (mss && displayOperating)
    out << ((mss->segmentType() == Open) ? mss->marketingCarrierCode()
                                         : mss->operatingCarrierCode());

  out << "\n";
  out << "FCIC MARKETING : ";

  if (fcis && displayMarketing)
    out << fcis->marketingCarrierCode();

  out << "\n";
  out << "FCIC OPERATING : ";

  if (fcis && displayOperating)
    out << ((fcis->segmentType() == Open) ? fcis->marketingCarrierCode()
                                          : fcis->operatingCarrierCode());

  out << "\n";
}


} // BaggageStringFormatter
} // tse
