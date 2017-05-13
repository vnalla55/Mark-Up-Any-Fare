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

#include "Taxes/LegacyTaxes/TaxGB.h"
#include "DBAccess/TaxCodeCabin.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "DBAccess/TaxRestrictionTransit.h"
#include "Taxes/LegacyTaxes/CabinValidator.h"
#include "Common/LocUtil.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Common/Vendor.h"

using namespace tse;
using namespace std;

const std::string TaxGB::GREAT_BRITAIN_CODE = "GB";

const uint16_t GREAT_BRITIAN_GB37 = 37;
const uint16_t GREAT_BRITIAN_GB38 = 38;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

const std::string TaxGB::_specialEquipmentTypes = "TRN,TGV,ICE";

TaxGB::TaxGB()
{
  _nLowCabinTaxCodeSPN = GREAT_BRITIAN_GB37;
  _nHighCabinTaxCodeSPN = GREAT_BRITIAN_GB38;
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxGB::~TaxGB() {}

// ----------------------------------------
// Description: TaxGB::validateCabin
// ---------------------------------------
bool
TaxGB::validateCabin(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     uint16_t travelSegIndex)
{
  return true;
} // end of validateCabin

// ----------------------------------------
// Description: TaxGB::validateTripType
// ---------------------------------------
bool
TaxGB::validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex)
{
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
TaxGB::validateFromTo(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      uint16_t& startIndex,
                      uint16_t& endIndex)
{

  if (taxCodeReg.loc1Type() == LOCTYPE_NONE)
    return true;

  _specialIndex = false;
  _travelSegSpecialTaxStartIndex = 0;
  _travelSegSpecialTaxEndIndex = 0;

  uint16_t index;
  uint16_t termIndex;
  bool foundStart = false;
  bool geoMatch;

  std::vector<TravelSeg*>::iterator travelSegI;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin() + startIndex;

  endIndex = taxResponse.farePath()->itin()->travelSeg().size() - 1;

  for (index = startIndex; index <= endIndex; ++index, ++travelSegI)
  {
    geoMatch = LocUtil::isInLoc(*(*travelSegI)->origin(),
                                taxCodeReg.loc1Type(),
                                taxCodeReg.loc1(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES);

    if ((geoMatch && taxCodeReg.loc1Type() == LOCTYPE_ZONE) || // Can't check exclind fo zone
        (geoMatch && taxCodeReg.loc1ExclInd() != TAX_EXCLUDE) ||
        (!geoMatch && taxCodeReg.loc1ExclInd() == TAX_EXCLUDE))
    {
      startIndex = index;
      endIndex = index; // From/To and Between must match on the same travelSeg
      foundStart = true;
      break;
    }
  } // end of ItinItem Index loop

  if (foundStart) // We have found the Loc1 -- now check loc2
  {
    if (taxCodeReg.loc2Type() == LOCTYPE_NONE)
      return true;

    std::vector<TravelSeg*>::const_iterator travelSegI;

    travelSegI = taxResponse.farePath()->itin()->travelSeg().begin() + startIndex;

    termIndex = startIndex;

    if ((*travelSegI) != taxResponse.farePath()->itin()->travelSeg().back() &&
        (!(*travelSegI)->isForcedStopOver()))
    {
      for (; (*travelSegI) != taxResponse.farePath()->itin()->travelSeg().back();
           travelSegI++, termIndex++)
      {
        if ((*travelSegI)->isForcedConx())
          continue;

        if ((*travelSegI)->isForcedStopOver())
          break;

        const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

        if (!airSeg)
          continue;

        if ((*travelSegI) != taxResponse.farePath()->itin()->travelSeg().back())
        {
          TravelSeg* travelSegTo = taxResponse.farePath()->itin()->travelSeg()[termIndex + 1];

          if (airSeg->destination()->nation() == GREAT_BRITAIN_CODE &&
              travelSegTo->origin()->nation() == GREAT_BRITAIN_CODE)
          {

            if (travelSegTo->isStopOver(airSeg, airSeg->geoTravelType(), TravelSeg::OTHER))
            {
              break;
            }

            if (airSeg->origin()->nation() == travelSegTo->destination()->nation())
            {
              break;
            }
          }
          else
          {
            if (travelSegTo->isStopOver(airSeg, SECONDS_PER_DAY))
            {
              break;
            }

            if (taxResponse.farePath()->itin()->travelSeg().size() == 2 &&
                airSeg->origin()->nation() == travelSegTo->destination()->nation())
            {
              break;
            }
          }
        }
      }

      _specialIndex = true;
      _travelSegSpecialTaxStartIndex = startIndex;
      _travelSegSpecialTaxEndIndex = termIndex;
      endIndex = termIndex;
    }

    geoMatch = LocUtil::isInLoc(*(*travelSegI)->destination(),
                                taxCodeReg.loc2Type(),
                                taxCodeReg.loc2(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES);

    if ((geoMatch && taxCodeReg.loc2Type() == LOCTYPE_ZONE) ||
        (geoMatch && taxCodeReg.loc2ExclInd() != TAX_EXCLUDE) ||
        (!geoMatch && taxCodeReg.loc2ExclInd() == TAX_EXCLUDE))
    {
      _specialIndex = true;
      _travelSegSpecialTaxStartIndex = startIndex;
      _travelSegSpecialTaxEndIndex = termIndex;
      endIndex = termIndex;
      return true; // We found a tax -- return it and expect to get called again
      // to check the rest of the itinItems.
    }
  } // foundStart
  // if !foundStop
  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic820);

  return false; // We fail this tax.
}

// ---------------------------------------
// Description:  TaxGB::validateTransit
// ---------------------------------------
bool
TaxGB::validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex)
{
  bool geoMatch1 = false;
  bool geoMatch2 = false;

  bool firstSegment = false;
  bool domDomTravel = false;
  bool intlDomTravel = false;

  std::vector<TravelSeg*>::const_iterator tvlSegI;
  std::vector<TravelSeg*>::const_iterator tvlSegPrevI;

  tvlSegI = taxResponse.farePath()->itin()->travelSeg().begin() + (travelSegIndex);

  Itin* itin = taxResponse.farePath()->itin();

  if (travelSegIndex > 0)
  {
    tvlSegPrevI = taxResponse.farePath()->itin()->travelSeg().begin() + (travelSegIndex - 1);

    if ((*tvlSegPrevI) != taxResponse.farePath()->itin()->travelSeg().front())
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*tvlSegPrevI);

      if (!airSeg)
      {
        tvlSegPrevI = taxResponse.farePath()->itin()->travelSeg().begin() + (travelSegIndex - 2);
      }
    }
  }
  else
  {
    firstSegment = true;
    tvlSegPrevI = tvlSegI;
  }

  const Loc* origin = (*tvlSegPrevI)->origin();
  const Loc* viaLoc = (*tvlSegPrevI)->destination();
  const Loc* destination = (*tvlSegI)->destination();

  geoMatch1 = origin->nation() == viaLoc->nation();
  geoMatch2 = destination->nation() == viaLoc->nation();

  // interested in two scenarios:
  // we are traveling within GB
  // we are going out of GB

  if (geoMatch1 && geoMatch2 && viaLoc->nation() == GREAT_BRITAIN_CODE)
  {
    domDomTravel = true;
  }
  else if (!geoMatch1 && geoMatch2 && viaLoc->nation() == GREAT_BRITAIN_CODE)
  {
    intlDomTravel = true;
  }

  TravelSeg* travelSegFront = itin->travelSeg().front();
  TravelSeg* travelSegBack = itin->travelSeg().back();

  if (travelSegFront == travelSegBack)
    return CabinValidator().validateCabinRestriction(trx, taxResponse, taxCodeReg, *tvlSegI);

  bool isForwardCabinCheckValidated;
  bool isReverseCabinCheckValidated;
  bool isTaxApplicableForward = false;
  bool isTaxApplicableReverse = false;

  isForwardCabinCheckValidated = forwardCabinCnxCheck(
      trx, taxResponse, taxCodeReg, tvlSegI, isTaxApplicableForward, domDomTravel || intlDomTravel);
  isReverseCabinCheckValidated = reverseCabinCnxCheck(
      trx, taxResponse, taxCodeReg, tvlSegI, isTaxApplicableReverse, domDomTravel || intlDomTravel);

  if (taxCodeReg.specialProcessNo() ==
      _nHighCabinTaxCodeSPN) // high tax fail only if both cabin checks are failing
  {
    if (!((isForwardCabinCheckValidated && isTaxApplicableForward) ||
          (isReverseCabinCheckValidated && isTaxApplicableReverse)))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::CLASS_OF_SERVICE, Diagnostic820);
      return false;
    }
  }
  else if (taxCodeReg.specialProcessNo() ==
           _nLowCabinTaxCodeSPN) // low tax fail if any cabin check is failing
  {
    if ((isForwardCabinCheckValidated && !isTaxApplicableForward) ||
        (isReverseCabinCheckValidated && !isTaxApplicableReverse))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::CLASS_OF_SERVICE, Diagnostic820);
      return false;
    }
  }

  MirrorImage mirrorImage;
  bool stopOver = mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex);
  if (stopOver)
    return true;

  // Case with MLA-LPL/MAN-MLA  (Arunk from LPL-MAN with less than 24 hours conx in GB)
  // Mirror Image function above return "FALSE" as it is an arunk!!!
  // Need to treat as Mirror Image...
  // *** as it is an arunk, initial code has already set tvlSegPrevI for segment before Arunk...
  // *** use it to compare to destination country...

  if (!domDomTravel && (*tvlSegPrevI)->destination() != (*tvlSegI)->origin() &&
      (*tvlSegPrevI)->origin()->nation() == (*tvlSegI)->destination()->nation())
    return true;

  if (isForcedStopOver(*tvlSegI, *tvlSegPrevI))
    return true;

  bool failedTransitRestriction = false;

  if (firstSegment)
  {
    // starting in GB flying out of GB
    if (origin->nation() != GREAT_BRITAIN_CODE)
      failedTransitRestriction = true;
    else
      return true;
  }
  else
  {
    // simply try if dom dom - 6 hours, otherwise 24 h
    failedTransitRestriction =
        !this->isStopOver(*tvlSegI, *tvlSegPrevI, domDomTravel || intlDomTravel);
  }

  if (failedTransitRestriction)
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);
    return false;
  }

  return true;
}

