//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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
#include "Xform/STLTktFeesPricingResponseFormatter.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "Common/XMLConstruct.h"
#include "DataModel/Fare.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/TktFeesRequest.h"
#include "Diagnostic/Diagnostic.h"
#include "TicketingFee/TicketingFeesService.h"
#include "Xform/DataModelMap.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/PricingResponseSTLTags.h"
#include "Xform/STLMessage.h"

#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

namespace
{
// config params
}

namespace tse
{
struct NonZeroAmount
{
  bool operator()(const TicketingFeesInfo* feeInfo) const
  {
    if (feeInfo->feeAmount() > EPSILON || feeInfo->feePercent() > EPSILON)
      return true;
    return false;
  }
};

static Logger
logger("atseintl.Xform.STLTktFeesPricingResponseFormatter");

const MoneyAmount STLTktFeesPricingResponseFormatter::INVALID_AMT = -1.0;

std::string
STLTktFeesPricingResponseFormatter::formatResponse(
    const std::string& responseString,
    TktFeesPricingTrx& tktFeesTrx,
    const ErrorResponseException::ErrorResponseCode errCode)
{
  LOG4CXX_INFO(logger, "Response (before XML tagging):\n" << responseString);
  XMLConstruct construct;
  construct.openElement(OBTicketingFeeRS);
  TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(tktFeesTrx.getRequest());
  if (!tktFeesReq->fullVersionRQ().empty())
    construct.addAttribute("version", tktFeesReq->fullVersionRQ());
  else
    construct.addAttribute("version", _version);

  Diagnostic& diag = tktFeesTrx.diagnostic();
  if (tktFeesTrx.diagnostic().diagnosticType() == Diagnostic854)
  {
    prepareHostPortInfo(tktFeesTrx, construct);
  }

  if (errCode > 0)
  {
    if (diag.diagnosticType() != DiagnosticNone && !diag.toString().empty())
    {
      prepareResponseText(diag.toString(), construct);
    }
    prepareMessage(construct, STL::Error, Message::errCode(errCode), responseString);
    LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::formatResponse() - error > 0 ");
  }
  else if (diag.diagnosticType() != DiagnosticNone && diag.diagnosticType() != Diagnostic854)
  {
    std::string tmpResponse = diag.toString();
    if (tmpResponse.length() == 0)
    {
      char tmpBuf[256];
      sprintf(tmpBuf, "DIAGNOSTIC %d RETURNED NO DATA", diag.diagnosticType());
      tmpResponse.insert(0, tmpBuf);
      prepareResponseText(tmpResponse, construct);
    }
    else
      prepareResponseText(tmpResponse, construct);
  }
  else
  {
    LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::formatResponse() - entered ");
    formatTktFeesResponse(construct, tktFeesTrx);
  }
  construct.closeElement();
  LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::formatResponse() - complete");
  return construct.getXMLData();
}

void
STLTktFeesPricingResponseFormatter::formatResponse(const ErrorResponseException& ere,
                                                   std::string& response)
{
  XMLConstruct construct;
  construct.openElement(OBTicketingFeeRS);
  construct.addAttribute("version", _version);
  prepareMessage(construct, STL::Error, Message::errCode(ere.code()), ere.message());
  construct.closeElement();

  response = construct.getXMLData();
}

void
STLTktFeesPricingResponseFormatter::prepareResponseText(const std::string& responseString,
                                                        XMLConstruct& construct,
                                                        const bool noSizeLImit) const
{
  LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::prepareResponseText - entered");
  std::string tmpResponse = responseString;

  size_t lastPos = 0;
  int recNum = 2;
  const int BUF_SIZE = 256;
  const int AVAILABLE_SIZE = BUF_SIZE - 52;
  while (1)
  {
    lastPos = tmpResponse.rfind("\n");
    if (lastPos != std::string::npos && lastPos > 0 &&
        lastPos == (tmpResponse.length() - 1)) // lint !e530
      tmpResponse.replace(lastPos, 1, "\0");
    else
      break;
  }
  char* pHolder = nullptr;
  int tokenLen = 0;
  for (char* token = strtok_r((char*)tmpResponse.c_str(), "\n", &pHolder); token != nullptr;
       token = strtok_r(nullptr, "\n", &pHolder), recNum++)
  {
    tokenLen = strlen(token);

    if (tokenLen == 0)
      continue;
    else if (tokenLen > AVAILABLE_SIZE && !noSizeLImit)
    {
      LOG4CXX_WARN(logger, "Line: [" << token << "] too long!");
      continue;
    }
    prepareMessage(construct, STL::Diagnostic, recNum + 1, token);
  }
  LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::prepareResponseText - complete");
}

void
STLTktFeesPricingResponseFormatter::prepareMessage(XMLConstruct& construct,
                                                   const std::string& msgType,
                                                   const uint16_t msgCode,
                                                   const std::string& msgText) const
{
  LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::prepareMessage() - entered");
  construct.openElement(STL::MessageInformation);
  construct.addAttribute(STL::MessageType, msgType);
  construct.addAttributeShort(STL::MessageCode, msgCode);
  construct.addElementData(msgText.c_str());
  construct.closeElement();
  LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::prepareMessage() - complete");
}

void
STLTktFeesPricingResponseFormatter::formatTktFeesResponse(XMLConstruct& construct,
                                                          TktFeesPricingTrx& tktFeesTrx)
{
  LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::formatTktFeesResponse() - entered");

  checkLimitOBFees(tktFeesTrx);

  for (std::vector<Itin*>::iterator itinI = tktFeesTrx.itin().begin(),
                                    itinEnd = tktFeesTrx.itin().end();
       itinI != itinEnd;
       ++itinI)
  {
    if ((*itinI) == nullptr || ((*itinI)->errResponseCode() != ErrorResponseException::NO_ERROR) ||
        ((*itinI)->farePath().empty()))
      continue;
    for (const auto pFarePath : (*itinI)->farePath())
    {
      if (pFarePath == nullptr)
        continue;
      processTktFeesSolutionPerPaxType(tktFeesTrx, (*itinI), construct, (*pFarePath));
    }
  }
  LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::formatTktFeesResponse() - complete");
}

void
STLTktFeesPricingResponseFormatter::processTktFeesSolutionPerPaxType(TktFeesPricingTrx& pricingTrx,
                                                                     const Itin* itin,
                                                                     XMLConstruct& construct,
                                                                     const FarePath& fPath)
{
  LOG4CXX_DEBUG(logger,
                "STLTktFeesPricingResponseFormatter::processTktFeesSolutionPerPaxType() - entered");
  TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(pricingTrx.getRequest());
  TktFeesRequest::PaxTypePayment* ptp = tktFeesReq->paxTypePaymentPerItin()[itin];
  if (ptp == nullptr)
  {
    LOG4CXX_ERROR(logger, "Null pointer to PTP data");
    throw ErrorResponseException(ErrorResponseException::PROCESSING_ERROR_DETECTED);
  }
  std::vector<TktFeesRequest::PassengerPaymentInfo*>& ppiV = ptp->ppiV();
  if (ppiV.empty())
  {
    LOG4CXX_ERROR(logger, "Empty ppiVector in PTP data");
    return;
  }
  for (TktFeesRequest::PassengerPaymentInfo* ppi : ppiV)
  {
    _tktFeesAmount = 0;
    construct.openElement(STL::PassengerOBFees);
    XMLConstruct constructTFS;
    processTktFeesSolution(pricingTrx, itin, constructTFS, fPath, ptp);

    processPaxType(pricingTrx, construct, ptp, ppi);
    processPassengerIdentity(pricingTrx, construct, ppi->paxRefObjectID());
    construct.addElementData(constructTFS.getXMLData().c_str());

    construct.closeElement();
  }
  LOG4CXX_DEBUG(
      logger, "STLTktFeesPricingResponseFormatter::processTktFeesSolutionPerPaxType() - complete");
}

void
STLTktFeesPricingResponseFormatter::processPaxType(TktFeesPricingTrx& pricingTrx,
                                                   XMLConstruct& construct,
                                                   TktFeesRequest::PaxTypePayment* ptp,
                                                   TktFeesRequest::PassengerPaymentInfo* ppi)
{
  LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::processPaxType() - entered");

  //  construct.addAttribute(STL::feeApplyInd, ...);
  if (ptp->paxType())
    construct.addAttribute(STL::passengerTypeCode, ptp->paxType()->requestedPaxType());
  if (!OBFeeOptionMaxLimitFound())
  {
    if (!ppi->fopVector().empty())
    {
      TktFeesRequest::FormOfPayment* fop = ppi->fopVector()[0];
      if (fop)
      {
        FopBinNumber& fbn = fop->fopBinNumber();
        if (fbn.size() == 6 &&
            (std::find_if(fbn.begin(), fbn.end(), !boost::bind<bool>(&isDigit, _1)) == fbn.end()))
        {
          construct.addAttributeDouble(STL::totalPriceAmount, ptp->amount(), ptp->noDec());
          construct.addAttributeDouble(
              STL::totalAmountWithOBFee, ptp->amount() + _tktFeesAmount, ptp->noDec());
        }
      }
    }
  }
  construct.addAttribute(STL::paymentCurrency, ptp->currency());

  LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::processPaxType() - complete");
}

void
STLTktFeesPricingResponseFormatter::processPassengerIdentity(TktFeesPricingTrx& pricingTrx,
                                                             XMLConstruct& construct,
                                                             SequenceNumber sn)
{
  LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::processPassengerIdentity() - entered");
  TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(pricingTrx.getRequest());

  std::vector<TktFeesRequest::PassengerIdentity*>& paxIdV = tktFeesReq->paxId();
  for (TktFeesRequest::PassengerIdentity* pId : paxIdV)
  {
    if (pId->objectId() == sn)
    {
      construct.openElement(STL::PassengerIdentity);
      construct.addAttributeShort(STL::firstNameNumber, pId->firstNameNumber());
      construct.addAttributeShort(STL::surNameNumber, pId->surNameNumber());
      construct.addAttributeShort(STL::pnrNameNumber, pId->pnrNameNumber());
      construct.closeElement();
    }
  }
  LOG4CXX_DEBUG(logger,
                "STLTktFeesPricingResponseFormatter::processPassengerIdentity() - complete");
}

void
STLTktFeesPricingResponseFormatter::processTktFeesSolution(
    TktFeesPricingTrx& pricingTrx,
    const Itin* itin,
    XMLConstruct& construct,
    const FarePath& fp,
    const TktFeesRequest::PaxTypePayment* ptp)
{
  LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::processTktFeesSolution() - entered");
  if (fp.maximumObFee() && EPSILON < fp.maximumObFee()->feeAmount())
  {
    const TicketingFeesInfo* feeInfo = fp.maximumObFee();
    prepareOBFee(pricingTrx, construct, feeInfo, itin);
    construct.addAttribute(STL::showNoOBFeeInd, "true");
    construct.closeElement();
    return; //  display MAX item only and do not display anything else when > 50 OBFees qualified
  }
  if (pricingTrx.isProcess2CC() && 2 == fp.collectedTktOBFees().size())
  {
    TicketingFeesInfo* firstFeeInfo = fp.collectedTktOBFees().front();
    TicketingFeesInfo* secondFeeInfo = fp.collectedTktOBFees().back();
    MoneyAmount secondCardChargeAmount =
        std::min(pricingTrx.getRequest()->paymentAmountFop(), ptp->amount());
    MoneyAmount firstCardChargeAmount = ptp->amount() - secondCardChargeAmount;
    if (firstCardChargeAmount < EPSILON)
      firstCardChargeAmount = 0.0;

    if (secondCardChargeAmount < EPSILON)
      secondCardChargeAmount = 0.0;

    if (firstFeeInfo)
      prepareOBFee(pricingTrx, construct, firstFeeInfo, itin, firstCardChargeAmount);
    else
      construct.openElement(STL::TicketingFee);

    construct.addAttribute(STL::requestedBinNumber, pricingTrx.getRequest()->formOfPayment());
    construct.addAttributeDouble(STL::chargeAmount, firstCardChargeAmount, ptp->noDec());
    construct.closeElement();

    if (secondFeeInfo)
      prepareOBFee(pricingTrx, construct, secondFeeInfo, itin, secondCardChargeAmount);
    else
      construct.openElement(STL::TicketingFee);

    construct.addAttribute(STL::requestedBinNumber, pricingTrx.getRequest()->secondFormOfPayment());
    construct.addAttributeDouble(STL::chargeAmount, secondCardChargeAmount, ptp->noDec());
    construct.closeElement();
  }
  else
  {
    for (const auto fee : fp.collectedTktOBFees())
    {
      prepareOBFee(pricingTrx, construct, fee, itin);
      construct.closeElement();
    }
  }
  LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::processTktFeesSolution() - complete");
}

void
STLTktFeesPricingResponseFormatter::prepareHostPortInfo(PricingTrx& trx, XMLConstruct& construct)
{
  LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::prepareHostPortInfo() - entered");
  std::vector<std::string> hostInfo;
  std::vector<std::string> buildInfo;
  std::vector<std::string> dbInfo;
  std::vector<std::string> configInfo;

  if (hostDiagString(hostInfo))
  {
    for (const auto& elem : hostInfo)
      prepareResponseText(elem, construct);
  }

  buildDiagString(buildInfo);
  for (const auto& elem : buildInfo)
    prepareResponseText(elem, construct);

  dbDiagString(dbInfo);
  for (const auto& elem : dbInfo)
    prepareResponseText(elem, construct);

  if (configDiagString(configInfo, trx))
  {
    for (const auto& elem : configInfo)
      prepareResponseText(elem, construct);
  }
  LOG4CXX_DEBUG(logger, "STLTktFeesPricingResponseFormatter::prepareHostPortInfo() - complete");
}

CurrencyCode&
STLTktFeesPricingResponseFormatter::getCurrencyCode(PricingTrx& pricingTrx, const Itin& currentItin)
    const
{
  TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(pricingTrx.getRequest());
  const Itin* pCurrentItin = &currentItin;
  std::map<const Itin*, TktFeesRequest::PaxTypePayment*>::const_iterator ptpIter =
      tktFeesReq->paxTypePaymentPerItin().find(pCurrentItin);
  if (ptpIter == tktFeesReq->paxTypePaymentPerItin().end())
    throw std::runtime_error("Null pointer to PTP data");
  TktFeesRequest::PaxTypePayment* p = ptpIter->second;
  if (p == nullptr)
    throw std::runtime_error("Null pointer to PTP data");
  return p->currency();
}

void
STLTktFeesPricingResponseFormatter::prepareOBFee(PricingTrx& pricingTrx,
                                                 XMLConstruct& construct,
                                                 const TicketingFeesInfo* feeInfo,
                                                 const Itin* currentItin,
                                                 const MoneyAmount& chargeAmt)
{
  TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(pricingTrx.getRequest());
  std::map<const Itin*, TktFeesRequest::PaxTypePayment*>::const_iterator ptpIter =
      tktFeesReq->paxTypePaymentPerItin().find(currentItin);

  if (ptpIter == tktFeesReq->paxTypePaymentPerItin().end())
    throw std::runtime_error("Null pointer to PTP data");
  TktFeesRequest::PaxTypePayment* p = ptpIter->second;
  if (p == nullptr)
    throw std::runtime_error("Null pointer to PTP data");
  CurrencyCode& paymentCurrencyCode = p->currency();

  construct.openElement(STL::TicketingFee);

  MoneyAmount totalFeeFareAmount;
  construct.addAttribute(STL::typeCode, feeInfo->serviceTypeCode() + feeInfo->serviceSubTypeCode());

  if (feeInfo->feePercent() > 0 && !_OBFeeOptionMaxLimitFound)
  {
    if (chargeAmt != INVALID_AMT)
    {
      calculateObFeeAmountFromPercentage(
          pricingTrx, construct, feeInfo, totalFeeFareAmount, paymentCurrencyCode, chargeAmt);
    }
    else
    {
      MoneyAmount chargeAmount = 0;
      bool rcode = getChargeAmount(p, chargeAmount);
      if (rcode)
        calculateObFeeAmountFromPercentage(pricingTrx,
                                           construct,
                                           feeInfo,
                                           totalFeeFareAmount,
                                           paymentCurrencyCode,
                                           chargeAmount);
    }
  }
  else
  {
    calculateObFeeAmountFromAmount(
        pricingTrx, construct, feeInfo, totalFeeFareAmount, paymentCurrencyCode, chargeAmt);
  }
  _tktFeesAmount += totalFeeFareAmount;

  construct.addAttribute(STL::binNumber, feeInfo->fopBinNumber());
  std::ostringstream combineITATAind;
  combineITATAind << feeInfo->refundReissue() << feeInfo->commission() << feeInfo->interline();
  construct.addAttribute(STL::iataIndicators, combineITATAind.str());
  construct.addAttributeChar(STL::noChargeInd, feeInfo->noCharge());
  if (feeInfo->feePercent() > 0.0)
  {
    construct.addAttributeDouble(
        STL::serviceFeePercentage, feeInfo->feePercent(), feeInfo->feePercentNoDec());
    MoneyAmount maxAmount = feeInfo->maxFeeAmount();
    const Money sourceMoney(maxAmount, feeInfo->maxFeeCur());
    Money targetMoney(paymentCurrencyCode);
    convertOBFeeCurrency(pricingTrx, sourceMoney, targetMoney);
    maxAmount = targetMoney.value();
    construct.addAttributeDouble(STL::maxAmount, maxAmount, p->noDec());
  }
}

bool
STLTktFeesPricingResponseFormatter::getChargeAmount(const TktFeesRequest::PaxTypePayment* ptp,
                                                    MoneyAmount& chargeAmount) const
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

void
STLTktFeesPricingResponseFormatter::calculateObFeeAmountFromAmount(
    PricingTrx& pricingTrx,
    XMLConstruct& construct,
    const TicketingFeesInfo* feeInfo,
    MoneyAmount& totalFeeFareAmount,
    const CurrencyCode& equivCurrencyCode,
    const MoneyAmount& chargeAmount)
{
  if (chargeAmount == 0.0)
  {
    totalFeeFareAmount = 0.0;
    construct.addAttributeDouble(STL::feeAmount, totalFeeFareAmount, feeInfo->noDec());
  }
  else if (feeInfo->feeAmount() < 0.0 || (feeInfo->cur() == equivCurrencyCode))
  {
    construct.addAttributeDouble(STL::feeAmount, feeInfo->feeAmount(), feeInfo->noDec());
    totalFeeFareAmount = feeInfo->feeAmount();
  }
  else
  {
    const DateTime& ticketingDate = pricingTrx.ticketingDate();
    Money targetMoney = convertOBFeeCurrencyByCode(pricingTrx, equivCurrencyCode, feeInfo);
    construct.addAttributeDouble(
        STL::feeAmount, targetMoney.value(), targetMoney.noDec(ticketingDate));
    totalFeeFareAmount = targetMoney.value();
  }
}

void
STLTktFeesPricingResponseFormatter::calculateObFeeAmountFromAmountMax(
    PricingTrx& pricingTrx,
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

void
STLTktFeesPricingResponseFormatter::calculateObFeeAmountFromPercentageMax(
    PricingTrx& pricingTrx,
    const TicketingFeesInfo* feeInfo,
    MoneyAmount& totalFeeFareAmount,
    const CurrencyCode& paymentCurrency,
    const MoneyAmount& chargeAmount)
{
  MoneyAmount calcAmount = (chargeAmount * feeInfo->feePercent()) / 100.0f;
  Money targetMoneyC(calcAmount, paymentCurrency);
  PricingResponseFormatter::roundOBFeeCurrency(pricingTrx, targetMoneyC);
  calcAmount = targetMoneyC.value();

  MoneyAmount maxAmount = feeInfo->maxFeeAmount();
  const Money sourceMoney(maxAmount, feeInfo->maxFeeCur());
  Money targetMoney(paymentCurrency);

  convertOBFeeCurrency(pricingTrx, sourceMoney, targetMoney);
  maxAmount = targetMoney.value();

  MoneyAmount lowestCalcAmount =
      PricingResponseFormatter::getLowestObFeeAmount(feeInfo->maxFeeCur(), calcAmount, maxAmount);
  totalFeeFareAmount = lowestCalcAmount;
}

void
STLTktFeesPricingResponseFormatter::calculateObFeeAmountFromPercentage(
    PricingTrx& pricingTrx,
    XMLConstruct& construct,
    const TicketingFeesInfo* feeInfo,
    MoneyAmount& totalFeeFareAmount,
    const CurrencyCode& equivCurrencyCode,
    const MoneyAmount& chargeAmount)
{
  MoneyAmount calcAmount = (chargeAmount * feeInfo->feePercent()) / 100.0f;
  Money targetMoneyC(calcAmount, equivCurrencyCode);
  PricingResponseFormatter::roundOBFeeCurrency(pricingTrx, targetMoneyC);
  calcAmount = targetMoneyC.value();

  MoneyAmount maxAmount = feeInfo->maxFeeAmount();
  const Money sourceMoney(maxAmount, feeInfo->maxFeeCur());
  Money targetMoney(equivCurrencyCode);

  convertOBFeeCurrency(pricingTrx, sourceMoney, targetMoney); // rounding
  maxAmount = targetMoney.value();

  MoneyAmount lowestCalcAmount =
      PricingResponseFormatter::getLowestObFeeAmount(feeInfo->maxFeeCur(), calcAmount, maxAmount);
  construct.addAttributeDouble(
      STL::feeAmount, lowestCalcAmount, targetMoney.noDec(pricingTrx.ticketingDate()));
  totalFeeFareAmount = lowestCalcAmount;
}

Money
STLTktFeesPricingResponseFormatter::convertOBFeeCurrencyByCode(
    PricingTrx& pricingTrx, const CurrencyCode& equivCurrencyCode, const TicketingFeesInfo* feeInfo)
{
  const Money sourceMoney(feeInfo->feeAmount(), feeInfo->cur());
  Money targetMoney(equivCurrencyCode);
  convertOBFeeCurrencyByMoney(pricingTrx, sourceMoney, targetMoney);
  return targetMoney;
}

void
STLTktFeesPricingResponseFormatter::convertOBFeeCurrencyByMoney(PricingTrx& pricingTrx,
                                                                const Money& sourceMoney,
                                                                Money& targetMoney)
{
  CurrencyConversionFacade converter;
  converter.convert(targetMoney, sourceMoney, pricingTrx, false, CurrencyConversionRequest::TAXES);
  RoundingFactor roundingFactor = 0;
  CurrencyNoDec roundingNoDec = 0;
  RoundingRule roundingRule = NONE;
  if (PricingResponseFormatter::getFeeRounding(
          pricingTrx, targetMoney.code(), roundingFactor, roundingNoDec, roundingRule))
  {
    CurrencyConverter curConverter;
    curConverter.round(targetMoney, roundingFactor, roundingRule);
  }
}

void
STLTktFeesPricingResponseFormatter::checkLimitOBFees(PricingTrx& pricingTrx)
{
  if (!pricingTrx.getRequest()->isCollectOBFee())
    return;

  uint32_t maxOBFeesOptions = TrxUtil::getConfigOBFeeOptionMaxLimit();
  bool OBFeeOptionMaxLimitFound = false;
  std::pair<const TicketingFeesInfo*, CurrencyCode> maxItem;
  const FarePath* lastOnePTC = nullptr;
  for (const auto elem : pricingTrx.itin())
  {
    if (elem == nullptr || (elem->errResponseCode() != ErrorResponseException::NO_ERROR) ||
        (elem->farePath().empty()))
      continue;
    for (const auto pFarePath : elem->farePath())
    {
      if (pFarePath == nullptr)
        continue;
      if (pFarePath->collectedTktOBFees().size() > maxOBFeesOptions)
      {
        OBFeeOptionMaxLimitFound = true;
        break;
      }
    }
  }
  if (!OBFeeOptionMaxLimitFound)
    return;

  std::pair<const TicketingFeesInfo*, MoneyAmount> maximum(nullptr, 0.0);
  CurrencyCode paymentCurrency; // from Max solution
  for (std::vector<Itin*>::iterator itinI = pricingTrx.itin().begin(),
                                    itinEnd = pricingTrx.itin().end();
       itinI != itinEnd;
       ++itinI)
  {
    if ((*itinI) == nullptr || ((*itinI)->errResponseCode() != ErrorResponseException::NO_ERROR) ||
        ((*itinI)->farePath().empty()))
      continue;
    for (const auto fPath : (*itinI)->farePath())
    {
      if (fPath == nullptr)
        continue;
      lastOnePTC = fPath;
      std::pair<const TicketingFeesInfo*, MoneyAmount> maxForPax(nullptr, 0.0);
      bool isZeroMax = false;
      if (maxOBFeesOptions < fPath->collectedTktOBFees().size())
        isZeroMax = checkForZeroMaximum(pricingTrx, *fPath);
      if (isZeroMax && fPath->collectedTktOBFees().empty())
        continue;
      if (!isZeroMax)
      {
        maxForPax = computeMaximumOBFeesPercent(pricingTrx, *fPath, *itinI);
        if (maxForPax.first)
        {
          if (maximum.second < maxForPax.second)
          {
            maximum.first = maxForPax.first;
            maximum.second = maxForPax.second;
            paymentCurrency = getPaymentCurrency(pricingTrx, *itinI);
            _itin = *itinI;
          }
        }
      }
    }
  }
  if (!maximum.first)
    return;
  clearAllFees(pricingTrx);

  const_cast<FarePath*>(lastOnePTC)->maximumObFee() =
      mockOBFeeInPaymentCurrency(pricingTrx, *(maximum.first), maximum.second, paymentCurrency);
  _OBFeeOptionMaxLimitFound = true;
}

void
STLTktFeesPricingResponseFormatter::clearAllFees(PricingTrx& pricingTrx) const
{
  for (const auto elem : pricingTrx.itin())
  {
    if (elem == nullptr || elem->farePath().empty())
      continue;
    for (const auto fPath : elem->farePath())
    {
      if (fPath == nullptr)
        continue;
      fPath->collectedTktOBFees().clear();
    }
  }
}

bool
STLTktFeesPricingResponseFormatter::checkForZeroMaximum(PricingTrx& pricingTrx, FarePath& farePath)
    const
{
  const std::vector<TicketingFeesInfo*>::const_iterator feesB =
      farePath.collectedTktOBFees().begin();
  const std::vector<TicketingFeesInfo*>::const_iterator feesE = farePath.collectedTktOBFees().end();
  if (find_if(feesB, feesE, NonZeroAmount()) != feesE)
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
STLTktFeesPricingResponseFormatter::computeMaximumOBFeesPercent(PricingTrx& pricingTrx,
                                                                FarePath& farePath,
                                                                Itin* itin)
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

  std::vector<TicketingFeesInfo*>::const_iterator feesBegin = farePath.collectedTktOBFees().begin();
  std::vector<TicketingFeesInfo*>::const_iterator feesEnd = farePath.collectedTktOBFees().end();

  for (; feesBegin != feesEnd; ++feesBegin)
  {
    const TicketingFeesInfo* feeInfo = *feesBegin;
    MoneyAmount feeAmt = 0.0;
    if (feeInfo->feePercent() > 0)
    {
      MoneyAmount chargeAmount = 0;
      bool rcode = getChargeAmount(p, chargeAmount);
      if (rcode)
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
    maximumForPax.first = *feesBegin;
  return maximumForPax;
}

CurrencyCode
STLTktFeesPricingResponseFormatter::getPaymentCurrency(PricingTrx& pricingTrx, const Itin* itin)
{
  CurrencyCode cc;
  TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(pricingTrx.getRequest());
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

} // namespace
