//----------------------------------------------------------------------------
//
//  File:        CurrencyUtil.h
//
//  Description: Common Currency operations
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/CurrencyConversionRequest.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <string>

namespace tse
{
class Loc;
class PricingTrx;
class Trx;
class Logger;

namespace
{
using ConversionType = CurrencyConversionRequest::ApplicationType;
}

class CurrencyUtil
{
public:
  static const bool getNationCurrency(const NationCode& nationCode,
                                      CurrencyCode& nationCurrency,
                                      const DateTime& ticketingDate);

  static const bool getConversionCurrency(const NationCode& nationCode,
                                          CurrencyCode& conversionCurrency,
                                          const DateTime& ticketingDate);

  static const bool getPricingCurrency(const NationCode& nationCode,
                                       CurrencyCode& pricingCurrency,
                                       const DateTime& ticketingDate);

  static const bool getNationalCurrency(const NationCode& nationCode,
                                        CurrencyCode& nationCurrency,
                                        const DateTime& ticketingDate);

  static const bool matchNationCurrency(const NationCode& nationCode,
                                        const CurrencyCode& sourceCurrency,
                                        const DateTime& ticketingDate);

  static const bool getSaleLocOverrideCurrency(const LocCode& overrideLoc,
                                               CurrencyCode& salesOverrideCurrency,
                                               const DateTime& ticketingDate);

  static bool isRestricted(PricingTrx& trx, const Loc* salesLoc, CurrencyCode& overrideCurrency);

  static bool formatDoubleWithNoDec(const double value, std::string& target, int noDecimals);

  static void formatDouble(const double& value, std::string& target, int noDecimals, bool removePeriodWhenInt = false);

  static const bool getDomesticPricingCurrency(const NationCode& nationCode,
                                               CurrencyCode& pricingCurrency,
                                               const DateTime& ticketingDate);

  static const bool getInternationalPricingCurrency(const NationCode& nationCode,
                                                    CurrencyCode& pricingCurrency,
                                                    const DateTime& ticketingDate);

  static std::string toString(const MoneyAmount amt, const CurrencyNoDec noDec);

  static const std::string NULL_CURRENCY;
  static const std::string& getControllingNationDesc(Trx& trx, const CurrencyCode& currency,
                                                     const DateTime& date);

  static void getControllingNationCode(Trx& trx,
                                       const std::string& countryDescription,
                                       NationCode& nationCode,
                                       bool& foundNation,
                                       bool& foundNationalCurrency,
                                       NationCode& nationWithMatchingNationalCurrency,
                                       const DateTime& dcDate,
                                       const CurrencyCode& targetCurrency);

  static void truncateNonNUCAmount(MoneyAmount& amount, unsigned noDec);
  static void truncateNUCAmount(MoneyAmount& amount);
  static void halveNUCAmount(MoneyAmount& amount);

  static MoneyAmount convertMoneyAmount(const MoneyAmount& sourceAmount,
                                        const CurrencyCode& sourceCurrency,
                                        const CurrencyCode& targetCurrency,
                                        PricingTrx& trx,
                                        ConversionType type = CurrencyConversionRequest::OTHER);

  static MoneyAmount convertMoneyAmount_OLD(const MoneyAmount& sourceAmount,
                                            const CurrencyCode& sourceCurrency,
                                            const CurrencyCode& targetCurrency,
                                            PricingTrx& _trx,
                                            ConversionType type = CurrencyConversionRequest::OTHER);

  static MoneyAmount convertMoneyAmount(const MoneyAmount& sourceAmount,
                                        const CurrencyCode& sourceCurrency,
                                        const CurrencyCode& targetCurrency,
                                        const DateTime& conversionDate,
                                        PricingTrx& trx,
                                        ConversionType type = CurrencyConversionRequest::OTHER);

  static MoneyAmount getMoneyAmountInNUC(const MoneyAmount& sourceAmount,
                                         const CurrencyCode& sourceCurrency,
                                         const DateTime& conversionDate,
                                         PricingTrx& trx,
                                         ConversionType type = CurrencyConversionRequest::OTHER);
  static void validateCurrencyCode(const PricingTrx& trx, const CurrencyCode& cur);

private:
  static Logger _logger;
};

} //tse

