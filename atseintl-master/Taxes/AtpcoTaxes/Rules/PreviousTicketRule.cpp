// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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
#include "Rules/PreviousTicketRule.h"
#include "Rules/RawPayments.h"
#include "ServiceInterfaces/PreviousTicketService.h"
#include "ServiceInterfaces/Services.h"

namespace tax
{
namespace
{
constexpr int PERCENT_PRECISION = 1000000;
}

PreviousTicketRule::PreviousTicketRule(type::Percent const& taxPercentage)
  : _taxPercentage(taxPercentage / PERCENT_PRECISION)
{
}

PreviousTicketApplicator
PreviousTicketRule::createApplicator(const type::Index& /*itinIndex*/,
                                     const Request& /*request*/,
                                     Services& services,
                                     RawPayments& /*rawPayments*/) const
{
  return PreviousTicketApplicator(this, services, _taxPercentage);
}

std::string
PreviousTicketRule::getDescription(Services& services) const
{
  std::ostringstream oss;
  oss << "REQUIRED TAX NOT CALCULATED FOR PREVIOUS TICKET\n"
      << "TAXES COMPUTED FOR PREVIOUS TICKET:\n";

  for (const auto& each : services.previousTicketService().getTaxesForPreviousTicket())
  {
    oss << each.getSabreTaxCode() << " ";
  }
  oss << std::endl;

  return oss.str();
}
}
