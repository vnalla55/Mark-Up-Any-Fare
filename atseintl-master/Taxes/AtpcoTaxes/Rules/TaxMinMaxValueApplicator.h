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
#include "Rules/BusinessRuleApplicator.h"

namespace tax
{

class BusinessRule;
class Services;
class PaymentDetail;
class TaxMinMaxValueRule;

class TaxMinMaxValueApplicator : public BusinessRuleApplicator
{
public:
  TaxMinMaxValueApplicator(const TaxMinMaxValueRule* rule,
                           const Services& services,
                           const type::CurrencyCode& paymentCurrency);

  ~TaxMinMaxValueApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const Services& _services;
  TaxMinMaxValueRule const* _taxMinMaxValueRule;
  const type::CurrencyCode& _paymentCurrency;
};

} // namespace tax
