// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "Rules/ApplicationLimitApplicator.h"
#include "Rules/ApplicationLimitRule.h"

namespace tax
{
ApplicationLimitRule::ApplicationLimitRule(type::TaxApplicationLimit taxApplicationLimit)
: _taxApplicationLimit(taxApplicationLimit)
{
}

ApplicationLimitRule::ApplicatorType
ApplicationLimitRule::createApplicator(type::Index const& itinIndex,
                                       const Request& request,
                                       Services& /*services*/,
                                       RawPayments& /*itinPayments*/) const
{
  const Itin& itin = request.getItinByIndex(itinIndex);
  return ApplicatorType(*this, *itin.geoPath(), itin.flightUsages());
}

std::string
ApplicationLimitRule::getDescription(Services&) const
{
  std::ostringstream buf;
  buf << "APPLICATION LIMIT DOES NOT APPLY ";
  if (_taxApplicationLimit == type::TaxApplicationLimit::FirstTwoPerUSRoundTrip)
    buf << "NO US ROUND TRIPS IN ITIN";
  buf << "\n";

  return buf.str();
}
}

