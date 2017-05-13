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
#include "Common/BSRCurrencyConverter.h"

#include "Common/BSRCollectionResults.h"
#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/Currency.h"
#include "DBAccess/CurrencySelection.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/NUCInfo.h"

#include <iostream>
#include <vector>

namespace tse
{
FIXEDFALLBACK_DECL(reduceTemporaryVectorsFixed);

static Logger logger("atseintl.Common.BSRCurrencyConverter");

BSRCurrencyConverter::BSRCurrencyConverter() {}

bool
BSRCurrencyConverter::convert(CurrencyConversionRequest& ccRequest,
                              CurrencyCollectionResults* results)
{
  // with flat amount (dummy) fare using calculation currency override as
  // fare currency, we might see convertion from NUC to others here
  if (UNLIKELY(ccRequest.source().code() == NUC))
  {
    if (ccRequest.target().code() == NUC)
    {
      ccRequest.target().value() = 0.0f;
      return false;
    }

    NUCCurrencyConverter ncc;
    return ncc.convert(ccRequest, results);
  }

  CurrencyCode conversionCurrency;
  CurrencyCode salesCurrencyCode;
  bool commonSalesLocCurrency = false;
  bool convertRC = true;
  ConversionRule conversionRule;
  double overRideRate1 = 0;
  double overRideRate2 = 0;
  bool secondRateOverride = false;
  bool firstRateOverride = false;
  bool dualRateOverride = false;
  ExchRate exchangeRate = 0.0;
  CurrencyNoDec exchangeRateNoDec = 0;
  Indicator exchangeRateType;
  BSRCollectionResults* bsrResults = nullptr;
  bool collectResults = false;
  RoundingRule rule = NONE;
  RoundingFactor factor = 0;
  MoneyAmount unRoundedAmount = 0;
  CurrencyNoDec roundingFactorNoDec = 0;
  bool hasOverride = false;
  bool hasSpecifiedCurrency = false;

  bsrResults = dynamic_cast<BSRCollectionResults*>(results);

  if (bsrResults)
  {
    if (LIKELY(bsrResults->collect()))
      collectResults = true;
  }

  const Agent* agent = ccRequest.getRequest().ticketingAgent();
  if (UNLIKELY(!agent))
  {
    LOG4CXX_INFO(logger, "Agent is null");
    return false;
  }

  if (LIKELY(ccRequest.applicationType() != CurrencyConversionRequest::FAREDISPLAY))
  {
    if (UNLIKELY(!validateInput(ccRequest.target(), ccRequest.source(), *agent, ccRequest.ticketDate())))
    {
      ccRequest.target().value() = 0.0f;
      return false;
    }
  }

  LOG4CXX_DEBUG(logger, "Entered BSRCurrencyConverter::convert");
  LOG4CXX_DEBUG(logger, "Target currency = " << ccRequest.target().code());
  LOG4CXX_DEBUG(logger, "Source currency = " << ccRequest.source().code());
  LOG4CXX_DEBUG(logger, "Source Amount   = " << ccRequest.source().value());
  LOG4CXX_DEBUG(logger, "Source Amount   = " << ccRequest.source().value());

  const Loc* agentLoc = agent->agentLocation();

  if (UNLIKELY(!(agentLoc)))
  {
    LOG4CXX_INFO(logger, "Agent location is null");
    return false;
  }

  const DateTime& ticketingDate = ccRequest.ticketDate();

  if (collectResults)
  {
    bsrResults->sourceCurrency() = ccRequest.source().code();
    bsrResults->targetCurrency() = ccRequest.target().code();
    bsrResults->sourceAmount() = ccRequest.source().value();
    bsrResults->sourceAmountNoDec() = ccRequest.source().noDec(ticketingDate);
    bsrResults->convertedAmountNoDec() = ccRequest.target().noDec(ticketingDate);
    bsrResults->nation() = agentLoc->nation();
  }

  if (ccRequest.applicationType() == CurrencyConversionRequest::DC_CONVERT)
  {
    LOG4CXX_DEBUG(logger, "Conversion request type is: DC CONVERT");
    commonSalesLocCurrency = ccRequest.commonSalesLocCurrency();

    salesCurrencyCode = ccRequest.salesLocCurrency();
    conversionCurrency = ccRequest.conversionCurrency();
  }
  else
  {

    commonSalesLocCurrency =
        determinePricingCurrency(ccRequest, salesCurrencyCode, conversionCurrency, hasOverride);
  }

  LOG4CXX_DEBUG(logger, "Sales Location determined");

  PricingRequest& pricingRequest = const_cast<PricingRequest&>(ccRequest.getRequest());
  bool isDAEntry = false;
  if (TrxUtil::newDiscountLogic(ccRequest.getRequest(), *ccRequest.trx()))
    isDAEntry = pricingRequest.isDAEntryNew();
  else
    isDAEntry = pricingRequest.isDAEntry();

  if (LIKELY(ccRequest.applicationType() != CurrencyConversionRequest::DC_CONVERT &&
      ccRequest.applicationType() != CurrencyConversionRequest::FAREDISPLAY && !isDAEntry))
  {

    if (!hasOverride)
    {
      if ((salesCurrencyCode == ccRequest.source().code()) &&
          (ccRequest.target().code() == ccRequest.source().code()))
      {
        LOG4CXX_DEBUG(logger, "Sales Currency Code is same is Source Currency Code");
        LOG4CXX_DEBUG(logger, "Sales Currency Code: " << salesCurrencyCode);
        LOG4CXX_DEBUG(logger, "Source Currency Code: " << ccRequest.source().code());
        ccRequest.target().value() = ccRequest.source().value();

        if (collectResults)
        {
          bsrResults->setEquivCurrencyCode(salesCurrencyCode);

          bsrResults->exchangeRate1() = 1.0;
          bsrResults->exchangeRate1NoDec() = bsrResults->sourceAmountNoDec();
          bsrResults->exchangeRateType1() = ' ';
        }

        LOG4CXX_DEBUG(logger, "No Conversion required");
        return true;
      }
    }
  }

  if ((ccRequest.applicationType() != CurrencyConversionRequest::TAXES &&
       ccRequest.applicationType() != CurrencyConversionRequest::DC_CONVERT &&
       ccRequest.applicationType() != CurrencyConversionRequest::FAREDISPLAY))
  {
    if (UNLIKELY(ccRequest.getRequest().rateAmountOverride() > 0))
    {
      overRideRate1 = ccRequest.getRequest().rateAmountOverride();
      LOG4CXX_DEBUG(logger, "Override rate = " << overRideRate1);
      firstRateOverride = true;
    }

    if (UNLIKELY(ccRequest.getRequest().secondRateAmountOverride() > 0))
    {
      overRideRate2 = ccRequest.getRequest().secondRateAmountOverride();
      LOG4CXX_DEBUG(logger, "Override rate 2 = " << overRideRate2);

      secondRateOverride = true;
    }

    if (LIKELY(ccRequest.getOptions()))
    {
      if (ccRequest.getOptions()->isMOverride())
      {
        if (LIKELY(!(ccRequest.getOptions()->currencyOverride().empty())))
        {
          hasSpecifiedCurrency = true;

          if (UNLIKELY(firstRateOverride && secondRateOverride))
          {
            if (salesCurrencyCode == ccRequest.getOptions()->currencyOverride())
            {
              LOG4CXX_ERROR(logger,
                            "BSR Double Override Error: Sales Currency same as WPM Currency");
              ErrorResponseException ex(ErrorResponseException::INVALID_INPUT);
              throw ex;
            }
          }
        }
      }
    }

    if (UNLIKELY(firstRateOverride && secondRateOverride))
      dualRateOverride = true;
  }

  if (TrxUtil::isIcerActivated(*ccRequest.trx(), ccRequest.ticketDate()))
  {
    LOG4CXX_DEBUG(logger, "ICER is activated - Apply direct rate");

    LOG4CXX_DEBUG(logger, "Sales currency is:  " << salesCurrencyCode);
    LOG4CXX_DEBUG(logger, "Target currency is: " << ccRequest.target().code());
    LOG4CXX_DEBUG(logger, "Source currency is: " << ccRequest.source().code());
    LOG4CXX_DEBUG(logger, "Source amount is:  " << ccRequest.source().value());
    LOG4CXX_DEBUG(logger, "TicketDate is:  " << ccRequest.ticketDate().toSimpleString());

    CurrencyCode bsrPrimeCurrency = ccRequest.source().code();
    CurrencyCode bsrCurrency = ccRequest.target().code();;
    conversionRule = MULTIPLY;

    if (collectResults)
    {
      bsrResults->bsrPrimeCurrencyGroup1() = bsrPrimeCurrency;
      bsrResults->bsrCurrency1() = bsrCurrency;
    }

    LOG4CXX_DEBUG(logger, "BSR Prime currency is:  " << bsrPrimeCurrency);
    LOG4CXX_DEBUG(logger, "BSR currency is:        " << bsrCurrency);

    double overrideRate = 0.0;

    if (UNLIKELY(secondRateOverride && hasSpecifiedCurrency))
      overrideRate = overRideRate2;
    else if (UNLIKELY(firstRateOverride))
      overrideRate = overRideRate1;

    int conversionRC = applyDirectConversion(ccRequest.target(),
                                             ccRequest.source(),
                                             bsrPrimeCurrency, // BSR Prime Currency
                                             bsrCurrency, // BSR Currency
                                             conversionRule,
                                             ccRequest,
                                             exchangeRate,
                                             exchangeRateNoDec,
                                             exchangeRateType,
                                             rule,
                                             factor,
                                             unRoundedAmount,
                                             roundingFactorNoDec,
                                             false,
                                             overrideRate);

    if (UNLIKELY(conversionRC == BSR_RATE_NOT_FOUND))
    {
      LOG4CXX_ERROR(logger, "Exchange Rate Not Available");
      return false;
    }

    if (collectResults)
    {
      bsrResults->exchangeRate1() = ( overrideRate > 0.0 ) ? overrideRate : exchangeRate;

      if (ccRequest.applicationType() == CurrencyConversionRequest::TAXES ||
          ccRequest.applicationType() == CurrencyConversionRequest::FAREDISPLAY)
      {
        bsrResults->taxReciprocalRate1() = (1 / exchangeRate);
        bsrResults->taxReciprocalRate1NoDec() = exchangeRateNoDec;
      }
      LOG4CXX_DEBUG(logger, "Exchange rate 1 " << bsrResults->exchangeRate1());
      bsrResults->exchangeRateType1() = exchangeRateType;
      LOG4CXX_DEBUG(logger, "Exchange rateType 1 " << bsrResults->exchangeRateType1());
      bsrResults->exchangeRate1NoDec() = exchangeRateNoDec;
      LOG4CXX_DEBUG(logger, "Exchange rateNoDecimals 1 " << bsrResults->exchangeRate1NoDec());

      bsrResults->convertedAmount() = ccRequest.target().value();
      LOG4CXX_DEBUG(logger, "Rounding rule  " << rule);
      bsrResults->roundingRule1() = rule;
      bsrResults->roundingFactor1() = factor;
      LOG4CXX_DEBUG(logger, "Rounding factor  " << factor);
      bsrResults->targetUnroundedAmount() = unRoundedAmount;
      LOG4CXX_DEBUG(logger, "Unrounded amount  " << bsrResults->targetUnroundedAmount());
      unRoundedAmount = 0;
      LOG4CXX_DEBUG(logger, "Unrounded amount  " << bsrResults->targetUnroundedAmount());
      bsrResults->roundingFactorNoDec1() = roundingFactorNoDec;
      roundingFactorNoDec = 0;
    }
    return true;
  }

  if (commonSalesLocCurrency)
  {
    LOG4CXX_DEBUG(logger, "Found common sales currency");

    LOG4CXX_DEBUG(logger, "Sales currency is:  " << salesCurrencyCode);
    LOG4CXX_DEBUG(logger, "Target currency is: " << ccRequest.target().code());
    LOG4CXX_DEBUG(logger, "Source currency is: " << ccRequest.source().code());
    LOG4CXX_DEBUG(logger, "Source amount is:  " << ccRequest.source().value());
    LOG4CXX_DEBUG(logger, "TicketDate is:  " << ccRequest.ticketDate().toSimpleString());

    CurrencyCode bsrCurrency;
    CurrencyCode bsrPrimeCurrency;

    if (salesCurrencyCode == ccRequest.source().code())
    {
      LOG4CXX_DEBUG(logger, "Sales currency is same as source currency");

      bsrPrimeCurrency = salesCurrencyCode;
      conversionRule = MULTIPLY;
      bsrCurrency = ccRequest.target().code();
    }
    else
    {
      LOG4CXX_DEBUG(logger, "Sales currency is different than source currency");

      bsrPrimeCurrency = ccRequest.target().code();
      conversionRule = DIVIDE;
      bsrCurrency = ccRequest.source().code();
    }

    if (collectResults)
    {
      bsrResults->bsrPrimeCurrencyGroup1() = bsrPrimeCurrency;
      bsrResults->bsrCurrency1() = bsrCurrency;
    }

    LOG4CXX_DEBUG(logger, "BSR Prime currency is:  " << bsrPrimeCurrency);
    LOG4CXX_DEBUG(logger, "BSR currency is:        " << bsrCurrency);

    double overrideRate = 0.0;

    if (secondRateOverride && hasSpecifiedCurrency)
      overrideRate = overRideRate2;
    else if (firstRateOverride)
      overrideRate = overRideRate1;

    int conversionRC = applyDirectConversion(ccRequest.target(),
                                             ccRequest.source(),
                                             bsrPrimeCurrency, // BSR Prime Currency
                                             bsrCurrency, // BSR Currency
                                             conversionRule,
                                             ccRequest,
                                             exchangeRate,
                                             exchangeRateNoDec,
                                             exchangeRateType,
                                             rule,
                                             factor,
                                             unRoundedAmount,
                                             roundingFactorNoDec,
                                             false,
                                             overrideRate);

    if (conversionRC == BSR_RATE_NOT_FOUND)
    {
      LOG4CXX_DEBUG(logger,
                    "BSR vector is empty, attempting conversion through Conversion currency");

      if (conversionCurrency.empty())
      {
        LOG4CXX_ERROR(logger, "No Conversion Currency for nation");
        return false;
      }

      Money intermediate(conversionCurrency);

      ExchRate exchangeRate1 = 0;
      CurrencyNoDec exchangeRateNoDec1 = 0;
      Indicator exchangeRateType1;

      bool foundFirstRate = bsrRate(salesCurrencyCode,
                                    intermediate.code(),
                                    ccRequest.ticketDate(),
                                    exchangeRate1,
                                    exchangeRateNoDec1,
                                    exchangeRateType1);

      if (foundFirstRate)
      {
        if (conversionRule == MULTIPLY)
        {
          bsrPrimeCurrency = salesCurrencyCode;
          bsrCurrency = intermediate.code();
        }
        else
        {
          bsrPrimeCurrency = intermediate.code();
          bsrCurrency = ccRequest.source().code();
        }
      }
      else
      {
        return false;
      }

      if (collectResults)
      {
        bsrResults->bsrPrimeCurrencyGroup1() = bsrPrimeCurrency;
        bsrResults->bsrCurrency1() = bsrCurrency;
      }

      LOG4CXX_DEBUG(logger, "BSR Prime currency is:  " << bsrPrimeCurrency);
      LOG4CXX_DEBUG(logger, "BSR currency is:        " << bsrCurrency);

      double overrideRate = 0.0;

      int conversionRC = applyDirectConversion(intermediate,
                                               ccRequest.source(),
                                               bsrPrimeCurrency, // BSR Prime Currency
                                               bsrCurrency, // BSR Currency
                                               conversionRule,
                                               ccRequest,
                                               exchangeRate,
                                               exchangeRateNoDec,
                                               exchangeRateType,
                                               rule,
                                               factor,
                                               unRoundedAmount,
                                               roundingFactorNoDec,
                                               false,
                                               overrideRate);

      if (conversionRC)
      {
        LOG4CXX_DEBUG(logger, "Intermediate Converted rounded amount: " << intermediate.value());

        if (collectResults)
        {
          bsrResults->intermediateCurrency() = intermediate.code();
          bsrResults->intermediateNoDec() = intermediate.noDec(ticketingDate);

          bsrResults->exchangeRate1() = exchangeRate;

          if (ccRequest.applicationType() == CurrencyConversionRequest::TAXES ||
              ccRequest.applicationType() == CurrencyConversionRequest::FAREDISPLAY)
          {
            bsrResults->taxReciprocalRate1() = (1 / exchangeRate);
            bsrResults->taxReciprocalRate1NoDec() = exchangeRateNoDec;
          }

          bsrResults->exchangeRateType1() = exchangeRateType;
          bsrResults->exchangeRate1NoDec() = exchangeRateNoDec;
          bsrResults->intermediateAmount() = intermediate.value();
          bsrResults->roundingRule1() = rule;
          bsrResults->roundingFactor1() = factor;
          bsrResults->intermediateUnroundedAmount() = unRoundedAmount;
          bsrResults->roundingFactorNoDec1() = roundingFactorNoDec;
          unRoundedAmount = 0;
          roundingFactorNoDec = 0;
        }

        if (foundFirstRate)
        {
          if (conversionRule == MULTIPLY)
          {
            bsrPrimeCurrency = intermediate.code();
            bsrCurrency = ccRequest.target().code();
          }
          else
          {
            bsrPrimeCurrency = ccRequest.target().code();
            bsrCurrency = intermediate.code();
          }
        }
        else
        {
          LOG4CXX_ERROR(logger, "Exchange Rate not Available");
          return false;
        }

        if (collectResults)
        {
          bsrResults->bsrPrimeCurrencyGroup2() = bsrPrimeCurrency;
          bsrResults->bsrCurrency2() = bsrCurrency;
        }

        LOG4CXX_DEBUG(logger, "BSR Prime currency is:  " << bsrPrimeCurrency);
        LOG4CXX_DEBUG(logger, "BSR currency is:        " << bsrCurrency);

        if (secondRateOverride && hasSpecifiedCurrency)
          overrideRate = overRideRate2;
        else if (firstRateOverride)
          overrideRate = overRideRate1;

        int conversionRC = applyDirectConversion(ccRequest.target(),
                                                 intermediate,
                                                 bsrPrimeCurrency, // BSR Prime Currency
                                                 bsrCurrency, // BSR Currency
                                                 conversionRule,
                                                 ccRequest,
                                                 exchangeRate,
                                                 exchangeRateNoDec,
                                                 exchangeRateType,
                                                 rule,
                                                 factor,
                                                 unRoundedAmount,
                                                 roundingFactorNoDec,
                                                 false,
                                                 overrideRate);

        LOG4CXX_DEBUG(logger, "Final Converted rounded amount: " << ccRequest.target().value());

        if (!conversionRC)
        {
          LOG4CXX_ERROR(logger, "Exchange Rate Not Available");
          convertRC = false;
        }
        else
        {
          if (collectResults)
          {
            if (firstRateOverride)
            {
              bsrResults->exchangeRate2() = overRideRate1;
              LOG4CXX_DEBUG(logger, "SECOND RATE AMOUNT5: " << bsrResults->exchangeRate2());
            }
            else if (secondRateOverride && hasSpecifiedCurrency)
            {
              bsrResults->exchangeRate2() = overRideRate2;
              LOG4CXX_DEBUG(logger, "SECOND RATE AMOUNT6: " << bsrResults->exchangeRate2());
            }
            else
            {
              bsrResults->exchangeRate2() = exchangeRate;
              LOG4CXX_DEBUG(logger, "SECOND RATE AMOUNT7: " << bsrResults->exchangeRate2());
            }

            if (ccRequest.applicationType() == CurrencyConversionRequest::TAXES)
            {
              bsrResults->taxReciprocalRate2() = (1 / exchangeRate);
              bsrResults->taxReciprocalRate2NoDec() = exchangeRateNoDec;
            }

            bsrResults->exchangeRateType2() = exchangeRateType;
            bsrResults->exchangeRate2NoDec() = exchangeRateNoDec;
            bsrResults->convertedAmount() = ccRequest.target().value();
            bsrResults->roundingRule2() = rule;
            bsrResults->roundingFactor2() = factor;
            bsrResults->targetUnroundedAmount() = unRoundedAmount;
            bsrResults->roundingFactorNoDec2() = roundingFactorNoDec;
            roundingFactorNoDec = 0;
            unRoundedAmount = 0;
          }
        }
      }
      else
      {
        LOG4CXX_DEBUG(logger,
                      "Exchange Rate Not Available: for currency: " << intermediate.code());
        convertRC = false;
      }
    }
    else
    {

      if (collectResults)
      {
        LOG4CXX_DEBUG(logger, "Exchange rate 1 before flip " << bsrResults->exchangeRate1());

        if (firstRateOverride)
          bsrResults->exchangeRate1() = overRideRate1;
        else
          bsrResults->exchangeRate1() = exchangeRate;

        if (ccRequest.applicationType() == CurrencyConversionRequest::TAXES ||
            ccRequest.applicationType() == CurrencyConversionRequest::FAREDISPLAY)
        {
          bsrResults->taxReciprocalRate1() = (1 / exchangeRate);
          bsrResults->taxReciprocalRate1NoDec() = exchangeRateNoDec;
        }
        LOG4CXX_DEBUG(logger, "Exchange rate 1 " << bsrResults->exchangeRate1());
        bsrResults->exchangeRateType1() = exchangeRateType;
        LOG4CXX_DEBUG(logger, "Exchange rateType 1 " << bsrResults->exchangeRateType1());
        bsrResults->exchangeRate1NoDec() = exchangeRateNoDec;
        LOG4CXX_DEBUG(logger, "Exchange rateNoDecimals 1 " << bsrResults->exchangeRate1NoDec());

        bsrResults->convertedAmount() = ccRequest.target().value();
        LOG4CXX_DEBUG(logger, "Rounding rule  " << rule);
        bsrResults->roundingRule1() = rule;
        bsrResults->roundingFactor1() = factor;
        LOG4CXX_DEBUG(logger, "Rounding factor  " << factor);
        bsrResults->targetUnroundedAmount() = unRoundedAmount;
        LOG4CXX_DEBUG(logger, "Unrounded amount  " << bsrResults->targetUnroundedAmount());
        unRoundedAmount = 0;
        LOG4CXX_DEBUG(logger, "Unrounded amount  " << bsrResults->targetUnroundedAmount());
        bsrResults->roundingFactorNoDec1() = roundingFactorNoDec;
        roundingFactorNoDec = 0;
      }
    }
  }
  else
  {
    LOG4CXX_DEBUG(logger, "Source or Target Currency does not match Sales Loc currency");

    LOG4CXX_DEBUG(
        logger,
        "BSR vector is empty, attempting conversion through Common Sales location currency");
    LOG4CXX_DEBUG(logger, "Sales currency is:  " << salesCurrencyCode);
    LOG4CXX_DEBUG(logger, "Target currency is: " << ccRequest.target().code());
    LOG4CXX_DEBUG(logger, "Source currency is: " << ccRequest.source().code());
    LOG4CXX_DEBUG(logger, "Source amount is:  " << ccRequest.source().value());

    Money intermediate(salesCurrencyCode);

    ExchRate exchangeRate1 = 0;
    CurrencyNoDec exchangeRateNoDec1 = 0;
    RoundingRule rule1 = NONE;
    RoundingFactor factor1 = 0;
    roundingFactorNoDec = 0;
    Indicator exchangeRateType1;

    bool foundFirstRate = bsrRate(intermediate.code(),
                                  ccRequest.source().code(),
                                  ccRequest.ticketDate(),
                                  exchangeRate1,
                                  exchangeRateNoDec1,
                                  exchangeRateType1);

    ExchRate exchangeRate2 = 0;
    CurrencyNoDec exchangeRateNoDec2 = 0;
    RoundingRule rule2 = NONE;
    RoundingFactor factor2 = 0;
    ExchRate overrideRate = 0;
    Indicator exchangeRateType2;

    bool foundSecondRate = bsrRate(intermediate.code(),
                                   ccRequest.target().code(),
                                   ccRequest.ticketDate(),
                                   exchangeRate2,
                                   exchangeRateNoDec2,
                                   exchangeRateType2);

    if ((foundFirstRate == true) && (foundSecondRate == true))
    {
      LOG4CXX_DEBUG(logger,
                    "Found both rates: exchangeRate1: " << exchangeRate1
                                                        << ", exchangeRate2: " << exchangeRate2);
      LOG4CXX_DEBUG(logger,
                    "Found both decimals: exchangeRateNoDec1: "
                        << exchangeRateNoDec1 << ", exchangeRateNoDec2: " << exchangeRateNoDec2);

      if (dualRateOverride || (firstRateOverride && hasSpecifiedCurrency))
        overrideRate = overRideRate1;
      else
        overrideRate = 0;

      applyDirectConversion(intermediate,
                            ccRequest.source(),
                            intermediate.code(), // BSR Prime Currency
                            ccRequest.source().code(), // BSR Currency
                            DIVIDE,
                            ccRequest,
                            exchangeRate1,
                            exchangeRateNoDec1,
                            exchangeRateType1,
                            rule1,
                            factor1,
                            unRoundedAmount,
                            roundingFactorNoDec,
                            true,
                            overrideRate);

      LOG4CXX_DEBUG(logger, "Intermediate value: " << intermediate.value());
      LOG4CXX_DEBUG(logger, "Intermediate Currency: " << intermediate.code());

      if (collectResults)
      {
        bsrResults->intermediateCurrency() = intermediate.code();
        bsrResults->intermediateNoDec() = intermediate.noDec(ticketingDate);

        if (dualRateOverride || (firstRateOverride && hasSpecifiedCurrency))
        {
          bsrResults->exchangeRate1() = overRideRate1;
          LOG4CXX_DEBUG(logger,
                        "USING OVERRIDE RATE FOR EXCH RATE 1: " << bsrResults->exchangeRate1());
        }
        else
          bsrResults->exchangeRate1() = exchangeRate1;

        if (ccRequest.applicationType() == CurrencyConversionRequest::TAXES ||
            ccRequest.applicationType() == CurrencyConversionRequest::FAREDISPLAY)
        {
          bsrResults->taxReciprocalRate1() = (1 / exchangeRate1);
          bsrResults->taxReciprocalRate1NoDec() = exchangeRateNoDec1;
        }

        bsrResults->exchangeRateType1() = exchangeRateType1;
        bsrResults->exchangeRate1NoDec() = exchangeRateNoDec1;
        bsrResults->intermediateAmount() = intermediate.value();
        bsrResults->roundingRule1() = rule1;
        bsrResults->roundingFactor1() = factor1;
        bsrResults->intermediateUnroundedAmount() = unRoundedAmount;
        bsrResults->roundingFactorNoDec1() = roundingFactorNoDec;
        LOG4CXX_DEBUG(logger, "Rounding Factor dec1:  " << bsrResults->roundingFactorNoDec1());
        roundingFactorNoDec = 0;
        unRoundedAmount = 0;
        bsrResults->bsrPrimeCurrencyGroup1() = intermediate.code();
        bsrResults->bsrCurrency1() = ccRequest.source().code();
      }

      LOG4CXX_DEBUG(logger, "Target value: " << ccRequest.target().value());

      if (dualRateOverride || (secondRateOverride && hasSpecifiedCurrency))
        overrideRate = overRideRate2;
      else if (firstRateOverride && !hasSpecifiedCurrency)
        overrideRate = overRideRate1;
      else
        overrideRate = 0;

      applyDirectConversion(ccRequest.target(),
                            intermediate,
                            intermediate.code(), // BSR Prime Currency
                            ccRequest.target().code(), // BSR Currency
                            MULTIPLY,
                            ccRequest,
                            exchangeRate2,
                            exchangeRateNoDec2,
                            exchangeRateType2,
                            rule2,
                            factor2,
                            unRoundedAmount,
                            roundingFactorNoDec,
                            true,
                            overrideRate);

      LOG4CXX_DEBUG(logger, "Final Converted rounded amount: " << ccRequest.target().value());

      if (collectResults)
      {
        if (!isZeroAmount(overrideRate))
        {
          bsrResults->exchangeRate2() = overrideRate;
          LOG4CXX_DEBUG(logger, "SECOND RATE AMOUNT1: " << bsrResults->exchangeRate2());
        }
        else
        {
          bsrResults->exchangeRate2() = exchangeRate2;
          LOG4CXX_DEBUG(logger, "SECOND RATE AMOUNT2: " << bsrResults->exchangeRate2());
        }

        if (ccRequest.applicationType() == CurrencyConversionRequest::TAXES ||
            ccRequest.applicationType() == CurrencyConversionRequest::FAREDISPLAY)
        {
          bsrResults->taxReciprocalRate2() = (1 / exchangeRate2);
          bsrResults->taxReciprocalRate2NoDec() = exchangeRateNoDec2;
        }

        bsrResults->exchangeRateType2() = exchangeRateType2;
        bsrResults->exchangeRate2NoDec() = exchangeRateNoDec2;
        bsrResults->convertedAmount() = ccRequest.target().value();
        bsrResults->roundingRule2() = rule2;
        bsrResults->roundingFactor2() = factor2;
        bsrResults->targetUnroundedAmount() = unRoundedAmount;
        bsrResults->roundingFactorNoDec2() = roundingFactorNoDec;
        LOG4CXX_DEBUG(logger, "Rounding Factor dec2:  " << bsrResults->roundingFactorNoDec2());
        roundingFactorNoDec = 0;
        unRoundedAmount = 0;
        bsrResults->bsrPrimeCurrencyGroup2() = intermediate.code();
        bsrResults->bsrCurrency2() = ccRequest.target().code();
      }
    }
    else
    {
      if (conversionCurrency.empty())
      {
        LOG4CXX_ERROR(logger, "Exchange Rate not Available");
        return false;
      }

      Money intermediate(conversionCurrency);
      ExchRate overrideRate = 0;

      if (dualRateOverride || (firstRateOverride && hasSpecifiedCurrency))
        overrideRate = overRideRate1;
      else
        overrideRate = 0;

      int conversionRC = applyDirectConversion(intermediate,
                                               ccRequest.source(),
                                               intermediate.code(), // BSR Prime Currency
                                               ccRequest.source().code(), // BSR Currency
                                               DIVIDE,
                                               ccRequest,
                                               exchangeRate,
                                               exchangeRateNoDec,
                                               exchangeRateType,
                                               rule1,
                                               factor1,
                                               unRoundedAmount,
                                               roundingFactorNoDec,
                                               false,
                                               overrideRate);

      LOG4CXX_DEBUG(logger, "Intermediate value: " << intermediate.value());
      LOG4CXX_DEBUG(logger, "Intermediate Currency: " << intermediate.code());

      if (conversionRC)
      {
        if (collectResults)
        {
          bsrResults->intermediateCurrency() = intermediate.code();
          bsrResults->intermediateNoDec() = intermediate.noDec(ticketingDate);
          bsrResults->intermediateAmount() = intermediate.value();

          if (dualRateOverride || (firstRateOverride && hasSpecifiedCurrency))
            bsrResults->exchangeRate1() = overrideRate;
          else
            bsrResults->exchangeRate1() = exchangeRate;

          if (ccRequest.applicationType() == CurrencyConversionRequest::TAXES)
          {
            bsrResults->taxReciprocalRate1() = (1 / exchangeRate);
            bsrResults->taxReciprocalRate1NoDec() = exchangeRateNoDec;
          }

          bsrResults->exchangeRateType1() = exchangeRateType;
          bsrResults->exchangeRate1NoDec() = exchangeRateNoDec;
          bsrResults->roundingRule1() = rule1;
          bsrResults->roundingFactor1() = factor1;
          bsrResults->intermediateUnroundedAmount() = unRoundedAmount;
          bsrResults->roundingFactorNoDec1() = roundingFactorNoDec;
          LOG4CXX_DEBUG(logger, "Rounding Factor dec1:  " << bsrResults->roundingFactorNoDec1());
          roundingFactorNoDec = 0;
          unRoundedAmount = 0;
          bsrResults->bsrPrimeCurrencyGroup1() = intermediate.code();
          bsrResults->bsrCurrency1() = ccRequest.source().code();
        }

        LOG4CXX_DEBUG(logger, "Target value: " << ccRequest.target().value());

        if (dualRateOverride || (secondRateOverride && hasSpecifiedCurrency))
          overrideRate = overRideRate2;
        else if (firstRateOverride && !hasSpecifiedCurrency)
          overrideRate = overRideRate1;
        else
          overrideRate = 0;

        int conversionRC = applyDirectConversion(ccRequest.target(),
                                                 intermediate,
                                                 intermediate.code(), // BSR Prime Currency
                                                 ccRequest.target().code(), // BSR Currency
                                                 MULTIPLY,
                                                 ccRequest,
                                                 exchangeRate,
                                                 exchangeRateNoDec,
                                                 exchangeRateType,
                                                 rule2,
                                                 factor2,
                                                 unRoundedAmount,
                                                 roundingFactorNoDec,
                                                 false,
                                                 overrideRate);

        LOG4CXX_DEBUG(logger, "Final Converted rounded amount: " << ccRequest.target().value());

        if (!conversionRC)
        {
          LOG4CXX_DEBUG(logger,
                        "Conversion Failed: " << intermediate.code() << " to "
                                              << ccRequest.target().code());
          convertRC = false;
        }
        else
        {
          if (collectResults)
          {
            if (secondRateOverride || firstRateOverride)
            {
              bsrResults->exchangeRate2() = overrideRate;
              LOG4CXX_DEBUG(logger, "SECOND RATE AMOUNT3: " << bsrResults->exchangeRate2());
            }
            else
            {
              bsrResults->exchangeRate2() = exchangeRate;
              LOG4CXX_DEBUG(logger, "SECOND RATE AMOUNT4: " << bsrResults->exchangeRate2());
            }

            if (ccRequest.applicationType() == CurrencyConversionRequest::TAXES)
            {
              bsrResults->taxReciprocalRate2() = (1 / exchangeRate);
              bsrResults->taxReciprocalRate2NoDec() = exchangeRateNoDec;
            }

            bsrResults->exchangeRateType2() = exchangeRateType;
            bsrResults->exchangeRate2NoDec() = exchangeRateNoDec;
            bsrResults->convertedAmount() = ccRequest.target().value();
            bsrResults->roundingRule2() = rule2;
            bsrResults->roundingFactor2() = factor2;
            bsrResults->targetUnroundedAmount() = unRoundedAmount;
            bsrResults->roundingFactorNoDec2() = roundingFactorNoDec;
            LOG4CXX_DEBUG(logger, "Rounding Factor dec2:  " << bsrResults->roundingFactorNoDec2());
            roundingFactorNoDec = 0;
            unRoundedAmount = 0;
            bsrResults->bsrPrimeCurrencyGroup2() = intermediate.code();
            bsrResults->bsrCurrency2() = ccRequest.target().code();
          }
        }
      }
      else
      {
        LOG4CXX_DEBUG(logger, "Bank Rate Not Available: for currency: " << intermediate.code());
        convertRC = false;
      }
    }
  }

  return convertRC;
}

bool
BSRCurrencyConverter::determinePricingCurrency(CurrencyConversionRequest& conversionRequest,
                                               CurrencyCode& salesCurrencyCode,
                                               CurrencyCode& conversionCurrency,
                                               bool& hasOverride)
{
  bool pricingCurrencyRC = false;
  DataHandle dataHandle(conversionRequest.ticketDate());

  const NationCode& nationCode = getPointOfSaleNation(conversionRequest);

  if (UNLIKELY(!nationCode.size()))
    return false;

  LOG4CXX_DEBUG(logger, "Point of Sale Nation: " << nationCode);

  const Nation* nation = dataHandle.getNation(nationCode, conversionRequest.ticketDate());

  if (LIKELY(nation != nullptr))
  {
    conversionCurrency = nation->conversionCur();

    if (nation->alternateCur().size())
    {
      LOG4CXX_DEBUG(logger, "Pricing currency is not blank");
      salesCurrencyCode = nation->alternateCur();
    }
    else if (LIKELY(nation->primeCur().size()))
    {
      LOG4CXX_DEBUG(logger, "National currency is not blank");
      salesCurrencyCode = nation->primeCur();
    }

    LOG4CXX_DEBUG(logger, "Conversion Currency: " << conversionCurrency);
    LOG4CXX_DEBUG(logger, "National Currency:   " << nation->primeCur());
    LOG4CXX_DEBUG(logger, "Pricing Currency:    " << nation->alternateCur());
    LOG4CXX_DEBUG(logger, "Sales Currency Code: " << salesCurrencyCode);

    if ((salesCurrencyCode == conversionRequest.source().code()) ||
        (salesCurrencyCode == conversionRequest.target().code()))
    {
      pricingCurrencyRC = true;
    }

    if (UNLIKELY(conversionRequest.trx() &&
        conversionRequest.trx()->excTrxType() == PricingTrx::PORT_EXC_TRX))
    {
      if ((conversionRequest.getOptions() &&
           !conversionRequest.getOptions()->baseFareCurrencyOverride().empty()) ||
          (conversionRequest.trx()->itin().front()->calcCurrencyOverride() ==
               conversionRequest.target().code() &&
           conversionRequest.target().code() != conversionRequest.source().code()))
      {
        hasOverride = true;
      }
    }

    if (LIKELY(conversionRequest.getOptions()))
    {
      if (conversionRequest.getOptions()->isMOverride())
      {
        if (LIKELY(!(conversionRequest.getOptions()->currencyOverride().empty())))
          hasOverride = true;
      }
      else
      {

        if ((conversionRequest.getRequest().salePointOverride().empty()))
        {
          const Customer* cust = (conversionRequest.getRequest().ticketingAgent()->agentTJR());

          if (cust)
          {
            if (LIKELY(!(cust->defaultCur().empty())))
            {
              salesCurrencyCode = cust->defaultCur();
              LOG4CXX_DEBUG(
                  logger,
                  "Setting sales currency code to TJR Default Cur: " << cust->defaultCur());
              LOG4CXX_DEBUG(logger, "Sales Currency Code: " << salesCurrencyCode);
            }
          }
        }
      }
    }
  }
  else
  {
    LOG4CXX_DEBUG(logger, "Nation is nil");
    pricingCurrencyRC = false;
  }

  return pricingCurrencyRC;
}

int
BSRCurrencyConverter::applyDirectConversion(Money& target,
                                            const Money& source,
                                            const CurrencyCode& bsrPrimeCurrency,
                                            const CurrencyCode& bsrCurrency,
                                            ConversionRule conversionRule,
                                            CurrencyConversionRequest& request,
                                            ExchRate& rate,
                                            CurrencyNoDec& rateNoDec,
                                            Indicator& rateType,
                                            RoundingRule& rule,
                                            RoundingFactor& factor,
                                            MoneyAmount& unRoundedAmount,
                                            CurrencyNoDec& roundingFactorNoDec,
                                            bool rateFound,
                                            double overRideRate)
{
  int conversionRC = CONVERSION_SUCCEEDED;
  ExchRate exchangeRate = 0.0;
  CurrencyNoDec exchangeRateNoDec = 0;
  bool bsrRC = true;
  RoundingFactor roundingFactor = 0;
  RoundingRule roundingRule = NONE;
  CurrencyNoDec factorNoDec = 0;
  Indicator exchangeRateType;

  if (LIKELY(!rateFound))
  {
    LOG4CXX_DEBUG(logger, "Pre-rate lookup not performed ");
    bsrRC = bsrRate(bsrPrimeCurrency,
                    bsrCurrency,
                    request.ticketDate(),
                    exchangeRate,
                    exchangeRateNoDec,
                    exchangeRateType);
  }

  if (LIKELY(bsrRC))
  {
    if (UNLIKELY(overRideRate > 0))
    {
      exchangeRate = overRideRate;
      if (TrxUtil::isIcerActivated(*request.trx(), request.ticketDate()))
      {
        rateNoDec = exchangeRateNoDec;
      }
    }
    else
    {
      if (UNLIKELY(rateFound))
      {
        exchangeRate = rate;
        LOG4CXX_DEBUG(logger, "Multiplicative rate: " << exchangeRate);
      }
      else
      {
        rate = exchangeRate;
        rateNoDec = exchangeRateNoDec;
        rateType = exchangeRateType;
      }
    }

    bool applyRC = applyExchRate(target, source, exchangeRate, conversionRule);

    if (LIKELY(applyRC))
    {
      unRoundedAmount = target.value();

      if (request.applicationType() &&
          request.applicationType() != CurrencyConversionRequest::NO_ROUNDING)
      {
        bool roundingRC = applyRounding(
            target.code(), target, request.ticketDate(), roundingRule, roundingFactor, factorNoDec);

        if (UNLIKELY(!roundingRC))
        {
          rule = NONE;
          factor = 0;
          roundingFactorNoDec = 0;
        }
        else
        {
          rule = roundingRule;
          factor = roundingFactor;
          roundingFactorNoDec = factorNoDec;
        }
      }
    }
    else
      LOG4CXX_ERROR(logger, "Failed to apply exchange rate");
  }
  else
    conversionRC = BSR_RATE_NOT_FOUND;

  return conversionRC;
}

bool
BSRCurrencyConverter::bsrRate(const CurrencyCode& bsrPrimeCurrency,
                              const CurrencyCode& bsrCurrency,
                              const DateTime& ticketDate,
                              ExchRate& rate,
                              CurrencyNoDec& rateNoDec,
                              Indicator& rateType)
{
  DataHandle dataHandle(ticketDate);

  LOG4CXX_DEBUG(logger, "Ticket date for bsrrate: " << ticketDate.toSimpleString());

  if (fallback::fixed::reduceTemporaryVectorsFixed())
  {
    const std::vector<BankerSellRate*>& bsrVect =
        dataHandle.getBankerSellRate(bsrPrimeCurrency, bsrCurrency, ticketDate);

    if (LIKELY(!bsrVect.empty()))
    {
      for (const auto bsrRate : bsrVect)
      {
        if (LIKELY((bsrRate->primeCur() == bsrPrimeCurrency) && (bsrRate->cur() == bsrCurrency)))
        {
          LOG4CXX_DEBUG(logger, "BSR found");
          LOG4CXX_DEBUG(logger, "Exchange Rate: " << bsrRate->rate());
          LOG4CXX_DEBUG(logger, "Exchange Rate No decimals: " << bsrRate->rateNodec());
          LOG4CXX_DEBUG(logger, "Exchange Rate Type: " << bsrRate->rateType());

          rate = bsrRate->rate();
          rateNoDec = bsrRate->rateNodec();
          rateType = bsrRate->rateType();
          return true;
        }
      }

      return false;
    }
    else
    {
      LOG4CXX_INFO(logger, "Bank Rate Not Available: " << bsrPrimeCurrency << " - " << bsrCurrency);
      return false;
    }
  }
  else
  {
    auto bsrRange = dataHandle.getBankerSellRateRange(bsrPrimeCurrency, bsrCurrency, ticketDate);

    if (LIKELY(!bsrRange.empty()))
    {
      for (const auto bsrRate : bsrRange)
      {
        if (LIKELY((bsrRate->primeCur() == bsrPrimeCurrency) && (bsrRate->cur() == bsrCurrency)))
        {
          LOG4CXX_DEBUG(logger, "BSR found");
          LOG4CXX_DEBUG(logger, "Exchange Rate: " << bsrRate->rate());
          LOG4CXX_DEBUG(logger, "Exchange Rate No decimals: " << bsrRate->rateNodec());
          LOG4CXX_DEBUG(logger, "Exchange Rate Type: " << bsrRate->rateType());

          rate = bsrRate->rate();
          rateNoDec = bsrRate->rateNodec();
          rateType = bsrRate->rateType();
          return true;
        }
      }

      return false;
    }
    else
    {
      LOG4CXX_INFO(logger, "Bank Rate Not Available: " << bsrPrimeCurrency << " - " << bsrCurrency);
      return false;
    }
  }
}

bool
BSRCurrencyConverter::applyExchRate(Money& target,
                                    const Money& source,
                                    ExchRate& exchangeRate,
                                    ConversionRule conversionRule)
{
  bool applyRC = true;

  LOG4CXX_DEBUG(logger, "BSR Exchange Rate : " << exchangeRate);

  if (LIKELY(exchangeRate))
  {
    if (LIKELY(conversionRule == MULTIPLY))
      target.value() = (source.value() * exchangeRate);
    else if (conversionRule == DIVIDE)
      target.value() = (source.value() / exchangeRate);

    LOG4CXX_DEBUG(logger, "Converted amount: " << target.value());
  }
  else
    LOG4CXX_DEBUG(logger, "Exchange Rate has zero value");

  return applyRC;
}

bool
BSRCurrencyConverter::applyRounding(const CurrencyCode& salesCurrencyCode,
                                    Money& target,
                                    const DateTime& ticketDate,
                                    RoundingRule& rule,
                                    RoundingFactor& factor,
                                    CurrencyNoDec& roundingFactorNoDec)
{
  double nucFactor = 0.0;
  double roundingFactor = 0.0;

  RoundingRule roundingRule;
  CarrierCode carrier; // TODO : remove carrier from NUC lookup
  CurrencyNoDec nucRoundingFactorNoDec;
  DateTime discontinueDate(pos_infin);
  DateTime effectiveDate(pos_infin);
  CurrencyNoDec nucFactorNoDec;

  bool nucRC = getNucInfo(carrier,
                          salesCurrencyCode,
                          ticketDate,
                          nucFactor,
                          roundingFactor,
                          roundingRule,
                          nucRoundingFactorNoDec,
                          nucFactorNoDec,
                          discontinueDate,
                          effectiveDate);

  if (LIKELY(roundingFactor))
  {
    if (LIKELY(nucRC))
    {
      rule = roundingRule;
      factor = roundingFactor;

      roundingFactorNoDec = nucRoundingFactorNoDec;

      bool roundRC = round(target, roundingFactor, roundingRule);

      if (UNLIKELY(!roundRC))
      {
        LOG4CXX_ERROR(logger, "Rounding failed");
        return false;
      }
    }
  }

  return true;
}

bool
BSRCurrencyConverter::applyRoundingRule(Money& target,
                                        const DateTime& ticketDate,
                                        RoundingRule rule)
{
  double nucFactor = 0.0;
  double roundingFactor = 0.0;
  RoundingRule roundingRule;
  CarrierCode carrier; // TODO : remove carrier from NUC lookup
  CurrencyNoDec nucRoundingFactorNoDec;
  DateTime discontinueDate(pos_infin);
  DateTime effectiveDate(pos_infin);
  CurrencyNoDec nucFactorNoDec;

  bool nucRC = getNucInfo(carrier,
                          target.code(),
                          ticketDate,
                          nucFactor,
                          roundingFactor,
                          roundingRule,
                          nucRoundingFactorNoDec,
                          nucFactorNoDec,
                          discontinueDate,
                          effectiveDate);

  if (roundingFactor)
  {
    if (nucRC)
    {
      bool roundRC = round(target, roundingFactor, rule);

      if (!roundRC)
      {
        LOG4CXX_ERROR(logger, "Rounding failed");
        return false;
      }
    }
  }

  return true;
}

const NationCode&
BSRCurrencyConverter::getPointOfSaleNation(CurrencyConversionRequest& conversionRequest)
{
  DataHandle dataHandle(conversionRequest.ticketDate());

  LOG4CXX_DEBUG(logger, "Entering getPointOfSaleNation method");

  if (LIKELY(conversionRequest.applicationType() != CurrencyConversionRequest::DC_CONVERT))
  {
    if (!(conversionRequest.getRequest().salePointOverride().empty()))
    {
      const LocCode& salesOverrideLoc = conversionRequest.getRequest().salePointOverride();
      const Loc* loc =
          conversionRequest.dataHandle().getLoc(salesOverrideLoc, conversionRequest.ticketDate());

      if (LIKELY(loc))
      {
        LOG4CXX_DEBUG(logger, "Sales Override Nation code is: " << loc->nation());
        return (loc->nation());
      }
    }
  }

  LOG4CXX_DEBUG(logger, "Leaving getPointOfSaleNation method");

  LOG4CXX_DEBUG(logger,
                "Agent Nation code = "
                    << conversionRequest.getRequest().ticketingAgent()->agentLocation()->nation());
  return (conversionRequest.getRequest().ticketingAgent()->agentLocation()->nation());
}

const NationCode&
BSRCurrencyConverter::getPointOfSaleNation(const PricingTrx& trx)
{
  DateTime currentDate = trx.transactionStartTime();

  LOG4CXX_DEBUG(logger, "Entering getPointOfSaleNation method");

  if (!(trx.getRequest()->salePointOverride().empty()))
  {
    const LocCode& salesOverrideLoc = trx.getRequest()->salePointOverride();
    DataHandle& dataHandle = const_cast<DataHandle&>(trx.dataHandle());
    const Loc* loc = dataHandle.getLoc(salesOverrideLoc, currentDate);

    if (loc)
    {
      LOG4CXX_DEBUG(logger, "Sales Override Nation code is: " << loc->nation());
      return (loc->nation());
    }
  }

  LOG4CXX_DEBUG(logger, "Leaving getPointOfSaleNation method");

  LOG4CXX_DEBUG(
      logger,
      "Agent Nation code = " << trx.getRequest()->ticketingAgent()->agentLocation()->nation());
  return (trx.getRequest()->ticketingAgent()->agentLocation()->nation());
}

bool
BSRCurrencyConverter::hasIndirectEquivAmtOverride(PricingTrx& trx)
{
  std::vector<std::string> equivAmtCurrencies;

  LOG4CXX_INFO(logger, "Entering BSRCurrencyConverter::hasIndirectEquivAmtOverride");

  std::vector<Itin*>::const_iterator itinIter = trx.itin().begin();
  std::vector<Itin*>::const_iterator itinEnd = trx.itin().end();

  const NationCode& nationCode = getPointOfSaleNation(trx);

  if (!nationCode.size())
    return false;

  for (; itinIter != itinEnd; itinIter++)
  {
    std::vector<FarePath*>::const_iterator fPathIter = (*itinIter)->farePath().begin();
    std::vector<FarePath*>::const_iterator fPathEnd = (*itinIter)->farePath().end();

    for (; fPathIter != fPathEnd; fPathIter++)
    {
      FarePath* farePath = (*fPathIter);

      if (!farePath)
        continue;

      std::vector<PricingUnit*>::const_iterator puIter = farePath->pricingUnit().begin();
      std::vector<PricingUnit*>::const_iterator puEnd = farePath->pricingUnit().end();

      const PaxType* requestedPaxType = farePath->paxType();

      if (!requestedPaxType)
        continue;

      for (; puIter != puEnd; puIter++)
      {
        PricingUnit* pricingUnit = (*puIter);

        if (!pricingUnit)
          continue;

        std::vector<FareUsage*>::const_iterator fuIter = pricingUnit->fareUsage().begin();
        std::vector<FareUsage*>::const_iterator fuEnd = pricingUnit->fareUsage().end();

        for (; fuIter != fuEnd; fuIter++)
        {
          FareUsage* fareUsage = (*fuIter);

          if (!fareUsage)
            continue;

          PaxTypeFare* paxTypeFare = fareUsage->paxTypeFare();

          if (!paxTypeFare)
            continue;

          if (requestedPaxType->paxType() != fareUsage->paxTypeFare()->paxType()->paxType())
          {
            LOG4CXX_DEBUG(logger, "Requested pax type : " << requestedPaxType->paxType());
            LOG4CXX_DEBUG(
                logger, "Fareusage pax type : " << fareUsage->paxTypeFare()->paxType()->paxType());

            return false;
          }
          else
          {

            FareMarket* fareMarket = fareUsage->paxTypeFare()->fareMarket();

            if (fareMarket)
            {
              PaxTypeBucket* paxTypeCortege = fareMarket->paxTypeCortege(requestedPaxType);

              if (paxTypeCortege)
              {
                LOG4CXX_DEBUG(
                    logger,
                    "PAXTYPE CORTEGE CURRENCY: " << paxTypeCortege->equivAmtOverrideCurrency());
                equivAmtCurrencies.push_back(paxTypeCortege->equivAmtOverrideCurrency());
              }
            }

          } // if fare passenger type equals requested passenger type

        } // for all FareUsages

      } // for all PricingUnits

    } // for all FarePaths

  } // for every itin in request

  if (!equivAmtCurrencies.size())
    return false;

  for (size_t i = 0; i < equivAmtCurrencies.size(); i++)
  {
    LOG4CXX_DEBUG(logger, "Equivalent amount currency code = " << equivAmtCurrencies[i]);

    if (equivAmtCurrencies[0] != equivAmtCurrencies[i])
    {
      LOG4CXX_DEBUG(logger, "Not all Equivalent amount currency codes are equal returning false");
      return false;
    }
  }

  if ((!equivAmtCurrencies.empty()) && (!equivAmtCurrencies[0].empty()))
  {
    LOG4CXX_DEBUG(logger, "Modifying Options currency override");
    LOG4CXX_DEBUG(logger, "C45 Options currency code = " << trx.getOptions()->currencyOverride());
    trx.getOptions()->currencyOverride() = equivAmtCurrencies[0];
    LOG4CXX_DEBUG(logger, "Indirect Equivalent amount currency code = " << equivAmtCurrencies[0]);
    return true;
  }

  LOG4CXX_INFO(logger, "Leaving BSRCurrencyConverter::hasIndirectEquivAmtOverride");

  return false;
}

} //tse
