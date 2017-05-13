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
#include "ServiceInterfaces/Services.h"
#include "Rules/OptionalServiceTagsRule.h"
#include "Rules/OptionalServiceTagsApplicator.h"
#include "DomainDataObjects/Request.h"

namespace tax
{

OptionalServiceTagsRule::OptionalServiceTagsRule(
    TaxableUnitTagSet const& applicableTaxableUnits)
  : _taxableUnitSet(applicableTaxableUnits)
{
}

OptionalServiceTagsRule::~OptionalServiceTagsRule() {}

OptionalServiceTagsRule::ApplicatorType
OptionalServiceTagsRule::createApplicator(type::Index const& /*itinIndex*/,
                                          const Request& /*request*/,
                                          Services& /*services*/,
                                          RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this);
}

std::string
OptionalServiceTagsRule::getDescription(Services&) const
{
  std::string result = "OPTIONAL SERVICES RESTRICTED TO:";

  if (_taxableUnitSet.hasTag(type::TaxableUnit::OCFlightRelated))
  {
    result += " FLIGHTRELATED OR PREPAID";
  }
  if (_taxableUnitSet.hasTag(type::TaxableUnit::OCTicketRelated))
  {
    result += " TICKETRELATED ";
  }
  if (_taxableUnitSet.hasTag(type::TaxableUnit::OCMerchandise))
  {
    result += " MERCHANDISE ";
  }
  if (_taxableUnitSet.hasTag(type::TaxableUnit::OCFareRelated))
  {
    result += " FARERELATED ";
  }
  if (_taxableUnitSet.hasTag(type::TaxableUnit::BaggageCharge))
  {
    result += " BAGGAGECHARGE ";
  }

  return result;
}
}
