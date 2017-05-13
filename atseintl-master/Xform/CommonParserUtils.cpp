//----------------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "Xform/CommonParserUtils.h"

#include <boost/optional.hpp>

#include "Common/Config/ConfigMan.h"
#include "Common/CurrencyUtil.h"
#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexBaseRequest.h"
#include "FareCalc/CalcTotals.h"

namespace tse {
FIXEDFALLBACK_DECL(validateAllCat16Records);
FIXEDFALLBACK_DECL(excDiscountSegsInit);
FIXEDFALLBACK_DECL(excDiscountAmountGroupInitFix);

MaxPenaltyInfo*
CommonParserUtils::validateMaxPenaltyInput(const PricingTrx& trx,
                                           boost::optional<smp::Mode> mpo,
                                           boost::optional<MaxPenaltyInfo::Filter> changeFilter,
                                           boost::optional<MaxPenaltyInfo::Filter> refundFilter)
{
  if ((mpo || changeFilter || refundFilter) &&
      (TrxUtil::getMaxPenaltyFailedFaresThreshold(trx) > 0))
  {
    if (trx.getOptions() && trx.getOptions()->noPenalties() && (!mpo || mpo.get() != smp::INFO))
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "UNABLE TO COMBINE PENALTY/REFUND QUALIFIERS");

    MaxPenaltyInfo* maxPenalty = trx.dataHandle().create<MaxPenaltyInfo>();
    maxPenalty->_changeFilter =
        changeFilter ? changeFilter.get() : MaxPenaltyInfo::Filter{smp::BOTH, {}, {}};
    maxPenalty->_refundFilter =
        refundFilter ? refundFilter.get() : MaxPenaltyInfo::Filter{smp::BOTH, {}, {}};

    if (changeFilter.is_initialized() != refundFilter.is_initialized() || !mpo.is_initialized())
    {
      maxPenalty->_mode = smp::AND;
    }
    else
    {
      maxPenalty->_mode = mpo.get();
    }

    return maxPenalty;
  }
  else
  {
    return nullptr;
  }
}

MaxPenaltyInfo::Filter
CommonParserUtils::validateMaxPenaltyFilter(const PricingTrx& trx,
                                            smp::RecordApplication abd,
                                            boost::optional<smp::ChangeQuery> mpi,
                                            boost::optional<MoneyAmount> mpa,
                                            boost::optional<CurrencyCode> mpc)
{
  boost::optional<Money> maxFee;
  if (mpa)
  {
    if (!mpc)
    {
      const Agent* ticketingAgent = trx.getRequest()->ticketingAgent();
      TSE_ASSERT(ticketingAgent);
      mpc = ticketingAgent->currencyCodeAgent();
    }
    else
      CurrencyUtil::validateCurrencyCode(trx, mpc.get());

    maxFee = Money(*mpa, *mpc);
  }

  return {abd, std::move(mpi), std::move(maxFee)};
}

bool
CommonParserUtils::isNonRefundable(const FarePath& farePath)
{
  for (const PricingUnit* pricingUnit : farePath.pricingUnit())
  {
    for (const FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      if (fallback::fixed::validateAllCat16Records())
      {
        // please remove also isAppendNR
        if (fareUsage->isAppendNR() &&
            (fareUsage->paxTypeFare()->penaltyRestInd() ==
             YES)) // PL#9760 Per Saranya the NR is in the penaltyRestInd CAT16 penalties.cpp
          return true;
      }

      if (fareUsage->isNonRefundable())
        return true;
    }
  }

  return false;
}

Money
CommonParserUtils::selectNonRefundableAmount(PricingTrx& pricingTrx, const FarePath& farePath)
{
  switch (pricingTrx.excTrxType())
  {
  case PricingTrx::AR_EXC_TRX:
    return farePath.getHigherNonRefundableAmount();
  case PricingTrx::AF_EXC_TRX:
    return static_cast<const RexBaseTrx&>(pricingTrx).exchangeItin().front()->getNonRefAmount();
  default:
    ;
  }

  return ExchangeUtil::convertCurrency(pricingTrx,
                                       farePath.getNonrefundableAmount(pricingTrx),
                                       farePath.baseFareCurrency(),
                                       farePath.itin()->useInternationalRounding());
}

Money
CommonParserUtils::nonRefundableAmount(PricingTrx& pricingTrx,
                                       const CalcTotals& calcTotals,
                                       bool nonRefundable)
{
  if (fallback::fixed::validateAllCat16Records())
  {
    return selectNonRefundableAmount(pricingTrx, *calcTotals.farePath);
  }
  else
  {
    if (nonRefundable)
    {
      return selectNonRefundableAmount(pricingTrx, *calcTotals.farePath);
    }
    else
    {
      return calcTotals.farePath->getNonrefundableAmountFromCat16(pricingTrx);
    }
  }
}

