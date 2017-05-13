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

#include "Common/OCUtil.h"
#include "DomainDataObjects/OptionalService.h"
#include "ServiceInterfaces/LocService.h"
#include "Rules/TaxPointLoc3AsPreviousPointApplicator.h"
#include "Rules/TaxPointLoc3AsPreviousPointRule.h"
#include "Rules/TaxPointUtils.h"

namespace tax
{

TaxPointLoc3AsPreviousPointApplicator::TaxPointLoc3AsPreviousPointApplicator(
    const TaxPointLoc3AsPreviousPointRule& rule,
    const LocZone& locZone,
    const type::Vendor& vendor,
    const LocService& locService)
  : BusinessRuleApplicator(&rule),
    _locZone(locZone),
    _vendor(vendor),
    _locService(locService),
    _taxPointLoc3AsPreviousPointRule(rule)
{
}

TaxPointLoc3AsPreviousPointApplicator::~TaxPointLoc3AsPreviousPointApplicator() {}

bool
TaxPointLoc3AsPreviousPointApplicator::apply(PaymentDetail& paymentDetail) const
{
  const Geo& loc1 = paymentDetail.getTaxPointBegin();

  const Geo* loc3 = TaxPointIterator(loc1).decrement(paymentDetail.mustBeTicketed()).getGeo();
  paymentDetail.setLoc3(loc3);

  bool loc3match = ((loc3 != nullptr) && _locService.isInLoc(loc3->locCode(), _locZone, _vendor));

  TaxableYqYrs& taxableYqYrs = paymentDetail.getMutableYqYrDetails();
  for (type::Index i = 0; i < taxableYqYrs._subject.size(); ++i)
  {
    if (taxableYqYrs.isFailedRule(i))
      continue;

    taxableYqYrs._data[i]._taxPointLoc3 = loc3;

    if (!loc3match)
      taxableYqYrs.setFailedRule(i, _taxPointLoc3AsPreviousPointRule);
  }

  if (!loc3match)
  {
    for(OptionalService & optionalService : paymentDetail.optionalServiceItems())
    {
      if (OCUtil::isOCSegmentRelated(optionalService.type()) && !optionalService.isFailed())
      {
        optionalService.setFailedRule(&_taxPointLoc3AsPreviousPointRule);
      }
    }

    paymentDetail.setFailedRule(&_taxPointLoc3AsPreviousPointRule);
    return !paymentDetail.areAllOptionalServicesFailed() || !taxableYqYrs.areAllFailed();
  }

  return true;
}

} // namespace tax
