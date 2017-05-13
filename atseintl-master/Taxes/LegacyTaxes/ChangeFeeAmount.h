// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PricingTrx.h"

#include <memory>

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class RefundPermutation;
class ReissueCharges;

class AbstractChangeFeeAmount
{
public:
  virtual MoneyAmount getAmountInPaymentCurrency() const = 0;
  virtual bool validate() const = 0;
  virtual ~AbstractChangeFeeAmount() {}
  static std::unique_ptr<const AbstractChangeFeeAmount> create(PricingTrx& trx,
      TaxResponse& taxResponse, TaxCodeReg& taxCodeReg,
      CurrencyCode paymentCurrency);
};

class RefundChangeFeeAmount : public AbstractChangeFeeAmount
{
public:
  RefundChangeFeeAmount(PricingTrx& trx, TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg, CurrencyCode paymentCurrency);
  MoneyAmount getAmountInPaymentCurrency() const override;
  bool validate() const override;

private:
  PricingTrx& _trx;
  TaxResponse& _taxResponse;
  TaxCodeReg& _taxCodeReg;
  CurrencyCode _paymentCurrency;
  const RefundPermutation* _permutation = nullptr;
};

class ReissueChangeFeeAmount : public AbstractChangeFeeAmount
{
public:
  ReissueChangeFeeAmount(PricingTrx& trx, TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg, CurrencyCode paymentCurrency);
  MoneyAmount getAmountInPaymentCurrency() const override;
  bool validate() const override;

private:
  PricingTrx& _trx;
  TaxResponse& _taxResponse;
  TaxCodeReg& _taxCodeReg;
  CurrencyCode _paymentCurrency;
  const ReissueCharges* _reissueCharges;
};

} // end of tse namespace
