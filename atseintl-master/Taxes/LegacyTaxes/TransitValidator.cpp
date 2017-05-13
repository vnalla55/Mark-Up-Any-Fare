// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include "Taxes/LegacyTaxes/TransitValidator.h"

#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/TypeConvert.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

using namespace tse;

namespace tse
{
FALLBACK_DECL(taxDisableTransitViaLocLogic);
FALLBACK_DECL(apo44470multiCityIsStop);
FALLBACK_DECL(treatTrsAsPlane);
}

bool
TransitValidator::validateTransitRestriction(PricingTrx& trx,
                                             TaxResponse& taxResponse,
                                             TaxCodeReg& taxCodeReg,
                                             uint16_t startIndex,
                                             bool mirrorImage,
                                             bool landToAirStopover)
{
  if (taxCodeReg.restrictionTransit().empty())
    return true;

  int32_t transitHours = 0;
  int32_t transitMinutes = 0;

  int64_t connectionMinutes = 0;
  int64_t transitTotalMinutes = 0;
  int64_t workTime = 0;

  const AirSeg* airSeg;
  const AirSeg* airSegTo;

  TaxRestrictionTransit& restrictTransit = taxCodeReg.restrictionTransit().front();

  if (!restrictTransit.viaLoc().empty() &&
    (!utc::isTransitViaLocDisabled(trx, taxCodeReg) || fallback::taxDisableTransitViaLocLogic(&trx)))
  {
    TravelSeg* travelSeg = getTravelSeg(taxResponse)[startIndex];

    airSeg = dynamic_cast<const AirSeg*>(travelSeg);

    if (!airSeg)
      return false;

    if (travelSeg == getTravelSeg(taxResponse).front())
      return false;

    bool locMatch = LocUtil::isInLoc(*travelSeg->origin(),
                                     restrictTransit.viaLocType(),
                                     restrictTransit.viaLoc(),
                                     Vendor::SABRE,
                                     MANUAL,
                                     LocUtil::TAXES,
                                     GeoTravelType::International,
                                     EMPTY_STRING(),
                                     trx.getRequest()->ticketingDT());

    if (!locMatch)
      return false;

    locMatch = LocUtil::isInLoc(*travelSeg->destination(),
                                taxCodeReg.loc2Type(),
                                taxCodeReg.loc2(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if (!locMatch)
      return false;

    travelSeg = getTravelSeg(taxResponse)[startIndex - 1];

    airSeg = dynamic_cast<const AirSeg*>(travelSeg);

    if (!airSeg)
      return false;

    locMatch = LocUtil::isInLoc(*travelSeg->origin(),
                                taxCodeReg.loc1Type(),
                                taxCodeReg.loc1(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if (!locMatch)
      return false;
  }

  transitHours = restrictTransit.transitHours();
  transitMinutes = restrictTransit.transitMinutes();
  bool openSegment = false;

  if ((transitHours >= 0) && (transitMinutes < 0))
    transitMinutes = 0;

  if (UNLIKELY((transitHours < 0) && (transitMinutes >= 0)))
    transitHours = 0;

  bool transitTravel = false;
  if (mirrorImage && utc::isMirrorStopLogicDisabled(trx, taxCodeReg))
  {
    mirrorImage = false;
  }

  do
  {
    if (mirrorImage)
      break;

    TravelSeg* travelSegTo = getTravelSeg(taxResponse)[startIndex];
    TravelSeg* travelSegFrom = getTravelSeg(taxResponse)[startIndex];

    if ((taxCodeReg.loc1Appl() != LocRestrictionValidator::TAX_ENPLANEMENT) &&
        ((taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DEPLANEMENT) ||
         (taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DESTINATION)))
    {
      if (travelSegFrom == getTravelSeg(taxResponse).back())
        break;

      travelSegTo = getTravelSeg(taxResponse)[startIndex + 1];

      airSeg = dynamic_cast<const AirSeg*>(travelSegTo);

      if (!airSeg)
      {
        if (startIndex+2 >= static_cast<int>(getTravelSeg(taxResponse).size()) ||
            !utc::transitValidatorSkipArunk(trx, taxCodeReg) )
          break;

        //second chance
        travelSegTo = getTravelSeg(taxResponse)[startIndex + 2];
        airSeg = dynamic_cast<const AirSeg*>(travelSegTo);
        if (!airSeg)
          break;
      }

      if (travelSegFrom->segmentType() == Open)
      {
        openSegment = true;

        if (travelSegFrom->isOpenWithoutDate())
          break;
      }
    }
    else
    {
      if (travelSegTo == getTravelSeg(taxResponse).front())
        break;

      travelSegFrom = getTravelSeg(taxResponse)[startIndex - 1];

      airSeg = dynamic_cast<const AirSeg*>(travelSegFrom);

      if (!airSeg)
      {
        if (startIndex < 2 || !utc::transitValidatorSkipArunk(trx, taxCodeReg) )
          break;

        //second chance
        travelSegFrom = getTravelSeg(taxResponse)[startIndex - 2];
        airSeg = dynamic_cast<const AirSeg*>(travelSegFrom);
        if (!airSeg)
          break;
      }

      if (UNLIKELY(travelSegTo->segmentType() == Open))
      {
        openSegment = true;

        if (travelSegTo->isOpenWithoutDate())
          break;
      }
    }

    if (!fallback::apo44470multiCityIsStop(&trx))
    {
      if (travelSegFrom && travelSegTo && travelSegFrom != travelSegTo &&
          utc::transitValidatorMultiCityIsStop(trx, taxCodeReg))
      {
        if (travelSegFrom->destAirport() != travelSegTo->origAirport() &&
            travelSegFrom->offMultiCity() == travelSegTo->boardMultiCity())
          return (restrictTransit.transitTaxonly() != YES);
      }
    }

    if (!fallback::treatTrsAsPlane(&trx))
    {
      if (landToAirStopover &&
          ((airSeg->equipmentType() == TRAIN) || (airSeg->equipmentType() == TGV) ||
           (airSeg->equipmentType() == ICE) || (airSeg->equipmentType() == BOAT) ||
           (airSeg->equipmentType() == LMO) || (airSeg->equipmentType() == BUS) ||
           (airSeg->equipmentType() == TRS)))
        break;
    }
    else
    {
      if (landToAirStopover &&
          ((airSeg->equipmentType() == TRAIN) || (airSeg->equipmentType() == TGV) ||
           (airSeg->equipmentType() == ICE) || (airSeg->equipmentType() == BOAT) ||
           (airSeg->equipmentType() == LMO) || (airSeg->equipmentType() == BUS)))
        break;
    }

    // Furthest Point and Round Trip cannot transit..
    // Furthest Point cannot be termination point..
    /*
          if (travelSegTo->furthestPoint())
          {
            if (travelSegTo->origin()->nation() == travelSegTo->destination()->nation())
            {
              if (travelSegFrom->origin()->nation() == travelSegTo->destination()->nation())
                 return true;
            }
            if (travelSegTo != taxResponse.farePath()->itin()->travelSeg().back())
               break;
          }
    */
    if (restrictTransit.sameFlight() == YES)
    {
      if ((travelSegFrom->segmentType() != Open) || (travelSegTo->segmentType() != Open))
      {
        airSeg = dynamic_cast<const AirSeg*>(travelSegFrom);
        airSegTo = dynamic_cast<const AirSeg*>(travelSegTo);

        if (airSeg && airSegTo &&
            airSeg->marketingFlightNumber() == airSegTo->marketingFlightNumber())
        {
          if (airSegTo->isStopOver(airSeg, airSeg->geoTravelType()) && !airSeg->isForcedConx())
            return true;

          TaxDiagnostic::collectErrors(
              trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic813);

          return false;
        }

        if ((transitHours <= 0) && (transitMinutes <= 0))
          return true;
      }
    }

    if (UNLIKELY(travelSegFrom->isForcedStopOver()))
      break;

    if (UNLIKELY(travelSegFrom->isForcedConx()))
    {
      transitTravel = true;
      break;
    }

    if (restrictTransit.sameDayInd() == YES || openSegment)
    {
      if (UNLIKELY((travelSegFrom->segmentType() == Open) || (travelSegTo->segmentType() == Open)))
      {
        if ((transitHours <= 0) && (transitMinutes <= 0))
          return true;
      }
      transitTravel = true;

      if (travelSegTo->departureDT().day() != travelSegFrom->arrivalDT().day() ||
          travelSegTo->departureDT().month() != travelSegFrom->arrivalDT().month())
        transitTravel = false;

      break;
    }

    if (UNLIKELY((transitHours <= 0) && (transitMinutes <= 0)))
      break;

    workTime = DateTime::diffTime(travelSegTo->departureDT(), travelSegFrom->arrivalDT());

    connectionMinutes = workTime / 60;
    transitTotalMinutes = (transitHours * 60) + transitMinutes; // lint !e647

    if (connectionMinutes > transitTotalMinutes)
      break;

    transitTravel = true;

  } while (!transitTravel);

  if (((restrictTransit.transitTaxonly() != YES) && (!transitTravel)) ||
      ((restrictTransit.transitTaxonly() == YES) && (transitTravel)))
  {
    if (!transitTravel)
      return true;

    if ((restrictTransit.transitDomDom() == YES) || (restrictTransit.transitDomIntl() == YES) ||
        (restrictTransit.transitIntlDom() == YES) || (restrictTransit.transitIntlIntl() == YES) ||
        (restrictTransit.transitSurfDom() == YES) || (restrictTransit.transitSurfIntl() == YES) ||
        (restrictTransit.transitOfflineCxr() == YES))
    {
      if (!validateTransitIndicators(trx, taxResponse, taxCodeReg, restrictTransit, startIndex))
      {
        TaxDiagnostic::collectErrors(
            trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION_IND, Diagnostic813);

        return false;
      }
    }
    return true;
  }

  if (transitTravel)
  {
    if ((restrictTransit.transitDomDom() == YES) || (restrictTransit.transitDomIntl() == YES) ||
        (restrictTransit.transitIntlDom() == YES) || (restrictTransit.transitIntlIntl() == YES) ||
        (restrictTransit.transitSurfDom() == YES) || (restrictTransit.transitSurfIntl() == YES) ||
        (restrictTransit.transitOfflineCxr() == YES))
    {
      if (validateTransitIndicators(trx, taxResponse, taxCodeReg, restrictTransit, startIndex))
        return true;
    }
  }

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic813);

  return false;
}

// ----------------------------------------------------------------------------
// Description:  validateTransitRestriction
// ----------------------------------------------------------------------------

bool
TransitValidator::validateTransitRestriction(PricingTrx& trx,
                                             TaxResponse& taxResponse,
                                             TaxCodeReg& taxCodeReg,
                                             uint16_t startIndex)
{
  if (taxCodeReg.restrictionTransit().empty())
    return true;

  int32_t transitHours = 0;
  int32_t transitMinutes = 0;

  int64_t connectionMinutes = 0;
  int64_t transitTotalMinutes = 0;
  int64_t workTime = 0;

  bool transitTravel = false;
  bool openSegment = false;
  const AirSeg* airSeg;
  const AirSeg* airSegTo;

  TaxRestrictionTransit& restrictTransit = taxCodeReg.restrictionTransit().front();

  transitHours = restrictTransit.transitHours();
  transitMinutes = restrictTransit.transitMinutes();

  if ((transitHours >= 0) && (transitMinutes < 0))
    transitMinutes = 0;

  if ((transitHours < 0) && (transitMinutes >= 0))
    transitHours = 0;

  do
  {
    TravelSeg* travelSegTo = getTravelSeg(taxResponse)[startIndex];
    TravelSeg* travelSegFrom = getTravelSeg(taxResponse)[startIndex];

    if ((taxCodeReg.loc1Appl() != LocRestrictionValidator::TAX_ENPLANEMENT) &&
        ((taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DEPLANEMENT) ||
         (taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DESTINATION)))
    {
      if (travelSegFrom == getTravelSeg(taxResponse).back())
        break;

      travelSegTo = getTravelSeg(taxResponse)[startIndex + 1];

      airSeg = dynamic_cast<const AirSeg*>(travelSegTo);

      if (!airSeg)
        break;

      if (travelSegFrom->segmentType() == Open)
      {
        openSegment = true;

        if (travelSegFrom->isOpenWithoutDate())
          break;
      }
    }
    else
    {
      if (travelSegTo == getTravelSeg(taxResponse).front())
        break;

      travelSegFrom = getTravelSeg(taxResponse)[startIndex - 1];

      airSeg = dynamic_cast<const AirSeg*>(travelSegFrom);

      if (!airSeg)
        break;

      if (travelSegTo->segmentType() == Open)
      {
        openSegment = true;

        if (travelSegTo->isOpenWithoutDate())
          break;
      }
    }

    if (restrictTransit.sameFlight() == YES)
    {
      if ((travelSegFrom->segmentType() != Open) || (travelSegTo->segmentType() != Open))
      {
        airSeg = dynamic_cast<const AirSeg*>(travelSegFrom);
        airSegTo = dynamic_cast<const AirSeg*>(travelSegTo);

        if (airSeg && airSegTo &&
            airSeg->marketingFlightNumber() == airSegTo->marketingFlightNumber())
        {
          if (airSegTo->isStopOver(airSeg, airSeg->geoTravelType()) && !airSeg->isForcedConx())
            return true;

          TaxDiagnostic::collectErrors(
              trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic813);

          return false;
        }

        if ((transitHours <= 0) && (transitMinutes <= 0))
          return true;
      }
    }

    if (restrictTransit.sameDayInd() == YES || openSegment)
    {
      if ((travelSegFrom->segmentType() == Open) || (travelSegTo->segmentType() == Open))
      {
        if ((transitHours <= 0) && (transitMinutes <= 0))
          return true;
      }
      transitTravel = true;

      if (travelSegTo->departureDT().day() != travelSegFrom->arrivalDT().day() ||
          travelSegTo->departureDT().month() != travelSegFrom->arrivalDT().month())
        transitTravel = false;

      break;
    }

    if ((transitHours <= 0) && (transitMinutes <= 0))
      break;

    workTime = DateTime::diffTime(travelSegTo->departureDT(), travelSegFrom->arrivalDT());

    connectionMinutes = workTime / 60;
    transitTotalMinutes = (transitHours * 60) + transitMinutes; // lint !e647

    if (connectionMinutes > transitTotalMinutes)
      break;

    transitTravel = true;

  } while (!transitTravel);

  if (((restrictTransit.transitTaxonly() != YES) && (!transitTravel)) ||
      ((restrictTransit.transitTaxonly() == YES) && (transitTravel)))
  {
    if ((restrictTransit.transitDomDom() == YES) || (restrictTransit.transitDomIntl() == YES) ||
        (restrictTransit.transitIntlDom() == YES) || (restrictTransit.transitIntlIntl() == YES) ||
        (restrictTransit.transitSurfDom() == YES) || (restrictTransit.transitSurfIntl() == YES) ||
        (restrictTransit.transitOfflineCxr() == YES))
    {
      if (!validateTransitIndicators(trx, taxResponse, taxCodeReg, restrictTransit, startIndex))
      {
        TaxDiagnostic::collectErrors(
            trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION_IND, Diagnostic813);

        return false;
      }
    }
    return true;
  }

  if (transitTravel)
  {
    if ((restrictTransit.transitDomDom() == YES) || (restrictTransit.transitDomIntl() == YES) ||
        (restrictTransit.transitIntlDom() == YES) || (restrictTransit.transitIntlIntl() == YES) ||
        (restrictTransit.transitSurfDom() == YES) || (restrictTransit.transitSurfIntl() == YES) ||
        (restrictTransit.transitOfflineCxr() == YES))
    {
      if (validateTransitIndicators(trx, taxResponse, taxCodeReg, restrictTransit, startIndex))
        return true;
    }
  }

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic813);

  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool validateTransitTime::validateTransitTime
//
// Description:  This function will check if the before segment transit time
//               matches the tax conditions. It considers transit time (set in
//               the tax). It can consider also open segments, depending on the
//               parameter.
//
// @param  PricingTrx - Transaction object, not used // TODO: get rid of.
// @param  TaxResponse - Tax response to send, contains all segments data.
// @param  TaxCodeReg - Registered tax object, contains all tax conditions.
// @param  startIndex - Considered segment's index.
// @param  shouldCheckOpen - True, iff validator should consider open segments.
//
// @return bool - true: if transit time doesn't exceed the tax transit time
//                      or the segment is open with the same date.
//                false: if transit time exceeds the tax transit time,
//                       there is no valid segment before
//                       or the segment is open with the different date.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TransitValidator::validateTransitTime(const PricingTrx& trx,
                                      const TaxResponse& taxResponse,
                                      const TaxCodeReg& taxCodeReg,
                                      uint16_t startIndex,
                                      bool shouldCheckOpen)
{
  if (taxCodeReg.restrictionTransit().empty())
    return false;

  int32_t transitHours = 0;
  int32_t transitMinutes = 0;

  int64_t connectionMinutes = 0;
  int64_t transitTotalMinutes = 0;
  int64_t workTime = 0;

  const TaxRestrictionTransit& restrictTransit = taxCodeReg.restrictionTransit().front();

  transitHours = restrictTransit.transitHours();
  transitMinutes = restrictTransit.transitMinutes();

  if ((transitHours >= 0) && (transitMinutes < 0))
    transitMinutes = 0;

  if (UNLIKELY((transitHours < 0) && (transitMinutes >= 0)))
    transitHours = 0;

  TravelSeg* travelSegTo = getTravelSeg(taxResponse)[startIndex];
  TravelSeg* travelSegFrom = getTravelSeg(taxResponse)[startIndex];

  bool openSegment = false;
  if (UNLIKELY((taxCodeReg.loc1Appl() != LocRestrictionValidator::TAX_ENPLANEMENT) &&
      ((taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DEPLANEMENT) ||
       (taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DESTINATION))))
  {
    if (travelSegFrom == getTravelSeg(taxResponse).back())
      return false;

    travelSegTo = getTravelSeg(taxResponse)[startIndex + 1];
    if (!travelSegTo->isAir())
      return false;

    if (shouldCheckOpen && travelSegFrom->segmentType() == Open)
    {
      openSegment = true;

      if (travelSegFrom->isOpenWithoutDate())
        return false;
    }
  }
  else
  {
    if (travelSegTo == getTravelSeg(taxResponse).front())
      return false;

    travelSegFrom = getTravelSeg(taxResponse)[startIndex - 1];

    if (!travelSegFrom->isAir())
      return false;

    if (UNLIKELY(shouldCheckOpen && travelSegTo->segmentType() == Open))
    {
      openSegment = true;

      if (travelSegTo->isOpenWithoutDate())
        return false;
    }
  }

  if (UNLIKELY(restrictTransit.sameDayInd() == YES || openSegment))
  {
    if (travelSegTo->departureDT().day() != travelSegFrom->arrivalDT().day() ||
        travelSegTo->departureDT().month() != travelSegFrom->arrivalDT().month())
      return false;

    return true;
  }

  if ((transitHours <= 0) && (transitMinutes <= 0))
    return false;

  workTime = DateTime::diffTime(travelSegTo->departureDT(), travelSegFrom->arrivalDT());

  connectionMinutes = workTime / 60;
  transitTotalMinutes = (transitHours * 60) + transitMinutes; // lint !e647

  if (connectionMinutes > transitTotalMinutes)
    return false;

  return true;
}

// ----------------------------------------------------------------------------
// Description:  validateTransitIndicators
// ----------------------------------------------------------------------------

bool
TransitValidator::validateTransitIndicators(PricingTrx& trx,
                                            TaxResponse& taxResponse,
                                            TaxCodeReg& taxCodeReg,
                                            TaxRestrictionTransit& restrictTransit,
                                            uint16_t startIndex)
{
  TravelSeg* travelSegTo = getTravelSeg(taxResponse)[startIndex];
  TravelSeg* travelSegFrom = getTravelSeg(taxResponse)[startIndex];

  if ((taxCodeReg.loc1Appl() != LocRestrictionValidator::TAX_ENPLANEMENT) &&
      ((taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DEPLANEMENT) ||
       (taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DESTINATION)))
  {
    if (travelSegFrom == getTravelSeg(taxResponse).back())
      return true;

    travelSegTo = getTravelSeg(taxResponse)[startIndex + 1];
  }
  else
  {
    if (UNLIKELY(travelSegTo == getTravelSeg(taxResponse).front()))
      return true;

    travelSegFrom = getTravelSeg(taxResponse)[startIndex - 1];
  }

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSegFrom);
  const AirSeg* airSegCurrent = dynamic_cast<const AirSeg*>(travelSegTo);
  const Loc* originLocation = travelSegFrom->origin();
  const Loc* originConnection = travelSegTo->origin();
  const Loc* destination = travelSegTo->destination();

  if (restrictTransit.transitDomDom() == YES)
  {
    if (airSeg && airSegCurrent && originLocation->nation() == originConnection->nation() &&
        originConnection->nation() == destination->nation())
      return true;
  }

  if (restrictTransit.transitDomIntl() == YES)
  {
    if (airSeg && airSegCurrent && originLocation->nation() == originConnection->nation() &&
        originConnection->nation() != destination->nation())
      return true;
    else if (validTransitIndicatorsIfMultiCity(travelSegTo, travelSegFrom))
      return true;
  }

  if (restrictTransit.transitIntlDom() == YES)
  {
    if (airSeg && airSegCurrent && originLocation->nation() != originConnection->nation() &&
        originConnection->nation() == destination->nation())
      return true;
  }

  if (restrictTransit.transitIntlIntl() == YES)
  {
    if (airSeg && airSegCurrent && originLocation->nation() != originConnection->nation() &&
        originConnection->nation() != destination->nation())
      return true;
  }

  if (restrictTransit.transitSurfDom() == YES)
  {
    if (!airSeg && airSegCurrent && originLocation->nation() == destination->nation())
      return true;
  }

  if (restrictTransit.transitSurfIntl() == YES)
  {
    if (!airSeg && airSegCurrent && originLocation->nation() != destination->nation())
      return true;
  }

  if (restrictTransit.transitOfflineCxr() == YES)
  {
    if (airSeg && airSegCurrent && airSeg->carrier() != airSegCurrent->carrier())
      return true;
  }

  return false;
}

bool
TransitValidator::validTransitIndicatorsIfMultiCity(TravelSeg* travelSegTo,
                                                    TravelSeg* travelSegFrom)
{
  return false;
}

const std::vector<TravelSeg*>&
TransitValidator::getTravelSeg(const TaxResponse& taxResponse) const
{
  return _travelSeg ? *_travelSeg : taxResponse.farePath()->itin()->travelSeg();
}
