#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "Pricing/Cat31PenaltyEstimator.h"
#include "RexPricing/ReissuePenaltyCalculator.h"

namespace tse
{
Cat31PenaltyEstimator::Cat31PenaltyEstimator(const CurrencyCode& targetCurrency,
                                             const FarePath& farePath,
                                             PricingTrx& trx,
                                             DiagManager diag)
  : _diag(std::move(diag))
{
  _state._targetCurrency = targetCurrency;
  _state._solutionCurrency = farePath.getCalculationCurrency();
  _state._trx = &trx;
  completeState(farePath);
}

MaxPenaltyResponse::Fees
Cat31PenaltyEstimator::estimate(const FcFeesVec& fees, const smp::RecordApplication& departureInd)
{
  if (departureInd & smp::BEFORE)
    CalculateC31Impl(getApplicableRecords(fees, smp::BEFORE),
                     _result._before, _state, smp::BEFORE).calculate();

  if (departureInd & smp::AFTER)
      CalculateC31Impl(getApplicableRecords(fees, smp::AFTER),
                       _result._after, _state, smp::AFTER).calculate();

  print(fees, departureInd);

  return std::move(_result);
}

void
Cat31PenaltyEstimator::print(const FcFeesVec& fees, const smp::RecordApplication& departureInd)
{
  if (!_state._trx->diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, "MAXPEN"))
   return;

  _diag << " CAT31 CHANGE FEE CALCULATIONS USING:\n";

  FcFees::size_type fareComponentIndex = 0;
  for (const FcFees& calculation : fees)
  {
    for (const FcFee& fee : calculation)
    {
      if (smp::printRecord3(*_state._trx, *std::get<2>(fee), departureInd, _diag))
      {
        Money fuAmount(0, _state._targetCurrency);
        Money puAmount = fuAmount;

        if (_state._targetCurrency != _state._solutionCurrency)
        {
          fuAmount = Money(
             CurrencyUtil::convertMoneyAmount(_state._ptfPuAmounts[fareComponentIndex].first,
                                              _state._solutionCurrency,
                                              _state._targetCurrency,
                                              *_state._trx),
             _state._targetCurrency);

          puAmount = Money(
             CurrencyUtil::convertMoneyAmount(_state._ptfPuAmounts[fareComponentIndex].second,
                                              _state._solutionCurrency,
                                              _state._targetCurrency,
                                              *_state._trx),
             _state._targetCurrency);
        }
        else
        {
          fuAmount = Money(_state._ptfPuAmounts[fareComponentIndex].first,
                           _state._solutionCurrency);
          puAmount = Money(_state._ptfPuAmounts[fareComponentIndex].first,
                           _state._solutionCurrency);
        }

        _diag << " FCI: " << (fareComponentIndex + 1)
             << " AMOUNT: " << Money(std::get<0>(fee), _state._targetCurrency)
             << " FARE AMT: " << fuAmount << " PU AMT: " << puAmount << '\n';
      }
    }
    ++fareComponentIndex;
  }

  smp::resultDiagnostic(_diag, _result, departureInd, "CHANGE", "CAT31");
}

void
Cat31PenaltyEstimator::completeState(const FarePath& farePath)
{
  _state._sameCarrier = true;
  const CarrierCode& carrier = farePath.pricingUnit()
                                  .front()
                                  ->fareUsage()
                                  .front()
                                  ->paxTypeFare()
                                  ->fareMarket()
                                  ->governingCarrier();

  for (const PricingUnit* pu : farePath.pricingUnit())
    for (const FareUsage* fu : pu->fareUsage())
    {
      _state._ptfPuAmounts.push_back(
         std::make_pair(fu->totalFareAmount(), pu->getTotalPuNucAmount()));
      if (carrier != fu->paxTypeFare()->fareMarket()->governingCarrier())
        _state._sameCarrier = false;
    }

  _state._isDirectOW = farePath.itin()->travelSeg().size() == 1;
  _state._fcCountInFirstPU = farePath.pricingUnit().front()->fareUsage().size();
}

FcFeesVec
Cat31PenaltyEstimator::getApplicableRecords(const FcFeesVec& fees,
                                            smp::RecordApplication application)
{
  FcFeesVec selected;

  if (fees.size() == 1 && application == smp::AFTER && _state._isDirectOW)
  {
    return selected;
  }

  uint32_t fcNumber = 1;
  for (const FcFees& fcFees : fees)
  {
    FcFees applicableFcFees;
    bool isFirstFC = (fcNumber == 1);
    bool isFirstPU = (fcNumber <= _state._fcCountInFirstPU);
    for (const FcFee& fcFee : fcFees)
    {
      smp::RecordApplication recAppl = smp::getRecordApplication(*std::get<2>(fcFee), application, isFirstFC, isFirstPU);

      if (recAppl & application)
      {
        applicableFcFees.push_back(fcFee);
      }
    }
    ++fcNumber;
    selected.push_back(std::move(applicableFcFees));
  }

  return selected;
}

} //tse
