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

#include "Rules/BusinessRuleApplicator.h"
#include "DataModel/Common/Types.h"

namespace tax
{

class CurrencyOfSaleRule;
class PaymentDetail;

class CurrencyOfSaleApplicator : public BusinessRuleApplicator
{
public:
  CurrencyOfSaleApplicator(const CurrencyOfSaleRule& rule,
                           const type::CurrencyCode& currencyOfSale,
                           const type::CurrencyCode& paymentCurrency,
                           const type::FormOfPayment& formOfPayment);

  ~CurrencyOfSaleApplicator();

  bool apply(PaymentDetail& /*paymentDetail*/) const;

private:
  const type::CurrencyCode& _currencyOfSale;
  const type::CurrencyCode& _paymentCurrency;
  const type::FormOfPayment& _formOfPayment;

  bool isMilesFormOfPayment() const;
};

} // namespace tax
