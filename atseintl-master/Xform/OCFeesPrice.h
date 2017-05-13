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

#include "Common/TsePrimitiveTypes.h"

#include <boost/function.hpp>

#include <vector>

namespace tse
{

class DataHandle;
class Money;
class OCFees;
class OCFeesUsage;

class OCFeesPrice
{
  const PricingTrx& _trx;

public:
  OCFeesPrice(const PricingTrx& trx) : _trx(trx) {}

  static OCFeesPrice*
  create(const OCFees& ocFees, const PricingTrx& trx, DataHandle& dataHandle);

  static OCFeesPrice*
  create(const OCFeesUsage& ocFeesUsage, const PricingTrx& trx, DataHandle& dataHandle);

  virtual MoneyAmount
  getFeeAmountInSellingCurrencyPlusTaxes(const OCFees& ocFees,
                                         const Money& targetMoney);

  virtual MoneyAmount
  getFeeAmountInSellingCurrencyPlusTaxes(const OCFeesUsage& ocFeesUsage,
                                         const Money& targetMoney);

  virtual MoneyAmount
  getFeeAmountInSellingCurrencyPlusTaxes(const OCFees& ocFees,
                                         const MoneyAmount& moneyAmount) = 0;

  virtual MoneyAmount
  getFeeAmountInSellingCurrencyPlusTaxes(const OCFeesUsage& ocFeesUsage,
                                         const MoneyAmount& moneyAmount);

  virtual MoneyAmount
  getBasePrice(const OCFees& ocFees, const MoneyAmount& feeAmount) = 0;

  virtual MoneyAmount
  getBasePrice(const OCFeesUsage& ocFeesUsage, const MoneyAmount& feeAmount);

  virtual MoneyAmount
  getEquivalentBasePrice(const OCFees& ocFees, const MoneyAmount& amount) = 0;

  virtual MoneyAmount
  getEquivalentBasePrice(const OCFeesUsage& ocFeesUsage, const MoneyAmount& amount);

protected:

  bool
  isExemptAllTaxes();

  bool
  isExemptSpecificTaxes();

  const std::vector<std::string>&
  getTaxIdExempted();

  virtual MoneyAmount
  sumUpTaxes(const OCFeesUsage* ocFeesUsage);

  MoneyAmount
  sumUpTaxes(const OCFees* ocFees);

  MoneyAmount
  sumUpTaxes(const std::vector<OCFees::TaxItem>& taxItems,
             const boost::function<MoneyAmount(const OCFees::TaxItem&)>& taxAmountCalc);

  bool
  isTaxExempted(const std::string& taxCode);

  std::vector<OCFees::TaxItem>::const_iterator
  setEndTaxOnOcIterator(const std::vector<OCFees::TaxItem>& taxItems);
};

/*
 * RegularOCFeesPrice uses tax related data from OCFeesSeg.
 */
class RegularOCFeesPrice : public OCFeesPrice
{
public:
  RegularOCFeesPrice(const PricingTrx& trx) : OCFeesPrice(trx) {}

  MoneyAmount
  getFeeAmountInSellingCurrencyPlusTaxes(const OCFees& ocFees,
                                         const Money& targetMoney) override;

  MoneyAmount
  getFeeAmountInSellingCurrencyPlusTaxes(const OCFeesUsage& ocFeesUsage,
                                         const Money& targetMoney) override;

  MoneyAmount
  getFeeAmountInSellingCurrencyPlusTaxes(const OCFees& ocFees,
                                         const MoneyAmount& moneyAmount) override;

  MoneyAmount
  getFeeAmountInSellingCurrencyPlusTaxes(const OCFeesUsage& ocFeesUsage,
                                         const MoneyAmount& moneyAmount) override;

  MoneyAmount
  getBasePrice(const OCFees& ocFees, const MoneyAmount& feeAmount) override;

  MoneyAmount
  getBasePrice(const OCFeesUsage& ocFeesUsage, const MoneyAmount& feeAmount) override;

  MoneyAmount
  getEquivalentBasePrice(const OCFees& ocFees, const MoneyAmount& amount) override;

  MoneyAmount
  getEquivalentBasePrice(const OCFeesUsage& ocFeesUsage, const MoneyAmount& amount) override;

protected:
  MoneyAmount
  sumUpTaxes(const OCFeesUsage* ocFeesUsage) override;
};

/*
 * BackingOutTaxesOCFeesPrice uses tax related data from OCFeesSeg
 */
class BackingOutTaxesOCFeesPrice : public OCFeesPrice
{
public:
  BackingOutTaxesOCFeesPrice(const PricingTrx& trx) : OCFeesPrice(trx) {}

  MoneyAmount
  getFeeAmountInSellingCurrencyPlusTaxes(const OCFees& ocFees,
                                         const Money& targetMoney) override;

  MoneyAmount
  getFeeAmountInSellingCurrencyPlusTaxes(const OCFeesUsage& ocFeesUsage,
                                         const Money& targetMoney) override;

  MoneyAmount
  getFeeAmountInSellingCurrencyPlusTaxes(const OCFees& ocFees,
                                         const MoneyAmount& moneyAmount) override;

  MoneyAmount
  getFeeAmountInSellingCurrencyPlusTaxes(const OCFeesUsage& ocFeesUsage,
                                         const MoneyAmount& moneyAmount) override;

  MoneyAmount
  getBasePrice(const OCFees& ocFees, const MoneyAmount& feeAmount) override;

  MoneyAmount
  getBasePrice(const OCFeesUsage& ocFeesUsage, const MoneyAmount& feeAmount) override;

  MoneyAmount
  getEquivalentBasePrice(const OCFees& ocFees, const MoneyAmount& amount) override;

  MoneyAmount
  getEquivalentBasePrice(const OCFeesUsage& ocFeesUsage, const MoneyAmount& amount) override;

protected:
  MoneyAmount
  sumUpTaxes(const OCFeesUsage* ocFeesUsage) override;
};
}
