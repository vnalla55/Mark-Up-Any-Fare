// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/YqyrUtil.h"
#include "DomainDataObjects/YqYr.h"
#include "Rules/PaymentDetail.h"
#include "Rules/YqYrAmountApplicator.h"
#include "Rules/YqYrAmountRule.h"
#include "ServiceInterfaces/MileageGetter.h"

namespace tax
{

YqYrAmountApplicator::YqYrAmountApplicator(const YqYrAmountRule& rule,
                                           const MileageGetter& mileageGetter,
                                           const GeoPath& geoPath)
  : BusinessRuleApplicator(&rule), _rule(rule), _geoPath(geoPath), _prorateCalculator(mileageGetter)
{
}

bool
YqYrAmountApplicator::apply(PaymentDetail& paymentDetail) const
{
  bool skipHiddenPoints =
      paymentDetail.ticketedPointTag() == type::TicketedPointTag::MatchTicketedPointsOnly;
  TaxableYqYrs& yqYrDetails = paymentDetail.getMutableYqYrDetails();

  yqYrDetails._taxableAmount = getTaxableAmount(_prorateCalculator,
                                                paymentDetail.taxAppliesToTagInd(),
                                                paymentDetail.getTaxPointBegin().id(),
                                                yqYrDetails,
                                                _geoPath,
                                                skipHiddenPoints);

  return true;
}
} // namespace tax
