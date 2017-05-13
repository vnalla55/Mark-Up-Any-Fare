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

#include "RexPricing/RexFareSelectorBasicStrategy.h"

#include "Common/FallbackUtil.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/RexPricingTrx.h"
#include "RexPricing/PreSelectedFaresStore.h"
#include "RexPricing/RexFareSelectorStrategyAlgo.h"
#include "RexPricing/RexFareSelectorStrategyPredicate.h"

namespace tse
{
FALLBACK_DECL(exactMatchOfDiscountedFares);

namespace
{
class CheckDiscountedAmount
{
public:
  CheckDiscountedAmount(const MoneyAmount& amount) : _amount(amount + EPSILON) {}

  bool operator()(const PaxTypeFareWrapper& wrp) const { return wrp.getAmount() > _amount; }

private:
  MoneyAmount _amount;
};

} // namespace

bool
RexFareSelectorBasicStrategy::select(FareCompInfo& fc,
                                     Iterator begin,
                                     Iterator end,
                                     std::vector<PaxTypeFareWrapper>& selected) const
{
  if (fallback::exactMatchOfDiscountedFares(&_trx))
  {
    // remove FareCompInfo::discounted() methods on the fallback removal
    if (fc.discounted())
    {
      static const double DISCOUNT_VARIANCE_SQ[] = { 1.0 };

      std::vector<PaxTypeFareWrapper> discounted;
      copyIf(begin,
             end,
             std::back_inserter(discounted),
             CheckDiscountedAmount(fc.getTktBaseFareCalcAmt()));
      return sequentialSelect<CheckVarianceAmount>(discounted.begin(),
                                                   discounted.end(),
                                                   fc.getTktBaseFareCalcAmt(),
                                                   DISCOUNT_VARIANCE_SQ,
                                                   selected);
    }
  }

  copyIf(begin,
         end,
         std::back_inserter(selected),
         CheckAmount(fc.getTktBaseFareCalcAmt(), getTolerance()));
  return !selected.empty();
}

void
RexFareSelectorBasicStrategy::setPreSelectedFares(
    const FareCompInfo& fc,
    RexDateSeqStatus status,
    const std::vector<PaxTypeFareWrapper>& preSelected) const
{
  _store.store(fc, status, preSelected);
}

} // tse
