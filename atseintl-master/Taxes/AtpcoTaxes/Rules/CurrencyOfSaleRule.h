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

#include "Rules/BusinessRule.h"
#include "DataModel/Common/Types.h"

namespace tax
{
class CurrencyOfSaleApplicator;
class Request;

class CurrencyOfSaleRule : public BusinessRule
{
public:
  typedef CurrencyOfSaleApplicator ApplicatorType;
  CurrencyOfSaleRule(const type::CurrencyCode& currencyOfSale);
  virtual ~CurrencyOfSaleRule();

  ApplicatorType createApplicator(const type::Index& /*itinIndex*/,
                                  const Request& request,
                                  Services& /*services*/,
                                  RawPayments& /*itinPayments*/) const;

  virtual std::string getDescription(Services& services) const override;

private:
  type::CurrencyCode _currencyOfSale;
};
}
