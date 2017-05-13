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

#include "RexPricing/RexFareSelectorVCTRStrategy.h"

#include "Common/VCTR.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/PaxTypeFare.h"
#include "RexPricing/RexFareSelectorStrategyAlgo.h"
#include "RexPricing/RexFareSelectorStrategyPredicate.h"

#include <cmath>

namespace tse
{
namespace
{
template <typename TariffSelector>
class TCheckVCTR : public std::unary_function<PaxTypeFareWrapper, bool>
{
public:
  explicit TCheckVCTR(const VCTR& vctr) : _vctr(vctr) {}

  bool operator()(const PaxTypeFareWrapper& wrp) const
  {
    return (wrp.get()->vendor() == _vctr.vendor() && wrp.get()->carrier() == _vctr.carrier() &&
            _tariffSelector(*wrp.get(), _vctr.tariff()) && wrp.get()->ruleNumber() == _vctr.rule());
  }

protected:
  const VCTR& _vctr;
  TariffSelector _tariffSelector;
};

class BaseTariff : public std::binary_function<PaxTypeFare, TariffNumber, bool>
{
public:
  bool operator()(const PaxTypeFare& ptf, const TariffNumber& tariff) const
  {
    return ptf.fareTariff() == tariff;
  }
};

class TcrTariff : public std::binary_function<PaxTypeFare, TariffNumber, bool>
{
public:
  bool operator()(const PaxTypeFare& ptf, const TariffNumber& tariff) const
  {
    return ptf.tcrRuleTariff() == tariff;
  }
};

} // namespace

bool
RexFareSelectorVCTRStrategy::select(FareCompInfo& fc,
                                    Iterator begin,
                                    Iterator end,
                                    std::vector<PaxTypeFareWrapper>& selected) const
{
  return selectStep(TCheckVCTR<BaseTariff>(fc.VCTR()), fc, begin, end, selected) ||
         selectStep(TCheckVCTR<TcrTariff>(fc.VCTR()), fc, begin, end, selected);
}

template <typename VctrSelector>
bool
RexFareSelectorVCTRStrategy::selectStep(VctrSelector vctrSelector,
                                        FareCompInfo& fc,
                                        Iterator begin,
                                        Iterator end,
                                        std::vector<PaxTypeFareWrapper>& selected) const
{
  end = stablePartition(begin, end, vctrSelector);

  Iterator mid =
      stablePartition(begin, end, CheckAmount(fc.getTktBaseFareCalcAmt(), getTolerance()));
  if (begin != mid)
  {
    selected.assign(begin, mid);
    return true;
  }

  if (mid != end)
  {
    static const double VCTR_VARIANCE_SQ[] = { 0.09, 0.33, 0.66, 1.0 };

    if (sequentialSelect<CheckVarianceAmount>(
            mid, end, fc.getTktBaseFareCalcAmt(), VCTR_VARIANCE_SQ, selected))
      return true;

    if (fc.hipIncluded())
    {
      selected.assign(mid, end);
      return true;
    }
  }

  return false;
}

} // tse
