#include "FareCalc/FareAmountsConverter.h"

#include "Common/BSRCollectionResults.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/Logger.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexBaseTrx.h"
#include "DBAccess/FareCalcConfig.h"
#include "Diagnostic/DiagManager.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcConsts.h"

#include <cmath>

namespace tse
{

FALLBACK_DECL(fallbackFixMslNetFareAmount);

namespace FareCalc
{

static Logger
logger("atseintl.FareCalc.FareAmountsConverter");

FareAmountsConverter::FareAmountsConverter(PricingTrx* trx,
                                           const FarePath* farePath,
                                           const FareCalcConfig* fcConfig,
                                           CalcTotals* calcTotals,
                                           CurrencyCode& lastBaseFareCurrencyCode,
                                           CurrencyNoDec& lastConvertedBaseFareNoDec)
  : _trx(trx),
    _farePath(farePath),
    _fcConfig(fcConfig),
    _calcTotals(calcTotals),
    _lastBaseFareCurrencyCode(lastBaseFareCurrencyCode),
    _lastConvertedBaseFareNoDec(lastConvertedBaseFareNoDec)
{
}

void
FareAmountsConverter::convertAmounts()
{
  if (_farePath->getTotalNUCAmount() < -EPSILON)
  {
    LOG4CXX_FATAL(logger, "Bad FarePath TotalNUCAmount " << _farePath->getTotalNUCAmount());
    throw ErrorResponseException(ErrorResponseException::BAD_FARE_PATH_TOTAL_NUC_AMOUNT);
  }

  processConvertedBaseFare();
  processConsolidatorPlusUp();
  processEquivalentFare();
  processFclNoDec();

  if (fallback::fallbackFixMslNetFareAmount(_trx) || !_farePath->isAdjustedSellingFarePath())
    convertNetChargeAmounts();

  LOG4CXX_DEBUG(logger,
                "OverrideCurr: " << _trx->getOptions()->currencyOverride() << " AgentCurr: "
                                 << _trx->getRequest()->ticketingAgent()->currencyCodeAgent()
                                 << " CalcCurr: " << _farePath->calculationCurrency()
                                 << " Converted: " << _calcTotals->convertedBaseFareCurrencyCode
                                 << " baseAmt: " << _calcTotals->convertedBaseFare
                                 << " into: " << _calcTotals->equivCurrencyCode
                                 << " equals: " << _calcTotals->equivFareAmount);
}

void
FareAmountsConverter::convertBaseFare() const
{
  convertBaseFareImpl(_farePath->getTotalNUCAmount(),
                      _farePath->calculationCurrency(),
                      _calcTotals->calcCurrencyNoDec,
                      _calcTotals->convertedBaseFare,
                      _calcTotals->convertedBaseFareNoDec,
                      _calcTotals->convertedBaseFareCurrencyCode,
                      _calcTotals->roeRate,
                      _calcTotals->roeRateNoDec,
                      _calcTotals->effectiveDate,
                      _calcTotals->discontinueDate,
                      true);
}

Money
FareAmountsConverter::convertConstructionToBaseFare(const Money& src, bool round) const
{
  CurrencyNoDec srcNoDec = 0;
  MoneyAmount convertedBaseFare;
  CurrencyNoDec convertedBaseFareNoDec;
  CurrencyCode convertedBaseFareCurrencyCode = _calcTotals->convertedBaseFareCurrencyCode;
  ExchRate roeRate;
  CurrencyNoDec roeRateNoDec = 0;
  DateTime effectiveDate;
  DateTime discontinueDate;

  convertBaseFareImpl(src.value(),
                      src.code(),
                      srcNoDec,
                      convertedBaseFare,
                      convertedBaseFareNoDec,
                      convertedBaseFareCurrencyCode,
                      roeRate,
                      roeRateNoDec,
                      effectiveDate,
                      discontinueDate,
                      round);

  return Money(convertedBaseFare, convertedBaseFareCurrencyCode);
}

void
FareAmountsConverter::convertBaseFareImpl(const MoneyAmount totalNUCAmount,
                                          const CurrencyCode& calculationCurrency,
                                          CurrencyNoDec& calcCurrencyNoDec,
                                          MoneyAmount& convertedBaseFare,
                                          CurrencyNoDec& convertedBaseFareNoDec,
                                          CurrencyCode& convertedBaseFareCurrencyCode,
                                          ExchRate& roeRate,
                                          CurrencyNoDec& roeRateNoDec,
                                          DateTime& effectiveDate,
                                          DateTime& discontinueDate,
                                          bool round) const
{
  PricingTrx& pricingTrx = *_trx;
  const FarePath& farePath = *_farePath;
  CalcTotals& totals = *_calcTotals;
  const Itin* itin = _farePath->itin();
  const DateTime& ticketingDate = pricingTrx.ticketingDate();

  if (pricingTrx.excTrxType() == PricingTrx::PORT_EXC_TRX && calculationCurrency != NUC &&
      calculationCurrency != farePath.baseFareCurrency())
  {
    BSRCurrencyConverter bsrConverter;

    DataHandle dataHandle(pricingTrx.ticketingDate());

    const Money sourceMoney(totalNUCAmount, calculationCurrency);
    Money targetMoney(farePath.baseFareCurrency());
    // Default:
    //   bool isInternational = true,
    //   ApplicationType applicationType = OTHER,
    //   bool reciprocalRate = false,
    //   PricingOptions* options = 0,
    //   bool useInternationalRounding = false,
    //   bool roundFare = true,
    //   PricingTrx* trx = 0);
    //
    CurrencyConversionRequest request(
        targetMoney,
        sourceMoney,
        pricingTrx.getRequest()->ticketingDT(),
        *(pricingTrx.getRequest()),
        dataHandle,
        false,
        (round ? CurrencyConversionRequest::OTHER : CurrencyConversionRequest::NO_ROUNDING),
        false,
        pricingTrx.getOptions(),
        false,
        true,
        &pricingTrx);

    BSRCollectionResults bsrResults;
    bsrResults.collect() = true;

    try
    {
      bool rc = bsrConverter.convert(request, &bsrResults);
      if (!rc)
      {
        LOG4CXX_FATAL(logger,
                      "BSR Rate:" << convertedBaseFareCurrencyCode
                                  << " and:" << totals.equivCurrencyCode << " was not available");
        throw ErrorResponseException(ErrorResponseException::UNABLE_TO_CALCULATE_BSR_NOT_AVAILABLE);
      }
    }
    catch (tse::ErrorResponseException& ex)
    {
      LOG4CXX_FATAL(logger, "BSR Converter exception: " << ex.what());
      throw ex;
    }
    calcCurrencyNoDec = sourceMoney.noDec(ticketingDate);

    convertedBaseFare = targetMoney.value();
    convertedBaseFareNoDec = targetMoney.noDec(ticketingDate);
    convertedBaseFareCurrencyCode = targetMoney.code();
  }
  else
  {
    NUCCurrencyConverter nucConverter;
    if (!nucConverter.convertBaseFare(pricingTrx,
                                      farePath,
                                      totalNUCAmount,
                                      convertedBaseFare,
                                      convertedBaseFareNoDec,
                                      convertedBaseFareCurrencyCode,
                                      roeRate,
                                      roeRateNoDec,
                                      effectiveDate,
                                      discontinueDate,
                                      itin->useInternationalRounding(),
                                      totals.roundBaseFare && round))
    {
      LOG4CXX_FATAL(logger, "Bad Amount " << convertedBaseFare);
      throw ErrorResponseException(ErrorResponseException::CANNOT_CALCULATE_CURRENCY);
    }
  }
}

void
FareAmountsConverter::processConvertedBaseFare()
{
  CalcTotals& totals = *_calcTotals;

  totals.convertedBaseFare = 0;
  totals.convertedBaseFareCurrencyCode = _farePath->calculationCurrency();

  convertBaseFare();

  if (LIKELY(_lastBaseFareCurrencyCode.empty()))
  {
    _lastBaseFareCurrencyCode = totals.convertedBaseFareCurrencyCode;
    _lastConvertedBaseFareNoDec = totals.convertedBaseFareNoDec;
  }
}

Money
FareAmountsConverter::convertConstructionToEquivalent(MoneyAmount amt) const
{
  Money src{std::fabs(amt), _farePath->calculationCurrency()};

  Money ret = convertConstructionToBaseFare(src, true);
  if (!_calcTotals->isCurrEquivEqCnvBase())
    ret = convertBaseFareToEquivalentImpl(ret, true).first;

  ret.value() = std::copysign(ret.value(), amt);
  return ret;
}

Money
FareAmountsConverter::convertBaseFareToEquivalent(const Money& convertedBaseFare, bool round) const
{
  return convertBaseFareToEquivalentImpl(convertedBaseFare, round).first;
}

std::pair<Money, BSRCollectionResults>
FareAmountsConverter::convertBaseFareToEquivalentImpl(const Money& convertedBaseFare, bool round)
    const
{
  // FIXME: handle round
  PricingTrx& pricingTrx = *_trx;
  MoneyAmount equivFareAmount = 0; // output

  CurrencyCode equivCurrencyCode = TrxUtil::getEquivCurrencyCode(pricingTrx); // output

  DateTime convertDT = defineConversionDate(pricingTrx, *_farePath);

  BSRCurrencyConverter bsrConverter;
  bool rc;
  DataHandle dataHandle(convertDT);

  const Money sourceMoney(convertedBaseFare);
  Money targetMoney(equivCurrencyCode);
  CurrencyConversionRequest request(
      targetMoney,
      sourceMoney,
      convertDT,
      *(pricingTrx.getRequest()),
      dataHandle,
      false,
      (round ? CurrencyConversionRequest::OTHER : CurrencyConversionRequest::NO_ROUNDING),
      false,
      pricingTrx.getOptions(),
      false,
      true,
      &pricingTrx);

  BSRCollectionResults bsrResults;
  bsrResults.collect() = true;

  try
  {
    rc = bsrConverter.convert(request, &bsrResults);
    if (!rc)
    {
      LOG4CXX_FATAL(logger,
                    "BSR Rate:" << _calcTotals->convertedBaseFareCurrencyCode
                                << " and:" << equivCurrencyCode << " was not available");
      throw ErrorResponseException(ErrorResponseException::UNABLE_TO_CALCULATE_BSR_NOT_AVAILABLE);
    }
  }
  catch (tse::ErrorResponseException& ex)
  {
    LOG4CXX_FATAL(logger, "BSR Converter exception: " << ex.what());
    throw ex;
  }

  // Reset equivCurrencyCode if no equivalent amount conversion was made
  // so equivalent amount is not displayed
  //
  if (!(bsrResults.equivCurrencyCode().empty()))
  {
    if (bsrResults.equivCurrencyCode() != equivCurrencyCode)
    {
      LOG4CXX_DEBUG(
          logger,
          "NO EQUIV AMT CONVERSION - SET EQUIV CUR TO: " << bsrResults.equivCurrencyCode());
      equivCurrencyCode = bsrResults.equivCurrencyCode();
    }
  }

  equivFareAmount = targetMoney.value();

  return std::make_pair(Money(equivFareAmount, equivCurrencyCode), bsrResults);
}

void
FareAmountsConverter::processEquivalentFare()
{
  CalcTotals& totals = *_calcTotals;

  std::pair<Money, BSRCollectionResults> conversionResult = convertBaseFareToEquivalentImpl(
      Money(totals.convertedBaseFare, totals.convertedBaseFareCurrencyCode), true);
  Money& targetMoney = conversionResult.first;
  BSRCollectionResults& bsrResults = conversionResult.second;

  totals.equivFareAmount = targetMoney.value();
  totals.equivCurrencyCode = targetMoney.code();
  totals.equivNoDec = targetMoney.noDec(_trx->ticketingDate());

  if (_trx->hasPriceDynamicallyDeviated())
  {
    Money base{totals.isCurrEquivEqCnvBase() ? totals.convertedBaseFare : totals.equivFareAmount,
               totals.equivCurrencyCode};
    totals.effectiveDeviation =
        base - convertConstructionToEquivalent(_farePath->getUndeviatedTotalNUCAmount());
  }

  if (TrxUtil::isIcerActivated(*_trx))
  {
    totals.bsrRate1NoDec = bsrResults.exchangeRate1NoDec();
    totals.bsrRate1 = bsrResults.exchangeRate1();
    return;
  }

  int64_t rateType = (int64_t)(1 / bsrResults.exchangeRate1()); // drop decimal
  totals.bsrRate1NoDec = bsrResults.exchangeRate1NoDec();
  totals.bsrRate2NoDec = bsrResults.exchangeRate2NoDec();
  totals.interCurrencyCode = bsrResults.intermediateCurrency();
  totals.intermediateAmount = bsrResults.intermediateAmount();

  if (totals.bsrRate1NoDec <= 2)
    totals.bsrRate1NoDec = 9;
  if (totals.convertedBaseFare >
      (totals.interCurrencyCode.empty() ? totals.equivFareAmount : totals.intermediateAmount))
  {
    if (rateType > 0)
      totals.bsrRate1 = bsrResults.exchangeRate1();
    else
      totals.bsrRate1 = (1 / bsrResults.exchangeRate1());
  }
  else
  {
    if (rateType > 0)
      totals.bsrRate1 = (1 / bsrResults.exchangeRate1());
    else
      totals.bsrRate1 = bsrResults.exchangeRate1();
  }

  rateType = (int64_t)(1 / bsrResults.exchangeRate2()); // drop decimal

  if (bsrResults.intermediateAmount() > totals.equivFareAmount)
  {
    if (rateType > 0)
      totals.bsrRate2 = bsrResults.exchangeRate2();
    else
      totals.bsrRate2 = (1 / bsrResults.exchangeRate2());
  }
  else
  {
    if (rateType > 0)
      totals.bsrRate2 = (1 / bsrResults.exchangeRate2());
    else
      totals.bsrRate2 = bsrResults.exchangeRate2();
  }
}

void
FareAmountsConverter::processFclNoDec()
{
  // NoDec (tempDec) setting used to display various amount on FCL line:
  _calcTotals->fclNoDec = _calcTotals->convertedBaseFareNoDec;
  if (_fcConfig->domesticNUC() == FareCalcConsts::FC_YES ||
      _farePath->calculationCurrency() != _farePath->itin()->originationCurrency())
  {
    _calcTotals->fclNoDec = 2; // Displayed as NUC amount with 2 dec digits
    _calcTotals->useNUC = true;
  }

  if (_trx->excTrxType() == PricingTrx::PORT_EXC_TRX && _farePath->calculationCurrency() != NUC &&
      _farePath->calculationCurrency() != _farePath->baseFareCurrency())
  {
    _calcTotals->fclNoDec = _calcTotals->calcCurrencyNoDec;
    _calcTotals->useNUC = false;
  }
}

const DateTime&
FareAmountsConverter::defineConversionDate(PricingTrx& trx, const FarePath& fp) const
{
  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    RexBaseTrx& rexBaseTrx = static_cast<RexBaseTrx&>(trx);
    if (rexBaseTrx.applyReissueExchange())
    {
      if (fp.useSecondRoeDate())
        return rexBaseTrx.newItinSecondROEConversionDate();
      return rexBaseTrx.newItinROEConversionDate();
    }
  }
  return trx.ticketingDate();
}

void
FareAmountsConverter::convertNetChargeAmounts()
{
  // Cat35 Net Remit Ticketing
  // Cats 8,9,12 Surcharges  CR # 1

  if (!_farePath->collectedNegFareData())
  {
    return;
  }

  PricingTrx& pricingTrx = *_trx;
  const FarePath& farePath = *_farePath;
  CalcTotals& totals = *_calcTotals;

  NUCCurrencyConverter nucConv;

  // Cat35 Net Remit Ticketing
  // Cats 8,9,12 Surcharges  CR # 1
  if (LIKELY(farePath.collectedNegFareData()))
  {
    CollectedNegFareData* cNegFareData =
        const_cast<CollectedNegFareData*>(farePath.collectedNegFareData());

    MoneyAmount netTotalAmt = cNegFareData->netTotalAmtCharges();
    netTotalAmt += cNegFareData->totalMileageCharges();
    netTotalAmt += totals.totalNetCharges;
    cNegFareData->netTotalAmtCharges() = netTotalAmt;

    // Convert total Net + charges from NUC to COC - base fare currency
    //
    FarePath fP = farePath; // dummy Fare Path just for conversion
    fP.setTotalNUCAmount(netTotalAmt);

    MoneyAmount convertedAmount(0);
    CurrencyNoDec convertedAmtNoDec(0);
    CurrencyCode convertedCurrencyCode;
    ExchRate exchRate;
    CurrencyNoDec exchNoDec;
    DateTime nucEffectiveDate;
    DateTime nucDiscontinueDate;

    if (!nucConv.convertBaseFare(pricingTrx,
                                 fP,
                                 fP.getTotalNUCAmount(),
                                 convertedAmount,
                                 convertedAmtNoDec,
                                 convertedCurrencyCode,
                                 exchRate,
                                 exchNoDec,
                                 nucEffectiveDate,
                                 nucDiscontinueDate,
                                 fP.itin()->useInternationalRounding()))
    {
      LOG4CXX_ERROR(logger, "Currency conversion error");
      return;
    }

    // Working with BSR
    MoneyAmount equivFareAmount = 0;

    if ((!totals.equivCurrencyCode.empty()) && (totals.equivCurrencyCode != convertedCurrencyCode))
    {
      BSRCurrencyConverter bsrConverter;
      bool rc;
      DataHandle dataHandle(pricingTrx.ticketingDate());

      const Money sourceMoney(convertedAmount, convertedCurrencyCode);
      Money targetMoney(equivFareAmount, totals.equivCurrencyCode);
      CurrencyConversionRequest request1(targetMoney,
                                         sourceMoney,
                                         pricingTrx.getRequest()->ticketingDT(),
                                         *(pricingTrx.getRequest()),
                                         dataHandle,
                                         false,
                                         CurrencyConversionRequest::OTHER,
                                         false,
                                         pricingTrx.getOptions(),
                                         false,
                                         true,
                                         &pricingTrx);
      BSRCollectionResults bsrResults;
      bsrResults.collect() = true;

      rc = bsrConverter.convert(request1, &bsrResults);
      if (!rc)
      {
        LOG4CXX_FATAL(logger,
                      "BSR Rate:" << convertedCurrencyCode << " and:" << totals.equivCurrencyCode
                                  << " was not available");
        return;
      }

      equivFareAmount = targetMoney.value();
      cNegFareData->netTotalAmtCharges() = equivFareAmount;
      cNegFareData->setNetTotalBaseAmtCharges(convertedAmount);
    }
    else
    {
      cNegFareData->netTotalAmtCharges() = convertedAmount;
      cNegFareData->setNetTotalBaseAmtCharges(convertedAmount);
    }
  }
}

void
FareAmountsConverter::processConsolidatorPlusUp()
{
  if (_farePath->itin()->isPlusUpPricing())
  {
    DiagManager diagManager(*_trx, Diagnostic864);
    DiagCollector* diag = diagManager.isActive() ? &diagManager.collector() : nullptr;

    Itin itn = *_farePath->itin();
    itn.consolidatorPlusUp()->addPlusUpToBaseFare(*_trx,
                                                  *_farePath,
                                                  _calcTotals->convertedBaseFareCurrencyCode,
                                                  _calcTotals->convertedBaseFare,
                                                  diag);

    if (diag)
      diag->flushMsg();
  }
}

Money
FareAmountsConverter::roundUp(Money money) const
{
  BSRCurrencyConverter bsr;
  bsr.applyRoundingRule(money, _trx->ticketingDate(), UP);
  return money;
}

} // namespace FareCalc
} // namespace tse
