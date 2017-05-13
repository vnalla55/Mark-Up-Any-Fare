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

#include "Rules/CarrierFlightRule.h"
#include "Rules/PassengerTypeCodeRule.h"
#include "Rules/TravelDatesRule.h"
#include "Rules/ValidatingCarrierRule.h"

#include <utility>

namespace tax
{

struct ItinGroup
{
  boost::optional<TravelDatesJourneyRule> _travelDatesJourneyRule;
  boost::optional<TravelDatesTaxPointRule> _travelDatesTaxPointRule;
  boost::optional<ValidatingCarrierRule> _validatingCarrierRule;
  boost::optional<CarrierFlightRule> _carrierFlightRule;
  boost::optional<PassengerTypeCodeRule> _passengerTypeCodeRule;

  template <template <class> class Functor, class ...Args>
  bool foreach(Args&&... args) const
  {
    return apply<TravelDatesJourneyRule, Functor>(
               _travelDatesJourneyRule, std::forward<Args>(args)...) &&
           apply<TravelDatesTaxPointRule, Functor>(
               _travelDatesTaxPointRule, std::forward<Args>(args)...) &&
           apply<ValidatingCarrierRule, Functor>(
               _validatingCarrierRule, std::forward<Args>(args)...) &&
           apply<CarrierFlightRule, Functor>(
               _carrierFlightRule, std::forward<Args>(args)...) &&
           apply<PassengerTypeCodeRule, Functor>(
               _passengerTypeCodeRule, std::forward<Args>(args)...);
  }
};
}


