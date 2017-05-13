//-------------------------------------------------------------------
//
//  File:        TaxContentHandler.cpp
//  Created:     December 22, 2004
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

#include "Xform/TaxContentHandler.h"

#include "Common/Logger.h"

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
logger("atseintl.Xform.TaxContentHandler");

void
TaxContentHandler::startElement(const XMLCh* const uri,
                                const XMLCh* const localname,
                                const XMLCh* const qname,
                                const Attributes& attrs)
{
  _text.clear();
  char* chTagName = XMLString::transcode(localname);
  std::string tagName(chTagName);
  delete[] chTagName;

  // The type of the tag is not known at this time. Call to classMapEntry
  // to sort it out.
  try
  {
    if (!_dataModelMap.classMapEntry(tagName, attrs))
    {
      LOG4CXX_ERROR(logger, "Unable to work with tag: " << tagName);
    }
  }
  catch (std::bad_cast&)
  {
    LOG4CXX_ERROR(logger, "DataModelMap is not a tax model map");
  }
}

void
TaxContentHandler::endElement(const XMLCh* const uri,
                              const XMLCh* const localname,
                              const XMLCh* const qname)
{
  char* chTagName = XMLString::transcode(localname);
  std::string tagName(chTagName);
  delete[] chTagName;

  try
  {
    _dataModelMap.saveMapEntry(tagName, _text);
  }
  catch (std::bad_cast&)
  {
    LOG4CXX_ERROR(logger, "DataModelMap is not a tax model map");
  }
}

void
TaxContentHandler::characters(const XMLCh* const chars, const unsigned int length)
{
  char* text = XMLString::transcode(chars);
  _text = text;
  delete[] text;
}

const bool
TaxContentHandler::parse(const char*& content)
{
  bool retCode = true;

  MemBufInputSource* mbis =
      new MemBufInputSource((const unsigned char*)content, strlen(content), "");
  if (mbis == nullptr)
  {
    std::cerr << "Failed to allocate mbis" << std::endl;
    return false;
  }
  _reader = XMLReaderFactory::createXMLReader();

  // Flag settings
  _reader->setFeature(XMLUni::fgSAX2CoreValidation, false);
  _reader->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);
  _reader->setFeature(XMLUni::fgSAX2CoreNameSpacePrefixes, true);

  // Set document handlers
  _reader->setContentHandler(this);
  _reader->setErrorHandler(this);

  try { _reader->parse(*mbis); }
  catch (const XMLException& xmle)
  {
    char* msgTxt = XMLString::transcode(xmle.getMessage());
    LOG4CXX_ERROR(logger, "XMLException: " << msgTxt);
    LOG4CXX_ERROR(logger, "Content: " << content);
    delete[] msgTxt;
    retCode = false;
  }
  catch (SAXParseException& spe)
  {
    char* msgTxt = XMLString::transcode(spe.getMessage());
    LOG4CXX_ERROR(logger, "SAXParseException: " << msgTxt);
    LOG4CXX_ERROR(logger, "Content: " << content);
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
