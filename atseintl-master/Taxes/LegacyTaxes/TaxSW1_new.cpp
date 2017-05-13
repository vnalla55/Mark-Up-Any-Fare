// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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
#include "Common/TravelSegUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/FarePath.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/TaxCodeValidator.h"
#include "Taxes/LegacyTaxes/TaxCodeValidatorNoPsg.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxSW1_new.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"

namespace tse
{

bool
TaxSW1_new::validateTripTypes(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex)
{
  bool isValid = Tax::validateTripTypes(trx, taxResponse, taxCodeReg, startIndex, endIndex);

  if (startIndex > 0 && isValid)
  {
    TravelSeg* travelSegTo = taxResponse.farePath()->itin()->travelSeg()[startIndex];
    TravelSeg* travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[startIndex - 1];

    if (travelSegFrom->destAirport() != travelSegTo->origAirport())
      isValid = false;
  }

  return isValid;
}

bool
TaxSW1_new::validateTransit(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t travelSegIndex)
{
  TravelSeg* travelSeg = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);

  if (!airSeg)
    return false;

  bool isValid = false;

  if (!_hiddenBrdAirport.empty())
    isValid = true;
  else
    isValid = validateTransitIfNoHiddenStop(trx, taxResponse, taxCodeReg, travelSegIndex);

  return isValid;
}

bool
TaxSW1_new::validateLocRestrictions(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t& startIndex,
                                   uint16_t& endIndex)
{
  return true;
}

bool
TaxSW1_new::validateTransitIfNoHiddenStop(PricingTrx& trx,
                                      TaxResponse& taxResponse,
                                      TaxCodeReg& taxCodeReg,
                                      uint16_t travelSegIndex)
{
  bool isValid = false;

  TransitValidator transitValidator;

  isValid = transitValidator.validateTransitRestriction(
      trx, taxResponse, taxCodeReg, travelSegIndex, false, _landToAirStopover);

  if (travelSegIndex > 0 && isValid)
  {

    TravelSeg* travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex - 1];
    TravelSeg* travelSegTo = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];

    bool isSameDay = (travelSegTo->departureDT().day() == travelSegFrom->arrivalDT().day() &&
                      travelSegTo->departureDT().month() == travelSegFrom->arrivalDT().month());

    if (!isSameDay)
      isValid = false;
  }

  return isValid;
}


bool
TaxSW1_new::validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  TaxCodeValidatorNoPsg taxCodeValidator;

  return taxCodeValidator.validateTaxCode(trx, taxResponse, taxCodeReg);
}

bool
TaxSW1_new::validateFinalGenericRestrictions(PricingTrx& trx,
                                      TaxResponse& taxResponse,
                                      TaxCodeReg& taxCodeReg,
                                      uint16_t& startIndex,
                                      uint16_t& /*endIndex*/)
{
  const PaxType* paxType = taxUtil::findActualPaxType (trx, taxResponse.farePath(), startIndex);
  TaxCodeValidator taxCodeValidator;
  return taxCodeValidator.validatePassengerRestrictions(trx, taxResponse, taxCodeReg, paxType);
}

} /* end tse namespace */
