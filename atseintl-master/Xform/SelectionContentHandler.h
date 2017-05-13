//-------------------------------------------------------------------
//
//  File:        SelectionContentHandler.h
//  Created:     March 14, 2006
//  Authors:     Greg Graham
//
//  Description: Members and methods for Parsing international XML2 content
//
//
//  Copyright Sabre 2006
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

#include "Common/TseStringTypes.h"

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

#include <string>
#include <vector>

namespace tse
{

class SelectionContentHandler : public xercesc::DefaultHandler
{
public:
  virtual ~SelectionContentHandler() = default;

  const bool parse(const char*& content);

  const std::string& content() const { return _content; }

  const std::string& getRequest() const { return _request; }

  const std::vector<uint16_t>& selection() const { return _selection; }

  const bool nomatch() const { return _noMatch == 'T'; }

  const bool recordQuote() const { return _recordQuote == 'T'; }
  const bool rebook() const { return _rebook == 'T'; }

protected:
  xercesc::SAX2XMLReader* _reader = nullptr;
  std::vector<uint16_t> _selection;
  std::string _request;
  std::string _encodedContent;
  std::string _content;
  bool _inDts = false;
  char _noMatch = 0;
  char _recordQuote = 0;
  char _rebook = 0;

  //--------------------------------------------------------------------------
  // @function SelectionContentHandler::startElement
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
  // @function SelectionContentHandler::endElement
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
  // @function SelectionContentHandler::characters
  //
  // Description: Over-ridden SAX2 method that captures characters defined by
  //              the current element.
  //
  // @param chars - The characters from the XML document
  // @param length - The number of characters to read from the array
  // @return void
  //--------------------------------------------------------------------------
  void characters(const XMLCh* const chars, const unsigned int length) override;

private:
  void transcodeToStrUpper(const XMLCh* const xch, std::string& str);

  void parseSelection(const std::string& selectionString, std::vector<uint16_t>& selectionList);
};
} // end namespace tse

