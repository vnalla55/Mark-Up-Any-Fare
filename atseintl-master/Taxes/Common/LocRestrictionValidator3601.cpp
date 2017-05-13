//---------------------------------------------------------------------------
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

#include <vector>

#include "Common/Convert.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/LocRestrictionValidator3601.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"

namespace tse
{

void
LocRestrictionValidator3601::findFareBreaks(const FarePath& farePath)
{
  std::vector<PricingUnit*>::const_iterator pricingUnitI;
  std::vector<FareUsage*>::iterator fareUsageI;
  _fareBreaksSet.clear();

  for (pricingUnitI = farePath.pricingUnit().begin(); pricingUnitI != farePath.pricingUnit().end();
       pricingUnitI++)
  {
    for (fareUsageI = (*pricingUnitI)->fareUsage().begin();
         fareUsageI != (*pricingUnitI)->fareUsage().end();
         fareUsageI++)
    {
      TravelSeg* travelSegBack = (*fareUsageI)->travelSeg().back();
      _fareBreaksSet.insert(tax::Convert::intToUshort(farePath.itin()->segmentOrder(travelSegBack) - 1));
    }
  }
  _fareBreaksFound = true;
}

void
LocRestrictionValidator3601::findFarthestPoint(PricingTrx& trx,
                                               const Itin& itin,
                                               const uint16_t& startIndex)
{
  _farthestPointFound = false;
  TravelSeg* startTravelSeg = itin.travelSeg()[startIndex];
  const AirSeg* startFlt = dynamic_cast<AirSeg*>(startTravelSeg);
  if (!startFlt)
    return;

  std::set<uint16_t>::const_iterator fareBreaksI;
  uint32_t highMiles = 0;
  uint32_t currentMiles = 0;

  for (fareBreaksI = _fareBreaksSet.begin(); fareBreaksI != _fareBreaksSet.end(); fareBreaksI++)
  {
    TravelSeg* currTravelSeg = itin.travelSeg()[*fareBreaksI];
    if (currTravelSeg->destination()->nation() == startTravelSeg->origin()->nation())
      continue;
    const AirSeg* currFlt = dynamic_cast<AirSeg*>(currTravelSeg);
    if (!currFlt)
      continue;
    currentMiles = ItinUtil::journeyMileage(startFlt, currFlt, trx, itin.travelDate());
    if (currentMiles > highMiles)
    {
      highMiles = currentMiles;
      _farthestSegIndex = *fareBreaksI;
    }
    _farthestPointFound = true;
  }
}

bool
LocRestrictionValidator3601::validateDestination(PricingTrx& trx,
                                                 TaxResponse& taxResponse,
                                                 TaxCodeReg& taxCodeReg,
                                                 uint16_t& startIndex,
                                                 uint16_t& endIndex)
{
  Itin* itin = taxResponse.farePath()->itin();
  TravelSeg* travelSeg;
  uint16_t i = 0;
  uint16_t index;

  if (!_farthestPointFound)
    i = 1;
  for (; i < 2; ++i)
  {
    if (i == 0)
    {
      travelSeg = itin->travelSeg()[_farthestSegIndex];
      index = _farthestSegIndex;
    }
    else
    {
      travelSeg = itin->travelSeg().back();
      index = tax::Convert::ulongToUshort(itin->travelSeg().size() - 1);
    }
    bool locMatch = LocUtil::isInLoc(*(travelSeg->destination()),
                                     taxCodeReg.loc2Type(),
                                     taxCodeReg.loc2(),
                                     Vendor::SABRE,
                                     MANUAL,
                                     LocUtil::TAXES,
                                     GeoTravelType::International,
                                     EMPTY_STRING(),
                                     trx.getRequest()->ticketingDT());

    if (taxCodeReg.loc2Type() == LOCTYPE_ZONE)
    {
      if (!locMatch)
        continue;
    }
    else if ((locMatch && taxCodeReg.loc2ExclInd() == TAX_EXCLUDE) ||
             (!locMatch && taxCodeReg.loc2ExclInd() != TAX_EXCLUDE))
    {
      continue;
    }
    endIndex = index;
    return true;
  }

  if (taxResponse.diagCollector()->isActive())
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::DESTINATION_LOCATION, Diagnostic816);
  }
  return false;
} // end TAX_DESTINATION
}
