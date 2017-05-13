// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2012
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/TaxSP9300.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"

namespace tse
{

bool
TaxSP9300::validateTripTypes(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex)
{
  if (UNLIKELY(taxCodeReg.restrictionTransit().size() <= 1))
  {
    TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
    locIt->setSkipHidden(true);

    locIt->toSegmentNo(startIndex);

    if (!(locIt->isInLoc1(taxCodeReg, trx) && locIt->isNextSegAirSeg()))
      return false;

    if (!taxCodeReg.restrictionTransit().empty())
    {
      TaxRestrictionTransit& restrictTransit = taxCodeReg.restrictionTransit().front();
      if (restrictTransit.transitHours())
        locIt->setStopHours(restrictTransit.transitHours());
    }
    locIt->setSurfaceAsStop(true);

    if (taxCodeReg.loc2() == "DE")
    {
      if (locIt->hasNext())
      {
        locIt->next();
        if (locIt->isInLoc2(taxCodeReg, trx) && locIt->isStop())
        {
          endIndex = startIndex;
          return true;
        }
      }
      return false;
    }
    else
    {
      while (locIt->hasNext() && locIt->isNextSegAirSeg())
      {
        locIt->next();
        if (locIt->loc()->nation() == "DE")
        {
          if (locIt->isStop())
            return false;
        }

        if (locIt->isInLoc2(taxCodeReg, trx))
        {
          endIndex = locIt->prevSegNo();

          // this is for diagnostic
          if (locIt->hasNext() && locIt->isNextSegAirSeg())
          {
            locIt->next();
            if (locIt->isInLoc2(taxCodeReg, trx))
              endIndex = locIt->prevSegNo();
          }

          return true;
        }
      }
      return false;
    }
  }

  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
  locIt->setSkipHidden(true);

  locIt->toSegmentNo(startIndex);

  if (!(locIt->isInLoc1(taxCodeReg, trx) && locIt->isNextSegAirSeg()))
    return false;

  locIt->setSurfaceAsStop(true);

  if (taxCodeReg.loc2() == "DE")
  {
    while (locIt->hasNext())
    {
      locIt->next();
      if (locIt->isInLoc2(taxCodeReg, trx))
      {
        if (isStop(trx, *locIt, taxCodeReg))
        {
          endIndex = locIt->prevSegNo();
          return true;
        }
      }
      else
      {
        return false;
      }
    }
  }
  else
  {
    while (locIt->hasNext())
    {
      locIt->next();
      if (locIt->loc()->nation() == "DE")
      {
        if (isStop(trx, *locIt, taxCodeReg))
          return false;
      }

      if (locIt->isInLoc2(taxCodeReg, trx) && locIt->isPrevSegAirSeg())
      {
        endIndex = locIt->prevSegNo();
        return true;
      }
    }
  }

  return false;
}

bool
TaxSP9300::validateTransit(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t travelSegIndex)
{
  if (taxCodeReg.restrictionTransit().size() <= 1)
    return Tax::validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
  locIt->setSkipHidden(true);

  locIt->toSegmentNo(travelSegIndex);
  locIt->setSurfaceAsStop(true);

  return isStop(trx, *locIt, taxCodeReg);
}

bool
TaxSP9300::isStop(PricingTrx& trx, TaxLocIterator& locIt, TaxCodeReg& taxCodeReg)
{
  if (!locIt.hasPrevious() || !locIt.hasNext() || !locIt.isPrevSegAirSeg() || locIt.isMirror())
    return true;

  const EquipmentType& prevEquipmentType = locIt.prevSeg()->equipmentType();
  if (_landToAirStopover &&
      ((prevEquipmentType == TRAIN) || (prevEquipmentType == TGV) || (prevEquipmentType == ICE) ||
       (prevEquipmentType == BOAT) || (prevEquipmentType == LMO) || (prevEquipmentType == BUS)))
    return true;

  std::vector<TaxRestrictionTransit>::const_iterator trIt = taxCodeReg.restrictionTransit().begin();

  locIt.next();
  const Loc* nextLoc = locIt.loc();
  locIt.previous();

  for (; trIt != taxCodeReg.restrictionTransit().end(); ++trIt)
  {
    if (trIt->viaLoc().empty())
    {
      continue;
    }

    if (LocUtil::isInLoc(*nextLoc,
                         trIt->viaLocType(),
                         trIt->viaLoc(),
                         Vendor::SABRE,
                         MANUAL,
                         LocUtil::TAXES,
                         GeoTravelType::International,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT()))
    {
      if (trIt->transitHours())
        locIt.setStopHours(trIt->transitHours());
      return locIt.isStop();
    }
  }

  return true;
}
}
