//----------------------------------------------------------------------------
//
//  Copyright Sabre 2010
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

#include "Xform/TktFeesPricingResponseFormatter.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/XMLConstruct.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "Diagnostic/Diagnostic.h"
#include "Xform/DataModelMap.h"
#include "Xform/PricingResponseXMLTags.h"

#include <iomanip>
#include <iterator>
#include <sstream>
#include <vector>

namespace tse
{
static Logger
logger("atseintl.Xform.TktFeesPricingResponseFormatter");

std::string
TktFeesPricingResponseFormatter::formatResponse(const std::string& responseString,
                                                TktFeesPricingTrx& tktFeesTrx,
                                                ErrorResponseException::ErrorResponseCode errCode)
{
  LOG4CXX_INFO(logger, "Response (before XML tagging):\n" << responseString);

  XMLConstruct construct;
  construct.openElement("TktFeesPricingResponse");

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
    // Parsing failed, return error message
    prepareMessage(construct, Message::TYPE_ERROR, Message::errCode(errCode), responseString);
    LOG4CXX_DEBUG(logger, "TktFeesPricingResponseFormatter::formatResponse() - error > 0 ");
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
    LOG4CXX_DEBUG(logger, "TktFeesPricingResponseFormatter::formatResponse() - start ");
    formatTktFeesResponse(construct, tktFeesTrx);
  }
  construct.closeElement();
  LOG4CXX_DEBUG(logger, "TktFeesPricingResponseFormatter::formatResponse() - XML response ready ");

  return construct.getXMLData();
}

void
TktFeesPricingResponseFormatter::formatResponse(const ErrorResponseException& ere, std::string& response)
{
  XMLConstruct construct;
  construct.openElement("TktFeesPricingResponse");
  prepareMessage(construct, Message::TYPE_ERROR, Message::errCode(ere.code()), ere.message());
  construct.closeElement();

  response = construct.getXMLData();
}

void
TktFeesPricingResponseFormatter::prepareResponseText(const std::string& responseString,
                                                     XMLConstruct& construct,
                                                     bool noSizeLImit) const
{
  std::string tmpResponse = responseString;

  size_t lastPos = 0;
  int recNum = 2;
  const int BUF_SIZE = 256;
  const int AVAILABLE_SIZE = BUF_SIZE - 52;
  // char tmpBuf[BUF_SIZE];
  // Clobber the trailing newline
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
    prepareMessage(construct, Message::TYPE_GENERAL, recNum + 1, token);
  }
}

void
TktFeesPricingResponseFormatter::formatTktFeesResponse(XMLConstruct& construct,
                                                       TktFeesPricingTrx& tktFeesTrx)
{
  LOG4CXX_DEBUG(logger, "TktFeesPricingResponseFormatter::formatTktFeesResponse() - entered");
  std::vector<Itin*>::const_iterator itinI = tktFeesTrx.itin().begin();
  for (; itinI != tktFeesTrx.itin().end(); itinI++)
  {
    currentItin() = (*itinI);
    buildTktFeesResponse(tktFeesTrx, currentItin(), construct);
  }
}

void
TktFeesPricingResponseFormatter::prepareHostPortInfo(PricingTrx& trx, XMLConstruct& construct)
{
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
}

void
TktFeesPricingResponseFormatter::buildTktFeesResponse(TktFeesPricingTrx& tktFeesTrx,
                                                      Itin* itin,
                                                      XMLConstruct& construct)
{
  LOG4CXX_DEBUG(logger, "TktFeesPricingResponseFormatter::buildTktFeesResponse() - entered");
  construct.openElement(xml2::ItinInfo);

  construct.addAttributeInteger(xml2::GenId, itin->getItinOrderNum());

  construct.closeElement();
  LOG4CXX_DEBUG(logger, "TktFeesPricingResponseFormatter::buildTktFeesResponse() - complete");
}
} // namespace
