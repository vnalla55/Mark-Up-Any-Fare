// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include "Xform/AtpcoTaxDisplayResponseFormatter.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "Common/XMLConstruct.h"
#include "DataModel/TaxTrx.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/TaxDisplayResponseFormatter.h"

#include <sstream>
#include <string>

namespace tse
{

const std::string AtpcoTaxDisplayResponseFormatter::XML_DECLARATION_TAG_TEXT =
    "xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"";

const std::string AtpcoTaxDisplayResponseFormatter::TAX_DISPLAY_XML_VERSION_TEXT = "2003A.TsabreXML1.0";

AtpcoTaxDisplayResponseFormatter::AtpcoTaxDisplayResponseFormatter()
: XML_NAMESPACE_TEXT(readConfigXMLNamespace("OTA_XML_NAMESPACE")),
  XML_NAMESPACE_XS_TEXT(readConfigXMLNamespace("OTA_XML_NAMESPACE_XS")),
  XML_NAMESPACE_XSI_TEXT(readConfigXMLNamespace("OTA_XML_NAMESPACE_XSI"))
{
}

void
AtpcoTaxDisplayResponseFormatter::formatResponse(TaxTrx& trx)
{
  if(trx.response().str().empty())
    return;

  XMLConstruct xml;

  addResponseHeader(trx, xml);

  std::ostringstream messageFailCodeStr;
  unsigned int lineNum = 3; // why from 3?

  std::istringstream response(trx.response().str());
  std::string responseLine;
  while(std::getline(response, responseLine))
  {
    if(responseLine.empty())
      responseLine = " "; // blank line

    messageFailCodeStr.str("");
    messageFailCodeStr.width(6);
    messageFailCodeStr.fill('0');
    messageFailCodeStr << lineNum++;

    xml.openElement(tse::xml2::MessageInformation);
    xml.addAttribute(tse::xml2::MessageType,      "X");
    xml.addAttribute(tse::xml2::MessageFailCode,  messageFailCodeStr.str());
    xml.addAttribute(tse::xml2::MessageText,      responseLine);
    xml.closeElement();
  }

  addResponseFooter(xml);
  trx.response().str(xml.getXMLData());
}

void
AtpcoTaxDisplayResponseFormatter::addResponseHeader(TaxTrx& taxTrx, XMLConstruct& construct)
{
  construct.addSpecialElement(XML_DECLARATION_TAG_TEXT.c_str());
  if (taxTrx.taxDisplayRequestRootElementType() == "AIRTAXDISPLAYRQ")
    construct.openElement("AirTaxDisplayRS");
  else
    construct.openElement("TaxDisplayRS");
  construct.addAttribute("xmlns", XML_NAMESPACE_TEXT);
  construct.addAttribute("xmlns:xs", XML_NAMESPACE_XS_TEXT);
  construct.addAttribute("xmlns:xsi", XML_NAMESPACE_XSI_TEXT);
  construct.addAttribute("Version", TAX_DISPLAY_XML_VERSION_TEXT);
}

void
AtpcoTaxDisplayResponseFormatter::addResponseFooter(XMLConstruct& construct)
{
  construct.closeElement(); // TaxRS
}

const std::string
AtpcoTaxDisplayResponseFormatter::readConfigXMLNamespace(const std::string& configName)
{
  ConfigMan& config = Global::config();

  std::string xmlNamespace;

  if (!config.getValue(configName, xmlNamespace, "TAX_SVC"))
  {
    //CONFIG_MAN_LOG_KEY_ERROR(_logger, configName, "TAX_SVC");
  }

  return xmlNamespace;
}

} /* namespace tse */
