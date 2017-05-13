// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/TaxSP1800.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Common/TaxRound.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "Common/TaxRound.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/CarrierValidator.h"
#include "DataModel/FarePath.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "Common/FallbackUtil.h"
#include "Common/TseUtil.h"

using namespace tse;

std::vector<TravelSeg*>::const_iterator
TaxSP1800::findTravelSegInItin(TravelSeg* travelSeg, const Itin* itin)
{
  std::vector<TravelSeg*>::const_iterator travelSegIter = itin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator travelSegEndIter = itin->travelSeg().end();

  for (; travelSegIter != travelSegEndIter; travelSegIter++)
  {
    if (itin->segmentOrder(travelSeg) == itin->segmentOrder(*travelSegIter))
      return travelSegIter;
  }

  return travelSegEndIter;
}

bool
TaxSP1800::wasTravelSegProcessed(uint16_t travelSegIndex)
{
  return travelSegIndex >= _travelSegProcessedStartIndex &&
         travelSegIndex <= _travelSegProcessedEndIndex;
}

void
TaxSP1800::resetTravelSegProcessedInfo()
{
  _travelSegProcessedStartIndex = 0xFFFF;
  _travelSegProcessedEndIndex = 0xFFFF;
  _numberOfFees = 0;
}

bool
TaxSP1800::validateLocRestrictions(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t& startIndex,
                                   uint16_t& endIndex)
{

  bool status = Tax::validateLocRestrictions(trx, taxResponse, taxCodeReg, startIndex, endIndex);

  if (!status)
    resetTravelSegProcessedInfo();

  return status;
}

bool
TaxSP1800::validateTripTypes(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex)
{
  bool status = Tax::validateTripTypes(trx, taxResponse, taxCodeReg, startIndex, endIndex);

  if (!status)
    resetTravelSegProcessedInfo();

  return status;
}

bool
TaxSP1800::validateRange(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex)
{
  bool status = Tax::validateRange(trx, taxResponse, taxCodeReg, startIndex, endIndex);

  if (!status)
    resetTravelSegProcessedInfo();

  return status;
}

bool
TaxSP1800::validateTransit(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t travelSegIndex)
{
  bool status = TaxSP18::validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

  if (!status)
    resetTravelSegProcessedInfo();

  return status;
}

bool
TaxSP1800::validateCarrierExemption(PricingTrx& trx,
                                    TaxResponse& taxResponse,
                                    TaxCodeReg& taxCodeReg,
                                    uint16_t travelSegIndex)
{
  bool status = Tax::validateCarrierExemption(trx, taxResponse, taxCodeReg, travelSegIndex);

  if (!status)
    resetTravelSegProcessedInfo();

  return status;
}

bool
TaxSP1800::validateEquipmentExemption(PricingTrx& trx,
                                      TaxResponse& taxResponse,
                                      TaxCodeReg& taxCodeReg,
                                      uint16_t travelSegIndex)
{
  bool status = Tax::validateEquipmentExemption(trx, taxResponse, taxCodeReg, travelSegIndex);

  if (!status)
    resetTravelSegProcessedInfo();

  return status;
}

bool
TaxSP1800::validateFareClass(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t travelSegIndex)
{
  bool status = Tax::validateEquipmentExemption(trx, taxResponse, taxCodeReg, travelSegIndex);

  if (!status)
    resetTravelSegProcessedInfo();

  return status;
}

bool
TaxSP1800::validateCabin(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex)
{
  bool status = Tax::validateCabin(trx, taxResponse, taxCodeReg, travelSegIndex);

  if (!status)
    resetTravelSegProcessedInfo();

  return status;
}

bool
TaxSP1800::validateTicketDesignator(PricingTrx& trx,
                                    TaxResponse& taxResponse,
                                    TaxCodeReg& taxCodeReg,
                                    uint16_t travelSegIndex)
{
  bool status = Tax::validateTicketDesignator(trx, taxResponse, taxCodeReg, travelSegIndex);

  if (!status)
    resetTravelSegProcessedInfo();

  return status;
}

