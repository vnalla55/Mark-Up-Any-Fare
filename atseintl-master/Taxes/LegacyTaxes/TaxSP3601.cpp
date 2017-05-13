#include <vector>

#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxSP3601.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"

namespace tse
{

bool
TaxSP3601::validateLocRestrictions(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t& startIndex,
                                   uint16_t& endIndex)
{
  if ((!_locRestrictionValidator.fareBreaksFound()) &&
      trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX && trx.getTrxType() != PricingTrx::IS_TRX)
    _locRestrictionValidator.findFareBreaks(*(taxResponse.farePath()));
  _locRestrictionValidator.findFarthestPoint(trx, *(taxResponse.farePath()->itin()), startIndex);
  return _locRestrictionValidator.validateLocation(
      trx, taxResponse, taxCodeReg, startIndex, endIndex);
}
}
