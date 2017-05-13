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

using namespace tse;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

LocRestrictionValidator::LocRestrictionValidator()
    {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

LocRestrictionValidator::~LocRestrictionValidator() {}

// ----------------------------------------------------------------------------
// Description:  LocRestrictionValidator
// ----------------------------------------------------------------------------

bool
LocRestrictionValidator::validateLocation(PricingTrx& trx,
                                          TaxResponse& taxResponse,
                                          TaxCodeReg& taxCodeReg,
                                          uint16_t& startIndex,
                                          uint16_t& endIndex)
{
  if ((taxCodeReg.loc1Type() == LOCTYPE_NONE) && (taxCodeReg.loc2Type() == LOCTYPE_NONE))
  {
    endIndex = startIndex;
    return true;
  }

  if (UNLIKELY((trx.diagnostic().diagnosticType() == FailTaxCodeDiagnostic) ||
      (trx.diagnostic().diagnosticType() == Diagnostic816)))
    taxResponse.diagCollector()->enable(FailTaxCodeDiagnostic, Diagnostic816);

  bool rc = false;

  switch (taxCodeReg.loc1Appl())
  {

  case TAX_ORIGIN:
  {
    rc = validateOrigin(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    if (!rc)
      return rc;

    break;
  }

  case TAX_ENPLANEMENT:
  {
    rc = validateEnplanement(trx, taxResponse, taxCodeReg, startIndex, endIndex);
    if (!rc)
      return rc;

    break;
  }
  default:
    break;
  } // end of switch

  switch (taxCodeReg.loc2Appl())
  {

  case TAX_DEPLANEMENT:
  {
    return rc = validateDeplanement(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  }

  case TAX_DESTINATION:
  {
    return rc = validateDestination(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  }

  case TAX_TERMINATION:
  {
    return rc = validateTermination(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  }

  default:
    return true;
  } // end of switch
} // end of validateLocation

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void LocRestrictionValidator::validateOrigin
//
// Description:  This function will validate ORIGIN
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
LocRestrictionValidator::validateOrigin(PricingTrx& trx,
                                        TaxResponse& taxResponse,
                                        TaxCodeReg& taxCodeReg,
                                        uint16_t& startIndex,
                                        uint16_t& endIndex)
{
  //
  // Per Legacy only the First Segment in Itinerary can apply..
  //

  if (startIndex)
  {
    if (UNLIKELY(taxResponse.diagCollector()->isActive()))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::ORIGIN_LOCATION, Diagnostic816);
    }

    return false;
  }

  const TravelSeg& travelSeg = *getTravelSeg(taxResponse).front();

  bool locMatch = LocUtil::isInLoc(*travelSeg.origin(),
                                   taxCodeReg.loc1Type(),
                                   taxCodeReg.loc1(),
                                   Vendor::SABRE,
                                   MANUAL,
                                   LocUtil::TAXES,
                                   GeoTravelType::International,
                                   EMPTY_STRING(),
                                   trx.getRequest()->ticketingDT());

  if ((locMatch && taxCodeReg.loc1Type() == LOCTYPE_ZONE) ||
      (locMatch && taxCodeReg.loc1ExclInd() != TAX_EXCLUDE) ||
      (!locMatch && taxCodeReg.loc1ExclInd() == TAX_EXCLUDE))
  {
    startIndex = 0;
    endIndex = getTravelSeg(taxResponse).size() - 1;
    return true;
  }

  if (taxResponse.diagCollector()->isActive())
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::ORIGIN_LOCATION, Diagnostic816);
  }
  return false;
} // end TAX_ORIGIN

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void LocRestrictionValidator::validateEnplanement
//
// Description:  This function will validate ENPLANEMENT
//
// @param  PricingTrx - Transaction object
// @param  TaxCodeReg - TaxCodeReg object
// @param  startIndex - first TrvelSeg to serach from
// @param  endIndex - last TravelSeg to search to
//
// @return bool - true :  applied;
//                false : is not applied;
//
// </PRE>
// ----------------------------------------------------------------------------

bool
LocRestrictionValidator::validateEnplanement(PricingTrx& trx,
                                             TaxResponse& taxResponse,
                                             TaxCodeReg& taxCodeReg,
                                             uint16_t& startIndex,
                                             uint16_t& endIndex)

{
  std::vector<TravelSeg*>::const_iterator travelSegI =
    getTravelSeg(taxResponse).begin() + startIndex;

  const AirSeg* airSeg;

  bool geoMatch;

  for (uint16_t index = startIndex; travelSegI != getTravelSeg(taxResponse).end();
       travelSegI++, index++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    geoMatch = LocUtil::isInLoc(*(*travelSegI)->origin(),
                                taxCodeReg.loc1Type(),
                                taxCodeReg.loc1(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if (taxCodeReg.loc1Type() == LOCTYPE_ZONE)
    {
      if (!geoMatch)
        continue;
    }
    else if ((geoMatch && taxCodeReg.loc1ExclInd() == TAX_EXCLUDE) ||
             (!geoMatch && taxCodeReg.loc1ExclInd() != TAX_EXCLUDE))
    {
      continue;
    }

    startIndex = index;
    endIndex = index;
    return true;
  }

  if (UNLIKELY(taxResponse.diagCollector()->isActive()))
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::ENPLANEMENT_LOCATION, Diagnostic816);
  }

  return false;
} // end TAX_ENPLANEMENT

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void LocRestrictionValidator::validateDeplanement
//
// Description:  This function will validate DEPLANEMENT
//
// @param  PricingTrx - Transaction object
// @param  TaxCodeReg - TaxCodeReg object
// @param  startIndex - first TrvelSeg to serach from
// @param  endIndex - last TravelSeg to search to
//
// @return bool - true :  applied;
//                false : is not applied;
//
// </PRE>
// ----------------------------------------------------------------------------

bool
LocRestrictionValidator::validateDeplanement(PricingTrx& trx,
                                             TaxResponse& taxResponse,
                                             TaxCodeReg& taxCodeReg,
                                             uint16_t& startIndex,
                                             uint16_t& endIndex)
{
  std::vector<TravelSeg*>::const_iterator travelSegI =
    getTravelSeg(taxResponse).begin() + startIndex;

  const AirSeg* airSeg;

  bool geoMatch;

  for (uint16_t index = startIndex; travelSegI != getTravelSeg(taxResponse).end();
       travelSegI++, index++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    geoMatch = LocUtil::isInLoc(*(*travelSegI)->destination(),
                                taxCodeReg.loc2Type(),
                                taxCodeReg.loc2(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if (taxCodeReg.loc2Type() == LOCTYPE_ZONE)
    {
      if (!geoMatch)
        continue;
    }
    else if ((geoMatch && taxCodeReg.loc2ExclInd() == TAX_EXCLUDE) ||
             (!geoMatch && taxCodeReg.loc2ExclInd() != TAX_EXCLUDE))
    {
      continue;
    }

    startIndex = index;
    endIndex = index;
    return true;
  }

  if (UNLIKELY(taxResponse.diagCollector()->isActive()))
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::DEPLANEMENT_LOCATION, Diagnostic816);
  }
  return false;
} // end TAX_DEPLANEMENT

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void LocRestrictionValidator::validateDestination
//
// Description:  This function will validate DESTINATION
//
// @param  PricingTrx - Transaction object
// @param  TaxCodeReg - TaxCodeReg object
// @param  startIndex - first TrvelSeg to serach from
// @param  endIndex - last TravelSeg to search to
//
// @return bool - true :  applied;
//                false : is not applied;
//
// </PRE>
// ----------------------------------------------------------------------------

bool
LocRestrictionValidator::validateDestination(PricingTrx& trx,
                                             TaxResponse& taxResponse,
                                             TaxCodeReg& taxCodeReg,
                                             uint16_t& startIndex,
                                             uint16_t& endIndex)
{
  std::vector<TravelSeg*>::const_iterator travelSegI =
      getTravelSeg(taxResponse).begin() + startIndex;
  const Itin* itin = _itin ? _itin : taxResponse.farePath()->itin();
  bool locMatch;

  for (uint16_t index = startIndex; travelSegI != getTravelSeg(taxResponse).end();
       travelSegI++, index++)
  {
    if ((!(*travelSegI)->isFurthestPoint(*itin)) &&
        (*travelSegI) != getTravelSeg(taxResponse).back())
      continue;

    locMatch = LocUtil::isInLoc(*(*travelSegI)->destination(),
                                taxCodeReg.loc2Type(),
                                taxCodeReg.loc2(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if (taxCodeReg.loc2Type() == LOCTYPE_ZONE)
    {
      if (!locMatch)
        continue;
    }
    else if ((locMatch && taxCodeReg.loc2ExclInd() == TAX_EXCLUDE) ||
             (!locMatch && taxCodeReg.loc2ExclInd() != TAX_EXCLUDE))
    {
      continue;
    }

    // PL72394 Across multiple segments for YQ Manual
    //  startIndex = index;
    endIndex = index;
    return true;
  }

  if (taxResponse.diagCollector()->isActive())
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::DESTINATION_LOCATION, Diagnostic816);
  }

  return false;
} // end TAX_DESTINATION

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void LocRestrictionValidator::validateTermination
//
// Description:  This function will validate TERMINATION
//
// @param  PricingTrx - Transaction object
// @param  TaxCodeReg - TaxCodeReg object
// @param  startIndex - first TrvelSeg to serach from
// @param  endIndex - last TravelSeg to search to
//
// @return bool - true :  applied;
//                false : is not applied;
//
// </PRE>
// ----------------------------------------------------------------------------

bool
LocRestrictionValidator::validateTermination(PricingTrx& trx,
                                             TaxResponse& taxResponse,
                                             TaxCodeReg& taxCodeReg,
                                             uint16_t& startIndex,
                                             uint16_t& endIndex)
{
  if (startIndex)
  {
    if (taxResponse.diagCollector()->isActive())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TERMINATION_LOCATION, Diagnostic816);
    }

    return false;
  }

  // lint -e{530}
  TravelSeg* travelSeg = getTravelSeg(taxResponse).back();

  bool locMatch = LocUtil::isInLoc(*travelSeg->destination(),
                                   taxCodeReg.loc2Type(),
                                   taxCodeReg.loc2(),
                                   Vendor::SABRE,
                                   MANUAL,
                                   LocUtil::TAXES,
                                   GeoTravelType::International,
                                   EMPTY_STRING(),
                                   trx.getRequest()->ticketingDT());

  if ((locMatch && taxCodeReg.loc2Type() == LOCTYPE_ZONE) ||
      (locMatch && taxCodeReg.loc2ExclInd() != TAX_EXCLUDE) ||
      (!locMatch && taxCodeReg.loc2ExclInd() == TAX_EXCLUDE))
  {
    startIndex = getTravelSeg(taxResponse).size() - 1;
    endIndex = startIndex;
    return true;
  }

  if (taxResponse.diagCollector()->isActive())
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TERMINATION_LOCATION, Diagnostic816);
  }
  return false;
} // end TAX_TERMINATION

const std::vector<TravelSeg*>&
LocRestrictionValidator::getTravelSeg(TaxResponse& taxResponse) const
{
  return _itin ? _itin->travelSeg() : taxResponse.farePath()->itin()->travelSeg();
}
