#pragma once

#include "Common/TseCodeTypes.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/CalculateC31Impl.h"

namespace tse
{
class FarePath;
class PricingTrx;

class Cat31PenaltyEstimator
{
public:
  using State = CalculateC31Impl::State;

  Cat31PenaltyEstimator(const CurrencyCode& targetCurrency,
                        const FarePath& farePath,
                        PricingTrx& trx,
                        DiagManager diag);
  MaxPenaltyResponse::Fees
  estimate(const FcFeesVec& fees, const smp::RecordApplication& departureInd);

private:
  void print(const FcFeesVec& fees, const smp::RecordApplication& departureInd);
  void completeState(const FarePath& farePath);
  FcFeesVec getApplicableRecords(const FcFeesVec& fees, smp::RecordApplication application);

  DiagManager _diag;
  State _state;
  MaxPenaltyResponse::Fees _result;
};
}//tse

