//-------------------------------------------------------------------
//
//  File:        EnhancedRefundDiscountApplier.h
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

#pragma once

#include "RexPricing/RefundDiscountApplier.h"

namespace tse
{

class EnhancedRefundDiscountApplier : public RefundDiscountApplier
{
public:
  explicit EnhancedRefundDiscountApplier(const PaxType& pt)
    : RefundDiscountApplier(pt), _infantWithSeat(false), _infantWithoutSeat(false)
  {
  }

  void assign(const PaxType& pt);

  static const EnhancedRefundDiscountApplier* create(DataHandle& dh, const PaxType& pt);

protected:
  EnhancedRefundDiscountApplier() : _infantWithSeat(false), _infantWithoutSeat(false) {}

  virtual void insertDiscountType(Indicator tag, std::vector<Indicator>& tags) const override;

  using RefundDiscountApplier::applyDiscount;

  virtual bool applyDiscount(Money& fee,
                             const std::vector<Indicator>& discountTags,
                             const DiscountInfo* discount) const override;
  const std::vector<Indicator> adjustDiscountTags(const std::vector<Indicator>& discountTags) const;

  friend class DataHandle;
  friend class EnhancedRefundDiscountApplierTest;

private:
  bool _infantWithSeat;
  bool _infantWithoutSeat;
};
}
