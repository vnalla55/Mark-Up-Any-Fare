//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/OBFeesUtils.h"

#include "Common/Assert.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConverter.h"
#include "Common/CurrencyUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/ObFeeDescriptors.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/AltPricingDetailObFeesTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DBAccess/Currency.h"
#include "DBAccess/TaxNation.h"
#include "FareCalc/CalcTotals.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/XformUtil.h"

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp);

namespace
{
Logger
logger("atseintl.Common.OBFeesUtils");

struct NonZeroAmount
{
  bool operator()(const TicketingFeesInfo* feeInfo) const
  {
    if (feeInfo->feeAmount() > EPSILON || feeInfo->feePercent() > EPSILON)
      return true;
    return false;
  }
};

bool
isValidItin(const Itin* itin)
{
  return (itin != nullptr && itin->errResponseCode() == ErrorResponseException::NO_ERROR &&
          !itin->farePath().empty());
}
}

namespace OBFeesUtils
{
int
getLastIdx(std::string response)
{
  size_t pos = response.rfind("Q0K=\"");
  if (pos != std::string::npos)
    return std::stoi(response.substr(pos + 5, 6));
  return 0;
}

std::string
convertIdxToString(const int idx, const int width, const char fill)
{
  std::stringstream ss;
  ss << std::setw(width) << std::setfill(fill) << idx;
  return ss.str();
}

std::string
formatBinNumber(const FopBinNumber& fopBinNumber)
{
  const size_t end = fopBinNumber.find('*');
  return fopBinNumber.substr(0, end);
}

std::string
prepareHeaderObFeeMsg()
{
  return "FORM OF PAYMENT FEES PER TICKET MAY APPLY";
}

std::string
prepareColumnsNamesObFeeMsg(const PaxTypeCode& paxTypeCode)
{
  std::ostringstream msgText;
  msgText << std::left;
  msgText << std::setw(9) << paxTypeCode;
  msgText << std::setw(25) << "DESCRIPTION";
  msgText << std::right;
  msgText << std::setw(10) << "FEE";
  msgText << std::setw(15) << "TKT TOTAL";

  return msgText.str();
}

std::string
wrapObFeeMsgLine(const std::string& line, int msgIdx)
{
  XMLConstruct construct;
  construct.openElement(xml2::MessageInformation);
  construct.addAttributeChar(xml2::MessageType, Message::TYPE_GENERAL);
  construct.addAttribute(xml2::MessageFailCode, convertIdxToString(msgIdx));
  construct.addAttribute(xml2::MessageText, line);
  construct.closeElement();

  return construct.getXMLData();
}

std::string
prepareTotalFeeAmout(const MoneyAmount& totalFeeAmount, const CurrencyNoDec& noDec)
{
  std::ostringstream msgText;

  if (totalFeeAmount > 0.0)
    msgText << std::setw(15) << std::setprecision(noDec) << std::fixed << totalFeeAmount;

  return msgText.str();
}

std::string
prepareObFeeMsg2CC(TicketingFeesInfo* feeInfo,
                   const MoneyAmount& feeAmount,
                   const CurrencyNoDec& noDec)
{
  std::ostringstream msgText;

  if (feeInfo)
  {
    msgText << std::left << " " << std::setw(5)
            << feeInfo->serviceTypeCode() + feeInfo->serviceSubTypeCode() << " - ";
    msgText << std::left << std::setw(25);

    if (feeInfo->fopBinNumber().empty())
      msgText << "ANY CC";
    else
      msgText << "CC NBR BEGINS WITH " + formatBinNumber(feeInfo->fopBinNumber());

    msgText << std::right << std::fixed << std::setw(10);
    msgText << std::setprecision(noDec) << feeAmount;
  }

  return msgText.str();
}

std::string
prepareObFeeMsg(PricingTrx& pricingTrx,
                const CalcTotals& calcTotals,
                TicketingFeesInfo* feeInfo,
                bool limitMaxOBFees,
                MoneyAmount feeAmt,
                MoneyAmount fareAmtWith2CCFee)
{
  std::ostringstream msgText;

  msgText << std::left << " " << std::setw(5)
          << feeInfo->serviceTypeCode() + feeInfo->serviceSubTypeCode() << " - ";
  msgText << std::left << std::setw(25);

  if (feeInfo->fopBinNumber().empty())
    msgText << "ANY CC";
  else
    msgText << "CC NBR BEGINS WITH " + formatBinNumber(feeInfo->fopBinNumber());

  MoneyAmount totalFeeFareAmount(0.0);
  CurrencyNoDec numDec = 0;

  msgText << std::right << std::fixed;

  if (Money::isZeroAmount(fareAmtWith2CCFee) && !pricingTrx.isProcess2CC())
  {
    calculateObFeeAmount(
        pricingTrx, calcTotals, feeInfo, totalFeeFareAmount, numDec, limitMaxOBFees);
    msgText << std::setw(10) << std::setprecision(numDec) << totalFeeFareAmount;
  }

  OBFeeSubType subType = TrxUtil::getOBFeeSubType(feeInfo->serviceSubTypeCode());
  if (subType == OBFeeSubType::OB_F_TYPE && !limitMaxOBFees)
  {
    Money moneyEquiv(calcTotals.equivCurrencyCode);
    totalFeeFareAmount += calcTotals.getTotalAmountPerPax();

    msgText.precision(moneyEquiv.noDec(pricingTrx.ticketingDate()));

    if (Money::isZeroAmount(fareAmtWith2CCFee) && !pricingTrx.isProcess2CC())
      msgText << std::setw(15) << std::setprecision(numDec) << totalFeeFareAmount;
    else
      msgText << std::setw(10) << feeAmt << std::setw(15) << fareAmtWith2CCFee;
  }
  return msgText.str();
}

void
calculateObFeeAmount(PricingTrx& pricingTrx,
                     const CalcTotals& calcTotals,
                     const TicketingFeesInfo* feeInfo,
                     MoneyAmount& totalFeeFareAmount,
                     CurrencyNoDec& noDec,
                     bool limitMaxOBFees,
                     const MoneyAmount& chargeAmount)
{
  if (feeInfo->feePercent() > 0)
    calculateObFeeAmountFromPercentage(
        pricingTrx, calcTotals, feeInfo, totalFeeFareAmount, noDec, limitMaxOBFees, chargeAmount);
  else
    calculateObFeeAmountFromAmount(pricingTrx, calcTotals, feeInfo, totalFeeFareAmount, noDec);
}

void
calculateObFeeAmountFromAmount(PricingTrx& pricingTrx,
                               const CalcTotals& calcTotals,
                               const TicketingFeesInfo* feeInfo,
                               MoneyAmount& totalFeeFareAmount,
                               CurrencyNoDec& noDec)
{
  if (feeInfo->feeAmount() < 0.0 ||
      (!calcTotals.equivCurrencyCode.empty() && feeInfo->cur() == calcTotals.equivCurrencyCode) ||
      (calcTotals.equivCurrencyCode.empty() &&
       feeInfo->cur() == calcTotals.convertedBaseFareCurrencyCode))
  {
    totalFeeFareAmount = feeInfo->feeAmount();
    noDec = feeInfo->noDec();
  }
  else
  {
    const DateTime& ticketingDate = pricingTrx.ticketingDate();
    Money targetMoney = convertOBFeeCurrency(pricingTrx, calcTotals, feeInfo);
    totalFeeFareAmount = targetMoney.value();
    noDec = targetMoney.noDec(ticketingDate);
  }
}

void
calculateObFeeAmountFromPercentage(PricingTrx& pricingTrx,
                                   const CalcTotals& calcTotals,
                                   const TicketingFeesInfo* feeInfo,
                                   MoneyAmount& totalFeeFareAmount,
                                   CurrencyNoDec& noDec,
                                   bool limitMaxOBFees,
                                   const MoneyAmount& chargeAmount)
{
  if (limitMaxOBFees) // calculation+conversion was done by Max
  {
    totalFeeFareAmount = feeInfo->feeAmount();
    noDec = feeInfo->noDec();
  }
  else
  {
    Money targetMoney(NUC);
    calculateObFeeAmountFromPercentage(
        pricingTrx, calcTotals, feeInfo, targetMoney, totalFeeFareAmount, chargeAmount);

    noDec = targetMoney.noDec(pricingTrx.ticketingDate());
  }
}

void
calculateObFeeAmountFromPercentage(PricingTrx& pricingTrx,
                                   const CalcTotals& calcTotals,
                                   const TicketingFeesInfo* feeInfo,
                                   Money& targetMoney,
                                   MoneyAmount& totalFeeFareAmount,
                                   const MoneyAmount& chargeAmount)
{
  MoneyAmount calcAmount;
  if (!Money::isZeroAmount(chargeAmount))
    calcAmount = (chargeAmount * feeInfo->feePercent()) / 100.0f;
  else
    calcAmount =
        calculateResidualObFeeAmount(pricingTrx, calcTotals.getTotalAmountPerPax(), feeInfo);

  const CurrencyCode& paymentCurrency = calcTotals.equivCurrencyCode.empty()
                                            ? calcTotals.convertedBaseFareCurrencyCode
                                            : calcTotals.equivCurrencyCode;

  Money targetMoneyC(calcAmount, paymentCurrency);
  roundOBFeeCurrency(pricingTrx, targetMoneyC);
  calcAmount = targetMoneyC.value();

  MoneyAmount maxAmount = feeInfo->maxFeeAmount();
  const Money sourceMoney(maxAmount, feeInfo->maxFeeCur());
  targetMoney.setCode(paymentCurrency);

  convertOBFeeCurrency(pricingTrx, sourceMoney, targetMoney); //+rounding
  maxAmount = targetMoney.value();

  MoneyAmount lowestCalcAmount = getLowestObFeeAmount(feeInfo->maxFeeCur(), calcAmount, maxAmount);
  totalFeeFareAmount = lowestCalcAmount;
}

MoneyAmount
calculateResidualObFeeAmount(PricingTrx& pricingTrx,
                             const MoneyAmount& totalPaxAmount,
                             const TicketingFeesInfo* feeInfo)
{
  bool residual = pricingTrx.getRequest()->chargeResidualInd();
  MoneyAmount amountFop = pricingTrx.getRequest()->paymentAmountFop();
  MoneyAmount calcAmt = 0.0;

  if (residual && amountFop != 0)
    calcAmt = (totalPaxAmount - amountFop);
  else if (!residual)
    calcAmt = amountFop;
  else
    calcAmt = totalPaxAmount;

  if (calcAmt <= 0.0)
    calcAmt = 0.0;
  MoneyAmount calcAmount = (calcAmt * feeInfo->feePercent()) / 100.0f;
  return calcAmount;
}

MoneyAmount
getLowestObFeeAmount(const CurrencyCode& maxFeeCur,
                     const MoneyAmount& calcAmount,
                     const MoneyAmount& maxAmount)
{
  if (maxFeeCur.empty())
    return calcAmount;
  if (calcAmount > maxAmount)
    return maxAmount;
  return calcAmount;
}

Money
convertOBFeeCurrency(PricingTrx& pricingTrx,
                     const CalcTotals& calcTotals,
                     const TicketingFeesInfo* feeInfo)
{
  const Money sourceMoney(feeInfo->feeAmount(), feeInfo->cur());
  Money targetMoney(calcTotals.equivCurrencyCode);

  if (calcTotals.equivCurrencyCode.empty() &&
      feeInfo->cur() != calcTotals.convertedBaseFareCurrencyCode)
  {
    targetMoney.setCode(calcTotals.convertedBaseFareCurrencyCode);
  }

  convertOBFeeCurrency(pricingTrx, sourceMoney, targetMoney);

  return targetMoney;
}

void
convertOBFeeCurrency(PricingTrx& pricingTrx, const Money& sourceMoney, Money& targetMoney)
{
  CurrencyConversionFacade converter;
  converter.convert(targetMoney, sourceMoney, pricingTrx, false, CurrencyConversionRequest::TAXES);
  roundOBFeeCurrency(pricingTrx, targetMoney);
}

void
roundOBFeeCurrency(PricingTrx& pricingTrx, Money& targetMoney)
{
  RoundingFactor roundingFactor = 0;
  CurrencyNoDec roundingNoDec = 0;
  RoundingRule roundingRule = NONE;

  if (getFeeRounding(pricingTrx, targetMoney.code(), roundingFactor, roundingNoDec, roundingRule))
  {
    CurrencyConverter curConverter;
    curConverter.round(targetMoney, roundingFactor, roundingRule);
  }
}

bool
getFeeRounding(PricingTrx& pricingTrx,
               const CurrencyCode& currencyCode,
               RoundingFactor& roundingFactor,
               CurrencyNoDec& roundingNoDec,
               RoundingRule& roundingRule)
{
  const DateTime& tickDate = pricingTrx.ticketingDate();
  const Currency* currency = nullptr;
  currency = pricingTrx.dataHandle().getCurrency(currencyCode);

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

  CurrencyUtil::getControllingNationCode(pricingTrx,
                                         controllingEntityDesc,
                                         nationCode,
                                         foundNation,
                                         foundNationalCurrency,
                                         nationWithMatchingNationalCurrency,
                                         tickDate,
                                         currencyCode);

  if (foundNation)
  {
    const TaxNation* taxNation = pricingTrx.dataHandle().getTaxNation(nationCode, tickDate);

    if (taxNation)
    {
      roundingFactor = taxNation->roundingUnit();
      roundingNoDec = taxNation->roundingUnitNodec();
      roundingRule = taxNation->roundingRule();

      return true;
    }
  }
  else if (foundNationalCurrency)
  {
    const TaxNation* taxNation =
        pricingTrx.dataHandle().getTaxNation(nationWithMatchingNationalCurrency, tickDate);

    if (taxNation)
    {
      roundingFactor = taxNation->roundingUnit();
      roundingNoDec = taxNation->roundingUnitNodec();
      roundingRule = taxNation->roundingRule();

      return true;
    }
  }
  return false;
}

MoneyAmount
calculateObFeeAmountFromAmountMax(PricingTrx& pricingTrx,
                                  const CalcTotals& calcTotals,
                                  const TicketingFeesInfo* feeInfo)
{
  if (feeInfo->feeAmount() < 0.0 ||
      (!calcTotals.equivCurrencyCode.empty() && feeInfo->cur() == calcTotals.equivCurrencyCode) ||
      (calcTotals.equivCurrencyCode.empty() &&
       feeInfo->cur() == calcTotals.convertedBaseFareCurrencyCode))
  {
    return feeInfo->feeAmount();
  }
  else
  {
    Money targetMoney = convertOBFeeCurrency(pricingTrx, calcTotals, feeInfo);
    return targetMoney.value();
  }
}

MoneyAmount
calculateObFeeAmountFromPercentageMax(PricingTrx& pricingTrx,
                                      const CalcTotals& calcTotals,
                                      const TicketingFeesInfo* feeInfo)
{
  MoneyAmount totalPaxAmount = calcTotals.getTotalAmountPerPax();
  MoneyAmount calcAmount = calculateResidualObFeeAmount(pricingTrx, totalPaxAmount, feeInfo);
  const CurrencyCode& paymentCurrency = calcTotals.equivCurrencyCode.empty()
                                            ? calcTotals.convertedBaseFareCurrencyCode
                                            : calcTotals.equivCurrencyCode;

  Money targetMoneyC(calcAmount, paymentCurrency);
  roundOBFeeCurrency(pricingTrx, targetMoneyC);
  calcAmount = targetMoneyC.value();

  MoneyAmount maxAmount = feeInfo->maxFeeAmount();
  const Money sourceMoney(maxAmount, feeInfo->maxFeeCur());
  Money targetMoney(paymentCurrency);

  convertOBFeeCurrency(pricingTrx, sourceMoney, targetMoney); //+rounding
  maxAmount = targetMoney.value();

  return getLowestObFeeAmount(feeInfo->maxFeeCur(), calcAmount, maxAmount);
}

std::pair<const TicketingFeesInfo*, MoneyAmount>
computeMaximumOBFeesPercent(PricingTrx& pricingTrx, const CalcTotals& calcTotals)
{
  std::pair<const TicketingFeesInfo*, MoneyAmount> maximumForPax(nullptr, 0.0);
  for (TicketingFeesInfo* feeInfo : calcTotals.farePath->collectedTktOBFees())
  {
    MoneyAmount feeAmt = 0.0;
    if (feeInfo->feePercent() > 0.0)
      feeAmt = calculateObFeeAmountFromPercentageMax(pricingTrx, calcTotals, feeInfo);
    else
      feeAmt = calculateObFeeAmountFromAmountMax(pricingTrx, calcTotals, feeInfo);

    if (maximumForPax.second < feeAmt)
    {
      maximumForPax.first = feeInfo;
      maximumForPax.second = feeAmt;
    }
  }
  if (!maximumForPax.first && !calcTotals.farePath->collectedTktOBFees().empty())
    maximumForPax.first = *(calcTotals.farePath->collectedTktOBFees().end());
  return maximumForPax;
}

void
prepareMessage(XMLConstruct& construct,
               const char msgType,
               const uint16_t msgCode,
               const std::string& msgText)
{
  construct.openElement(xml2::MessageInformation);
  construct.addAttributeChar(xml2::MessageType, msgType);
  construct.addAttributeShort(xml2::MessageFailCode, msgCode);
  construct.addAttribute(xml2::MessageText, msgText);
  construct.closeElement();
}

static bool
hasMaxOBFeesOptionsVec(const Itin& itin, const uint32_t maxOBFeesOptionCfg)
{
  if (!isValidItin(&itin))
    return false;

  for (const FarePath* fp : itin.farePath())
    if (fp && fp->collectedTktOBFees().size() > maxOBFeesOptionCfg)
      return true;

  return false;
}

bool
hasMaxOBFeesOptionsVec(PricingTrx& pricingTrx)
{
  const uint32_t maxOBFeesOptions = TrxUtil::getConfigOBFeeOptionMaxLimit();

  for (const Itin* itin : pricingTrx.itin())
    if (hasMaxOBFeesOptionsVec(*itin, maxOBFeesOptions))
      return true;
  return false;
}

void
clearAllFeesAndSetMaximum(const Itin& itin, const TicketingFeesInfo* maxFee)
{
  if (!isValidItin(&itin))
    return;

  FarePath* lastFp = nullptr;

  for (FarePath* fp : itin.farePath())
  {
    if (!fp)
      continue;
    fp->collectedTktOBFees().clear();
    lastFp = fp;
  }
  TSE_ASSERT(lastFp && "clearAllFeesAndSetMaximum");

  lastFp->maximumObFee() = maxFee;
}

bool
checkLimitOBFees(TktFeesPricingTrx& trx, const Itin& itin)
{
  if (!trx.getRequest()->isCollectOBFee())
    return false;

  const uint32_t maxOBFeesOptions = TrxUtil::getConfigOBFeeOptionMaxLimit();
  if (!hasMaxOBFeesOptionsVec(itin, maxOBFeesOptions))
    return false;

  std::pair<const TicketingFeesInfo*, MoneyAmount> maximum(nullptr, 0.0);
  CurrencyCode paymentCurrency;

  for (FarePath* fPath : itin.farePath())
  {
    if (!fPath)
      continue;

    if (maxOBFeesOptions < fPath->collectedTktOBFees().size() && checkForZeroMaximum(trx, *fPath))
      continue;

    std::pair<const TicketingFeesInfo*, MoneyAmount> maxForPax =
        computeMaximumOBFeesPercent(trx, *fPath, &itin);

    if (!maxForPax.first || maximum.second >= maxForPax.second)
      continue;

    maximum.first = maxForPax.first;
    maximum.second = maxForPax.second;
    paymentCurrency = getPaymentCurrency(trx, &itin);
  }

  if (!maximum.first)
    return false;

  clearAllFeesAndSetMaximum(
      itin, mockOBFeeInPaymentCurrency(trx, *maximum.first, maximum.second, paymentCurrency));
  return true;
}

bool
checkForZeroMaximum(PricingTrx& pricingTrx, FarePath& farePath)
{
  const std::vector<TicketingFeesInfo*>::const_iterator feesB =
      farePath.collectedTktOBFees().begin();
  const std::vector<TicketingFeesInfo*>::const_iterator feesE = farePath.collectedTktOBFees().end();

  if (std::any_of(feesB, feesE, NonZeroAmount()))
    return false;

  // all fees zero, make last matched zero fee only one for this PTC
  // but only when it not contain FOP
  std::vector<TicketingFeesInfo*> lastZeroIfNotFOP;
  if (farePath.collectedTktOBFees().back()->fopBinNumber().empty())
    lastZeroIfNotFOP.push_back(farePath.collectedTktOBFees().back());

  farePath.collectedTktOBFees() = lastZeroIfNotFOP;
  return true;
}

std::pair<const TicketingFeesInfo*, MoneyAmount>
computeMaximumOBFeesPercent(PricingTrx& pricingTrx, FarePath& farePath, const Itin* itin)
{
  std::pair<const TicketingFeesInfo*, MoneyAmount> maximumForPax(nullptr, 0.0);

  TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(pricingTrx.getRequest());
  std::map<const Itin*, TktFeesRequest::PaxTypePayment*>::const_iterator ptpIter =
      tktFeesReq->paxTypePaymentPerItin().find(itin);
  if (ptpIter == tktFeesReq->paxTypePaymentPerItin().end())
    return maximumForPax;
  TktFeesRequest::PaxTypePayment* p = ptpIter->second;
  if (p == nullptr)
    return maximumForPax;
  CurrencyCode& paymentCurrencyCode = p->currency();

  for (const TicketingFeesInfo* feeInfo : farePath.collectedTktOBFees())
  {
    MoneyAmount feeAmt = 0.0;
    if (feeInfo->feePercent() > 0)
    {
      MoneyAmount chargeAmount = 0;
      if (getChargeAmount(p, chargeAmount))
        calculateObFeeAmountFromPercentageMax(
            pricingTrx, feeInfo, feeAmt, paymentCurrencyCode, chargeAmount);
    }
    else
      calculateObFeeAmountFromAmountMax(pricingTrx, feeInfo, feeAmt, paymentCurrencyCode);

    if (maximumForPax.second < feeAmt)
    {
      maximumForPax.first = feeInfo;
      maximumForPax.second = feeAmt;
    }
  }
  if (!maximumForPax.first && !farePath.collectedTktOBFees().empty())
    maximumForPax.first = *(farePath.collectedTktOBFees().end());
  return maximumForPax;
}

void
calculateObFeeAmountFromPercentageMax(PricingTrx& pricingTrx,
                                      const TicketingFeesInfo* feeInfo,
                                      MoneyAmount& totalFeeFareAmount,
                                      const CurrencyCode& paymentCurrency,
                                      const MoneyAmount& chargeAmount)
{
  MoneyAmount calcAmount = (chargeAmount * feeInfo->feePercent()) / 100.0f;
  Money targetMoneyC(calcAmount, paymentCurrency);
  roundOBFeeCurrency(pricingTrx, targetMoneyC);
  calcAmount = targetMoneyC.value();

  MoneyAmount maxAmount = feeInfo->maxFeeAmount();
  const Money sourceMoney(maxAmount, feeInfo->maxFeeCur());
  Money targetMoney(paymentCurrency);

  convertOBFeeCurrency(pricingTrx, sourceMoney, targetMoney);
  maxAmount = targetMoney.value();

  MoneyAmount lowestCalcAmount = getLowestObFeeAmount(feeInfo->maxFeeCur(), calcAmount, maxAmount);
  totalFeeFareAmount = lowestCalcAmount;
}

void
calculateObFeeAmountFromAmountMax(PricingTrx& pricingTrx,
                                  const TicketingFeesInfo* feeInfo,
                                  MoneyAmount& feeAmount,
                                  const CurrencyCode& paymentCurrency)
{
  if (feeInfo->feeAmount() < 0.0 || (feeInfo->cur() == paymentCurrency))
    feeAmount = feeInfo->feeAmount();
  else
  {
    Money targetMoney = convertOBFeeCurrencyByCode(pricingTrx, paymentCurrency, feeInfo);
    feeAmount = targetMoney.value();
  }
}

CurrencyCode
getPaymentCurrency(TktFeesPricingTrx& tktFeesPricingTrx, const Itin* itin)
{
  CurrencyCode cc;
  TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(tktFeesPricingTrx.getRequest());
  std::map<const Itin*, TktFeesRequest::PaxTypePayment*>::const_iterator ptpIter =
      tktFeesReq->paxTypePaymentPerItin().find(itin);
  if (ptpIter == tktFeesReq->paxTypePaymentPerItin().end())
    throw std::runtime_error("Null pointer to PTP data");
  TktFeesRequest::PaxTypePayment* p = ptpIter->second;
  if (p == nullptr)
    throw std::runtime_error("Null pointer to PTP data");
  cc = p->currency();
  return cc;
}

Money
convertOBFeeCurrencyByCode(PricingTrx& pricingTrx,
                           const CurrencyCode& equivCurrencyCode,
                           const TicketingFeesInfo* feeInfo)
{
  const Money sourceMoney(feeInfo->feeAmount(), feeInfo->cur());
  Money targetMoney(equivCurrencyCode);
  convertOBFeeCurrencyByMoney(pricingTrx, sourceMoney, targetMoney);
  return targetMoney;
}

void
convertOBFeeCurrencyByMoney(PricingTrx& pricingTrx, const Money& sourceMoney, Money& targetMoney)
{
  CurrencyConversionFacade converter;
  converter.convert(targetMoney, sourceMoney, pricingTrx, false, CurrencyConversionRequest::TAXES);
  RoundingFactor roundingFactor = 0;
  CurrencyNoDec roundingNoDec = 0;
  RoundingRule roundingRule = NONE;
  if (getFeeRounding(pricingTrx, targetMoney.code(), roundingFactor, roundingNoDec, roundingRule))
  {
    CurrencyConverter curConverter;
    curConverter.round(targetMoney, roundingFactor, roundingRule);
  }
}

bool
getChargeAmount(const TktFeesRequest::PaxTypePayment* ptp, MoneyAmount& chargeAmount)
{
  const std::vector<TktFeesRequest::PassengerPaymentInfo*>& ppiV = ptp->ppiV();
  if (ppiV.empty())
    return false;
  const TktFeesRequest::PassengerPaymentInfo* ppi = ppiV[0]; // 1st Fop CreditCard
  const std::vector<TktFeesRequest::FormOfPayment*>& fopV = ppi->fopVector();
  if (fopV.empty())
    return false;

  if (fopV[0]->chargeAmountInRequest())
    chargeAmount = fopV[0]->chargeAmount();
  else
    chargeAmount = ptp->amount();
  return true;
}

bool
checkLimitOBFees(PricingTrx& pricingTrx, FareCalcCollector& fareCalcCollector)
{
  if (!pricingTrx.getRequest()->isCollectOBFee())
    return false;

  if (!hasMaxOBFeesOptionsVec(pricingTrx))
    return false;

  uint32_t maxOBFeesOptions = TrxUtil::getConfigOBFeeOptionMaxLimit();

  std::pair<const TicketingFeesInfo*, MoneyAmount> maximum(nullptr, 0.0);
  CurrencyCode paymentCurrency;

  for (CalcTotals* totals : fareCalcCollector.passengerCalcTotals())
  {
    if (totals->farePath->processed())
    {
      std::pair<const TicketingFeesInfo*, MoneyAmount> maxForPax(nullptr, 0.0);
      bool isZeroMax = false;
      if (maxOBFeesOptions < totals->farePath->collectedTktOBFees().size())
        isZeroMax = checkForZeroMaximum(pricingTrx, *totals);

      if (isZeroMax && totals->farePath->collectedTktOBFees().empty())
        continue;
      if (!isZeroMax)
      {
        maxForPax = OBFeesUtils::computeMaximumOBFeesPercent(pricingTrx, *totals);
        if (maxForPax.first)
        {
          if (maximum.second < maxForPax.second)
          {
            maximum.first = maxForPax.first;
            maximum.second = maxForPax.second;
          }
        }
      }
      if (paymentCurrency.empty())
        paymentCurrency = totals->equivCurrencyCode.empty()
                              ? totals->convertedBaseFareCurrencyCode
                              : totals->equivCurrencyCode;
    }
  }
  if (!maximum.first)
    return false;
  const FarePath* lastOnePTC = clearAllFeesAndGetLastPTC(fareCalcCollector.passengerCalcTotals());

  const_cast<FarePath*>(lastOnePTC)->maximumObFee() =
      mockOBFeeInPaymentCurrency(pricingTrx, *(maximum.first), maximum.second, paymentCurrency);
  return true;

}

const FarePath*
clearAllFeesAndGetLastPTC(const std::vector<CalcTotals*>& calcTotals)
{
  const FarePath* lastOnePTC = nullptr;

  for (CalcTotals* totalsIter : calcTotals)
  {
    if (totalsIter->farePath->processed())
    {
      const_cast<FarePath*>(totalsIter->farePath)->collectedTktOBFees().clear();

      lastOnePTC = totalsIter->farePath;
    }
  }

  return lastOnePTC;
}

bool
checkForZeroMaximum(PricingTrx& pricingTrx, const CalcTotals& calcTotals)
{
  const std::vector<TicketingFeesInfo*>::const_iterator feesBegin =
      calcTotals.farePath->collectedTktOBFees().begin();
  const std::vector<TicketingFeesInfo*>::const_iterator feesEnd =
      calcTotals.farePath->collectedTktOBFees().end();

  if (std::any_of(feesBegin, feesEnd, NonZeroAmount()))
    return false;

  // all fees zero, make last matched zero fee only one for this PTC
  // but only when it not contain FOP
  std::vector<TicketingFeesInfo*> lastZeroIfNotFOP;
  if (calcTotals.farePath->collectedTktOBFees().back()->fopBinNumber().empty())
    lastZeroIfNotFOP.push_back(calcTotals.farePath->collectedTktOBFees().back());

  const_cast<FarePath*>(calcTotals.farePath)->collectedTktOBFees() = lastZeroIfNotFOP;
  return true;
}

const TicketingFeesInfo*
mockOBFeeInPaymentCurrency(PricingTrx& pricingTrx,
                           const TicketingFeesInfo& sourceFee,
                           const MoneyAmount& feeAmount,
                           const CurrencyCode& paymentCurrency)
{
  TicketingFeesInfo* convertedFee;
  pricingTrx.dataHandle().get(convertedFee);
  convertedFee->getValuableData(sourceFee);

  Money targetMoney(feeAmount, paymentCurrency);
  roundOBFeeCurrency(pricingTrx, targetMoney);
  convertedFee->cur() = paymentCurrency;
  convertedFee->feeAmount() = targetMoney.value();
  convertedFee->noDec() = targetMoney.noDec();

  return convertedFee;
}

bool
isBinCorrect(const FopBinNumber& bin)
{
  return bin.size() == FOP_BIN_SIZE &&
         std::find_if(bin.begin(), bin.end(), !boost::bind<bool>(&isDigit, _1)) == bin.end();
}

void
prepareOBFees(PricingTrx& pricingTrx,
              CalcTotals& calcTotals,
              XMLConstruct& construct,
              bool limitMaxOBFees)
{
  size_t maxNumFType = 0, maxNumTType = 0, maxNumRType = 0;
  getNumberOfOBFees(pricingTrx, *calcTotals.farePath, maxNumFType, maxNumTType, maxNumRType);

  if (pricingTrx.getRequest()->isCollectOBFee())
  {
    if (calcTotals.farePath->maximumObFee() &&
        EPSILON < calcTotals.farePath->maximumObFee()->feeAmount())
    {
      const TicketingFeesInfo* feeInfo = calcTotals.farePath->maximumObFee();

      OBFeesUtils::prepareOBFee(pricingTrx, calcTotals, construct, feeInfo, limitMaxOBFees);
      construct.addAttributeChar(xml2::ShowNoObFees, 'T');

      construct.addAttribute(xml2::MessageText, prepareMaxAmtMsg(pricingTrx, *feeInfo));

      construct.closeElement();
      // this statement is added to display MAX item only and do not display anything else
      // it'll happening only when there are more than 50 OB Fees are qualified
    }
    else
    {
      const std::vector<TicketingFeesInfo*>& collectedOBFees =
          calcTotals.farePath->collectedTktOBFees();

      if (pricingTrx.isProcess2CC() && 2 == collectedOBFees.size())
        OBFeesUtils::prepare2CardsOBFee(pricingTrx, calcTotals, construct, collectedOBFees);
      else
        OBFeesUtils::prepareAllOBFees(
            pricingTrx, calcTotals, construct, maxNumFType, limitMaxOBFees, collectedOBFees);
    }
  }

  if (pricingTrx.getRequest()->isCollectTTypeOBFee())
    OBFeesUtils::prepareAllOBFees(pricingTrx,
                                  calcTotals,
                                  construct,
                                  maxNumTType,
                                  limitMaxOBFees,
                                  calcTotals.farePath->collectedTTypeOBFee());

  if (pricingTrx.getRequest()->isCollectRTypeOBFee())
    OBFeesUtils::prepareAllOBFees(pricingTrx,
                                  calcTotals,
                                  construct,
                                  maxNumRType,
                                  limitMaxOBFees,
                                  calcTotals.farePath->collectedRTypeOBFee());
}

void
prepareOBFee(PricingTrx& pricingTrx,
             CalcTotals& calcTotals,
             XMLConstruct& construct,
             const TicketingFeesInfo* feeInfo,
             bool limitMaxOBFees,
             MoneyAmount feeAmt,
             MoneyAmount fareAmtWith2CCFee)
{
  construct.openElement(xml2::ServiceFeeDetailedInfo);

  construct.addAttribute(xml2::ServiceTypecode,
                         feeInfo->serviceTypeCode() + feeInfo->serviceSubTypeCode());

  MoneyAmount totalFeeFareAmount(0.0);
  CurrencyNoDec noDec = 0;

  if (Money::isZeroAmount(fareAmtWith2CCFee) && !pricingTrx.isProcess2CC())
  {
    OBFeesUtils::calculateObFeeAmount(
        pricingTrx, calcTotals, feeInfo, totalFeeFareAmount, noDec, limitMaxOBFees);
    construct.addAttributeDouble(xml2::ServiceFeeAmount, totalFeeFareAmount, noDec);
  }

  OBFeeSubType subType = TrxUtil::getOBFeeSubType(feeInfo->serviceSubTypeCode());
  if (subType == OBFeeSubType::OB_F_TYPE && !limitMaxOBFees)
  {
    totalFeeFareAmount += calcTotals.getTotalAmountPerPax();
    Money moneyEquiv(calcTotals.equivCurrencyCode);
    if (Money::isZeroAmount(fareAmtWith2CCFee) && !pricingTrx.isProcess2CC())
      construct.addAttributeDouble(xml2::ServiceFeeAmountTotal,
                                   totalFeeFareAmount,
                                   moneyEquiv.noDec(pricingTrx.ticketingDate()));
    else
    {
      construct.addAttributeDouble(
          xml2::ServiceFeeAmount, feeAmt, moneyEquiv.noDec(pricingTrx.ticketingDate()));

      construct.addAttributeDouble(xml2::ServiceFeeAmountTotal,
                                   fareAmtWith2CCFee,
                                   moneyEquiv.noDec(pricingTrx.ticketingDate()));
    }
  }

  construct.addAttribute(xml2::FopBINNumber, feeInfo->fopBinNumber());
  std::ostringstream combineITATAind;
  combineITATAind << feeInfo->refundReissue() << feeInfo->commission() << feeInfo->interline();
  construct.addAttribute(xml2::IATAindCombined, combineITATAind.str());
  construct.addAttributeChar(xml2::NoChargeInd, feeInfo->noCharge());
  if (feeInfo->feePercent() > 0.0)
  {
    construct.addAttributeDouble(
        xml2::ServiceFeePercent, feeInfo->feePercent(), feeInfo->feePercentNoDec());
    const CurrencyCode& paymentCurrency = calcTotals.equivCurrencyCode.empty()
                                              ? calcTotals.convertedBaseFareCurrencyCode
                                              : calcTotals.equivCurrencyCode;
    MoneyAmount maxAmount = feeInfo->maxFeeAmount();
    if (feeInfo->maxFeeCur() == paymentCurrency)
    {
      construct.addAttributeDouble(xml2::MaxServiceFeeAmt, maxAmount, feeInfo->maxFeeNoDec());
    }
    else
    {
      const Money sourceMoney(maxAmount, feeInfo->maxFeeCur());
      Money targetMoney(paymentCurrency);
      convertOBFeeCurrency(pricingTrx, sourceMoney, targetMoney); //+ do rounding
      maxAmount = targetMoney.value();
      construct.addAttributeDouble(
          xml2::MaxServiceFeeAmt, maxAmount, targetMoney.noDec(pricingTrx.ticketingDate()));
    }
  }

  static ObFeeDescriptors obFeeDesc;
  std::string serviceDescription = feeInfo->commercialName().empty()
                                       ? obFeeDesc.getDescription(feeInfo->serviceSubTypeCode())
                                       : feeInfo->commercialName();
  construct.addAttribute(xml2::ServiceDescription, serviceDescription);

  if ((!fallback::fallbackValidatingCxrMultiSp(&pricingTrx) || pricingTrx.overrideFallbackValidationCXRMultiSP())
      && calcTotals.farePath &&
      !calcTotals.farePath->defaultValidatingCarrier().empty())
  {
    // OB Fee is collected for default validating carrier of primary settlement plan
    construct.addAttribute(xml2::ValidatingCxrCode,
                           calcTotals.farePath->defaultValidatingCarrier());
  }
}

void
prepareOBFee(PricingTrx& pricingTrx,
             CalcTotals& calcTotals,
             XMLConstruct& construct,
             TicketingFeesInfo* feeInfo,
             const FopBinNumber& fopBin,
             const MoneyAmount& chargeAmount,
             const CurrencyNoDec& numDec,
             const MoneyAmount& feeAmt,
             const MoneyAmount& fareAmtWith2CCFee)
{
  if (feeInfo)
    prepareOBFee(pricingTrx, calcTotals, construct, feeInfo, false, feeAmt, fareAmtWith2CCFee);
  else
  {
    construct.openElement(xml2::ServiceFeeDetailedInfo);
    Money moneyEquiv(calcTotals.equivCurrencyCode);
    construct.addAttributeDouble(xml2::ServiceFeeAmountTotal,
                                 fareAmtWith2CCFee,
                                 moneyEquiv.noDec(pricingTrx.ticketingDate()));
  }

  construct.addAttribute(xml2::RequestedBin, fopBin);
  construct.addAttributeDouble(xml2::CardChargeAmount, chargeAmount, numDec);
  construct.closeElement();
}

void
prepareAllOBFees(PricingTrx& pricingTrx,
                 CalcTotals& calcTotals,
                 XMLConstruct& construct,
                 size_t maxOBFeesOptions,
                 bool limitMaxOBFees,
                 const std::vector<TicketingFeesInfo*>& collectedOBFees)
{
  if (collectedOBFees.empty())
    return;

  uint32_t numProcessed = 0;

  for (TicketingFeesInfo* feeInfo : collectedOBFees)
  {
    if (numProcessed >= maxOBFeesOptions)
      break;
    OBFeesUtils::prepareOBFee(pricingTrx, calcTotals, construct, feeInfo, limitMaxOBFees);
    construct.closeElement();
    ++numProcessed;
  }
}

void
calculate2CardsOBFee(PricingTrx& pricingTrx,
                     const CalcTotals& calcTotals,
                     const std::vector<TicketingFeesInfo*>& collectedOBFees,
                     MoneyAmount& firstCardChargeAmount,
                     MoneyAmount& feeAmt1,
                     MoneyAmount& secondCardChargeAmount,
                     MoneyAmount& feeAmt2,
                     MoneyAmount& totalFeeFareAmount,
                     CurrencyNoDec& noDec)
{
  secondCardChargeAmount =
      std::min(pricingTrx.getRequest()->paymentAmountFop(), calcTotals.getTotalAmountPerPax());
  firstCardChargeAmount = calcTotals.getTotalAmountPerPax() - secondCardChargeAmount;
  Money moneyEquiv(calcTotals.equivCurrencyCode);

  if (collectedOBFees.front() && !Money::isZeroAmount(firstCardChargeAmount))
    calculateObFeeAmount(pricingTrx,
                         calcTotals,
                         collectedOBFees.front(),
                         feeAmt1,
                         noDec,
                         false,
                         firstCardChargeAmount);
  if (collectedOBFees.back() && !Money::isZeroAmount(secondCardChargeAmount))
    calculateObFeeAmount(pricingTrx,
                         calcTotals,
                         collectedOBFees.back(),
                         feeAmt2,
                         noDec,
                         false,
                         secondCardChargeAmount);

  totalFeeFareAmount = calcTotals.getTotalAmountPerPax() + feeAmt1 + feeAmt2;
}

void
prepare2CardsOBFee(PricingTrx& pricingTrx,
                   CalcTotals& calcTotals,
                   XMLConstruct& construct,
                   const std::vector<TicketingFeesInfo*>& collectedOBFees)
{
  CurrencyNoDec noDec = 0;
  MoneyAmount totalFeeFareAmount(0.0), feeAmt1(0.0), feeAmt2(0.0);
  MoneyAmount firstCardChargeAmount(0.0), secondCardChargeAmount(0.0);
  calculate2CardsOBFee(pricingTrx,
                       calcTotals,
                       collectedOBFees,
                       firstCardChargeAmount,
                       feeAmt1,
                       secondCardChargeAmount,
                       feeAmt2,
                       totalFeeFareAmount,
                       noDec);

  construct.addAttributeDouble(xml2::ServiceFeeAmount, totalFeeFareAmount, noDec);

  prepareOBFee(pricingTrx,
               calcTotals,
               construct,
               collectedOBFees.front(),
               pricingTrx.getRequest()->formOfPayment(),
               firstCardChargeAmount,
               noDec,
               feeAmt1,
               totalFeeFareAmount);

  prepareOBFee(pricingTrx,
               calcTotals,
               construct,
               collectedOBFees.back(),
               pricingTrx.getRequest()->secondFormOfPayment(),
               secondCardChargeAmount,
               noDec,
               feeAmt2,
               totalFeeFareAmount);
}

void
getNumberOfOBFees(PricingTrx& trx,
                  const FarePath& farePath,
                  size_t& maxNumFType,
                  size_t& maxNumTType,
                  size_t& maxNumRType)
{
  size_t minNumFType =
      ((!trx.getRequest()->formOfPayment().empty()) && (trx.getRequest()->isCollectOBFee()))
          ? farePath.collectedTktOBFees().size()
          : 2;

  maxNumTType = std::min(MAX_NUM_OB_FEE - minNumFType, farePath.collectedTTypeOBFee().size());
  maxNumRType =
      std::min(MAX_NUM_OB_FEE - minNumFType - maxNumTType, farePath.collectedRTypeOBFee().size());
  maxNumFType = MAX_NUM_OB_FEE - maxNumTType - maxNumRType;
}

void
prepareCalcTotalsForObFees(const PaxDetail& paxDetail,
                           const FarePath& farePath,
                           CalcTotals& calcTotals)
{
  calcTotals.setTaxAmount(paxDetail.totalTaxes());
  calcTotals.equivFareAmount =
      paxDetail.equivalentAmount() ? paxDetail.equivalentAmount() : paxDetail.baseFareAmount();
  calcTotals.equivCurrencyCode = !paxDetail.equivalentCurrencyCode().empty()
                                     ? paxDetail.equivalentCurrencyCode()
                                     : paxDetail.baseCurrencyCode();
  calcTotals.convertedBaseFare = calcTotals.equivFareAmount;
  calcTotals.convertedBaseFareCurrencyCode = calcTotals.equivCurrencyCode;

  for (const PricingUnit* pricingUnit : farePath.pricingUnit())
    for (const FareUsage* fareUsage : pricingUnit->fareUsage())
      for (const TravelSeg* tvlSeg : fareUsage->travelSeg())
        calcTotals.fareUsages[tvlSeg] = fareUsage;
}

void
formatWpnObFeeResponseMaxOptions(const MoneyAmount& feeAmount, int& msgIdx, std::string& response)
{
  std::ostringstream msgText;
  msgText << std::left << std::fixed << std::setprecision(2) << feeAmount;
  const std::string txt = "     MAXIMUM AMOUNT PER PASSENGER - " + msgText.str();

  response += OBFeesUtils::wrapObFeeMsgLine(txt, ++msgIdx);
}

bool
validatePaxTypeCode(PaxTypeCode farePathPaxType, PaxTypeCode paxDetailPaxType)
{
  if (farePathPaxType == paxDetailPaxType)
    return true;

  if (farePathPaxType[0] == paxDetailPaxType[0])
  {
    if (std::isdigit(paxDetailPaxType[1]) && std::isdigit(paxDetailPaxType[2]))
    {
      paxDetailPaxType[1] = 'N';
      paxDetailPaxType[2] = 'N';
      return farePathPaxType == paxDetailPaxType;
    }
  }
  return false;
}

void
formatWpnObFeeResponse(PricingDetailTrx& pricingDetailTrx,
                       const PaxDetail& paxDetail,
                       const FarePath& farePath,
                       bool obFeesLimit,
                       int& msgIdx,
                       std::string& response)
{
  CalcTotals calcTotals;
  prepareCalcTotalsForObFees(paxDetail, farePath, calcTotals);

  if (!farePath.collectedTktOBFees().empty())
  {
    response += OBFeesUtils::wrapObFeeMsgLine(OBFeesUtils::prepareHeaderObFeeMsg(), ++msgIdx);

    if (pricingDetailTrx.isProcess2CC() && farePath.collectedTktOBFees().size() == 2)
    {
      if (!farePath.collectedTktOBFees().front() && !farePath.collectedTktOBFees().back())
        return;

      response += OBFeesUtils::wrapObFeeMsgLine(
          OBFeesUtils::prepareColumnsNamesObFeeMsg(paxDetail.paxType()), ++msgIdx);

      CurrencyNoDec noDec = 0;
      MoneyAmount totalFeeFareAmount(0.0), feeAmt1(0.0), feeAmt2(0.0);
      MoneyAmount firstCardChargeAmount(0.0), secondCardChargeAmount(0.0);
      calculate2CardsOBFee(pricingDetailTrx,
                           calcTotals,
                           farePath.collectedTktOBFees(),
                           firstCardChargeAmount,
                           feeAmt1,
                           secondCardChargeAmount,
                           feeAmt2,
                           totalFeeFareAmount,
                           noDec);

      std::string firstCard =
          prepareObFeeMsg2CC(farePath.collectedTktOBFees().front(), feeAmt1, noDec);
      std::string secondCard =
          prepareObFeeMsg2CC(farePath.collectedTktOBFees().back(), feeAmt2, noDec);

      if (secondCard.empty())
        firstCard += prepareTotalFeeAmout(totalFeeFareAmount, noDec);
      else
        secondCard += prepareTotalFeeAmout(totalFeeFareAmount, noDec);

      if (!firstCard.empty())
        response += OBFeesUtils::wrapObFeeMsgLine(firstCard, ++msgIdx);
      if (!secondCard.empty())
        response += OBFeesUtils::wrapObFeeMsgLine(secondCard, ++msgIdx);
    }
    else
    {
      response += OBFeesUtils::wrapObFeeMsgLine(
          OBFeesUtils::prepareColumnsNamesObFeeMsg(paxDetail.paxType()), ++msgIdx);

      for (TicketingFeesInfo* tktInfo : farePath.collectedTktOBFees())
      {
        TSE_ASSERT(tktInfo);

        const std::string txt = OBFeesUtils::prepareObFeeMsg(
            pricingDetailTrx, calcTotals, tktInfo, obFeesLimit, tktInfo->feeAmount());

        response += OBFeesUtils::wrapObFeeMsgLine(txt, ++msgIdx);
      }
    }
    response += OBFeesUtils::wrapObFeeMsgLine(" ", ++msgIdx);
  }
}

void
addObFeesInfo(PricingDetailTrx& pricingDetailTrx,
              std::string& response,
              const PaxDetail* paxDetail,
              const Itin* itin)
{
  int msgIdx = OBFeesUtils::getLastIdx(response);

  if (!paxDetail || !itin || itin->farePath().empty() || itin->farePath().size() > 1)
  {
    response = "<PricingResponse>" + response + "</PricingResponse>";
    return;
  }

  const FarePath& farePath = *itin->farePath().back();
  if (!farePath.paxType() ||
      !validatePaxTypeCode(farePath.paxType()->paxType(), paxDetail->paxType()))
    return;

  const bool obFeesLimit = OBFeesUtils::checkLimitOBFees(pricingDetailTrx, *itin);
  if (obFeesLimit)
  {
    response += OBFeesUtils::wrapObFeeMsgLine(OBFeesUtils::prepareHeaderObFeeMsg(), ++msgIdx);
    OBFeesUtils::formatWpnObFeeResponseMaxOptions(
        farePath.maximumObFee()->feeAmount(), msgIdx, response);
  }
  else
    OBFeesUtils::formatWpnObFeeResponse(
        pricingDetailTrx, *paxDetail, farePath, obFeesLimit, msgIdx, response);
}

void
addObFeeInfoWpan(AltPricingDetailObFeesTrx& trx)
{
  TSE_ASSERT(trx.itin().size() == trx.paxDetails().size());
  TSE_ASSERT(trx.itin().size() == trx.accompRestrictionVec().size());

  for (size_t itinId = 0; itinId < trx.paxDetails().size(); ++itinId)
  {
    std::string& response = trx.accompRestrictionVec()[itinId].selectionXml();
    const size_t pxiNodeEnd = response.find("</PXI>");
    if (pxiNodeEnd == std::string::npos)
      continue;

    const Itin& itin = *trx.itin()[itinId];
    if (itin.farePath().empty())
      continue;
    FarePath& farePath = *itin.farePath().back();
    const PaxDetail& paxDetail = *trx.paxDetails()[itinId];

    CalcTotals calcTotals;
    calcTotals.farePath = &farePath;
    prepareCalcTotalsForObFees(paxDetail, farePath, calcTotals);

    if ((!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
      && !farePath.collectedTktOBFees().empty())
      setDefaultValidatingCxrForObFees(paxDetail, farePath);

    XMLConstruct construct;
    prepareOBFees(trx, calcTotals, construct, checkLimitOBFees(trx, itin));

    response.insert(pxiNodeEnd, construct.getXMLData());
  }
}

std::string
displayGreenScreenMsg(PricingTrx& trx, const CalcTotals& calcTotals)
{
  std::ostringstream msgText;
  msgText << OBFeesUtils::ObfSectionStart << "\n";
  msgText << OBFeesUtils::prepareHeaderObFeeMsg() << "\n";

  if (calcTotals.farePath->maximumObFee())
  {
    msgText << prepareMaxAmtMsg(trx, *calcTotals.farePath->maximumObFee()) << "\n";
  }
  else
  {
    if (trx.isProcess2CC() && 2 == calcTotals.farePath->collectedTktOBFees().size())
    {
      if (calcTotals.farePath->collectedTktOBFees().front() ||
          calcTotals.farePath->collectedTktOBFees().back())
      {
        msgText << OBFeesUtils::prepareColumnsNamesObFeeMsg(calcTotals.truePaxType.data()) << "\n";

        CurrencyNoDec noDec = 0;
        MoneyAmount totalFeeFareAmount(0.0), feeAmt1(0.0), feeAmt2(0.0);
        MoneyAmount firstCardChargeAmount(0.0), secondCardChargeAmount(0.0);
        calculate2CardsOBFee(trx,
                             calcTotals,
                             calcTotals.farePath->collectedTktOBFees(),
                             firstCardChargeAmount,
                             feeAmt1,
                             secondCardChargeAmount,
                             feeAmt2,
                             totalFeeFareAmount,
                             noDec);

        std::string firstCard =
            prepareObFeeMsg2CC(calcTotals.farePath->collectedTktOBFees().front(), feeAmt1, noDec);
        std::string secondCard =
            prepareObFeeMsg2CC(calcTotals.farePath->collectedTktOBFees().back(), feeAmt2, noDec);

        if (secondCard.empty())
          firstCard += prepareTotalFeeAmout(totalFeeFareAmount, noDec);
        else
          secondCard += prepareTotalFeeAmout(totalFeeFareAmount, noDec);

        if (!firstCard.empty())
          msgText << firstCard << "\n";
        if (!secondCard.empty())
          msgText << secondCard << "\n";
      }
    }
    else
    {
      msgText << OBFeesUtils::prepareColumnsNamesObFeeMsg(calcTotals.truePaxType.data()) << "\n";

      for (TicketingFeesInfo* tktInfo : calcTotals.farePath->collectedTktOBFees())
      {
        TSE_ASSERT(tktInfo);
        msgText << OBFeesUtils::prepareObFeeMsg(trx, calcTotals, tktInfo, false) << "\n";
      }
    }
  }
  msgText << "\n" << OBFeesUtils::ObfSectionEnd;
  return msgText.str();
}

std::string
prepareMaxAmtMsg(const PricingTrx& trx, const TicketingFeesInfo& feeInfo)
{
  std::ostringstream trailerMsg;
  trailerMsg << "MAXIMUM AMOUNT PER PASSENGER - ";
  trailerMsg.precision(Money(feeInfo.cur()).noDec(trx.ticketingDate()));
  trailerMsg.setf(std::ios::fixed, std::ios::floatfield);
  trailerMsg << feeInfo.feeAmount();
  return trailerMsg.str();
}

void
setDefaultValidatingCxrForObFees(const PaxDetail& paxDetail, FarePath& farePath)
{
  if (paxDetail.vclInfo().empty())
    return;

  const char* const searchTagDCX = "<DCX";
  const char* const searchTagB00 = "B00=";

  const std::string& vclInfo = paxDetail.vclInfo();
  size_t dcxBegin = vclInfo.find(searchTagDCX);
  if (dcxBegin == std::string::npos)
    return;

  size_t b00Begin = vclInfo.find(searchTagB00, dcxBegin);
  if (b00Begin == std::string::npos)
    return;

  size_t cxrBegin = b00Begin + 5; // locate default validating carrier code <DCX B00="SN"
  size_t cxrEnd = vclInfo.find('"', cxrBegin);
  if (cxrEnd == std::string::npos)
    return;

  size_t cxrLen = cxrEnd - cxrBegin;
  if (cxrLen < 2 || cxrLen > 3) // typedef Code<3> CarrierCode;
    return;

  farePath.defaultValidatingCarrier() = vclInfo.substr(cxrBegin, cxrLen);
}

} // namespace OBFeesUtils
} // namespace tse
