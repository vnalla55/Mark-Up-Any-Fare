// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
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

#include "Taxes/LegacyTaxes/TaxUS2_01.h"

#include "Common/BSRCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TaxRound.h"
#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/TaxUS2.h"

namespace tse
{
log4cxx::LoggerPtr
TaxUS2_01::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxUS2_01"));

const std::string TaxUS2_01::AIATA_AREA_1 = "1";

/**
returns true when location is outside continental USA, Cananad, Mexico, Hawaii, Alaska
*/
bool
TaxUS2_01::isInternational(const Loc& loc)
{
  if (taxUtil::checkLocCategory(loc) == taxUtil::OTHER && (!LocUtil::isCanada(loc)) && (!LocUtil::isMexico(loc)))
    return true;
  else
    return false;
}

void
TaxUS2_01::calcItinSign(std::string& sign, std::string& signWild, Itin& itin, PricingTrx& trx)
{
  std::string temp;
  std::string prevTemp;
  sign.clear();
  signWild.clear();
  if (taxUtil::soldInUS(trx))
  {
    sign.push_back('X');
    signWild.push_back('X');
  }
  const AirSeg* airSeg;
  std::vector<TravelSeg*>::const_iterator travelSegI = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator travelSegIEnd = itin.travelSeg().end();
  for (; travelSegI != travelSegIEnd; ++travelSegI)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);
    if (!airSeg)
      continue;
    temp.clear();
    addLocChar(temp, *(*travelSegI)->origin(), taxUtil::soldInUS(trx));
    addLocChar(temp, *(*travelSegI)->destination(), taxUtil::soldInUS(trx));
    sign.append(temp);
    if (temp != prevTemp)
    {
      signWild.append(temp);
      prevTemp = temp;
      if (temp[0] == temp[1])
        signWild.push_back('*');
    }
    else
    {
      if (signWild[signWild.size() - 1] != '*')
        signWild.push_back('*');
    }
  }
}

void
TaxUS2_01::addLocChar(std::string& sign, const Loc& loc, bool soldInUS)
{
  switch (taxUtil::checkLocCategory(loc))
  {
  case taxUtil::US:
    sign.push_back('U');
    break;

  case taxUtil::ALASKA:
    sign.push_back('A');
    break;

  case taxUtil::HAWAII:
    sign.push_back('H');
    break;

  default:
    if (soldInUS && taxUtil::isBufferZone(loc))
      sign.push_back('B');
    else
      sign.push_back('O');
    break;
  }
}

/**
returns true if itinerary is eligible for applying US2
*/
bool
TaxUS2_01::validUS2(Itin& itin, bool soldInUS)
{
  const AirSeg* airSeg;

  std::vector<TravelSeg*>::const_iterator travelSegI = itin.travelSeg().begin();
  for (; travelSegI != itin.travelSeg().end(); travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);
    if (!airSeg)
      continue;

    if ((taxUtil::checkLocCategory(*airSeg->origin()) != taxUtil::US) &&
        (!(soldInUS && taxUtil::isBufferZone(*airSeg->origin()))))
      return true;

    if ((taxUtil::checkLocCategory(*airSeg->destination()) != taxUtil::US) &&
        (!(soldInUS && taxUtil::isBufferZone(*airSeg->destination()))))
      return true;
  }
  return false;
}

void
TaxUS2_01::ItinAnalisis::setToOutsideBufferZoneFlag(const Loc& loc)
{
  if (taxUtil::checkLocCategory(loc) != taxUtil::OTHER)
    toOutsideBufferZone = false;
  else if (!taxUtil::isBufferZone(loc))
    toOutsideBufferZone = true;
}

void
TaxUS2_01::ItinAnalisis::setFromOutsideBufferZoneFlag(const Loc& loc)
{
  if (taxUtil::checkLocCategory(loc) != taxUtil::OTHER)
    fromOutsideBufferZone = false;
  else if (!taxUtil::isBufferZone(loc))
    fromOutsideBufferZone = true;
}

void
TaxUS2_01::ItinAnalisis::setWasLocFlags(enum taxUtil::LocCategory locCat)
{
  switch (locCat)
  {
  case taxUtil::US:
    wasUS = true;
    break;
  case taxUtil::HAWAII:
    wasHAWAII = true;
    break;
  case taxUtil::ALASKA:
    wasALASKA = true;
    break;
  default:
    return;
  }
}

void
TaxUS2_01::ItinAnalisis::setWillBeLocFlags(enum taxUtil::LocCategory locCat)
{
  switch (locCat)
  {
  case taxUtil::US:
    willBeUS = true;
    break;
  case taxUtil::HAWAII:
    willBeHAWAII = true;
    break;
  case taxUtil::ALASKA:
    willBeALASKA = true;
    break;
  default:
    return;
  }
}

bool
TaxUS2_01::ItinAnalisis::wasLoc(enum taxUtil::LocCategory locCat)
{
  switch (locCat)
  {
  case taxUtil::US:
    return wasUS;
    break;
  case taxUtil::HAWAII:
    return wasHAWAII;
    break;
  case taxUtil::ALASKA:
    return wasALASKA;
    break;
  default:
    return false;
  }
}

