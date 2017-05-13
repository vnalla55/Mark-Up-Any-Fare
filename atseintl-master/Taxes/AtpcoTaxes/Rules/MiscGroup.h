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

#include "Rules/AlternateRefAkHiFactorsRule.h"
#include "Rules/ExemptTagRule.h"
#include "Rules/PreviousTicketRule.h"
#include "Rules/ReportingRecordRule.h"
#include "Rules/SectorDetailRule.h"
#include "Rules/ServiceBaggageRule.h"
#include "Rules/TaxMatchingApplTagRule.h"
#include "Rules/TaxOnFareRule.h"
#include "Rules/TaxOnOptionalServiceRule.h"
#include "Rules/TaxOnTaxRule.h"
#include "Rules/TaxOnYqYrRule.h"
#include "Rules/TicketMinMaxValueRule.h"
#include "Rules/TicketMinMaxValueOCRule.h"

#include "boost/optional.hpp"

#include <utility>

namespace tax
{
struct MiscGroup
{
  boost::optional<SectorDetailRule> _sectorDetailRule;
  boost::optional<ServiceBaggageRule> _serviceBaggageRule;
  boost::optional<TicketMinMaxValueRule> _ticketMinMaxValueRule;
  boost::optional<TicketMinMaxValueOCRule> _ticketMinMaxValueOCRule;
  boost::optional<TaxOnTaxRule> _taxOnTaxRule;
  boost::optional<ReportingRecordRule> _reportingRecordRule;
  boost::optional<TaxOnYqYrRule> _taxOnYqYrRule;
  boost::optional<TaxOnOptionalServiceRule> _taxOnOptionalServiceRule;
  boost::optional<AlternateRefAkHiFactorsRule> _alternateRefAkHiFactorsRule;
  boost::optional<TaxOnFareRule> _taxOnFareRule;
  boost::optional<TaxMatchingApplTagRule> _taxMatchingApplTagRule;
  boost::optional<ExemptTagRule> _exemptTagRule;
  boost::optional<PreviousTicketRule> _previousTicketRule;

  template <template <class> class Functor, class ...Args>
  bool foreach(Args&&... args) const
  {
    return apply<SectorDetailRule, Functor>(
               _sectorDetailRule, std::forward<Args>(args)...) &&
           apply<ServiceBaggageRule, Functor>(
               _serviceBaggageRule, std::forward<Args>(args)...) &&
           apply<TicketMinMaxValueRule, Functor>(
               _ticketMinMaxValueRule, std::forward<Args>(args)...) &&
           apply<TicketMinMaxValueOCRule, Functor>(
               _ticketMinMaxValueOCRule, std::forward<Args>(args)...) &&
           apply<TaxOnTaxRule, Functor>(
               _taxOnTaxRule, std::forward<Args>(args)...) &&
           apply<ReportingRecordRule, Functor>(
               _reportingRecordRule, std::forward<Args>(args)...) &&
           apply<TaxOnYqYrRule, Functor>(
               _taxOnYqYrRule, std::forward<Args>(args)...) &&
           apply<TaxOnOptionalServiceRule, Functor>(
               _taxOnOptionalServiceRule, std::forward<Args>(args)...) &&
           apply<AlternateRefAkHiFactorsRule, Functor>(
               _alternateRefAkHiFactorsRule, std::forward<Args>(args)...) &&
           apply<TaxOnFareRule, Functor>(
               _taxOnFareRule, std::forward<Args>(args)...) &&
           apply<TaxMatchingApplTagRule, Functor>(
               _taxMatchingApplTagRule, std::forward<Args>(args)...) &&
           apply<ExemptTagRule, Functor>(
               _exemptTagRule, std::forward<Args>(args)...) &&
           apply<PreviousTicketRule, Functor>(
               _previousTicketRule, std::forward<Args>(args)...);
  }
};
}
