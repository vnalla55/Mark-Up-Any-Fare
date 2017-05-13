//----------------------------------------------------------------------------
//
//  File:  CurrencyResponseFormatter.cpp
//  Description: See CurrencyResponseFormatter.h file
//  Created:  February 17, 2005
//  Authors:  Mike Carroll
//
//  Copyright Sabre 2003
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

#include "Xform/CurrencyResponseFormatter.h"

#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/XMLConstruct.h"
#include "DataModel/PricingRequest.h"
#include "Diagnostic/Diagnostic.h"

#include <vector>

namespace tse
{
FALLBACK_DECL(fixSSDTP103CurrencyDisplay);

namespace
{
Logger _logger(
    ("atseintl.Xform.CurrencyResponseFormatter"));
}

std::string
CurrencyResponseFormatter::formatResponse(CurrencyTrx& currencyTrx)
{
  std::string tmpResponse = currencyTrx.response().str();
  LOG4CXX_INFO(_logger, "Response (before XML tagging): " << tmpResponse);

  size_t lastPos = 0;
  recNum = 2;
  const int BUF_SIZE = 256;
  const int AVAILABLE_SIZE = BUF_SIZE - 52;

  XMLConstruct construct;
  construct.openElement("CurrencyConversionResponse");

  if (currencyTrx.getRequest()->diagnosticNumber() == Diagnostic854)
    buildDiag854(construct, recNum);

  if (tmpResponse.length() == 0)
  {
    if (fallback::fixSSDTP103CurrencyDisplay(&currencyTrx))
      addMessageLine("REQUEST RECEIVED - NO RESPONSE DATA", construct, "E", recNum + 1);
    else
      addMessageLine("UNABLE TO DISPLAY - BSR NOT AVAILABLE", construct, "X", recNum + 1);
  }
  else
  {
    // Clobber the trailing newline
    while (1)
    {
      lastPos = tmpResponse.rfind("\n");
      if (lastPos > 0 && lastPos == (tmpResponse.length() - 1))
        tmpResponse.replace(lastPos, 1, "\0");
      else
        break;
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
        LOG4CXX_WARN(_logger, "Line: [" << token << "] too long!");
        continue;
      }
      addMessageLine(token, construct, currencyTrx.isErrorResponse() ? "E" : "X", recNum + 1);
    }
  }
  construct.closeElement();

  return construct.getXMLData();
}

void CurrencyResponseFormatter::formatResponse(const ErrorResponseException& ere, std::string& response)
{
  XMLConstruct construct;
  construct.openElement("CurrencyConversionResponse");
  addMessageLine(ere.message(), construct, "E", Message::errCode(ere.code()));
  construct.closeElement();

  response = construct.getXMLData();
}

} // namespace
