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

#include "Rules/TicketMinMaxValueApplicatorBase.h"

namespace tax
{
class TicketMinMaxValueOCRule;
class PaymentDetail;

class TicketMinMaxValueOCApplicator : public TicketMinMaxValueApplicatorBase
{
public:
  TicketMinMaxValueOCApplicator(const TicketMinMaxValueOCRule& rule,
                                const FarePath& farePath,
                                const CurrencyService& currencyService,
                                const type::CurrencyCode& paymentCurrency,
                                type::MoneyAmount totalYqYrAmount);

  virtual bool apply(PaymentDetail& paymentDetail) const override;
};

} // namespace tax
