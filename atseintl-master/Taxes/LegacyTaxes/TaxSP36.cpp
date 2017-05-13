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

#include "Taxes/LegacyTaxes/TaxSP36.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "DBAccess/TaxCodeCabin.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

#include "Taxes/LegacyTaxes/TripTypesValidator.h"
#include "Taxes/LegacyTaxes/FareClassValidator.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "DBAccess/TaxRestrictionTransit.h"
#include "Taxes/LegacyTaxes/CabinValidator.h"
#include "Taxes/LegacyTaxes/EquipmentValidator.h"

#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "Common/LocUtil.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Common/Vendor.h"
#include "DBAccess/DataHandle.h"
#include "Common/TrxUtil.h"

using namespace tse;
using namespace std;

const string
TaxSP36::EVERY_CLASSES("*");

bool
TaxSP36::validateCabin(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex)
{
  return true;
} // end of validateCabin

bool
TaxSP36::validateTripTypes(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t& startIndex,
                           uint16_t& endIndex)
{
  if (strncmp(taxCodeReg.taxCode().c_str(), "RG", 2) == 0)
  {
    TripTypesValidator tripTypesValidator;

    return tripTypesValidator.validateTrip(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  }

  return validateFromTo(trx, taxResponse, taxCodeReg, startIndex, endIndex);
} // end of validateTripType

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
TaxSP36::validateFromTo(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        TaxCodeReg& taxCodeReg,
                        uint16_t& startIndex,
                        uint16_t& endIndex)
{
  if (UNLIKELY((taxCodeReg.loc1Type() == LOCTYPE_NONE) && (taxCodeReg.loc2Type() == LOCTYPE_NONE)))
    return true;

  bool locMatch;
  std::vector<TravelSeg*>::const_iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin() + startIndex;

  if (UNLIKELY((taxCodeReg.loc1Type() == LOCTYPE_NONE) && (taxCodeReg.loc2Type() != LOCTYPE_NONE)))
  {
    locMatch = LocUtil::isInLoc(*(*travelSegI)->destination(),
                                taxCodeReg.loc2Type(),
                                taxCodeReg.loc2(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES);

    if ((locMatch && taxCodeReg.loc2Type() == LOCTYPE_ZONE) ||
        (locMatch && taxCodeReg.loc2ExclInd() != YES) ||
        (!locMatch && taxCodeReg.loc2ExclInd() == YES))
      return true;

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic819);

    return false;
  }

  bool foundStart = false;
  const AirSeg* airSeg;

  endIndex = taxResponse.farePath()->itin()->travelSeg().size() - 1;

  for (uint16_t index = startIndex; index <= endIndex; index++, travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (UNLIKELY(!airSeg))
      continue;

    locMatch = LocUtil::isInLoc(*airSeg->origin(),
                                taxCodeReg.loc1Type(),
                                taxCodeReg.loc1(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES);

    if (LIKELY((locMatch && taxCodeReg.loc1Type() == LOCTYPE_ZONE) || // can't check ExclInd for Zone
        (locMatch && taxCodeReg.loc1ExclInd() != YES) ||
        (!locMatch && taxCodeReg.loc1ExclInd() == YES)))
    {
      startIndex = index;
      endIndex = index;
      foundStart = true;
      break;
    }
  }

  if (LIKELY(foundStart))
  {
    if (UNLIKELY(taxCodeReg.loc2Type() == LOCTYPE_NONE))
      return true;

    endIndex = findTaxStopOverIndex(trx, taxResponse, taxCodeReg, startIndex);

    // vector size check

    if (UNLIKELY((endIndex > (taxResponse.farePath()->itin()->travelSeg().size() - 1)) ||
        (taxResponse.farePath()->itin()->travelSeg().empty())))

    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::BAD_DATA_HANDLER_POINTER, Diagnostic819);
      return false;
    }

    travelSegI = taxResponse.farePath()->itin()->travelSeg().begin() + endIndex;

    locMatch = LocUtil::isInLoc(*(*travelSegI)->destination(),
                                taxCodeReg.loc2Type(),
                                taxCodeReg.loc2(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES);

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
TaxSP36::findTaxStopOverIndex(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t startIndex)
{
  if (UNLIKELY(taxCodeReg.nextstopoverrestr() != YES))
    return startIndex;

  std::vector<TravelSeg*>::const_iterator travelSegI;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin() + startIndex;

  MirrorImage mirrorImage;
  TransitValidator transitValidator;

  const AirSeg* airSeg;
  uint16_t index = startIndex;
  bool transit;

  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++, index++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      break;

    if (UNLIKELY(airSeg->isForcedConx()))
      continue;

    if (UNLIKELY(airSeg->isForcedStopOver()))
      return index;

    if (index == startIndex)
      continue;

    if (mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, index))
      break;

    transit = transitValidator.validateTransitTime(trx, taxResponse, taxCodeReg, index);

    if (!transit)
      break;
  }

  index--;

  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin() + index;

  airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

  if (UNLIKELY(!airSeg))
    return index;

  if (UNLIKELY(airSeg->isForcedConx()))
    index++;

  return index;
}

// ---------------------------------------
// Description:  TaxSP36::validateTransit
// ---------------------------------------
bool
TaxSP36::validateTransit(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex)
{
  const Itin* itin = taxResponse.farePath()->itin();

  std::vector<TravelSeg*>::const_iterator travelSegI;
  travelSegI = itin->travelSeg().begin() + travelSegIndex;

  TravelSeg* travelSegFront = itin->travelSeg().front();
  TravelSeg* travelSegBack = itin->travelSeg().back();

  if (travelSegFront == travelSegBack)
  {
    if (CabinValidator().validateCabinRestriction(trx, taxResponse, taxCodeReg, *travelSegI))
      return true;

    return false;
  }

  if (!stopOverTaxNation(trx, taxResponse, taxCodeReg, travelSegIndex))
    return false;

  if (taxChargeOncePerDirection(
          trx, taxResponse, taxCodeReg, itin->segmentOrder(itin->travelSeg()[travelSegIndex])))
    return false;

  if (reverseCabinCnxCheck(trx, taxResponse, taxCodeReg, travelSegI))
    return true;

  if (forwardCabinCnxCheck(trx, taxResponse, taxCodeReg, travelSegI))
    return true;

  return false;
} // end validateTransit

// ---------------------------------------
// Description:  TaxSP36::stopOverTaxNation
// ---------------------------------------
bool
TaxSP36::stopOverTaxNation(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t travelSegIndex)
{
  MirrorImage mirrorImage;

  bool stopOver = mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex);

  TransitValidator transitValidator;

  return transitValidator.validateTransitRestriction(
      trx, taxResponse, taxCodeReg, travelSegIndex, stopOver, _landToAirStopover);
}

