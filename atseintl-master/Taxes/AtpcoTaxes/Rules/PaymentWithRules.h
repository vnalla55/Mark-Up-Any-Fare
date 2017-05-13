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

namespace tax
{
struct CalculatorsGroups;
class PaymentDetail;

struct PaymentWithRules
{
  PaymentWithRules(PaymentDetail&,
                   const CalculatorsGroups*);

  PaymentDetail* paymentDetail;
  const CalculatorsGroups* calculatorsGroups;

  PaymentDetail& getPaymentDetail()
  {
    return *paymentDetail;
  }

  const PaymentDetail& getPaymentDetail() const
  {
    return *paymentDetail;
  }

  type::Index getGeoId() const
  {
    return getPaymentDetail().getTaxPointBegin().id();
  }

  type::MoneyAmount
  getAmount() const
  {
    return paymentDetail->taxAmt();
  }
};

}

