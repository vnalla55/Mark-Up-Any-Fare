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

#include <sstream>
#include "Rules/PaymentDetail.h"
#include "Rules/TicketMinMaxValueApplicator.h"
#include "Rules/TicketMinMaxValueRule.h"

namespace tax
{
TicketMinMaxValueApplicator::TicketMinMaxValueApplicator(const TicketMinMaxValueRule& rule,
                                                         const FarePath& farePath,
                                                         const CurrencyService& currencyService,
                                                         const type::CurrencyCode& paymentCurrency,
                                                         type::MoneyAmount totalYqYrAmount)
  : TicketMinMaxValueApplicatorBase(
        rule, farePath, currencyService, paymentCurrency, totalYqYrAmount)
{
}

bool
TicketMinMaxValueApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (paymentDetail.getItineraryDetail().isFailedRule())
  {
    return true;
  }

  type::MoneyAmount amount = 0;
  if (rule().tktValApplQualifier() == type::TktValApplQualifier::BaseFare)
  {
    amount = baseFareAmount();
  }
  else if (rule().tktValApplQualifier() == type::TktValApplQualifier::FareWithFees)
  {
    amount = baseFareAmount() + totalYqYrAmount() + paymentDetail.changeFeeAmount();
  }
  else
  {
    paymentDetail.getMutableItineraryDetail().setFailedRule(&rule());
    return false;
  }

  if (!isWithinLimits(amount, paymentDetail.applicatorFailMessage()))
  {
    paymentDetail.getMutableItineraryDetail().setFailedRule(&rule());
  }

  return !paymentDetail.isFailedRule();
}

} // namespace tax
