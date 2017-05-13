//----------------------------------------------------------------------------
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
#include "Xform/XMLConvertUtils.h"

#include "Common/Logger.h"
#include "Common/Message.h"
#include "Common/OBFeesUtils.h"
#include "Common/XMLConstruct.h"
#include "DataModel/AltPricingDetailTrx.h"
#include "DataModel/AltPricingDetailObFeesTrx.h"
#include "DataModel/AltPricingTrxData.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/RexBaseTrx.h"
#include "DBAccess/NoPNROptions.h"
#include "Diagnostic/DiagTools.h"
#include "FareCalc/FareCalcConsts.h"
#include "Xform/PricingResponseFormatter.h"

#include <string>
#include <cstring>

namespace tse
{
static Logger
logger("atseintl.Xform.XMLConvertUtils");

uint32_t XMLConvertUtils::_maxTotalBuffSize;

std::string
XMLConvertUtils::formatWtfrResponse(AltPricingTrx* altTrx)
{
  XMLConstruct construct;
  construct.openElement("ValidationResponse");

  AltPricingTrx::AccompRestrictionVec::iterator arIter = altTrx->accompRestrictionVec().begin();
  AltPricingTrx::AccompRestrictionVec::iterator arIterEnd = altTrx->accompRestrictionVec().end();
  for (; arIter != arIterEnd; arIter++)
  {
    construct.openElement(xml2::ValidationResult);
    construct.addAttributeInteger(xml2::WpnOptionNumber, arIter->selectionNumber());
    construct.addAttributeBoolean(xml2::TicketGuaranteed, arIter->guaranteed());
    construct.closeElement();
  }

  construct.closeElement();
  return construct.getXMLData();
}

std::string
XMLConvertUtils::formatWpaWtfrResponse(AltPricingTrx* altTrx)
{
  const char* const searchTag = "<SUM ";
  const size_t searchTagLen = strlen(searchTag);
  std::string response("<MultiPricingResponse>");

  AltPricingTrx::AccompRestrictionVec::iterator arIter = altTrx->accompRestrictionVec().begin();
  AltPricingTrx::AccompRestrictionVec::iterator arIterEnd = altTrx->accompRestrictionVec().end();

  if (std::any_of(arIter, arIterEnd, predicate::SurfacePredicate()))
    return formatWpanWtfrErrorResponse();

  response += altTrx->agentXml();
  response += altTrx->billingXml();

  for (; arIter != arIterEnd; arIter++)
  {
    std::string attrStr(xml2::TicketGuaranteed);
    attrStr += "=\"";
    attrStr += (arIter->guaranteed() ? "T" : "F");
    attrStr += "\" ";

    std::string& selXml = arIter->selectionXml();
    size_t idx = selXml.find(searchTag);
    if (idx == std::string::npos)
    {
      LOG4CXX_WARN(logger, "invalid selection XML");
      continue;
    }
    selXml.insert(idx + searchTagLen, attrStr);
    response += selXml;
  }

  response += "</MultiPricingResponse>";
  return response;
}

std::string
XMLConvertUtils::formatWpanWtfrErrorResponse()
{
  std::string errorText = "ISSUE SEPARATE TICKETS-INTL SURFACE RESTRICTED\n"
                          "TICKETING NOT PERMITTED\n"
                          " ";

  std::string response("<PricingResponse>");
  formatResponse(errorText, response, 2, "E");
  response += "</PricingResponse>";

  return response;
}
int
XMLConvertUtils::formatResponse(std::string& tmpResponse,
                                std::string& xmlResponse,
                                const std::string& msgType)
{
  return formatResponse(tmpResponse, xmlResponse, 2, msgType);
}

int
XMLConvertUtils::formatResponse(std::string& tmpResponse,
                                std::string& xmlResponse,
                                int recNum,
                                const std::string& msgType,
                                PricingDetailTrx* trx)
{
  LOG4CXX_INFO(logger, "Response (before XML tagging):\n" << tmpResponse);

  size_t totalBuffSize = 0;
  const int BUF_SIZE = 256;
  const int AVAILABLE_SIZE = BUF_SIZE - 52;
  char tmpBuf[BUF_SIZE];

  // Clobber the trailing newline
  while (*(tmpResponse.rbegin()) == '\n')
  {
    tmpResponse.erase(tmpResponse.size() - 1);
  }
  char* pHolder = nullptr;
  size_t tokenLen = 0;
  for (char* token = strtok_r((char*)tmpResponse.c_str(), "\n", &pHolder); token != nullptr;
       token = strtok_r(nullptr, "\n", &pHolder), recNum++)
  {
    tokenLen = strlen(token);
    if (tokenLen == 0)
      continue;
    else if (tokenLen > AVAILABLE_SIZE)
    {
      LOG4CXX_WARN(logger, "Line: [" << token << "] too long!");
      continue;
    }

    if (skipServiceFeeTemplate(trx, token))
    {
      recNum--;
      continue;
    }

    sprintf(tmpBuf, "<MSG N06=\"%s\" Q0K=\"%06d\" S18=\"%s\"/>", msgType.c_str(), recNum, token);

    // if the output buffer is too big for PSS overwrite the last line with error message and get
    // out
    totalBuffSize += strlen(tmpBuf);
    if (totalBuffSize > _maxTotalBuffSize)
    {
      sprintf(tmpBuf,
              "<MSG N06=\"%s\" Q0K=\"%06d\" S18=\"RESPONSE TOO LONG FOR CRT\"/>",
              msgType.c_str(),
              recNum + 1);
      xmlResponse.append(tmpBuf);
      break;
    }
    if (recNum == 1)
      xmlResponse = xmlResponse.insert(0, tmpBuf);
    else
      xmlResponse.append(tmpBuf);
  }
  LOG4CXX_INFO(logger, "Response (after XML tagging):\n" << xmlResponse);
  return recNum;
}

bool
XMLConvertUtils::skipServiceFeeTemplate(PricingDetailTrx* trx, const char* msg)
{
  if (!trx || !trx->wpnTrx())
    return false;

  return std::strstr(msg, FareCalcConsts::SERVICE_FEE_AMOUNT_LINE.c_str()) ||
         std::strstr(msg, FareCalcConsts::SERVICE_FEE_TOTAL_LINE.c_str());
}

std::string
XMLConvertUtils::prepareResponseText(const std::string& responseString, bool noSizeLimit)
{
  XMLConstruct construct;

  int recNum = 3;
  const int AVAILABLE_SIZE = 256;

  std::istringstream iss(responseString);
  std::string line;

  for (; std::getline(iss, line); recNum++)
  {
    if (line.length() > AVAILABLE_SIZE && !noSizeLimit)
    {
      const char* msg = "LINE TOO LONG";
      OBFeesUtils::prepareMessage(construct, Message::TYPE_GENERAL, recNum, msg);
      continue;
    }

    OBFeesUtils::prepareMessage(construct, Message::TYPE_GENERAL, recNum, line);

    // limit the size of the output returned
    if (construct.getXMLData().size() > _maxTotalBuffSize && !noSizeLimit)
    {
      const char* msg = "RESPONSE TOO LONG FOR CRT";
      OBFeesUtils::prepareMessage(construct, Message::TYPE_GENERAL, recNum + 1, msg);
      break;
    }
  }
  return construct.getXMLData();
}
std::string
XMLConvertUtils::printDiagReturnedNoData(const DiagnosticTypes number)
{
  std::ostringstream o;
  o << "DIAGNOSTIC " << number << " RETURNED NO DATA";
  return o.str();
}
void
XMLConvertUtils::wrapWpnRespWitnMainTag(std::string& response)
{
  response = "<PricingResponse>" + response + "</PricingResponse>";
}

template <>
std::string
XMLConvertUtils::formatWpanDetailResponse(AltPricingDetailObFeesTrx* altTrx)
{
  std::ostringstream stream;
  std::string xmlResponse;
  int recNumber = 2;

  if (std::any_of(altTrx->accompRestrictionVec().begin(),
                  altTrx->accompRestrictionVec().end(),
                  predicate::SurfacePredicate()))
    return formatWpanWtfrErrorResponse();

  if (altTrx->getRequest()->isCollectOBFee())
    OBFeesUtils::addObFeeInfoWpan(*altTrx);

  std::string response("<PricingResponse>");
  std::string psgSum = getWpanDetailPsgSum(altTrx);

  if (psgSum.find("S66=\"") != std::string::npos)
  {
    response += altTrx->agentXml();
    response += altTrx->billingXml();
    response += psgSum;

    if (altTrx->getOptions()->isRecordQuote())
    {
      std::string tmp = "PRICE QUOTE RECORD RETAINED\n \n";
      formatResponse(tmp, xmlResponse, recNumber);
    }
  }

  for (const auto paxDetail : altTrx->paxDetails())
  {
    std::string singleResponse;

    const char* const searchBookCodes = "APPLICABLE BOOKING CLASS - ";
    std::string wpnDetails(paxDetail->wpnDetails());
    size_t loc = 0;
    while ((loc = wpnDetails.find("\\n", loc)) != std::string::npos)
    {
      size_t next = wpnDetails.find(searchBookCodes);
      if (altTrx->rebook() && (next != std::string::npos))
      {
        size_t diff = (next - loc);
        if ((diff == 2) || ((diff == 7) && (altTrx->vendorCrsCode() == ABACUS_MULTIHOST_ID)))
        {
          wpnDetails.replace(loc, 2, "\n");
          size_t size = wpnDetails.size();
          next = wpnDetails.find_last_of("\\n", size);

          wpnDetails = wpnDetails.erase(loc + 1, (next + 3 - loc + 1));
          continue;
        }
      }
      wpnDetails.replace(loc, 2, "\n");
    }

    singleResponse += wpnDetails + "\n";
    if (!altTrx->rebook() || (paxDetail != altTrx->paxDetails().back()))
      singleResponse += " \n";

    recNumber = formatResponse(singleResponse, xmlResponse, recNumber);
    xmlResponse += formatBaggageResponse(paxDetail->baggageResponse(), recNumber);
  }

  response += xmlResponse;
  response += "</PricingResponse>";
  return response;
}

std::string
XMLConvertUtils::formatBaggageResponse(std::string baggageResponse, int& recNum)
{
  std::string result = "";
  size_t loc = 0;
  while ((loc = baggageResponse.find("\\n", loc)) != std::string::npos)
    baggageResponse.replace(loc, 2, "\n");
  recNum = formatResponse(baggageResponse, result, recNum, "Y");
  return result;
}

std::string
XMLConvertUtils::rexPricingTrxResponse(RexBaseTrx& trx)
{
  Diagnostic& diag = trx.diagnostic();

  FareCalcCollector* fcc =
      ((diag.diagnosticType() == DiagnosticNone || diag.diagnosticType() == Diagnostic855) &&
       !trx.fareCalcCollector().empty())
          ? trx.fareCalcCollector().front()
          : nullptr;

  std::string response = diag.toString();

  if (diag.diagnosticType() != DiagnosticNone && response.empty())
  {
    response = printDiagReturnedNoData(diag.diagnosticType());
  }

  PricingResponseFormatter formatter;
  return formatter.formatResponse(response, false, trx, fcc);
}
std::string
XMLConvertUtils::formatWpnResponse(PricingDetailTrx& pricingDetailTrx)
{
  std::string wqApplicableBookingCodesLine = "";
  const bool isDetailedWQ = (pricingDetailTrx.billing()->actionCode().find("WQ") == 0);
  // if this is detailed WQ entry - obtain WQCC table options for agent
  NoPNROptions* noPnrOptions = nullptr;
  if (isDetailedWQ)
    noPnrOptions = NoPNRPricingTrx::getNoPNROptions(&(pricingDetailTrx.ticketingAgent()),
                                                    pricingDetailTrx.dataHandle());
  std::string response;
  int recNum = 2;
  int index = 0;

  if (pricingDetailTrx.diagnostic().diagnosticType() == Diagnostic870)
  {
    std::string diagTxt = pricingDetailTrx.diagnostic().toString();
    if (diagTxt.empty())
      diagTxt = printDiagReturnedNoData(Diagnostic870);
    response = prepareResponseText(diagTxt);
  }
  else
  {
    if (pricingDetailTrx.ticketingAgent().vendorCrsCode() == AXESS_MULTIHOST_ID)
    {
      std::string notFormattedString = "VT \n";
      recNum = formatResponse(notFormattedString, response, recNum);
    }

    const std::vector<PaxDetail*>& paxDetails = pricingDetailTrx.paxDetails();
    const std::vector<Itin*>& itins = pricingDetailTrx.itin();
    const bool itinEqPaxDtl = paxDetails.size() == itins.size();

    if (!itinEqPaxDtl)
      LOG4CXX_ERROR(logger, "XMLException, ObFees: size of paxDetails and itins are different!");

    for (size_t paxDtlIdx = 0; paxDtlIdx < paxDetails.size(); ++paxDtlIdx)
    {
      response += formatWpnDetailsResponse(paxDetails[paxDtlIdx]->wpnDetails(),
                                           isDetailedWQ,
                                           !index++,
                                           wqApplicableBookingCodesLine,
                                           noPnrOptions,
                                           recNum,
                                           pricingDetailTrx);

      response += formatBaggageResponse(paxDetails[paxDtlIdx]->baggageResponse(), recNum);

      if (pricingDetailTrx.getRequest()->isCollectOBFee() && itinEqPaxDtl)
        OBFeesUtils::addObFeesInfo(
            pricingDetailTrx, response, paxDetails[paxDtlIdx], itins[paxDtlIdx]);
    }

    if (isDetailedWQ && noPnrOptions && noPnrOptions->displayFinalWarningMsg2() == 'Y')
    {
      std::string notFormattedString = noPnrOptions->passengerDetailPTCBreak() != 'Y' ? " \n" : "";
      notFormattedString += "** TOTALS INCLUDE KNOWN TAXES AND FEES **\n"
                            "** TOTAL FARE, TAXES AND FEES MAY CHANGE ONCE FLIGHTS ARE \n"
                            "   CONFIRMED **\n";

      std::string formattedString;
      formatResponse(notFormattedString, formattedString, recNum);
      response += formattedString;
    }

    if (isDetailedWQ && wqApplicableBookingCodesLine.size() > 0)
      response = wqApplicableBookingCodesLine + response;
  }
  wrapWpnRespWitnMainTag(response);
  return response;
}

std::string
XMLConvertUtils::formatWpnDetailsResponse(std::string wpnDetails,
                                          bool isDetailedWQ,
                                          bool isFirst,
                                          std::string& wqApplicableBookingCodesLine,
                                          NoPNROptions* noPnrOptions,
                                          int& recNum,
                                          PricingDetailTrx& trx)
{
  std::string result = "";

  size_t loc = 0;
  while ((loc = wpnDetails.find("\\n", loc)) != std::string::npos)
    wpnDetails.replace(loc, 2, "\n");

  if (isDetailedWQ)
  {
    std::string::size_type firstEol = wpnDetails.find("\n");
    if (firstEol != std::string::npos)
    {
      if (isFirst)
        wqApplicableBookingCodesLine = wpnDetails.substr(0, firstEol);
      // remove the applied booking codes line from response
      wpnDetails = wpnDetails.substr(firstEol + 1);
    }
  }

  // for NoPNR transacion - only separate options if WQCC setting is 'Y'
  if (!isDetailedWQ || (noPnrOptions && noPnrOptions->passengerDetailPTCBreak() == 'Y'))
    wpnDetails += " \n"; // separate options by blank

  recNum = formatResponse(wpnDetails, result, recNum, "X", &trx);

  return result;
}

XMLSingleLevelFinder::ElementPair
XMLSingleLevelFinder::singleTagLookup(const char* pattern) const
{
  return singleTagLookupImpl(pattern, 0);
}

std::vector<XMLSingleLevelFinder::ElementPair>
XMLSingleLevelFinder::multipleTagLookup(const char* pattern) const
{
  std::vector<ElementPair> results;

  std::size_t position{0};
  while (true)
  {
    auto elem = singleTagLookupImpl(pattern, position);
    if (elem.first == std::string::npos || elem.second == std::string::npos)
      break;

    std::size_t pre = position;
    position += (elem.second + 1 - pre);
    results.push_back(std::move(elem));
  }

  return results;
}

XMLSingleLevelFinder::ElementPair
XMLSingleLevelFinder::singleTagLookupImpl(const char* pattern, std::size_t from) const
{
  static const std::string xmlEndTag{">"};

  std::size_t start = _str.find(pattern, from);
  if (start == std::string::npos)
    return std::make_pair(std::string::npos, std::string::npos);

  std::size_t end = _str.find(xmlEndTag, start);
  if (end == std::string::npos)
    return std::make_pair(std::string::npos, std::string::npos);

  return std::make_pair(start, end);
}

void
XMLSingleLevelFinder::swapStringsByPosition(std::string& output, ElementPair from, ElementPair to)
{
  std::string tag = output.substr(from.first, from.second - from.first + 1);
  output.erase(from.first, from.second - from.first + 1);
  output.insert(to.first, tag);
}
}
