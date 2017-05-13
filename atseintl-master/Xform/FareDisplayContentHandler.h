//-------------------------------------------------------------------
//
//  File:        FareDisplayContentHandler.h
//  Created:     January 29, 2005
//  Authors:     Mike Carroll
//
//  Description: Members and methods for Parsing international XML content
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
#include "Xform/FareDisplayModelMap.h"

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

#include <string>

namespace tse
{
class DataModelMap;

class FareDisplayContentHandler : public xercesc::DefaultHandler
{
public:
  explicit FareDisplayContentHandler(DataModelMap& dataModelMap) : _dataModelMap(dataModelMap) {}

  virtual ~FareDisplayContentHandler() = default;

  const bool parse(const char*& content);

  const std::string& content() const { return _content; }

protected:
  xercesc::SAX2XMLReader* _reader = nullptr;

  DataModelMap& _dataModelMap;
  std::string _encodedContent;
  std::string _content;
  bool _inDts = false;

  //--------------------------------------------------------------------------
  // @function FareDisplayContentHandler::startElement
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
  // @function FareDisplayContentHandler::endElement
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
  // @function FareDisplayContentHandler::characters
  //
  // Description: Over-ridden SAX2 method that captures characters defined by
  //              the current element.
  //
  // @param chars - The characters from the XML document
  // @param length - The number of characters to read from the array
  // @return void
  //--------------------------------------------------------------------------
  void characters(const XMLCh* const xchars, const unsigned int length) override;

private:
  //--------------------------------------------------------------------------
  // @function FareDisplayContentHandler::setupWPRD_DTSContent
  //
  // Description: For WPRD, decompress the DTS section and delete the
  //  pricing responses not selected.
  //--------------------------------------------------------------------------

  void setupWPRD_DTSContent();
};
} // end namespace tse