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

#include "Taxes/LegacyTaxes/TaxGB03.h"

#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeCabin.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxRestrictionTransit.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/CabinValidator.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxGB.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"

#include <string>
#include <vector>

namespace tse
{
  FIXEDFALLBACK_DECL(fallbackGBTaxEquipmentException);
}

using namespace tse;
using namespace std;

const uint16_t GREAT_BRITIAN_GB3703 = 3703;
const uint16_t GREAT_BRITIAN_GB3803 = 3803;

TaxGB03::TaxGB03()
  : _nLowCabinTaxCodeSPN(GREAT_BRITIAN_GB3703), _nHighCabinTaxCodeSPN(GREAT_BRITIAN_GB3803)
{
}

void
TaxGB03::preparePortionOfTravelIndexes(PricingTrx& trx,
                                       TaxResponse& taxResponse,
                                       TaxCodeReg& taxCodeReg)
{

  _portionOfTravelBeginEndIndexes.clear();

  uint16_t startIndex = 0;
  uint16_t breakIndex = 0;
  std::pair<uint16_t, uint16_t> indexes;
  uint16_t endIndex = taxResponse.farePath()->itin()->travelSeg().size() - 1;
  const Itin* itin = taxResponse.farePath()->itin();

  breakIndex = findMirror(trx, taxResponse, taxCodeReg, startIndex);
  if (breakIndex < endIndex)
  {

    indexes = make_pair(startIndex, breakIndex);
    _portionOfTravelBeginEndIndexes.push_back(indexes);
    startIndex = breakIndex + 1;
    indexes = make_pair(startIndex, endIndex);
    _portionOfTravelBeginEndIndexes.push_back(indexes);

    return;
  }
  else
  {
    if (isConnNotDomItin(trx, taxResponse, taxCodeReg, startIndex, endIndex))
    {

      TravelSeg* travelSegBegin = itin->travelSeg()[startIndex];
      TravelSeg* travelSegEnd = itin->travelSeg()[endIndex];

      if (travelSegBegin->origin()->nation() != UNITED_KINGDOM &&
          travelSegBegin->origin()->nation() == travelSegEnd->destination()->nation())
      {
        uint16_t breakIndex = findTurnaroundPt(itin, startIndex, endIndex);

        if (itin->travelSeg()[breakIndex]->origin()->nation() == UNITED_KINGDOM ||
            (breakIndex == 0 &&
             itin->travelSeg()[breakIndex]->destination()->nation() == UNITED_KINGDOM))
        {
          if (breakIndex)
            breakIndex--;

          indexes = make_pair(startIndex, breakIndex);
          _portionOfTravelBeginEndIndexes.push_back(indexes);
          startIndex = breakIndex + 1;
        }
      }
    }
  }

  do
  {
    if (!itin->travelSeg()[startIndex]->isAir())
    {
      ++startIndex;
      continue;
    }

    breakIndex = findEndOfConnectedJourney(trx, taxResponse, taxCodeReg, startIndex);

    indexes = make_pair(startIndex, breakIndex);

    _portionOfTravelBeginEndIndexes.push_back(indexes);
    startIndex = breakIndex + 1;

  } while (startIndex <= endIndex);
}

bool
TaxGB03::validateCabin(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex)
{
  return true;
}

