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

#include "Common/RexPricingPhase.h"
#include "DataModel/Trx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexBaseTrx.h"

namespace tse
{
RexPricingPhase::RexPricingPhase(Trx* trx)
{
  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(trx);
  if (pricingTrx == nullptr)
    return;

  const bool isExchange = pricingTrx->excTrxType() == PricingTrx::AR_EXC_TRX;
  const bool isRefund = pricingTrx->excTrxType() == PricingTrx::AF_EXC_TRX;

  _isExcPhase =
      (isExchange || isRefund) &&
      (static_cast<const RexBaseTrx*>(pricingTrx))->trxPhase() == RexBaseTrx::REPRICE_EXCITIN_PHASE;
  _isNewPhase =
      (isExchange || isRefund) &&
      (static_cast<const RexBaseTrx*>(pricingTrx))->trxPhase() == RexBaseTrx::PRICE_NEWITIN_PHASE;
}

bool
RexPricingPhase::isExcPhase() const
{
  return _isExcPhase;
}

bool
RexPricingPhase::isNewPhase() const
{
  return _isNewPhase;
}

} // end of tse namespace
