// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "DataModel/Common/Types.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TaxOnTicketingFeeApplicator.h"
#include "Rules/TaxOnTicketingFeeRule.h"
#include "ServiceInterfaces/CurrencyService.h"

namespace tax
{
TaxOnTicketingFeeApplicator::TaxOnTicketingFeeApplicator(TaxOnTicketingFeeRule const& parent,
                                                         const CurrencyService& currencyService)
      : BusinessRuleApplicator(&parent),
    _taxOnTicketingFeeRule(parent),
    _currencyService(currencyService)
{
}

void
TaxOnTicketingFeeApplicator::applyPercent(PaymentDetail& paymentDetail) const
{
  for(TicketingFee& ticketingFee : paymentDetail.ticketingFees())
  {
    ticketingFee.taxAmount() = ticketingFee.amount() * paymentDetail.taxAmt();
  }
}

void
TaxOnTicketingFeeApplicator::applyFlat(PaymentDetail& paymentDetail) const
{
  for(TicketingFee& ticketingFee : paymentDetail.ticketingFees())
  {
    type::Money taxMoney = { paymentDetail.taxAmt(), paymentDetail.taxCurrency() };
    try
    {
      ticketingFee.taxAmount() = _currencyService.convertTo(paymentDetail.taxEquivalentCurrency(), taxMoney);
    }
    catch(const std::runtime_error&)
    {
      ticketingFee.taxAmount() = paymentDetail.taxAmt();
    }
  }
}

bool
TaxOnTicketingFeeApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (_taxOnTicketingFeeRule.isPercent())
    applyPercent(paymentDetail);
  else
    applyFlat(paymentDetail);

  return true;
}
} // end of tax namespace
