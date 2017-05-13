//----------------------------------------------------------------------------
//
//  File:  PfcDisplayResponseFormatter.cpp
//  Description: See PfcDisplayResponseFormatter.h file
//  Created: August, 2007
//  Authors:  Dean van Decker
//
//  Copyright Sabre 2007
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

#include "Xform/PfcDisplayResponseFormatter.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "DataModel/TaxTrx.h"
#include "Xform/PricingResponseXMLTags.h"

#include <vector>

using namespace tse;
using namespace std;

static Logger
logger("atseintl.Xform.PfcDisplayResponseFormatter");

//----------------------------------------------------------------------------
// PfcDisplayResponseFormatter::PfcDisplayResponseFormatter
//----------------------------------------------------------------------------
PfcDisplayResponseFormatter::PfcDisplayResponseFormatter() {}

//----------------------------------------------------------------------------
// PfcDisplayResponseFormatter::~PfcDisplayResponseFormatter
//----------------------------------------------------------------------------
PfcDisplayResponseFormatter::~PfcDisplayResponseFormatter() {}

//----------------------------------------------------------------------------
// PfcDisplayResponseFormatter::formatResponse
//----------------------------------------------------------------------------
void
PfcDisplayResponseFormatter::formatResponse(TaxTrx& taxTrx)
{
  XMLConstruct construct;
  construct.openElement("PFCDisplayResponse");

  buildMessage(taxTrx.response().str(), construct);
  construct.closeElement();

  taxTrx.response().str(EMPTY_STRING());
  taxTrx.response() << construct.getXMLData();

  return;
}

//----------------------------------------------------------------------------
// PfcDisplayResponseFormatter::buildMessage
//----------------------------------------------------------------------------
void
PfcDisplayResponseFormatter::buildMessage(const std::string& response,
                                          XMLConstruct& construct,
                                          std::string msgType)
{
  // Attaching MSG elements

  std::string tmpResponse = response;

  /*TaxDisplayFormatter dispFmt(60);
  dispFmt.format(tmpResponse);*/

  LOG4CXX_DEBUG(logger, "Before XML:\n" << tmpResponse);

  unsigned int lastPos = 0;
  int recNum = 2;
  const int BUF_SIZE = 256;
  const int AVAILABLE_SIZE = BUF_SIZE - 52;
  char tmpBuf[BUF_SIZE];

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
  int tokenLen = 0;
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
    construct.openElement(xml2::MessageInformation);
    construct.addAttribute(xml2::MessageType, msgType);

    sprintf(tmpBuf, "%06d", msgType == "E" ? 0 : recNum + 1);
    construct.addAttribute(xml2::MessageFailCode, tmpBuf);
    construct.addAttribute(xml2::MessageText, token);
    construct.closeElement();
  }

  recNum++;
  construct.openElement(xml2::MessageInformation);
  construct.addAttribute(xml2::MessageType, msgType);

  sprintf(tmpBuf, "%06d", msgType == "E" ? 0 : recNum + 1);
  construct.addAttribute(xml2::MessageFailCode, tmpBuf);
  construct.closeElement();

  LOG4CXX_DEBUG(logger, "XML output:\n" << construct.getXMLData());
  LOG4CXX_DEBUG(logger, "Finished in formatResponse");

  return;
}
//----------------------------------------------------------------------------
// PfcDisplayResponseFormatter::formatResponse
//----------------------------------------------------------------------------
void
PfcDisplayResponseFormatter::formatResponse(TaxTrx& taxTrx, ErrorResponseException& ere)
{
  std::string response = taxTrx.response().str();
  formatResponse(ere, response);

  taxTrx.response().str(EMPTY_STRING());
  taxTrx.response() << response;
}

//----------------------------------------------------------------------------
// PfcDisplayResponseFormatter::formatResponse
//----------------------------------------------------------------------------
void
PfcDisplayResponseFormatter::formatResponse(const ErrorResponseException& ere, std::string& response)
{
  XMLConstruct construct;
  construct.openElement("PFCDisplayResponse");

  response = ere.message();
  buildMessage(response, construct, "E");

  construct.closeElement();
  response = construct.getXMLData();
}
