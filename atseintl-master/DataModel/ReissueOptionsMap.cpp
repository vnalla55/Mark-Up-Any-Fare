//-------------------------------------------------------------------
//
//  File:        ReissueOptionsMap.cpp
//  Created:     May 22, 2007
//  Authors:     Grzegorz Cholewiak
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

#include "DataModel/ReissueOptionsMap.h"

#include <algorithm>
#include <iterator>
#include <set>

#include <stdint.h>

namespace tse
{
namespace
{
template <typename T>

bool sortPairs(const std::pair<T, ExchShopCalendar::DateRange>& lhs,
               const std::pair<T, ExchShopCalendar::DateRange>& rhs)
{
  return lhs.first < rhs.first &&
         lhs.second.firstDate.getOnlyDate() < rhs.second.lastDate.getOnlyDate() &&
         lhs.second.firstDate.getOnlyDate() < rhs.second.lastDate.getOnlyDate();
}


template <typename T>
bool equalPairs(const std::pair<T, ExchShopCalendar::DateRange>& lhs,
                const std::pair<T, ExchShopCalendar::DateRange>& rhs)
{
  return lhs.first == rhs.first &&
         lhs.second.firstDate.getOnlyDate() == rhs.second.lastDate.getOnlyDate() &&
         lhs.second.firstDate.getOnlyDate() == rhs.second.lastDate.getOnlyDate();
}
}

void
ReissueOptions::insertOption(const PaxTypeFare* ptf,
                             const VoluntaryChangesInfo* rec3,
                             const ExchShopCalendar::DateRange dateRange)
{
  _volChanges.emplace_back(ptf, std::make_pair(rec3, dateRange));
}

void ReissueOptions::insertOption(const PaxTypeFare* ptf,
                                  const VoluntaryChangesInfo* rec3,
                                  const ReissueSequence* t988seq,
                                  const ExchShopCalendar::DateRange dateRange)
{
  auto ptfWithR3 = std::make_pair(ptf, rec3);
  auto seqWithDateRange = std::make_pair(t988seq, dateRange);
  _reissueSeqs.emplace_back(ptfWithR3, seqWithDateRange);
}

size_t
ReissueOptions::getRec3s(const PaxTypeFare* ptf,
                         std::vector<R3WithDateRange>& r3v) const
{
  for(const auto& r3Pair : _volChanges)
  {
    if (ptf == r3Pair.first)
    {
      r3v.push_back(r3Pair.second);
    }
  }
  std::sort(r3v.begin(), r3v.end(), &sortPairs<const VoluntaryChangesInfo*>);
  std::unique(r3v.begin(), r3v.end(), &equalPairs<const VoluntaryChangesInfo*>);
  return r3v.size();
}

size_t
ReissueOptions::getT988s(const PaxTypeFare* ptf,
                         const VoluntaryChangesInfo* rec3,
                         std::vector<ReissueSeqWithDateRange>& t988v) const
{
  for(const auto& seqPair : _reissueSeqs)
  {
    if (ptf == seqPair.first.first && rec3 == seqPair.first.second)
    {
      t988v.push_back(seqPair.second);
    }
  }
  std::sort(t988v.begin(), t988v.end(), &sortPairs<const ReissueSequence*>);
  std::unique(t988v.begin(), t988v.end(), &equalPairs<const ReissueSequence*>);
  return t988v.size();
}

} //tse
