// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Rules/RulesGroupApplyFunctor.h"

#include "Rules/FlatTaxRule.h"
#include "Rules/PercentageTaxRule.h"
#include "Rules/TaxMinMaxValueRule.h"
#include "Rules/TaxOnChangeFeeRule.h"
#include "Rules/TaxOnTicketingFeeRule.h"
#include "Rules/TaxRoundingRule.h"
#include "Rules/TaxRoundingOCRule.h"
#include "Rules/YqYrAmountRule.h"

#include <utility>

namespace tax
{
struct ApplyGroup
{
  boost::optional<YqYrAmountRule> _yqYrAmountRule;
  boost::optional<FlatTaxRule> _flatTaxRule;
  boost::optional<PercentageTaxRule> _percentageTaxRule;
  boost::optional<TaxMinMaxValueRule> _taxMinMaxValueRule;
  boost::optional<TaxOnChangeFeeRule> _taxOnChangeFeeRule;
  boost::optional<TaxOnTicketingFeeRule> _taxOnTicketingFeeRule;
  boost::optional<TaxRoundingRule> _taxRoundingRule;
  boost::optional<TaxRoundingOCRule> _taxRoundingOCRule;

  template <template <class> class Functor, class ...Args>
  bool foreach(Args&&... args) const
  {
    return apply<YqYrAmountRule, Functor>(
               _yqYrAmountRule, std::forward<Args>(args)...) &&
           apply<FlatTaxRule, Functor>(
               _flatTaxRule, std::forward<Args>(args)...) &&
           apply<PercentageTaxRule, Functor>(
               _percentageTaxRule, std::forward<Args>(args)...) &&
           apply<TaxMinMaxValueRule, Functor>(
               _taxMinMaxValueRule, std::forward<Args>(args)...) &&
           apply<TaxOnChangeFeeRule, Functor>(
               _taxOnChangeFeeRule, std::forward<Args>(args)...) &&
           apply<TaxOnTicketingFeeRule, Functor>(
               _taxOnTicketingFeeRule, std::forward<Args>(args)...) &&
           apply<TaxRoundingRule, Functor>(
               _taxRoundingRule, std::forward<Args>(args)...) &&
           apply<TaxRoundingOCRule, Functor>(
               _taxRoundingOCRule, std::forward<Args>(args)...);
  }
};
}

