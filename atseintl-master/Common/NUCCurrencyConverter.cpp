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
#include "Common/NUCCurrencyConverter.h"

#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/CurrencyUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NUCCollectionResults.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/Response.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/NUCInfo.h"

#include <string>

namespace tse
{

Logger
NUCCurrencyConverter::_logger("atseintl.Common.NUCCurrencyConverter");

const double NUCCurrencyConverter::USE_INTERNATIONAL_ROUNDING = -1;
const double NUCCurrencyConverter::NO_ROUNDING = 0;
const std::string
NUCCurrencyConverter::MISSING_NUC_RATE_FOR("MISSING NUC RATE FOR ");

NUCCurrencyConverter::NUCCurrencyConverter() {}

bool
NUCCurrencyConverter::convert(CurrencyConversionRequest& request,
                              CurrencyCollectionResults* results,
                              CurrencyConversionCache* cache)
{
  double nucFactor = 0.0;
  double roundingFactor = 0.0;
  bool convertToNucs = true;
  CurrencyCode lookUpCurrency;
  RoundingRule roundingRule;
  CarrierCode carrier; // TODO: Remove this in April
  CurrencyNoDec roundingFactorNoDec;
  CurrencyNoDec nucFactorNoDec;
  NUCCollectionResults* nccResults = nullptr;
  bool collectResults = false;
  DateTime discontinueDate(pos_infin);
  DateTime effectiveDate(pos_infin);

  LOG4CXX_DEBUG(_logger, "Entered NUCCurrencyConverter");

  nccResults = dynamic_cast<NUCCollectionResults*>(results);

  if (nccResults)
  {
    if (LIKELY(nccResults->collect()))
      collectResults = true;
  }

  if (LIKELY(request.applicationType() != CurrencyConversionRequest::FAREDISPLAY))
  {
    if (UNLIKELY(!validateInput(request.target(), request.source())))
    {
      request.target().value() = 0.0f;
      return false;
    }
  }

  LOG4CXX_DEBUG(_logger, "Completed data validation");

  // Determine whether or not we are converting to or from NUCs
  //
  if (request.source().isNuc())
  {
    lookUpCurrency = request.target().code();
    convertToNucs = false;
  }
  else if (LIKELY(request.target().isNuc()))
    lookUpCurrency = request.source().code();

  LOG4CXX_DEBUG(_logger, "Currency: " << lookUpCurrency);

  bool nucRC = getNucInfo(carrier,
                          lookUpCurrency,
                          request.ticketDate(),
                          nucFactor,
                          roundingFactor,
                          roundingRule,
                          roundingFactorNoDec,
                          nucFactorNoDec,
                          discontinueDate,
                          effectiveDate,
                          cache);

  if (!nucRC)
  {
    LOG4CXX_ERROR(
        _logger,
        "No NUC information retrieved from DBAccess layer for Currency: " << lookUpCurrency);
    LOG4CXX_ERROR(_logger, "Ticketing Date : " << request.ticketDate().toSimpleString());
    std::string errMsg(MISSING_NUC_RATE_FOR);
    errMsg += lookUpCurrency;
    ErrorResponseException ex(ErrorResponseException::MISSING_NUC_RATE, errMsg.c_str());
    throw ex;
  }

  if (!convertToNucs && request.trx() != nullptr)
  {
    const PricingTrx& trx = *request.trx();

    if ((trx.excTrxType() == PricingTrx::PORT_EXC_TRX ||
         trx.excTrxType() == PricingTrx::AR_EXC_TRX ||
         trx.excTrxType() == PricingTrx::AF_EXC_TRX) &&
        trx.getRequest()->roeOverride() != 0.0f)
    {
      nucFactor = trx.getRequest()->roeOverride();
      nucFactorNoDec = trx.getRequest()->roeOverrideNoDec();
    }
  }

  if (collectResults)
  {
    nccResults->exchangeRate() = nucFactor;
    nccResults->discontinueDate() = discontinueDate;
    nccResults->effectiveDate() = effectiveDate;
    nccResults->exchangeRateNoDec() = nucFactorNoDec;
    nccResults->roundingFactor() = roundingFactor;
    nccResults->roundingRule() = roundingRule;
  }

  if (LIKELY(request.applicationType() != CurrencyConversionRequest::FAREDISPLAY))
  {
    if (isZeroAmount(request.source()))
    {
      request.target().value() = 0.0;
      return true;
    }
  }

  if (convertToNucs)
  {
    // Convert from a currency to NUCS
    request.target().value() = request.source().value()/nucFactor;

    LOG4CXX_DEBUG(_logger,
                  "CONVERTING SOURCE AMOUNT TO NUCS: " << std::setprecision(12)
                                                       << request.source().value());
    LOG4CXX_DEBUG(_logger, "CONVERTING SOURCE CODE: " << request.source().code());
    LOG4CXX_DEBUG(_logger, "CONVERTING NUC RATIO: " << nucFactor);
    LOG4CXX_DEBUG(_logger, "CONVERTING RESULT: " << request.target().value());
    LOG4CXX_DEBUG(_logger, "ROUNDING RESULT: " << request.roundFare());

    if (UNLIKELY(collectResults))
      nccResults->nucAmount() = request.target().value();

    if (UNLIKELY(!request.roundFare()))
      return true;

    CurrencyUtil::truncateNUCAmount(request.target().value());
    return true;
  }

  // Convert from NUCs to a currency
  request.target().value() = request.source().value()*nucFactor;

  LOG4CXX_DEBUG(_logger,
                "CONVERTING SOURCE AMOUNT FROM NUCS: " << std::setprecision(12)
                                                       << request.source().value());
  LOG4CXX_DEBUG(_logger, "CONVERTING NUC RATIO: " << nucFactor);
  LOG4CXX_DEBUG(_logger, "CONVERTING RESULT: " << request.target().value());
  LOG4CXX_DEBUG(_logger, "Rounding Rule: " << roundingRule);
  LOG4CXX_DEBUG(_logger, "International NUC Rounding Factor: " << roundingFactor);

  if (UNLIKELY(!request.roundFare()))
    return true;

  return roundFare(request.target(),
                   roundingRule,
                   roundingFactor,
                   request.ticketDate(),
                   request.dataHandle(),
                   request.useInternationalRounding());
}

//-------------------------------------------------------------------
//
//   @method convertBaseFare
//
//   Description: Converts the Base Fare NUC amount into a local currency
//
//   @param  PricingTrx    - transaction
//   @param  FarePath      - fare path.
//   @param  MoneyAmount&  - converted base fare amount - return param
//   @param  CurrencyNoDec - number of decimals in converted base fare amount - return param
//   @param  CurrencyCode  - base fare currency code  - return param
//   @param  ExchRate      - nuc exchange rate - return param
//   @param  CurrencyNoDec - number of decimals in nuc exchange rate  - return param
//   @param  DateTime      - nuc effective date - return param
//
//   @return bool          - true , conversion succeeded, else false.
//
//
//-------------------------------------------------------------------
bool
NUCCurrencyConverter::convertBaseFare(PricingTrx& trx,
                                      const FarePath& farePath,
                                      MoneyAmount nucAmount,
                                      MoneyAmount& convertedAmount,
                                      CurrencyNoDec& convertedAmtNoDec,
                                      CurrencyCode& convertedCurrencyCode,
                                      ExchRate& roeRate,
                                      CurrencyNoDec& roeRateNoDec,
                                      DateTime& nucEffectiveDate,
                                      DateTime& nucDiscontinueDate,
                                      bool useInternationalRounding,
                                      bool roundBaseFare)
{
  CurrencyCode nuc("NUC");

  const Itin* itin = farePath.itin(); // lint !e530
  LOG4CXX_INFO(_logger, "Entering NUCCurrencyConverter::convertBaseFare");

  if (LIKELY(itin))
  {
    const GeoTravelType& itinTravelType = itin->geoTravelType();

    Money nucBaseFare(nucAmount, nuc);

    CurrencyCode baseFareCurrency;

    if (UNLIKELY((trx.excTrxType() == PricingTrx::PORT_EXC_TRX ||
         trx.excTrxType() == PricingTrx::AR_EXC_TRX ||
         trx.excTrxType() == PricingTrx::AF_EXC_TRX) &&
        !trx.getOptions()->baseFareCurrencyOverride().empty()))
    {
      baseFareCurrency = trx.getOptions()->baseFareCurrencyOverride();
    }
    else if (itin->geoTravelType() == GeoTravelType::Transborder)
    {
      baseFareCurrency = checkBaseFareCurrency(itin, farePath);
    }
    else
    {
      LOG4CXX_DEBUG(_logger, "FARE PATH BASE FARE: " << farePath.baseFareCurrency());
      baseFareCurrency = farePath.baseFareCurrency();
    }

    LOG4CXX_DEBUG(_logger,
                  "IS DOMESTIC US: " << !ItinUtil::isDomesticOfNation(itin, UNITED_STATES));
    LOG4CXX_DEBUG(_logger, "IS TRANSBORDER: " << static_cast<int>(itin->geoTravelType()));

    bool isDomesticPeru = ItinUtil::isDomesticPeru(&trx, itin);

    if ((itin->geoTravelType() == GeoTravelType::Domestic) || (itin->geoTravelType() == GeoTravelType::ForeignDomestic))
      checkDomesticRounding(trx, itin, baseFareCurrency, useInternationalRounding);

    LOG4CXX_DEBUG(_logger, "USING INTL ROUNDING: " << useInternationalRounding);

    LOG4CXX_DEBUG(_logger, "BASE FARE: " << baseFareCurrency);
    Money baseFare(baseFareCurrency);

    if (UNLIKELY(const_cast<FarePath&>(farePath).applyNonIATARounding(trx)))
      baseFare.setApplyNonIATARounding();

    if (farePath.calculationCurrency() != NUC)
    {
      double roundingFactor = 0.0;
      RoundingRule roundingRule;

      double nucFactor = 0.0;
      CarrierCode carrier;
      CurrencyNoDec roundingFactorNoDec;
      CurrencyNoDec nucFactorNoDec;
      DateTime discontinueDate(pos_infin);
      DateTime effectiveDate(pos_infin);

      baseFare.value() = nucAmount;
      convertedCurrencyCode = baseFare.code();

      const Currency* currency = nullptr;
      currency = trx.dataHandle().getCurrency( convertedCurrencyCode );

      if (currency)
      {
        convertedAmtNoDec = currency->noDec();
        LOG4CXX_DEBUG(_logger, "NO OF DECIMALS IN CURRENCY: " << convertedAmtNoDec);
        LOG4CXX_DEBUG(_logger,
                      "TICKETING DT: " << trx.getRequest()->ticketingDT().toSimpleString());
        LOG4CXX_DEBUG(_logger, "CURRENCY EXP DATE: " << currency->expireDate().toSimpleString());
        LOG4CXX_DEBUG(_logger, "CURRENCY EXP DATE: " << currency->effDate().toSimpleString());
      }

      bool nucRC = getNucInfo(carrier,
                              baseFare.code(),
                              trx.ticketingDate(),
                              nucFactor,
                              roundingFactor,
                              roundingRule,
                              roundingFactorNoDec,
                              nucFactorNoDec,
                              discontinueDate,
                              effectiveDate);

      if (!nucRC)
      {
        std::string errMsg(MISSING_NUC_RATE_FOR);
        errMsg += baseFare.code();
        ErrorResponseException ex(ErrorResponseException::MISSING_NUC_RATE, errMsg.c_str());
        throw ex;
      }

      bool roundRC = true;

      if (!isZeroAmount(baseFare))
      {

        if (roundBaseFare && !isDomesticPeru)
          roundRC = roundFare(baseFare,
                              roundingRule,
                              roundingFactor,
                              trx.ticketingDate(),
                              trx.dataHandle(),
                              useInternationalRounding);
      }

      convertedAmount = baseFare.value();
      nucEffectiveDate = effectiveDate;
      nucDiscontinueDate = discontinueDate;

      if ((trx.excTrxType() == PricingTrx::PORT_EXC_TRX ||
           trx.excTrxType() == PricingTrx::AR_EXC_TRX ||
           trx.excTrxType() == PricingTrx::AF_EXC_TRX) &&
          trx.getRequest()->roeOverride() != 0.0f)
      {
        nucFactor = trx.getRequest()->roeOverride();
        nucFactorNoDec = trx.getRequest()->roeOverrideNoDec();
      }

      roeRate = nucFactor;
      roeRateNoDec = nucFactorNoDec;

      return roundRC;
    }

    NUCCollectionResults nucResults;
    nucResults.collect() = true;

    DateTime convertDT = defineConversionDate(trx, farePath);

    CurrencyConversionRequest request(
        baseFare, nucBaseFare, convertDT, *(trx.getRequest()), trx.dataHandle());
    request.setTrx(&trx);
    request.useInternationalRounding() = useInternationalRounding;

    request.roundFare() = roundBaseFare;
    LOG4CXX_DEBUG(_logger, "ROUND BASE FARE: " << request.roundFare());

    if (itinTravelType != GeoTravelType::International)
    {
      request.isInternational() = false;
    }

    bool convertRC = convert(request, &nucResults);

    if (!convertRC)
      return false;

    convertedCurrencyCode = baseFare.code();
    convertedAmount = baseFare.value();
    LOG4CXX_DEBUG(_logger, "Converted base fare amount: " << convertedAmount);
    convertedAmtNoDec = baseFare.noDec();
    nucEffectiveDate = nucResults.effectiveDate();
    nucDiscontinueDate = nucResults.discontinueDate();
    roeRate = nucResults.exchangeRate();
    roeRateNoDec = nucResults.exchangeRateNoDec();
  }
  else
    return false;

  LOG4CXX_INFO(_logger, "Leaving NUCCurrencyConverter::convertBaseFare");

  return true;
}

//------------------------------------------------------------------------------
//
//   @method determineInternationalOrigin
//
//   Description: Determines if there are any arunk segments contained
//                within an international itinerary and if so then
//                use the currency of that nation to roll nucs into the
//                local curency.
//
//   @param  FareUsage     - fare path.
//   @param  LocCode       - market where international travel originates
//
//------------------------------------------------------------------------------
bool
NUCCurrencyConverter::hasArunkSegments(const FareUsage* fareUsage, LocCode& originLoc)
{
  LOG4CXX_DEBUG(_logger, "Entered hasArunkSegments");

  const std::vector<TravelSeg*>& travelSegVector = fareUsage->travelSeg();

  for (unsigned int j = 0; j < travelSegVector.size(); j++) // lint !e574
  {
    TravelSeg* travelSeg = travelSegVector[j];

    if (dynamic_cast<ArunkSeg*>((travelSeg)))
    {
      LOG4CXX_DEBUG(_logger, "Leaving hasArunkSegments - Arunk Segment found");

      unsigned int nextTravelSegInd = ++j;

      if (nextTravelSegInd < travelSegVector.size()) // lint !e574
      {
        TravelSeg* travelSeg = travelSegVector[nextTravelSegInd];
        originLoc = travelSeg->origin()->loc();
        return true;
      }
    }
  }

  LOG4CXX_DEBUG(_logger, "Leaving hasArunkSegments - no Arunk Segments found");

  return false;
}

//----------------------------------------------------------------------------------
//
//   @method roundFare
//
//   Description: Rounds the fare amount. If the itinerary is domestic it
//   will check for domestic override to use for rounding. Otherwise it will
//   use the international rounding.
//
//   @param  Money             - target
//   @param  RoundingRule      - rounding rule
//   @param  RoundingFactor    - rounding factor
//   @param  bool              - whether or not itinerary is international
//
//   @return bool
//
//----------------------------------------------------------------------------------
bool
NUCCurrencyConverter::roundFare(Money& target,
                                RoundingRule& roundingRule,
                                RoundingFactor& roundingFactor,
                                const DateTime& ticketDate,
                                DataHandle& dataHandle,
                                bool useInternationalRounding)
{
  LOG4CXX_DEBUG(_logger, "Entered NUCCurrencyConverter::roundFare");

  if (!useInternationalRounding)
  {
    const Currency* currency = nullptr;
    currency = dataHandle.getCurrency( target.code() );

    if (LIKELY(currency))
    {
      if (currency->domRoundingFactor() > 0)
      {
        LOG4CXX_DEBUG(_logger,
                      "Domestic rounding factor is not zero for currency: " << target.code());
        roundingFactor = currency->domRoundingFactor();

        LOG4CXX_DEBUG(_logger, "Domestic NUC Rounding Factor: " << roundingFactor);
      }
      else if (isZeroAmount(currency->domRoundingFactor()))
      {
        LOG4CXX_DEBUG(_logger, "Domestic rounding factor is zero: " << target.code());
        roundingRule = NONE;
        roundingFactor = currency->domRoundingFactor();
      }
    }
  }

  LOG4CXX_DEBUG(_logger, "NUC Rounding Factor: " << roundingFactor);

  if (LIKELY((!isZeroAmount(roundingFactor)) || (roundingRule == NONE)))
  {
    bool roundRC = round(target, roundingFactor, roundingRule);

    if (UNLIKELY(!roundRC))
    {
      LOG4CXX_ERROR(_logger, "Rounding failed = NUC to local currency ");
      return false;
    }
  }

  LOG4CXX_DEBUG(_logger, "Leaving NUCCurrencyConverter::roundFare");

  return true;
}

//----------------------------------------------------------------------------------
//
//   @method checkDomesticRounding
//
//   Description: Checks to see if the base fare currency we priced with
//   is the national currency of this country. If it is not then
//   use international rounding rules.
//
//   @param  PricingTrx        - trx
//   @param  Itin              - itin
//   @param  CurrencyCode      - currencyCode
//   @param  bool              - useInternationalRounding
//
//   @return bool
//
//----------------------------------------------------------------------------------
void
NUCCurrencyConverter::checkDomesticRounding(PricingTrx& trx,
                                            const Itin* itin,
                                            const CurrencyCode& currencyCode,
                                            bool& useInternationalRounding)
{
  LOG4CXX_DEBUG(_logger, "ENTERED CHECK DOMESTIC ROUNDING");
  const std::string& controllingCtryOfCurrency =
      CurrencyUtil::getControllingNationDesc(trx, currencyCode, trx.ticketingDate());

  bool foundNationalCurrency = false;
  bool foundNation = false;
  NationCode nationWithMatchingNationalCurrency;
  NationCode controllingNationOfCurrency;

  NationCode originNation = ItinUtil::originNation(*itin);

  CurrencyUtil::getControllingNationCode(trx,
                                         controllingCtryOfCurrency,
                                         controllingNationOfCurrency,
                                         foundNation,
                                         foundNationalCurrency,
                                         nationWithMatchingNationalCurrency,
                                         trx.ticketingDate(),
                                         currencyCode);

  LOG4CXX_DEBUG(_logger, "CONTROLLING CTRY OF CURRENCY: " << controllingCtryOfCurrency);
  LOG4CXX_DEBUG(_logger, "CONTROLLING NATION OF CURRENCY: " << controllingNationOfCurrency);
  LOG4CXX_DEBUG(_logger, "FOUND NATION: " << foundNation);
  LOG4CXX_DEBUG(_logger, "FOUND NATIONAL CURRENCY: " << foundNationalCurrency);
  LOG4CXX_DEBUG(_logger, "FOUND NATION WITH MATCHING NATIONAL CURRENCY: " << foundNationalCurrency);

  if (foundNation)
  {
    if (originNation != controllingNationOfCurrency)
    {
      LOG4CXX_DEBUG(_logger, "USING INTL ROUNDING");
      useInternationalRounding = true;
    }
  }
  else if (foundNationalCurrency)
  {
    if (originNation != nationWithMatchingNationalCurrency)
    {
      LOG4CXX_DEBUG(_logger, "USING INTL ROUNDING");
      useInternationalRounding = true;
    }
  }

  LOG4CXX_DEBUG(_logger, "USING INTL ROUNDING: " << useInternationalRounding);

  LOG4CXX_DEBUG(_logger, "LEAVING CHECK FOREIGN DOMESTIC ROUNDING");
}

const DateTime&
NUCCurrencyConverter::defineConversionDate(PricingTrx& trx, const FarePath& fp)
{
  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::AF_EXC_TRX)
  {
    RexBaseTrx& rexBaseTrx = static_cast<RexBaseTrx&>(trx);
    if (rexBaseTrx.excTrxType() == PricingTrx::AR_EXC_TRX && rexBaseTrx.applyReissueExchange())
    {
      if (rexBaseTrx.isAnalyzingExcItin())
        return rexBaseTrx.getHistoricalBsrRoeDate();

      if (fp.useSecondRoeDate())
        return rexBaseTrx.newItinSecondROEConversionDate();
      return rexBaseTrx.newItinROEConversionDate();
    }
    return rexBaseTrx.originalTktIssueDT();
  }
  return trx.ticketingDate();
}

