// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include "Pricing/BCMixedClassValidator.h"

#include "Common/TseConsts.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"

#include <vector>
namespace tse
{
bool
BCMixedClassValidator::validate(std::string& localRes, FarePath& farePath)
{
  bool valid = checkRebookedClassesForRex(farePath);
  localRes += (valid ? "\nREBOOKED CLS:P " : "\nREBOOKED CLS:F ");

  if (valid && farePath.rebookClassesExists())
  {
    BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(&_trx);
    if (excTrx->applyReissueExchange())
      checkReissueExchangeIndForRebookFP(farePath);
  }
  return valid;
}

void
BCMixedClassValidator::checkReissueExchangeIndForRebookFP(FarePath& fp)
{
  BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(&_trx);
  TravelSeg* tSExh = excTrx->exchangeItin().front()->travelSeg().front();

  FareUsage* fu = fp.pricingUnit().front()->fareUsage().front();
  std::vector<PaxTypeFare::SegmentStatus>& segmentStatus = fu->segmentStatus();
  if (!segmentStatus.empty() &&
      segmentStatus[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
      !segmentStatus[0]._bkgCodeReBook.empty())
  {
    if (fp.exchangeReissue() == EXCHANGE &&
        segmentStatus[0]._bkgCodeReBook == tSExh->getBookingCode() &&
        fu->travelSeg()[0]->changeStatus() == TravelSeg::INVENTORYCHANGED)
    {
      fp.exchangeReissue() = REISSUE;
    }
    else if (fp.exchangeReissue() == REISSUE &&
             segmentStatus[0]._bkgCodeReBook != tSExh->getBookingCode() &&
             fu->travelSeg()[0]->changeStatus() == TravelSeg::UNCHANGED)
    {
      fp.exchangeReissue() = EXCHANGE;
    }
  }
}

void
BCMixedClassValidator::determineIfRebookClassesExist(FarePath& fp)
{
  for (PricingUnit* pricingUnit : fp.pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      for (PaxTypeFare::SegmentStatus& segmentStatus : fareUsage->segmentStatus())
      {
        if (!segmentStatus._bkgCodeSegStatus.isSet(
                PaxTypeFare::BKSS_REX_WP_LOCALMARKET_VALIDATION) &&
            segmentStatus._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
            !segmentStatus._bkgCodeReBook.empty())
        {
          fp.rebookClassesExists() = true;
          return;
        }
      }
    }
  }
}

bool
BCMixedClassValidator::checkRebookedClassesForRex(FarePath& fp)
{
  fp.rebookClassesExists() = false;
  determineIfRebookClassesExist(fp);

  return _trx.getRequest()->isLowFareRequested() || !fp.rebookClassesExists();
}
} // tse namespace
