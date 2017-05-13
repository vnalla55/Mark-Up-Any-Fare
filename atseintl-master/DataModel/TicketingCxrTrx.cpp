//-------------------------------------------------------------------
//
//  Authors:
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
//-------------------------------------------------------------------

#include "DataModel/TicketingCxrTrx.h"

#include "Common/Logger.h"
#include "Common/ValidatingCxrUtil.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "Xform/STLTicketingCxrResponseFormatter.h"

namespace tse
{
static Logger
logger("atseintl.DataModel.TicketingCxrTrx");

void
TicketingCxrTrx::buildMessageText(std::string& text) const
{
  if (isResultValid())
  {
    if (isMultipleGSASwap())
    {
      text = "ALTERNATE VALIDATING CARRIER/S - ";
      vcx::join(text, _valCxrs, ' ');
    }
    else
    {
      text += _valCxrs[0]; // we are checking its size in isValid
      if (isGsaSwap()) // single GSA swap
      {
        text += " PER GSA AGREEMENT WITH ";
        if (_request)
          text += _request->getValidatingCxr();
      }
    }
  }
  else // Error conditions
  {
    if (vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH == _valStatus)
    {
      if (_request)
      {
        text = _request->getValidatingCxr();
        text += vcx::getValidationCxrMsg(_valStatus);
        text += _valCxrData.interlineFailedCxr;
      }
    }
    else if (vcx::CXR_DOESNOT_PARTICIPATES_IN_SETTLEMENTPLAN == _valStatus)
    {
      if (_request)
      {
        text =_request->getValidatingCxr();
        text += vcx::getValidationCxrMsg(_valStatus);
        if (_isGTCCarrier)
          text += " - GTC CARRIER";
      }
    }
    else
    {
      text = vcx::getValidationCxrMsg(_valStatus);
    }
  }
}

void
TicketingCxrTrx::convert(tse::ErrorResponseException& ere, std::string& response)
{
  LOG4CXX_DEBUG(logger, "TicketingCxrTrx response: " << xml2());
  STLTicketingCxrResponseFormatter responseFormatter;
  response = responseFormatter.formatResponse(
      (ere.code() > 0 && ere.message().empty()) ? "UNKNOWN EXCEPTION" : ere.message(),
      *this,
      ere.code());
}

bool
TicketingCxrTrx::convert(std::string& response)
{
  std::string tmpResponse;
  LOG4CXX_DEBUG(logger, "TicketingCxrTrx response: " << xml2());
  STLTicketingCxrResponseFormatter responseFormatter;
  response = responseFormatter.formatResponse(tmpResponse, *this);
  LOG4CXX_DEBUG(logger, "Response: " << response);

  return true;
}

} // tse namespace
