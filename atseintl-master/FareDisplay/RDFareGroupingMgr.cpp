//-------------------------------------------------------------------
//  File   : RDFareGroupingMgr.cpp
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

#include "FareDisplay/RDFareGroupingMgr.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/RoutingSequenceGenerator.h"

namespace tse
{
static Logger
logger("atseintl.FareDisplay.Grouping");

bool
RDFareGroupingMgr::groupFares(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(logger, "Entering RDFareGroupingMgr::groupFares() ");

  std::vector<PaxTypeFare*>& fares(trx.allPaxTypeFare());

  if (trx.isERD() && fares.size() > 1)
  {
    groupFaresByMultiAirport(fares, trx.getOptions(), trx);
  }

  if (fares.empty())
  {
    LOG4CXX_INFO(logger, "Leaving RDFareGroupingMgr::groupFares() -- No Fares for the Market ");
    return true;
  }
  else
  {
    if (fares.size() > 1)
    {
      processGrouping(trx);

      std::string rtg;
      std::vector<PaxTypeFare*>::iterator i(fares.begin());
      for (; i != fares.end();)
      {
        FareDisplayInfo* info = (*i)->fareDisplayInfo();
        if (info != nullptr && !info->routingSequence().empty())
          rtg = info->routingSequence();
        else
          rtg = (*i)->routingNumber();
        if (isNotRequestedFBR(trx, rtg))
          i = fares.erase(i);
        else
          ++i;
      }
    }
    else if (trx.isLongRD() || trx.isERD())
    {
      FareDisplayInfo* info = (fares.front())->fareDisplayInfo();
      if (info)
      {
        if (isSequenceTranslationRequired(trx))
        {
          RoutingSequenceGenerator sequenceGenerator;
          info->routingSequence() = sequenceGenerator.getRTGSeq(fares.front()->globalDirection());
        }
      }
    }
    else if (!trx.getOptions()->routing().empty())
    {
      FareDisplayInfo* info = (fares.front())->fareDisplayInfo();
      if (info)
        info->routingSequence() = trx.getOptions()->routing();
    }
  }
  return true;
}

bool
RDFareGroupingMgr::isNotRequestedFBR(FareDisplayTrx& trx, std::string& rtg)
{
  if (!trx.isShortRD())
    return false;
  if (!rtg.empty())
    return rtg != trx.getOptions()->routingNumber();
  else
    return false;
}

// -------------------------------------------------------------------
// @MethodName  RDFareGroupingMgr::groupFaresByMultiAirport
//
// Iterate through all matched PaxTypefares filtering out more restrictive
// locations e.g. JFK-LON & NYC-LON -result will be NYC-LON.
// The only one fare may be returned.
// -------------------------------------------------------------------------
void
RDFareGroupingMgr::groupFaresByMultiAirport(std::vector<PaxTypeFare*>& faresToMerge,
                                            const FareDisplayOptions* fdo,
                                            FareDisplayTrx& trx)
{
  LOG4CXX_INFO(logger, "Entered RDFareGroupingMgr::mergeFaresByMultiAirport()");

  std::vector<PaxTypeFare*> filtered[3];
  std::vector<PaxTypeFare*>::iterator i;

  // Filtration on first city
  for (i = faresToMerge.begin(); i != faresToMerge.end(); i++)
  {
    if ((*i)->fareMarket()->boardMultiCity() == (*i)->fare()->market1())
    {
      filtered[0].push_back(*i);
    }
  }

  if (filtered[0].empty())
  {
    filtered[0] = faresToMerge;
  }

  // Filtration on 2-nd city
  for (i = filtered[0].begin(); i != filtered[0].end(); i++)
  {
    if ((*i)->fareMarket()->offMultiCity() == (*i)->fare()->market2())
    {
      filtered[1].push_back(*i);
      break;
    }
  }

  if (filtered[1].empty())
  {
    filtered[1].push_back(filtered[0].front());
  }

  const PaxTypeFare* baseFare = nullptr;

  for (i = filtered[1].begin(); i != filtered[1].end(); i++)
  {
    baseFare = (*i)->fareWithoutBase();
    if (fdo && baseFare->createDate().isValid() && fdo->linkNumber() == baseFare->linkNumber() &&
        fdo->sequenceNumber() == baseFare->sequenceNumber() &&
        fdo->createDate().dateToString(DDMMMYY, "") ==
            baseFare->createDate().dateToString(DDMMMYY, "") &&
        fdo->createTime() == baseFare->createDate().timeToSimpleString())
    {
      filtered[2].push_back(*i);
      break;
    }
  }
  if (filtered[2].empty())
  {
    filtered[2] = filtered[1];
  }
  faresToMerge = filtered[2];

  LOG4CXX_INFO(logger, "Leaving RDFareGroupingMgr::mergeFaresByMultiAirport()");
}
}
