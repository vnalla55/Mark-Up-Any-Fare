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
#include "Rules/PaymentDetail.h"
#include "Rules/TaxPointUtils.h"
#include "Rules/TaxPointLoc2CompareApplicator.h"
#include "Rules/TaxPointLoc2CompareRule.h"

namespace tax
{

TaxPointLoc2CompareApplicator::TaxPointLoc2CompareApplicator(const TaxPointLoc2CompareRule& rule)
  : BusinessRuleApplicator(&rule),
    _taxPointLoc2CompareRule(rule)
{
}

TaxPointLoc2CompareApplicator::~TaxPointLoc2CompareApplicator() {}

bool
TaxPointLoc2CompareApplicator::apply(PaymentDetail& paymentDetail) const
{
  bool mustBeTicketed = (_taxPointLoc2CompareRule.getTicketedPointTag() ==
                         type::TicketedPointTag::MatchTicketedPointsOnly);
  bool mustBeStop = (_taxPointLoc2CompareRule.getTaxPointLoc2Compare() ==
                     type::TaxPointLoc2Compare::Stopover);

  TaxPointUtil tUtil(paymentDetail);
  TaxPointIterator pIt(paymentDetail.getTaxPointLoc1());

  const Geo* prevNextGeo = nullptr;
  while (pIt.decrement(mustBeStop || mustBeTicketed).isValid())
  {
    if ((mustBeStop && tUtil.isStop(*pIt.getGeo())) || !mustBeStop)
    {
      prevNextGeo = pIt.getGeo();
      break;
    }
  }

  for(OptionalService& optionalService : paymentDetail.optionalServiceItems())
  {
    if (OCUtil::isOCSegmentRelated(optionalService.type()) && !optionalService.isFailed())
    {
      if (!prevNextGeo ||
          (prevNextGeo->cityCode() == optionalService.getTaxPointLoc2().cityCode()))
        optionalService.setFailedRule(&_taxPointLoc2CompareRule);
    }
  }

  TaxableYqYrs& taxableYqYrs = paymentDetail.getMutableYqYrDetails();
  for (type::Index i = 0; i < taxableYqYrs._subject.size(); ++i)
  {
    if (taxableYqYrs.isFailedRule(i))
      continue;

    if (!prevNextGeo ||
        (prevNextGeo->cityCode() == taxableYqYrs._data[i]._taxPointLoc2->cityCode()))
      taxableYqYrs.setFailedRule(i, _taxPointLoc2CompareRule);
  }

  if (!prevNextGeo ||
      (prevNextGeo->cityCode() == paymentDetail.getTaxPointLoc2().cityCode()))
    return !taxableYqYrs.areAllFailed() || !paymentDetail.areAllOptionalServicesFailed();

  return true;
}

} // namespace tax
