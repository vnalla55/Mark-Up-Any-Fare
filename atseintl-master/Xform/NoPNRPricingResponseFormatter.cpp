//----------------------------------------------------------------------------
//
//  File:  NoPNRPricingResponseFormatter.cpp
//  Description: See NoPNRPricingResponseFormatter.h file
//  Created:  January 30, 2008
//  Authors:  Marcin Augustyniak
//
//  Copyright Sabre 2005
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

#include "Xform/NoPNRPricingResponseFormatter.h"

#include "Common/FareCalcUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/XMLConstruct.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/NoPNRFareCalcCollector.h"
#include "Util/Base64.h"
#include "Util/CompressUtil.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/XformUtil.h"

#include <map>
#include <regex>

namespace tse
{

log4cxx::LoggerPtr
NoPNRPricingResponseFormatter::_logger(
    log4cxx::Logger::getLogger("atseintl.Xform.NoPNRPricingResponseFormatter"));
log4cxx::LoggerPtr
NoPNRPricingResponseFormatter::_uncompressedLogger(
    log4cxx::Logger::getLogger("atseintl.Xform.NoPNRPricingResponseFormatterUncompressed"));

void NoPNRPricingResponseFormatter::formatResponse(const ErrorResponseException& ere, std::string& response)
{
  XMLConstruct construct;
  construct.openElement("NoPNRPricingResponse");

  ResponseFormatter::addMessageLine(ere.message(), construct, "E", Message::errCode(ere.code()));

  construct.closeElement();

  response = construct.getXMLData();

}

std::string
NoPNRPricingResponseFormatter::formatResponse(const std::string& responseString,
                                              bool displayOnly,
                                              PricingTrx& pricingTrx,
                                              FareCalcCollector* fareCalcCollector,
                                              ErrorResponseException::ErrorResponseCode tktErrCode)
{
  LOG4CXX_INFO(_logger, "Response (before XML tagging):\n" << responseString);

  XMLConstruct construct;

  // check for diagnostic or normal response
  if (fareCalcCollector)
  {
    // normal response has a PricingResponse compressed and embedded in an NoPNRPricingResponse
    construct.openElement("PricingResponse");
    prepareAgent(pricingTrx, construct);
    prepareBilling(pricingTrx, *fareCalcCollector, construct);
    preparePassengers(pricingTrx, *fareCalcCollector, construct);
  }
  else
  {
    // diagnostics are an NoPNRPricingResponse with MSG elements
    construct.openElement("NoPNRPricingResponse");
  }

  if (pricingTrx.diagnostic().diagnosticType() == Diagnostic854)
  {
    prepareHostPortInfo(pricingTrx, construct);
  }

  bool noSizeLImit = false;

  if (pricingTrx.billing() != nullptr && pricingTrx.billing()->requestPath() == SWS_PO_ATSE_PATH)
    noSizeLImit = true;

  // create MSG elements the same for diagnostics or normal responses
  if (tktErrCode > 0)
  {
    // Pricing failed, return error message
    prepareErrorMessage(pricingTrx, construct, tktErrCode, responseString);

    Diagnostic& diag = pricingTrx.diagnostic();
    if (diag.diagnosticType() != DiagnosticNone && !diag.toString().empty())
    {
      prepareResponseText(diag.toString(), construct, noSizeLImit);
    }
  }
  else
  {
    // Attaching MSG elements
    if (pricingTrx.billing()->requestPath() != SWS_PO_ATSE_PATH ||
        pricingTrx.diagnostic().diagnosticType() != DiagnosticNone)
      prepareResponseText(responseString, construct, noSizeLImit);
  }

  construct.closeElement();

  LOG4CXX_INFO(_uncompressedLogger,
               "Uncompressed PricingResponse in XML " << construct.getXMLData().size()
                                                      << " bytes:\n" << construct.getXMLData());
  LOG4CXX_INFO(_logger,
               "Uncompressed PricingResponse size " << construct.getXMLData().size() << " bytes");

  // do compression and embedding only for non-diagnostics
  if (fareCalcCollector)
  {
    std::string pricingResponse = construct.getXMLData();

    XMLConstruct noPNRConstruct;
    noPNRConstruct.openElement("NoPNRPricingResponse");
    noPNRConstruct.openElement("DTS");
    noPNRConstruct.addAttributeInteger("Q0S", numOfValidCalcTotals(pricingTrx, *fareCalcCollector));

    bool isLowFare = pricingTrx.getRequest()->isLowFareRequested();
    noPNRConstruct.addAttributeBoolean("PBV", isLowFare);

    if (pricingTrx.billing()->requestPath() == SWS_PO_ATSE_PATH)
    {
      noPNRConstruct.addElementData(pricingResponse.c_str());
    }
    else
    {
      CompressUtil::compressBz2(pricingResponse);
      std::string encodedPricingResponse = Base64::encode(pricingResponse);
      noPNRConstruct.addElementData(encodedPricingResponse.c_str());
    }
    noPNRConstruct.closeElement(); // DTS

    if (pricingTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
      prepareResponseText(responseString, noPNRConstruct, noSizeLImit);

    noPNRConstruct.closeElement(); // NoPNRPricingResponse

    LOG4CXX_INFO(_logger,
                 "Compressed PricingResponse in XML " << noPNRConstruct.getXMLData().size()
                                                      << " bytes:\n"
                                                      << noPNRConstruct.getXMLData());

    return noPNRConstruct.getXMLData();
  }
  else
  {
    return construct.getXMLData();
  }
}

void
NoPNRPricingResponseFormatter::scanTotalsItin(CalcTotals& calcTotals,
                                              bool& fpFound,
                                              bool& infantMessage,
                                              char& nonRefundable,
                                              MoneyAmount& moneyAmountAbsorbtion)
{
  fpFound = true;
  scanFarePath(*calcTotals.farePath, infantMessage, nonRefundable, moneyAmountAbsorbtion);
}

size_t
NoPNRPricingResponseFormatter::numValidCalcTotals(FareCalcCollector& fareCalcCollector)
{
  size_t count = 0;

  FareCalcCollector::CalcTotalsMap::const_iterator totalsIter =
      fareCalcCollector.calcTotalsMap().begin();
  FareCalcCollector::CalcTotalsMap::const_iterator totalsIterEnd =
      fareCalcCollector.calcTotalsMap().end();
  for (; totalsIter != totalsIterEnd; totalsIter++)
  {
    CalcTotals& calcTotals = *totalsIter->second;
    if (calcTotals.wpaInfo.psgDetailRefNo != 0 && !calcTotals.wpaInfo.wpnDetailResponse.empty())
      count++;
  }
  return count;
}

void
NoPNRPricingResponseFormatter::preparePassengers(PricingTrx& pricingTrx,
                                                 FareCalcCollector& fareCalcCollector,
                                                 XMLConstruct& construct)
{
  const FareCalcConfig& fcConfig = *(FareCalcUtil::getFareCalcConfig(pricingTrx));
  const DateTime& ticketingDate = pricingTrx.ticketingDate();

  uint16_t numPassengers = 0;

  numPassengers = numOfValidCalcTotals(pricingTrx, fareCalcCollector);

  CalcTotals* calcTotalsTemp = nullptr;

  for (uint16_t paxNumber = 1; paxNumber <= numPassengers; ++paxNumber)
  {
    bool paxFound = false;
    FareCalcCollector::CalcTotalsMap::const_iterator totalsIter =
        fareCalcCollector.calcTotalsMap().begin();
    FareCalcCollector::CalcTotalsMap::const_iterator totalsIterEnd =
        fareCalcCollector.calcTotalsMap().end();
    for (; totalsIter != totalsIterEnd; totalsIter++)
    {
      if (pricingTrx.getOptions() && !pricingTrx.getOptions()->isPDOForFRRule() &&
          totalsIter->second->adjustedCalcTotal != nullptr &&
          !totalsIter->second->adjustedCalcTotal->wpaInfo.wpnDetailResponse.empty() &&
          totalsIter->second->adjustedCalcTotal->wpaInfo.psgDetailRefNo == paxNumber)
      {
        calcTotalsTemp = totalsIter->second->adjustedCalcTotal;
        paxFound = true;
        break;
      }
      else
      {
        if (totalsIter->second != nullptr && !totalsIter->second->wpaInfo.wpnDetailResponse.empty() &&
            totalsIter->second->wpaInfo.psgDetailRefNo == paxNumber)
        {
          calcTotalsTemp = totalsIter->second;
          paxFound = true;
          break;
        }
      }
    }
    if (!paxFound)
      continue;

    CalcTotals& calcTotals = *calcTotalsTemp;

    construct.openElement(xml2::SummaryInfo);

    Money money(calcTotals.convertedBaseFareCurrencyCode);
    construct.addAttributeDouble(
        xml2::TotalPriceAll, calcTotals.convertedBaseFare, money.noDec(ticketingDate));
    construct.addAttribute(xml2::TotalCurrencyCode, calcTotals.convertedBaseFareCurrencyCode);
    if (calcTotals.netRemitCalcTotals != nullptr)
    {
      construct.addAttributeDouble(xml2::NetRemitBaseFareAmount,
                                   calcTotals.netRemitCalcTotals->convertedBaseFare,
                                   money.noDec(ticketingDate));
    }

    prepareCommonSummaryAttrs(pricingTrx, fareCalcCollector, construct, totalsIter->second);

    preparePassengerInfo(pricingTrx, fcConfig, calcTotals, paxNumber, construct);

    MAFUtil::checkElementONV(pricingTrx, &calcTotals, construct);

    construct.closeElement();
  }
}

void
NoPNRPricingResponseFormatter::addAdditionalPaxInfo(PricingTrx& pricingTrx,
                                                    CalcTotals& calcTotals,
                                                    uint16_t paxNumber,
                                                    XMLConstruct& construct)
{
  construct.addAttribute(xml2::RequestedPassengerType, calcTotals.requestedPaxType);
  construct.addAttributeInteger(xml2::WpnOptionNumber, calcTotals.wpaInfo.psgDetailRefNo);

  if (pricingTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
  {
    std::string wpnDetails = std::regex_replace(calcTotals.wpaInfo.wpnDetailResponse, std::regex("\n"), "\\n");
    construct.addAttribute(xml2::WpnDetails, wpnDetails);
  }
}

void
NoPNRPricingResponseFormatter::prepareFarePath(PricingTrx& pricingTrx,
                                               CalcTotals& calcTotals,
                                               const CurrencyNoDec& noDec1,
                                               const CurrencyNoDec& noDec2,
                                               uint16_t paxNumber,
                                               bool stopoverFlag,
                                               const FuFcIdMap& fuFcIdCol,
                                               XMLConstruct& construct)
{
  traverseTravelSegs(pricingTrx,
                     calcTotals,
                     noDec1,
                     noDec2,
                     *calcTotals.farePath,
                     paxNumber,
                     stopoverFlag,
                     fuFcIdCol,
                     construct);
}
}
