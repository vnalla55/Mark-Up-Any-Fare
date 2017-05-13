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
#include <boost/lexical_cast.hpp>

#include "Rules/SaleDateRule.h"

#include "Rules/SaleDateApplicator.h"
#include "DomainDataObjects/Request.h"

namespace tax
{

SaleDateRule::SaleDateRule(type::Date effDate, type::Timestamp discDate)
  : _effDate(effDate), _discDate(discDate)
{
}

SaleDateRule::~SaleDateRule() {}

SaleDateRule::ApplicatorType
SaleDateRule::createApplicator(type::Index const& /*itinIndex*/,
                               const Request& request,
                               Services& /*services*/,
                               RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(this,
                        _effDate,
                        _discDate,
                        type::Timestamp(request.ticketingOptions().ticketingDate(),
                                        request.ticketingOptions().ticketingTime()));
}

std::string
SaleDateRule::getDescription(Services&) const
{
  return std::string("SALE DATE MUST BE BETWEEN ") + boost::lexical_cast<std::string>(_effDate) +
         " AND " + boost::lexical_cast<std::string>(_discDate);
}
}
