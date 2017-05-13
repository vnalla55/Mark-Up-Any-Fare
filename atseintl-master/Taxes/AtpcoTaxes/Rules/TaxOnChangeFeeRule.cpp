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

//#include "DomainDataObjects/Request.h"
#include "Rules/TaxOnChangeFeeRule.h"
#include "Rules/TaxOnChangeFeeApplicator.h"
#include "ServiceInterfaces/Services.h"

namespace tax
{
TaxOnChangeFeeRule::TaxOnChangeFeeRule(const type::PercentFlatTag& percentFlatTag)
  : _percentFlatTag(percentFlatTag)
{
}

TaxOnChangeFeeRule::~TaxOnChangeFeeRule()
{
}

TaxOnChangeFeeRule::ApplicatorType
TaxOnChangeFeeRule::createApplicator(type::Index const& /*itinIndex*/,
                                     const Request& /*request*/,
                                     Services& services,
                                     RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this, services);
}

std::string
TaxOnChangeFeeRule::getDescription(Services&) const
{
  return std::string("TAXONCHANGEFEE RULE CAN NOT BE APLICABLE IF TAXABLE UNIT TAG IS NOT SET");
}
}
