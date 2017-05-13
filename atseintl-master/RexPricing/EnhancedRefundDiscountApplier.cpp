//-------------------------------------------------------------------
//
//  File:        EnhancedRefundDiscountApplier.cpp
//  Created:     May 07, 2010
//
//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "RexPricing/EnhancedRefundDiscountApplier.h"

#include "Common/Logger.h"
#include "DataModel/PaxType.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/PaxTypeInfo.h"

namespace tse
{
static Logger
logger("atseintl.RexPricing.EnhancedRefundDiscountApplier");

void
EnhancedRefundDiscountApplier::insertDiscountType(Indicator tag, std::vector<Indicator>& tags) const
{
  switch (tag)
  {
  case INFANT_WITH_SEAT_CAT19:
  case INFANT_WITHOUT_SEAT_CAT19:
  case INFANT_WITHOUT_SEAT_NOFEE:
    tags.push_back(tag);
    return;
  default:
    ;
  }
  RefundDiscountApplier::insertDiscountType(tag, tags);
}

void
EnhancedRefundDiscountApplier::assign(const PaxType& pt)
{
  if (pt.paxTypeInfo().isInfant() && pt.paxTypeInfo().numberSeatsReq() > 0)
    _infantWithSeat = true;

  if (pt.paxTypeInfo().infantInd() == YES && !pt.paxTypeInfo().numberSeatsReq())
    _infantWithoutSeat = true;

  RefundDiscountApplier::assign(pt);
}

const EnhancedRefundDiscountApplier*
EnhancedRefundDiscountApplier::create(DataHandle& dh, const PaxType& pt)
{
  LOG4CXX_DEBUG(logger,
                "You are using a deprecated class (EnhancedRefundDiscountApplier), use "
                "RefundDiscountApplier instead.");
  EnhancedRefundDiscountApplier* app;
  dh.get(app);
  app->assign(pt);
  return app;
}

bool
EnhancedRefundDiscountApplier::applyDiscount(Money& fee,
                                             const std::vector<Indicator>& discountTags,
                                             const DiscountInfo* discount) const
{
  for (const auto discountTag : discountTags)
  {
    switch (discountTag)
    {
    case INFANT_WITH_SEAT_CAT19: // 0
      if (_infantWithSeat && isDiscount(discount, 19))
        return applyDiscount(fee, *discount);
      break;

    case INFANT_WITHOUT_SEAT: // 1
      if ((_infantWithSeat || _infantWithoutSeat) && isDiscount(discount, 19))
        return applyDiscount(fee, *discount);
      break;

    case INFANT_WITH_SEAT: // 2
      if (_infantWithoutSeat)
      {
        fee.value() = 0.0;
        return true;
      }
      if (_infantWithSeat && isDiscount(discount, 19))
        return applyDiscount(fee, *discount);
      break;

    case INFANT_WITHOUT_SEAT_CAT19: // 8
      if (_infantWithoutSeat && isDiscount(discount, 19))
        return applyDiscount(fee, *discount);
      break;

    case INFANT_WITHOUT_SEAT_NOFEE: // 9
      if (_infantWithoutSeat)
      {
        fee.value() = 0.0;
        return true;
      }
      break;

    default:
      ;
    }
  }

  return RefundDiscountApplier::applyDiscount(fee, adjustDiscountTags(discountTags), discount);
}

const std::vector<Indicator>
EnhancedRefundDiscountApplier::adjustDiscountTags(const std::vector<Indicator>& discountTags) const
{
  std::vector<Indicator> adjustedDiscountTags;
  std::remove_copy_if(discountTags.begin(),
                      discountTags.end(),
                      std::back_inserter(adjustedDiscountTags),
                      std::bind2nd(std::less<Indicator>(), 51));

  return adjustedDiscountTags;
}
}
