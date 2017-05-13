#pragma once

#include "DataModel/MaxPenaltyInfo.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include <boost/logic/tribool.hpp>

namespace tse
{

class VoluntaryChangesInfoW;
class PricingTrx;


namespace
{
using FcFee = std::tuple<MoneyAmount, Indicator, const VoluntaryChangesInfoW*>;
using FcFees = std::vector<FcFee>;
using FcFeesVec = std::vector<FcFees>;
using FeeIndexed = std::pair<const FcFee*, FcFees::size_type>;
using PtfPuAmounts = std::vector<std::pair<MoneyAmount, MoneyAmount>>;
using tribool = boost::logic::tribool;

}

class CalculateC31Impl
{
public:
  struct State
  {
    PricingTrx* _trx;
    CurrencyCode _targetCurrency;
    CurrencyCode _solutionCurrency;
    PtfPuAmounts _ptfPuAmounts;
    bool _sameCarrier;
    bool _isDirectOW;
    uint32_t _fcCountInFirstPU;
  };

  CalculateC31Impl(FcFeesVec fees, MaxPenaltyResponse::Fee& fee,
                   State state, smp::RecordApplication departure);
  void calculate();

private:
  bool allR3Missing();
  const FcFee* findHighestFee();
  tribool detectCancelAndStartOverProcessingTag(const VoluntaryChangesInfoW& c31Rec3);
  void markNonchangeable();
  bool considerLater(const VoluntaryChangesInfoW& c31Rec3, uint32_t fareComponentIndex);
  bool convertAndStore(const MoneyAmount& source, uint32_t fcIndex);
  void findHighestOfAppl3(std::vector<MoneyAmount>& feeOnFC, bool& hasEachOfChangedFC);
  bool findHighestOnEveryFC();

  State _state;

  FcFeesVec _fees;
  MaxPenaltyResponse::Fee& _fee;
  std::vector<MoneyAmount> _partialResults;
  smp::RecordApplication _departureInd;
  const MoneyAmount NOT_INITIALIZED = -1.0;
};

} //tse
