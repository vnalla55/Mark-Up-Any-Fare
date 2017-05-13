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

#include <boost/optional.hpp>
#include "DataModel/Common/Types.h"

namespace tax
{
struct OutputExchangeReissueDetails
{
  OutputExchangeReissueDetails() : isMixedTax(false), isPartialTax(false)
  // reissueRestrictionApply(false),
  // taxApplyToReissue(false),
  // reissueTaxRefundable(false)
  {
  }

  bool isMixedTax;
  bool isPartialTax;
  boost::optional<type::MoneyAmount> minTaxAmount;
  boost::optional<type::MoneyAmount> maxTaxAmount;
  boost::optional<type::CurrencyCode> minMaxTaxCurrency;
  boost::optional<type::CurDecimals> minMaxTaxCurrencyDecimals;

  //  bool reissueRestrictionApply;
  //  bool taxApplyToReissue;
  //  bool reissueTaxRefundable;
  //  boost::optional<std::string> reissueTaxCurrency;
  //  boost::optional<type::MoneyAmount> reissueTaxAmount;
  //  boost::optional<type::MoneyAmount> taxRateUsed;
};

inline bool
operator==(const OutputExchangeReissueDetails& lhs, const OutputExchangeReissueDetails& rhs)
{
  return lhs.isMixedTax == rhs.isMixedTax && lhs.isPartialTax == rhs.isPartialTax &&
         lhs.minTaxAmount == rhs.minTaxAmount && lhs.maxTaxAmount == rhs.maxTaxAmount &&
         lhs.minMaxTaxCurrency == rhs.minMaxTaxCurrency &&
         lhs.minMaxTaxCurrencyDecimals == rhs.minMaxTaxCurrencyDecimals;
  // lhs.reissueRestrictionApply == rhs.reissueRestrictionApply &&
  // lhs.taxApplyToReissue == rhs.taxApplyToReissue &&
  // lhs.reissueTaxRefundable == rhs.reissueTaxRefundable &&
  // lhs.reissueTaxCurrency == rhs.reissueTaxCurrency &&
  // lhs.reissueTaxAmount == rhs.reissueTaxAmount && lhs.taxRateUsed == rhs.taxRateUsed;
}

} // namespace tax
