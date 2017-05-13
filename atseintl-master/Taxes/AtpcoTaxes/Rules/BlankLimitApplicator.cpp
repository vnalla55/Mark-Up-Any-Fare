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
#include "BlankLimitApplicator.h"
#include "BlankLimitRule.h"
#include "PaymentDetail.h"
#include "DomainDataObjects/GeoPath.h"


namespace tax
{

BlankLimitApplicator::BlankLimitApplicator(const BlankLimitRule* parent,
                                           RawPayments& rawPayments)
  : BusinessRuleApplicator(parent), _rawPayments(rawPayments)
{
}

BlankLimitApplicator::~BlankLimitApplicator() {}

bool
BlankLimitApplicator::apply(PaymentDetail& paymentDetail) const
{
  for (RawPayments::value_type& rawPayment : _rawPayments)
  {
    if (rawPayment.detail.isCalculated() && !rawPayment.detail.getItineraryDetail().isFailedRule() &&
        paymentDetail.taxName() == *rawPayment.taxName)
    {
      if (rawPayment.detail.taxApplicationLimit() == type::TaxApplicationLimit::OnceForItin ||
          rawPayment.detail.taxApplicationLimit() ==
              type::TaxApplicationLimit::OncePerSingleJourney)
      {
        if (paymentDetail.taxAmt() > rawPayment.detail.taxAmt())
        {
          rawPayment.detail.getMutableItineraryDetail().setFailedRule(getBusinessRule());
          return true;
        }
      }
    }
  }

  return true;
}
}
