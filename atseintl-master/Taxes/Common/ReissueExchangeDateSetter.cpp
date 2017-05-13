// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Taxes/Common/ReissueExchangeDateSetter.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/PricingUnit.h"

namespace tse
{

ReissueExchangeDateSetter::ReissueExchangeDateSetter(PricingTrx& pricingTrx, const FarePath& fp)
  : _pricingTrx(pricingTrx),
    _fp(fp),
    _resetTrxSecondRoeIndicator(false),
    _savedTrxSecondRoeIndicator(false),
    _savedTicketingDate(DateTime::emptyDate())
{
  if (RexPricingTrx::isRexTrxAndNewItin(_pricingTrx) &&
      static_cast<RexBaseTrx&>(_pricingTrx).applyReissueExchange())
  {
    RexBaseTrx& rexBaseTrx = static_cast<RexBaseTrx&>(_pricingTrx);
    if (_fp.useSecondRoeDate() && !rexBaseTrx.newItinSecondROEConversionDate().isEmptyDate())
    {
      _savedTrxSecondRoeIndicator = rexBaseTrx.useSecondROEConversionDate();
      rexBaseTrx.useSecondROEConversionDate() = true;
      _resetTrxSecondRoeIndicator = true;
    }

    const DateTime& historicalDate = rexBaseTrx.previousExchangeDT().isEmptyDate()
                                         ? rexBaseTrx.originalTktIssueDT()
                                         : rexBaseTrx.previousExchangeDT();
    const DateTime& taxRetrievalDate =
        useHistoricalDate() ? historicalDate : rexBaseTrx.currentTicketingDT();
    if (rexBaseTrx.ticketingDate() != taxRetrievalDate)
    {
      _savedTicketingDate = rexBaseTrx.ticketingDate();
      rexBaseTrx.setFareApplicationDT(taxRetrievalDate);
    }
  }
}

ReissueExchangeDateSetter::~ReissueExchangeDateSetter()
{
  if (RexPricingTrx::isRexTrxAndNewItin(_pricingTrx) &&
      static_cast<RexBaseTrx&>(_pricingTrx).applyReissueExchange())
  {
    if (_resetTrxSecondRoeIndicator)
      static_cast<RexBaseTrx&>(_pricingTrx).useSecondROEConversionDate() =
          _savedTrxSecondRoeIndicator;
    if (!_savedTicketingDate.isEmptyDate())
      static_cast<RexBaseTrx&>(_pricingTrx).setFareApplicationDT(_savedTicketingDate);
  }
}

bool
ReissueExchangeDateSetter::useHistoricalDate()
{
  if (_fp.isReissue())
  {
    std::vector<PricingUnit*>::const_iterator puI, puE;
    std::vector<FareUsage*>::const_iterator fuI, fuE;

    for (puI = _fp.pricingUnit().begin(), puE = _fp.pricingUnit().end(); puI != puE; ++puI)
    {
      for (fuI = (*puI)->fareUsage().begin(), fuE = (*puI)->fareUsage().end(); fuI != fuE; ++fuI)
      {
        if (!((*fuI)->paxTypeFare()->retrievalFlag() & FareMarket::RetrievCurrent))
          return true;
      }
    }

    return false;
  }

  std::vector<PricingUnit*>::const_iterator puI, puE;
  std::vector<FareUsage*>::const_iterator fuI, fuE;
  for (puI = _fp.pricingUnit().begin(), puE = _fp.pricingUnit().end(); puI != puE; ++puI)
  {
    for (fuI = (*puI)->fareUsage().begin(), fuE = (*puI)->fareUsage().end(); fuI != fuE; ++fuI)
    {
      if ((*fuI)->paxTypeFare()->retrievalFlag() & FareMarket::RetrievCurrent)
        return false;
    }
  }
  return true;
}

}
