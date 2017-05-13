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
#include "Rules/TicketMinMaxValueOCApplicator.h"
#include "Rules/TicketMinMaxValueOCRule.h"

namespace tax
{
TicketMinMaxValueOCApplicator::TicketMinMaxValueOCApplicator(
    const TicketMinMaxValueOCRule& rule,
    const FarePath& farePath,
    const CurrencyService& currencyService,
    const type::CurrencyCode& paymentCurrency,
    type::MoneyAmount totalYqYrAmount)
  : TicketMinMaxValueApplicatorBase(
        rule, farePath, currencyService, paymentCurrency, totalYqYrAmount)
{
}

// TicketMinMaxValueOCApplicator::~TicketMinMaxValueOCApplicator() {}

bool
TicketMinMaxValueOCApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (paymentDetail.areAllOptionalServicesFailed())
  {
    return true;
  }

  if (paymentDetail.optionalServiceItems().empty())
  {
    return true;
  }

  const type::MoneyAmount baseFareWithFees =
      baseFareAmount() + totalYqYrAmount() + paymentDetail.changeFeeAmount();
  bool isValidForBaseFareWithFees =
      isWithinLimits(baseFareWithFees, paymentDetail.applicatorFailMessage());
  bool isValidBaseFare = isWithinLimits(baseFareAmount(), paymentDetail.applicatorFailMessage());

  for (OptionalService& oc : paymentDetail.optionalServiceItems())
  {
    if (oc.isFailed())
    {
      continue;
    }

    bool isValid = true;
    if (oc.type() == tax::type::OptionalServiceTag::FareRelated)
    {
      if (rule().tktValApplQualifier() == type::TktValApplQualifier::BaseFare)
      {
        isValid = isValidBaseFare;
      }
      else if (rule().tktValApplQualifier() == type::TktValApplQualifier::FareWithFees)
      {
        isValid = isValidForBaseFareWithFees;
      }
      else
      {
        paymentDetail.getMutableItineraryDetail().setFailedRule(&rule());
        return false;
      }
    }
    else
    {
      isValid = isWithinLimits(oc.amount(), paymentDetail.applicatorFailMessage());
    }

    if (!isValid)
    {
      oc.setFailedRule(&rule());
    }
  }

  return !paymentDetail.areAllOptionalServicesFailed();
}

} // namespace tax
