//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
#include "Xform/STLTicketingCxrDisplayResponseFormatter.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/ValidatingCxrConst.h"
#include "Common/XMLConstruct.h"
#include "DataModel/TicketingCxrDisplayTrx.h"
#include "Diagnostic/Diagnostic.h"
#include "Xform/PricingResponseSTLTags.h"
#include "Xform/STLMessage.h"
#include "Xform/XformUtil.h"

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp);

static Logger
logger("atseintl.Xform.STLTicketingCxrDisplayResponseFormatter");

void STLTicketingCxrDisplayResponseFormatter::formatResponse(const ErrorResponseException& ere, std::string& response)
{
  XMLConstruct construct;
  construct.openElement(TicketingCxrDisplayRS);
  construct.addAttribute("version", dispSchemaVersion);
  construct.addAttribute("xmlns", "http://stl.sabre.com/AirPricing/validating_cxr/v0");

  TicketingCxrResponseUtil::prepareMessage(construct, STL::Error, Message::errCode(ere.code()), ere.message());
  construct.closeElement();

  response = construct.getXMLData();
}

std::string
STLTicketingCxrDisplayResponseFormatter::formatResponse(
    const std::string& responseString,
    TicketingCxrDisplayTrx& tcdTrx,
    const ErrorResponseException::ErrorResponseCode errCode)
{
  LOG4CXX_INFO(logger, "Response (before XML tagging):\n" << responseString);
  XMLConstruct construct;
  construct.openElement(TicketingCxrDisplayRS);
  construct.addAttribute("version", dispSchemaVersion);
  construct.addAttribute("xmlns", "http://stl.sabre.com/AirPricing/validating_cxr/v0");

  Diagnostic& diag = tcdTrx.diagnostic();
  if (diag.diagnosticType() == Diagnostic854 || diag.diagnosticType() == Diagnostic191)
  {
    LOG4CXX_DEBUG(logger, "STLTicketingCxrDisplayResponseFormatter::formatResponse() - diagnostic ");
    prepareDiagnostic(tcdTrx, diag, construct);
  }
  else
  {
    if (errCode > 0)
    {
      TicketingCxrResponseUtil::prepareMessage(construct, STL::Error, Message::errCode(errCode), responseString);
      LOG4CXX_DEBUG(logger, "STLTicketingCxrDisplayResponseFormatter::formatResponse() - error > 0 ");
    }
    else
    {
      LOG4CXX_DEBUG(logger, "STLTicketingCxrDisplayResponseFormatter::formatResponse() - entered ");
      if (tcdTrx.getValidationStatus()==vcx::NO_MSG)
      {
        construct.openElement(STL::Country);
        construct.addAttribute(STL::code, tcdTrx.getCountry());
        construct.closeElement();
        construct.openElement(STL::PrimeHost);
        construct.addAttribute(STL::code, tcdTrx.getPrimeHost());
        construct.closeElement();
        formatTicketingCxrDisplayResponse(construct, tcdTrx);
      }
      else
      {
        TicketingCxrResponseUtil::prepareMessage(construct,
            STL::Error,
            tcdTrx.getValidationStatus(),
            vcx::getValidationCxrMsg(tcdTrx.getValidationStatus()));
      }
    }
  }
  construct.closeElement();
  LOG4CXX_INFO(logger, "Response (before XML tagging):\n" << responseString);
  return construct.getXMLData();
}

void
STLTicketingCxrDisplayResponseFormatter::formatTicketingCxrDisplayResponse(XMLConstruct& construct,
                                                             TicketingCxrDisplayTrx& tcdTrx)
{
  LOG4CXX_DEBUG(logger,
      "STLTicketingCxrDisplayResponseFormatter::formatTicketingCxrDisplayResponse() - entered");

  TicketingCxrDisplayResponse& resp = tcdTrx.getResponse();
  if (vcx::DISPLAY_INTERLINE_AGMT==tcdTrx.getRequest()->getRequestType())
  {
    const InterlineAgreements& agmts = resp.interlineAgreements();
    if (!agmts.empty())
    {
      construct.openElement(STL::InterlineAgreementDisplay);
      //@todo we need to replace validating cxr code with it's name
      construct.addAttribute(STL::carrierName, resp.validatingCxr());
      for (const auto& agmt : agmts)
      {
        construct.openElement(STL::InterlineAgreements);
        std::string agmtType(agmt.first);
        construct.addAttribute(STL::agreementType, (agmtType.empty() ? "PPR" : agmtType.c_str()));
        formatCarriers(construct, agmt.second);
        construct.closeElement();
      }
      construct.closeElement();
    }
  }
  if (vcx::DISPLAY_VCXR==tcdTrx.getRequest()->getRequestType())
  {
      const ValidatingCxrDisplayMap& valCxrMap = resp.validatingCxrDisplayMap();
      if(!valCxrMap.empty())
      {
        // Return items in settlement plan type order
        for (SettlementPlanType spType : vcx::SP_HIERARCHY)
        {
          if ((spType == "GTC" || spType == "IPC") &&
              fallback::fallbackValidatingCxrMultiSp(&tcdTrx) &&
              !tcdTrx.overrideFallbackValidationCXRMultiSP())
            continue;
          ValidatingCxrDisplayMapIter it = valCxrMap.find( spType );
          if ( it != valCxrMap.end() )
          {
            const TicketingCxrValidatingCxrDisplay& tcxDisplay = it->second;
            construct.openElement(STL::ValidatingCxrDisplay);
            construct.addAttribute(STL::settlementPlanCode, spType);
            construct.addAttribute(STL::settlementPlanName, vcx::getSettlementPlanName(spType));
            formatTicketingCxrValidatingCarrier(construct, tcxDisplay);
            formatTicketingCxrGeneralSalesAgent(construct, tcxDisplay);
            formatTicketingCxrNeutralValidatingCxr(construct, tcxDisplay);
            construct.closeElement();
          }
        }
      }
  }
  LOG4CXX_DEBUG(logger,
      "STLTicketingCxrDisplayResponseFormatter::formatTicketingCxrDisplayResponse() - complete");
}


