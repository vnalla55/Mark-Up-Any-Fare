//-------------------------------------------------------------------
//
//  File:        RefundDiscountApplier.h
//  Created:     October 28, 2009
//
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/Money.h"
#include "Common/SmallBitSet.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DiscountInfo.h"

#include <vector>

namespace tse
{

class PaxTypeFare;
class VoluntaryRefundsInfo;
class PaxType;
class DiscountInfo;
class DataHandle;

class RefundDiscountApplier
{
public:
  explicit RefundDiscountApplier(const PaxType& pt) : _discountStatus(pt) {}

  virtual ~RefundDiscountApplier() {}

  void assign(const PaxType& pt) { _discountStatus.assign(pt); }

  bool apply(Money& amt, const PaxTypeFare& ptf, const VoluntaryRefundsInfo& rec3) const;

  bool apply(Money& amt, const VoluntaryRefundsInfo& rec3) const;

  static const RefundDiscountApplier* create(DataHandle& dh, const PaxType& pt);

  virtual bool applyDiscount(Money& fee,
                             const std::vector<Indicator>& discountTags,
                             const DiscountInfo* discount) const;

protected:
  RefundDiscountApplier() {}

  std::vector<Indicator> getDiscountTypes(const VoluntaryRefundsInfo& rec3) const;

  virtual void insertDiscountType(Indicator tag, std::vector<Indicator>& tags) const;

  bool applyDiscount(Money& fee, const DiscountInfo& discount) const;

  const DiscountInfo* getDiscountInfo(const PaxTypeFare& fu) const;

  bool isDiscount(const DiscountInfo* discount, int cat) const
  {
    return discount && discount->category() == cat;
  }

  enum DisountTypeState
  { DT_InfantWithoutSeat = 1 << 2,
    DT_InfantWithSeat = 1 << 3,
    DT_Child = 1 << 4,
    DT_Youth = 1 << 5,
    DT_Senior = 1 << 6,
    DT_ChildOrInfant = 1 << 7,
    DT_InfantWithoutSeatRaw = 1 << 8,
    DT_InfantWithSeatRaw = 1 << 9 };

  class PaxTypeDiscountStatus : protected SmallBitSet<uint16_t, DisountTypeState>
  {
  public:
    PaxTypeDiscountStatus() {}

    explicit PaxTypeDiscountStatus(const PaxType& pt) { assign(pt); }

    void assign(const PaxType& pt);

    bool isInfantWithoutSeat() const { return isSet(DT_InfantWithoutSeat); }
    bool isInfantWithSeat() const { return isSet(DT_InfantWithSeat); }
    bool isInfantWithoutSeatRaw() const { return isSet(DT_InfantWithoutSeatRaw); }
    bool isInfantWithSeatRaw() const { return isSet(DT_InfantWithSeatRaw); }
    bool isChild() const { return isSet(DT_Child); }
    bool isYouth() const { return isSet(DT_Youth); }
    bool isSenior() const { return isSet(DT_Senior); }
    bool isChildOrInfant() const { return isSet(DT_ChildOrInfant); }
  } _discountStatus;

  static constexpr Indicator INFANT_WITH_SEAT_CAT19 = '0';
  static constexpr Indicator INFANT_WITHOUT_SEAT = '1';
  static constexpr Indicator INFANT_WITH_SEAT = '2';
  static constexpr Indicator CHILD = '3';
  static constexpr Indicator YOUTH = '4';
  static constexpr Indicator SENIOR = '5';
  static constexpr Indicator CHILD_OR_INFANT = '6';
  static constexpr Indicator ANY_CAT22 = '7';
  static constexpr Indicator INFANT_WITHOUT_SEAT_CAT19 = '8';
  static constexpr Indicator INFANT_WITHOUT_SEAT_NOFEE = '9';

  friend class DataHandle;
  friend class RefundDiscountApplierTest;
};
}
