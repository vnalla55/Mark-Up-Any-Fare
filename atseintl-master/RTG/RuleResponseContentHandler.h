//-------------------------------------------------------------------
//
//  File:        RuleResponseContentHandler.h
//  Created:     May 26, 2005
//  Authors:     Mike Carroll
//
//  Description: Members and methods for Rule response XML content
//
//
//  Updates:
//
//  Copyright Sabre 2004
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
#pragma once

#include "Common/TseEnums.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

#include <map>
#include <string>

namespace tse
{
using RuleTextMapKey = std::pair<CatNumber, int>;
using RuleTextMap = std::map<RuleTextMapKey, std::string>;

class RuleResponseContentHandler : public xercesc::DefaultHandler
{
public:
  //--------------------------------------------------------------------------
  // @function RuleResponseContentHandler::RuleResponseContentHandler
  //
  // Description: constructor
  //
  // @param textHolder - where to put rule text
  // @param textMap    - where to put rule text map
  // @param diagHolder - where to put rtg diagnostic info
  //--------------------------------------------------------------------------
  RuleResponseContentHandler(std::string& textHolder, RuleTextMap& textMap, std::string& diagHolder)
    : _ruleText(textHolder), _rtgDiagInfo(diagHolder), _ruleTextMap(textMap)
  {
  }

  virtual ~RuleResponseContentHandler();

  //--------------------------------------------------------------------------
  // @function RuleResponseContentHandler::initialize
  //
  // Description: Perform startup initialization
  //
  // @param none
  // @return bool - true if successful, false otherwise
  //--------------------------------------------------------------------------
  const bool initialize();

  //--------------------------------------------------------------------------
  // @function RuleResponseContentHandler::parse
  //
  // Description: Entry point to set content and error handlers.  Initializes
  //              all required XML utilities.  Calls the reader parse routine
  //              to handle the document content.
  //
  // @param chars - The characters from the XML document
  // @param length - The number of characters to read from the array
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  const bool parse(const char* content);

protected:
  xercesc::SAX2XMLReader* _reader = nullptr;

  std::string& _ruleText;
  std::string& _rtgDiagInfo;

  bool _bundledRequest = false;
  CatNumber _catNum = -1;
  int _ruleType = -1;
  RuleTextMap& _ruleTextMap;

  //--------------------------------------------------------------------------
  // @function RuleResponseContentHandler::startElement
  //
  // Description: Over-ridden SAX2 method that captures the start tag and
  //              where applicable extracts the attributes where applicable
  //
  // @param uri - The URI of the associated namespace for this element
  // @param localname - The local part of the element name
  // @param qname - The QName of this element
  // @param attrs - The attributes attached to this element, if any
  // @return void
  //--------------------------------------------------------------------------
  void startElement(const XMLCh* const uri,
                    const XMLCh* const localname,
                    const XMLCh* const qname,
                    const xercesc::Attributes& attrs) override;

  //--------------------------------------------------------------------------
  // @function RuleResponseContentHandler::processResponseAtts
  //
  // Description: This is the equivalent of a document end for an RTG
  //              rule response
  //
  // @param attrs - attributes to be processed
  // @return void
  //--------------------------------------------------------------------------
  void processResponseAtts(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function RuleResponseContentHandler::processDGNAtts
  //
  // Description: This is to process the attributes of DGN tag
  //
  // @param attrs - attributes to be processed
  // @return void
  //--------------------------------------------------------------------------
  void processDGNAtts(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function RuleResponseContentHandler::processRTRAtts
  //
  // Description: This is to process the attributes of RTR tag
  //
  // @param attrs - attributes to be processed
  // @return void
  //--------------------------------------------------------------------------
  void processRTRAtts(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function RuleResponseContentHandler::processExceptionAtts
  //
  // Description: This is to process the attributes of GenericException tag
  //
  // @param attrs - attributes to be processed
  // @return void
  //--------------------------------------------------------------------------
  void processExceptionAtts(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function RuleResponseContentHandler::endElement
  //
  // Description: Over-ridden SAX2 method that captures the end tag event
  //
  // @param uri - The URI of the associated namespace for this element
  // @param localname - The local part of the element name
  // @param qname - The QName of this element
  // @return void
  //--------------------------------------------------------------------------
  void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname)
      override
  {
  }

  //--------------------------------------------------------------------------
  // @function RuleResponseContentHandler::characters
  //
  // Description: Over-ridden SAX2 method that captures characters defined by
  //              the current element.
  //
  // @param chars - The characters from the XML document
  // @param length - The number of characters to read from the array
  // @return void
  //--------------------------------------------------------------------------
  void characters(const XMLCh* const chars, const unsigned int length) override {}

private:
  //--------------------------------------------------------------------------
  // @function RuleResponseContentHandler::updateRuleTextMap
  //
  // Description: Insert rule text for a given category/rule type to
  //              the Rule Text Map.
  //
  // @return void
  //--------------------------------------------------------------------------
  void updateRuleTextMap();

  const uint16_t _maxLineLength = 63;
};
} // end namespace tse

