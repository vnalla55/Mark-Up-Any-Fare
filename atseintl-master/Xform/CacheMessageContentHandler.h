//-------------------------------------------------------------------
//
//  File:        CacheMessageContentHandler.h
//
//  Description: Members and methods for parsing XML cache update requests
//
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

#include "Xform/CacheMessageXMLParser.h"

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

#include <vector>

namespace tse
{

class CacheMessageContentHandler : public xercesc::DefaultHandler
{
public:
  CacheMessageContentHandler(CacheMessageXMLParser& parser);

  bool parse(const char* data);

private:
  void startElement(const XMLCh* uri,
                    const XMLCh* localname,
                    const XMLCh* name,
                    const xercesc::Attributes& attributes) override;

  void endElement(const XMLCh* uri, const XMLCh* localname, const XMLCh* name) override;

  void characters(const XMLCh* chars, unsigned int len) override;

  CacheMessageXMLParser& _parser;

  std::vector<XMLCh> _text;
};
}

