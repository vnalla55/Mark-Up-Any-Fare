//-------------------------------------------------------------------
//
//  Copyright Sabre 2010
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Rules/DepartureDateValidator.h"

#include "Common/DateTime.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/ReissueSequence.h"

#include <string>

namespace tse
{

namespace
{

const Indicator BYPASS_ADVANCE_PURCHASE_NO = 'N';
const Indicator IND_BLANK = ' ';
const std::string BLANK;
const int VAL_BLANK = 0;
const Indicator FARECOMPONENT = 'F';
const Indicator PRICINGUNIT = ' ';
const Indicator JOURNEY = 'J';
const Indicator NEWTKTDATE = ' ';

inline bool
hasAdvanceTicketingBlank(const ReissueSequenceW& seq)
{
  return (seq.ticketResvInd() == IND_BLANK && seq.reissueTOD() < 1 &&
          seq.reissuePeriod() == BLANK && seq.reissueUnit() == BLANK &&
          seq.optionInd() == IND_BLANK && seq.departure() == VAL_BLANK &&
          seq.departureUnit() == IND_BLANK && seq.departureInd() == IND_BLANK);
}

inline bool
hasDepartureDateFromNewTicketForMeasurePoint(const ReissueSequenceW& seq, Indicator measurePoint)
{
  return (seq.fromAdvResInd() == NEWTKTDATE && seq.toAdvResInd() == measurePoint);
}

inline bool
getFirstMeasurePoint(const std::vector<ProcessTagPermutation*>& permutations,
                     Indicator& measurePoint)
{
  if (permutations.empty() || permutations.front()->processTags().empty() ||
      !permutations.front()->processTags().front()->reissueSequence()->orig())
    return false;

  measurePoint = permutations.front()->processTags().front()->reissueSequence()->toAdvResInd();

  return true;
}

} // namespace

void
DepartureDateValidator::processPermutations()
{
  _isValid = (_trx && _trx->getOptions()->AdvancePurchaseOption() == BYPASS_ADVANCE_PURCHASE_NO &&
              hasSameMeasurePoint(_trx->processTagPermutations(), _toAdvResMeasurePoint));
}

bool
DepartureDateValidator::hasSameMeasurePoint(const Permutations& permutations,
                                            Indicator& measurePoint) const
{
  typedef std::vector<ProcessTagPermutation*>::const_iterator PermIt;
  typedef std::vector<ProcessTagInfo*> Tags;
  typedef Tags::const_iterator TagIt;

  if (!getFirstMeasurePoint(permutations, measurePoint))
    return false;

  for (PermIt i = permutations.begin(); i != permutations.end(); ++i)
  {
    const Tags& tags = (*i)->processTags();
    if (tags.empty())
      return false;

    for (TagIt j = tags.begin(); j != tags.end(); ++j)
    {
      const ReissueSequenceW& seq = *(*j)->reissueSequence();
      if (!seq.orig() || !hasAdvanceTicketingBlank(seq) ||
          !hasDepartureDateFromNewTicketForMeasurePoint(seq, measurePoint))
        return false;
    }
  }
  return true;
}

const DateTime&
DepartureDateValidator::getDepartureDate(const TravelSeg& ts) const
{
  if (_trx == nullptr || _trx->trxPhase() != RexBaseTrx::PRICE_NEWITIN_PHASE || !_isValid)
    return DateTime::emptyDate();

  return ts.unflown() ? _trx->currentTicketingDT() : _trx->originalTktIssueDT();
}

const DateTime&
DepartureDateValidator::getDepartureDate(const FareMarket& fm, const Itin& itin) const
{
  switch (_toAdvResMeasurePoint)
  {
  case FARECOMPONENT:
    return getDepartureDate(*fm.travelSeg().front());
  case JOURNEY:
    return getDepartureDate(*itin.travelSeg().front());
  }
  return DateTime::emptyDate();
}

const DateTime&
DepartureDateValidator::getDepartureDate(const PricingUnit& pu, const Itin& itin) const
{
  switch (_toAdvResMeasurePoint)
  {
  case PRICINGUNIT:
    return getDepartureDate(
        *pu.fareUsage().front()->paxTypeFare()->fareMarket()->travelSeg().front());
  case JOURNEY:
    return getDepartureDate(*itin.travelSeg().front());
  }
  return DateTime::emptyDate();
}

} // tse