bool
TaxUS2_01::ItinAnalisis::willBeLoc(enum taxUtil::LocCategory locCat)
{
  switch (locCat)
  {
  case taxUtil::US:
    return willBeUS;
    break;
  case taxUtil::HAWAII:
    return willBeHAWAII;
    break;
  case taxUtil::ALASKA:
    return willBeALASKA;
    break;
  default:
    return false;
  }
}

/**
Fills up itinAnalisisVector vector.
Each element of vector relates to a segment in itinerary.
It goes forward then backward through itinerary setting up data in ItinAnalis objects
which are elements in mentioned vector.
Data in each element keeps information about journey to which given segments belongs.
They are: journey's starting and ending locations, flags telling if location categories
exist in journey before or after given segment.
That information is used in validateSegment method to determine if given segment is part
of transit or mirror image type of journey.
Journeys are chunks of itinerary which are confined by points outside continental USA,
Hawaii, Alaska or stopovers.

INPUT:
itin

CHANGES:
itinAnalisisVector
_isInternationalPt
*/
void
TaxUS2_01::analyzeItin(Itin& itin)
{
  ItinAnalisis currValues;
  std::vector<TravelSeg*>::const_iterator travelSegI = itin.travelSeg().begin();
  const Loc* loc;
  TravelSeg* travelSegAdj;
  Loc currLoc;
  int index = 0;
  _isInternationalPt = false;

  for (; travelSegI != itin.travelSeg().end(); ++travelSegI, ++index)
  {
    loc = (*travelSegI)->origin();
    itinAnalisisVector.resize(index + 1);

    AirSeg* airSeg = dynamic_cast<AirSeg*>(*travelSegI);
    if (!airSeg)
      continue;

    if (index != 0)
      travelSegAdj = *(travelSegI - 1);
    else
      travelSegAdj = nullptr;

    if (taxUtil::checkLocCategory(*loc) == taxUtil::OTHER || taxUtil::isStopOver(*travelSegI, travelSegAdj))
    {
      currLoc = *loc;
      currValues.resetWasLocFlags();
    }

    if (isInternational(*loc))
      _isInternationalPt = true;

    currValues.setFromOutsideBufferZoneFlag(*loc);
    currValues.setWasLocFlags(taxUtil::checkLocCategory(*loc));
    itinAnalisisVector[index].copyWasLocFlags(currValues);
    itinAnalisisVector[index].journeyOriginLoc = currLoc;
  }

  do
  {
    if (travelSegI != itin.travelSeg().end())
      travelSegAdj = *travelSegI;
    else
      travelSegAdj = nullptr;

    --travelSegI;
    --index;

    AirSeg* airSeg = dynamic_cast<AirSeg*>(*travelSegI);
    if (!airSeg)
      continue;

    loc = (*travelSegI)->destination();
    if (taxUtil::checkLocCategory(*loc) == taxUtil::OTHER || taxUtil::isStopOver(travelSegAdj, *travelSegI))
    {
      currLoc = *loc;
      currValues.resetWillBeLocFlags();
    }

    if (isInternational(*loc))
      _isInternationalPt = true;

    currValues.setToOutsideBufferZoneFlag(*loc);
    currValues.setWillBeLocFlags(taxUtil::checkLocCategory(*loc));
    itinAnalisisVector[index].copyWillBeLocFlags(currValues);
    itinAnalisisVector[index].journeyDestinationLoc = currLoc;
  } while (travelSegI != itin.travelSeg().begin());
}

bool
TaxUS2_01::validateTripTypes(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex)
{
  Itin& itin = *(taxResponse.farePath()->itin());

  if (!_isItineraryValidated)
  {
    _soldInUS = taxUtil::soldInUS(trx);
    _validUS2 = validUS2(itin, _soldInUS);
    if (_validUS2)
      analyzeItin(itin);
    _isItineraryValidated = true;
  }

  enum TaxDiagnostic::FailCodes diagCode;

  if (_validUS2)
    diagCode = validateSegment(itin, startIndex);
  else
    diagCode = TaxDiagnostic::ITINERARY;

  if (diagCode != TaxDiagnostic::NONE && diagCode != TaxDiagnostic::OTHER)
  {
    TaxDiagnostic::collectErrors(trx, taxCodeReg, taxResponse, diagCode, Diagnostic820);
  }
  if (diagCode == TaxDiagnostic::NONE)
  {
    if (!isMostDistantUS(trx, taxResponse))
    {
      if (!taxUtil::isSurfaceSegmentAFactor(taxResponse, startIndex))
        return false;
    }

    endIndex = startIndex;

    if (taxUtil::doUsTaxesApplyOnYQYR(trx, *(taxResponse.farePath())))
    {
      if (! taxUtil::doesUS2Apply(_travelSegStartIndex, _travelSegStartIndex, trx, taxResponse, taxCodeReg))
        return false;
    }

    return true;
  }
  else
  {
    return false;
  }
} // end of validateTrip

