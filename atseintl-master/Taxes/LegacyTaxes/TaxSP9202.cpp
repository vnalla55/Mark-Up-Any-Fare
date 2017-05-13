// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
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
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxSP9202.h"
#include "Taxes/Common/TaxUtility.h"
#include "DBAccess/TaxCodeCabin.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "DBAccess/TaxRestrictionTransit.h"
#include "Common/LocUtil.h"
#include "Common/FallbackUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Common/Vendor.h"
#include "Common/TseUtil.h"
#include <vector>
#include <string>

using namespace tse;
using namespace std;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxSP9202::TaxSP9202() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxSP9202::~TaxSP9202() {}

void
TaxSP9202::preparePortionOfTravelIndexes(PricingTrx& trx,
                                         TaxResponse& taxResponse,
                                         TaxCodeReg& taxCodeReg)
{
  _portionOfTravelBeginEndIndexes.clear();

  uint16_t index = 0;
  uint16_t breakIndex = 0;
  uint16_t endIndex = taxResponse.farePath()->itin()->travelSeg().size() - 1;
  std::pair<uint16_t, uint16_t> indexes;

  do
  {
    breakIndex = findJourneyBreak(trx, taxResponse, taxCodeReg, index);
    indexes = make_pair(index, breakIndex);
    _portionOfTravelBeginEndIndexes.push_back(indexes);
    index = breakIndex + 1;

  } while (index <= endIndex);
}

bool
TaxSP9202::applyPortionOfTravel(PricingTrx& trx,
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

  endIndex = partialTravelBeginEndI->second;

  const Itin* itin = taxResponse.farePath()->itin();

  if (!itin->travelSeg()[endIndex]->isAir())
    endIndex--;

  return true;
}

bool
TaxSP9202::validateLocRestrictions(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t& startIndex,
                                   uint16_t& endIndex)
{
  if (!applyPortionOfTravel(trx, taxResponse, taxCodeReg, startIndex, endIndex))
    return false;

  const Itin* itin = taxResponse.farePath()->itin();

  for (; startIndex <= endIndex; startIndex++)
  {
    if (!itin->travelSeg()[startIndex]->isAir())
      continue;

    if (isDomSeg(itin->travelSeg()[startIndex]))
    {
      continue;
    }

    if (validateLoc1(taxResponse.farePath()->itin()->travelSeg()[startIndex]->origin(),
                     taxCodeReg) &&
        validateLoc2(taxResponse.farePath()->itin()->travelSeg()[endIndex]->destination(),
                     taxCodeReg))
    {
      return true;
    }

    break;
  }

  return false;
}

bool
TaxSP9202::validateTripTypes(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex)
{
  return true;
}

bool
TaxSP9202::validateTransit(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t travelSegIndex)
{
  return true;
}

bool
TaxSP9202::setJourneyBreakEndIndex(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t& startIndex,
                                   uint16_t& endIndex)
{
  uint16_t currentIndex = 0;
  uint16_t currentEndIndex = findJourneyBreak(trx, taxResponse, taxCodeReg, currentIndex);

  if (startIndex == 0)
  {
    endIndex = currentEndIndex;
    return true;
  }

  endIndex = taxResponse.farePath()->itin()->travelSeg().size() - 1;

  do
  {
    if (currentEndIndex >= endIndex)
      return false;

    currentIndex = ++currentEndIndex;
    currentEndIndex = findJourneyBreak(trx, taxResponse, taxCodeReg, currentIndex);

  } while (startIndex != currentIndex);

  endIndex = currentEndIndex;

  return true;
}

