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

#include "DomainDataObjects/Request.h"
#include "Rules/SpecialTaxProcessingApplicator.h"
#include "Rules/SpecialTaxProcessingRule.h"

namespace tax
{

SpecialTaxProcessingRule::SpecialTaxProcessingRule(const type::TaxProcessingApplTag& applicationTag)
  : _applicationTag(applicationTag)
{
}

SpecialTaxProcessingRule::~SpecialTaxProcessingRule()
{
}

SpecialTaxProcessingRule::ApplicatorType
SpecialTaxProcessingRule::createApplicator(type::Index const& /*itinIndex*/,
                                           const Request& request,
                                           Services& /*services*/,
                                           RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this, request.processing().tch());
}

std::string
SpecialTaxProcessingRule::getDescription(Services&) const
{
  return "TAXPROCESSINGAPPLTAG IS: " + _applicationTag.asString();
}

} /* namespace tax */
