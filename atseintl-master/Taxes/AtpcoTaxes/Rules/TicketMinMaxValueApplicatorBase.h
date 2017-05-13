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

#include <boost/optional.hpp>
#include "DataModel/Common/Types.h"
#include "Rules/BusinessRuleApplicator.h"

namespace tax
{
class BusinessRule;
class CurrencyService;
class FarePath;
class PaymentDetail;
class TicketMinMaxValueRuleBase;

class TicketMinMaxValueApplicatorBase : public BusinessRuleApplicator
{
public:
  TicketMinMaxValueApplicatorBase(const TicketMinMaxValueRuleBase& rule,
                                  const FarePath& farePath,
                                  const CurrencyService& currencyService,
                                  const type::CurrencyCode& paymentCurrency,
                                  type::MoneyAmount totalYqYrAmount);

  virtual ~TicketMinMaxValueApplicatorBase();

  virtual bool apply(PaymentDetail& paymentDetail) const = 0;

  const TicketMinMaxValueRuleBase& rule() const { return _tktMinMaxValueRuleBase; };
  const type::MoneyAmount& baseFareAmount() const { return _baseFareAmount; };
  const CurrencyService& currencyService() const { return _currencyService; };
  const type::CurrencyCode& paymentCurrency() const { return _paymentCurrency; };
  const type::MoneyAmount& totalYqYrAmount() const { return _totalYqYrAmount; };

  bool isWithinLimits(const type::MoneyAmount& amount,
                      boost::optional<std::string&> messageOutput) const;

private:
  const TicketMinMaxValueRuleBase& _tktMinMaxValueRuleBase;
  const type::MoneyAmount _baseFareAmount;
  const CurrencyService& _currencyService;
  const type::CurrencyCode& _paymentCurrency;
  type::MoneyAmount _totalYqYrAmount;
};

} // namespace tax
