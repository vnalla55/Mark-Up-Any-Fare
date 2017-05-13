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

#include "Rules/CurrencyOfSaleRule.h"
#include "Rules/CustomerRestrictionRule.h"
#include "Rules/PointOfTicketingRule.h"
#include "Rules/SaleDateRule.h"
#include "Rules/ServiceFeeSecurityRule.h"
#include "Rules/ThirdPartyTagRule.h"

namespace tax
{

struct TicketGroup
{
  boost::optional<CurrencyOfSaleRule> _currencyOfSaleRule;
  boost::optional<ThirdPartyTagRule> _thirdPartyTagRule;
  boost::optional<PointOfTicketingRule> _pointOfTicketingRule;
  boost::optional<SaleDateRule> _saleDateRule;
  boost::optional<ServiceFeeSecurityRule> _serviceFeeSecurityRule;
  boost::optional<CustomerRestrictionRule> _customerRestrictionRule;

  template <template <class> class Functor, class ...Args>
  bool foreach(Args&&... args) const
  {
    return apply<CurrencyOfSaleRule, Functor>(
               _currencyOfSaleRule, std::forward<Args>(args)...) &&
           apply<ThirdPartyTagRule, Functor>(
               _thirdPartyTagRule, std::forward<Args>(args)...) &&
           apply<PointOfTicketingRule, Functor>(
               _pointOfTicketingRule, std::forward<Args>(args)...) &&
           apply<SaleDateRule, Functor>(
               _saleDateRule, std::forward<Args>(args)...) &&
           apply<ServiceFeeSecurityRule, Functor>(
               _serviceFeeSecurityRule, std::forward<Args>(args)...) &&
           apply<CustomerRestrictionRule, Functor>(
               _customerRestrictionRule, std::forward<Args>(args)...);
  }
};
}

