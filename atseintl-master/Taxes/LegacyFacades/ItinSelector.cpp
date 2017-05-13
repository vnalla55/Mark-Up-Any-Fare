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

#include "Common/TrxUtil.h"
#include "DataModel/ExcItin.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include <algorithm>
#include <iterator>

namespace tse
{
ItinSelector::ItinSelector(tse::PricingTrx& trx) : _trx(trx)
{
  _isExcangeTrx = _trx.excTrxType() == PricingTrx::AR_EXC_TRX;
  _isRefundTrx = _trx.excTrxType() == PricingTrx::AF_EXC_TRX;
  _isTaxInfoReq = _trx.excTrxType() == PricingTrx::TAX_INFO_TRX;
  _isExcItin =
      (_isExcangeTrx || _isRefundTrx) &&
      (static_cast<const RexBaseTrx*>(&_trx))->trxPhase() == RexBaseTrx::REPRICE_EXCITIN_PHASE;

  _isFullRefund = _isRefundTrx && static_cast<const RefundPricingTrx&>(_trx).fullRefund();
}

std::vector<Itin*>
ItinSelector::get() const
{
  std::vector<Itin*> result;
  if (isExcItin() || isCat33FullRefund())
  {
    RexBaseTrx& rexTrx = static_cast<RexBaseTrx&>(_trx);
    std::copy(
        rexTrx.exchangeItin().begin(), rexTrx.exchangeItin().end(), std::back_inserter(result));
  }
  else
  {
    std::copy(_trx.itin().begin(), _trx.itin().end(), std::back_inserter(result));
  }
  return result;
}

std::vector<Itin*>
ItinSelector::getItin() const
{
  std::vector<Itin*> result;

  if (isExcItin())
  {
    RexBaseTrx& rexTrx = static_cast<RexBaseTrx&>(_trx);
    std::copy(
        rexTrx.exchangeItin().begin(), rexTrx.exchangeItin().end(), std::back_inserter(result));
  }
  else
  {
    std::copy(_trx.itin().begin(), _trx.itin().end(), std::back_inserter(result));
  }

  return result;
}

bool
ItinSelector::isExcItin() const
{
  return _isExcItin;
}

bool
ItinSelector::isExchangeTrx() const
{
  return _isExcangeTrx;
}

bool
ItinSelector::isRefundTrx() const
{
  return _isRefundTrx;
}

bool
ItinSelector::isCat33FullRefund() const
{
  return _isFullRefund && TrxUtil::isAutomatedRefundCat33Enabled(_trx);
}

bool
ItinSelector::isNewItin() const
{
  return !_isExcItin;
}

bool
ItinSelector::isTaxInfoReq() const
{
  return _isTaxInfoReq;
}
}
