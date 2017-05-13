// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "Rules/PaymentDetail.h"
#include "Util/BranchPrediction.h"

#include <cassert>
#include <memory>
#include <utility>
#include <vector>

namespace tax
{

class TaxName;

struct TaggedPaymentDetail
{
  const TaxName* const taxName;
  PaymentDetail detail;

  TaggedPaymentDetail(const PaymentRuleData& paymentRuleData,
                      const Geo& taxPointBegin,
                      const Geo& taxPointEnd,
                      const TaxName& taxName,
                      const type::CarrierCode& marketingCarrier = BLANK_CARRIER)
    : taxName(&taxName)
    , detail(paymentRuleData, taxPointBegin, taxPointEnd, taxName, marketingCarrier) {}
};

// RawPayments is a trimmed version of std::vector<TaggedPaymentDetail>
// it is stripped of push_back and copy operations. We want to enforce
// at compile-time no copying of TaggedPaymentDetail-s.
class RawPayments : private std::vector<TaggedPaymentDetail>
{
  typedef std::vector<TaggedPaymentDetail> base;

public:
  struct WithCapacity
  {
    size_t _capacity;
    explicit WithCapacity (size_t c) : _capacity(c) {}
  };

  RawPayments() = default;
  RawPayments(RawPayments&&) = default;
  RawPayments(const RawPayments&) = delete;
  explicit RawPayments(WithCapacity c) { base::reserve(c._capacity); }

  friend void swap(RawPayments& lhs, RawPayments& rhs) { lhs.swap(rhs); }

  PaymentDetail&
  emplace_back(const PaymentRuleData& paymentRuleData,
               const Geo& taxPointBegin,
               const Geo& taxPointEnd,
               const TaxName& taxName,
               const type::CarrierCode& marketingCarrier = BLANK_CARRIER)
  {
    assert (base::capacity() != 0);
    if (UNLIKELY(base::size() >= base::capacity()))
      throw std::range_error("capacity in RawPayments exceeded");

    base::emplace_back(paymentRuleData, taxPointBegin, taxPointEnd, taxName, marketingCarrier);
    return base::back().detail;
  }

  void reserve(size_t c)
  {
    assert (base::size() == 0);
    base::reserve(c);
  }

  using base::empty;
  using base::size;
  using base::operator[];
  using base::back;
  using base::begin;
  using base::end;
  using base::capacity;

  using base::value_type;
  using base::iterator;
  using base::const_iterator;
};

} // namespace tax

