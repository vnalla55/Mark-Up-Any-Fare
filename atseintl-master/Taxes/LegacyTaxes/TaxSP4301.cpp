// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Taxes/LegacyTaxes/TaxSP4301.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Common/TseUtil.h"

namespace tse
{

bool
TaxLocIterator43::isStop()
{
  if (loc()->loc() == "UIO")
    return true;

  if (isMirror())
    return true;

  return TaxLocIterator::isStop();
}

TaxLocIterator*
TaxSP4301::getLocIterator(FarePath& farePath)
{
  if (!_locIteratorInitialized)
  {
    _locIt43.setSkipHidden(true);
    _locIt43.setSurfaceAsStop(true);
    _locIt43.initialize(farePath);
    _locIteratorInitialized = true;
  }

  return &_locIt43;
}

bool
TaxSP4301::validateTripTypes(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex)
{
  return validateFromToWithHiddenPoints(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

bool
TaxSP4301::validateTransit(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t travelSegIndex)
{
  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
  locIt->toSegmentNo(travelSegIndex);
  return locIt->isStop();
}
}
