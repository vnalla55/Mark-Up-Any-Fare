#include "Common/CurrencyConversionFacade.h"

#include "Common/BSRCollectionResults.h"
#include "Common/CurrencyUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/ItinUtil.h"
#include "Common/Money.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TSSCacheCommon.h"
#include "DataModel/Agent.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexBaseTrx.h"
#include "DBAccess/Currency.h"
#include "DBAccess/Loc.h"


namespace tse
{

static Logger logger("atseintl.Common.CurrencyConversionFacade");

namespace
{
inline bool
shouldRoundFare(const PricingTrx& trx, const Money& target, bool useIntRounding, bool roundFare)
{
  return target.isNuc() ?
         !(trx.excTrxType() == PricingTrx::AR_EXC_TRX && !useIntRounding) : roundFare;
}

} // namespace

bool
CurrencyConversionFacade::convertCalc(Money& target,
                                      const Money& source,
                                      PricingTrx& trx,
                                      bool useInternationalRounding,
                                      ConversionType applType,
                                      bool reciprocalRate,
                                      CurrencyCollectionResults* results,
                                      CurrencyConversionCache* cache,
                                      bool roundFare)

{
  LOG4CXX_INFO(logger, "Entered CurrencyConversionFacade::convertCalc()");

  if ((source.value() >= 0.0) && (source.value() < EPSILON))
  {
    LOG4CXX_INFO(logger, "   Source value contains zero amount");
    target.value() = 0.0;
    return true;
  }

  if (UNLIKELY(target.code() == source.code()))
  {
    target.value() = source.value();
    return true;
  }

  if (LIKELY((target.isNuc()) || (source.isNuc())))
  {
    LOG4CXX_INFO(logger, "   Performing NUC conversion");

    if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::AF_EXC_TRX))
    {
      const DateTime& calcDate = defineConversionDate(trx);
      DataHandle dataHandle(calcDate);
      CurrencyConversionRequest request(target,
                                        source,
                                        calcDate,
                                        *(trx.getRequest()),
                                        dataHandle,
                                        true,
                                        applType,
                                        reciprocalRate,
                                        trx.getOptions(),
                                        useInternationalRounding,
                                        shouldRoundFare(trx, target,
                                                        useInternationalRounding, roundFare));

      return nucConverter().convert(request, nullptr);
    }

    CurrencyConversionRequest request(target,
                                      source,
                                      trx.getRequest()->ticketingDT(),
                                      *(trx.getRequest()),
                                      trx.dataHandle(),
                                      true,
                                      applType,
                                      reciprocalRate,
                                      trx.getOptions(),
                                      useInternationalRounding,
                                      roundFare);

    return nucConverter().convert(request, nullptr, cache);
  }

  LOG4CXX_INFO(logger, "   Performing double NUC conversion");
  Money tempNuc(NUC);

  CurrencyConversionRequest request(tempNuc,
                                    source,
                                    trx.getRequest()->ticketingDT(),
                                    *(trx.getRequest()),
                                    trx.dataHandle(),
                                    true,
                                    applType,
                                    reciprocalRate,
                                    trx.getOptions(),
                                    useInternationalRounding);

  if (!nucConverter().convert(request, nullptr, cache))
    return false;

  CurrencyConversionRequest request2(target,
                                     tempNuc,
                                     trx.getRequest()->ticketingDT(),
                                     *(trx.getRequest()),
                                     trx.dataHandle(),
                                     true,
                                     applType,
                                     reciprocalRate,
                                     trx.getOptions(),
                                     useInternationalRounding);

  return nucConverter().convert(request2, nullptr, cache);
}

bool
CurrencyConversionFacade::convert(Money& target,
                                  const Money& source,
                                  const PricingTrx& trx,
                                  bool useInternationalRounding,
                                  CurrencyConversionRequest::ApplicationType applType,
                                  bool reciprocalRate,
                                  CurrencyCollectionResults* results)

