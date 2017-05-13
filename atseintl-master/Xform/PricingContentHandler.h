//-------------------------------------------------------------------
//
//  File:        PricingContentHandler.h
//  Created:     January 29, 2005
//  Authors:     Mike Carroll
//
//  Description: Members and methods for Parsing international XML2 content
//
//
//  Updates:
//          03/18/04 - Mike Carroll - file created.
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

#include "DataModel/Trx.h"
#include "Xform/PricingModelMap.h"

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

#include <string>

namespace tse
{
class DataModelMap;

class PricingContentHandler : public xercesc::DefaultHandler
{
public:
  explicit PricingContentHandler(DataModelMap& dataModelMap) : _dataModelMap(dataModelMap) {}

  virtual ~PricingContentHandler() = default;

  //--------------------------------------------------------------------------
  // @function PricingContentHandler::parse
  //
  // Description: Entry point to set content and error handlers.  Initializes
  //              all required XML utilities.  Calls the reader parse routine
  //              to handle the document content.
  //
  // @param chars - The characters from the XML document
  // @param length - The number of characters to read from the array
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  const bool parse(const char*& content);

private:
  xercesc::SAX2XMLReader* _reader = nullptr;

  DataModelMap& _dataModelMap;

  //--------------------------------------------------------------------------
  // @function PricingContentHandler::startElement
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
  // @function PricingContentHandler::endElement
  //
  // Description: Over-ridden SAX2 method that captures the end tag event
  //
  // @param uri - The URI of the associated namespace for this element
  // @param localname - The local part of the element name
  // @param qname - The QName of this element
  // @return void
  //--------------------------------------------------------------------------
  void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) override;

  //--------------------------------------------------------------------------
  // @function PricingContentHandler::characters
  //
  // Description: Over-ridden SAX2 method that captures characters defined by
  //              the current element.
  //
  // @param chars - The characters from the XML document
  // @param length - The number of characters to read from the array
  // @return void
  //--------------------------------------------------------------------------
  void characters(const XMLCh* const chars, const unsigned int length) override {}
};
} // end namespace tse

