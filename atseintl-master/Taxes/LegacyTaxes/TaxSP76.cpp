// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2009
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

#include "Taxes/LegacyTaxes/TaxSP76.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/AirSeg.h"

using namespace tse;
using namespace std;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxSP76::TaxSP76() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxSP76::~TaxSP76() {}

// ----------------------------------------------------------------------------
// Description:  validateCarrierExemption
// ----------------------------------------------------------------------------

bool
TaxSP76::validateCarrierExemption(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t travelSegIndex)
{
  std::vector<TaxExemptionCarrier>::iterator exemptionCxrIt = taxCodeReg.exemptionCxr().begin();
  std::vector<TaxExemptionCarrier>::iterator exemptionCxrEndIt = taxCodeReg.exemptionCxr().end();

  for (; exemptionCxrIt != exemptionCxrEndIt; exemptionCxrIt++)
  {
    std::vector<TravelSeg*>::iterator travelSegIter =
        taxResponse.farePath()->itin()->travelSeg().begin();
    std::vector<TravelSeg*>::iterator travelSegEndIter =
        taxResponse.farePath()->itin()->travelSeg().end();
    ;

    for (; travelSegIter != travelSegEndIter; travelSegIter++)
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegIter);

      if (!airSeg)
        continue;

      if (airSeg->marketingCarrierCode() != (*exemptionCxrIt).carrier())
        break;
    }

    if (travelSegIter == travelSegEndIter)
      break;
  }

  if (exemptionCxrIt == exemptionCxrEndIt)
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::CARRIER_EXEMPTION, Diagnostic821);

    return false;
  }

  return true;
}
