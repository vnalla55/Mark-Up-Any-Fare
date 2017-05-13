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
#include "FreeBagService/AncillaryTrxScopeDateSetter.h"

#include "Common/DateTime.h"
#include "Common/FreeBaggageUtil.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Itin.h"

namespace tse
{
AncillaryTrxScopeDateSetter::AncillaryTrxScopeDateSetter(AncillaryPricingTrx& ancTrx)
  : _ancTrx(ancTrx),
    _handleDate(ancTrx.dataHandle().ticketDate()),
    _ticketingDate(ancTrx.ticketingDate())
{
  _handleDate.setHistoricalIncludesTime();
  _ticketingDate.setHistoricalIncludesTime();
}

AncillaryTrxScopeDateSetter::~AncillaryTrxScopeDateSetter() { update(_handleDate, _ticketingDate); }

void
AncillaryTrxScopeDateSetter::updateDate(const Itin* itin)
{
  const std::map<const Itin*, DateTime>& itin2TicketingDateMap =
      static_cast<AncRequest*>(_ancTrx.getRequest())->ticketingDatesPerItin();

  const std::map<const Itin*, DateTime>::const_iterator itItin2Date =
      itin2TicketingDateMap.find(itin);

  if (itItin2Date != itin2TicketingDateMap.end())
    update(itItin2Date->second, itItin2Date->second);
  else
    update(_handleDate, _ticketingDate);
}

void
AncillaryTrxScopeDateSetter::update(const DateTime& handleDate, const DateTime& ticketingDate)
{
  _ancTrx.dataHandle().setTicketDate(handleDate);
  if (FreeBaggageUtil::isItBaggageDataTransaction(&_ancTrx))
    _ancTrx.ticketingDate() = ticketingDate;
}

} // tse
