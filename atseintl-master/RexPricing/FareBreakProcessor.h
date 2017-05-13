//-------------------------------------------------------------------
//
//  File:        FareBreakProcessor.h
//  Created:     August 16, 2007
//  Authors:     Simon Li
//
//  Updates:
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

#include <map>
#include <vector>

namespace tse
{
class FareMarket;
class Itin;
class ProcessTagInfo;
class TravelSeg;

class FareBreakProcessor
{
public:
  FareBreakProcessor() : _excItin(nullptr), _newItin(nullptr), _permutationPassKeepFB(0) {}

  bool
  setup(const Itin& excItin, const Itin& newItin, const std::vector<ProcessTagInfo*>& processTags);

  bool isFareBreakValid(const FareMarket& fareMarket) const;

protected:
  enum FareBreakInd
  {
    NO_FAREBREAK,
    ALLOW_FAREBREAK,
    MANDATORY_FAREBREAK
  };

  typedef std::vector<FareBreakInd>::size_type FareBreakInfoIndex;

  bool findKeepFBTvl(const ProcessTagInfo& fcTagInfo,
                     std::map<uint16_t, const FareMarket*>& excFMNeedKeepFB) const;

  bool findSameFBTvl(const std::vector<TravelSeg*>& oldFMTvlSegs,
                     const std::vector<TravelSeg*>& newItinTvlSegs,
                     const FareBreakInfoIndex& startSearchSegOrder,
                     FareBreakInfoIndex& locStart,
                     FareBreakInfoIndex& locStop) const;

  const std::vector<TravelSeg*>& newItinTvlSegs() const;

  const Itin* _excItin;
  const Itin* _newItin;

  std::vector<FareBreakInd> _fareBreakInfo;

private:
  bool _permutationPassKeepFB;
};

} // tse

