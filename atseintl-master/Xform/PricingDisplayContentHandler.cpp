//-------------------------------------------------------------------
//
//  File:        PricingDisplayContentHandler.cpp
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

#include "Xform/PricingDisplayContentHandler.h"

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
logger("atseintl.Xform.PricingDisplayContentHandler");

void
PricingDisplayContentHandler::startElement(const XMLCh* const uri,
                                           const XMLCh* const localname,
                                           const XMLCh* const qname,
                                           const Attributes& attrs)
{
  char* chTagName = XMLString::transcode(qname);
  std::string tagName(chTagName);
  delete[] chTagName;
  std::transform(tagName.begin(), tagName.end(), tagName.begin(), (int (*)(int))toupper);

  // The type of the tag is not known at this time. Call to classMapEntry
  // to sort it out.
  try
  {
    PricingDisplayModelMap& pricingDisplayModelMap =
        dynamic_cast<PricingDisplayModelMap&>(_dataModelMap);
    if (!pricingDisplayModelMap.classMapEntry(tagName, attrs))
    {
      LOG4CXX_ERROR(logger, "startElement unable to work with tag: " << tagName);
    }
  }
  catch (std::bad_cast&)
  {
    LOG4CXX_ERROR(logger, "DataModelMap is not a Pricing Display model map");
  }
}

void
PricingDisplayContentHandler::endElement(const XMLCh* const uri,
                                         const XMLCh* const localname,
                                         const XMLCh* const qname)
{
  char* chTagName = XMLString::transcode(qname);
  std::string tagName(chTagName);
  delete[] chTagName;
  std::transform(tagName.begin(), tagName.end(), tagName.begin(), (int (*)(int))toupper);

  try
  {
    PricingDisplayModelMap& pricingDisplayModelMap =
        dynamic_cast<PricingDisplayModelMap&>(_dataModelMap);
    pricingDisplayModelMap.saveMapEntry(tagName);
  }
  catch (std::bad_cast&)
  {
    LOG4CXX_ERROR(logger, "DataModelMap is not a Pricing Display model map");
  }
}

const bool
PricingDisplayContentHandler::parse(const char*& content)
{
  bool retCode = true;

  MemBufInputSource* mbis =
      new MemBufInputSource((const unsigned char*)content, strlen(content), "");
  if (mbis == nullptr)
  {
    LOG4CXX_FATAL(logger, "Unable to allocate MemBufInputSource");
    return false;
  }

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
