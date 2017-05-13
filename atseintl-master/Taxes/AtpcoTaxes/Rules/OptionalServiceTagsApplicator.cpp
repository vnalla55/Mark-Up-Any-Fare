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
#include "Rules/OptionalServiceTagsApplicator.h"
#include "Rules/OptionalServiceTagsRule.h"
#include "Rules/PaymentDetail.h"
#include "DataModel/Common/Types.h"

namespace tax
{

OptionalServiceTagsApplicator::OptionalServiceTagsApplicator(OptionalServiceTagsRule const& parent)
  : BusinessRuleApplicator(&parent), _optionalServiceTagsRule(parent)
{
}

OptionalServiceTagsApplicator::~OptionalServiceTagsApplicator() {}

bool
OptionalServiceTagsApplicator::serviceMatchesTaxableUnitTag(
    type::OptionalServiceTag const& optionalServiceTag) const
{
  const TaxableUnitTagSet& taxableUnitSet = _optionalServiceTagsRule.taxableUnitSet();

  if (taxableUnitSet.hasTag(type::TaxableUnit::OCFlightRelated) &&
      (optionalServiceTag == type::OptionalServiceTag::FlightRelated ||
      optionalServiceTag == type::OptionalServiceTag::PrePaid))
    return true;

  if (taxableUnitSet.hasTag(type::TaxableUnit::OCTicketRelated) &&
      optionalServiceTag == type::OptionalServiceTag::TicketRelated)
    return true;

  if (taxableUnitSet.hasTag(type::TaxableUnit::OCMerchandise) &&
      optionalServiceTag == type::OptionalServiceTag::Merchandise)
    return true;

  if (taxableUnitSet.hasTag(type::TaxableUnit::OCFareRelated) &&
      optionalServiceTag == type::OptionalServiceTag::FareRelated)
    return true;

  if (taxableUnitSet.hasTag(type::TaxableUnit::BaggageCharge) &&
      optionalServiceTag == type::OptionalServiceTag::BaggageCharge)
    return true;

  return false;
}

bool
OptionalServiceTagsApplicator::apply(PaymentDetail& paymentDetail) const
{
  for(OptionalService & optionalService : paymentDetail.optionalServiceItems())
  {
    if (!optionalService.isFailed() && !serviceMatchesTaxableUnitTag(optionalService.type()))
    {
      optionalService.setFailedRule(&_optionalServiceTagsRule);
    }
  }

  return true;
}
}
