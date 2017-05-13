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

#include "Rules/BlankLimitRule.h"
#include "Rules/ContinuousJourneyLimitRule.h"
#include "Rules/OnePerItinLimitRule.h"
#include "Rules/SectorOverlappingRule.h"
#include "Rules/SingleJourneyLimitRule.h"
#include "Rules/UsOneWayAndRoundTripLimitRule.h"

#include <boost/optional.hpp>

namespace tax
{
// No foreach() in this group. Limits and sector overlapping are checked differently
struct LimitGroup
{
  LimitGroup() : _limitType(type::TaxApplicationLimit::Unlimited) {}

  type::TaxApplicationLimit _limitType;
  boost::optional<BlankLimitRule> _blankLimitRule;
  boost::optional<OnePerItinLimitRule> _onePerItinLimitRule;
  boost::optional<UsOneWayAndRoundTripLimitRule> _usOneWayAndRoundTripLimitRule;
  boost::optional<ContinuousJourneyLimitRule> _continuousJourneyLimitRule;
  boost::optional<SingleJourneyLimitRule> _singleJourneyLimitRule;
  SectorOverlappingRule _sectorOverlappingRule;

  const BusinessRule&
  getLimitRule() const
  {
    switch (_limitType)
    {
    case type::TaxApplicationLimit::OnceForItin:
      return _onePerItinLimitRule.get();
    case type::TaxApplicationLimit::FirstTwoPerContinuousJourney:
      return _continuousJourneyLimitRule.get();
    case type::TaxApplicationLimit::OncePerSingleJourney:
      return _singleJourneyLimitRule.get();
    case type::TaxApplicationLimit::FirstTwoPerUSRoundTrip:
      return _usOneWayAndRoundTripLimitRule.get();
    default:
      return _blankLimitRule.get();
    }
  }
};

}