{
  LOG4CXX_INFO(logger, "Entered CurrencyConversionFacade::convert()");

  if ((source.value() >= 0.0) && (source.value() < EPSILON))
  {
    LOG4CXX_INFO(logger, "Source value contains zero amount");
    target.value() = 0.0;
    return true;
  }

  const DateTime& calcDate =
      (trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::AF_EXC_TRX)
          ? static_cast<const RexBaseTrx&>(trx).fareApplicationDT()
          : trx.getRequest()->ticketingDT();

  DataHandle dataHandle(calcDate);

  if ((target.isNuc()) || (source.isNuc()))
  {
    LOG4CXX_INFO(logger, "Performing NUC conversion");

    CurrencyConversionRequest request(target,
                                      source,
                                      calcDate,
                                      *(trx.getRequest()),
                                      trx.dataHandle(),
                                      true,
                                      applType,
                                      reciprocalRate,
                                      trx.getOptions(),
                                      useInternationalRounding,
                                      shouldRoundFare(trx, target,
                                                      useInternationalRounding, _roundFare));

    if (LIKELY( nucConverter().convert(request, results) ))
      return true;
  }
  else
  {
    CurrencyConversionRequest request(target,
                                      source,
                                      calcDate,
                                      *(trx.getRequest()),
                                      dataHandle,
                                      true,
                                      applType,
                                      reciprocalRate,
                                      trx.getOptions(),
                                      useInternationalRounding,
                                      true,
                                      &trx);

    if (request.applicationType() == CurrencyConversionRequest::DC_CONVERT)
    {
      LOG4CXX_DEBUG(logger, "Conversion request type is: DC CONVERT");

      CurrencyCode pricingCurrency;
      NationCode nation = trx.getRequest()->ticketingAgent()->agentLocation()->nation();
      CurrencyUtil::getNationCurrency(nation, pricingCurrency, trx.getRequest()->ticketingDT());

      request.salesLocCurrency() = pricingCurrency;

      CurrencyCode conversionCurrency;
      CurrencyUtil::getConversionCurrency(
          nation, conversionCurrency, trx.getRequest()->ticketingDT());

      request.conversionCurrency() = conversionCurrency;

      if ((pricingCurrency == target.code()) || (pricingCurrency == source.code()))
      {
        LOG4CXX_DEBUG(logger, "Common sales location established");
        request.commonSalesLocCurrency() = true;
      }
    }

    BSRCollectionResults* bsrResults = dynamic_cast<BSRCollectionResults*>(results);
    if (bsrResults)
      bsrResults->collect() = true;

    try
    {
      if (LIKELY(_bsrConverter.convert(request, bsrResults)))
      {
        LOG4CXX_DEBUG(logger, "Converted amount for currency " << target.code()
                                                               << ", " << target.value());
        return true;
      }
    }
    catch (ErrorResponseException& ex)
    {
      LOG4CXX_DEBUG(logger, "BSRCurrencyConverter exception: " << ex.what());
      throw ex;
    }

    LOG4CXX_DEBUG(logger, "BSR conversion failed for : " << target.code());
  }

  LOG4CXX_INFO(logger, "Leaving CurrencyConversionFacade::convert()");

  return false;
}

bool
CurrencyConversionFacade::convertWithCurrentDate(Money& target,
                                                 const Money& source,
                                                 PricingTrx& trx)
{
  if (source.value() >= 0.0 && source.value() < EPSILON)
  {
    LOG4CXX_INFO(logger, "Source value contains zero amount");
    target.value() = 0.0;
    return true;
  }

  CurrencyConversionRequest request(target,
                                    source,
                                    DateTime::localTime(),
                                    *(trx.getRequest()),
                                    trx.dataHandle(),
                                    true,
                                    CurrencyConversionRequest::FARES,
                                    false,
                                    trx.getOptions(),
                                    false,
                                    true);
  if (target.isNuc() || source.isNuc())
  {
    LOG4CXX_INFO(logger, "Performing NUC conversion");

    if (_nucConverter.convert(request, nullptr))
      return true;
  }
  else
  {
    LOG4CXX_INFO(logger, "Performing BSR conversion");

    try
    {
      if (_bsrConverter.convert(request, nullptr))
        return true;
    }
    catch (ErrorResponseException& error)
    {
      LOG4CXX_DEBUG(logger, "BSRCurrencyConverter exception: " << error.what());
      throw error;
    }
    LOG4CXX_DEBUG(logger, "BSR conversion failed for : " << target.code());
  }
  return false;
}

bool
CurrencyConversionFacade::convert(Money& target,
                                  const Money& localCurrency,
                                  const PricingTrx& trx,
                                  const CurrencyCode& calculationCurrency,
                                  MoneyAmount& convertedAmount,
                                  bool useInternationalRounding,
                                  CurrencyConversionRequest::ApplicationType applType,
                                  bool reciprocalRate)

