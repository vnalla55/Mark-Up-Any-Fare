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
#include <cassert>

#include "Common/OCUtil.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TaxPointLoc2Applicator.h"
#include "Rules/TaxPointLoc2Rule.h"
#include "ServiceInterfaces/LocService.h"

namespace tax
{

TaxPointLoc2Applicator::TaxPointLoc2Applicator(
    const TaxPointLoc2Rule& rule,
    const LocService& locService)
  : BusinessRuleApplicator(&rule),
    _rule(rule),
    _locService(locService)
{
}

TaxPointLoc2Applicator::~TaxPointLoc2Applicator() {}

bool
TaxPointLoc2Applicator::apply(PaymentDetail& paymentDetail) const
{
  for (OptionalService& optionalService : paymentDetail.optionalServiceItems())
  {
    if (OCUtil::isOCSegmentRelated(optionalService.type()) && !optionalService.isFailed())
    {
      const Geo& geo = optionalService.getTaxPointLoc2();

      if ((paymentDetail.ticketedPointTag() == type::TicketedPointTag::MatchTicketedPointsOnly &&
           geo.unticketedTransfer() == type::UnticketedTransfer::Yes) ||
          !_locService.isInLoc(geo.loc().code(), _rule.getLocZone(), _rule.getVendor()))
      {
        optionalService.setFailedRule(&_rule);
      }
    }
  }

  TaxableYqYrs& taxableYqYrs = paymentDetail.getMutableYqYrDetails();
  for (type::Index i = 0; i < taxableYqYrs._subject.size(); ++i)
  {
    if (taxableYqYrs.isFailedRule(i))
      continue;

    if (!_locService.isInLoc(taxableYqYrs._data[i]._taxPointLoc2->loc().code(),
        _rule.getLocZone(), _rule.getVendor()))
      taxableYqYrs.setFailedRule(i, _rule);
  }

  if (_locService.isInLoc(paymentDetail.getTaxPointLoc2().loc().code(),
      _rule.getLocZone(), _rule.getVendor()))
    return true;

  paymentDetail.setFailedRule(&_rule);
  return !taxableYqYrs.areAllFailed() || !paymentDetail.areAllOptionalServicesFailed();
}

} // namespace tax
