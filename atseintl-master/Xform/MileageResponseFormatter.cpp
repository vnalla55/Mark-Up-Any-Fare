//----------------------------------------------------------------------------
//
//  File:  MileageResponseFormatter.cpp
//  Description: See MileageResponseFormatter.h file
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

#include "Xform/MileageResponseFormatter.h"

#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Common/XMLConstruct.h"
#include "DataModel/MileageTrx.h"

#include <vector>

namespace tse
{
static Logger
logger("atseintl.Xform.MileageResponseFormatter");

void MileageResponseFormatter::formatResponse(const ErrorResponseException& ere, std::string& response)
{
  XMLConstruct xml;
  xml.openElement("MileageResponse");

  addMessageLine(ere.message(), xml, "E", Message::errCode(ere.code()));
  xml.closeElement();

  response = xml.getXMLData();
}

std::string
MileageResponseFormatter::formatResponse(MileageTrx& mileageTrx)
{
  std::string tmpResponse = mileageTrx.response().str();
  LOG4CXX_INFO(logger, "Response (before XML tagging): " << tmpResponse);

  updateDiagResponseText(mileageTrx, tmpResponse);

  unsigned int lastPos = 0;
  int recNum = 2;
  const int BUF_SIZE = 256;
  const int AVAILABLE_SIZE = BUF_SIZE - 52;

  XMLConstruct construct;
  construct.openElement("MileageResponse");

  if (mileageTrx.diagnostic().diagnosticType() == Diagnostic854)
    buildDiag854(construct, recNum);

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

    addMessageLine(token, construct, "X", recNum + 1);
  }
  construct.closeElement();

  return construct.getXMLData();
}

void
MileageResponseFormatter::updateDiagResponseText(MileageTrx& mileageTrx, std::string& response)
{
  if (mileageTrx.diagnostic().diagnosticType() != DiagnosticNone &&
      mileageTrx.diagnostic().diagnosticType() != Diagnostic854)
  {
    std::ostringstream diagMessage;
    if (mileageTrx.diagnostic().diagnosticType() == UpperBound)
    {
      mainDiagResponse(diagMessage);
      response = diagMessage.str();
    }
    else
    {
      response = mileageTrx.diagnostic().toString();
      if (response.empty())
      {
        diagMessage << "DIAGNOSTIC " << mileageTrx.getRequest()->diagnosticNumber()
                    << " RETURNED NO DATA";
        response = diagMessage.str();
      }
    }
  }
}

void
MileageResponseFormatter::mainDiagResponse(std::ostringstream& oss) const
{
  oss << "***************************************************************\n";
  oss << "MILEAGE DIAGNOSTICS MASTER LIST\n";
  oss << "***************************************************************\n";
  oss << "ADD CROSS OF LORRAINE Q/* BEFORE EACH MILEAGE DIAGNOSTIC.\n";
  oss << "ENTER A SLASH AFTER THE NUMBER TO ADD APPLICABLE QUALIFIERS\n";
  oss << "  /FM                SPECIFIC FARE MARKET\n";
  oss << "  /SQ                SEQUENCE NUMBER\n";
  oss << " \n";
  oss << " \n";
  oss << "196                  MILEAGE XML REQUEST TO ATSE\n";
  oss << "197                  MILEAGE XML RESPONSE FROM ATSE\n";
  oss << "452                  TPM EXCLUSION\n";
  oss << "854                  DATABASE SERVER AND PORT NUMBER\n";
  oss << "***************************************************************\n";
}
}