bool
TaxSP1800::validateSequence(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            uint16_t& travelSegStartIndex,
                            uint16_t& travelSegEndIndex,
                            bool checkSpn = false)
{
  bool status = TaxSP18::validateSequence(
      trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex, checkSpn);

  if (!status)
    resetTravelSegProcessedInfo();

  return status;
}

bool
TaxSP1800::validateFinalGenericRestrictions(PricingTrx& trx,
                                            TaxResponse& taxResponse,
                                            TaxCodeReg& taxCodeReg,
                                            uint16_t& travelSegStartIndex,
                                            uint16_t& travelSegEndIndex)
{
  _travelSegStartIndex = travelSegStartIndex;
  _travelSegEndIndex = travelSegStartIndex;

  if (wasTravelSegProcessed(travelSegStartIndex))
    return _numberOfFees;

  const Itin* itin = taxResponse.farePath()->itin();

  // TODO remove?
  if (itin->travelSeg().size() <= travelSegStartIndex)
    return false;

  TravelSeg* travelSeg = itin->travelSeg()[travelSegStartIndex];
  std::vector<TravelSeg*>::const_iterator travelSegIter = findTravelSegInItin(travelSeg, itin);
  std::vector<TravelSeg*>::const_iterator travelSegEndIter = itin->travelSeg().end();

  if (travelSegIter == travelSegEndIter)
    return false;

  const AirSeg* airSeg;
  CarrierValidator carrierValidator;
  travelSegEndIndex = travelSegStartIndex;
  _numberOfFees = 1;
  bool locMatch = false;

  if ((*travelSegIter) == itin->travelSeg().back())
    return true;
  else
  {
    bool rtHasStopover = false;
    uint16_t farthest = 0;
    uint16_t maxDistance = 0;

    if (utc::isAySPN1800OptionB(trx, taxSpecConfig()))
    {
      TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
      locIt->setTurnAroundAfterSurface(true);
      locIt->setSkipHidden(true);
      locIt->setSurfaceAsStop(false);

      if (_segmentOrderRoundTrip != 0)
      {
         locIt->toSegmentNo(0);

         if (locIt->nextSeg()->geoTravelType() == GeoTravelType::International)
            locIt->setStopHours(12);
         else
            locIt->setStopHours(4);

         const Loc* start = locIt->loc();
         locIt->toSegmentNo(_segmentOrderRoundTrip);

         while (locIt->hasPrevious())
         {
           locIt->previous();
           if(locIt->isStop() && locIt->hasPrevious() && locIt->isNextSegAirSeg())
             rtHasStopover = true;

           if(locIt->hasPrevious())
           {
             uint16_t distance = TseUtil::greatCircleMiles(*start, *locIt->loc());
             if (distance > maxDistance)
             {
               maxDistance = distance;
               farthest =  locIt->segNo();
               if (!locIt->isNextSegAirSeg())
                 ++farthest;
             }
           }
         }
      }
    }

    std::vector<TravelSeg*>::const_iterator travelSegToIter = travelSegIter;

    for (; (*travelSegToIter) != itin->travelSeg().back(); travelSegEndIndex++)
    {
      travelSegToIter++;
      airSeg = dynamic_cast<const AirSeg*>(*travelSegToIter);

      if (!airSeg)
        continue;

      locMatch = LocUtil::isInLoc(*(*travelSegToIter)->origin(),
                                  taxCodeReg.loc1Type(),
                                  taxCodeReg.loc1(),
                                  Vendor::SABRE,
                                  MANUAL,
                                  LocUtil::TAXES,
                                  GeoTravelType::International,
                                  EMPTY_STRING(),
                                  trx.getRequest()->ticketingDT());

      if (taxCodeReg.itineraryType() == ROUND_TRIP)
      {
        if (locMatch)
        {
          if (carrierValidator.validateCarrier(trx, taxResponse, taxCodeReg, travelSegEndIndex + 1))
            _numberOfFees++;
        }

        if (itin->segmentOrder(*travelSegToIter) != _segmentOrderRoundTrip)
          continue;

        travelSegEndIndex++;
        break;
      }

      if (!locMatch)
        break;

      travelSegIter = travelSegToIter;

      for (; travelSegIter != itin->travelSeg().begin();)
      {
        travelSegIter--;
        airSeg = dynamic_cast<const AirSeg*>(*travelSegIter);

        if (!airSeg)
          continue;

        break;
      }

      if ((*travelSegToIter)->isForcedStopOver())
        break;

      TravelSeg::Application application = TravelSeg::OTHER;

      if ((*travelSegToIter)->geoTravelType() == GeoTravelType::International)
        application = TravelSeg::TAXES;

      if ((*travelSegToIter)
              ->isStopOver((*travelSegIter), (*travelSegIter)->geoTravelType(), application) &&
          !(*travelSegToIter)->isForcedConx())
        break;

      if (utc::isAySPN1800OptionB(trx, taxSpecConfig()))
      {
        TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
        locIt->setTurnAroundAfterSurface(true);
        locIt->setSkipHidden(true);
        locIt->toSegmentNo(itin->segmentOrder(*travelSegToIter) - 1);

        if (locIt->isMirror() || isBetweenMirrors(locIt) || isExtendedMirror(locIt) ||
            (locIt->segNo() == farthest && _segmentOrderRoundTrip != 0 && !rtHasStopover))
          break;
      }

      if (!carrierValidator.validateCarrier(trx, taxResponse, taxCodeReg, travelSegEndIndex + 1))
        continue;

      _numberOfFees++;
    }
  }

  if ((taxCodeReg.maxTax() > 0.0) && (taxCodeReg.taxAmt() > 0.0))
  {
    uint16_t maxNumberOfFees = static_cast<uint16_t>(taxCodeReg.maxTax() / taxCodeReg.taxAmt());

    if (_numberOfFees > maxNumberOfFees)
      _numberOfFees = maxNumberOfFees;
  }

  _travelSegProcessedStartIndex = travelSegStartIndex;
  _travelSegProcessedEndIndex = travelSegEndIndex;

  _travelSegStartIndex = travelSegStartIndex;

  _travelSegEndIndex = travelSegStartIndex;

  travelSegEndIndex = travelSegStartIndex;
  return true;
}

