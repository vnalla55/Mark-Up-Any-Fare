//----------------------------------------------------------------------------
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

#include "Diagnostic/Diag220Collector.h"

#include "DataModel/AirSeg.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/PaxTypeInfo.h"

#include <iomanip>
#include <iostream>
#include <set>

namespace tse
{
void
Diag220Collector::printHeader()
{
  if (_active)
  {
    *this << "***************     PASSENGER TYPE DISPLAY      ***************\n";
  }
}

//----------------------------------------------------------------------------
// Display Requested PaxTypes
//----------------------------------------------------------------------------
void
Diag220Collector::displayPaxTypes(PricingTrx& trx)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(std::ios::left, std::ios::adjustfield);

  //--------------------------------------------------
  // Create a unique set of carriers in the itinerary
  //--------------------------------------------------
  typedef std::set<CarrierCode> CarrierSet;
  CarrierSet cxrSet;

  if (trx.getOptions()->isIataFares())
  {
    cxrSet.insert(INDUSTRY_CARRIER);
  }
  else
  {
    const AirSeg* airSeg;

    std::vector<Itin*>::const_iterator itinI = trx.itin().begin();
    for (; itinI != trx.itin().end(); itinI++)
    {
      std::vector<TravelSeg*>::const_iterator travelSegI = (*itinI)->travelSeg().begin();

      for (; travelSegI != (*itinI)->travelSeg().end(); travelSegI++)
      {
        airSeg = dynamic_cast<AirSeg*>(*travelSegI);

        if (!airSeg)
        {
          continue;
        }

        cxrSet.insert(airSeg->carrier());
      }
    }
  }

  //---------------------------------------------
  // Process each paxType in the Pricing Request
  //---------------------------------------------
  const std::vector<PaxType*>& paxTypes = trx.paxType();
  std::vector<PaxType*>::const_iterator paxTypeIter = paxTypes.begin();

  for (; paxTypeIter != paxTypes.end(); ++paxTypeIter)
  {
    PaxType& curPaxType = **paxTypeIter;

    dc << "REQUESTED PASSENGER TYPE - " << curPaxType.paxType() << std::endl;

    //-----------------------------------
    // For each carrier in the itinerary
    //-----------------------------------
    std::set<CarrierCode>::const_iterator cxrItr = cxrSet.begin();
    std::set<CarrierCode>::const_iterator cxrEnd = cxrSet.end();

    for (; cxrItr != cxrEnd; ++cxrItr)
    {
      displayCxrPaxTypes(*cxrItr, curPaxType);
    }

    displayCxrPaxTypes(ANY_CARRIER, curPaxType);

    dc << " \n";
  }
}

void
Diag220Collector::displayCxrPaxTypes(const CarrierCode& carrier, const PaxType& paxType)
{
  const std::map<CarrierCode, std::vector<PaxType*>*>& actualPaxTypes = paxType.actualPaxType();
  std::map<CarrierCode, std::vector<PaxType*>*>::const_iterator grpI = actualPaxTypes.find(carrier);

  if (grpI == actualPaxTypes.end())
    return;

  DiagCollector& dc(*this);

  const std::vector<PaxType*>* grpPaxTypes = (*grpI).second;
  std::vector<PaxType*>::const_iterator paxTypesI, paxTypesEnd;
  paxTypesI = grpPaxTypes->begin();
  paxTypesEnd = grpPaxTypes->end();

  for (; paxTypesI != paxTypesEnd; ++paxTypesI)
  {
    PaxType& curPaxType = **paxTypesI;
    dc << carrier << " " << curPaxType.paxType() << " " << std::setw(4) << curPaxType.vendorCode();
    if (!curPaxType.stateCode().empty())
    {
      dc << " STATE-" << curPaxType.stateCode();
    }

    const PaxTypeInfo*& paxTypeInfo = curPaxType.paxTypeInfo();
    if (paxTypeInfo != nullptr)
    {
      dc << " ADT-" << paxTypeInfo->adultInd() << " CHD-" << paxTypeInfo->childInd() << " INF-"
         << paxTypeInfo->infantInd();
    }
    dc << " \n";
  }
}
}
