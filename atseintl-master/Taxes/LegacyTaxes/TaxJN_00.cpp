//---------------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "DBAccess/TaxCodeReg.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/CabinValidator.h"
#include "Taxes/LegacyTaxes/TaxJN_00.h"

namespace tse
{

bool
TaxJN_00::validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  //reset state
  _cabinWasChecked = false;

  return Tax::validateItin(trx, taxResponse, taxCodeReg);
}

bool
TaxJN_00::validateCabin(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        TaxCodeReg& taxCodeReg,
                        uint16_t travelSegIndex)
{
  if (_cabinWasChecked)
    return _cabinIsValid;

  _cabinWasChecked = true;

  std::vector<TravelSeg*>::const_iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin();

  //find first air/flight segment
  for ( ; travelSegI!=taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
    if ( (*travelSegI)->isAir()
        && (*travelSegI)->segmentType() != Surface
        && (*travelSegI)->segmentType() != Arunk )
    {
      break;
    }

  _cabinIsValid = CabinValidator().validateCabinRestriction(trx, taxResponse, taxCodeReg, *travelSegI);

  return _cabinIsValid;
}



}; //namespace tse