CurrencyCode
NUCCurrencyConverter::checkBaseFareCurrency(const Itin* itin, const FarePath& farePath)
{
  LOG4CXX_DEBUG(_logger, "Entered NUCCurrencyConverter::checkBaseFareCurrency");

  if (farePath.hasMultipleCurrency())
    return itin->originationCurrency();

  for (unsigned int i = 0; i < itin->fareMarket().size(); i++)
  {
    FareMarket* fareMarket = itin->fareMarket()[i];
    if (fareMarket->geoTravelType() == GeoTravelType::Transborder) // check if fm is transborder
    {
      std::vector<PaxTypeBucket>::iterator paxTypeCortegeItBegin =
          fareMarket->paxTypeCortege().begin();
      std::vector<PaxTypeBucket>::iterator paxTypeCortegeEnd = fareMarket->paxTypeCortege().end();
      for (; paxTypeCortegeItBegin != paxTypeCortegeEnd; paxTypeCortegeItBegin++)
      {
        PaxTypeBucket& paxTypeCortege = (*paxTypeCortegeItBegin);
        if (farePath.paxType() == paxTypeCortege.requestedPaxType())
        {
          if (paxTypeCortege.isMarketCurrencyPresent())
            return itin->originationCurrency();
          else
          {
            const std::vector<PricingUnit*>& pu = farePath.pricingUnit();
            CurrencyCode paxTypeFarecurr("");
            PricingUnit* priceableUnit = const_cast<PricingUnit*>(pu[0]);
            paxTypeFarecurr = priceableUnit->getFirstPaxTypeFareCurrency();
            if (paxTypeFarecurr.empty())
              return itin->originationCurrency();
            else
              return paxTypeFarecurr;
          }
        } // paxtypematch
      } // paxtypecortege
    } // transborder
  }
  return itin->originationCurrency();
}

} // tse
