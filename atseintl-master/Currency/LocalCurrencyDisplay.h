//----------------------------------------------------------------------------
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
class DateTime;
class Money;
class CurrencyTrx;

/**
*   @class LocalCurrencyDisplay
*
*   Description:
*   LocalCurrencyDisplay is responsible for displaying the exchange rates
*   and conversion calcuation results for local currencies.
*
*/

class LocalCurrencyDisplay
{
public:
  enum DCApplType
  { DC_CONVERT,
    DC_DISPLAY };

  LocalCurrencyDisplay(CurrencyTrx& trx) : _trx(trx) {}
  /**
  *
  *   @method convert
  *
  *   Description: Performs a DC conversion.
  *
  *   @return bool - conversion succeeded , else false
  */
  bool convert();

  /**
  *
  *   @method displayRate
  *
  *   Description: Displays the exchange rates for a given primary and secondary
  *                currency code.
  *
  *   @param  CurrencyCode  - primary currency used in retrieving BSR exchange
  *                           rate.
  *
  *   @return bool          - display succeeded , else false
  */
  bool displayRate(const CurrencyCode& primeCurrency);

  void displayRate(const DateTime& effectiveDate);

  /**
  *
  *   @method displayAllRates
  *
  *   Description: Displays all of the exchange rates for a given primary
  *                currency code.
  *
  *   @param  CurrencyCode  - primary currency used in retrieving BSR exchange
  *                           rate.
  *
  *   @param  Trx           - transaction object - should be a derived instance
  *                           of CurrencyTrx.
  *
  *   @return bool          - display succeeded , else false
  */
  bool displayAllRates();

  void displayAllRates(const DateTime& effectiveDate);

  void formatCurrentAndFutureLines(const CurrencyCode& primeCurrency,
                                   const CurrencyCode& sourceCurrency,
                                   const DateTime& effectiveDate,
                                   const std::string& countryName);

  /**
  *   @method getNationCurrency
  *
  *   Description: Retrieves either bank rate currency , pricing currency or
  *                national currency for a given nation.
  *
  *   @param  NationCode    - nation code
  *   @param  DateTime      - current date
  *   @param  CurrencyCode  - nation currency code - return parameter
  *   @param  bool          - used to indicate whether or not to perform
  *                           a two step retrieval using the bank rate
  *                           currency.
  *
  *   @return void
  */

  void getNationCurrency(const NationCode& nation,
                         DateTime& date,
                         CurrencyCode& nationalCurrency,
                         CurrencyCode& pricingCurrency,
                         CurrencyCode& bankRateCurrency,
                         bool& twoStepRetrieval);

  CurrencyCode getPrimeCurrency(const NationCode& nation,
                                const DateTime& date);

  /**
  *   @method getNation
  *
  *   Description: Retrieves nation code using either the nation specified in the
  *                DC display entry or the ticketing agent nation.
  *
  *   @return  NationCode    - nation code
  */
  const NationCode& getNation();

  /**
  *   @method getCurrentBSRRate
  *
  *   Description: Retrieves current exchange rate using input currency codes
  *
  *   @param   CurrencyCode  - prime currency group code
  *   @param   CurrencyCode  - secondary currency code
  *   @param   DateTime      - current date
  *   @param   CurrencyNoDec - rateNoDec
  *   @param   DateTime      - discontinued date
  *   @param   Indicator     - rate type
  *   @param   ExchRate      - current BSR rate
  *
  *   @return  bool          - true,false whether current rate retrieved
  */
  bool getCurrentBSRRate(const CurrencyCode& primeCurrency,
                         const CurrencyCode& currency,
                         const DateTime& date,
                         CurrencyNoDec& rateNoDec,
                         DateTime& discontinueDate,
                         Indicator& rateType,
                         ExchRate& futureRate);

