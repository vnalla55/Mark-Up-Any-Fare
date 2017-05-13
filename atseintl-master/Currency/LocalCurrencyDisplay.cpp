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

#include "Currency/LocalCurrencyDisplay.h"

#include "Common/BSRCollectionResults.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/CurrencyUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/CurrencyTrx.h"
#include "DataModel/PricingRequest.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/TaxNation.h"

#include <algorithm>

namespace tse
{
FIXEDFALLBACK_DECL(reduceTemporaryVectorsFixed);

static Logger
logger("atseintl.Currency.LocalCurrencyDisplay");

const std::string LocalCurrencyDisplay::NULL_CURRENCY;
const int LocalCurrencyDisplay::INVALID_NO_DECIMALS = -1;
const double LocalCurrencyDisplay::ONE = 1.000000;
const double LocalCurrencyDisplay::NO_BSR_RATE = 0;

const std::string
LocalCurrencyDisplay::ROUND_UP("ROUNDED UP TO NEXT");
const std::string
LocalCurrencyDisplay::ROUND_DOWN("ROUNDED DOWN TO");
const std::string
LocalCurrencyDisplay::ROUND_NEAREST("ROUNDED TO NEAREST");
const std::string
LocalCurrencyDisplay::ROUND_NONE("NO ROUNDING");
const std::string
LocalCurrencyDisplay::FARES_APPL("FARES");
const std::string
LocalCurrencyDisplay::TAX_APPL("TAXES");

//-----------------------------------------------------------------------------
//   @method convert
//
//   Description: Performs a DC conversion
//
//   @return bool   - conversion succeeded , else false
//-----------------------------------------------------------------------------
bool
LocalCurrencyDisplay::convert()
{
  DateTime dcDate = _trx.pssLocalDate();
  CurrencyCode primaryCurrency;
  CurrencyCode nationalCurrency;
  CurrencyCode pricingCurrency;
  CurrencyCode bankRateCurrency;
  bool getReciprocalRate = true;
  bool twoStepRetrieval = false;
  bool convertRC = false;
  RoundingFactor taxRoundingFactor = 0;
  CurrencyNoDec taxRoundingNoDec = 0;
  RoundingRule taxRoundingRule = NONE;
  LOG4CXX_DEBUG(logger, "Original precision: " << _trx.response().precision());
  LOG4CXX_DEBUG(logger, "PSS LOCAL DATE: " << dcDate.toSimpleString());

  const NationCode& nation = getNation();

  if (nation.empty())
  {
    LOG4CXX_DEBUG(logger, "Nation code is empty");
    return false;
  }

  // Check for a historical date
  //
  if (!(_trx.baseDT().isInfinity()))
    dcDate = _trx.baseDT();

  getNationCurrency(
      nation, dcDate, nationalCurrency, pricingCurrency, bankRateCurrency, twoStepRetrieval);

  if (pricingCurrency.size())
    primaryCurrency = pricingCurrency;
  else if (nationalCurrency.size())
    primaryCurrency = nationalCurrency;

  if (_trx.sourceCurrency().empty())
    _trx.sourceCurrency() = primaryCurrency;

  if (_trx.targetCurrency().empty())
    _trx.targetCurrency() = primaryCurrency;

  Money source(_trx.amount(), _trx.sourceCurrency());
  Money target(_trx.targetCurrency());

  if (TrxUtil::isIcerActivated(_trx, dcDate)) //Remove BSRDSP keyword logic
  {
    if (_trx.reciprocal() == 'F')
    {
      LOG4CXX_DEBUG(logger, "Using multiplicative rate");
      getReciprocalRate = false;
    }
  }
  else
  {
    if ((_trx.eprBDK() == 'T' && _trx.reciprocal() == 'T') ||
        (_trx.eprBDK() == 'F' && _trx.reciprocal() == 'F'))
    {
      LOG4CXX_DEBUG(logger, "Using reciprocal rate");
      getReciprocalRate = false;
    }
  }

  CurrencyConversionRequest request(target,
                                    source,
                                    dcDate,
                                    *(_trx.getRequest()),
                                    _trx.dataHandle(),
                                    false,
                                    CurrencyConversionRequest::DC_CONVERT,
                                    getReciprocalRate);

  LOG4CXX_DEBUG(logger, "Primary Currency: " << primaryCurrency);
  LOG4CXX_DEBUG(logger, "Source  Currency: " << _trx.sourceCurrency());
  LOG4CXX_DEBUG(logger, "Target  Currency: " << _trx.targetCurrency());
  LOG4CXX_DEBUG(logger, "Source  amount:   " << _trx.amount());

  request.salesLocCurrency() = primaryCurrency;
  request.conversionCurrency() = bankRateCurrency;
  LOG4CXX_DEBUG(logger, "Sales loc Currency: " << request.salesLocCurrency());

  if ((primaryCurrency == _trx.sourceCurrency()) || (primaryCurrency == _trx.targetCurrency()))
  {
    LOG4CXX_DEBUG(logger, "Common sales location established");
    request.commonSalesLocCurrency() = true;
  }

  BSRCollectionResults results;

  BSRCurrencyConverter bsrConverter;
  CurrencyConverter curConverter;

  results.collect() = true;

  convertRC = bsrConverter.convert(request, &results);

  if (convertRC)
  {
    std::string faresRoundStr1;
    std::string faresRoundStr2;

    formatRoundingMsg(results.roundingRule1(), faresRoundStr1);
    formatRoundingMsg(results.roundingRule2(), faresRoundStr2);

    std::string taxRoundStr1;
    std::string taxRoundStr2;

    if (results.intermediateCurrency().empty())
    {
      bool taxRC = getTaxRounding(
          _trx.targetCurrency(), dcDate, taxRoundingFactor, taxRoundingNoDec, taxRoundingRule);

      // Line 1
      //
      if (_trx.getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
      {
        _trx.response() << "VD " << std::endl;
      }

      formatRateLine(results.bsrPrimeCurrencyGroup1(),
                     results.exchangeRate1(),
                     results.bsrCurrency1(),
                     results.exchangeRate1NoDec(),
                     getReciprocalRate,
                     results.exchangeRateType1());

      formatTruncationLine(target.code(),
                           target.noDec(dcDate),
                           results.targetUnroundedAmount(),
                           results.roundingFactor1(),
                           results.roundingFactorNoDec1(),
                           taxRoundingFactor,
                           taxRoundingNoDec,
                           taxRC);

      formatRoundingMsg(taxRoundingRule, taxRoundStr1);

      formatRoundingLine(target.code(),
                         target.noDec(dcDate),
                         target.value(),
                         faresRoundStr1,
                         results.roundingFactor1(),
                         results.roundingFactorNoDec1(),
                         FARES_APPL);

      if (taxRC)
      {
        Money roundedTaxAmount(results.targetUnroundedAmount(), target.code());

        curConverter.round(roundedTaxAmount, taxRoundingFactor, taxRoundingRule);

        formatRoundingMsg(taxRoundingRule, taxRoundStr1);

        formatRoundingLine(target.code(),
                           target.noDec(dcDate),
                           roundedTaxAmount.value(),
                           taxRoundStr1,
                           taxRoundingFactor,
                           taxRoundingNoDec,
                           TAX_APPL);
      }
      else
        _trx.response() << target.code() << "  - TAX ROUNDING INFO UNAVAILABLE"
                        << "\n";
    }
    else
    {
      // Line 1
      //
      if (_trx.getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
      {
        _trx.response() << "VD " << std::endl;
      }

      _trx.response() << "\n" << source.code().c_str() << _trx.amountStr() << " "
                      << "CONVERTED THROUGH COMMON CURRENCY "
                      << results.bsrPrimeCurrencyGroup1().c_str() << std::endl;

      // Line 2
      //
      formatRateLine(results.bsrPrimeCurrencyGroup1(),
                     results.exchangeRate1(),
                     results.bsrCurrency1(),
                     results.exchangeRate1NoDec(),
                     getReciprocalRate,
                     results.exchangeRateType1());

      bool taxRC = getTaxRounding(results.intermediateCurrency(),
                                  dcDate,
                                  taxRoundingFactor,
                                  taxRoundingNoDec,
                                  taxRoundingRule);

      formatTruncationLine(results.intermediateCurrency(),
                           results.intermediateNoDec(),
                           results.intermediateUnroundedAmount(),
                           results.roundingFactor1(),
                           results.roundingFactorNoDec1(),
                           taxRoundingFactor,
                           taxRoundingNoDec,
                           taxRC);

      formatRoundingMsg(taxRoundingRule, taxRoundStr1);

      formatRoundingLine(results.intermediateCurrency(),
                         results.intermediateNoDec(),
                         results.intermediateAmount(),
                         faresRoundStr1,
                         results.roundingFactor1(),
                         results.roundingFactorNoDec1(),
                         FARES_APPL);

      if (taxRC)
      {
        Money roundedTaxAmount(results.intermediateUnroundedAmount(),
                               results.intermediateCurrency());

        curConverter.round(roundedTaxAmount, taxRoundingFactor, taxRoundingRule);

        formatRoundingMsg(taxRoundingRule, taxRoundStr1);

        formatRoundingLine(results.intermediateCurrency(),
                           results.intermediateNoDec(),
                           roundedTaxAmount.value(),
                           taxRoundStr1,
                           taxRoundingFactor,
                           taxRoundingNoDec,
                           TAX_APPL);
      }
      else
        _trx.response() << "  - TAX ROUNDING INFO UNAVAILABLE"
                        << "\n";

      _trx.response().precision(results.intermediateNoDec());

      // Line 1
      //
      _trx.response() << results.intermediateCurrency().c_str() << results.intermediateAmount()
                      << " "
                      << "CONVERTED THROUGH COMMON CURRENCY "
                      << results.bsrPrimeCurrencyGroup2().c_str() << std::endl;

      _trx.response().precision(results.exchangeRate2NoDec());

      // Line 2
      //

      formatRateLine(results.bsrPrimeCurrencyGroup2(),
                     results.exchangeRate2(),
                     results.bsrCurrency2(),
                     results.exchangeRate2NoDec(),
                     getReciprocalRate,
                     results.exchangeRateType2());

      taxRC = getTaxRounding(
          results.targetCurrency(), dcDate, taxRoundingFactor, taxRoundingNoDec, taxRoundingRule);

      formatTruncationLine(target.code(),
                           target.noDec(dcDate),
                           results.targetUnroundedAmount(),
                           results.roundingFactor2(),
                           results.roundingFactorNoDec2(),
                           taxRoundingFactor,
                           taxRoundingNoDec,
                           taxRC);

      formatRoundingMsg(taxRoundingRule, taxRoundStr2);

      formatRoundingLine(target.code(),
                         target.noDec(dcDate),
                         target.value(),
                         faresRoundStr2,
                         results.roundingFactor2(),
                         results.roundingFactorNoDec2(),
                         FARES_APPL);

      if (taxRC)
      {
        Money roundedTaxAmount(results.targetUnroundedAmount(), target.code());

        curConverter.round(roundedTaxAmount, taxRoundingFactor, taxRoundingRule);

        formatRoundingMsg(taxRoundingRule, taxRoundStr2);

        formatRoundingLine(target.code(),
                           target.noDec(dcDate),
                           roundedTaxAmount.value(),
                           taxRoundStr2,
                           taxRoundingFactor,
                           taxRoundingNoDec,
                           TAX_APPL);
      }
      else
        _trx.response() << " - TAX ROUNDING INFO UNAVAILABLE"
                        << "\n";
    }
  }
  else
  {
    if (_trx.getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
    {
      _trx.response() << "VE " << std::endl;
    }
    _trx.response() << "\n"
                    << "UNABLE TO CALCULATE - BSR NOT AVAILABLE"
                    << "\n";
  }

  return convertRC;
}

//-----------------------------------------------------------------------------------
// Display the direct exchange rate for a given primary and target currency code.
//-----------------------------------------------------------------------------------
void
LocalCurrencyDisplay::displayRate(const DateTime& effectiveDate)
{
  const NationCode& nation = getNation();
  if ( nation.empty() )
    return;

  const CurrencyCode primeCurrency = getPrimeCurrency( nation, effectiveDate );

  LOG4CXX_DEBUG(logger, "Primary currency: " << primeCurrency);
  LOG4CXX_DEBUG(logger, "Source  currency: " << _trx.sourceCurrency());

  DateTime      expirationDate = effectiveDate;
  CurrencyNoDec rateNoDec = 0;
  Indicator     rateType;
  ExchRate      bsrRate = 0.0;

  bool bsrRateRC = getCurrentBSRRate( primeCurrency,
                                      _trx.sourceCurrency(),
                                      effectiveDate,
                                      rateNoDec,
                                      expirationDate,
                                      rateType,
                                      bsrRate );

  if ( !bsrRateRC )
  {
    if ( _trx.getRequest()->ticketingAgent()->axessUser() )
      _trx.response() << "VE " << std::endl;

    _trx.response() << std::endl << "UNABLE TO DISPLAY - BSR NOT AVAILABLE" << std::endl;
    return;
  }

  if ( _trx.getRequest()->ticketingAgent()->axessUser() )
    _trx.response() << "VD " << std::endl;

  _trx.response() << "COUNTRY         CURRENCY  CODE    DECIMALS    RATE \n";

  const std::string countryName =
     CurrencyUtil::getControllingNationDesc( _trx, _trx.sourceCurrency(), effectiveDate );

  processDetailLine( countryName, _trx.sourceCurrency(), bsrRate, rateNoDec, rateType );

  if ( !expirationDate.isInfinity() )
  {
    DateTime      futureEffectiveDate = effectiveDate;
    ExchRate      futureRate = 0.0;
    CurrencyNoDec futureRateNoDec = 0;
    Indicator     futureRateType;

    bool futureRateRC = getFutureBSRRate( primeCurrency,
                                          _trx.sourceCurrency(),
                                          expirationDate,
                                          futureEffectiveDate,
                                          futureRateNoDec,
                                          futureRateType,
                                          futureRate );

    if ( futureRateRC )
      formatFutureRateLine( futureEffectiveDate, futureRate, futureRateNoDec, futureRateType );
  }

  _trx.response() << "END ITEM\n";

}

//-----------------------------------------------------------------------------
// Display the direct exchange rates for a given primary currency code
// for all nations (the target currencies).
//-----------------------------------------------------------------------------
void
LocalCurrencyDisplay::displayAllRates(const DateTime& effectiveDate )
{
  const NationCode& country = getNation();
  if ( country.empty() )
    return;

  const CurrencyCode primeCurrency = getPrimeCurrency( country, effectiveDate );

  if ( primeCurrency.empty() )
  {
    if ( _trx.getRequest()->ticketingAgent()->axessUser() )
    {
      _trx.response() << "VE " << std::endl;
      _trx.response() << std::endl << "REQUEST RECEIVED - NO RESPONSE DATA" << std::endl;
    }
    return;
  }

  const std::vector<Nation*>& nationsList = _trx.dataHandle().getAllNation( effectiveDate );

  if ( nationsList.empty() )
  {
    if ( _trx.getRequest()->ticketingAgent()->axessUser() )
    {
      _trx.response() << "VE " << std::endl;
      _trx.response() << std::endl << "REQUEST RECEIVED - NO RESPONSE DATA" << std::endl;
    }
    return;
  }

  if ( _trx.getRequest()->ticketingAgent()->axessUser() )
  {
    _trx.response() << "VD " << std::endl;
  }

  _trx.response() << "COUNTRY         CURRENCY  CODE    DECIMALS    RATE \n";

  for (Nation* nation : nationsList)
  {
    if ( _trx.dcAlphaChar()  && ( _trx.dcAlphaChar() != nation->description()[0] ) )
      continue;

    if ( nation->expireDate() < effectiveDate )
      continue;

    if ( !nation->alternateCur().empty() )
    {
      formatCurrentAndFutureLines(primeCurrency,
                                  nation->alternateCur(),
                                  effectiveDate,
                                  nation->description());
    }

    if ( !nation->primeCur().empty() && ( nation->primeCur() != nation->alternateCur() ) )
    {
      formatCurrentAndFutureLines(primeCurrency,
                                  nation->primeCur(),
                                  effectiveDate,
                                  nation->description());
    }
  }

  _trx.response() << "END ITEM\n";

}

//----------------------------------------------------------------------------------------
//   Description: Retrieves pricing currency and/or national currency for a given nation.
//----------------------------------------------------------------------------------------
CurrencyCode
LocalCurrencyDisplay::getPrimeCurrency(const NationCode& nationCode,
                                       const DateTime& date)
{
  const Nation* nation = _trx.dataHandle().getNation( nationCode, date );

  if ( !nation )
    return "";

  return !nation->alternateCur().empty() ? nation->alternateCur() : nation->primeCur();
}

void
LocalCurrencyDisplay::formatCurrentAndFutureLines(const CurrencyCode& primeCurrency,
                                                  const CurrencyCode& sourceCurrency,
                                                  const DateTime& effectiveDate,
                                                  const std::string& countryName)
{
  DateTime      expirationDate = effectiveDate;
  CurrencyNoDec rateNoDec = 0;
  Indicator     rateType;
  ExchRate      bsrRate = 0.0;

  bool bsrRateRC = getCurrentBSRRate(primeCurrency,
                                     sourceCurrency,
                                     effectiveDate,
                                     rateNoDec,
                                     expirationDate,
                                     rateType,
                                     bsrRate);
  if (bsrRateRC)
    processDetailLine( countryName, sourceCurrency, bsrRate, rateNoDec, rateType );

  if ( !expirationDate.isInfinity() && !_trx.baseCountry().empty() )
  {
    DateTime      futureEffectiveDate = effectiveDate;
    ExchRate      futureRate = 0.0;
    CurrencyNoDec futureRateNoDec = 0;
    Indicator     futureRateType;

    bool futureRateRC = getFutureBSRRate(primeCurrency,
                                         sourceCurrency,
                                         expirationDate,
                                         futureEffectiveDate,
                                         futureRateNoDec,
                                         futureRateType,
                                         futureRate);
    if (futureRateRC)
      formatFutureRateLine( futureEffectiveDate, futureRate, futureRateNoDec, futureRateType );
  }

}

//-----------------------------------------------------------------------------
//   @method displayRate
//
//   Description: Displays the exchange rates for a given primary and secondary
//                currency code.
//
//   @param  CurrencyCode  - specified currency in DC display
//
//   @return bool          - display succeeded , else false
//-----------------------------------------------------------------------------
bool
LocalCurrencyDisplay::displayRate(const CurrencyCode& primeCurrency)
{
  DateTime currentDate = _trx.pssLocalDate();
  DateTime expirationDate = _trx.pssLocalDate();
  bool twoStepRetrieval = false;
  CurrencyCode primaryCurrency;
  CurrencyCode nationalCurrency;
  CurrencyCode pricingCurrency;
  CurrencyCode bankRateCurrency;
  CurrencyNoDec rateNoDec = 0;
  Indicator rateType;

  LOG4CXX_DEBUG(logger, "PSS LOCAL DATE: " << currentDate.toSimpleString());
  // Retrieve nation code or override nation code
  //
  const NationCode& nation = getNation();

  if (nation.empty())
  {
    LOG4CXX_DEBUG(logger, "Nation code is empty");
    return false;
  }

  LOG4CXX_DEBUG(logger, "Nation code is : " << nation);

  // Check for historical date
  //
  if (!(_trx.baseDT().isInfinity()))
  {
    currentDate = _trx.baseDT();
    LOG4CXX_DEBUG(logger, "Historical date: " << currentDate.toSimpleString());
  }

  getNationCurrency(
      nation, currentDate, nationalCurrency, pricingCurrency, bankRateCurrency, twoStepRetrieval);

  if (pricingCurrency.size())
    primaryCurrency = pricingCurrency;
  else if (nationalCurrency.size())
    primaryCurrency = nationalCurrency;

  LOG4CXX_DEBUG(logger, "Primary currency: " << primaryCurrency);
  LOG4CXX_DEBUG(logger, "Source  currency: " << _trx.sourceCurrency());

  LOG4CXX_DEBUG(logger, "Performing Direct conversion");

  ExchRate bsrRate = 0.0;

  bool bsrRateRC = getCurrentBSRRate(primaryCurrency,
                                     _trx.sourceCurrency(),
                                     currentDate,
                                     rateNoDec,
                                     expirationDate,
                                     rateType,
                                     bsrRate);

  if (bsrRateRC)
  {
    LOG4CXX_DEBUG(logger, "Performed Direct conversion");
    LOG4CXX_DEBUG(logger, "Rate retrieved: " << bsrRate);

    //      NationCode nation;
    std::string countryName =
        CurrencyUtil::getControllingNationDesc(_trx, _trx.sourceCurrency(), currentDate);
    if (_trx.getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
    {
      _trx.response() << "VD " << std::endl;
    }

    _trx.response() << "COUNTRY         CURRENCY  CODE    DECIMALS    RATE \n";

    processDetailLine(countryName, _trx.sourceCurrency(), bsrRate, rateNoDec, rateType);

    DateTime effectiveDate = currentDate;

    LOG4CXX_DEBUG(logger, "Expiration date: " << expirationDate.toSimpleString());

    if (!expirationDate.isInfinity())
    {
      ExchRate futureRate = 0.0;

      bool futureRateRC = getFutureBSRRate(primaryCurrency,
                                           _trx.sourceCurrency(),
                                           expirationDate,
                                           effectiveDate,
                                           rateNoDec,
                                           rateType,
                                           futureRate);

      LOG4CXX_DEBUG(logger, "Effective date: " << effectiveDate.toSimpleString());

      if (futureRateRC)
        formatFutureRateLine(effectiveDate, futureRate, rateNoDec, rateType);
    }

    _trx.response() << "END ITEM\n";
  }
  else if (bankRateCurrency.size())
  {

    ExchRate rate1 = 0.0;

    bool rate1RC = getCurrentBSRRate(bankRateCurrency,
                                     _trx.sourceCurrency(),
                                     currentDate,
                                     rateNoDec,
                                     expirationDate,
                                     rateType,
                                     rate1);

    if (rate1RC)
    {
      if (_trx.getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
      {
        _trx.response() << "VD " << std::endl;
      }

      _trx.response() << "\n";
      _trx.response() << "CONVERSION RATE FROM COMMON CURRENCY " << bankRateCurrency.c_str()
                      << "\n";

      _trx.response() << "COUNTRY         CURRENCY  CODE    DECIMALS    RATE \n";

      std::string countryName1 =
          CurrencyUtil::getControllingNationDesc(_trx, _trx.sourceCurrency(), currentDate);

      processDetailLine(countryName1, _trx.sourceCurrency(), rate1, rateNoDec, rateType);

      DateTime effectiveDate = currentDate;

      if (!expirationDate.isInfinity())
      {
        ExchRate futureRate = 0.0f;

        bool futureRateRC = getFutureBSRRate(bankRateCurrency,
                                             _trx.sourceCurrency(),
                                             expirationDate,
                                             effectiveDate,
                                             rateNoDec,
                                             rateType,
                                             futureRate);

        if (futureRateRC)
          formatFutureRateLine(effectiveDate, futureRate, rateNoDec, rateType);
      }
    }
    else
    {
      if (_trx.getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
      {
        _trx.response() << "VE " << std::endl;
      }
      _trx.response() << "\n"
                      << "UNABLE TO DISPLAY - BSR NOT AVAILABLE"
                      << "\n";

      return false;
    }

    ExchRate rate2 = 0.0;

    bool rate2RC = getCurrentBSRRate(
        primaryCurrency, bankRateCurrency, currentDate, rateNoDec, expirationDate, rateType, rate2);

    if (rate2RC)
    {
      _trx.response() << "\n";
      _trx.response() << "CONVERSION RATE FROM COMMON CURRENCY " << primaryCurrency.c_str() << "\n";

      _trx.response() << "COUNTRY         CURRENCY  CODE    DECIMALS    RATE \n";

      std::string countryName2 =
          CurrencyUtil::getControllingNationDesc(_trx, bankRateCurrency, currentDate);

      processDetailLine(countryName2, bankRateCurrency, rate2, rateNoDec, rateType);

      DateTime effectiveDate = currentDate;

      if (!expirationDate.isInfinity())
      {
        ExchRate futureRate = 0.0f;
        bool futureRateRC = getFutureBSRRate(primaryCurrency,
                                             _trx.sourceCurrency(),
                                             expirationDate,
                                             effectiveDate,
                                             rateNoDec,
                                             rateType,
                                             futureRate);

        if (futureRateRC)
          formatFutureRateLine(effectiveDate, futureRate, rateNoDec, rateType);
      }

      _trx.response() << "END ITEM\n";
    }
    else
    {
      if (_trx.getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
      {
        _trx.response() << "VE " << std::endl;
      }
      _trx.response() << "\n"
                      << "UNABLE TO DISPLAY - BSR NOT AVAILABLE"
                      << "\n";

      return false;
    }
  }
  else
  {
    if (_trx.getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
    {
      _trx.response() << "VE " << std::endl;
    }
    _trx.response() << "\n"
                    << "UNABLE TO DISPLAY - BSR NOT AVAILABLE"
                    << "\n";
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
//
//   @method displayAllRates
//
//   Description: Displays all of the exchange rates for a given primary
//                currency code.
//
//   @return bool  - display succeeded , else false
//-----------------------------------------------------------------------------
bool
LocalCurrencyDisplay::displayAllRates()
{
  CurrencyCode primaryCurrency;
  CurrencyCode nationalCurrency;
  CurrencyCode pricingCurrency;
  CurrencyCode bankRateCurrency;
  DateTime currentDate = _trx.pssLocalDate();
  DateTime expirationDate = _trx.pssLocalDate();
  bool twoStepRetrieval = false;
  CurrencyNoDec rateNoDec = 0;
  bool displayBankRateHeader = false;
  Indicator rateType;

  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::displayAllRates");
  const NationCode& nation = getNation();
  LOG4CXX_DEBUG(logger, "PSS LOCAL DATE: " << currentDate.toSimpleString());

  if (nation.empty())
    return false;

  // Check for historical date
  //
  if (!(_trx.baseDT().isInfinity()))
  {
    currentDate = _trx.baseDT();
    LOG4CXX_DEBUG(logger, "Historical date: " << currentDate.toSimpleString());
  }

  LOG4CXX_DEBUG(logger, "Nation code is : " << nation);

  getNationCurrency(
      nation, currentDate, nationalCurrency, pricingCurrency, bankRateCurrency, twoStepRetrieval);

  if (bankRateCurrency.size())
  {
    if (!_trx.baseCountry().empty())
    {
      if (nationalCurrency.size())
        primaryCurrency = nationalCurrency;
    }
    else
      primaryCurrency = bankRateCurrency;

    displayBankRateHeader = true;
  }
  else if (pricingCurrency.size())
    primaryCurrency = pricingCurrency;
  else if (nationalCurrency.size())
  {
    primaryCurrency = nationalCurrency;
  }

  if (primaryCurrency.empty())
  {
    if (_trx.getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
    {
      _trx.response() << "VE " << std::endl;
      _trx.response() << "\n"
                      << "REQUEST RECEIVED - NO RESPONSE DATA"
                      << "\n";
    }
    return false;
  }

  LOG4CXX_DEBUG(logger, "Bank Rate currency for nation: " << nation << ", " << bankRateCurrency);
  LOG4CXX_DEBUG(logger, "Primary currency for nation: " << nation << ", " << primaryCurrency);

  const std::vector<Nation*>& nationsList = _trx.dataHandle().getAllNation(currentDate);

  if (nationsList.empty())
  {
    if (_trx.getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
    {
      _trx.response() << "VE " << std::endl;
      _trx.response() << "\n"
                      << "REQUEST RECEIVED - NO RESPONSE DATA"
                      << "\n";
    }
    return false;
  }

  LOG4CXX_DEBUG(logger, "Number of Nations: " << nationsList.size());

  std::vector<Nation*>::const_iterator nationsListIter = nationsList.begin();
  std::vector<Nation*>::const_iterator nationsListEnd = nationsList.end();

  std::string eur("EUR");
  if (_trx.getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
  {
    _trx.response() << "VD " << std::endl;
  }

  if ((displayBankRateHeader) && (!_trx.baseCountry().empty()))
  {
    if (bankRateCurrency == eur)
    {
      _trx.response() << std::endl;
      _trx.response() << "DIRECT RATES LISTED - OTHER RATES CONVERTED FROM COMMON" << std::endl
                      << "CURRENCY EUR"
                      << " - USE DC*" << nation.c_str() << "/CUR " << nation.c_str()
                      << " REPRESENTS AN EMU NATION CODE" << std::endl;
    }
    else
    {
      bool foundNationalCurrency = false;
      bool foundNation = false;
      NationCode nationWithMatchingNationalCurrency;
      NationCode nationCode;

      std::string countryDesc =
          CurrencyUtil::getControllingNationDesc(_trx, bankRateCurrency, currentDate);

      CurrencyUtil::getControllingNationCode(_trx,
                                             countryDesc,
                                             nationCode,
                                             foundNation,
                                             foundNationalCurrency,
                                             nationWithMatchingNationalCurrency,
                                             currentDate,
                                             _trx.targetCurrency());

      _trx.response() << std::endl;
      _trx.response() << "DIRECT RATES LISTED - OTHER RATES CONVERTED FROM COMMON" << std::endl
                      << "CURRENCY " << bankRateCurrency.c_str() << " - USE DC*";

      if (foundNation)
        _trx.response() << nationCode.c_str();
      else if (foundNationalCurrency)
        _trx.response() << nationWithMatchingNationalCurrency.c_str();

      _trx.response() << "/CUR" << std::endl;
    }
  }
  else if (displayBankRateHeader)
  {
    _trx.response() << std::endl;
    _trx.response() << "CONVERSION RATE FROM COMMON CURRENCY " << primaryCurrency.c_str()
                    << " - USE DC*" << nation.c_str() << "/CUR " << std::endl << "FOR DIRECT RATES"
                    << std::endl;
  }

  _trx.response() << "COUNTRY         CURRENCY  CODE    DECIMALS    RATE \n";

  for (; nationsListIter != nationsListEnd; nationsListIter++)
  {
    const Nation* currentNation = *nationsListIter;
    std::string countryName = currentNation->description();

    if (_trx.dcAlphaChar())
    {
      if (countryName[0] != _trx.dcAlphaChar())
        continue;
    }

    LOG4CXX_DEBUG(logger, "Nation code : " << currentNation->nation());
    LOG4CXX_DEBUG(logger, "Country name: " << countryName);
    LOG4CXX_DEBUG(logger,
                  "Country expiration date: " << currentNation->expireDate().toSimpleString());
    LOG4CXX_DEBUG(logger, "Country disc  date: " << currentNation->discDate().toSimpleString());

    if (currentNation->expireDate() < currentDate)
    {
      LOG4CXX_DEBUG(logger, "Skipping expired nation record");
      continue;
    }

    const CurrencyCode nationalCurrency2 = currentNation->primeCur();
    const CurrencyCode pricingCurrency2 = currentNation->alternateCur();

    if (!pricingCurrency2.empty())
    {
      LOG4CXX_DEBUG(logger, "Nation has a pricing currency");

      ExchRate bsrRate = 0.0;

      bool bsrRateRC = getCurrentBSRRate(primaryCurrency,
                                         pricingCurrency2,
                                         currentDate,
                                         rateNoDec,
                                         expirationDate,
                                         rateType,
                                         bsrRate);

      if (bsrRateRC)
      {
        processDetailLine(countryName, pricingCurrency2, bsrRate, rateNoDec, rateType);
      }

      DateTime effectiveDate = currentDate;

      LOG4CXX_DEBUG(logger, "Expiration date: " << expirationDate.toSimpleString());

      if (!expirationDate.isInfinity())
      {
        ExchRate futureRate = 0.0;

        CurrencyCode pricingCurrency(pricingCurrency2);

        if (!_trx.baseCountry().empty())
        {
          bool futureRateRC = getFutureBSRRate(primaryCurrency,
                                               pricingCurrency,
                                               expirationDate,
                                               effectiveDate,
                                               rateNoDec,
                                               rateType,
                                               futureRate);

          LOG4CXX_DEBUG(logger, "Effective date: " << effectiveDate.toSimpleString());

          if (futureRateRC)
            formatFutureRateLine(effectiveDate, futureRate, rateNoDec, rateType);
        }
      }
    }

    if (!nationalCurrency2.empty())
    {
      LOG4CXX_DEBUG(logger, "Nation has a national currency currency");

      if (pricingCurrency2 == nationalCurrency2)
        continue;

      ExchRate bsrRate = 0.0;

      bool bsrRateRC = getCurrentBSRRate(primaryCurrency,
                                         nationalCurrency2,
                                         currentDate,
                                         rateNoDec,
                                         expirationDate,
                                         rateType,
                                         bsrRate);

      if (bsrRateRC)
      {
        processDetailLine(countryName, nationalCurrency2, bsrRate, rateNoDec, rateType);
      }

      DateTime effectiveDate = currentDate;

      LOG4CXX_DEBUG(logger, "Expiration date: " << expirationDate.toSimpleString());

      if (!expirationDate.isInfinity())
      {
        ExchRate futureRate = 0.0;

        CurrencyCode nationalCurrency(nationalCurrency2);

        if (!_trx.baseCountry().empty())
        {
          bool futureRateRC = getFutureBSRRate(primaryCurrency,
                                               nationalCurrency,
                                               expirationDate,
                                               effectiveDate,
                                               rateNoDec,
                                               rateType,
                                               futureRate);

          LOG4CXX_DEBUG(logger, "Effective date: " << effectiveDate.toSimpleString());

          if (futureRateRC)
            formatFutureRateLine(effectiveDate, futureRate, rateNoDec, rateType);
        }
      }
    }
  }

  _trx.response() << "END ITEM\n";

  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::displayAllRates");

  return true;
}

//-----------------------------------------------------------------------------
//
//   @method getNationCurrency
//
//   Description: Retrieves either bank rate currency , pricing currency or
//                national currency for a given nation.
//
//   @param  NationCode    - nation code
//   @param  DateTime      - current date
//   @param  CurrencyCode  - currency code of nation
//   @param  bool          - used to indicate whether or not to perform
//                           a two step retrieval using the bank rate
//                           currency.
//
//   @return bool          - true - retrieved currency else false
//-----------------------------------------------------------------------------
void
LocalCurrencyDisplay::getNationCurrency(const NationCode& nationCode,
                                        DateTime& date,
                                        CurrencyCode& nationalCurrency,
                                        CurrencyCode& pricingCurrency,
                                        CurrencyCode& bankRateCurrency,
                                        bool& twoStepRetrieval)
{
  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::getNationCurrency");

  LOG4CXX_DEBUG(logger, "Nation code: " << nationCode);
  LOG4CXX_DEBUG(logger, "Date: " << date.toSimpleString());

  const Nation* nation = _trx.dataHandle().getNation(nationCode, date);

  if (nation)
  {
    if (nation->conversionCur().size())
    {
      bankRateCurrency = nation->conversionCur();
      LOG4CXX_DEBUG(logger, "Bank Rate currency is not blank: " << bankRateCurrency);
      twoStepRetrieval = true;
    }

    if (nation->alternateCur().size())
    {
      pricingCurrency = nation->alternateCur();
      LOG4CXX_DEBUG(logger, "Pricing currency is not blank: " << pricingCurrency);
    }

    if (nation->primeCur().size())
    {
      nationalCurrency = nation->primeCur();
      LOG4CXX_DEBUG(logger, "National currency is not blank: " << nationalCurrency);
    }
  }

  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::getNationCurrency");
}

//-----------------------------------------------------------------------------
//   @method getNation
//
//   Description: Retrieves nation code using either the nation specified in the
//                DC display entry or the ticketing agent nation.
//
//   @return  NationCode    - nation code
//-----------------------------------------------------------------------------
const NationCode&
LocalCurrencyDisplay::getNation()
{
  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::getNation");

  if (!_trx.baseCountry().empty())
  {
    LOG4CXX_DEBUG(logger, "Base Country is: " << _trx.baseCountry());
    return _trx.baseCountry();
  }

  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::getNation");

  return (_trx.getRequest()->ticketingAgent()->agentLocation()->nation());
}

//-----------------------------------------------------------------------------
//   @method getFutureBSRRate
//
//   Description: Retrieves future exchange rate using input currency codes
//
//   @return bool - true , yes future rate retrieved else false;
//-----------------------------------------------------------------------------
bool
LocalCurrencyDisplay::getFutureBSRRate(const CurrencyCode& primeCurrency,
                                       const CurrencyCode& currency,
                                       const DateTime& expirationDate,
                                       DateTime& effectiveDate,
                                       CurrencyNoDec& rateNoDec,
                                       Indicator& rateType,
                                       ExchRate& futureRate)
{
  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::getFutureBSRRate");

  DateTime futureDate = expirationDate.nextDay();

  LOG4CXX_DEBUG(logger, "FUTURE DATE: " << futureDate.toSimpleString());
  LOG4CXX_DEBUG(logger, "TRX TICKETING DATE: " << _trx.dataHandle().ticketDate().toSimpleString());

  DataHandle dataHandle(futureDate);

  if (fallback::fixed::reduceTemporaryVectorsFixed())
  {
    const std::vector<BankerSellRate*>& rateVec =
        dataHandle.getBankerSellRate(primeCurrency, currency, futureDate);

    LOG4CXX_DEBUG(logger, "RATE VEC SIZE = " << rateVec.size());

    if (!rateVec.empty())
    {
      const BankerSellRate* dbFutureRate = rateVec.front();
      effectiveDate = dbFutureRate->effDate();
      rateNoDec = dbFutureRate->rateNodec();
      rateType = dbFutureRate->rateType();
      futureRate = dbFutureRate->rate();
      return true;
    }
  }
  else
  {
    auto bsrRange = dataHandle.getBankerSellRateRange(primeCurrency, currency, futureDate);
    LOG4CXX_DEBUG(logger, "RATE VEC SIZE = " << bsrRange.size());

    if (!bsrRange.empty())
    {
      const BankerSellRate* dbFutureRate = *bsrRange.begin();
      effectiveDate = dbFutureRate->effDate();
      rateNoDec = dbFutureRate->rateNodec();
      rateType = dbFutureRate->rateType();
      futureRate = dbFutureRate->rate();
      return true;
    }
  }
  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::getFutureBSRRate");

  return false;
}

//-----------------------------------------------------------------------------
//   @method processDetailLine
//
//   Description: Retrieves currency related output details and BSR exchanges rates
//
//   @param   string        - country description
//   @param   CurrencyCode  - currency - either pricing,national or bank rate currency
//   @param   ExchRate      - exchange rate
//
//   @return  void
//-----------------------------------------------------------------------------
void
LocalCurrencyDisplay::processDetailLine(const std::string countryDescription,
                                        const CurrencyCode& currencyCode,
                                        const ExchRate& bsrRate,
                                        CurrencyNoDec& rateNoDec,
                                        Indicator& rateType)
{
  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::processDetailLine");
  DateTime dcDate = _trx.pssLocalDate();

  if (!(_trx.baseDT().isInfinity()))
    dcDate = _trx.baseDT();

  LOG4CXX_DEBUG(logger, "DC DATE: " << dcDate.toSimpleString());

  const std::string& currencyDescription = getCurrencyDescription(currencyCode, dcDate);

  if (currencyDescription == NULL_CURRENCY)
    return;

  const CurrencyNoDec& noDecimals = getCurrencyNoDecimals(currencyCode);

  if (noDecimals == INVALID_NO_DECIMALS)
    return;

  LOG4CXX_INFO(logger, "CURRENCY DESCRIPTION: " << currencyDescription);

  if (countryDescription.empty())
  {
    LOG4CXX_ERROR(logger, "Country description is empty");
    return;
  }

  LOG4CXX_DEBUG(logger, "Country description: " << countryDescription);

  LOG4CXX_DEBUG(logger, "Before empty call");

  if (currencyDescription.size() == 0)
  {
    LOG4CXX_ERROR(logger, "Currency description is empty");
    return;
  }

  LOG4CXX_DEBUG(logger, "After empty call");

  LOG4CXX_DEBUG(logger, "Currency description: " << currencyDescription);
  LOG4CXX_DEBUG(logger, "Currency Number of Decimals: " << noDecimals);

  if (TrxUtil::isIcerActivated(_trx, dcDate)) //Remove BSRDSP keyword logic
  {
    if (!(_trx.reciprocal() == 'T'))
    {
      LOG4CXX_DEBUG(logger, "Formatting multiplicative rates");

      // Display Multiplicative Rates
      //
      formatOutput(countryDescription,
                  currencyDescription,
                  currencyCode,
                  noDecimals,
                  bsrRate,
                  rateNoDec,
                  rateType,
                  false);
    }
    else
    {
      LOG4CXX_DEBUG(logger, "Formatting reciprocal rates");

      // Display Calculated Reciprocal exchange rates
      //
      const ExchRate& reciprocalRate = 1 / bsrRate;

      formatOutput(countryDescription,
                  currencyDescription,
                  currencyCode,
                  noDecimals,
                  reciprocalRate,
                  rateNoDec,
                  rateType,
                  true);
    }
  }
  else
  {
    if ((_trx.eprBDK() == 'T' && _trx.reciprocal() == 'T') ||
        (_trx.eprBDK() == 'F' && !(_trx.reciprocal() == 'T')))
    {
      LOG4CXX_DEBUG(logger, "Formatting multiplicative rates");

      // Display Multiplicative Rates
      //
      formatOutput(countryDescription,
                  currencyDescription,
                  currencyCode,
                  noDecimals,
                  bsrRate,
                  rateNoDec,
                  rateType,
                  false);
    }
    else
    {
      LOG4CXX_DEBUG(logger, "Formatting reciprocal rates");

      // Display Calculated Reciprocal exchange rates
      //
      const ExchRate& reciprocalRate = 1 / bsrRate;

      formatOutput(countryDescription,
                  currencyDescription,
                  currencyCode,
                  noDecimals,
                  reciprocalRate,
                  rateNoDec,
                  rateType,
                  true);
    }
  }

  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::processDetailLine");
}

//-----------------------------------------------------------------------------
//   @method getCurrencyDescription
//
//   Description: Retrieves currency description
//
//   @param   CurrencyCode  - key to Currency cache
//
//   @return  string        - description of currency - returned
//
//-----------------------------------------------------------------------------
const std::string&
LocalCurrencyDisplay::getCurrencyDescription(const CurrencyCode& code, const DateTime& dcDate)
{
  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::getCurrencyDescription");
  LOG4CXX_DEBUG(logger, "DC DATE: " << dcDate.toSimpleString());

  const Currency* currency = nullptr;
  currency = _trx.dataHandle().getCurrency( code );

  if (!currency)
  {
    LOG4CXX_DEBUG(logger, "Currency cache pointer returned NULL");
    return NULL_CURRENCY;
  }

  LOG4CXX_INFO(logger, "CURRENCY DESCRIPTION: " << currency->curName());
  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::getCurrencyDescription");

  return currency->curName();
}

//-----------------------------------------------------------------------------
//   @method getCurrencyNoDecimals
//
//   Description: Retrieves currency number of decimals
//
//   @param   CurrencyCode  - key to Currency cache
//
//   @return   CurrencyNoDec - number of decimals in currency
//
//-----------------------------------------------------------------------------
const CurrencyNoDec&
LocalCurrencyDisplay::getCurrencyNoDecimals(const CurrencyCode& code)
{
  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::getCurrencyNoDecimals");
  DateTime dcDate = _trx.pssLocalDate();

  if (!(_trx.baseDT().isInfinity()))
    dcDate = _trx.baseDT();

  const Currency* currency = nullptr;
  currency = _trx.dataHandle().getCurrency( code );

  if (!currency)
  {
    LOG4CXX_DEBUG(logger, "No Decimals returned");
    return INVALID_NO_DECIMALS;
  }

  LOG4CXX_INFO(logger, "Number of Currency Decimals: " << currency->noDec());
  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::getCurrencyNoDecimals");

  return currency->noDec();
}

//-----------------------------------------------------------------------------
//   @method getCurrentBSRRate
//
//   Description: Retrieves current exchange rate using input currency codes
//
//   @return  bool - true/false whether or not rate retrieved
//
//-----------------------------------------------------------------------------
bool
LocalCurrencyDisplay::getCurrentBSRRate(const CurrencyCode& primaryCurrency,
                                        const CurrencyCode& currency,
                                        const DateTime& date,
                                        CurrencyNoDec& rateNoDec,
                                        DateTime& expirationDate,
                                        Indicator& rateType,
                                        ExchRate& currentRate)
{
  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::getCurrentBSRRate");

  LOG4CXX_DEBUG(logger, "Primary currency: " << primaryCurrency);
  LOG4CXX_DEBUG(logger, "Currency: " << currency);
  LOG4CXX_DEBUG(logger, "PSS DATE: " << _trx.pssLocalDate().toSimpleString());
  LOG4CXX_DEBUG(logger, "DATE: " << date.toSimpleString());
  LOG4CXX_DEBUG(logger, "DATAHANDLE DATE: " << _trx.dataHandle().ticketDate().toSimpleString());

  LOG4CXX_DEBUG(logger, "Primary currency: " << primaryCurrency);
  LOG4CXX_DEBUG(logger, "Currency: " << currency);
  DataHandle dataHandle(_trx.pssLocalDate());

  if (fallback::fixed::reduceTemporaryVectorsFixed())
  {
    const std::vector<BankerSellRate*>& rateVec =
        dataHandle.getBankerSellRate(primaryCurrency, currency, date);

    if (!rateVec.empty())
    {
      const BankerSellRate* bsrRate = rateVec.front();
      expirationDate = bsrRate->expireDate();
      rateNoDec = bsrRate->rateNodec();
      rateType = bsrRate->rateType();
      LOG4CXX_DEBUG(logger, "Expire date: " << bsrRate->expireDate().toSimpleString());
      LOG4CXX_DEBUG(logger, "Rate : " << bsrRate->rate());
      LOG4CXX_DEBUG(logger, "Rate number of decimals : " << bsrRate->rateNodec());
      currentRate = bsrRate->rate();
      return true;
    }
  }
  else
  {
    auto bsrRange = dataHandle.getBankerSellRateRange(primaryCurrency, currency, date);
    if (bsrRange.begin() != bsrRange.end())
    {
      const BankerSellRate* bsrRate = *bsrRange.begin();
      expirationDate = bsrRate->expireDate();
      rateNoDec = bsrRate->rateNodec();
      rateType = bsrRate->rateType();
      LOG4CXX_DEBUG(logger, "Expire date: " << bsrRate->expireDate().toSimpleString());
      LOG4CXX_DEBUG(logger, "Rate : " << bsrRate->rate());
      LOG4CXX_DEBUG(logger, "Rate number of decimals : " << bsrRate->rateNodec());
      currentRate = bsrRate->rate();
      return true;
    }
  }

  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::getCurrentBSRRate");

  return false;
}

//-----------------------------------------------------------------------------
//   @method formatOutput
//
//   Description: Formats the output to the CurrencyTrx ostringstream response
//
//   @param   string        - country description
//   @param   string        - currency description
//   @param   CurrencyCode  - key to Currency cache
//   @param   CurrencyNoDec - number of decimals in currency
//   @param   ExchRate      - rate of exchange
//   @param   CurrencyNoDec - rateNoDec
//
//   @return  void
//-----------------------------------------------------------------------------
void
LocalCurrencyDisplay::formatOutput(const std::string countryDescription,
                                   const std::string currencyDescription,
                                   const CurrencyCode& code,
                                   const CurrencyNoDec& numberDecimals,
                                   const ExchRate& exchangeRate,
                                   CurrencyNoDec& rateNoDec,
                                   Indicator& rateType,
                                   bool isReciprocal)
{
  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::formatOutput");

  _trx.response().setf(std::ios::left, std::ios::adjustfield);
  _trx.response().setf(std::ios::fixed, std::ios::floatfield);

  _trx.response() << std::setw(15) << countryDescription.substr(0, 15).c_str() << " "
                  << std::setw(9) << currencyDescription.substr(0, 9).c_str() << " " << std::setw(3)
                  << code.c_str() << "        " << std::setw(1) << numberDecimals << "      ";

  _trx.response().precision(rateNoDec);

  std::string exchangeRateStr = formatExchangeRate(exchangeRate, rateNoDec, isReciprocal);

  _trx.response() << std::setw(15) << exchangeRateStr << " ";

  if (rateType == 'B')
  {
    _trx.response() << std::setw(3);
    _trx.response() << "BSR"
                    << "\n";
  }
  else if (rateType == 'I')
  {
    _trx.response() << std::setw(3);
    _trx.response() << "ICH"
                    << "\n";
  }
  else
  {
    _trx.response() << std::setw(1);
    _trx.response() << "\n";
  }

  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::formatOutput");
}

//-----------------------------------------------------------------------------
//   @method formatFutureRateLine
//
//   Description: Retrieves currency related output details and BSR exchanges rates
//
//   @param   string        - country description
//   @param   CurrencyCode  - currency - either pricing,national or bank rate currency
//   @param   ExchRate      - exchange rate
//   @param   CurrencyNoDec - rateNoDec
//
//   @return  void
//-----------------------------------------------------------------------------
void
LocalCurrencyDisplay::formatFutureRateLine(const DateTime& effectiveDate,
                                           const ExchRate& futureRate,
                                           CurrencyNoDec& rateNoDec,
                                           Indicator& rateType)
{
  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::formatFutureRateLine");

  const DateTime dcDate = !_trx.baseDT().isInfinity() ? _trx.baseDT() : _trx.pssLocalDate();
  if (TrxUtil::isIcerActivated(_trx, dcDate)) //Remove BSRDSP keyword logic
  {
    if (!(_trx.reciprocal() == 'T'))
    {
      LOG4CXX_DEBUG(logger, "Formatting multiplicative rates");

      // Display Multiplicative Rates
      //
      _trx.response() << "\n";
      _trx.response() << "     *** EFFECTIVE " << effectiveDate.dateToString(DDMMM, "")
                      << "          NEW RATE  ";

      _trx.response().setf(std::ios::left, std::ios::adjustfield);
      _trx.response().setf(std::ios::fixed, std::ios::floatfield);
      _trx.response().precision(rateNoDec);

      std::string futureRateStr = formatExchangeRate(futureRate, rateNoDec, false);

      _trx.response() << std::setw(15) << futureRateStr << " ";
    }
    else
    {
      LOG4CXX_DEBUG(logger, "Formatting reciprocal rates");

      // Display Calculated Reciprocal exchange rates
      //
      const ExchRate& reciprocalRate = 1 / futureRate;

      _trx.response() << "\n";
      _trx.response() << "     *** EFFECTIVE " << effectiveDate.dateToString(DDMMM, "")
                      << "          NEW RATE  ";

      _trx.response().setf(std::ios::left, std::ios::adjustfield);
      _trx.response().setf(std::ios::fixed, std::ios::floatfield);
      _trx.response().precision(rateNoDec);

      std::string futureRateStr = formatExchangeRate(reciprocalRate, rateNoDec, true);

      _trx.response() << std::setw(15) << futureRateStr << " ";
    }
  }
  else
  {
    if ((_trx.eprBDK() == 'T' && _trx.reciprocal() == 'T') ||
        (_trx.eprBDK() == 'F' && !(_trx.reciprocal() == 'T')))
    {
      LOG4CXX_DEBUG(logger, "Formatting multiplicative rates");

      // Display Multiplicative Rates
      //
      _trx.response() << "\n";
      _trx.response() << "     *** EFFECTIVE " << effectiveDate.dateToString(DDMMM, "")
                      << "          NEW RATE  ";

      _trx.response().setf(std::ios::left, std::ios::adjustfield);
      _trx.response().setf(std::ios::fixed, std::ios::floatfield);
      _trx.response().precision(rateNoDec);

      std::string futureRateStr = formatExchangeRate(futureRate, rateNoDec, false);

      _trx.response() << std::setw(15) << futureRateStr << " ";
    }
    else
    {
      LOG4CXX_DEBUG(logger, "Formatting reciprocal rates");

      // Display Calculated Reciprocal exchange rates
      //
      const ExchRate& reciprocalRate = 1 / futureRate;

      _trx.response() << "\n";
      _trx.response() << "     *** EFFECTIVE " << effectiveDate.dateToString(DDMMM, "")
                      << "          NEW RATE  ";

      _trx.response().setf(std::ios::left, std::ios::adjustfield);
      _trx.response().setf(std::ios::fixed, std::ios::floatfield);
      _trx.response().precision(rateNoDec);

      std::string futureRateStr = formatExchangeRate(reciprocalRate, rateNoDec, true);

      _trx.response() << std::setw(15) << futureRateStr << " ";
    }
  }

  if (rateType == 'B')
  {
    _trx.response() << std::setw(3);
    _trx.response() << "BSR"
                    << "\n";
  }
  else if (rateType == 'I')
  {
    _trx.response() << std::setw(3);
    _trx.response() << "ICH"
                    << "\n";
  }
  else
  {
    _trx.response() << std::setw(1);
    _trx.response() << "\n";
  }

  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::formatFutureRateLine");
}

//-----------------------------------------------------------------------------
//
//   @method getTaxRounding
//
//   Description: Retrieves Tax Rounding information for the target currency
//
//   @param   DateTime       - date
//   @param   RoundingFactor - tax rounding factor
//   @param   CurrencyNoDec  - number of decimals in rounding factor
//   @param   RoundingRule   - tax rounding rule
//
//   @return  bool           - true - retrieved tax information,else false
//-----------------------------------------------------------------------------
bool
LocalCurrencyDisplay::getTaxRounding(const CurrencyCode& currencyCode,
                                     DateTime& dcDate,
                                     RoundingFactor& roundingFactor,
                                     CurrencyNoDec& roundingNoDec,
                                     RoundingRule& roundingRule)
{
  LOG4CXX_INFO(logger, "Entered LocalCurrencyDisplay::getTaxRounding");

  LOG4CXX_INFO(logger, "Target Currency code: " << _trx.targetCurrency());
  LOG4CXX_INFO(logger, "Size of Target Currency code: " << _trx.targetCurrency().size());

  const Currency* currency = nullptr;
  currency = _trx.dataHandle().getCurrency( currencyCode );

  if (!currency)
  {
    LOG4CXX_ERROR(logger, "DBAccess getCurrency returned null currency pointer");
    return false;
  }

  if (currency->taxOverrideRoundingUnit() > 0)
  {
    roundingFactor = currency->taxOverrideRoundingUnit();
    roundingNoDec = currency->taxOverrideRoundingUnitNoDec();
    roundingRule = currency->taxOverrideRoundingRule();

    return true;
  }

  const std::string controllingEntityDesc = currency->controllingEntityDesc();
  LOG4CXX_INFO(logger, "Currency country description: " << currency->controllingEntityDesc());

  bool foundNationalCurrency = false;
  bool foundNation = false;
  NationCode nationWithMatchingNationalCurrency;
  NationCode nationCode;

  CurrencyUtil::getControllingNationCode(_trx,
                                         controllingEntityDesc,
                                         nationCode,
                                         foundNation,
                                         foundNationalCurrency,
                                         nationWithMatchingNationalCurrency,
                                         dcDate,
                                         _trx.targetCurrency());

  if (foundNation)
  {
    const TaxNation* taxNation = _trx.dataHandle().getTaxNation(nationCode, dcDate);

    if (taxNation)
    {

      roundingFactor = taxNation->roundingUnit();
      roundingNoDec = taxNation->roundingUnitNodec();
      roundingRule = taxNation->roundingRule();

      LOG4CXX_INFO(logger, "Tax Nation : " << taxNation->nation());
      LOG4CXX_INFO(logger, "Rounding factor: " << roundingFactor);
      LOG4CXX_INFO(logger, "Rounding factor no dec: " << roundingNoDec);
      LOG4CXX_INFO(logger, "Rounding rule: " << roundingRule);

      LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::getTaxRounding");

      return true;
    }

    LOG4CXX_ERROR(logger, "Failed to retrieve tax rounding information for nation: " << nationCode);
  }
  else if (foundNationalCurrency)
  {
    const TaxNation* taxNation =
        _trx.dataHandle().getTaxNation(nationWithMatchingNationalCurrency, dcDate);

    if (taxNation)
    {

      roundingFactor = taxNation->roundingUnit();
      roundingNoDec = taxNation->roundingUnitNodec();
      roundingRule = taxNation->roundingRule();

      LOG4CXX_INFO(logger, "Tax Nation : " << taxNation->nation());
      LOG4CXX_INFO(logger, "Rounding factor: " << roundingFactor);
      LOG4CXX_INFO(logger, "Rounding factor no dec: " << roundingNoDec);
      LOG4CXX_INFO(logger, "Rounding rule: " << roundingRule);

      LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::getTaxRounding");

      return true;
    }

    LOG4CXX_ERROR(logger,
                  "Failed to retrieve tax rounding information for nation: "
                      << nationWithMatchingNationalCurrency);
  }

  return false;
}

//-------------------------------------------------------------------------------
//   @method formatRateLine
//
//   Description: Formats the rate line
//
//   @param   CurrencyCode   - target currency code
//   @param   ExchRate       - exchange rate between these 2 currencies
//   @param   CurrencyCode   - source currency code
//   @param   bool           - whether or not this is a reciprocal rate
//
//   @return  void
//-------------------------------------------------------------------------------
void
LocalCurrencyDisplay::formatRateLine(const CurrencyCode& target,
                                     const ExchRate& exchangeRate,
                                     const CurrencyCode& source,
                                     const CurrencyNoDec& rateNoDec,
                                     bool isReciprocalRate,
                                     Indicator rateType)

{
  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::formatRateLine()");

  LOG4CXX_INFO(logger, "Source Currency : " << _trx.sourceCurrency());
  LOG4CXX_INFO(logger, "Target Currency : " << _trx.targetCurrency());

  if (rateType == 'I')
    _trx.response() << "RATE ICH 1";
  else // its a BSR.
    _trx.response() << "RATE BSR 1";

  if (isReciprocalRate)
  {
    ExchRate reciprocalRate = 1 / exchangeRate;
    std::string rateStr = formatExchangeRate(reciprocalRate, rateNoDec, true);

    LOG4CXX_INFO(logger, "reciprocal rate " << reciprocalRate);
    LOG4CXX_INFO(logger, "reciprocal rate no dec " << rateNoDec);

    _trx.response() << source.c_str();

    if (reciprocalRate < ONE)
    {
      LOG4CXX_DEBUG(logger, "Reciprocal Exchange rate less than one");
      _trx.response() << std::setw(3) << " - " << std::left << std::setw(18);
    }
    else
      _trx.response() << std::setw(4) << " -  " << std::left << std::setw(17);

    _trx.response() << rateStr << target.c_str() << std::endl;
  }
  else
  {
    std::string rateStr = formatExchangeRate(exchangeRate, rateNoDec, false);

    LOG4CXX_DEBUG(logger, "Rate length: " << rateStr.size());

    _trx.response() << target.c_str();

    if (exchangeRate < ONE)
    {
      LOG4CXX_DEBUG(logger, "Exchange rate less than one");
      _trx.response() << std::setw(3) << " - " << std::left << std::setw(18);
    }
    else
      _trx.response() << std::setw(4) << " -  " << std::left << std::setw(17);

    _trx.response() << rateStr << source.c_str() << std::endl;
  }

  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::formatRateLine()");
}
//-------------------------------------------------------------------------------
//   @method formatRoundingLine
//
//   Description: Formats the rounding line for FARES and TAXES
//
//   @param   CurrencyCode   - currency code
//   @param   CurrencyNoDec  - number of decimals in currency
//   @param   MoneyAmount    - amount
//   @param   string         - rounding rule
//   @param   RoundingFactor - rounding factor
//   @param   CurrencyNoDec  - number of decimals in rounding factor
//   @param   string         - application - FARES or TAXES
//
//   @return  void
//-------------------------------------------------------------------------------
void
LocalCurrencyDisplay::formatRoundingLine(const CurrencyCode& currencyCode,
                                         const CurrencyNoDec& targetNoDec,
                                         MoneyAmount& amount,
                                         const std::string& roundingStr,
                                         const RoundingFactor& roundingFactor,
                                         const CurrencyNoDec& roundingNoDec,
                                         const std::string& roundApplication)
{
  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::formatRoundingLine()");
  LOG4CXX_DEBUG(logger, "old precision = " << _trx.response().precision());

  _trx.response().precision(targetNoDec);
  _trx.response().setf(std::ios::left, std::ios::adjustfield);

  LOG4CXX_DEBUG(logger, "Amount = " << amount);

  std::ostringstream roundingFactorStr;

  roundingFactorStr << roundingFactor;

  _trx.response() << std::setw(3) << currencyCode.c_str() << std::setw(8) << "        "
                  << std::setw(10) << amount << std::setw(1) << " " << std::setw(21)
                  << roundingStr.c_str();

  if ((roundingFactor > 0.0) && (roundingFactor < 1.00))
  {
    _trx.response() << std::setw(6);
    _trx.response().setf(std::ios::left, std::ios::adjustfield);
    _trx.response() << roundingFactorStr.str().c_str();
  }
  else
  {
    _trx.response() << std::setw(6);
    _trx.response().setf(std::ios::left, std::ios::adjustfield);
    _trx.response().precision(0);
    _trx.response() << roundingFactor;
  }

  _trx.response().setf(std::ios::left, std::ios::adjustfield);
  _trx.response() << std::setw(3) << " - " << std::setw(5) << roundApplication.c_str() << std::endl;

  LOG4CXX_INFO(logger, "Exiting LocalCurrencyDisplay::formatRoundingLine()");
}

//------------------------------------------------------------------------------------
//   @method formatTruncationLine
//
//   Description: Formats the truncation line
//
//   @param   CurrencyCode   - currency code
//   @param   MoneyAmount    - amount
//   @param   RoundingFactor - fares rounding factor
//   @param   CurrencyNoDec  - number of decimals in fares rounding factor
//   @param   RoundingFactor - taxes rounding factor
//   @param   CurrencyNoDec  - number of decimals in taxes rounding factor
//   @param   bool           - whether or not tax rounding info is available
//
//   @return  void
//------------------------------------------------------------------------------------
void
LocalCurrencyDisplay::formatTruncationLine(const CurrencyCode& currencyCode,
                                           const CurrencyNoDec& noDec,
                                           const MoneyAmount& unroundedAmount,
                                           const RoundingFactor& faresRoundingFactor,
                                           const CurrencyNoDec& faresRoundingNoDec,
                                           const RoundingFactor& taxRoundingFactor,
                                           const CurrencyNoDec& taxRoundingNoDec,
                                           bool taxInfoAvailable)
{
  CurrencyNoDec truncationDecimals = 0;

  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::formatTrunctionLine()");

  LOG4CXX_DEBUG(logger, "Number of decimals in fares rounding factor: " << faresRoundingNoDec);
  LOG4CXX_DEBUG(logger, "fares rounding factor: " << faresRoundingFactor);

  LOG4CXX_DEBUG(logger, "Number of decimals in tax rounding factor: " << taxRoundingNoDec);
  LOG4CXX_DEBUG(logger, "tax rounding factor: " << taxRoundingFactor);

  int numFaresDecimals = 0;
  int numTaxDecimals = 0;

  if (faresRoundingNoDec > 0)
    numFaresDecimals = numTruncationDecimals(faresRoundingFactor, faresRoundingNoDec);
  else
    numFaresDecimals = 1;

  if (taxRoundingNoDec > 0)
    numTaxDecimals = numTruncationDecimals(taxRoundingFactor, taxRoundingNoDec);
  else
    numTaxDecimals = 1;

  if (numFaresDecimals > numTaxDecimals)
    truncationDecimals = numFaresDecimals;
  else
    truncationDecimals = numTaxDecimals;

  if (truncationDecimals < 0)
    truncationDecimals = 1;

  _trx.response() << std::setw(3) << currencyCode.c_str() << std::setw(8) << "        "
                  << std::setw(10);

  _trx.response().setf(std::ios::left, std::ios::adjustfield);
  _trx.response().setf(std::ios::fixed, std::ios::floatfield);

  if (truncationDecimals > 6)
    _trx.response().precision(6);
  else
    _trx.response().precision(truncationDecimals);

  LOG4CXX_DEBUG(logger, "Returned unrounded amount: " << unroundedAmount);

  std::string truncStr;

  CurrencyUtil::formatDouble(unroundedAmount, truncStr, truncationDecimals);

  std::string::size_type idx = truncStr.find(".");

  if (idx == std::string::npos) // lint !e530
  {
    truncStr += ".";

    for (int i = 0; i < truncationDecimals; i++)
      truncStr += "0";
  }
  else
  {
    int actualDecimals = 0;

    for (unsigned int i = (idx + 1); i < truncStr.size(); i++)
      actualDecimals++;

    LOG4CXX_DEBUG(logger, "Actual number of decimals: " << actualDecimals);

    if (actualDecimals < truncationDecimals)
    {
      int numZeroesToAdd = (truncationDecimals - actualDecimals);

      for (int i = 0; i < numZeroesToAdd; i++)
        truncStr += "0";
    }
  }

  LOG4CXX_DEBUG(logger, "TRUNC STR = " << truncStr);

  _trx.response() << truncStr << std::setw(1) << " "
                  << "TRUNCATED" << std::endl;

  LOG4CXX_INFO(logger, "Exiting LocalCurrencyDisplay::formatTrunctionLine()");
}

//------------------------------------------------------------------------------------
//   @method numTruncationDecimals
//
//   Description: According to the rules of voodoo retrieves the number
//                of decimals to be displayed on the truncation line.
//
//   @param   RoundingFactor - rounding factor
//   @param   CurrencyNoDec  - number of decimals in currency
//
//   @return  int            - number of decimals to be displayed
//------------------------------------------------------------------------------------
CurrencyNoDec
LocalCurrencyDisplay::numTruncationDecimals(const RoundingFactor& roundingFactor,
                                            const CurrencyNoDec& noDec)
{
  CurrencyNoDec numDecimals = 0;

  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::numTruncationDecimals()");
  LOG4CXX_DEBUG(logger, "ROUNDING FACTOR: " << roundingFactor);
  LOG4CXX_DEBUG(logger, "NO DEC: " << noDec);

  std::ostringstream tmpStr;

  tmpStr << roundingFactor;

  std::string tmpAmount = tmpStr.str();

  std::string::size_type idx = tmpAmount.find(".");

  if (idx == std::string::npos) // lint !e530
  {
    LOG4CXX_DEBUG(logger, "No decimal found in rounding factor");
    numDecimals = -1;
    return numDecimals;
  }

  LOG4CXX_DEBUG(logger, "Index = " << idx);
  LOG4CXX_DEBUG(logger, "Temp Amount = " << tmpAmount);

  for (unsigned int i = (idx + 1); i < tmpAmount.size(); i++) // lint !e574
  {
    int value = tmpAmount[i] - '0';
    LOG4CXX_DEBUG(logger, "value = " << value);

    if (value > 0)
    {
      numDecimals = i;
      break;
    }
  }

  LOG4CXX_DEBUG(logger, "number of decimals  = " << numDecimals);

  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::numTruncationDecimals()");

  return numDecimals;
} // lint !533

//-----------------------------------------------------------------------------
//   @method formatExchangeRate
//
//   Description: Converts the exchange rate to a string
//
//   @param   ExchRate      - exchange rate
//   @param   CurrencyNoDec - rateNoDec
//
//   @return  string
//-----------------------------------------------------------------------------
std::string
LocalCurrencyDisplay::formatExchangeRate(const ExchRate& exchangeRate,
                                         const CurrencyNoDec& rateNoDec,
                                         bool isReciprocal)
{
  LOG4CXX_INFO(logger, "Entering LocalCurrencyDisplay::formatExchangeRate");

  std::string zeroStr("0");

  LOG4CXX_DEBUG(logger, "Exchange Rate: " << exchangeRate);
  LOG4CXX_DEBUG(logger, "Number of decimals in rate: " << rateNoDec);

  std::ostringstream exchangeRateStr;
  std::string rateStr;

  if (isReciprocal)
  {
    exchangeRateStr.precision(15);

    if (exchangeRate < ONE)
    {
      LOG4CXX_DEBUG(logger, "Exchange rate less than zero");
      exchangeRateStr << std::fixed << exchangeRate;
    }
    else
    {
      LOG4CXX_DEBUG(logger, "Exchange rate not less than zero");
      exchangeRateStr << std::showpoint << std::fixed << exchangeRate;
    }

    std::string tmpRateStr = exchangeRateStr.str();
    LOG4CXX_DEBUG(logger, "tmp rate: " << tmpRateStr);
    LOG4CXX_DEBUG(logger, "tmp rate size: " << tmpRateStr.size());
    std::string::size_type decimalIdx = tmpRateStr.find(".");
    std::string integralPart = tmpRateStr.substr(0, decimalIdx);
    LOG4CXX_DEBUG(logger, "integral part: " << integralPart);

    std::string fractionalPart;

    LOG4CXX_DEBUG(logger, "UnFormatted rate: " << tmpRateStr);

    if (exchangeRate < ONE)
      fractionalPart = tmpRateStr.substr((decimalIdx + 1), (exchangeRateStr.precision() - 2));
    else
      fractionalPart = tmpRateStr.substr((decimalIdx + 1), (exchangeRateStr.precision()));

    LOG4CXX_DEBUG(logger, "fractional part: " << fractionalPart);

    if (integralPart == "0")
    {
      rateStr = "0." + fractionalPart;
      std::string::size_type idx = rateStr.find_last_not_of(zeroStr);

      if (idx != std::string::npos)
        rateStr.erase((idx + 1), rateStr.size());
    }
    else if ((exchangeRate - ONE) < EPSILON)
      rateStr = integralPart + "." + "00000";
    else
    {
      rateStr = integralPart + "." + fractionalPart;

      removeTrailingZeroes(rateStr);
    }

    truncateExchangeRate(rateStr);

    LOG4CXX_DEBUG(logger, "Formatted rate: " << rateStr);
  }
  else
  {
    exchangeRateStr.precision(rateNoDec);
    exchangeRateStr << std::showpoint << std::fixed << exchangeRate;
    std::string tmpRateStr = exchangeRateStr.str();
    LOG4CXX_DEBUG(logger, "tmp rate: " << tmpRateStr);
    LOG4CXX_DEBUG(logger, "tmp rate size: " << tmpRateStr.size());
    std::string::size_type idx = tmpRateStr.find(".");
    std::string integralPart = tmpRateStr.substr(0, idx);
    LOG4CXX_DEBUG(logger, "integral part: " << integralPart);

    std::string fractionalPart;

    if (integralPart == "0")
      fractionalPart = tmpRateStr.substr((idx + 1), tmpRateStr.size());
    else
      fractionalPart = tmpRateStr.substr((idx + 1), rateNoDec);

    LOG4CXX_DEBUG(logger, "fractional part: " << fractionalPart);

    if (integralPart == "0")
    {
      rateStr = "0." + fractionalPart;
      removeTrailingZeroes(rateStr);
    }
    else if ((exchangeRate - ONE) < EPSILON)
      rateStr = integralPart + "." + "00000";
    else
    {
      if (rateNoDec > 0)
        rateStr = integralPart + "." + fractionalPart;
      else if (fractionalPart.empty())
        rateStr = integralPart;
    }

    truncateExchangeRate(rateStr);
  }

  LOG4CXX_DEBUG(logger, "Formatted rate: " << rateStr);

  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::formatExchangeRate");

  return rateStr;
}

//-----------------------------------------------------------------------------
//   @method removeTrailingZeroes
//
//   Description: Removes trailing zeroes
//
//   @param   string      - exchange rate
//
//   @return  string
//-----------------------------------------------------------------------------
void
LocalCurrencyDisplay::removeTrailingZeroes(std::string& rateStr)
{
  LOG4CXX_INFO(logger, "Entered LocalCurrencyDisplay::removeTrailingZeroes");

  std::string zeroStr("0");

  std::string::size_type idx = rateStr.find_last_not_of(zeroStr);

  if (idx != std::string::npos)
    rateStr.erase((idx + 1), rateStr.size());

  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::removeTrailingZeroes");
}
//-----------------------------------------------------------------------------
//   @method truncateExchangeRate
//
//   Description: Exchange rate can display a max of 15 chars
//
//   @param   string      - exchange rate
//
//   @return  string
//-----------------------------------------------------------------------------
void
LocalCurrencyDisplay::truncateExchangeRate(std::string& rateStr)
{
  LOG4CXX_INFO(logger, "Entered LocalCurrencyDisplay::truncateExchangeRate");
  LOG4CXX_DEBUG(logger, "RATE TO CHECK FOR TRUNCATION: " << rateStr);

  if (rateStr.size() > 15)
  {
    int numCharsToErase = rateStr.size() - 15;
    LOG4CXX_DEBUG(logger, "Number of chars to erase: " << numCharsToErase);
    rateStr.erase((15), numCharsToErase);
  }

  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::truncateExchangeRate");
}

//-----------------------------------------------------------------------------
//   @method formatRoundingMsg
//
//   Description: Formats the rounding message for FARES and TAXES
//                according to the rounding rule
//
//   @param   RoundingRule   - rounding rule
//   @param   string         - formatted string , in/out parameter
//
//   @return  void
//-----------------------------------------------------------------------------
void
LocalCurrencyDisplay::formatRoundingMsg(RoundingRule& roundingRule, std::string& roundingMsg)
{
  LOG4CXX_INFO(logger, "Entered LocalCurrencyDisplay::formatRoundingMsg");

  if (roundingRule == UP)
    roundingMsg = ROUND_UP;
  else if (roundingRule == DOWN)
    roundingMsg = ROUND_DOWN;
  else if (roundingRule == NEAREST)
    roundingMsg = ROUND_NEAREST;
  else
    roundingMsg = ROUND_NONE;

  LOG4CXX_INFO(logger, "Leaving LocalCurrencyDisplay::formatRoundingMsg");
}
} // namespace tse