// ---------------------------------------
// Description:  TaxSP36::ReverseCabinCnxCheck
// ---------------------------------------
bool
TaxSP36::reverseCabinCnxCheck(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              std::vector<TravelSeg*>::const_iterator travelSegI)
{
  bool stopOverMirrorImage = false;
  bool chargeTax = false;

  const AirSeg* airSeg;
  const Itin* itin = taxResponse.farePath()->itin();

  std::vector<TravelSeg*>::const_iterator travelSegFromI = travelSegI;
  std::vector<TravelSeg*>::const_iterator travelSegToI = travelSegI;
  std::vector<TravelSeg*>::const_iterator tsFromI;
  std::vector<TravelSeg*>::const_iterator tsToI;

  for (; travelSegFromI != itin->travelSeg().begin(); travelSegToI--)
  {
    travelSegFromI--;
    tsFromI = travelSegFromI;
    tsToI = travelSegToI;

    airSeg = dynamic_cast<const AirSeg*>(*tsFromI);

    if (!airSeg)
    {
      if (tsFromI == itin->travelSeg().begin())
        break;

      tsFromI--;
    }

    airSeg = dynamic_cast<const AirSeg*>(*tsToI);

    if (!airSeg)
    {
      if (*tsToI == itin->travelSeg().back())
        break;

      tsToI++;
    }

    if (((*tsFromI)->origin()->nation() == (*tsToI)->destination()->nation() &&
         (*tsFromI)->origin()->nation() != (*tsToI)->origin()->nation()) ||
        (*tsFromI)->origin()->loc() == (*tsToI)->destination()->loc())
      stopOverMirrorImage = true;

    if ((*tsFromI)->isForcedStopOver())
      stopOverMirrorImage = true;

    if (((*tsToI)->isStopOver((*tsFromI), SECONDS_PER_DAY)) && !(*tsFromI)->isForcedConx())
      stopOverMirrorImage = true;

    CabinValidator cabinValidator;
    if (stopOverMirrorImage)
    {
      if (cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsToI))
        return true;

      break;
    }

    if (taxChargeOncePerDirection(trx, taxResponse, taxCodeReg, itin->segmentOrder(*tsFromI)))
      return false;

    if (cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsFromI))
      chargeTax = true;

    if (cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsToI))
      chargeTax = true;
  }

  if (chargeTax)
    return true;

  return false;
}

