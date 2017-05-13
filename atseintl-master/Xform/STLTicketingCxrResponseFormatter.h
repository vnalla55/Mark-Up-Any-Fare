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
#pragma once

#include "Common/XMLConstruct.h"
#include "DataModel/TicketingCxrTrx.h"
#include "Xform/CustomXMLParser/IAttributes.h"
#include "Xform/PricingResponseFormatter.h"


static const std::string TicketingCxrServiceRS = "TicketingCxrServiceRS";
static const std::string _version = "1.0.0";

namespace tse
{
class Diagnostic;
class STLTicketingCxrResponseFormatter : public PricingResponseFormatter
{
public:
  static void formatResponse(const ErrorResponseException& ere, std::string& response);

  virtual std::string formatResponse(
      const std::string& respStr,
      TicketingCxrTrx& tcsTrx,
      const ErrorResponseException::ErrorResponseCode errCode = ErrorResponseException::NO_ERROR);
  virtual void formatTicketingCxrResponse(XMLConstruct& c, TicketingCxrTrx& tcxTrx);
  void prepareValidationResult(const TicketingCxrTrx& tcsTrx, XMLConstruct& c);
  void prepareValidatingCxr(const TicketingCxrTrx& tcsTrx, XMLConstruct& c);
  void prepareParticipatingCxr(const TicketingCxrTrx& tcsTrx, XMLConstruct& c);
  void prepareTicketType(const TicketingCxrTrx& tcsTrx, XMLConstruct& c);
  void prepareTrailerMessage(const TicketingCxrTrx& tcsTrx, XMLConstruct& construct);
  void prepareDiagnostic(TicketingCxrTrx& trx, Diagnostic& diag, XMLConstruct& construct);
  void prepareHostPortInfo(TicketingCxrTrx& tcsTrx, XMLConstruct& construct);

};
}

