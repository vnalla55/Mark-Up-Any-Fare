//----------------------------------------------------------------------------
//
// Copyright Sabre 2013
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include "FreeBagService/BaggageTicketingDateScope.h"
#include "Common/TrxUtil.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"

namespace tse
{

BaggageTicketingDateScope::BaggageTicketingDateScope(PricingTrx& trx, const FarePath* farePath)
  : _rexTrx(nullptr)
{
  if (!farePath || !TrxUtil::isBaggage302ExchangeActivated(trx))
    return;

  _rexTrx = dynamic_cast<BaseExchangeTrx*>(&trx);

  if (_rexTrx)
    setTicketingDate(isItReissue(*farePath));
}

BaggageTicketingDateScope::BaggageTicketingDateScope(PricingTrx& trx, const Itin* itin) : _rexTrx(nullptr)
{
  if (!itin || !TrxUtil::isBaggage302ExchangeActivated(trx))
    return;

  _rexTrx = dynamic_cast<BaseExchangeTrx*>(&trx);

  if (_rexTrx)
    setTicketingDate(isItReissue(*itin));
}

BaggageTicketingDateScope::~BaggageTicketingDateScope()
{
  if (UNLIKELY(_rexTrx))
  {
    _rexTrx->dataHandle().setTicketDate(_oldDataHandleTicketingDate);
    _rexTrx->ticketingDate() = _oldTrxTicketingDate;
  }
}

bool
BaggageTicketingDateScope::isItReissue(const FarePath& farePath) const
{
  if (_rexTrx->reqType() == PARTIAL_EXCHANGE && !_rexTrx->applyReissueExcPrimary())
    return true;
  return farePath.isReissue();
}

inline bool
BaggageTicketingDateScope::isItReissue(const Itin& itin) const
{
  for (const FarePath* fp : itin.farePath())
  {
    if (isItReissue(*fp))
      return true;
  }
  return false;
}

inline void
BaggageTicketingDateScope::setTicketingDate(const bool useOriginalTktDate)
{
  _oldDataHandleTicketingDate = _rexTrx->dataHandle().ticketDate();
  _oldTrxTicketingDate = _rexTrx->ticketingDate();

  const DateTime& dateToSet =
      useOriginalTktDate ? _rexTrx->originalTktIssueDT() : _rexTrx->currentTicketingDT();

  _rexTrx->dataHandle().setTicketDate(dateToSet);
  _rexTrx->ticketingDate() = dateToSet;
}

} // tse
