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
#include "DomainDataObjects/Request.h"
#include "Rules/TaxMatchingApplTagRule.h"
#include "Rules/TaxMatchingApplTagApplicator.h"
#include "ServiceInterfaces/Services.h"

namespace tax
{

TaxMatchingApplTagRule::TaxMatchingApplTagRule(type::TaxMatchingApplTag const& taxMatchingApplTag,
                                               bool alternateTurnaroundDeterminationLogic)
  : _taxMatchingApplTag(taxMatchingApplTag),
    _alternateTurnaroundDeterminationLogic(alternateTurnaroundDeterminationLogic)
{
}

TaxMatchingApplTagRule::~TaxMatchingApplTagRule() {}

TaxMatchingApplTagRule::ApplicatorType
TaxMatchingApplTagRule::createApplicator(type::Index const& itinIndex,
                                         const Request& request,
                                         Services& services,
                                         RawPayments& /*itinPayments*/) const
{
  Itin const& itin = request.getItinByIndex(itinIndex);
  assert (itin.geoPath());
  assert (itin.geoPathMapping());

  return ApplicatorType(this,
                        itin,
                        services.mileageService());
}

std::string
TaxMatchingApplTagRule::getDescription(Services&) const
{
  return std::string("APPLY IF MATCHES WITH TAXMATCHINGAPPLTAG ") + _taxMatchingApplTag.asString();
}
}
