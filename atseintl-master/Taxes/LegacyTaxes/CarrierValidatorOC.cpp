// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2010
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

#include "Taxes/LegacyTaxes/CarrierValidatorOC.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DataModel/FarePath.h"
#include "DBAccess/Loc.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "ServiceFees/OCFees.h"
#include <algorithm>
#include <boost/bind.hpp>

using namespace tse;

bool
CarrierValidatorOC::validateCarrier(PricingTrx& trx,
                                    TaxResponse& taxResponse,
                                    TaxCodeReg& taxCodeReg,
                                    uint16_t travelSegIndex)

{

  if (taxCodeReg.exemptionCxr().empty())
    return true;

  bool isValid =
      std::find_if(taxCodeReg.exemptionCxr().begin(),
                   taxCodeReg.exemptionCxr().end(),
                   boost::bind(&CarrierValidatorOC::validateExemptCxrRecord, this, _1)) !=
      taxCodeReg.exemptionCxr().end();

  if ((isValid && taxCodeReg.exempcxrExclInd() != TAX_EXCLUDE) ||
      (!isValid && taxCodeReg.exempcxrExclInd() == TAX_EXCLUDE))
  {
    return true;
  }
  else
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::CARRIER_EXEMPTION, Diagnostic821);
    return false;
  }
}

bool
CarrierValidatorOC::validateExemptCxrRecord(const TaxExemptionCarrier& taxExemptionCarrier)
{
  return validateCarrierCode(_ocFees->carrierCode(), taxExemptionCarrier.carrier());
}
