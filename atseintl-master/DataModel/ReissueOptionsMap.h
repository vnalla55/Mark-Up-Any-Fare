//-------------------------------------------------------------------
//
//  File:        ReissueOptionsMap.h
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

#pragma once

#include "Common/DateTime.h"
#include "Common/ExchShopCalendarUtils.h"
#include "DataModel/PaxTypeFare.h"

#include <map>
#include <utility>
#include <vector>

namespace tse
{
class VoluntaryChangesInfo;
class ReissueSequence;

class ReissueOptions
{
public:
  using R3WithDateRange = std::pair<const VoluntaryChangesInfo*, ExchShopCalendar::DateRange>;
  using ReissueSeqWithDateRange = std::pair<const ReissueSequence*, ExchShopCalendar::DateRange>;
  using PaxTypeFareWithR3 = std::pair<const PaxTypeFare*, const VoluntaryChangesInfo*>;

  void insertOption(const PaxTypeFare* ptf,
                    const VoluntaryChangesInfo* rec3,
                    const ExchShopCalendar::DateRange dateRange = ExchShopCalendar::DateRange());

  void insertOption(const PaxTypeFare* ptf,
                    const VoluntaryChangesInfo* rec3,
                    const ReissueSequence* t988seq,
                    const ExchShopCalendar::DateRange dateRange = ExchShopCalendar::DateRange());

  size_t getRec3s(const PaxTypeFare* ptf,
                  std::vector<R3WithDateRange>& r3v) const;

  size_t getT988s(const PaxTypeFare* ptf,
                  const VoluntaryChangesInfo* rec3,
                  std::vector<ReissueSeqWithDateRange>& t988v) const;

private:
  std::vector<std::pair<const PaxTypeFare*, R3WithDateRange>> _volChanges;
  std::vector<std::pair<PaxTypeFareWithR3, ReissueSeqWithDateRange>> _reissueSeqs;

};
}