// -----------------------------------------
// Description:  TaxGB::reverseCabinCnxCheck
// -----------------------------------------
bool
TaxGB::reverseCabinCnxCheck(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            std::vector<TravelSeg*>::const_iterator travelSegI,
                            bool& isTaxApplicable,
                            bool isTravelWithinUK)
{
  bool stopOverMirrorImage = false;

  if (taxCodeReg.specialProcessNo() == _nHighCabinTaxCodeSPN)
    isTaxApplicable = false;
  else
    isTaxApplicable = true;

  const AirSeg* airSeg;
  Itin* itin = taxResponse.farePath()->itin();

  std::vector<TravelSeg*>::const_iterator travelSegFromI = travelSegI;
  std::vector<TravelSeg*>::const_iterator travelSegToI = travelSegI;
  std::vector<TravelSeg*>::const_iterator tsFromI;
  std::vector<TravelSeg*>::const_iterator tsToI;

  if (travelSegFromI == itin->travelSeg().begin())
    return false;

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

    if (isForcedStopOver(*tsToI, *tsFromI))
      stopOverMirrorImage = true;

    if (this->isStopOver(*tsToI, *tsFromI, isTravelWithinUK) && !(*tsFromI)->isForcedConx())
      stopOverMirrorImage = true;

    CabinValidator cabinValidator;
    if (stopOverMirrorImage)
    {
      if (cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsToI))
        isTaxApplicable = true;
      else
        isTaxApplicable = false;
      return true;
    }

    if (taxCodeReg.specialProcessNo() == _nHighCabinTaxCodeSPN)
    {
      if (cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsFromI) ||
          cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsToI))
      {
        isTaxApplicable = true;
        break;
      }
    }
    else
    {
      if (!cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsFromI) ||
          !cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsToI))
      {
        isTaxApplicable = false;
        break;
      }
    }
  }

  return true;
}

