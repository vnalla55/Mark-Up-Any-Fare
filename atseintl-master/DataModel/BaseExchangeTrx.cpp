//-------------------------------------------------------------------
//
//  File:        BaseExchangeTrx.cpp
//  Created:     March 25, 2008
//  Design:
//
//  Description: Transaction's root object.
//
//  Updates:
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "DataModel/BaseExchangeTrx.h"

#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/Billing.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingTrx.h"
#include "Rules/RuleConst.h"

namespace tse
{
BaseExchangeTrx::BaseExchangeTrx()
{
  _baggagePolicy->setupPolicy(BaggagePolicy::UNFLOWN_TRAVELS, false);
}

const DateTime&
BaseExchangeTrx::originalTktIssueDT() const
{
  return (applyReissueExchange() && !_previousExchangeDT.isEmptyDate()) ? _previousExchangeDT
                                                                        : _originalTktIssueDT;
}

void
BaseExchangeTrx::setActionCode()
{
  if (!_billing)
    return;
  if (_billing->actionCode().size() > 3)
    _billing->actionCode() = _billing->actionCode().substr(0, 3) + _reqType;
  else
    _billing->actionCode() += _reqType;
}

void
BaseExchangeTrx::setItinIndex(uint16_t itinIndex)
{
  if (_itinIndex == itinIndex)
    return;

  _itinIndex = itinIndex;
  std::vector<FareCompInfo*>::const_iterator fcIt = exchangeItin().front()->fareComponent().begin();
  for (; fcIt != exchangeItin().front()->fareComponent().end(); ++fcIt)
    (*fcIt)->setItinIndex(itinIndex);

  ExcItin* excItin = exchangeItin().front();
  excItin->setExcItinIndex(itinIndex, newItin().size());

  if (getTrxType() == PricingTrx::MIP_TRX)
  {
    int16_t pnrIndex = 1;
    for (TravelSeg* segment : curNewItin()->travelSeg())
      segment->pnrSegment() = pnrIndex++;
  }
}

uint16_t
BaseExchangeTrx::getItinPos(const Itin* itin) const
{
  if (!_motherItinIndex.empty())
  {
    if (_motherItinIndex.find(itin) == _motherItinIndex.end())
      throw ErrorResponseException(ErrorResponseException::REISSUE_RULES_FAIL);

    return _motherItinIndex.find(itin)->second;
  }
  else
  {
    std::vector<Itin*>::const_iterator itinIter = newItin().begin();
    for (uint16_t itinIndex = 0; itinIter != newItin().end(); ++itinIter, ++itinIndex)
      if (*itinIter == itin)
        return itinIndex;
  }

  throw ErrorResponseException(ErrorResponseException::REISSUE_RULES_FAIL);
}

void
BaseExchangeTrx::setBsrRoeDate(const Indicator& ind)
{
  if (ind == EXCHANGE)
  {
    applyCurrentBsrRoeDate();
  }
  else if (ind == REISSUE)
  {
    setHistoricalBsrRoeDate();
    applyHistoricalBsrRoeDate();
  }
}

void
BaseExchangeTrx::setHistoricalBsrRoeDate()
{
  _historicalBsrRoeDate = _originalTktIssueDT;
  if (!previousExchangeDT().isEmptyDate())
    _historicalBsrRoeDate = _previousExchangeDT;
}

bool
BaseExchangeTrx::isExcRtw() const
{
  return static_cast<const RexPricingOptions*>(getOptions())->isExcTaggedAsRtw();
}
} // tse namespace
