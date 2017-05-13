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

#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Taxes/LegacyTaxes/TaxCodeValidatorNoPsg.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"

#include "Taxes/LegacyTaxes/TaxSW.h"

namespace tse
{

bool
TransitValidatorSW::validTransitIndicatorsIfMultiCity(TravelSeg* travelSegTo,
                                                        TravelSeg* travelSegFrom)
{
  const AirSeg* airSegFrom = dynamic_cast<const AirSeg*>(travelSegFrom);
  const AirSeg* airSegTo = dynamic_cast<const AirSeg*>(travelSegTo);

  return (airSegFrom && airSegTo && travelSegFrom->destAirport() != travelSegTo->origAirport() &&
          travelSegFrom->offMultiCity() == travelSegTo->boardMultiCity());
}

bool
TaxSW::validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  TaxCodeValidatorNoPsg taxCodeValidator;

  return taxCodeValidator.validateTaxCode(trx, taxResponse, taxCodeReg);
}

bool
TaxSW::validateFinalGenericRestrictions(PricingTrx& trx,
                                      TaxResponse& taxResponse,
                                      TaxCodeReg& taxCodeReg,
                                      uint16_t& startIndex,
                                      uint16_t& /*endIndex*/)
{
  const PaxType* paxType = taxUtil::findActualPaxType (trx, taxResponse.farePath(), startIndex);
  TaxCodeValidator taxCodeValidator;
  return taxCodeValidator.validatePassengerRestrictions(trx, taxResponse, taxCodeReg, paxType);
}

bool
TaxSW::validateTransit(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex)
{
  return validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex, false);
}

bool
TaxSW::validateTransit(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex,
                         bool isMirrorImage)
{
  TransitValidatorSW transitValidator;

  bool transitValidatorResult = transitValidator.validateTransitRestriction(
      trx, taxResponse, taxCodeReg, travelSegIndex, isMirrorImage, _landToAirStopover);

  if (transitValidatorResult == true && travelSegIndex > 0 && // we need prevoius segment
      !taxResponse.taxItemVector().empty()) // we need tax item to check against)
  {
    // perform extra check if this result is not a case
    // when previous segment is a "domestic" flight that
    // should be handled as international
    // (currently: NRT-NGO or NGO-NRT for selected carrier/flight)
    // we need to check if previous segment has SW tax and if it's
    // at the same day.

    std::vector<TaxItem*>::const_iterator taxItemI;
    std::vector<TaxItem*>::const_iterator taxItemEndIter = taxResponse.taxItemVector().end();

    for (taxItemI = taxResponse.taxItemVector().begin(); taxItemI != taxItemEndIter; taxItemI++)
    {
      if ((*taxItemI)->taxCode() == taxCodeReg.taxCode() && // this is SW tax
          (*taxItemI)->travelSegStartIndex() == travelSegIndex - 1)
      {
        // this might be the segment we are interesting in.. just check the same day
        Itin* itin = taxResponse.farePath()->itin();
        std::vector<TravelSeg*>::const_iterator prevTravelSegI =
            itin->travelSeg().begin() + (travelSegIndex - 1);
        std::vector<TravelSeg*>::const_iterator currTravelSegI =
            itin->travelSeg().begin() + (travelSegIndex);

        if (((*currTravelSegI)->departureDT().day() == (*prevTravelSegI)->arrivalDT().day()) &&
            ((*currTravelSegI)->departureDT().month() == (*prevTravelSegI)->arrivalDT().month()))
        {

          if (validateTransitLoc((*prevTravelSegI)->destAirport(),
                                 (*currTravelSegI)->origAirport()))
          {
            TaxDiagnostic::collectErrors(
                trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

            return false;
          }
        }
      }
    }
  }

  return transitValidatorResult;
}

}