uint16_t
TaxSP9202::findJourneyBreak(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            uint16_t startIndex)
{
  int64_t transitTotalMinutes = 24 * 60;
  if (!taxCodeReg.restrictionTransit().empty())
  {
    TaxRestrictionTransit& restrictTransit = taxCodeReg.restrictionTransit().front();
    transitTotalMinutes = restrictTransit.transitHours() * 60 + restrictTransit.transitMinutes();
  }

  int64_t connectTimeMin = 0;

  const Itin* itin = taxResponse.farePath()->itin();
  uint16_t breakIndex = itin->travelSeg().size() - 1;
  uint16_t mirrorIndex = findMirrorImage(trx, taxResponse);

  TravelSeg* travelSegFrom;
  TravelSeg* travelSegTo;

  uint16_t indexFrom = startIndex;
  uint16_t indexTo = indexFrom + 1;

  for (; indexTo < itin->travelSeg().size(); indexFrom++, indexTo++)
  {
    travelSegFrom = itin->travelSeg()[indexFrom];

    if (!travelSegFrom->isAir())
      continue;

    travelSegTo = itin->travelSeg()[indexTo];

    if (!travelSegTo->isAir())
    {
      breakIndex = indexTo;
      break;
    }

    if (travelSegFrom->isForcedStopOver())
    {
      breakIndex = indexFrom;
      break;
    }

    if (travelSegFrom->isForcedConx())
      continue;

    if (travelSegFrom->segmentType() == Open && travelSegFrom->isOpenWithoutDate())
    {
      breakIndex = indexFrom;
      break;
    }

    connectTimeMin =
        DateTime::diffTime(travelSegTo->departureDT(), travelSegFrom->arrivalDT()) / 60;

    if ((connectTimeMin > transitTotalMinutes))
    {
      breakIndex = indexFrom;
      break;
    }
  }

  if (mirrorIndex >= startIndex && mirrorIndex < breakIndex)
    breakIndex = mirrorIndex;

  return breakIndex;
}

bool
TaxSP9202::isDomSeg(const TravelSeg* travelSeg) const
{
  return travelSeg->origin()->nation() == travelSeg->destination()->nation();
}

bool
TaxSP9202::validateLoc1(const Loc*& loc, TaxCodeReg& taxCodeReg)
{

  return isGeoMatch(*loc, taxCodeReg.loc1Type(), taxCodeReg.loc1(), taxCodeReg.loc1ExclInd());
}

bool
TaxSP9202::validateLoc2(const Loc*& loc, TaxCodeReg& taxCodeReg)
{
  return isGeoMatch(*loc, taxCodeReg.loc2Type(), taxCodeReg.loc2(), taxCodeReg.loc2ExclInd());
}

bool
TaxSP9202::isGeoMatch(const Loc& aLoc, LocTypeCode& locType, LocCode& loc, Indicator& locExclInd)
{
  if (locType == LOCTYPE_NONE)
    return true;

  bool geoMatch = LocUtil::isInLoc(aLoc, locType, loc, Vendor::SABRE, MANUAL, LocUtil::TAXES);

  return ((geoMatch && locType == LOCTYPE_ZONE) || (geoMatch && locExclInd != 'Y') ||
          (!geoMatch && locExclInd == 'Y'));
}

uint16_t
TaxSP9202::findMirrorImage(PricingTrx& trx, TaxResponse& taxResponse)
{
  const Itin* itin = taxResponse.farePath()->itin();

  uint16_t startIndex = 0;
  uint16_t endIndex = itin->travelSeg().size() - 1;

  if ((itin->travelSeg().size()) % 2 != 0)
    return endIndex;

  TravelSeg* travelSegFront;
  TravelSeg* travelSegBack;

  uint16_t indexFront = startIndex;
  uint16_t indexBack = endIndex;

  for (; indexFront < indexBack; indexFront++, indexBack--)
  {
    travelSegFront = itin->travelSeg()[indexFront];
    travelSegBack = itin->travelSeg()[indexBack];

    if ((travelSegFront->isAir() && !travelSegBack->isAir()) ||
        (!travelSegFront->isAir() && travelSegBack->isAir()))
      return endIndex;

    if (travelSegFront->origin()->nation() != travelSegBack->destination()->nation() ||
        travelSegFront->destination()->nation() != travelSegBack->origin()->nation())
    {
      return endIndex;
    }
  }

  return itin->travelSeg().size() / 2 - 1;
}
