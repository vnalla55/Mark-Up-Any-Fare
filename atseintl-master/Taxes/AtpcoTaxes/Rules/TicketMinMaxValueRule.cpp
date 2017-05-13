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
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "Rules/TicketMinMaxValueApplicator.h"
#include "Rules/TicketMinMaxValueRule.h"
#include "ServiceInterfaces/Services.h"

namespace tax
{
TicketMinMaxValueRule::TicketMinMaxValueRule(const type::TktValApplQualifier& tktValApplQualifier,
                                             const type::CurrencyCode& tktValCurrency,
                                             const type::IntMoneyAmount tktValMin,
                                             const type::IntMoneyAmount tktValMax,
                                             const type::CurDecimals tktValCurrDecimals)
  : TicketMinMaxValueRuleBase(
        tktValApplQualifier, tktValCurrency, tktValMin, tktValMax, tktValCurrDecimals)
{
}

TicketMinMaxValueRule::~TicketMinMaxValueRule()
{
}

TicketMinMaxValueRule::ApplicatorType
TicketMinMaxValueRule::createApplicator(const type::Index& itinIndex,
                                        const Request& request,
                                        Services& services,
                                        RawPayments& /*itinPayments*/) const
{
  const Itin& itin = request.getItinByIndex(itinIndex);
  const type::Index& farePathRefId = itin.farePathRefId();

  type::MoneyAmount totalYqYrAmount = getYqyrAmountForItin(itin, request);

  return ApplicatorType(*this,
                        request.farePaths()[farePathRefId],
                        services.currencyService(),
                        request.ticketingOptions().paymentCurrency(),
                        totalYqYrAmount);
}
}
