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
#include "Rules/TaxCodeConversionRule.h"
#include "Rules/TaxCodeConversionApplicator.h"

namespace tax
{

std::string TaxCodeConversionRule::getDescription(Services&) const
{
  return "COMPUTE SABRE-COMPATIBLE TAX CODE FROM ATPCO TAX TYPE AND TAX NAME";
}

TaxCodeConversionRule::ApplicatorType
TaxCodeConversionRule::createApplicator(type::Index const&,
                                        const Request&,
                                        Services& services,
                                        RawPayments&) const
{
  return ApplicatorType(*this, services);
}

} // namespace tax

