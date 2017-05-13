// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Common/GroupedContainers.h"
#include "Rules/ItinPayments.h"
#include "Rules/RawPayments.h"

#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>

namespace tax
{

struct ItinsPayments
{
  typedef GroupedContainers<std::vector<RawPayments> > ItinsRawPayments;

  ItinsRawPayments _itinsRawPayments;
  boost::ptr_vector<ItinPayments> _itinPayments;
  type::CurrencyCode paymentCurrency;
  type::CurDecimals paymentCurrencyNoDec;

  void swap(ItinsPayments& rhs)
  {
    boost::swap(_itinsRawPayments, rhs._itinsRawPayments);
    boost::swap(_itinPayments, rhs._itinPayments);
    boost::swap(paymentCurrency, rhs.paymentCurrency);
    boost::swap(paymentCurrencyNoDec, rhs.paymentCurrencyNoDec);
  }

  void resize(type::Index size)
  {
    _itinPayments.resize(size);
  }
};

inline void swap(ItinsPayments& lhs, ItinsPayments& rhs) { lhs.swap(rhs); }

} // namespace tax

