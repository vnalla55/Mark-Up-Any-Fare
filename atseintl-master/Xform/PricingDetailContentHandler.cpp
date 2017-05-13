//-------------------------------------------------------------------
//
//  File:        PricingDetailContentHandler.cpp
//  Created:     January 12, 2005
//  Authors:     Mike Carroll
//
//  Description: Members and methods for Parsing WPDF international XML content
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

#include "Xform/PricingDetailContentHandler.h"

#include "Common/Logger.h"
#include "Xform/DataModelMap.h"
#include "Xform/PricingDetailModelMap.h"

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
logger("atseintl.Xform.PricingDetailContentHandler");

void
PricingDetailContentHandler::startElement(const XMLCh* const uri,
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
    PricingDetailModelMap& pricingDetailModelMap =
        dynamic_cast<PricingDetailModelMap&>(_dataModelMap);
    if (!pricingDetailModelMap.classMapEntry(tagName, attrs))
    {
      LOG4CXX_ERROR(logger, "Unable to work with tag: " << tagName);
    }
  }
  catch (std::bad_cast&)
  {
    LOG4CXX_ERROR(logger, "DataModelMap is not a PricingDetail model map");
  }
}

void
PricingDetailContentHandler::endElement(const XMLCh* const uri,
                                        const XMLCh* const localname,
                                        const XMLCh* const qname)
{
  char* chTagName = XMLString::transcode(qname);
  std::string tagName(chTagName);
  delete[] chTagName;
  std::transform(tagName.begin(), tagName.end(), tagName.begin(), (int (*)(int))toupper);

  try
  {
    PricingDetailModelMap& pricingDetailModelMap =
        dynamic_cast<PricingDetailModelMap&>(_dataModelMap);
    pricingDetailModelMap.saveMapEntry(tagName);
  }
  catch (std::bad_cast&)
  {
    LOG4CXX_ERROR(logger, "DataModelMap is not a tax model map");
  }
}

const bool
PricingDetailContentHandler::parse(const char*& content)
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

    // ignore missing DTL tag at end of message
    char* tagPtr = msgTxt + strlen(msgTxt) - 4;
    retCode = (strncmp(tagPtr, "DTL", 3) == 0);

    delete[] msgTxt;
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