// -----------------------------------------
// Description:  TaxGB::forwardCabinCnxCheck
// -----------------------------------------
bool
TaxGB::forwardCabinCnxCheck(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            std::vector<TravelSeg*>::const_iterator travelSegI,
                            bool& isTaxApplicable,
                            bool isTravelWithinUK)
{
  bool stopOverMirrorImage = false;

  if (taxCodeReg.specialProcessNo() == _nHighCabinTaxCodeSPN)
    isTaxApplicable = false;
  else
    isTaxApplicable = true;

  const AirSeg* airSeg;
  Itin* itin = taxResponse.farePath()->itin();

  std::vector<TravelSeg*>::const_iterator travelSegFromI = travelSegI;
  std::vector<TravelSeg*>::const_iterator travelSegToI = travelSegI;
  std::vector<TravelSeg*>::const_iterator tsFromI;
  std::vector<TravelSeg*>::const_iterator tsToI;

  travelSegToI++;

  if (travelSegToI == itin->travelSeg().end())
    return false;

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

    if (this->isStopOver(*tsToI, *tsFromI, isTravelWithinUK) && !(*tsFromI)->isForcedConx())
      stopOverMirrorImage = true;

    CabinValidator cabinValidator;
    if (stopOverMirrorImage)
    {
      if (cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsFromI))
        isTaxApplicable = true;
      else
        isTaxApplicable = false;

      return true;
    }

    if (taxCodeReg.specialProcessNo() == _nHighCabinTaxCodeSPN)
    {
      if (cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsFromI) ||
          cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsToI))
      {
        isTaxApplicable = true;
        break;
      }
    }
    else
    {
      if (!cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsFromI) ||
          !cabinValidator.validateCabinRestriction(trx, taxResponse, taxCodeReg, *tsToI))
      {
        isTaxApplicable = false;
        break;
      }
    }
  }

  return true;
}

