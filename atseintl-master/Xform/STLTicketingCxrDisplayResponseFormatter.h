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

#include "Common/ValidatingCxrConst.h"
#include "Common/XMLConstruct.h"
#include "DataModel/TicketingCxrDisplayTrx.h"
#include "Xform/CustomXMLParser/IAttributes.h"
#include "Xform/PricingResponseFormatter.h"


static const std::string TicketingCxrDisplayRS = "TicketingCxrDisplayRS";
static const std::string dispSchemaVersion = "1.0.0";

namespace tse
{
class Diagnostic;
class STLTicketingCxrDisplayResponseFormatter : public PricingResponseFormatter
{
public:
  static void formatResponse(const ErrorResponseException& ere, std::string& response);

  virtual std::string formatResponse(
      const std::string& respStr,
      TicketingCxrDisplayTrx& tcsTrx,
      const ErrorResponseException::ErrorResponseCode errCode = ErrorResponseException::NO_ERROR);
  virtual void formatTicketingCxrDisplayResponse(XMLConstruct& c, TicketingCxrDisplayTrx& tcxTrx);
  void prepareDiagnostic(TicketingCxrDisplayTrx& trx, Diagnostic& diag, XMLConstruct& construct);
  void prepareResponseText(const std::string& responseString,
                           XMLConstruct& construct,
                           uint16_t& codeNum,
                           const bool noSizeLImit = false) const;
  void prepareHostPortInfo(TicketingCxrDisplayTrx& tcsTrx, XMLConstruct& construct);
  void formatTicketingCxrNeutralValidatingCxr(XMLConstruct& construct,
                                          const TicketingCxrValidatingCxrDisplay& tcxDisplay);
  void formatTicketingCxrGeneralSalesAgent(XMLConstruct& construct,
                                          const TicketingCxrValidatingCxrDisplay& tcxDisplay);
  void formatTicketingCxrValidatingCarrier(XMLConstruct& construct,
                                          const TicketingCxrValidatingCxrDisplay& tcxDisplay);
  void formatTicketingCxrValidatingCxr(XMLConstruct& construct,
                                       const vcx::TicketType ticketType,
                                       const std::set<CarrierCode>& carriers);
  void formatCarriers(XMLConstruct& construct,
                      const std::set<CarrierCode>& carriers);

};
}