  /**
  *   @method getFutureBSRRate
  *
  *   Description: Retrieves future exchange rate using input currency codes
  *
  *   @param   CurrencyCode  - prime currency group code
  *   @param   CurrencyCode  - secondary currency code
  *   @param   DateTime      - discontinued date
  *   @param   CurrencyNoDec - rateNoDec
  *   @param   DateTime      - effective date
  *   @param   Indicator     - rate type
  *   @parm    ExchRate      - future BSR rate or zero - NO_BSR_RATE
  *
  *   @return  bool          - true there is a future rate else false
  */
  bool getFutureBSRRate(const CurrencyCode& primeCurrency,
                        const CurrencyCode& currency,
                        const DateTime& discontinueDate,
                        DateTime& futureEffectiveDate,
                        CurrencyNoDec& futureRateNoDec,
                        Indicator& futureRateType,
                        ExchRate& futureRate);
  /**
  *   @method getCurrencyDescription
  *
  *   Description: Retrieves currency description
  *
  *   @param   CurrencyCode  - key to Currency cache
  *
  *   @return  string        - description of currency - returned
  */
  const std::string& getCurrencyDescription(const CurrencyCode& code, const DateTime& dcDate);

  /**
  *   @method getCurrencyNoDecimals
  *
  *   Description: Retrieves currency number of decimals
  *
  *   @param   CurrencyCode  - key to Currency cache
  *
  *   @return   CurrencyNoDec - number of decimals in currency
  */
  const CurrencyNoDec& getCurrencyNoDecimals(const CurrencyCode& code);

  /**
  *   @method processDetailLine
  *
  *   Description: Retrieves currency related output details and BSR exchanges rates
  *
  *   @param   string        - country description
  *   @param   CurrencyCode  - currency - either pricing,national or bank rate currency
  *   @param   ExchRate      - exchange rate
  *   @param   CurrencyNoDec - rateNoDec
  *   @param   Indicator     - rate type
  *
  *   @return  void
  */
  void processDetailLine(const std::string countryDescription,
                         const CurrencyCode& currencyCode,
                         const ExchRate& bsrRate,
                         CurrencyNoDec& rateNoDec,
                         Indicator& rateType);

  /**
  *   @method formatRateLine
  *
  *   Description: Formats the rate line
  *
  *   @param   CurrencyCode  - target currency
  *   @param   ExchRate      - exchange rate
  *   @param   CurrencyCode  - source currency
  *   @param   CurrencyNoDec - rateNoDec
  *   @param   bool          - whether or not this is a reciprocal rate
  *   @param   Indicator     - rateType B = BSR  I = Iata Clearing House.

  *
  *   @return  void
  */
  void formatRateLine(const CurrencyCode& target,
                      const ExchRate& exchangeRate,
                      const CurrencyCode& source,
                      const CurrencyNoDec& rateNoDec,
                      bool isReciprocalRate,
                      Indicator rateType);

  /**
  *   @method formatFutureRateLine
  *
  *   Description: Formats the detail line to display the future exchange rate.
  *
  *   @param   DateTime      - effective date this rate goes into effect
  *   @param   ExchRate      - future exchange rate
  *   @param   CurrencyNoDec - rateNoDec
  *   @param   Indicator     - rate type
  *
  *   @return  void
  */
  void formatFutureRateLine(const DateTime& effectiveDate,
                            const ExchRate& futureRate,
                            CurrencyNoDec& rateNoDec,
                            Indicator& rateType);
  /**
  *   @method formatOutput
  *
  *   Description: Formats the output to the CurrencyTrx ostringstream response
  *
  *   @param   string        - country description
  *   @param   string        - currency description
  *   @param   CurrencyCode  - key to Currency cache
  *   @param   CurrencyNoDec - number of decimals in currency
  *   @param   ExchRate      - rate of exchange
  *   @param   CurrencyNoDec - rateNoDec
  *   @param   Indicator     - rate type
  *
  *   @return  void
  */
  void formatOutput(const std::string countryDescription,
                    const std::string currencyDecription,
                    const CurrencyCode& code,
                    const CurrencyNoDec& numberDecimals,
                    const ExchRate& exchangeRate,
                    CurrencyNoDec& rateNoDec,
                    Indicator& rateType,
                    bool isReciprocal);

  /**
  *   @method formatExchangeRate
  *
  *   Description: Converts the exchange rate to a string
  *
  *   @param   ExchRate      - exchange rate
  *   @param   CurrencyNoDec - rateNoDec
  *
  *   @return  string
  */
  std::string formatExchangeRate(const ExchRate& exchangeRate,
                                 const CurrencyNoDec& rateNoDec,
                                 bool isReciprocal = false);

