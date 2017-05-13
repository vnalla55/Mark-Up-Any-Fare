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

#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{

class TicketMinMaxValueRuleBase : public BusinessRule
{
public:
  TicketMinMaxValueRuleBase(const type::TktValApplQualifier& tktValApplQualifier,
                            const type::CurrencyCode& tktValCurrency,
                            const type::IntMoneyAmount tktValMin,
                            const type::IntMoneyAmount tktValMax,
                            const type::CurDecimals tktValCurrDecimals);

  virtual ~TicketMinMaxValueRuleBase();

  virtual std::string getDescription(Services& services) const override;

  const type::TktValApplQualifier& tktValApplQualifier() const { return _tktValApplQualifier; }
  const type::CurrencyCode& tktValCurrency() const { return _tktValCurrency; }
  const type::MoneyAmount& tktValMin() const { return _tktValMin; }
  const type::MoneyAmount& tktValMax() const { return _tktValMax; }

private:
  type::TktValApplQualifier _tktValApplQualifier;
  type::CurrencyCode _tktValCurrency;
  type::MoneyAmount _tktValMin;
  type::MoneyAmount _tktValMax;
};
}