void
CommonParserUtils::addZeroDiscountForSegsWithNoDiscountIfReqHasDiscount(
    Discounts& discounts, const std::vector<TravelSeg*>& travelSegs, const bool isMip)
{
  if (discounts.isDAorPAentry())
  {
    bool needIncreaseGroup = true;
    size_t nextGrp = discounts.getAmounts().size();
    for (const TravelSeg* travelSeg : travelSegs)
    {
      if (travelSeg->isAir() && discounts.getAmount(travelSeg->segmentOrder()) == nullptr)
      {
        if (needIncreaseGroup)
          ++nextGrp;
        discounts.addAmount(
            nextGrp, travelSeg->segmentOrder(), 0.0, discounts.getAmounts().front().currencyCode);
        needIncreaseGroup = false;
      }
      else
      {
        needIncreaseGroup = true;
      }
    }
  }

  if (discounts.isDPorPPentry())
  {
    for (const TravelSeg* travelSeg : travelSegs)
    {
      if (travelSeg->isAir() &&
          discounts.getPercentage(travelSeg->segmentOrder(), isMip) == nullptr)
      {
        discounts.addPercentage(travelSeg->segmentOrder(), 0.0);
      }
    }
  }
}
void
CommonParserUtils::initShoppingDiscountForSegsWithNoDiscountIfReqHasDiscount(
    RexBaseRequest& request, ExcItin* itin)
{
  std::function<int16_t(const TravelSeg*)> segmentValueGetterFunc = [&](const TravelSeg* seg)
  { return seg->pnrSegment(); };
  initDiscountForSegsWithNoDiscountIfReqHasDiscount(request, itin, segmentValueGetterFunc);
}

void
CommonParserUtils::initPricingDiscountForSegsWithNoDiscountIfReqHasDiscount(RexBaseRequest& request,
                                                                            ExcItin* itin)
{
  std::function<int16_t(const TravelSeg*)> segmentValueGetterFunc = [&](const TravelSeg* seg)
  { return seg->segmentOrder(); };
  initDiscountForSegsWithNoDiscountIfReqHasDiscount(request, itin, segmentValueGetterFunc);
}

void
CommonParserUtils::initDiscountForSegsWithNoDiscountIfReqHasDiscount(
    RexBaseRequest& request,
    ExcItin* itin,
    std::function<int16_t(const TravelSeg*)>& segmentValueGetterFunc)
{
  Discounts& discounts = request.excDiscounts();
  const bool isAmount =
      fallback::fixed::excDiscountSegsInit() ? discounts.isPAEntry() : discounts.isDAorPAentry();
  const bool isPercent =
      fallback::fixed::excDiscountSegsInit() ? discounts.isPPEntry() : discounts.isDPorPPentry();

  if ((!isAmount && !isPercent) || !itin)
  {
    return;
  }

  for (FareCompInfo* fc : itin->fareComponent())
  {
    if (isAmount)
    {
      auto discountAmtPos =
          std::find_if(fc->fareMarket()->travelSeg().begin(),
                       fc->fareMarket()->travelSeg().end(),
                       [&](TravelSeg* seg)
                       { return discounts.getAmount(segmentValueGetterFunc(seg)); });
      MoneyAmount markupAmt = 0;
      CurrencyCode currency = itin->calcCurrencyOverride().empty() ?
                              itin->calculationCurrency() :
                              itin->calcCurrencyOverride();
      std::size_t group = 0;

      if (discountAmtPos != fc->fareMarket()->travelSeg().end())
      {
        const DiscountAmount* da = discounts.getAmount(segmentValueGetterFunc(*discountAmtPos));
        markupAmt = da->amount;
        currency = da->currencyCode;
        if (!fallback::fixed::excDiscountAmountGroupInitFix())
        {
          group = std::distance(&discounts.getAmounts().front(), da) + 1;
        }
      }

      for (const TravelSeg* travelSeg : fc->fareMarket()->travelSeg())
      {
        if (discounts.getAmount(segmentValueGetterFunc(travelSeg)) == nullptr)
        {
          discounts.addAmount(group, segmentValueGetterFunc(travelSeg), markupAmt, currency);
        }
      }
    }
    if (isPercent)
    {
      auto discountPerPos =
          std::find_if(fc->fareMarket()->travelSeg().begin(),
                       fc->fareMarket()->travelSeg().end(),
                       [&](TravelSeg* seg)
                       { return discounts.getPercentage(segmentValueGetterFunc(seg), false); });

      Percent markupPercent = 0;
      if (discountPerPos != fc->fareMarket()->travelSeg().end())
      {
        markupPercent = *discounts.getPercentage(segmentValueGetterFunc(*discountPerPos), false);
      }

      for (const TravelSeg* travelSeg : fc->fareMarket()->travelSeg())
      {
        if (discounts.getPercentage(segmentValueGetterFunc(travelSeg), false) == nullptr)
        {
          discounts.addPercentage(segmentValueGetterFunc(travelSeg), markupPercent);
        }
      }
    }
  }
}

} /* namespace tse */
