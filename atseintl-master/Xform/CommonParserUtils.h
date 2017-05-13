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

#pragma once


#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/MaxPenaltyInfo.h"

#include <boost/optional.hpp>

namespace tse {
struct CalcTotals;
class Discounts;
class FarePath;
class PricingTrx;
class RexBaseRequest;
class ExcItin;

class CommonParserUtils
{
public:
  static MaxPenaltyInfo*
  validateMaxPenaltyInput(const PricingTrx& trx,
                          boost::optional<smp::Mode> mpo,
                          boost::optional<MaxPenaltyInfo::Filter> changeFilter,
                          boost::optional<MaxPenaltyInfo::Filter> refundFilter);

  static MaxPenaltyInfo::Filter
  validateMaxPenaltyFilter(const PricingTrx& trx,
                           smp::RecordApplication abd,
                           boost::optional<smp::ChangeQuery> mpi,
                           boost::optional<MoneyAmount> mpa,
                           boost::optional<CurrencyCode> mpc);

  static bool isNonRefundable(const FarePath& farePath);

  static Money selectNonRefundableAmount(PricingTrx& pricingTrx, const FarePath& farePath);

  static Money nonRefundableAmount(PricingTrx& pricingTrx,
                                   const CalcTotals& calcTotals,
                                   bool nonRefundable);

  static void
  addZeroDiscountForSegsWithNoDiscountIfReqHasDiscount(Discounts& discounts,
                                                       const std::vector<TravelSeg*>& travelSegs,
                                                       const bool isMip);

  static void
  initPricingDiscountForSegsWithNoDiscountIfReqHasDiscount(RexBaseRequest& request, ExcItin* itin);
  static void
  initShoppingDiscountForSegsWithNoDiscountIfReqHasDiscount(RexBaseRequest& request, ExcItin* itin);

private:
  static void initDiscountForSegsWithNoDiscountIfReqHasDiscount(
      RexBaseRequest& request,
      ExcItin* itin,
      std::function<int16_t(const TravelSeg*)>& segmentValueGetterFunc);
};

} /* namespace tse */
