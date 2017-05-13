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

#include "Common/LocationUtil.h"
#include "Common/OCUtil.h"
#include "DomainDataObjects/GeoPath.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TaxPointLoc2InternationalDomesticRule.h"
#include "Rules/TaxPointLoc2InternationalDomesticApplicator.h"

namespace tax
{

TaxPointLoc2InternationalDomesticApplicator::TaxPointLoc2InternationalDomesticApplicator(
    const TaxPointLoc2InternationalDomesticRule& rule)
  : BusinessRuleApplicator(&rule), _rule(rule)
{
}

TaxPointLoc2InternationalDomesticApplicator::~TaxPointLoc2InternationalDomesticApplicator() {}

bool
TaxPointLoc2InternationalDomesticApplicator::matches(const type::Nation& nation, const Geo& geo)
    const
{
  return ((_rule.intlDomInd() == type::IntlDomInd::Domestic &&
           LocationUtil::isDomestic(nation, geo.getNation()))
          ||
          (_rule.intlDomInd() == type::IntlDomInd::International &&
           LocationUtil::isInternational(nation, geo.getNation())));
}

bool
TaxPointLoc2InternationalDomesticApplicator::apply(PaymentDetail& paymentDetail) const
{
  const type::Nation& nation = paymentDetail.getTaxPointLoc1().loc().nation();

  TaxableYqYrs& taxableYqYrs = paymentDetail.getMutableYqYrDetails();
  for (type::Index i = 0; i < taxableYqYrs._subject.size(); ++i)
  {
    if (taxableYqYrs.isFailedRule(i))
      continue;

    if (!matches(nation, *taxableYqYrs._data[i]._taxPointLoc2))
      taxableYqYrs.setFailedRule(i, _rule);
  }

  bool matchedLoc2 = matches(nation, paymentDetail.getTaxPointLoc2());

  for(OptionalService & optionalService : paymentDetail.optionalServiceItems())
  {
    if (OCUtil::isOCSegmentRelated(optionalService.type()) && !optionalService.isFailed())
    {
      if (!matches(nation, optionalService.getTaxPointLoc2()))
      {
        optionalService.setFailedRule(&_rule);
      }
    }
  }

  if (!matchedLoc2)
  {
    paymentDetail.setFailedRule(&_rule);
    return !paymentDetail.areAllOptionalServicesFailed() || !taxableYqYrs.areAllFailed();
  }

  return true;
}

} // namespace tax
