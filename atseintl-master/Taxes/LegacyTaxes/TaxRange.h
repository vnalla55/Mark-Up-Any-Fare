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

namespace tse
{
class PricingTrx;
class TaxResponse;
class TaxCodeReg;
class FarePath;

class TaxRange
{

public:
  static constexpr char FARE = 'F';
  static constexpr char MILEAGE = 'M';
  static constexpr char TRIP = 'T';
  static constexpr char TOTAL = 'A';
  static constexpr char STOP_OVER = 'S';
  static constexpr char DESTINATION = 'D';

  TaxRange() = default;
  virtual ~TaxRange() = default;

  virtual bool validateRange(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex) const;

  MoneyAmount applyRange(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         const MoneyAmount& taxAmount,
                         const CurrencyCode& paymentCurrency,
                         uint16_t& startIndex,
                         uint16_t& endIndex,
                         TaxCodeReg& taxCodeReg) const;

  MoneyAmount retrieveTripFare(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               FarePath& farePath,
                               const CurrencyCode& currencyCode,
                               uint16_t& startIndex) const;

  virtual MoneyAmount retrieveTotalFare(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                FarePath& farePath,
                                const CurrencyCode& currencyCode) const;

  uint32_t retrieveRangeMiles(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t& startIndex,
                              uint16_t& endIndex) const;

private:
  TaxRange(const TaxRange& calc) = delete;
  TaxRange& operator=(const TaxRange& calc) = delete;
};
}