void
STLTicketingCxrDisplayResponseFormatter::formatTicketingCxrNeutralValidatingCxr(
    XMLConstruct& construct,
    const TicketingCxrValidatingCxrDisplay& tcxDisplay)
{
  if(tcxDisplay.neutralValidatingCxr().empty())
    return;

  construct.openElement(STL::NeutralValidatingCxrs);
  formatCarriers(construct, tcxDisplay.neutralValidatingCxr());
  construct.closeElement();
}

void
STLTicketingCxrDisplayResponseFormatter::formatTicketingCxrGeneralSalesAgent(
    XMLConstruct& construct,
    const TicketingCxrValidatingCxrDisplay& tcxDisplay)
{
  if(tcxDisplay.generalSalesAgentMap().empty())
    return;

  for (const auto& elem : tcxDisplay.generalSalesAgentMap())
  {
    construct.openElement(STL::GeneralSalesAgents);
    construct.addAttribute(STL::carrierName, elem.first.c_str());
    formatCarriers(construct, elem.second);
    construct.closeElement();
  }
}

// Build response in ticket type order
void
STLTicketingCxrDisplayResponseFormatter::formatTicketingCxrValidatingCarrier(
    XMLConstruct& construct,
    const TicketingCxrValidatingCxrDisplay& tcxDisplay)

{
    if(tcxDisplay.validatingCarrierMap().empty())
      return;

    ValidatingCarrierMapIter it = tcxDisplay.validatingCarrierMap().find( vcx::ETKT_REQ );
    if ( it != tcxDisplay.validatingCarrierMap().end() )
    {
      formatTicketingCxrValidatingCxr( construct, it->first, it->second );
    }

    it = tcxDisplay.validatingCarrierMap().find( vcx::ETKT_PREF );
    if ( it != tcxDisplay.validatingCarrierMap().end() )
    {
      formatTicketingCxrValidatingCxr( construct, it->first, it->second );
    }

    it = tcxDisplay.validatingCarrierMap().find( vcx::PAPER_TKT_PREF );
    if ( it != tcxDisplay.validatingCarrierMap().end() )
    {
      formatTicketingCxrValidatingCxr( construct, it->first, it->second );
    }
}

void
STLTicketingCxrDisplayResponseFormatter::formatTicketingCxrValidatingCxr(
    XMLConstruct& construct,
    const vcx::TicketType ticketType,
    const std::set<CarrierCode>& carriers)

{
  construct.openElement(STL::ValidatingCxrs);
  construct.addAttribute(STL::ticketType, vcx::getTicketTypeText(ticketType));
  formatCarriers(construct, carriers);
  construct.closeElement();
}

void
STLTicketingCxrDisplayResponseFormatter::formatCarriers(
    XMLConstruct& construct,
    const std::set<CarrierCode>& carriers)
{
  for (const CarrierCode carrier : carriers)
  {
    construct.openElement(STL::Carrier);
    construct.addAttribute(STL::code, carrier);
    construct.closeElement();
  }
}

void
STLTicketingCxrDisplayResponseFormatter::prepareDiagnostic(TicketingCxrDisplayTrx& tcdTrx,
                                                    Diagnostic& diag,
                                                    XMLConstruct& construct)
{
  LOG4CXX_DEBUG(logger, "STLTicketingCxrDisplayResponseFormatter::prepareDiagnostic() - entered");
  uint16_t codeNum = 1;
  if (tcdTrx.diagnostic().diagnosticType() == Diagnostic854)
    prepareHostPortInfo(tcdTrx, construct);
  if (diag.diagnosticType() != DiagnosticNone && !diag.toString().empty())
    TicketingCxrResponseUtil::prepareResponseText(diag.toString(), construct, codeNum);
  LOG4CXX_DEBUG(logger, "STLTicketingCxrDisplayResponseFormatter::prepareDiagnostic() - exit");
}


/*
 * @todo This is common method shared between TicketingFee and TicketingCxrSvc.
 * Can we move it to parent class?
 */
void
STLTicketingCxrDisplayResponseFormatter::prepareHostPortInfo(TicketingCxrDisplayTrx& tcdTrx,
                                                      XMLConstruct& construct)
{
  LOG4CXX_DEBUG(logger, "STLTicketingCxrDisplayResponseFormatter::prepareHostPortInfo() - entered");
  std::vector<std::string> hostInfo;
  std::vector<std::string> buildInfo;
  std::vector<std::string> dbInfo;
  std::vector<std::string> configInfo;
  uint16_t codeNum = 1;

  if (hostDiagString(hostInfo))
  {
    for (const auto& elem : hostInfo)
      TicketingCxrResponseUtil::prepareResponseText(elem, construct, codeNum);
  }

  buildDiagString(buildInfo);
  for (const auto& elem : buildInfo)
    TicketingCxrResponseUtil::prepareResponseText(elem, construct, codeNum);

  dbDiagString(dbInfo);
  for (const auto& elem : dbInfo)
    TicketingCxrResponseUtil::prepareResponseText(elem, construct, codeNum);

  if (configDiagString(configInfo, tcdTrx))
  {
    for (const auto& elem : configInfo)
      TicketingCxrResponseUtil::prepareResponseText(elem, construct, codeNum);
  }
  LOG4CXX_DEBUG(logger, "STLTicketingCxrDisplayResponseFormatter::prepareHostPortInfo() - complete");
}

}