bool
TaxGB03::validateLocRestrictions(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg,
                                 uint16_t& startIndex,
                                 uint16_t& endIndex)
{
  std::vector<std::pair<uint16_t, uint16_t> >::const_iterator partialTravelBeginEndI =
      std::find_if(_portionOfTravelBeginEndIndexes.begin(),
                   _portionOfTravelBeginEndIndexes.end(),
                   taxUtil::IsFirstIndex(startIndex));

  if (partialTravelBeginEndI == _portionOfTravelBeginEndIndexes.end())
    return false;

  if (UNLIKELY(partialTravelBeginEndI->first != startIndex))
    return false;

  endIndex = partialTravelBeginEndI->second;

  bool status = false;
  if (validateLoc1(taxResponse.farePath()->itin()->travelSeg()[startIndex]->origin(),
                   taxCodeReg)) // GB originating
  {
    status = onValidLoc1(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  }
  else
  {
    status = onInvalidLoc1(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  }

  return status && validateCabins(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

bool
TaxGB03::validateTripTypes(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t& startIndex,
                           uint16_t& endIndex)
{
  return true;
}

bool
TaxGB03::validateTransit(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex)
{
  return true;
}

uint16_t
TaxGB03::findEndOfConnectedJourney(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t startIndex,
                                   bool skipIntSurface) const
{
  const Itin* itin = taxResponse.farePath()->itin();

  const uint16_t lastIndex = itin->travelSeg().size() - 1;
  uint16_t endIndex = lastIndex;

  if (startIndex >= lastIndex)
    return startIndex;

  uint16_t indexFrom = startIndex;
  uint16_t indexTo = startIndex + 1;

  TravelSeg* travelSegFrom = itin->travelSeg()[indexFrom];
  TravelSeg* travelSegTo = itin->travelSeg()[indexTo];
  TravelSeg* airSegTo;

  bool isUKOrigin = (travelSegFrom->origin()->nation() == UNITED_KINGDOM);
  bool isDepartureFromUK = isUKDeparture(travelSegFrom);

  for (; indexTo <= lastIndex; indexFrom++, indexTo++)
  {
    travelSegFrom = itin->travelSeg()[indexFrom];
    travelSegTo = itin->travelSeg()[indexTo];

    airSegTo = travelSegTo;
    if (!travelSegTo->isAir())
    {
      if (indexTo < lastIndex)
        airSegTo = itin->travelSeg()[indexTo + 1];
      else
      {
        endIndex = indexTo; // last segment
        break;
      }
    }

    bool isDomStopover = (isDomOrUKArival(travelSegFrom)) && isUKSeg(airSegTo);

    if (isDomStopover)
    {
      MirrorImage mirrorImage;

      if (mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, indexTo))
      {
        endIndex = indexFrom;
        break;
      }
    }

    if (isStopOver(travelSegFrom, airSegTo, isDomStopover))
    {
      endIndex = indexFrom;
      break;
    }

    isDepartureFromUK = isDepartureFromUK || isUKDeparture(travelSegFrom);

    if (isDepartureFromUK)
    {
      if (travelSegTo->destination()->nation() == UNITED_KINGDOM)
      {
        if (isUKOrigin)
        {
          endIndex = indexFrom;
        }
        else
        {
          endIndex = indexTo;
        }
        break;
      }
    }
  }

  // international surface and mirror image exception
  travelSegTo = itin->travelSeg()[endIndex];
  if (travelSegTo->isAir() && travelSegTo->destination()->nation() == UNITED_KINGDOM)
  {
    endIndex = findIntSurface(itin, startIndex, endIndex);
  }

  if (isUKOrigin && endIndex < lastIndex)
  {
    travelSegTo = itin->travelSeg()[endIndex + 1];
    if (travelSegTo->isAir() && travelSegTo->destination()->nation() == UNITED_KINGDOM)
    {
      uint16_t intSurfaceIndex = findIntSurface(itin, startIndex, endIndex);

      if (intSurfaceIndex != endIndex)
      {
        endIndex = intSurfaceIndex;
      }
      else
      {
        uint16_t nextIndex = startIndex + 1;
        endIndex = findMirrorImage(trx, itin, nextIndex, endIndex);
      }
    }
  }

  return endIndex;
}

bool
TaxGB03::validateCabins(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        TaxCodeReg& taxCodeReg,
                        uint16_t& startIndex,
                        uint16_t& endIndex) const
{
  const Itin* itin = taxResponse.farePath()->itin();
  std::vector<TravelSeg*>::const_iterator travelSegFromI = itin->travelSeg().begin() + startIndex;
  std::vector<TravelSeg*>::const_iterator travelSegEndI = itin->travelSeg().begin() + endIndex + 1;

  bool validAll = true;
  bool validAtLeastOne = false;

  for (; travelSegFromI != travelSegEndI; ++travelSegFromI)
  {
    if (!(*travelSegFromI)->isAir())
      continue;

    if ((*travelSegFromI)->equipmentType() == BUS || (*travelSegFromI)->equipmentType() == TRAIN)
    {
      //update segment index (diag817)
      if (validAll && !validAtLeastOne && (startIndex < endIndex))
        ++startIndex;
      continue;
    }

    if (CabinValidator().validateCabinRestriction(trx, taxResponse, taxCodeReg, *travelSegFromI))
      validAtLeastOne = true;
    else
      validAll = false;

    if (!validAll && validAtLeastOne) // check only for optimalization
      break;
  }

  if (taxCodeReg.specialProcessNo() == _nHighCabinTaxCodeSPN && (validAll || validAtLeastOne))
    return true;
  else if (taxCodeReg.specialProcessNo() == _nLowCabinTaxCodeSPN && validAll)
    return true;

  return false;
}

bool
TaxGB03::onValidLoc1(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     uint16_t& startIndex,
                     uint16_t& endIndex) const
{
  return validateLoc2(taxResponse.farePath()->itin()->travelSeg()[endIndex]->destination(),
                      taxCodeReg);
}

bool
TaxGB03::onInvalidLoc1(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t& startIndex,
                       uint16_t& endIndex) const
{

  if (startIndex == endIndex)
    return false;

  const Itin* itin = taxResponse.farePath()->itin();

  if (!isRoundTrip(itin, startIndex, endIndex))
    return false;

  // mirror image application
  uint16_t stopOver = findMirrorImage(trx, itin, startIndex, endIndex);

  if (stopOver < endIndex)
  {

    stopOver++;

    if (validateLoc1(taxResponse.farePath()->itin()->travelSeg()[stopOver]->origin(), taxCodeReg))
    {
      startIndex = stopOver;

      return validateLoc2(taxResponse.farePath()->itin()->travelSeg()[endIndex]->destination(),
                          taxCodeReg);
    }
  }
  else
  {
    // turnaround application
    stopOver = findTurnaroundPt(itin, startIndex, endIndex);

    if (stopOver <= endIndex)
    {
      if (validateLoc1(taxResponse.farePath()->itin()->travelSeg()[stopOver]->origin(), taxCodeReg))
      {
        startIndex = stopOver;

        return validateLoc2(taxResponse.farePath()->itin()->travelSeg()[endIndex]->destination(),
                            taxCodeReg);
      }
    }
  }

  startIndex = endIndex;
  return false;
}

uint16_t
TaxGB03::findMirrorImage(PricingTrx& trx,
                         const Itin* itin,
                         const uint16_t& startIndex,
                         const uint16_t& endIndex) const
{

  if ((endIndex - startIndex + 1) % 2 != 0)
    return endIndex;

  TravelSeg* travelSegFront;
  TravelSeg* travelSegBack;

  uint16_t indexFront = startIndex;
  uint16_t indexBack = endIndex;

  bool isUKJournay = isUKJourney(*itin, startIndex, endIndex);

  indexFront = startIndex;
  indexBack = endIndex;

  for (; indexFront < indexBack; indexFront++, indexBack--)
  {
    travelSegFront = itin->travelSeg()[indexFront];
    travelSegBack = itin->travelSeg()[indexBack];

    if ((travelSegFront->isAir() && !travelSegBack->isAir()) ||
        (!travelSegFront->isAir() && travelSegBack->isAir()))
    {
      return endIndex;
    }

    if (!isUKJournay)
    {
      if (travelSegFront->origin()->nation() != travelSegBack->destination()->nation() ||
          travelSegFront->destination()->nation() != travelSegBack->origin()->nation())
      {
        return endIndex;
      }
    }
    else
    {
      bool locMatch = LocUtil::isInLoc(*travelSegFront->destination(),
                                       LOCTYPE_CITY,
                                       travelSegBack->origin()->loc(),
                                       Vendor::SABRE,
                                       MANUAL,
                                       LocUtil::TAXES,
                                       GeoTravelType::International,
                                       EMPTY_STRING(),
                                       trx.getRequest()->ticketingDT());

      if (locMatch)
      {
        locMatch = LocUtil::isInLoc(*travelSegFront->origin(),
                                    LOCTYPE_CITY,
                                    travelSegBack->destination()->loc(),
                                    Vendor::SABRE,
                                    MANUAL,
                                    LocUtil::TAXES,
                                    GeoTravelType::International,
                                    EMPTY_STRING(),
                                    trx.getRequest()->ticketingDT());
      }

      if (!locMatch)
      {
        return endIndex;
      }
    }
  }

  return (endIndex - startIndex + 1) / 2 + (startIndex - 1);
}

uint16_t
TaxGB03::findIntSurface(const Itin* itin, uint16_t& startIndex, uint16_t& endIndex) const
{
  uint16_t index = startIndex;
  TravelSeg* travelSeg;

  for (; index <= endIndex; index++)
  {
    travelSeg = itin->travelSeg()[index];

    if (UNLIKELY(!travelSeg->isAir() && !isDomSeg(travelSeg)))
    {
      if (index) // not first seg
        return --index; // prev seg
    }
  }

  return endIndex;
}

bool
TaxGB03::isRoundTrip(const Itin* itin, uint16_t& startIndex, uint16_t& endIndex) const
{
  return (itin->travelSeg()[startIndex]->origin()->nation() ==
          itin->travelSeg()[endIndex]->destination()->nation());
}

uint16_t
TaxGB03::findTurnaroundPt(const Itin* itin, uint16_t& startIndex, uint16_t& endIndex) const
{
  int16_t turnaroundPt = startIndex;

  const Loc& originBegin = *itin->travelSeg()[startIndex]->origin();
  const Loc& destinationBegin = *itin->travelSeg()[startIndex]->destination();

  uint32_t maxDistance = TseUtil::greatCircleMiles(originBegin, destinationBegin);

  uint16_t index = startIndex;
  TravelSeg* travelSeg;

  for (; index <= endIndex; index++)
  {
    travelSeg = itin->travelSeg()[index];

    if (!travelSeg->isAir())
      continue;

    const Loc& originCurrent = *travelSeg->origin();
    const uint32_t distance = TseUtil::greatCircleMiles(originBegin, originCurrent);

    if (distance > maxDistance)
    {
      maxDistance = distance;
      turnaroundPt = index;
    }
  }

  return turnaroundPt;
}

bool
TaxGB03::validateLoc1(const Loc*& loc, TaxCodeReg& taxCodeReg) const
{
  return isGeoMatch(*loc, taxCodeReg.loc1Type(), taxCodeReg.loc1(), taxCodeReg.loc1ExclInd());
}

bool
TaxGB03::validateLoc2(const Loc*& loc, TaxCodeReg& taxCodeReg) const
{
  return isGeoMatch(*loc, taxCodeReg.loc2Type(), taxCodeReg.loc2(), taxCodeReg.loc2ExclInd());
}

bool
TaxGB03::isGeoMatch(const Loc& aLoc, LocTypeCode& locType, LocCode& loc, Indicator& locExclInd)
    const
{
  if (UNLIKELY(locType == LOCTYPE_NONE))
    return true;

  bool geoMatch = LocUtil::isInLoc(aLoc, locType, loc, Vendor::SABRE, MANUAL, LocUtil::TAXES);

  return ((geoMatch && locType == LOCTYPE_ZONE) || (geoMatch && locExclInd != TaxGB::TAX_EXCLUDE) ||
          (!geoMatch && locExclInd == TaxGB::TAX_EXCLUDE));
}

bool
TaxGB03::isStopOver(TravelSeg* travelSegFrom, TravelSeg* travelSegTo, bool isDomestic) const
{

  if (UNLIKELY(travelSegFrom->isForcedStopOver()))
    return true;

  if (UNLIKELY(travelSegFrom->isForcedConx()))
    return false;

  if (UNLIKELY(travelSegFrom->segmentType() == Open && travelSegFrom->isOpenWithoutDate()))
    return true;

  if (isDomestic)
    return isDomStopOver(travelSegFrom, travelSegTo);
  else
    return isIntStopOver(travelSegFrom, travelSegTo);
}

bool
TaxGB03::isUKJourney(const Itin& itin, uint16_t startIndex, uint16_t endIndex) const
{
  bool isUKJournay = true;

  TravelSeg* travelSeg;
  for (; startIndex <= endIndex; startIndex++)
  {
    travelSeg = itin.travelSeg()[startIndex];

    if (!isUKSeg(travelSeg))
    {
      isUKJournay = false;
      break;
    }
  }

  return isUKJournay;
}

bool
TaxGB03::isIntStopOver(TravelSeg* travelSegFrom, TravelSeg* travelSegTo) const
{
  int64_t transitTotalMinIntl = 1440;
  int64_t connectTimeMin =
      DateTime::diffTime(travelSegTo->departureDT(), travelSegFrom->arrivalDT()) / 60;

  if ((connectTimeMin > transitTotalMinIntl))
    return true;

  return false;
}

bool
TaxGB03::isDomStopOver(TravelSeg* travelSegFrom, TravelSeg* travelSegTo) const
{
  if(fallback::fixed::fallbackGBTaxEquipmentException())
  {
    const std::string equipment("TRN,TGV,ICE");

    if (!(travelSegFrom->equipmentType().empty()) &&
        equipment.find(travelSegFrom->equipmentType()) != std::string::npos)
      return true;
  }
  else if (TaxGB::isSpecialEquipment(travelSegFrom->equipmentType()))
    return true;

  DateTime dtDepart = travelSegTo->departureDT();
  DateTime dtArrival = travelSegFrom->arrivalDT();
  DateTime nextDay = travelSegFrom->arrivalDT().addDays(1);

  int64_t departTime = dtDepart.hours() * 60 + dtDepart.minutes();
  int64_t arrivalTime = dtArrival.hours() * 60 + dtArrival.minutes();

  int64_t transitTotalMinDom = 6 * 60;
  int64_t _5PM = 17 * 60;
  int64_t _10AM = 10 * 60;

  int64_t connectionMin =
      DateTime::diffTime(travelSegTo->departureDT(), travelSegFrom->arrivalDT()) / 60;

  if ((connectionMin > transitTotalMinDom) &&
      !(arrivalTime > _5PM && departTime < _10AM &&
        travelSegTo->departureDT().day() == nextDay.day() &&
        travelSegTo->departureDT().month() == nextDay.month()))
    return true;

  return false;
}

bool
TaxGB03::isDomSeg(TravelSeg* travelSeg) const
{
  return travelSeg->origin()->nation() == travelSeg->destination()->nation();
}

bool
TaxGB03::isUKSeg(TravelSeg* travelSeg) const
{
  return isDomSeg(travelSeg) && travelSeg->origin()->nation() == UNITED_KINGDOM;
}

bool
TaxGB03::isUKDeparture(TravelSeg* travelSeg) const
{
  return travelSeg->origin()->nation() == UNITED_KINGDOM &&
         travelSeg->destination()->nation() != UNITED_KINGDOM;
}

bool
TaxGB03::isDomOrUKArival(TravelSeg* travelSeg) const
{
  return travelSeg->destination()->nation() == UNITED_KINGDOM;
}

uint16_t
TaxGB03::findMirror(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    TaxCodeReg& taxCodeReg,
                    uint16_t startIndex) const
{
  const Itin* itin = taxResponse.farePath()->itin();
  const uint16_t lastIndex = itin->travelSeg().size() - 1;

  uint16_t mirrorIndex = findMirrorImage(trx, itin, startIndex, lastIndex);

  bool isDomesticConn = false;
  bool isDomesticItin = true;

  if (mirrorIndex < lastIndex)
  {

    uint16_t indexFrom = startIndex;
    uint16_t indexTo = startIndex + 1;

    TravelSeg* travelSegFrom = itin->travelSeg()[indexFrom];
    TravelSeg* travelSegTo = itin->travelSeg()[indexTo];

    for (; indexTo <= lastIndex; indexFrom++, indexTo++)
    {
      travelSegFrom = itin->travelSeg()[indexFrom];
      travelSegTo = itin->travelSeg()[indexTo];

      if (UNLIKELY(!travelSegTo->isAir()))
      {
        if (indexTo < lastIndex)
          travelSegTo = itin->travelSeg()[indexTo + 1];
        else
          return lastIndex;
      }

      isDomesticConn = isUKSeg(travelSegFrom) && isUKSeg(travelSegTo);

      if (!isDomesticConn)
        isDomesticItin = false;

      if (isStopOver(travelSegFrom, travelSegTo, isDomesticConn))
        return lastIndex;
    }
  }

  if (isDomesticItin)
    return lastIndex;
  else
    return mirrorIndex;
}

bool
TaxGB03::isConnNotDomItin(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          TaxCodeReg& taxCodeReg,
                          uint16_t& startIndex,
                          uint16_t& endIndex) const
{
  bool isDomesticConn = false;
  bool isDomesticItin = true;

  const Itin* itin = taxResponse.farePath()->itin();

  uint16_t indexFrom = startIndex;
  uint16_t indexTo = startIndex + 1;

  TravelSeg* travelSegFrom = nullptr;
  TravelSeg* travelSegTo = nullptr;

  for (; indexTo <= endIndex; indexFrom++, indexTo++)
  {
    travelSegFrom = itin->travelSeg()[indexFrom];
    travelSegTo = itin->travelSeg()[indexTo];

    if (!travelSegTo->isAir())
    {
      if (indexTo < endIndex)
        travelSegTo = itin->travelSeg()[indexTo + 1];
      else
        return true;
    }

    isDomesticConn = isUKSeg(travelSegFrom) && isUKSeg(travelSegTo);

    if (!isDomesticConn)
      isDomesticItin = false;

    if (isStopOver(travelSegFrom, travelSegTo, isDomesticConn))
      return false;
  }

  return !isDomesticItin;
}
