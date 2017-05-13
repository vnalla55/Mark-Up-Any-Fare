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
#include "Common/MoneyUtil.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/FlatTaxRule.h"
#include "Rules/FlatTaxApplicator.h"
#include "Rules/MathUtils.h"

namespace tax
{
FlatTaxRule::FlatTaxRule(TaxableUnitTagSet const& applicableTaxableUnits)
  : _taxableUnitSet(applicableTaxableUnits)
{
}

FlatTaxRule::~FlatTaxRule() {}

FlatTaxRule::ApplicatorType
FlatTaxRule::createApplicator(type::Index const& /* itinIndex */,
                              const Request& request,
                              Services& services,
                              RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(this,
                        services,
                        _taxableUnitSet,
                        request.ticketingOptions().paymentCurrency());
}

std::string
FlatTaxRule::getDescription(Services&) const
{
  if (_taxableUnitSet.isEmpty())
    return std::string("FLATTAX RULE CAN NOT BE APLICABLE IF TAXABLE UNIT TAG IS NOT SET");

  return std::string("FLAT TAX AMOUNT");
}
}
