//----------------------------------------------------------------------------
//  File:        RoutingControllerESV.cpp
//  Created:     2008-07-20
//
//  Description: ESV routing controller
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "Fares/RoutingControllerESV.h"

#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "DBAccess/MarketRoutingInfo.h"

namespace tse
{
static Logger
logger("atseintl.Fares.RoutingControllerESV");

bool
RoutingControllerESV::validateRouting(Itin* itin, PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(_trx, "FVO VALIDATE ROUTING");

  LOG4CXX_DEBUG(logger, "RoutingControllerESV::validateRouting(Itin*, PaxTypeFare*)");

  // Get travel segments for specific fare market
  std::vector<TravelSeg*>& travelSegVec = paxTypeFare->fareMarket()->travelSeg();

  bool isNonStop = ((1 == travelSegVec.size()) && (true == travelSegVec[0]->hiddenStops().empty()));

  bool isDirect = (1 == travelSegVec.size());

  switch (paxTypeFare->fare()->fareInfo()->negViaAppl())
  {
  case 'R':

    switch (paxTypeFare->fare()->fareInfo()->nonstopDirectInd())
    {
    case 'N':

      if (true != isNonStop)
      {
        return false;
      }

      break;

    case 'D':

      if (true != isDirect)
      {
        return false;
      }

      break;

    case 'E':

      if ((true != isNonStop) && (true != isDirect))
      {
        return false;
      }

      break;
    }

    break;

  case 'N':

    switch (paxTypeFare->fare()->fareInfo()->nonstopDirectInd())
    {
    case 'N':

      if (true == isNonStop)
      {
        return false;
      }

      break;

    case 'D':

      if (true == isDirect)
      {
        return false;
      }

      break;

    case 'E':

      if ((true == isNonStop) || (true == isDirect))
      {
        return false;
      }

      break;
    }

    break;
  }

  // Skip routing validation if there is no conection cities
  if (1 == travelSegVec.size())
  {
    TSELatencyData metrics(_trx, "PASS FVO VALIDATE ROUTING");
    return true;
  }

  switch (travelSegVec.size())
  {
  case 2:
  {
    const MarketRoutingInfo& boundRoutingInfo =
        _trx.dataHandle().getMarketRoutingInfo(paxTypeFare->vendor(),
                                               paxTypeFare->carrier(),
                                               paxTypeFare->routingNumber(),
                                               paxTypeFare->tcrRoutingTariff1(),
                                               paxTypeFare->origin(),
                                               paxTypeFare->destination(),
                                               true,
                                               false);

    if (nullptr == boundRoutingInfo.singles())
    {
      return false;
    }

    if (boundRoutingInfo.singles()->empty())
    {
      return false;
    }

    // Find connection city in singles
    const LocCode& connAirport = travelSegVec[0]->destination()->loc();
    const LocCode& connCity = travelSegVec[0]->offMultiCity();

    bool bFound = false;

    if (boundRoutingInfo.singles()->end() != std::find(boundRoutingInfo.singles()->begin(),
                                                       boundRoutingInfo.singles()->end(),
                                                       connAirport))
    {
      bFound = true;
    }

    if ((false == bFound) && (connAirport != connCity) &&
        (boundRoutingInfo.singles()->end() != std::find(boundRoutingInfo.singles()->begin(),
                                                        boundRoutingInfo.singles()->end(),
                                                        connCity)))
    {
      bFound = true;
    }

    if (true == bFound)
    {
      TSELatencyData metrics(_trx, "PASS FVO VALIDATE ROUTING");
    }

    return bFound;
  }
  break;

  case 3:
  {
    const MarketRoutingInfo& boundRoutingInfo =
        _trx.dataHandle().getMarketRoutingInfo(paxTypeFare->vendor(),
                                               paxTypeFare->carrier(),
                                               paxTypeFare->routingNumber(),
                                               paxTypeFare->tcrRoutingTariff1(),
                                               paxTypeFare->origin(),
                                               paxTypeFare->destination(),
                                               false,
                                               true);

    if (nullptr == boundRoutingInfo.doubles())
    {
      return false;
    }

    if (boundRoutingInfo.doubles()->empty())
    {
      return false;
    }

    bool marketsFlipped = ((paxTypeFare->origin() != travelSegVec[0]->origin()->loc()) &&
                           (paxTypeFare->origin() != travelSegVec[0]->boardMultiCity()));

    // Find connection city in doubles
    const LocCode& connAirport1 = travelSegVec[0]->destination()->loc();
    const LocCode& connAirport2 = travelSegVec[1]->destination()->loc();
    const LocCode& connCity1 = travelSegVec[0]->offMultiCity();
    const LocCode& connCity2 = travelSegVec[1]->offMultiCity();

    bool bFound = false;

    const LocCode& mkt1 = marketsFlipped ? connAirport2 : connAirport1;
    const LocCode& mkt2 = marketsFlipped ? connAirport1 : connAirport2;
    MarketRoutingDoubles brmp1(mkt1, mkt2);

    if (boundRoutingInfo.doubles()->end() !=
        std::find(boundRoutingInfo.doubles()->begin(), boundRoutingInfo.doubles()->end(), brmp1))
    {
      bFound = true;
    }

    if ((false == bFound) && (connAirport1 != connCity1))
    {
      const LocCode& mkt1 = marketsFlipped ? connAirport2 : connCity1;
      const LocCode& mkt2 = marketsFlipped ? connCity1 : connAirport2;
      MarketRoutingDoubles brmp2(mkt1, mkt2);

      if (boundRoutingInfo.doubles()->end() !=
          std::find(boundRoutingInfo.doubles()->begin(), boundRoutingInfo.doubles()->end(), brmp2))
      {
        bFound = true;
      }
    }

    if ((false == bFound) && (connAirport2 != connCity2))
    {
      const LocCode& mkt1 = marketsFlipped ? connCity2 : connAirport1;
      const LocCode& mkt2 = marketsFlipped ? connAirport1 : connCity2;
      MarketRoutingDoubles brmp3(mkt1, mkt2);

      if (boundRoutingInfo.doubles()->end() !=
          std::find(boundRoutingInfo.doubles()->begin(), boundRoutingInfo.doubles()->end(), brmp3))
      {
        bFound = true;
      }
    }

    if ((false == bFound) && (connAirport1 != connCity1) && (connAirport2 != connCity2))
    {
      const LocCode& mkt1 = marketsFlipped ? connCity2 : connCity1;
      const LocCode& mkt2 = marketsFlipped ? connCity1 : connCity2;
      MarketRoutingDoubles brmp4(mkt1, mkt2);

      if (boundRoutingInfo.doubles()->end() !=
          std::find(boundRoutingInfo.doubles()->begin(), boundRoutingInfo.doubles()->end(), brmp4))
      {
        bFound = true;
      }
    }

    if (true == bFound)
    {
      TSELatencyData metrics(_trx, "PASS FVO VALIDATE ROUTING");
    }

    return bFound;
  }
  break;

  default:
    LOG4CXX_ERROR(logger,
                  "RoutingControllerESV::process - Unsupported number of connection cities.");
    return false;
  }

  TSELatencyData metricsEnd(_trx, "PASS FVO VALIDATE ROUTING");
  return true;
}
}