  /**
  *   @method getTaxRounding
  *
  *   Description: Retrieves Tax Rounding information for this nation
  *
  *   @param   CurrencyCode  - key to Currency cache
  *   @param   DateTime       - date
  *   @param   RoundingFactor - tax rounding factor
  *   @param   CurrencyNoDec  - number of decimals in rounding factor
  *   @param   RoundingRule   - tax rounding rule
  *
  *   @return  bool  - true - retrieved tax information,else false
  */
  bool getTaxRounding(const CurrencyCode& currencyCode,
                      DateTime& date,
                      RoundingFactor& roundingFactor,
                      CurrencyNoDec& roundingNoDec,
                      RoundingRule& roundingRule);

  /**
  *   @method formatRoundingLine
  *
  *   Description: Formats the rounding line for FARES and TAXES
  *
  *   @param   CurrencyCode   - currency code
  *   @param   CurrencyNoDec  - number of decimals in currency
  *   @param   MoneyAmount    - amount
  *   @param   string         - rounding rule
  *   @param   RoundingFactor - rounding factor
  *   @param   CurrencyNoDec  - number of decimals in rounding factor
  *   @param   string         - application - FARES or TAXES
  *
  *   @return  void
  */
  void formatRoundingLine(const CurrencyCode& currencyCode,
                          const CurrencyNoDec& targetNoDec,
                          MoneyAmount& amount,
                          const std::string& roundingStr,
                          const RoundingFactor& roundingFactor,
                          const CurrencyNoDec& roundingNoDec,
                          const std::string& roundApplication);

  /**
  *   @method formatRoundingMsg
  *
  *   Description: Formats the rounding message for FARES and TAXES
  *                according to the rounding rule
  *
  *   @param   RoundingRule   - rounding rule
  *   @param   string         - formatted string , in/out parameter
  *
  *   @return  void
  */
  void formatRoundingMsg(RoundingRule& roundingRule, std::string& roundingMsg);

  /**
  *   @method formatTruncationLine
  *
  *   Description: Formats the truncation line
  *
  *   @param   CurrencyCode   - currency code
  *   @param   CurrencyNoDec  - number of decimals in currency
  *   @param   MoneyAmount    - amount
  *   @param   RoundingFactor - fares rounding factor
  *   @param   CurrencyNoDec  - number of decimals in fares rounding factor
  *   @param   RoundingFactor - taxes rounding factor
  *   @param   CurrencyNoDec  - number of decimals in taxes rounding factor
  *   @param   bool           - whether or not tax rounding info is available
  *
  *   @return  void
  */
  void formatTruncationLine(const CurrencyCode& currencyCode,
                            const CurrencyNoDec& noDec,
                            const MoneyAmount& unroundedAmount,
                            const RoundingFactor& faresRoundingFactor,
                            const CurrencyNoDec& faresRoundingNoDec,
                            const RoundingFactor& taxRoundingFactor,
                            const CurrencyNoDec& taxRoundingNoDec,
                            bool taxInfoAvailable);

  /**
  *   @method numTruncationDecimals
  *
  *   Description: According to the rules of voodoo retrieves the number
  *                of decimals to be displayed on the truncation line.
  *
  *   @param   RoundingFactor - rounding factor
  *   @param   CurrencyNoDec  - number of decimals in currency
  *
  *   @return  int            - number of decimals to be displayed
  */
  CurrencyNoDec
  numTruncationDecimals(const RoundingFactor& roundingFactor, const CurrencyNoDec& noDec);

private:
  void removeTrailingZeroes(std::string& str);
  void truncateExchangeRate(std::string& str);

  static const std::string NULL_CURRENCY;
  static const int INVALID_NO_DECIMALS;
  static const double ONE;
  static const double NO_BSR_RATE;

  static const std::string ROUND_UP;
  static const std::string ROUND_DOWN;
  static const std::string ROUND_NEAREST;
  static const std::string ROUND_NONE;
  static const std::string FARES_APPL;
  static const std::string TAX_APPL;

  CurrencyTrx& _trx;
};
}
