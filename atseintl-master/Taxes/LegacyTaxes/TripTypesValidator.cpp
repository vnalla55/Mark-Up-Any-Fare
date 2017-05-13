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

#include "Taxes/LegacyTaxes/TripTypesValidator.h"

#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
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
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

using namespace tse;

namespace tse
{
FALLBACK_DECL(taxWithinSpecGetLastLoc2);
FALLBACK_DECL(apo44968apo44798XRandTQtaxfix);
FALLBACK_DECL(checkNumberOfSegments);
}

const std::string
TripTypesValidator::TAX_CODE_TS("TS");

bool
TripTypesValidator::validateTrip(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg,
                                 uint16_t& startIndex,
                                 uint16_t& endIndex)
{
  if (!fallback::checkNumberOfSegments(&trx))
  {
    auto restrictNumSegments = utc::numberOfSegments(trx, taxCodeReg);
    if (restrictNumSegments && restrictNumSegments != getTravelSeg(taxResponse).size())
    {
      TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::NO_TAX_ADDED, Diagnostic819);
      return false;
    }
  }
  switch (taxCodeReg.tripType())
  {
  case TAX_FROM_TO:
  {
    return validateFromTo(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  }

  case TAX_BETWEEN:
  {
    return validateBetween(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  }

  case TAX_WITHIN_SPEC:
  {
    return validateWithinSpec(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  }

  case TAX_WITHIN_WHOLLY:
  {
    return validateWithinWholly(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  }

  default:
    return true;
  } // end of switch
} // end of validateTrip

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void TripTypesValidator::validateFromToBetween
//
// Description:  This function will validate TAX_FROM_TO and TAX_BETWEEN
//
// @param  PricingTrx - Transaction object
// @param  TaxCodeReg - TaxCodeReg object
// @param  startIndex - first TrvelSeg to serach from
// @param  endIndex - last TravelSeg to search to
//
// @return bool - true : tax applied;
//                false : tax is not applied;
//
// </PRE>
// ----------------------------------------------------------------------------

bool
TripTypesValidator::validateFromTo(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t& startIndex,
                                   uint16_t& endIndex)

{
  if ((taxCodeReg.loc1Type() == LOCTYPE_NONE) && (taxCodeReg.loc2Type() == LOCTYPE_NONE))
    return true;

  bool locMatch = false;
  std::vector<TravelSeg*>::const_iterator travelSegI =
      getTravelSeg(taxResponse).begin() + startIndex;

  if ((taxCodeReg.loc1Type() == LOCTYPE_NONE) && (taxCodeReg.loc2Type() != LOCTYPE_NONE))
  {
    locMatch = LocUtil::isInLoc(*(*travelSegI)->destination(),
                                taxCodeReg.loc2Type(),
                                taxCodeReg.loc2(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if ((locMatch && taxCodeReg.loc2Type() == LOCTYPE_ZONE) ||
        (locMatch && taxCodeReg.loc2ExclInd() != YES) ||
        (!locMatch && taxCodeReg.loc2ExclInd() == YES))
      return true;

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic819);

    return false;
  }

  bool foundStart = false;

  endIndex = getTravelSeg(taxResponse).size() - 1;

  for (uint16_t index = startIndex; index <= endIndex; index++, travelSegI++)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    locMatch = LocUtil::isInLoc(*airSeg->origin(),
                                taxCodeReg.loc1Type(),
                                taxCodeReg.loc1(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if ((locMatch && taxCodeReg.loc1Type() == LOCTYPE_ZONE) || // can't check ExclInd for Zone
        (locMatch && taxCodeReg.loc1ExclInd() != YES) ||
        (!locMatch && taxCodeReg.loc1ExclInd() == YES))
    {
      startIndex = index;
      endIndex = index;
      foundStart = true;
      break;
    }
  }

  if (foundStart)
  {
    if (taxCodeReg.loc2Type() == LOCTYPE_NONE)
      return true;

    endIndex = findTaxStopOverIndex(trx, taxResponse, taxCodeReg, startIndex);

    travelSegI = getTravelSeg(taxResponse).begin() + endIndex;

    locMatch = LocUtil::isInLoc(*(*travelSegI)->destination(),
                                taxCodeReg.loc2Type(),
                                taxCodeReg.loc2(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if ((locMatch && taxCodeReg.loc2Type() == LOCTYPE_ZONE) ||
        (locMatch && taxCodeReg.loc2ExclInd() != YES) ||
        (!locMatch && taxCodeReg.loc2ExclInd() == YES))
      return true;
  }

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic819);

  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void TripTypesValidator::validateBetween
//
// Description:  This function will validate  TAX_BETWEEN
//
// @param  PricingTrx - Transaction object
// @param  TaxCodeReg - TaxCodeReg object
// @param  startIndex - first TrvelSeg to serach from
// @param  endIndex - last TravelSeg to search to
//
// @return bool - true : tax applied;
//                false : tax is not applied;
//
// </PRE>
// ----------------------------------------------------------------------------

bool
TripTypesValidator::validateBetween(PricingTrx& trx,
                                    TaxResponse& taxResponse,
                                    TaxCodeReg& taxCodeReg,
                                    uint16_t& startIndex,
                                    uint16_t& endIndex)

{
  if ((taxCodeReg.loc1Type() == LOCTYPE_NONE) || (taxCodeReg.loc2Type() == LOCTYPE_NONE))
    return true;

  uint16_t index;
  const AirSeg* airSeg;
  bool locMatch;

  std::vector<TravelSeg*>::const_iterator travelSegIter =
      getTravelSeg(taxResponse).begin() + startIndex;

  std::vector<TravelSeg*>::const_iterator travelSegEndIter = getTravelSeg(taxResponse).end();

  for (index = startIndex; travelSegIter != travelSegEndIter; index++, travelSegIter++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegIter);

    if (!airSeg)
      continue;

    locMatch = LocUtil::isInLoc(*airSeg->origin(),
                                taxCodeReg.loc1Type(),
                                taxCodeReg.loc1(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if ((locMatch && taxCodeReg.loc1Type() == LOCTYPE_ZONE) ||
        (locMatch && taxCodeReg.loc1ExclInd() != YES) ||
        (!locMatch && taxCodeReg.loc1ExclInd() == YES))
      break;
  }

  if (travelSegIter != travelSegEndIter)
  {
    for (; travelSegIter != travelSegEndIter; travelSegIter++)
    {
      airSeg = dynamic_cast<const AirSeg*>(*travelSegIter);

      if (!airSeg)
        continue;

      locMatch = LocUtil::isInLoc(*airSeg->destination(),
                                  taxCodeReg.loc2Type(),
                                  taxCodeReg.loc2(),
                                  Vendor::SABRE,
                                  MANUAL,
                                  LocUtil::TAXES,
                                  GeoTravelType::International,
                                  EMPTY_STRING(),
                                  trx.getRequest()->ticketingDT());

      if ((locMatch && taxCodeReg.loc2Type() == LOCTYPE_ZONE) ||
          (locMatch && taxCodeReg.loc2ExclInd() != YES) ||
          (!locMatch && taxCodeReg.loc2ExclInd() == YES))
      {
        startIndex = index;
        endIndex = index;
        return true;
      }
    }
  }

  travelSegIter = getTravelSeg(taxResponse).begin() + startIndex;

  for (index = startIndex; travelSegIter != travelSegEndIter; index++, travelSegIter++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegIter);

    if (!airSeg)
      continue;

    locMatch = LocUtil::isInLoc(*airSeg->origin(),
                                taxCodeReg.loc2Type(),
                                taxCodeReg.loc2(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if ((locMatch && taxCodeReg.loc2Type() == LOCTYPE_ZONE) ||
        (locMatch && taxCodeReg.loc2ExclInd() != YES) ||
        (!locMatch && taxCodeReg.loc2ExclInd() == YES))
      break;
  }

  if (travelSegIter != travelSegEndIter)
  {
    for (; travelSegIter != travelSegEndIter; travelSegIter++)
    {
      airSeg = dynamic_cast<const AirSeg*>(*travelSegIter);

      if (!airSeg)
        continue;

      locMatch = LocUtil::isInLoc(*airSeg->destination(),
                                  taxCodeReg.loc1Type(),
                                  taxCodeReg.loc1(),
                                  Vendor::SABRE,
                                  MANUAL,
                                  LocUtil::TAXES,
                                  GeoTravelType::International,
                                  EMPTY_STRING(),
                                  trx.getRequest()->ticketingDT());

      if ((locMatch && taxCodeReg.loc1Type() == LOCTYPE_ZONE) ||
          (locMatch && taxCodeReg.loc1ExclInd() != YES) ||
          (!locMatch && taxCodeReg.loc1ExclInd() == YES))
      {
        startIndex = index;
        endIndex = index;
        return true;
      }
    }
  }

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_BETWEEN, Diagnostic819);

  return false; // We fail this tax.
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void TripTypesValidator::validateWithinSpec
//
// Description:  This function will validate TAX_WITHIN_SPEC
//
// @param  PricingTrx - Transaction object
// @param  TaxCodeReg - TaxCodeReg object
// @param  startIndex - first TrvelSeg to serach from
// @param  endIndex - last TravelSeg to search to
//
// @return bool - true : tax applied;
//                false : tax is not applied;
//
// </PRE>
// ----------------------------------------------------------------------------

bool
TripTypesValidator::validateWithinSpec(PricingTrx& trx,
                                       TaxResponse& taxResponse,
                                       TaxCodeReg& taxCodeReg,
                                       uint16_t& startIndex,
                                       uint16_t& endIndex)
//  case TAX_WITHIN_SPEC:
//  {         // Find Loc1 at any BoardPoint in the itinerary -- then find the loc 2 at any offPoint
// after that.
// This assumes that if Loc1 = X and Loc2 = Y and the itinerary does a
//     0   1   2   3   4   5   6   7
//     A - B - X - C - X - Y - D - Y
// default behaviour: start=5, end=5
// with UTC: STARTENDWITHINSPEC=Y: start=2, end=5
// with UTC: STARTENDWITHINSPEC=Y WITHINLATESTLOC2=Y: start=2, end=7
{
  if ((taxCodeReg.loc1Type() == LOCTYPE_NONE) || (taxCodeReg.loc2Type() == LOCTYPE_NONE))
    return false;

  uint16_t index = startIndex;
  const AirSeg* airSeg;
  bool locMatch;

  std::vector<TravelSeg*>::const_iterator travelSegI =
      getTravelSeg(taxResponse).begin() + startIndex;

  for (; travelSegI != getTravelSeg(taxResponse).end(); travelSegI++, index++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    locMatch = LocUtil::isInLoc(*airSeg->origin(),
                                taxCodeReg.loc1Type(),
                                taxCodeReg.loc1(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if ((locMatch && taxCodeReg.loc1Type() == LOCTYPE_ZONE) ||
        (locMatch && taxCodeReg.loc1ExclInd() != YES) ||
        (!locMatch && taxCodeReg.loc1ExclInd() == YES))
      break;
  }

  uint16_t tmpIndex = index;
  bool ret = false;
  bool getLastOccurence = utc::useWithinLastLoc2Logic(trx, taxCodeReg) &&
      !fallback::taxWithinSpecGetLastLoc2(&trx);

  for (; travelSegI != getTravelSeg(taxResponse).end(); travelSegI++, index++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    locMatch = LocUtil::isInLoc(*airSeg->destination(),
                                taxCodeReg.loc2Type(),
                                taxCodeReg.loc2(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if ((locMatch && taxCodeReg.loc2Type() == LOCTYPE_ZONE) ||
        (locMatch && taxCodeReg.loc2ExclInd() != YES) ||
        (!locMatch && taxCodeReg.loc2ExclInd() == YES))
    {
      if (utc::isStartEndWithinSpec(trx, taxCodeReg))
        startIndex = tmpIndex;
      else
        startIndex = index;
      endIndex = index;

      ret = true;
      if (!getLastOccurence)
        break;
    }
  }

  if (!ret)
    TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_WITHIN, Diagnostic818);

  return ret;
} // end TAX_WITHIN_SPEC

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void TripTypesValidator::validateWithinWholly
//
// Description:  This function will validate TAX_WITHIN_WHOLLY
//
// @param  PricingTrx - Transaction object
// @param  TaxCodeReg - TaxCodeReg object
// @param  startIndex - first TrvelSeg to serach from
// @param  endIndex - last TravelSeg to search to
//
// @return bool - true : tax applied;
//                false : tax is not applied;
//
// </PRE>
// ----------------------------------------------------------------------------

bool
TripTypesValidator::validateWithinWholly(PricingTrx& trx,
                                         TaxResponse& taxResponse,
                                         TaxCodeReg& taxCodeReg,
                                         uint16_t& startIndex,
                                         uint16_t& endIndex)
{
  bool locMatch;

  if (UNLIKELY(taxCodeReg.loc1Type() == LOCTYPE_NONE))
    return false;

  LocCode taxLocation = taxCodeReg.loc1();
  LocTypeCode locType = taxCodeReg.loc1Type();

  if ((taxCodeReg.loc1Appl() == TAX_ORIGIN) || (taxCodeReg.loc1Appl() == TAX_ENPLANEMENT))
  {
    taxLocation = taxCodeReg.loc2();
    locType = taxCodeReg.loc2Type();
  }

  const AirSeg* airSeg;

  std::vector<TravelSeg*>::const_iterator travelSegI = getTravelSeg(taxResponse).begin();
  for ( ; travelSegI != getTravelSeg(taxResponse).end(); travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    locMatch = LocUtil::isInLoc(*airSeg->origin(),
                                locType,
                                taxLocation,
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if (!locMatch)
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_WHOLLY, Diagnostic817);

      endIndex = getTravelSeg(taxResponse).size() - 1;
      startIndex = endIndex;
      return false;
    }

    locMatch = LocUtil::isInLoc(*airSeg->destination(),
                                locType,
                                taxLocation,
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if (!locMatch)
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_WHOLLY, Diagnostic817);

      endIndex = getTravelSeg(taxResponse).size() - 1;
      startIndex = endIndex;
      return false;
    }
  }
  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void TripTypesValidator::isTaxStopOver
//
// Description:  This function will validate TAX_WITHIN_WHOLLY
//
// @param  PricingTrx - Transaction object
// @param  TaxCodeReg - TaxCodeReg object
// @param  startIndex - first TrvelSeg to serach from
//
// @return bool of True to fail Tax
//
// </PRE>
// ----------------------------------------------------------------------------

uint16_t
TripTypesValidator::findTaxStopOverIndex(const PricingTrx& trx,
                                         const TaxResponse& taxResponse,
                                         const TaxCodeReg& taxCodeReg,
                                         uint16_t startIndex,
                                         MirrorImage& mirrorImage,
                                         TransitValidator& transitValidator)
{

  if (taxCodeReg.nextstopoverrestr() != YES)
    return startIndex;

  if (taxCodeReg.specialProcessNo() && taxCodeReg.taxCode() != "JP1" &&
      taxCodeReg.taxCode() != "GR")
    return startIndex;

  std::vector<TravelSeg*>::const_iterator travelSegI =
    getTravelSeg(taxResponse).begin() + startIndex;
  uint16_t index = startIndex;

  for (; travelSegI != getTravelSeg(taxResponse).end(); travelSegI++, index++)
  {
    const TravelSeg* travelSeg = *travelSegI;
    if (!travelSeg->isAir())
      return index - 1;

    bool mirror = mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, index);
    bool transitTime = transitValidator.validateTransitTime(trx, taxResponse, taxCodeReg, index);
    bool discardForcedConx;
    if (UNLIKELY(taxCodeReg.taxCode().equalToConst("JP1")))
      discardForcedConx = (taxCodeReg.loc1Appl() == LocRestrictionValidator::TAX_ENPLANEMENT ||
                           (taxCodeReg.loc2Appl() != LocRestrictionValidator::TAX_DEPLANEMENT &&
                            taxCodeReg.loc2Appl() != LocRestrictionValidator::TAX_DESTINATION));
    else
      discardForcedConx = false;

    if (LIKELY(!travelSeg->isForcedConx() || discardForcedConx))
    {
      if (index != startIndex && (mirror || !transitTime))
        return index - 1;
      else if (UNLIKELY(travelSeg->isForcedStopOver()))
        return index;
    }
  }

  return index - 1;
}

uint16_t
TripTypesValidator::findTaxStopOverIndex_new(const PricingTrx& trx,
                                             const TaxResponse& taxResponse,
                                             const TaxCodeReg& taxCodeReg,
                                             uint16_t startIndex,
                                             MirrorImage& mirrorImage,
                                             TransitValidator& transitValidator)
{
  if (taxCodeReg.nextstopoverrestr() != YES)
    return startIndex;

  if (taxCodeReg.specialProcessNo())
    return startIndex;

  std::vector<TravelSeg*>::const_iterator travelSegI =
    getTravelSeg(taxResponse).begin() + startIndex;

  const AirSeg* airSeg;
  uint16_t index = startIndex;
  bool transit;
  bool stopOver = false;
  bool afterStopOver = false;
  bool connection = false;
  bool afterConnection = false;

  for (; travelSegI != getTravelSeg(taxResponse).end(); travelSegI++, index++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
    {
      if (index == 0)
        return 0;
      else
        break;
    }

    if (stopOver)
      afterStopOver = true;

    if (connection)
      afterConnection = true;
    else
      afterConnection = false;

    if ((index != startIndex) && (mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, index)))
      break;

    if ((taxCodeReg.loc1Appl() != LocRestrictionValidator::TAX_ENPLANEMENT) &&
        ((taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DEPLANEMENT) ||
         (taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DESTINATION)))
    {
      if (airSeg->isForcedConx())
        continue;

      if (airSeg->isForcedStopOver())
      {
        if (index != startIndex)
          break;
      }
    }
    else
    {
      if (airSeg->isForcedStopOver())
        stopOver = true;

      if (airSeg->isForcedConx())
      {
        connection = true;
        stopOver = false;
      }
      else
        connection = false;
    }

    if (index == startIndex)
      continue;

    transit = transitValidator.validateTransitTime(trx, taxResponse, taxCodeReg, index);

    if (((!transit) || afterStopOver) && (!afterConnection))
      break;
  }

  index--;

  return findFarthestPointIndex(trx, taxResponse, taxCodeReg, startIndex, index);
}

uint16_t
TripTypesValidator::findTaxStopOverIndex(const PricingTrx& trx,
                                         const TaxResponse& taxResponse,
                                         const TaxCodeReg& taxCodeReg,
                                         uint16_t startIndex)
{
  std::unique_ptr<MirrorImage> mirrorImage;
  if (!fallback::apo44968apo44798XRandTQtaxfix(&trx))
  {
    const auto* taxSpecConfig = !taxCodeReg.specConfigName().empty()
        ? &trx.dataHandle().getTaxSpecConfig(taxCodeReg.specConfigName())
        : nullptr;
    mirrorImage.reset(new MirrorImage(trx.getRequest()->ticketingDT(), taxSpecConfig));
  }
  else
  {
    mirrorImage.reset(new MirrorImage());
  }
  TransitValidator transitValidator;

  transitValidator.setTravelSeg(_travelSeg);
  mirrorImage->setTravelSeg(_travelSeg);

  if (taxCodeReg.taxCode() != TAX_CODE_TS)
    return findTaxStopOverIndex(
        trx, taxResponse, taxCodeReg, startIndex, *mirrorImage, transitValidator);
  else
    return findTaxStopOverIndex_new(
        trx, taxResponse, taxCodeReg, startIndex, *mirrorImage, transitValidator);
}

uint16_t
TripTypesValidator::findFarthestPointIndex(const PricingTrx& trx,
                                           const TaxResponse& taxResponse,
                                           const TaxCodeReg& taxCodeReg,
                                           uint16_t startIndex,
                                           uint16_t stopOverIndex)
{
  if (getTravelSeg(taxResponse)[stopOverIndex]->isForcedStopOver())
    return stopOverIndex;

  uint32_t highMiles = 0;
  uint32_t currentMiles = 0;
  uint16_t farthestPoint = stopOverIndex;

  TravelSeg* startTravelSeg = getTravelSeg(taxResponse)[startIndex];
  TravelSeg* currTravelSeg = startTravelSeg;

  const AirSeg* startFlt = dynamic_cast<AirSeg*>(startTravelSeg);

  for (uint16_t currentIndex = startIndex; currentIndex != stopOverIndex; currentIndex++)
  {
    currTravelSeg = getTravelSeg(taxResponse)[currentIndex];

    if (currTravelSeg->destination()->nation() == startTravelSeg->origin()->nation())
      continue;

    const AirSeg* currFlt = dynamic_cast<AirSeg*>(currTravelSeg);

    if (!currFlt)
      continue;

    currentMiles = ItinUtil::journeyMileage(startFlt,
                                            currFlt,
                                            const_cast<PricingTrx&>(trx),
                                            taxResponse.farePath()->itin()->travelDate());

    if (currentMiles > highMiles)
    {
      highMiles = currentMiles;
      farthestPoint = currentIndex;
    }
  }

  return farthestPoint;
}

const std::vector<TravelSeg*>&
TripTypesValidator::getTravelSeg(const TaxResponse& taxResponse) const
{
  return _travelSeg ? *_travelSeg : taxResponse.farePath()->itin()->travelSeg();
}
