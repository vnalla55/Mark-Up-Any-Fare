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

#include "Taxes/LegacyTaxes/TaxSP54.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"

using namespace tse;
using namespace std;

const char*
TaxSP54::TAX_CODE_ZX("ZX");

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxSP54::TaxSP54() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxSP54::~TaxSP54() {}

// ----------------------------------------------------------------------------
// Description:  validateTransit
// ----------------------------------------------------------------------------

bool
TaxSP54::validateTransit(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex)
{
  std::vector<TravelSeg*>::const_iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin() + travelSegIndex;

  uint16_t index = travelSegIndex;

  if (taxCodeReg.taxCode() == TAX_CODE_ZX)
  {
    travelSegI++;
    index++;

    if (travelSegI == taxResponse.farePath()->itin()->travelSeg().end())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

      return false;
    }
  }
  else
  {
    if (travelSegIndex < 1)
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

      return false;
    }
    travelSegI--;
  }

  if ((*travelSegI)->origin()->nation() == (*travelSegI)->destination()->nation())
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

    return false;
  }

  TransitValidator transitValidator;

  bool transit = transitValidator.validateTransitRestriction(trx, taxResponse, taxCodeReg, index);

  if (!transit)
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

    return false;
  }

  MirrorImage mirrorImage;

  bool stopOver = mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, index);

  if (stopOver)
  {
    if (taxCodeReg.restrictionTransit().empty())
      return true;

    TaxRestrictionTransit& restrictionTransit = taxCodeReg.restrictionTransit().front();

    if (restrictionTransit.transitTaxonly() == YES)
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

      return false;
    }
  }

  return true;
}
