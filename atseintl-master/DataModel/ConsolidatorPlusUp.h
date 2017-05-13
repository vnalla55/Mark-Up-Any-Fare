//-------------------------------------------------------------------
//
//  File:        ConsolidatorPlusUp.h
//  Created:     July 9, 2007
//  Design:      Marco Cartolano / Adrian Tovar
//  Authors:
//
//  Description: Consolidator plus up information for Plus Up Pricing
//
//  Updates:
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"


#include <set>

namespace tse
{

class PricingTrx;
class FarePath;
class DiagCollector;

class ConsolidatorPlusUp
{
public:
  static const char* TAX_CODE_RC;
  static const char* TAX_CODE_XG;
  static const char* TAX_CODE_XQ;
  static constexpr Indicator NET_SUBMIT_FARE = 'T';
  static constexpr Indicator NET_SUBMIT_FARE_UPD = 'C';

  ConsolidatorPlusUp();

  void initialize(PricingTrx& trx,
                  const MoneyAmount& amount,
                  const CurrencyCode& currencyCode,
                  const TktDesignator& tktDesignator);

  void addPlusUpToFarePath(PricingTrx& trx, FarePath*& farePath, DiagCollector* diag = nullptr);

  void addPlusUpToBaseFare(PricingTrx& trx,
                           const FarePath& farePath,
                           const CurrencyCode& baseFareCurrencyCode,
                           MoneyAmount& baseFare,
                           DiagCollector* diag = nullptr);

  MoneyAmount calcTaxablePlusUpAmount(PricingTrx& trx,
                                      TaxCode& taxCode,
                                      const FarePath* farePath,
                                      Percent taxableAmtPercent = 1.0);

  void qualifyFarePath(PricingTrx& trx, FarePath& farePath, DiagCollector* diag = nullptr);

  // Accessors
  MoneyAmount& amount() { return _amount; }
  const MoneyAmount& amount() const { return _amount; }

  CurrencyCode& currencyCode() { return _currencyCode; }
  const CurrencyCode& currencyCode() const { return _currencyCode; }

  TktDesignator& tktDesignator() { return _tktDesignator; }
  const TktDesignator& tktDesignator() const { return _tktDesignator; }

  MoneyAmount& fareCalcAmount() { return _fareCalcAmount; }
  const MoneyAmount& fareCalcAmount() const { return _fareCalcAmount; }

  std::set<TaxCode>& taxCodes() { return _taxCodes; }
  const std::set<TaxCode>& taxCodes() const { return _taxCodes; }

  bool isCanadianPointOfSale() { return _canadianPointOfSale; }
  const bool isCanadianPointOfSale() const { return _canadianPointOfSale; }

  bool isDiagHeaderAdded() { return _diagHeaderAdded; }
  const bool isDiagHeaderAdded() const { return _diagHeaderAdded; }

private:
  MoneyAmount _amount;
  MoneyAmount _fareCalcAmount;
  TktDesignator _tktDesignator;
  std::set<TaxCode> _taxCodes;
  CurrencyCode _currencyCode;
  bool _canadianPointOfSale;
  bool _diagHeaderAdded;
  bool _diagTaxesAdded;

  MoneyAmount convertAmount(PricingTrx& trx,
                            MoneyAmount& amount,
                            const CurrencyCode& sourceCurrency,
                            const CurrencyCode& targetCurrency,
                            ExchRate& exchangeRate);
};

inline ConsolidatorPlusUp::ConsolidatorPlusUp()
  : _amount(0.0),
    _fareCalcAmount(0.0),
    _canadianPointOfSale(false),
    _diagHeaderAdded(false),
    _diagTaxesAdded(false)
{
}

} // tse namespace