// ---------------------------------------
// Description:  TaxSP36::ReverseCabinCnxCheck
// ---------------------------------------
bool
TaxSP36::forwardCabinCnxCheck(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              std::vector<TravelSeg*>::const_iterator travelSegI)
{
  EquipmentValidator equipmentValidator;

  bool stopOverMirrorImage = false;
  bool chargeTax = false;

  const AirSeg* airSeg;
  const Itin* itin = taxResponse.farePath()->itin();

  std::vector<TravelSeg*>::const_iterator travelSegFromI = travelSegI;
  std::vector<TravelSeg*>::const_iterator travelSegToI = travelSegI;
  std::vector<TravelSeg*>::const_iterator tsFromI;
  std::vector<TravelSeg*>::const_iterator tsToI;

  travelSegToI++;

  for (; travelSegToI != itin->travelSeg().end(); travelSegFromI++, travelSegToI++)
  {
    tsFromI = travelSegFromI;
    tsToI = travelSegToI;

    airSeg = dynamic_cast<const AirSeg*>(*tsFromI);

    if (!airSeg)
    {
      if (tsFromI == itin->travelSeg().begin())
        break;

      tsFromI--;
    }

    airSeg = dynamic_cast<const AirSeg*>(*tsToI);

    if (!airSeg)
    {
      if (*tsToI == itin->travelSeg().back())
        break;

      tsToI++;
    }

    if (((*tsFromI)->origin()->nation() == (*tsToI)->destination()->nation() &&
         (*tsFromI)->origin()->nation() != (*tsToI)->origin()->nation()) ||
        (*tsFromI)->origin()->loc() == (*tsToI)->destination()->loc())
      stopOverMirrorImage = true;

    if ((*tsFromI)->isForcedStopOver())
      stopOverMirrorImage = true;

    if ((*tsToI)->isStopOver((*tsFromI), SECONDS_PER_DAY) && !(*tsFromI)->isForcedConx())
      stopOverMirrorImage = true;

    CabinValidator cabinValidator;
    if (stopOverMirrorImage)
    {
      if (cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsFromI))
        return true;

      break;
    }

    if (taxChargeOncePerDirection(trx, taxResponse, taxCodeReg, itin->segmentOrder(*tsToI)))
      return false;

    if (cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsFromI))
      chargeTax = true;

    if (cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsToI))
      chargeTax = true;
  }

  if (chargeTax)
    return true;

  return false;
}

// Check for charge directionality
//
//

bool
TaxSP36::taxChargeOncePerDirection(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t segmentOrder)
{
  TaxResponse::TaxItemVector::const_iterator taxItemI = taxResponse.taxItemVector().begin();

  for (; taxItemI != taxResponse.taxItemVector().end(); taxItemI++)
  {
    if ((*taxItemI)->taxCode().substr(0, 2) != taxCodeReg.taxCode().substr(0, 2))
      continue;

    if (segmentOrder == (*taxItemI)->segmentOrderStart())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TAX_ONCE_PER_SEGMENT, Diagnostic809);
      return true;
    }
  }
  return false;
}
