/*----------------------------------------------------------------------------
 *  File:    MIPFamilyLogicUtils.cpp
 *  Created: Jan 27, 2012
 *  Authors: Artur de Sousa Rocha
 *
 *  Copyright Sabre 2012
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/

#include "Pricing/MIPFamilyLogicUtils.h"

#include "Common/FallbackUtil.h"
#include "Common/TNBrands/ItinBranding.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/AirSeg.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/GroupFarePath.h"

namespace tse
{
FALLBACK_DECL(setFurthestPointForChildItins)
FALLBACK_DECL(familyLogicSideTrip)

void
MIPFamilyLogicUtils::populateSimilarItinBrAllData(Itin& child, Itin& motherItin)
{
  //TODO(andrzej.fediuk) FL: add assertion for mothers itin branding?
  if (motherItin.isItinBrandingSet() && !child.isItinBrandingSet())
  {
    child.setItinBranding(&motherItin.getItinBranding());
    child.fmpMatrix() = motherItin.fmpMatrix();
  }
}

void
MIPFamilyLogicUtils::populateSimilarItinData(const PricingTrx& trx, Itin& child, Itin& motherItin)
{
  child.geoTravelType() = motherItin.geoTravelType();
  child.ticketingCarrier() = motherItin.ticketingCarrier();
  child.ticketingCarrierPref() = motherItin.ticketingCarrierPref();
  child.sequenceNumber() = motherItin.sequenceNumber();
  if (fallback::setFurthestPointForChildItins(&trx))
    child.furthestPointSegmentOrder() = motherItin.furthestPointSegmentOrder();
  child.salesNationRestr() = motherItin.salesNationRestr();
  child.tripCharacteristics() = motherItin.tripCharacteristics();
  child.intlSalesIndicator() = motherItin.intlSalesIndicator();
  child.calculationCurrency() = motherItin.calculationCurrency();
  child.originationCurrency() = motherItin.originationCurrency();
  child.paxGroup() = motherItin.paxGroup();
  child.simpleTrip() = motherItin.simpleTrip();

  if (!fallback::familyLogicSideTrip(&trx))
    child.setHasSideTrip(motherItin.hasSideTrip());

  if (trx.isBRAll())
    populateSimilarItinBrAllData(child, motherItin);
}

bool
MIPFamilyLogicUtils::isThroughFareMarket(const PricingTrx& trx, const FareMarket& fareMarket)
{
  return std::any_of(trx.originDest().begin(),
                     trx.originDest().end(),
                     [&fareMarket](const PricingTrx::OriginDestination& od)
                     {
    return fareMarket.boardMultiCity() == od.boardMultiCity &&
           fareMarket.offMultiCity() == od.offMultiCity &&
           fareMarket.travelSeg().front()->departureDT().date() == od.travelDate.date();
  });
}

bool
MIPFamilyLogicUtils::pricedThroughFare(const PricingTrx& trx, const FarePath& farePath)
{
  for (const PricingUnit* pricingUnit : farePath.pricingUnit())
  {
    for (const FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      const FareMarket& fm = *fareUsage->paxTypeFare()->fareMarket();
      if (!isThroughFareMarket(trx, fm))
        return false;
    }
  }
  return true;
}

bool
MIPFamilyLogicUtils::pricedThroughFare(const PricingTrx& trx, const std::vector<FPPQItem*>& groupFPath)
{
  return std::all_of(groupFPath.begin(),
                     groupFPath.end(),
                     [&trx](const FPPQItem* fppqItem)
                     { return pricedThroughFare(trx, *fppqItem->farePath()); });
}

bool
MIPFamilyLogicUtils::pricedThroughFare(const PricingTrx& trx, const GroupFarePath& groupFPath)
{
  return pricedThroughFare(trx, groupFPath.groupFPPQItem());
}

bool
MIPFamilyLogicUtils::sameCarriers(const Itin& motherItin, const Itin& estItin)
{
  if (motherItin.travelSeg().size() != estItin.travelSeg().size())
    return false;

  auto itinTvlSeg = motherItin.travelSeg().cbegin();
  const auto itinTvlSegEnd = motherItin.travelSeg().cend();
  auto estTvlSeg = estItin.travelSeg().cbegin();

  for (; itinTvlSeg != itinTvlSegEnd; ++itinTvlSeg, ++estTvlSeg)
  {
    if (!(*itinTvlSeg)->isAir() || !(*estTvlSeg)->isAir())
      continue; // do not check if it's ARUNK segment

    const AirSeg* itinAirSeg = (*itinTvlSeg)->toAirSeg();
    const AirSeg* estAirSeg = (*estTvlSeg)->toAirSeg();

    if (itinAirSeg->carrier() != estAirSeg->carrier())
      return false;
  }

  return true;
}

} //tse
