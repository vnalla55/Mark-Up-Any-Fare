// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
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

#include "Taxes/LegacyTaxes/TaxSP9100.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Common/LocUtil.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"

namespace tse
{

const uint16_t INTERNATIONAL_STOP_HOURS = 24;
const uint16_t DOMESTIC_TRANSBORDER_STOP_HOURS = 4;

bool
TaxSP9100::isUSCA(const Loc& loc)
{
  return (LocUtil::isCanada(loc) || LocUtil::isStPierreMiquelon(loc) ||
          (LocUtil::isUS(loc) && !(LocUtil::isHawaii(loc) || LocUtil::isUSTerritoryOnly(loc))));
}

bool
TaxSP9100::isMirrorImage(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex)
{
  MirrorImage mirrorImage;

  return mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex);
}

bool
TaxSP9100::validateTransit(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t travelSegIndex)
{
  bool firstTime = !_locIteratorInitialized;
  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
  if (firstTime)
  {
    locIt->setSurfaceAsStop(true);
    locIt->setStopHours(DOMESTIC_TRANSBORDER_STOP_HOURS);
    for (locIt->toFront(); !locIt->isEnd(); locIt->next())
    {
      if (!isUSCA(*locIt->loc()))
      {
        locIt->setStopHours(INTERNATIONAL_STOP_HOURS);
        break;
      }
    }
  }

  locIt->toSegmentNo(travelSegIndex);

  if (!taxCodeReg.restrictionTransit().empty())
  {
    TaxRestrictionTransit& restrictTransit = taxCodeReg.restrictionTransit().front();
    if (restrictTransit.transitTaxonly() == YES)
      return !(locIt->isStop() || isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex));
  }

  return locIt->isStop() || isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex);
}
}
