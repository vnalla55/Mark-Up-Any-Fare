//-------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//-------------------------------------------------------------------

#include "RexPricing/RexFareSelectorHipStrategy.h"

#include "DataModel/FareCompInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/RexPricingTrx.h"
#include "RexPricing/RexFareSelectorStrategyAlgo.h"
#include "RexPricing/RexFareSelectorStrategyPredicate.h"

#include <cmath>
#include <functional>

namespace tse
{

namespace
{
class CheckHipAmount : public std::unary_function<PaxTypeFareWrapper, bool>
{
public:
  CheckHipAmount(const MoneyAmount& amount) : _amount(amount) {}

  bool operator()(const PaxTypeFareWrapper& wrp) const { return wrp.getAmount() < _amount; }

protected:
  MoneyAmount _amount;
};
}

bool
RexFareSelectorHipStrategy::select(FareCompInfo& fc,
                                   Iterator begin,
                                   Iterator end,
                                   std::vector<PaxTypeFareWrapper>& selected) const
{
  end = stablePartition(begin, end, CheckHipAmount(fc.getTktBaseFareCalcAmt()));
  static const double HIP_VARIANCE_SQ[] = { 1.0 };

  if (sequentialSelect<CheckVarianceAmount>(
          begin, end, fc.getTktBaseFareCalcAmt(), HIP_VARIANCE_SQ, selected))
    return true;

  if (begin != end)
  {
    selected.assign(begin, end);
    return true;
  }

  return false;
}

} // tse