bool
TaxSP1800::isBetweenMirrors(TaxLocIterator* locIt)
{
  uint16_t savedSegNo = locIt->segNo();

  if (locIt->hasPrevious())
  {
    locIt->previous();
    if (locIt->isMirror())
    {
      locIt->next();
      if (locIt->hasNext())
      {
        locIt->next();
        if (locIt->isMirror())
        {
          locIt->toSegmentNo(savedSegNo);
          return true;
        }
      }
    }
  }
  locIt->toSegmentNo(savedSegNo);
  return false;
}

bool
TaxSP1800::isExtendedMirror(TaxLocIterator* locIt)
{
  uint16_t savedSegNo = locIt->segNo();

  if (locIt->hasPrevious() && !locIt->isPrevSegAirSeg())
  {
    locIt->previous();
    if (locIt->hasPrevious())
    {
      locIt->previous();
      const Loc* prevLoc = locIt->loc();
      locIt->next();
      locIt->next();
      if (locIt->hasNext())
      {
        locIt->next();
        if (prevLoc == locIt->locDeplanement())
        {
          locIt->toSegmentNo(savedSegNo);
          return true;
        }
      }
    }
  }
  locIt->toSegmentNo(savedSegNo);
  return false;
}

void
TaxSP1800::adjustTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  Tax::adjustTax(trx, taxResponse, taxCodeReg);
}

void
TaxSP1800::taxCreate(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     uint16_t travelSegStartIndex,
                     uint16_t travelSegEndIndex)
{
  if (_numberOfFees)
  {
    Tax::taxCreate(trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex);
    --_numberOfFees;
  }
}
