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

#include "Taxes/LegacyTaxes/TaxUT_00.h"

#include "Common/FallbackUtil.h"
#include "Common/TrxUtil.h"
#include "DBAccess/TaxCodeReg.h"
#include "Diagnostic/DiagManager.h"

using namespace tse;
using namespace std;

namespace tse
{
FALLBACK_DECL(taxUtRestoreTransitLogic);
}

bool
TaxUT_00::validateTransit(PricingTrx& trx, TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg, uint16_t travelSegIndex)
{
  //with fallback remove whole method
  if (fallback::taxUtRestoreTransitLogic(&trx))
    return true;

  return Tax::validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);
}

bool
TaxUT_00::validateFinalGenericRestrictions(PricingTrx& trx, TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg, uint16_t& startIndex, uint16_t& endIndex)
{
  DiagManager diag(trx, Diagnostic818);

  if (diag.isActive())
  {
    diag << "***\n***UT START SPN PROCESSING***\n***\n" <<
      "START IDX-" << startIndex << " END IDX-" << endIndex << '\n';
  }

  bool ret = false;
  if (!taxCodeReg.restrictionTransit().empty())
  {
    TaxRestrictionTransit transit = taxCodeReg.restrictionTransit().front();
    if (transit.viaLoc().empty() || transit.viaLocType()==0)
    {
      diag << "TRANSIT RECORD DOESNT CONTAIN VIA LOCATION\n";
    }
    else
    {
      TaxLocIterator& locIt = *getLocIterator(*(taxResponse.farePath()), startIndex, transit);
      ret = checkStopover(trx, locIt, transit, endIndex, diag);
    }
  }
  else
  {
    diag << "EMPTY TRANSIT RECORD\n";
  }

  diag << "***\n***UT END SPN PROCESSING***\n***\n";

  return ret;
}

TaxLocIterator*
TaxUT_00::getLocIterator(FarePath& farePath, uint16_t startIndex,
    const TaxRestrictionTransit& transit)
{
  TaxLocIterator* locIt = Tax::getLocIterator(farePath);
  locIt->setSkipHidden(true);
  locIt->toSegmentNo(startIndex);
  locIt->setStopHours(transit.transitHours());

  return locIt;
}


bool
TaxUT_00::checkStopover(PricingTrx& trx,
                        TaxLocIterator& locIt,
                        const TaxRestrictionTransit& transit,
                        uint16_t endIndex,
                        DiagManager& diag) const
{
  while (locIt.hasNext())
  {
    if (locIt.nextSegNo() > endIndex)
      return false;
    if (locIt.isStop() && locIt.isInLoc(transit.viaLocType(), transit.viaLoc(), '\0', trx, false))
    {
      if (diag.isActive())
      {
        diag << "SEGNO-" << locIt.nextSegNo() << " STOPOVER FOUND- " <<
            locIt.nextSeg()->origin()->loc() << '\n';
      }
      return true;
    }
    locIt.next();
  }

  return false;
}


