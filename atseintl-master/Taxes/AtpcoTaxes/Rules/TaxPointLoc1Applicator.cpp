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
#include "DomainDataObjects/Geo.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TaxPointLoc1Applicator.h"
#include "Rules/TaxPointLoc1Rule.h"
#include "ServiceInterfaces/LocService.h"

namespace tax
{

TaxPointLoc1Applicator::TaxPointLoc1Applicator(TaxPointLoc1Rule const& rule,
                                               LocService const& locService)
  : BusinessRuleApplicator(&rule), _locService(locService), _taxPointLoc1Rule(rule)
{
}

TaxPointLoc1Applicator::~TaxPointLoc1Applicator() {}

bool
TaxPointLoc1Applicator::apply(PaymentDetail& paymentDetail) const
{
  const Geo& loc1 = paymentDetail.getTaxPointBegin();
  if (!_locService.isInLoc(
          loc1.locCode(), _taxPointLoc1Rule.locZone(), _taxPointLoc1Rule.vendor()))
  {
    for(OptionalService & optionalService : paymentDetail.optionalServiceItems())
    {
      if (OCUtil::isOCSegmentRelated(optionalService.type()) && !optionalService.isFailed())
      {
        optionalService.setFailedRule(&_taxPointLoc1Rule);
      }
    }

    TaxableYqYrs& taxableYqYrs = paymentDetail.getMutableYqYrDetails();
    for (type::Index i = 0; i < taxableYqYrs._subject.size(); ++i)
    {
      if (taxableYqYrs.isFailedRule(i))
        continue;

      taxableYqYrs.setFailedRule(i, _taxPointLoc1Rule);
    }

    paymentDetail.setFailedRule(&_taxPointLoc1Rule);
    return !paymentDetail.areAllOptionalServicesFailed() || !taxableYqYrs.areAllFailed();
  }

  return true;
}

} // namespace tax
