//-------------------------------------------------------------------
//
//  File:
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/ScheduleCountMgr.h"

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DSS/FlightCount.h"

#include <algorithm>
#include <vector>

namespace tse
{
namespace ScheduleCountMgr
{
namespace
{
Logger
logger("atseintl.FareDisplay.ScheduleCountMgr");

bool
isCarrierHasFare(const std::vector<PaxTypeFare*>& fares, const CarrierCode carrier)
{
  if (carrier == INDUSTRY_CARRIER)
    return false; // we dont care of YY fares as we dont want to display YY schedule count.
  return std::any_of(fares.cbegin(),
                     fares.cend(),
                     [carrier](const PaxTypeFare* ptf)
                     { return ptf && (ptf->carrier() == carrier); });
}

bool
isCarrierHasSchedule(const std::vector<FlightCount*>& flightCounts, const CarrierCode cxr)
{
  return std::any_of(flightCounts.cbegin(),
                     flightCounts.cend(),
                     [cxr](const FlightCount* flightCount)
                     { return flightCount && flightCount->equalCxr(cxr); });
}

void
addFltCount(FareDisplayTrx& trx, std::vector<FlightCount*>& fltCounts, const CarrierCode carrier)
{
  FlightCount* ft = &trx.dataHandle().safe_create<FlightCount>(carrier);
  fltCounts.push_back(ft);
}

void
removeFltCount(std::vector<FlightCount*>& flightCounts, const CarrierCode cxr)
{
  std::vector<FlightCount*>::iterator i =
      std::find_if(flightCounts.begin(),
                   flightCounts.end(),
                   [cxr](const FlightCount* flightCount)
                   { return flightCount && flightCount->equalCxr(cxr); });

  if (i != flightCounts.end())
    flightCounts.erase(i);
}

void
filterCarriers(FareDisplayTrx& trx)
{
  for (const auto prefCxr : trx.preferredCarriers())
  {
    if (isCarrierHasFare(trx.allPaxTypeFare(), prefCxr))
    {
      if (!isCarrierHasSchedule(trx.fdResponse()->scheduleCounts(), prefCxr))
        addFltCount(trx, trx.fdResponse()->scheduleCounts(), prefCxr);
    }
    else
    {
      removeFltCount(trx.fdResponse()->scheduleCounts(), prefCxr);
    }
  }
}
} // namespace (anonymous)

void
process(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(logger, "Entered ScheduleCountMgr::process() ");
  if (trx.isScheduleCountRequested())
  {
    LOG4CXX_DEBUG(logger, "Filtering Schedule Count  ");
    filterCarriers(trx);
  }
  else
  {
    LOG4CXX_DEBUG(logger, "NO SCHEDULE COUNT PRESENT ");
  }
}
} // namespace ScheduleCountMgr
} // namespace tse
