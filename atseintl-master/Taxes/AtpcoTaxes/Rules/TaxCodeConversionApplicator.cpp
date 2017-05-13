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
#include <cassert>

#include "Rules/TaxCodeConversionApplicator.h"
#include "Rules/TaxCodeConversionRule.h"
#include "Factories/MakeSabreCode.h"

namespace tax
{
TaxCodeConversionApplicator::TaxCodeConversionApplicator(const TaxCodeConversionRule& rule,
                                                         const Services& services)
  : BusinessRuleApplicator(&rule), _services(services)
{
}

bool
TaxCodeConversionApplicator::apply(PaymentDetail& paymentDetail) const
{
  const TaxName& taxName = paymentDetail.taxName();

  boost::optional<type::SabreTaxCode> taxCode;
  bool hasAnyApplicableOc = false;
  for (OptionalService& optionalService : paymentDetail.optionalServiceItems())
  {
    if (!taxCode)
    {
      taxCode = makeServiceSabreCode(taxName.taxCode());
    }
    optionalService.sabreTaxCode() = *taxCode;

    if (!optionalService.isFailed())
      hasAnyApplicableOc = true;
  }

  assert(!hasAnyApplicableOc || taxCode);
  paymentDetail.sabreTaxCode() =
      hasAnyApplicableOc
          ? *taxCode
          : makeItinSabreCode(taxName.taxCode(), taxName.taxType(), taxName.percentFlatTag());
  return true;
}

} // namespace tax
