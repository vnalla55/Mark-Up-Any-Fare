//-------------------------------------------------------------------
//
//  File:        FareDisplayContentHandler.cpp
//  Created:     January 29, 2005
//  Authors:     Mike Carroll
//
//  Description: Members and methods for Parsing international XML content
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

#include "Xform/FareDisplayContentHandler.h"

#include "Common/Logger.h"
#include "Util/Base64.h"
#include "Util/CompressUtil.h"

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

#include <iostream>
#include <stack>
#include <string>
#include <vector>

using namespace XERCES_CPP_NAMESPACE;
namespace tse
{
static Logger
logger("atseintl.Xform.FareDisplayContentHandler");

void
FareDisplayContentHandler::startElement(const XMLCh* const uri,
                                        const XMLCh* const localname,
                                        const XMLCh* const qname,
                                        const xercesc::Attributes& attrs)
{
  char* chTagName = XMLString::transcode(qname);
  std::string tagName(chTagName);
  delete[] chTagName;
  std::transform(tagName.begin(), tagName.end(), tagName.begin(), (int (*)(int))toupper);

  if (tagName == "DTS")
  {
    _inDts = true;
  }

  // The type of the tag is not known at this time. Call to classMapEntry
  // to sort it out.
  try
  {
    FareDisplayModelMap& fareDisplayModelMap = dynamic_cast<FareDisplayModelMap&>(_dataModelMap);
    if (!fareDisplayModelMap.classMapEntry(tagName, attrs))
    {
      LOG4CXX_ERROR(logger, "startElement unable to work with tag: " << tagName);
    }
  }
  catch (std::bad_cast&)
  {
    LOG4CXX_ERROR(logger, "DataModelMap is not a Fare Display model map");
  }
}

void
FareDisplayContentHandler::endElement(const XMLCh* const uri,
                                      const XMLCh* const localname,
                                      const XMLCh* const qname)
{
  char* chTagName = XMLString::transcode(qname);
  std::string tagName(chTagName);
  delete[] chTagName;

  if (tagName == "DTS")
  {
    _inDts = false;
    _content = Base64::decode(_encodedContent);
    CompressUtil::decompressBz2(_content);
  }
  else
  {
    try
    {
      FareDisplayModelMap& fareDisplayModelMap = dynamic_cast<FareDisplayModelMap&>(_dataModelMap);
      fareDisplayModelMap.saveMapEntry(tagName);
    }
    catch (std::bad_cast&)
    {
      LOG4CXX_ERROR(logger, "DataModelMap is not a Fare Display model map");
    }
  } // end of else
}

void
FareDisplayContentHandler::characters(const XMLCh* const xchars, const unsigned int length)
{
  if (_inDts)
  {
    const char* chars = XMLString::transcode(xchars);
    _encodedContent.append(chars);
  }
}

const bool
FareDisplayContentHandler::parse(const char*& content)
{
  bool retCode = true;

  MemBufInputSource* mbis =
      new MemBufInputSource((const unsigned char*)content, strlen(content), "");
  _reader = XMLReaderFactory::createXMLReader();

  // Flag settings
  _reader->setFeature(XMLUni::fgSAX2CoreValidation, false);
  _reader->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);

  // Set document handlers
  _reader->setContentHandler(this);
  _reader->setErrorHandler(this);

  try { _reader->parse(*mbis); }
  catch (const XMLException& xmle)
  {
    char* msgTxt = XMLString::transcode(xmle.getMessage());
    LOG4CXX_ERROR(logger, "XMLException: " << msgTxt);
    delete[] msgTxt;
    retCode = false;
  }
  catch (SAXParseException& spe)
  {
    char* msgTxt = XMLString::transcode(spe.getMessage());
    LOG4CXX_ERROR(logger, "SAXParseException: " << msgTxt);
    delete[] msgTxt;
    retCode = false;
  }
  if (mbis)
    delete mbis;
  if (_reader)
  {
    delete _reader;
    _reader = nullptr;
  }

  return retCode;
}
}
