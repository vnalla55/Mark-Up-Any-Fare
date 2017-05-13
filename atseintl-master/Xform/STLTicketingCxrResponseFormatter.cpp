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
#include "Xform/STLTicketingCxrResponseFormatter.h"

#include "Common/Logger.h"
#include "Common/ValidatingCxrConst.h"
#include "Common/XMLConstruct.h"
#include "DataModel/TicketingCxrTrx.h"
#include "Diagnostic/Diagnostic.h"
#include "Xform/PricingResponseSTLTags.h"
#include "Xform/STLMessage.h"
#include "Xform/XformUtil.h"

namespace tse
{
static Logger
logger("atseintl.Xform.STLTicketingCxrResponseFormatter");

void STLTicketingCxrResponseFormatter::formatResponse(const ErrorResponseException& ere, std::string& response)
{
  XMLConstruct construct;
  construct.openElement(TicketingCxrServiceRS);
  construct.addAttribute("version", _version);
  construct.addAttribute("xmlns", "http://stl.sabre.com/AirPricing/validating_cxr/v0");

  TicketingCxrResponseUtil::prepareMessage(construct, STL::Error, Message::errCode(ere.code()), ere.message());
  construct.closeElement();

  response = construct.getXMLData();
}

std::string
STLTicketingCxrResponseFormatter::formatResponse(
    const std::string& responseString,
    TicketingCxrTrx& tcsTrx,
    const ErrorResponseException::ErrorResponseCode errCode)
{
  LOG4CXX_INFO(logger, "Response (before XML tagging):\n" << responseString);
  XMLConstruct construct;
  construct.openElement(TicketingCxrServiceRS);
  construct.addAttribute("version", _version);
  construct.addAttribute("xmlns", "http://stl.sabre.com/AirPricing/validating_cxr/v0");

  Diagnostic& diag = tcsTrx.diagnostic();
  if (diag.diagnosticType() == Diagnostic854 || diag.diagnosticType() == Diagnostic191)
  {
    LOG4CXX_DEBUG(logger, "STLTicketingCxrResponseFormatter::formatResponse() - diagnostic ");
    prepareDiagnostic(tcsTrx, diag, construct);
  }
  else
  {
    construct.openElement(STL::ValidatingCxrResult);
    if (errCode > 0)
    {
      TicketingCxrResponseUtil::prepareMessage(construct, STL::Error, Message::errCode(errCode), responseString);
      LOG4CXX_DEBUG(logger, "STLTicketingCxrResponseFormatter::formatResponse() - error > 0 ");
    }
    else
    {
      LOG4CXX_DEBUG(logger, "STLTicketingCxrResponseFormatter::formatResponse() - entered ");
      formatTicketingCxrResponse(construct, tcsTrx);
    }
    construct.closeElement();
  }
  construct.closeElement();
  LOG4CXX_INFO(logger, "Response (before XML tagging):\n" << responseString);
  return construct.getXMLData();
}

void
STLTicketingCxrResponseFormatter::formatTicketingCxrResponse(XMLConstruct& construct,
                                                             TicketingCxrTrx& tcsTrx)
{
  LOG4CXX_DEBUG(logger,
                "STLTicketingCxrResponseFormatter::formatTicketingCxrResponse() - entered");
  bool isError = (vcx::NO_RESULT == tcsTrx.getValidationResult() ||
                  vcx::NOT_VALID == tcsTrx.getValidationResult() ||
                  vcx::ERROR == tcsTrx.getValidationResult());
  prepareValidationResult(tcsTrx, construct);
  if (tcsTrx.validatingCxrs().size() && !isError && !tcsTrx.isMultipleGSASwap())
  {
    prepareValidatingCxr(tcsTrx, construct);
    prepareParticipatingCxr(tcsTrx, construct);
    prepareTicketType(tcsTrx, construct);
  }
  prepareTrailerMessage(tcsTrx, construct);
  LOG4CXX_DEBUG(logger,
                "STLTicketingCxrResponseFormatter::formatTicketingCxrResponse() - complete");
}

void
STLTicketingCxrResponseFormatter::prepareValidationResult(const TicketingCxrTrx& tcsTrx,
                                                          XMLConstruct& construct)
{
  LOG4CXX_DEBUG(logger, "STLTicketingCxrResponseFormatter::processValidationResult() - entered");
  construct.openElement(STL::ValidationResult);
  construct.addElementData(vcx::getValidationResultText(tcsTrx.getValidationResult()));
  construct.closeElement();
  LOG4CXX_DEBUG(logger, "TicketingCxrResponseFormatter::prepareValidationResult() - complete");
}

void
STLTicketingCxrResponseFormatter::prepareValidatingCxr(const TicketingCxrTrx& tcsTrx,
                                                       XMLConstruct& construct)
{
  LOG4CXX_DEBUG(logger, "STLTicketingCxrResponseFormatter::processValidationResult() - entered");
  if (tcsTrx.validatingCxrs().size())
  {
    construct.openElement(STL::ValidatingCxr);
    construct.addElementData(tcsTrx.validatingCxrs().front().c_str());
    construct.closeElement();
  }
  LOG4CXX_DEBUG(logger, "TicketingCxrResponseFormatter::prepareValidationResult() - complete");
}

void
STLTicketingCxrResponseFormatter::prepareParticipatingCxr(const TicketingCxrTrx& tcsTrx,
                                                          XMLConstruct& construct)
{
  LOG4CXX_DEBUG(logger, "STLTicketingCxrResponseFormatter::processValidationResult() - entered");

  const std::vector<vcx::ParticipatingCxr>& cxrs = tcsTrx.vcxrData().participatingCxrs;
  for (const auto& cxr : cxrs)
  {
    construct.openElement(STL::ParticipatingCxr);
    construct.addAttribute(STL::iatLevel, vcx::getITAgreementTypeCodeText(cxr.agmtType));
    construct.addElementData(cxr.cxrName.c_str());
    construct.closeElement();
  }
  LOG4CXX_DEBUG(logger, "TicketingCxrResponseFormatter::prepareValidationResult() - complete");
}

void
STLTicketingCxrResponseFormatter::prepareTicketType(const TicketingCxrTrx& tcsTrx,
                                                    XMLConstruct& construct)
{
  LOG4CXX_DEBUG(logger, "STLTicketingCxrResponseFormatter::prepareTicketType() - entered");
  if (vcx::NO_TKT_TYPE != tcsTrx.getTicketType())
  {
    construct.openElement(STL::TicketType);
    construct.addElementData(vcx::getTicketTypeText(tcsTrx.getTicketType()));
    construct.closeElement();
  }
  LOG4CXX_DEBUG(logger, "TicketingCxrResponseFormatter::prepareTicketType() - complete");
}

void
STLTicketingCxrResponseFormatter::prepareTrailerMessage(const TicketingCxrTrx& tcsTrx,
                                                        XMLConstruct& construct)
{
  LOG4CXX_DEBUG(logger, "STLTicketingCxrResponseFormatter::prepareTrailerMessage() - entered");
  vcx::ValidationStatus validationStatus = tcsTrx.getValidationStatus();
  std::string msg(vcx::getValidationCxrMsg(validationStatus));
  tcsTrx.buildMessageText(msg);
  TicketingCxrResponseUtil::prepareMessage(construct, STL::General, validationStatus, msg);
}

void
STLTicketingCxrResponseFormatter::prepareDiagnostic(TicketingCxrTrx& tcsTrx,
                                                    Diagnostic& diag,
                                                    XMLConstruct& construct)
{
  LOG4CXX_DEBUG(logger, "STLTicketingCxrResponseFormatter::prepareDiagnostic() - entered");
  uint16_t codeNum = 1;
  if (tcsTrx.diagnostic().diagnosticType() == Diagnostic854)
    prepareHostPortInfo(tcsTrx, construct);
  if (diag.diagnosticType() != DiagnosticNone && !diag.toString().empty())
    TicketingCxrResponseUtil::prepareResponseText(diag.toString(), construct, codeNum);
  LOG4CXX_DEBUG(logger, "STLTicketingCxrResponseFormatter::prepareDiagnostic() - exit");
}

/*
 * @todo This is common method shared between TicketingFee and TicketingCxrSvc.
 * Can we move it to parent class?
 */
void
STLTicketingCxrResponseFormatter::prepareHostPortInfo(TicketingCxrTrx& tcsTrx,
                                                      XMLConstruct& construct)
{
  LOG4CXX_DEBUG(logger, "STLTicketingCxrResponseFormatter::prepareHostPortInfo() - entered");
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

  if (configDiagString(configInfo, tcsTrx))
  {
    for (const auto& elem : configInfo)
      TicketingCxrResponseUtil::prepareResponseText(elem, construct, codeNum);
  }
  LOG4CXX_DEBUG(logger, "STLTicketingCxrResponseFormatter::prepareHostPortInfo() - complete");
}

}
