//----------------------------------------------------------------------------
//  File:        Diag890Collector.cpp
//  Authors:
//  Created:
//
//  Description: Diagnostic 890 Branded Fares - request to Branded service
//  Updates:
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

#include "Diagnostic/Diag890Collector.h"

#include <boost/algorithm/string.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/tokenizer.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <sstream>

#include <xalanc/Include/PlatformDefinitions.hpp>
#include <xalanc/XalanTransformer/XalanTransformer.hpp>

XALAN_USING_XERCES(XMLPlatformUtils)
XALAN_USING_XALAN(XalanTransformer)

namespace tse
{

//----------------------------------------------------------------------------
// Display XML
//----------------------------------------------------------------------------

const std::string
Diag890Collector::XSLT_TRANSFORM_TO_INTERMEDIATE_XML_FORMAT =
  "<xsl:stylesheet version=\"1.0\"\n"
  " xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\"\n"
  " xmlns:xalan=\"http://xml.apache.org/xalan\">\n"
  "  <xsl:output method=\"xml\" indent=\"yes\" xalan:indent-amount=\"1\"/>\n"
  "  <xsl:strip-space elements=\"*\"/>\n"
  "  <xsl:template match=\"/\">\n"
  "    <xsl:apply-templates select=\"@* | node()\"/> \n"
  "  </xsl:template>\n"
  "  <xsl:template match=\"text()[normalize-space()]\">\n"
  "      <xsl:value-of select=\".\"/>\n"
  "  </xsl:template>\n"
  "  <xsl:template match=\"@*\">\n"
  "    <xsl:element name=\"attr\"><xsl:value-of select=\"concat(' ', local-name(), '-:', ., ':') \"/>\n"
  "    </xsl:element>\n"
  "  </xsl:template>\n"
  "  <xsl:template match=\"node()\">\n"
  "    <xsl:copy>\n"
  "      <xsl:element name=\"node\"><xsl:value-of select=\"concat('-', name(), '-')\"/>\n"
  "      </xsl:element>\n"
  "      <xsl:apply-templates select=\"@*\"/> \n"
  "      <xsl:choose>\n"
  "        <xsl:when test=\"@*\"><xsl:element name=\"attribute_separator\"/></xsl:when>\n"
  "      </xsl:choose>\n"
  "      <xsl:apply-templates select=\"node()\"/>    \n"
  "      <xsl:element name=\"node\"><xsl:value-of select=\"concat('-/', name())\"/>\n"
  "      </xsl:element>\n"
  "    </xsl:copy>\n"
  "  </xsl:template>\n"
  "</xsl:stylesheet>\n";

const std::string
Diag890Collector::XSLT_TRANSFORM_TO_TEXT_FORMAT =
  "<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\""
  " xmlns:xalan=\"http://xml.apache.org/xalan\">\n"
  "<xsl:output method=\"text\"/>\n"
  "  <xsl:template match=\"/\"><xsl:value-of select=\".\"/></xsl:template>\n"
  "</xsl:stylesheet>";

void
Diag890Collector::printFormattedXml(DiagCollector& dc, const std::string& xmlData)
{
  static boost::mutex mutex;
  boost::lock_guard<boost::mutex> lock(mutex);

  XMLPlatformUtils::Initialize();
  XalanTransformer::initialize();

  convertXmlToText(dc, xmlData);

  XalanTransformer::terminate();
  XMLPlatformUtils::Terminate();
  XalanTransformer::ICUCleanUp();
}

void
Diag890Collector::convertXmlToText(DiagCollector& dc, const std::string& xmlData)
{
  XalanTransformer theXalanTransformer;

  std::istringstream input(xmlData),
                     intermediateFormatXslt(XSLT_TRANSFORM_TO_INTERMEDIATE_XML_FORMAT),
                     toTextXslt(XSLT_TRANSFORM_TO_TEXT_FORMAT);
  std::ostringstream output;
  std::stringstream  intermediateOutput;

  if(theXalanTransformer.transform(input, intermediateFormatXslt, intermediateOutput) == 0
     && theXalanTransformer.transform(intermediateOutput, toTextXslt, output) == 0)
  {
     boost::char_separator<char> sep("\n");
     std::string uppercaseText = boost::to_upper_copy(output.str());
     boost::tokenizer<boost::char_separator<char> > tokens(uppercaseText, sep);
     for (boost::tokenizer<boost::char_separator<char> >::iterator tokenIter = tokens.begin();
          tokenIter != tokens.end(); ++tokenIter)
     {
       const std::string& str = *tokenIter;
       if(str.find_first_not_of(' ') != std::string::npos)
         addMultilineInfo(dc, str);
     }
  }
  else
  {
     dc << "INTERNAL ERROR WHILE TRANSFORMING XML";
  }
  dc << std::endl << " " << std::endl;
}

void
Diag890Collector::displayXML(const std::string& xmlData,
                             const std::string& diagHeader,
                             BrandingMessageFormat xmlFormat,
                             StatusBrandingService status)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  printHeader(dc, diagHeader);
  if (BS_GREEN_SCREEN_FORMAT == xmlFormat)
  {
    printFormattedXml(dc, xmlData);
  }
  else if ( BS_CDATA_SECTION == xmlFormat)
  {
    dc << " BRAND SERVICE MESSAGE AVAILABLE IN CDATA SECTION\n";
    _rootDiag->appendAdditionalDataSection(xmlData);
  }
  else
  {
    dc << xmlData << "\n\n";
  }
  printFooter(dc, diagHeader);
  if (status == StatusBrandingService::NO_BS_ERROR)
  {
    return;
  }
  std::string errorMessage;
  getBrandingServiceErrorMessage(status, errorMessage);
  dc << errorMessage << "\n"
     << " "
     << "\n";
}

void
Diag890Collector::displayXML_old(const std::string& xmlData,
                             const std::string& diagHeader,
                             bool formatXml,
                             StatusBrandingService status)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  printHeader(dc, diagHeader);
  if (formatXml)
  {
    printFormattedXml(dc, xmlData);
  }
  else
  {
    dc << xmlData << "\n\n";
  }
  printFooter(dc, diagHeader);
  if (status == StatusBrandingService::NO_BS_ERROR)
  {
    return;
  }
  std::string errorMessage;
  getBrandingServiceErrorMessage(status, errorMessage);
  dc << errorMessage << "\n"
     << " "
     << "\n";
}

void
Diag890Collector::printHeader(DiagCollector& dc, const std::string& diagHeader)
{
  dc << "***************************************************************\n"
     << " " << diagHeader << " - START\n"
     << "***************************************************************\n"
     << " \n";
}

void
Diag890Collector::printFooter(DiagCollector& dc, const std::string& diagHeader)
{
  dc << "***************************************************************\n"
     << " " << diagHeader << " - END\n"
     << "***************************************************************\n"
     << " \n";
}

} // tse
