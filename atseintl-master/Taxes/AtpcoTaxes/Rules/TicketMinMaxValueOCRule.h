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

#pragma once

#include "Rules/TicketMinMaxValueRuleBase.h"

namespace tax
{
class Request;
class TicketMinMaxValueOCApplicator;

class TicketMinMaxValueOCRule : public TicketMinMaxValueRuleBase
{
public:
  typedef TicketMinMaxValueOCApplicator ApplicatorType;
  TicketMinMaxValueOCRule(const type::TktValApplQualifier& tktValApplQualifier,
                          const type::CurrencyCode& tktValCurrency,
                          const type::IntMoneyAmount tktValMin,
                          const type::IntMoneyAmount tktValMax,
                          const type::CurDecimals tktValCurrDecimals);

  virtual ~TicketMinMaxValueOCRule();

  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;
};
}