/**
Determines if US2 tax should be charged for given segment.
INPUT:
itin, segIndex, _isInternationalPt, itinAnalisisVector, _soldInUS

CHANGES:
_halfTaxFlag

RETURNS:
TaxDiagnostic::NONE if tax should be applied
*/
enum TaxDiagnostic::FailCodes
TaxUS2_01::validateSegment(Itin& itin, uint16_t segIndex)
{
  _halfTaxFlag = false;

  AirSeg* airSeg = dynamic_cast<AirSeg*>(itin.travelSeg()[segIndex]);
  if (!airSeg)
    return TaxDiagnostic::OTHER;

  enum taxUtil::LocCategory origLocCat = taxUtil::checkLocCategory(*airSeg->origin());
  enum taxUtil::LocCategory destLocCat = taxUtil::checkLocCategory(*airSeg->destination());
  enum taxUtil::LocCategory journeyOrigLocCat =
      taxUtil::checkLocCategory(itinAnalisisVector[segIndex].journeyOriginLoc);
  enum taxUtil::LocCategory journeyDestLocCat =
      taxUtil::checkLocCategory(itinAnalisisVector[segIndex].journeyDestinationLoc);

  _intToUS = false;

  if ((origLocCat == taxUtil::OTHER) && (destLocCat != taxUtil::OTHER))
  {
    _intToUS = true;

    if (journeyDestLocCat == taxUtil::OTHER)
    {
      NationCode origNation = airSeg->origin()->nation();
      IATAAreaCode origArea = airSeg->origin()->area();
      NationCode destNation = itinAnalisisVector[segIndex].journeyDestinationLoc.nation();
      IATAAreaCode destArea = itinAnalisisVector[segIndex].journeyDestinationLoc.area();

      if ((origArea == AIATA_AREA_1 && destNation != origNation) || origArea != destArea)
        return TaxDiagnostic::TRANSIT;
    }

    if ((!_isInternationalPt) && _soldInUS && taxUtil::isBufferZone(*airSeg->origin()))
    {
      if (itinAnalisisVector[segIndex].fromOutsideBufferZone)
      {
        return TaxDiagnostic::NONE;
      }
      else if ((journeyDestLocCat == taxUtil::ALASKA) || (journeyDestLocCat == taxUtil::HAWAII))
      {
        _halfTaxFlag = true;
        return TaxDiagnostic::NONE;
      }
      else
        return TaxDiagnostic::OTHER;
    }

    return TaxDiagnostic::NONE;
  }

  if ((origLocCat != taxUtil::OTHER) && (destLocCat == taxUtil::OTHER))
  {
    if (journeyOrigLocCat == taxUtil::OTHER)
    {
      NationCode destNation = airSeg->destination()->nation();
      IATAAreaCode destArea = airSeg->destination()->area();
      NationCode origNation = itinAnalisisVector[segIndex].journeyOriginLoc.nation();
      IATAAreaCode origArea = itinAnalisisVector[segIndex].journeyOriginLoc.area();

      if ((origArea == AIATA_AREA_1 && destNation != origNation) || origArea != destArea)
        return TaxDiagnostic::TRANSIT;
    }

    if ((!_isInternationalPt) && _soldInUS && taxUtil::isBufferZone(*airSeg->destination()))
    {
      if (itinAnalisisVector[segIndex].toOutsideBufferZone)
      {
        return TaxDiagnostic::NONE;
      }
      else if ((journeyOrigLocCat == taxUtil::ALASKA) || (journeyOrigLocCat == taxUtil::HAWAII))
      {
        _halfTaxFlag = true;
        return TaxDiagnostic::NONE;
      }
      else
      {
        return TaxDiagnostic::OTHER;
      }
    }

    return TaxDiagnostic::NONE;
  }

  if ((journeyOrigLocCat == taxUtil::OTHER) && (journeyDestLocCat == taxUtil::OTHER))
    return TaxDiagnostic::OTHER;

  if (origLocCat != destLocCat)
  {
    _halfTaxFlag = true;

    if (itinAnalisisVector[segIndex].wasLoc(destLocCat))
      return TaxDiagnostic::NONE;

    if (itinAnalisisVector[segIndex].willBeLoc(origLocCat))
      return TaxDiagnostic::NONE;

    if ((destLocCat == journeyDestLocCat) && (journeyOrigLocCat != taxUtil::OTHER))
      return TaxDiagnostic::NONE;

    return TaxDiagnostic::OTHER;
  }

  return TaxDiagnostic::OTHER;
}

void
TaxUS2_01::adjustTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  if (_halfTaxFlag)
  {
    _halfTaxFlag = false;
    _taxAmount = TaxUS2::calculateHalfTaxAmount(trx, taxResponse, _paymentCurrency, taxCodeReg);
  }
}

bool
TaxUS2_01::isMostDistantUS(PricingTrx& trx, TaxResponse& taxResponse)
{
  if (!_mostDistantUSInitialized)
  {
    _mostDistantUS = taxUtil::isMostDistantUS(trx, taxResponse);
    _mostDistantUSInitialized = true;
  }
  return _mostDistantUS;
}
}
