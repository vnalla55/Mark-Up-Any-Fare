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

#include "Rules/TaxOnTicketingFeeRule.h"
#include "Rules/TaxOnTicketingFeeApplicator.h"
#include "ServiceInterfaces/Services.h"

namespace tax
{
TaxOnTicketingFeeRule::TaxOnTicketingFeeRule(const type::PercentFlatTag& percentFlatTag)
  : _percentFlatTag(percentFlatTag)
{
}

TaxOnTicketingFeeRule::ApplicatorType
TaxOnTicketingFeeRule::createApplicator(type::Index const& /*itinIndex*/,
                                        const Request& /*request*/,
                                        Services& services,
                                        RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this, services.currencyService());
}

bool
TaxOnTicketingFeeRule::isPercent() const
{
  return percentFlatTag() == type::PercentFlatTag::Percent;
}

std::string
TaxOnTicketingFeeRule::getDescription(Services&) const
{
  return std::string("TAXONTICKETINGFEE RULE CAN NOT BE APLICABLE IF TAXABLE UNIT TAG IS NOT SET");
}
}
