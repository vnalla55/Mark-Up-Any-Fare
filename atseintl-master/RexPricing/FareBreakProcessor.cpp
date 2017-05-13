//-------------------------------------------------------------------
//
//  File:        FareBreakProcessor.cpp
//  Created:     Aug 16, 2007
//  Authors:     Simon Li
//
//  Description: Get fare break points information from permutation;
//               Validate fare break requirement
//
//  Updates:
//
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
//-------------------------------------------------------------------

#include "RexPricing/FareBreakProcessor.h"

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/ProcessTagInfo.h"

#include <algorithm>

namespace tse
{

const std::vector<TravelSeg*>&
FareBreakProcessor::newItinTvlSegs() const
{
  return _newItin->travelSeg();
}

bool
FareBreakProcessor::setup(const Itin& excItin,
                          const Itin& newItin,
                          const std::vector<ProcessTagInfo*>& processTags)
{
  _excItin = &excItin;
  _newItin = &newItin;
  _permutationPassKeepFB = false;

  std::vector<TravelSeg*>::size_type totalTvlPoints = newItin.travelSeg().size() + 1;
  _fareBreakInfo.resize(totalTvlPoints);

  for (std::vector<TravelSeg*>::size_type locOrder = 0; locOrder < totalTvlPoints; locOrder++)
  {
    _fareBreakInfo[locOrder] = ALLOW_FAREBREAK;
  }

  std::map<uint16_t, const FareMarket*> excFMNeedKeepFB;

  std::vector<ProcessTagInfo*>::const_iterator fcTagIter = processTags.begin();
  const std::vector<ProcessTagInfo*>::const_iterator fcTagIterEnd = processTags.end();

  for (; fcTagIter != fcTagIterEnd; fcTagIter++)
  {
    const ProcessTagInfo& fcTagInfo = **fcTagIter;

    if (!findKeepFBTvl(fcTagInfo, excFMNeedKeepFB))
    {
      return false; // this permutation should fail
    }
  }

  std::map<uint16_t, const FareMarket*>::const_iterator fmIter = excFMNeedKeepFB.begin();
  std::map<uint16_t, const FareMarket*>::const_iterator fmIterEnd = excFMNeedKeepFB.end();

  FareBreakInfoIndex segOrderSearchFrom = 0;

  for (; fmIter != fmIterEnd; fmIter++)
  {
    FareBreakInfoIndex locStart = 0, locStop = 0;

    if (!findSameFBTvl(
            (fmIter->second)->travelSeg(), newItinTvlSegs(), segOrderSearchFrom, locStart, locStop))
      return false;

    segOrderSearchFrom = locStop;

    _fareBreakInfo[locStart] = MANDATORY_FAREBREAK;
    _fareBreakInfo[locStop] = MANDATORY_FAREBREAK;
    for (FareBreakInfoIndex locOrder = locStart + 1; locOrder < locStop; ++locOrder)
    {
      _fareBreakInfo[locOrder] = NO_FAREBREAK;
    }
  }

  _permutationPassKeepFB = true;
  return true;
}

bool
FareBreakProcessor::findKeepFBTvl(const ProcessTagInfo& fcTagInfo,
                                  std::map<uint16_t, const FareMarket*>& excFMNeedKeepFB) const
{
  if (fcTagInfo.processTag() == KEEP_THE_FARES ||
      fcTagInfo.processTag() == REISSUE_DOWN_TO_LOWER_FARE)
  {
    uint16_t locOrderTvlStart = _excItin->segmentOrder(fcTagInfo.fareMarket()->travelSeg().front());
    if (locOrderTvlStart < 0)
      return false;

    excFMNeedKeepFB[locOrderTvlStart] = fcTagInfo.fareMarket();
  }
  // @ TODO, decide by Tag and byte 24-31
  return true;
}

bool
FareBreakProcessor::isFareBreakValid(const FareMarket& fareMarket) const
{
  if (!_excItin || !_newItin || !_permutationPassKeepFB)
    return false;

  std::vector<TravelSeg*>::const_iterator tsBeginI = newItinTvlSegs().begin();
  std::vector<TravelSeg*>::const_iterator tsEndI = newItinTvlSegs().end();

  std::vector<TravelSeg*>::const_iterator fmTvlBeginIter =
      std::find(tsBeginI, tsEndI, fareMarket.travelSeg().front());
  if (fmTvlBeginIter == tsEndI)
    return false;

  auto startLocOrder = std::distance(tsBeginI, fmTvlBeginIter);

  if (_fareBreakInfo[startLocOrder] == NO_FAREBREAK)
    return false;

  std::vector<TravelSeg*>::const_iterator fmTvlEndIter =
      std::find(tsBeginI, tsEndI, fareMarket.travelSeg().back());
  if (fmTvlEndIter == tsEndI)
    return false;

  auto endLocOrder = std::distance(tsBeginI, fmTvlEndIter) + 1;

  if (_fareBreakInfo[endLocOrder] == NO_FAREBREAK)
    return false;

  for (auto locOrder = startLocOrder + 1; locOrder < endLocOrder; locOrder++)
  {
    if (_fareBreakInfo[locOrder] == MANDATORY_FAREBREAK)
      return false;
  }

  return true;
}

bool
FareBreakProcessor::findSameFBTvl(const std::vector<TravelSeg*>& excItinTvlSegs,
                                  const std::vector<TravelSeg*>& newItinTvlSegs,
                                  const FareBreakInfoIndex& startSearchSegOrder,
                                  FareBreakInfoIndex& locStart,
                                  FareBreakInfoIndex& locStop) const
{
  // simple scheme, no check if we have two travel segments from/to same
  // points in same PNR now
  const LocCode& origAirport = excItinTvlSegs.front()->origAirport();
  const LocCode& destAirport = excItinTvlSegs.back()->destAirport();

  std::vector<TravelSeg*>::const_iterator tvlSegI = newItinTvlSegs.begin();
  const std::vector<TravelSeg*>::const_iterator tvlSegIEnd = newItinTvlSegs.end();

  FareBreakInfoIndex segOrder = 0;
  bool foundOrig = false;
  FareBreakInfoIndex numSkipSeg = startSearchSegOrder;

  for (; tvlSegI != tvlSegIEnd; tvlSegI++, segOrder++)
  {
    if (numSkipSeg != 0)
    {
      numSkipSeg--;
      continue;
    }

    if ((*tvlSegI)->origAirport() == origAirport)
    {
      locStart = segOrder;
      foundOrig = true;
    }
    if ((*tvlSegI)->destAirport() == destAirport)
    {
      locStop = segOrder + 1;
      if (foundOrig)
        return true;
    }
  }

  return false;
}

} // tse
