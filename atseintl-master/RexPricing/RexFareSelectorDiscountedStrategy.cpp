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

#include "RexPricing/RexFareSelectorDiscountedStrategy.h"

#include "DataModel/FareCompInfo.h"
#include "RexPricing/PreSelectedFaresStore.h"
#include "RexPricing/RexFareSelectorStrategyAlgo.h"
#include "RexPricing/RexFareSelectorStrategyPredicate.h"

#include <cmath>

namespace tse
{

bool
RexFareSelectorDiscountedStrategy::select(FareCompInfo& fc,
                                          Iterator begin,
                                          Iterator end,
                                          std::vector<PaxTypeFareWrapper>& selected) const
{
  static const double MAYBE_DISCOUNT_VARIANCE_SQ[] = { 1.0 };

  return sequentialSelect<CheckVarianceAmount>(
      begin, end, fc.getTktBaseFareCalcAmt(), MAYBE_DISCOUNT_VARIANCE_SQ, selected);
}

bool
RexFareSelectorDiscountedStrategy::getPreSelectedFares(const FareCompInfo& fc,
                                                       RexDateSeqStatus status,
                                                       std::vector<PaxTypeFareWrapper>& preSelected)
    const
{
  return _store.restore(fc, status, preSelected);
}

} // tse
