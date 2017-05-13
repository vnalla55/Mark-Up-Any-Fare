// ----------------------------------------------------------------
//
//   Author : Michal Mlynek
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Diversity.h"
#include "Diagnostic/Diag941Collector.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/Diversity/DmcRequirement.h"
#include "Pricing/Shopping/FiltersAndPipes/INamedPredicate.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"

namespace tse
{

class DmcScheduleRepeatLimitRequirement : public DmcRequirement,
                                          public utils::INamedPredicate<utils::SopCombination>
{
public:
  DmcScheduleRepeatLimitRequirement(DmcRequirementsSharedContext& sharedCtx)
    : _stats(sharedCtx._stats),
      _scheduleRepeatLimitValue(sharedCtx._trx.getRequest()->getScheduleRepeatLimit()),
      _dc(nullptr)
  {
    if (sharedCtx._dc != nullptr)
      _dc = dynamic_cast<Diag941Collector*>(sharedCtx._dc);
  }

  DmcScheduleRepeatLimitRequirement(ItinStatistic& stats, uint16_t scheduleRepeatLimit)
    : _stats(stats), _scheduleRepeatLimitValue(scheduleRepeatLimit), _dc(nullptr)
  {
  }

  bool getThrowAwayCombination(const shpq::SopIdxVecArg& comb) const
  {
    if (LIKELY(_scheduleRepeatLimitValue == 0))
      return false;

    for (unsigned int legId = 0; legId < comb.size(); ++legId)
    {
      if (_stats.getSopPairing(legId, comb[legId]) >= _scheduleRepeatLimitValue)
      {
        if (_dc)
        {
          _dc->addCombinationResult(comb, Diag941Collector::SRL);
        }

        return true;
      }
    }

    return false;
  }

  bool operator()(const utils::SopCombination& comb) override { return !getThrowAwayCombination(comb); }

  std::string getName() const override { return "Schedule Repeat Limit"; }

private:
  const ItinStatistic& _stats;
  const uint16_t _scheduleRepeatLimitValue;
  Diag941Collector* _dc;
};

} // ns tse