bool
TaxGB::isStopOver(const TravelSeg* pTvlSeg, const TravelSeg* pTvlSegPrev, bool isTravelWithinUK)
{

  DateTime dtDepart = pTvlSeg->departureDT();
  DateTime dtArrival = pTvlSegPrev->arrivalDT();

  DateTime nextDay = pTvlSegPrev->arrivalDT().addDays(1);

  int32_t departHr = dtDepart.hours();
  int32_t departMin = dtDepart.minutes();
  int64_t ttlDepartMin = ((departHr * 60) + departMin); // lint !e647

  int32_t arrivalHr = dtArrival.hours();
  int32_t arrivalMin = dtArrival.minutes();
  int64_t ttlArrivalMin = (arrivalHr * 60) + arrivalMin; // lint !e647

  // Data 1700 hours and 1000 hours Check
  int32_t seventeenHr = 17;
  int32_t seventeenMin = 0;
  int64_t ttlSeventeenMin = ((seventeenHr * 60) + seventeenMin); // lint !e647

  int32_t tenHr = 10;
  int32_t tenMin = 0;
  int64_t ttlTenMin = ((tenHr * 60) + tenMin); // lint !e647

  int64_t connectTime = 0;
  int64_t connectionMin = 0;

  connectTime = DateTime::diffTime(pTvlSeg->departureDT(), pTvlSegPrev->arrivalDT());
  connectionMin = (connectTime / 60);

  int32_t transitTotalMinDom = 360; // 6 hours = 360 mins = 21600 secs
  int32_t transitTotalMinIntl = 1440; // 24 hours = 1440 mins = 86400 secs

  if (isTravelWithinUK) // checek 6 hours
  {
    if ((connectionMin >= transitTotalMinDom) &&
        !((ttlArrivalMin > ttlSeventeenMin) && (ttlDepartMin < ttlTenMin) &&
          (pTvlSeg->departureDT().day() == nextDay.day() &&
           pTvlSeg->departureDT().month() == nextDay.month())))
      return true;
    else
      return false;
  }
  else
  {
    if ((connectionMin >= transitTotalMinIntl))
      return true;
    else
      return false;
  }

  return false;
}

bool
TaxGB::isForcedStopOver(const TravelSeg*, const TravelSeg* pTvlSegPrev)
{
  if (pTvlSegPrev->isForcedStopOver())
    return true;

  return false;
}
