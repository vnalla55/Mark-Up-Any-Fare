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
#pragma once

#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{
class Request;
class Services;
class TaxMinMaxValueApplicator;

class TaxMinMaxValueRule : public BusinessRule
{
public:
  typedef TaxMinMaxValueApplicator ApplicatorType;
  TaxMinMaxValueRule(const type::CurrencyCode& taxValCurrency,
                     const type::IntMoneyAmount taxValMin,
                     const type::IntMoneyAmount taxValMax,
                     const type::CurDecimals& taxValCurrDecimals);
  virtual ~TaxMinMaxValueRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& /*itinIndex*/,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  const type::MoneyAmount& taxValMin() const { return _taxValMin; }
  const type::MoneyAmount& taxValMax() const { return _taxValMax; }
  const type::CurrencyCode& taxValCurrency() const { return _taxValCurrency; }
  const type::CurDecimals& taxValCurrencyDecimals() const { return _taxValCurrencyDecimals; }

private:
  type::CurrencyCode _taxValCurrency;
  type::CurDecimals _taxValCurrencyDecimals;
  type::MoneyAmount _taxValMin;
  type::MoneyAmount _taxValMax;
};
}
