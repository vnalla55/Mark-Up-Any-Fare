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

#include "Rules/ApplicationLimitRule.h"
#include "Rules/OutputTypeIndicatorRule.h"
#include "Rules/PointOverlappingForItineraryRule.h"
#include "Rules/PointOverlappingForYqYrRule.h"
#include "Rules/SpecialTaxProcessingRule.h"

#include <utility>

namespace tax
{

struct ProcessingOptionGroup
{
  boost::optional<OutputTypeIndicatorRule> _outputTypeIndicatorRule;
  boost::optional<ApplicationLimitRule> _applicationLimitRule;
  boost::optional<PointOverlappingForItineraryRule> _pointOverlappingForItineraryRule;
  boost::optional<PointOverlappingForYqYrRule> _pointOverlappingForYqYrRule;
  boost::optional<SpecialTaxProcessingRule> _specialTaxProcessingRule;

  template <template <class> class Functor, class ...Args>
  bool foreach(bool foundItinApplication,
               bool foundYqYrApplication,
               Args&&... args) const
  {
    if (!apply<OutputTypeIndicatorRule, Functor>(
             _outputTypeIndicatorRule, std::forward<Args>(args)...))
      return false;

    if (!apply<ApplicationLimitRule, Functor>(
             _applicationLimitRule, std::forward<Args>(args)...))
      return false;

    bool overlapForItin = foundItinApplication
        ? apply<PointOverlappingForItineraryRule, Functor>(
            _pointOverlappingForItineraryRule, std::forward<Args>(args)...)
        : true;
    bool overlapForYqYr = foundYqYrApplication
        ? apply<PointOverlappingForYqYrRule, Functor>(
            _pointOverlappingForYqYrRule, std::forward<Args>(args)...)
        : true;

    return (overlapForItin || overlapForYqYr) &&
           apply<SpecialTaxProcessingRule, Functor>(
               _specialTaxProcessingRule, std::forward<Args>(args)...);
  }
};
}
