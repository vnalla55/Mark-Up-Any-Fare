//-------------------------------------------------------------------
//
//  File:        RefundDiscountApplier.cpp
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

#include "RexPricing/RefundDiscountApplier.h"

#include "Common/FallbackUtil.h"
#include "Common/Money.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"

namespace tse
{
FIXEDFALLBACK_DECL(enhancedRefundDiscountApplierRefactor)
FIXEDFALLBACK_DECL(fixCat33Discount)

const Indicator RefundDiscountApplier::INFANT_WITH_SEAT_CAT19;
const Indicator RefundDiscountApplier::INFANT_WITHOUT_SEAT;
const Indicator RefundDiscountApplier::INFANT_WITH_SEAT;
const Indicator RefundDiscountApplier::CHILD;
const Indicator RefundDiscountApplier::YOUTH;
const Indicator RefundDiscountApplier::SENIOR;
const Indicator RefundDiscountApplier::CHILD_OR_INFANT;
const Indicator RefundDiscountApplier::ANY_CAT22;
const Indicator RefundDiscountApplier::INFANT_WITHOUT_SEAT_CAT19;
const Indicator RefundDiscountApplier::INFANT_WITHOUT_SEAT_NOFEE;

bool
RefundDiscountApplier::apply(Money& amt, const PaxTypeFare& ptf, const VoluntaryRefundsInfo& rec3)
    const
{
  std::vector<Indicator> discountTags = getDiscountTypes(rec3);

  if (discountTags.empty())
    return false;

  return applyDiscount(amt, discountTags, getDiscountInfo(ptf));
}

bool
RefundDiscountApplier::apply(Money& amt, const VoluntaryRefundsInfo& rec3) const
{
  if (!_discountStatus.isInfantWithoutSeat())
    return false;

  std::vector<Indicator> discountTags = getDiscountTypes(rec3);

  if (std::find(discountTags.begin(), discountTags.end(), INFANT_WITHOUT_SEAT) ==
      discountTags.end())
    return false;

  amt.value() = 0.0;
  return true;
}

namespace
{
typedef Indicator (VoluntaryRefundsInfo::*TagGetter)() const;

const unsigned numberDiscountTags = 4;

const TagGetter getters[numberDiscountTags] = { &VoluntaryRefundsInfo::discountTag1,
                                                &VoluntaryRefundsInfo::discountTag2,
                                                &VoluntaryRefundsInfo::discountTag3,
                                                &VoluntaryRefundsInfo::discountTag4 };
}

std::vector<Indicator>
RefundDiscountApplier::getDiscountTypes(const VoluntaryRefundsInfo& rec3) const
{
  std::vector<Indicator> tags;
  tags.reserve(numberDiscountTags);
  for (const auto& getter : getters)
  {
    insertDiscountType((rec3.*getter)(), tags);
  }
  return tags;
}

void
RefundDiscountApplier::insertDiscountType(Indicator tag, std::vector<Indicator>& tags) const
{
  switch (tag)
  {
  case INFANT_WITHOUT_SEAT:
  case INFANT_WITH_SEAT:
  case CHILD:
  case YOUTH:
  case SENIOR:
  case CHILD_OR_INFANT:
  case ANY_CAT22:
  case INFANT_WITH_SEAT_CAT19:
  case INFANT_WITHOUT_SEAT_CAT19:
  case INFANT_WITHOUT_SEAT_NOFEE:
    tags.push_back(tag);
  }
}

const DiscountInfo*
RefundDiscountApplier::getDiscountInfo(const PaxTypeFare& ptf) const try
{
  // if (!ptf.isDiscounted()) return 0;
  return &ptf.discountInfo();
}
catch (...) { return nullptr; }

bool
RefundDiscountApplier::applyDiscount(Money& fee, const DiscountInfo& discount) const
{
  if (!discount.cur1().empty() && discount.fareAmt1() > EPSILON)
    return false;

  fee.value() *= 1E-2 * discount.discPercent();
  return true;
}

bool
RefundDiscountApplier::applyDiscount(Money& fee,
                                     const std::vector<Indicator>& discountTags,
                                     const DiscountInfo* discount) const
{
  // when removing enhancedRefundDiscountApplierRefactor please also remove
  // enhancedREfundDiscountApplier class
  bool fallback = fallback::fixed::enhancedRefundDiscountApplierRefactor();
  for (const auto discountTag : discountTags)
  {
    switch (discountTag)
    {
    case INFANT_WITH_SEAT_CAT19:
      if (!fallback)
      {
        if (_discountStatus.isInfantWithSeatRaw() &&
            isDiscount(discount, RuleConst::CHILDREN_DISCOUNT_RULE))
          return applyDiscount(fee, *discount);
      }

      break;
    case INFANT_WITHOUT_SEAT:
      if (!fallback::fixed::fixCat33Discount())
      {
        if ((_discountStatus.isInfantWithSeatRaw() || _discountStatus.isInfantWithoutSeatRaw()) &&
            isDiscount(discount, RuleConst::CHILDREN_DISCOUNT_RULE))
          return applyDiscount(fee, *discount);
      }
      else
      {
        if (!fallback
                ? (_discountStatus.isInfantWithSeatRaw() || _discountStatus.isInfantWithoutSeatRaw())
                : _discountStatus.isInfantWithoutSeat())
        {
          fee.value() = 0.0;
          return true;
        }
      }

      break;
    case INFANT_WITH_SEAT:
      if (!fallback && _discountStatus.isInfantWithoutSeatRaw())
      {
        fee.value() = 0.0;
        return true;
      }

      if ((!fallback ? _discountStatus.isInfantWithSeatRaw()
                     : _discountStatus.isInfantWithSeat()) &&
          isDiscount(discount, RuleConst::CHILDREN_DISCOUNT_RULE))
        return applyDiscount(fee, *discount);

      break;
    case CHILD:
      if (_discountStatus.isChild() && isDiscount(discount, RuleConst::CHILDREN_DISCOUNT_RULE))
        return applyDiscount(fee, *discount);
      break;

    case YOUTH:
      if (_discountStatus.isYouth() && isDiscount(discount, RuleConst::OTHER_DISCOUNT_RULE))
        return applyDiscount(fee, *discount);

      break;
    case SENIOR:
      if (_discountStatus.isSenior() && isDiscount(discount, RuleConst::OTHER_DISCOUNT_RULE))
        return applyDiscount(fee, *discount);

      break;
    case CHILD_OR_INFANT:
      if (_discountStatus.isChildOrInfant() &&
          isDiscount(discount, RuleConst::CHILDREN_DISCOUNT_RULE))
        return applyDiscount(fee, *discount);

      break;
    case ANY_CAT22:
      if (isDiscount(discount, RuleConst::OTHER_DISCOUNT_RULE))
        return applyDiscount(fee, *discount);

      break;
    case INFANT_WITHOUT_SEAT_CAT19:
      if (!fallback && _discountStatus.isInfantWithoutSeatRaw() &&
          isDiscount(discount, RuleConst::CHILDREN_DISCOUNT_RULE))
        return applyDiscount(fee, *discount);

      break;
    case INFANT_WITHOUT_SEAT_NOFEE:
      if (!fallback && _discountStatus.isInfantWithoutSeatRaw())
      {
        fee.value() = 0.0;
        return true;
      }

      break;
    default:
      ;
    }
  }
  return false;
}

const RefundDiscountApplier*
RefundDiscountApplier::create(DataHandle& dh, const PaxType& pt)
{
  RefundDiscountApplier* app;
  dh.get(app);
  app->assign(pt);
  return app;
}

// --== PaxTypeDiscountStatus ==--
namespace
{
template <unsigned size>
inline bool
isPaxType(const PaxTypeCode (&tab)[size], const PaxTypeCode& code)
{
  return std::find(tab, tab + size, code) != (tab + size);
}

inline bool
isInfantWithoutSeat(const PaxType& pax)
{
  static const PaxTypeCode types[] = { "INF", "JNF", "LIF", "INY", "INR" };
  return isPaxType(types, pax.paxType());
}

inline bool
isInfantWithSeat(const PaxType& pax)
{
  static const PaxTypeCode types[] = { "INS", "JNS", "LNS", "ISR" };
  return isPaxType(types, pax.paxType());
}

inline bool
isInfantWithSeatRaw(const PaxType& pt)
{
  return pt.paxTypeInfo().isInfant() && pt.paxTypeInfo().numberSeatsReq() > 0;
}

inline bool
isInfantWithoutSeatRaw(const PaxType& pt)
{
  return pt.paxTypeInfo().infantInd() == YES && pt.paxTypeInfo().numberSeatsReq() == 0;
}

inline bool
isYouth(const PaxType& pax)
{
  static const PaxTypeCode types[] = { "GIY", "GYT", "ITY", "YSB", "YTH"
                                       "YTR", "ZNN", "ZYC", "ZZS" };
  return isPaxType(types, pax.paxType());
}

inline bool
isSenior(const PaxType& pax)
{
  static const PaxTypeCode types[] = { "JRC", "SCC", "SCF", "SCM", "SNN", "SRC",
                                       "SRR", "TS1", "YCB", "ZSF", "ZSM" };
  return isPaxType(types, pax.paxType());
}

} // empty namespace

void
RefundDiscountApplier::PaxTypeDiscountStatus::assign(const PaxType& pt)
{
  set(DT_InfantWithoutSeat, tse::isInfantWithoutSeat(pt));
  set(DT_InfantWithSeat, tse::isInfantWithSeat(pt));
  set(DT_InfantWithoutSeatRaw, tse::isInfantWithoutSeatRaw(pt));
  set(DT_InfantWithSeatRaw, tse::isInfantWithSeatRaw(pt));
  set(DT_Child, pt.paxTypeInfo().isChild());
  set(DT_Youth, tse::isYouth(pt));
  set(DT_Senior, tse::isSenior(pt));
  set(DT_ChildOrInfant, (pt.paxTypeInfo().isChild() || pt.paxTypeInfo().isInfant()));
}
} // tse namespace
