// ---------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FarePath.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"

#include <log4cxx/helpers/objectptr.h>
namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse

{
class PricingTrx;
class Money;
class CurrencyConversionFacade;
class TaxSpecConfigReg;

namespace YQYR
{

class ServiceFeeRec1Validator;

class Tax
{
  friend class UTCHiddenTest;

public:
  static constexpr char YES = 'Y';
  static constexpr char PERCENTAGE = 'P';
  static constexpr char FIXED = 'F';
  static constexpr char APPLIED = 'C';
  static constexpr char BOOKLET = 'B';
  static constexpr bool CHECK_SPN = true;

  enum HandleHiddenPoints
  {
    HIDDEN_POINT_NOT_HANDLED = 0,
    HIDDEN_POINT_LOC1,
    HIDDEN_POINT_LOC2,
    HIDDEN_POINT_BOTH_LOCS
  };

  Tax() = default;
  virtual ~Tax() {}

  const MoneyAmount& taxAmount() const { return _taxAmount; }
  const MoneyAmount& taxableFare() const { return _taxableFare; }
  const uint16_t& travelSegStartIndex() const { return _travelSegStartIndex; }
  const uint16_t& travelSegEndIndex() const { return _travelSegEndIndex; }
  const MoneyAmount& taxablePartialFare() const { return _taxablePartialFare; }
  const uint16_t& paymentCurrencyNoDec() const { return _paymentCurrencyNoDec; }
  const CurrencyCode& paymentCurrency() const { return _paymentCurrency; }
  const CurrencyCode& intermediateCurrency() const { return _intermediateCurrency; }
  const CurrencyNoDec& intermediateNoDec() const { return _intermediateNoDec; }
  const ExchRate& exchangeRate1() const { return _exchangeRate1; }
  const CurrencyNoDec& exchangeRate1NoDec() const { return _exchangeRate1NoDec; }
  const ExchRate& exchangeRate2() const { return _exchangeRate2; }
  const CurrencyNoDec& exchangeRate2NoDec() const { return _exchangeRate2NoDec; }
  const MoneyAmount& intermediateUnroundedAmount() const { return _intermediateUnroundedAmount; }
  const MoneyAmount& intermediateAmount() const { return _intermediateAmount; }
  const std::vector<TaxSpecConfigReg*>* taxSpecConfig() const { return nullptr; }

  virtual MoneyAmount
  calculateFareDependendTaxableAmount(PricingTrx& trx,
                                      TaxResponse& taxResponse,
                                      tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1,
                                      CurrencyConversionFacade& ccFacade);
  virtual MoneyAmount
  calculateChangeFeeTaxableAmount(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1,
                                  CurrencyConversionFacade& ccFacade);

protected:
  virtual MoneyAmount fareAmountInNUC(const PricingTrx& trx, const TaxResponse& taxResponse);

  MoneyAmount _taxAmount = 0;
  MoneyAmount _taxableFare = 0;
  MoneyAmount _nucTaxableFare = 0;

  CurrencyCode _paymentCurrency;
  uint16_t _paymentCurrencyNoDec = 0;

  uint16_t _travelSegStartIndex = 0;
  uint16_t _travelSegEndIndex = 0;
  MoneyAmount _taxablePartialFare = 0;
  CurrencyCode _intermediateCurrency;
  CurrencyNoDec _intermediateNoDec = 0;
  ExchRate _exchangeRate1 = 0;
  CurrencyNoDec _exchangeRate1NoDec = 0;
  ExchRate _exchangeRate2 = 0;
  CurrencyNoDec _exchangeRate2NoDec = 0;
  MoneyAmount _intermediateUnroundedAmount = 0;
  MoneyAmount _intermediateAmount = 0;

private:
  Tax(const Tax& map);
  Tax& operator=(const Tax& map);

  static log4cxx::LoggerPtr _logger;
};

inline MoneyAmount
Tax::fareAmountInNUC(const PricingTrx& trx, const TaxResponse& taxResponse)
{
  return taxResponse.farePath()->getTotalNUCAmount();
}
} // YQYR
} // tse