{
  LOG4CXX_INFO(logger, "Entered CurrencyConversionFacade::convert()");

  if (localCurrency.code() == calculationCurrency)
  {
    target.value() = localCurrency.value();
    convertedAmount = localCurrency.value();
    return true;
  }

  bool nucRC1 = false;

  const BaseExchangeTrx* rexBaseTrx = nullptr;

  if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::AF_EXC_TRX))
    rexBaseTrx = static_cast<const BaseExchangeTrx*>(&trx);

  if (UNLIKELY(rexBaseTrx))
  {
    const DateTime& calcDate = defineConversionDate(trx);
    DataHandle dataHandle(calcDate);

    CurrencyConversionRequest request1(target,
                                       localCurrency,
                                       calcDate,
                                       *(trx.getRequest()),
                                       dataHandle,
                                       true,
                                       applType,
                                       reciprocalRate,
                                       trx.getOptions(),
                                       useInternationalRounding,
                                       shouldRoundFare(trx, target,
                                                       useInternationalRounding, true));
    nucRC1 = nucConverter().convert(request1, nullptr);
  }
  else
  {
    CurrencyConversionRequest request1(target,
                                       localCurrency,
                                       trx.getRequest()->ticketingDT(),
                                       *(trx.getRequest()),
                                       trx.dataHandle(),
                                       true,
                                       applType,
                                       reciprocalRate,
                                       trx.getOptions(),
                                       useInternationalRounding);

    nucRC1 = nucConverter().convert(request1, nullptr);
  }

  if (UNLIKELY(!nucRC1))
    return false;

  if (UNLIKELY((calculationCurrency != localCurrency.code()) && (calculationCurrency != NUC)))
  {
    Money calcCurrency(calculationCurrency);

    bool nucRC2 = false;

    if (rexBaseTrx)
    {
      const DateTime& calcDate = defineConversionDate(trx);
      DataHandle dataHandle(calcDate);
      CurrencyConversionRequest request2(calcCurrency,
                                         target,
                                         calcDate,
                                         *(trx.getRequest()),
                                         dataHandle,
                                         true,
                                         applType,
                                         reciprocalRate,
                                         trx.getOptions(),
                                         useInternationalRounding);

      nucRC2 = nucConverter().convert(request2, nullptr);
    }
    else
    {
      CurrencyConversionRequest request2(calcCurrency,
                                         target,
                                         trx.getRequest()->ticketingDT(),
                                         *(trx.getRequest()),
                                         trx.dataHandle(),
                                         true,
                                         applType,
                                         reciprocalRate,
                                         trx.getOptions(),
                                         useInternationalRounding);

      nucRC2 = nucConverter().convert(request2, nullptr);
    }

    if (!nucRC2)
      return false;

    convertedAmount = calcCurrency.value();
  }
  else
    convertedAmount = target.value();

  LOG4CXX_INFO(logger, "Leaving CurrencyConversionFacade::convert()");

  return true;
}

bool
CurrencyConversionFacade::round(Money& target,
                                PricingTrx& trx,
                                bool useInternationalRounding)
{
  return tsscache::ccfRound(*this, target, trx, useInternationalRounding, trx.getBaseIntId());
}

bool
CurrencyConversionFacade::roundCB(Money& target,
                                  PricingTrx& trx,
                                  bool useInternationalRounding)
{
  bool roundRC = false;
  double nucFactor = 0.0;
  double roundingFactor = 0.0;
  RoundingRule roundingRule;
  CarrierCode carrier;
  CurrencyNoDec roundingFactorNoDec;
  CurrencyNoDec nucFactorNoDec;
  DateTime discontinueDate(pos_infin);
  DateTime effectiveDate(pos_infin);

  if (target.code() == NUC)
  {
    CurrencyUtil::truncateNUCAmount(target.value());
    return true;
  }

  CurrencyConverter ccConverter;

  bool nucRC = ccConverter.getNucInfo(
      carrier,
      target.code(),
      (trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::AF_EXC_TRX)
      ? defineConversionDate(trx)
      : trx.getRequest()->ticketingDT(),
      nucFactor,
      roundingFactor,
      roundingRule,
      roundingFactorNoDec,
      nucFactorNoDec,
      discontinueDate,
      effectiveDate);

  if (UNLIKELY(!nucRC))
  {
    LOG4CXX_ERROR(
        logger, "No NUC information retrieved from DBAccess layer for Currency: " << target.code());
    LOG4CXX_ERROR(logger, "Ticketing Date : " << trx.getRequest()->ticketingDT().toSimpleString());
    return false;
  }

  LOG4CXX_DEBUG(logger, "Rounding Rule: " << roundingRule);
  LOG4CXX_DEBUG(logger, "International NUC Rounding Factor: " << roundingFactor);

  if ( !useInternationalRounding )
  {
    const Currency* currency = trx.dataHandle().getCurrency( target.code() );
    if ( currency && ( currency->domRoundingFactor() > 0 ) )
    {
      roundingFactor = currency->domRoundingFactor();
      LOG4CXX_DEBUG(logger, "Domestic Currency Rounding Factor: " << roundingFactor);
    }
    else if ( currency && ( currency->domRoundingFactor() == NUCCurrencyConverter::NO_ROUNDING ) )
    {
      roundingRule = NONE;
      roundingFactor = currency->domRoundingFactor();
      LOG4CXX_DEBUG(logger, "Domestic Currency Rounding Factor: " << roundingFactor);
    }
  }

  roundRC = ccConverter.round(target, roundingFactor, roundingRule);

  return roundRC;
}

const DateTime&
CurrencyConversionFacade::defineConversionDate(const PricingTrx& trx) const
{
  const RexBaseTrx& rexBaseTrx = static_cast<const RexBaseTrx&>(trx);
  if (rexBaseTrx.excTrxType() == PricingTrx::AR_EXC_TRX && rexBaseTrx.applyReissueExchange())
  {
    if (rexBaseTrx.isAnalyzingExcItin())
      return rexBaseTrx.getHistoricalBsrRoeDate();

    if (rexBaseTrx.trxPhase() != RexBaseTrx::IDLE_PHASE)
    {
      if (!rexBaseTrx.newItinSecondROEConversionDate().isEmptyDate() &&
          (_useSecondRoeDate || rexBaseTrx.useSecondROEConversionDate()))
      {
        return rexBaseTrx.newItinSecondROEConversionDate();
      }

      return rexBaseTrx.newItinROEConversionDate();
    }
  }

  return rexBaseTrx.originalTktIssueDT();
}

} //tse
