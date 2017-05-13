//---------------------------------------------:-------------------------------
//  File:        Diag931Collector.C
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

#include "Diagnostic/Diag931Collector.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/ShoppingTrx.h"

#include <algorithm>
#include <numeric>
#include <sstream>

namespace tse
{
namespace
{
ConfigurableValue<char>
groupItins("SHOPPING_OPT", "GROUP_ITINS", ' ');
ConfigurableValue<char>
groupInterlineItins("SHOPPING_OPT", "GROUP_INTERLINE_ITINS", ' ');
ConfigurableValue<int>
maxGroupCfg("SHOPPING_OPT", "MAX_GROUP", -1);
ConfigurableValue<int>
multiPaxMaxItn("SHOPPING_OPT", "MULTI_PAX_MAX_ITN", -1);
ConfigurableValue<int>
complexTripMaxItn("SHOPPING_OPT", "COMPLEX_TRIP_MAX_ITN", -1);
}

void
Diag931Collector::outputHeader(const ShoppingTrx& trx)
{
  char groupItin = groupItins.getValue();
  char groupInterlineItin = groupInterlineItins.getValue();
  int maxGroup = maxGroupCfg.getValue();
  int groupMaxItin = -1;

  if (!trx.isAltDates())
  {
    if (trx.paxType().size() + trx.excludedPaxType().size() > 1)
      groupMaxItin = multiPaxMaxItn.getValue();
    else if (trx.legs().size() > 2)
      groupMaxItin = complexTripMaxItn.getValue();
  }

  if (groupItin == ShoppingTrx::BLANK)
  {
    *this << "GROUP ITINS : IS NOT IN CONFIG FILE DEFAULT TO Y \n";
  }
  else
  {
    *this << "GROUP ITINS : " << groupItin << "\n";
    *this << "GROUP INTERLINE_ITINS : " << groupInterlineItin << "\n";
    *this << "MAX GROUP : " << maxGroup << "\n";
    *this << "GROUP MAX ITINS: " << groupMaxItin << "\n";
  }
}

void
Diag931Collector::outputItin(const SopIdVec& sops,
                             int16_t carrierGroupIndex,
                             const ShoppingTrx& shoppingTrx,
                             int16_t itemNo,
                             bool interItin,
                             const std::string mapKey)
{
  itemNo += 1;
  *this << "--------------------------------------------------- \n";
  *this << "ITEM  " << itemNo << "      : CARRIER GROUP :: " << carrierGroupIndex << "  ";

  if (interItin)
  {
    *this << "INTER ITIN";
  }
  else
  {
    *this << "DOM ITIN";
  }

  *this << "\n";
  size_t sopsSize = sops.size();
  *this << "SOPS :: ";

  for (size_t s = 0; s != sopsSize; ++s)
    *this << ShoppingUtil::findSopId(shoppingTrx, s, sops[s]) << " ";

  *this << "\n";
  std::string govCarrier = shoppingTrx.legs()[0].sop()[sops[0]].governingCarrier();
  *this << "GOVERNING CARRIER FOR THE FIRST SOP  " << govCarrier << "\n";
  *this << "GROUP MAP KEY :  " << mapKey << "\n";

  for (size_t s = 0; s != sopsSize; ++s)
  {
    const Itin* sopItin = shoppingTrx.legs()[s].sop()[sops[s]].itin();
    // find all item in sop is the same flight
    std::vector<TravelSeg*>::const_iterator tvlSegIter = sopItin->travelSeg().begin();

    for (; tvlSegIter != sopItin->travelSeg().end(); ++tvlSegIter)
    {
      AirSeg* sopAirSeg = dynamic_cast<AirSeg*>(*tvlSegIter);
      *this << s + 1 << " " << sopAirSeg->marketingCarrierCode();
      *this << std::setw(5) << sopAirSeg->marketingFlightNumber();
      *this << " " << std::setw(3) << sopAirSeg->origin()->loc();
      *this << " " << std::setw(3) << sopAirSeg->destination()->loc();
      std::string depDTStr = sopAirSeg->departureDT().dateToString(DDMMMYY, "");
      std::string arrDTStr = sopAirSeg->arrivalDT().dateToString(DDMMMYY, "");
      *this << " " << std::setw(5) << depDTStr;
      *this << " " << std::setw(5) << arrDTStr;
      *this << "\n";
    }
  }
}
}
