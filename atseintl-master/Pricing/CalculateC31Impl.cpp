#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "DataModel/ProcessTagInfo.h"
#include "DBAccess/ReissueSequence.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "Pricing/CalculateC31Impl.h"
#include "RexPricing/ReissuePenaltyCalculator.h"
#include "Rules/VoluntaryChanges.h"

namespace tse
{
BOOST_TRIBOOL_THIRD_STATE(notOnlyForCancel)

CalculateC31Impl::CalculateC31Impl(FcFeesVec fees, MaxPenaltyResponse::Fee& fee,
                                   State state, smp::RecordApplication departure)
  : _state(state), _fees(fees), _fee(fee), _departureInd(departure)

{
  _partialResults.resize(fees.size());
}

bool
CalculateC31Impl::allR3Missing()
{
  if (_fees.size() != _state._ptfPuAmounts.size())
    return true;

  for (const FcFees& calculation : _fees)
    if (calculation.empty())
      return true;

  return false;
}

const FcFee*
CalculateC31Impl::findHighestFee()
{
  const FcFee* highestFee = nullptr;

  for (const FcFees& calculation : _fees)
  {
    for (const FcFee& fee : calculation)
      if (!highestFee || std::get<0>(*highestFee) < std::get<0>(fee))
        highestFee = &fee;
  }

  return highestFee;
}

tribool
CalculateC31Impl::detectCancelAndStartOverProcessingTag(const VoluntaryChangesInfoW& c31Rec3)
{
  const std::vector<ReissueSequence*>& t988Seqs =
      _state._trx->dataHandle().getReissue(c31Rec3.vendor(),
                                           c31Rec3.reissueTblItemNo(),
                                           _state._trx->dataHandle().ticketDate(),
                                           _state._trx->dataHandle().ticketDate());

  auto cancelAndStartOver = [](const ReissueSequence* t988Seq)
                            {
                              return t988Seq->processingInd() == ProcessTag::CANCEL_AND_START_OVER;
                            };

  uint32_t cancelCount = std::count_if(t988Seqs.begin(), t988Seqs.end(), cancelAndStartOver);

  if (cancelCount == 0)
  {
    return false;
  }
  else if (cancelCount == t988Seqs.size())
  {
    return true;
  }
  else
  {
    return notOnlyForCancel;
  }
}

void
CalculateC31Impl::markNonchangeable()
{
  uint32_t fareComponentIndex = 0;
  _fee._non = false;

  for (FcFees& calculation : _fees)
  {
    FcFees subsetForFurtherProcessing;
    for (const FcFee& fee : calculation)
    {
      if (considerLater(*std::get<2>(fee), fareComponentIndex))
        subsetForFurtherProcessing.push_back(fee);
    }
    calculation.swap(subsetForFurtherProcessing);
    ++fareComponentIndex;
  }
}

bool
CalculateC31Impl::considerLater(const VoluntaryChangesInfoW& c31Rec3,
                                uint32_t fareComponentIndex)
{
  if (c31Rec3.waiverTblItemNo() != 0)
  {
    return false;
  }

  // returning false will prevent to calculate penalty in regular way from this R3
  tribool status = detectCancelAndStartOverProcessingTag(c31Rec3);
  if (notOnlyForCancel(status))
  {
    _fee._non = true;
  }
  else if(status)
  {
    _fee._non = true;
    return false;
  }

  if (c31Rec3.changeInd() == VoluntaryChanges::CHG_IND_J)
  {
    if (_state._sameCarrier)
    {
      _fee._non = true;
      return false;
    }
    else
      return convertAndStore(_state._ptfPuAmounts[fareComponentIndex].second, fareComponentIndex);
  }

  if (c31Rec3.changeInd() == VoluntaryChanges::CHG_IND_P)
  {
    _fee._non = true;
    return convertAndStore(_state._ptfPuAmounts[fareComponentIndex].second, fareComponentIndex);
  }

  if (c31Rec3.changeInd() == VoluntaryChanges::NOT_PERMITTED)
  {
    _fee._non = true;
    return false;
  }

  return true;
}

bool
CalculateC31Impl::convertAndStore(const MoneyAmount& source, uint32_t fareComponentIndex)
{
  MoneyAmount convertedAmt = 0.0;
  if (_state._targetCurrency == _state._solutionCurrency)
  {
    convertedAmt = source;
  }
  else
  {
    convertedAmt = CurrencyUtil::convertMoneyAmount(source, _state._solutionCurrency,
                                                    _state._targetCurrency, *_state._trx);
  }

  if (convertedAmt > _partialResults[fareComponentIndex])
    _partialResults[fareComponentIndex] = convertedAmt;

  return false;
}

void
CalculateC31Impl::calculate()
{
  if (allR3Missing())
  {
    _fee._non = true;
    return;
  }

  MoneyAmount maximum = NOT_INITIALIZED;
  std::vector<MoneyAmount> maxFeeWith3FromAllFCs;
  bool hasFeeAppl3 = false;
  findHighestOfAppl3(maxFeeWith3FromAllFCs, hasFeeAppl3);
  markNonchangeable();

  if (hasFeeAppl3 && findHighestOnEveryFC())
  {
    for (uint32_t i = 0; i < maxFeeWith3FromAllFCs.size(); ++i)
    {
      if (maxFeeWith3FromAllFCs[i] != NOT_INITIALIZED)
      {
        std::vector<MoneyAmount> localMaxPerFC = _partialResults;
        localMaxPerFC[i] = maxFeeWith3FromAllFCs[i];
        MoneyAmount localMax = std::accumulate(localMaxPerFC.begin(), localMaxPerFC.end(), 0.0,
                                               [](MoneyAmount sum, const MoneyAmount& fee)
                                               {
                                                  return sum + fee;
                                               });
        if (maximum < localMax)
        {
          maximum = localMax;
        }
      }
    }
  }
  else
  {
    const FcFee* highestFee = findHighestFee();
    if (highestFee)
    {
      maximum = std::get<0>(*highestFee);
    }
  }

  if (maximum == NOT_INITIALIZED &&
      std::any_of(_partialResults.begin(), _partialResults.end(),
                  [](const MoneyAmount& amt) { return amt != 0.0; }))
  {
    maximum = *std::max_element(_partialResults.cbegin(), _partialResults.cend());
  }

  if (maximum != NOT_INITIALIZED)
  {
    _fee._fee = Money(maximum, _state._targetCurrency);
  }
  else
  {
    _fee._non = true;
  }
}

void
CalculateC31Impl::findHighestOfAppl3(std::vector<MoneyAmount>& feeOnFC, bool& hasEachOfChangedFC)
{
  for (FcFees& fcFees : _fees)
  {
    MoneyAmount match = NOT_INITIALIZED;
    for (FcFee& fee : fcFees)
    {
      if (std::get<1>(fee) == ReissuePenaltyCalculator::EACH_OF_CHANGED_FC &&
         match < std::get<0>(fee))
      {
        match = std::get<0>(fee);
        hasEachOfChangedFC = true;
      }
    }
    feeOnFC.push_back(match);
  }
}

bool
CalculateC31Impl::findHighestOnEveryFC()
{
  uint32_t fcIndex = 0;
  bool hasAnyFees = false;
  for (FcFees& fcFees : _fees)
  {
    if (!fcFees.empty())
    {
      FcFee match = *std::max_element(fcFees.begin(), fcFees.end(),
                                      [](const FcFee& a, const FcFee& b)
                                      {
                                         return std::get<0>(a) < std::get<0>(b);
                                      });

      _partialResults[fcIndex] = std::get<0>(match);
      hasAnyFees = true;
    }
    ++fcIndex;
  }
  return hasAnyFees;
}

} /* namespace tse */
