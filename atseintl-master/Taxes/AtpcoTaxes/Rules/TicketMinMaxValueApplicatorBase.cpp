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

#include "Rules/BasePathUtils.h"
#include "Rules/TicketMinMaxValueApplicatorBase.h"
#include "Rules/TicketMinMaxValueRuleBase.h"

#include <sstream>
#include "DataModel/Common/CodeIO.h"
#include "ServiceInterfaces/CurrencyService.h"

namespace tax
{
namespace
{
bool
isWithinLowerLimit(const TicketMinMaxValueRuleBase& rule, const type::MoneyAmount& amount)
{
  return amount >= rule.tktValMin();
}

bool
isWithinUpperLimit(const TicketMinMaxValueRuleBase& rule, const type::MoneyAmount& amount)
{
  bool noMaxLimit = rule.tktValMin() && rule.tktValMax() == 0;
  return noMaxLimit || amount <= rule.tktValMax();
}
}

TicketMinMaxValueApplicatorBase::TicketMinMaxValueApplicatorBase(
    const TicketMinMaxValueRuleBase& rule,
    const FarePath& farePath,
    const CurrencyService& currencyService,
    const type::CurrencyCode& paymentCurrency,
    type::MoneyAmount totalYqYrAmount)
  : BusinessRuleApplicator(&rule),
    _tktMinMaxValueRuleBase(rule),
    _baseFareAmount(tax::BasePathUtils::baseFareAmount(farePath)),
    _currencyService(currencyService),
    _paymentCurrency(paymentCurrency),
    _totalYqYrAmount(totalYqYrAmount)
{
}

TicketMinMaxValueApplicatorBase::~TicketMinMaxValueApplicatorBase()
{
}

bool
TicketMinMaxValueApplicatorBase::isWithinLimits(const type::MoneyAmount& amount,
                                                boost::optional<std::string&> messageOutput) const
    try
{
  type::Money moneyToConvert = {amount, paymentCurrency()};
  type::MoneyAmount amountConverted =
      currencyService().convertTo(rule().tktValCurrency(), moneyToConvert);

  return isWithinLowerLimit(rule(), amountConverted) && isWithinUpperLimit(rule(), amountConverted);
}
catch (std::runtime_error const& exception)
{
  if (messageOutput)
  {
    std::ostringstream msg;
    msg << "CONVERSION " << paymentCurrency() << "-" << rule().tktValCurrency() << " FAILED";

    *messageOutput = msg.str();
  }
  return false;
}

} // namespace tax
