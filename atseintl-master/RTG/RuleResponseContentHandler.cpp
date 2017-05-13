//-------------------------------------------------------------------
//
//  File:        RuleResponseContentHandler.cpp
//  Created:     May 26, 2005
//  Authors:     Mike Carroll
//
//  Description: Members and methods for Parsing Rule response content
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
#include "RTG/RuleResponseContentHandler.h"

#include "Common/XMLChString.h"
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
logger("atseintl.Xform.RuleResponseContentHandler");

RuleResponseContentHandler::~RuleResponseContentHandler()
{
  try { XMLPlatformUtils::Terminate(); }
  catch (... /*const XMLException& e*/)
  {
    // Ignore
  }
}

const bool
RuleResponseContentHandler::initialize()
{
  try { XMLPlatformUtils::Initialize(); }
  catch (const XMLException& e) { return false; }
  return true;
}

void
RuleResponseContentHandler::startElement(const XMLCh* const uri,
                                         const XMLCh* const localname,
                                         const XMLCh* const qname,
                                         const Attributes& attrs)
{
  char* chTagName = XMLString::transcode(qname);
  std::string tagName(chTagName);
  delete[] chTagName;
  std::transform(tagName.begin(), tagName.end(), tagName.begin(), (int (*)(int))toupper);

  if (tagName == "GENERATERULERESPONSE" || tagName == "GENERATERBRESPONSE")
  {
    processResponseAtts(attrs);

    if (_bundledRequest && tagName == "GENERATERULERESPONSE")
    {
      updateRuleTextMap();
    }
  }
  else if (tagName == "GENERATEBUNDLEDRULERESPONSE")
  {
    LOG4CXX_DEBUG(logger, "Received GENERATEBUNDLEDRULERESPONSE tag");

    _bundledRequest = true;
  }
  else if (tagName == "DGN")
  {
    LOG4CXX_DEBUG(logger, "Processing DGN tag");

    processDGNAtts(attrs);
  }
  else if (tagName == "RTR")
  {
    LOG4CXX_DEBUG(logger, "Processing RTR tag");

    _catNum = -1;
    _ruleType = -1;
    _ruleText.clear();

    processRTRAtts(attrs);
  }
  else if (tagName == "GENERICEXCEPTION")
  {
    LOG4CXX_DEBUG(logger, "Processing GENERICEXCEPTION tag");

    processExceptionAtts(attrs);

    if (_bundledRequest)
    {
      updateRuleTextMap();
    }
  }
  else
  {
    LOG4CXX_WARN(logger, "Unknown tag: " + tagName);
  }
}

void
RuleResponseContentHandler::processResponseAtts(const Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In processResponseAtts");

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    LOG4CXX_DEBUG(logger, xmlStr.c_str()); // " = " << xmlValue);

    if (xmlStr == "S43") // Full rule text
      _ruleText.append(xmlValue.c_str());

    else if (xmlStr == "S46") // RTG diagnostic information
      _rtgDiagInfo.append(xmlValue.c_str());

    else if (xmlStr == "S31") // Error message
      _ruleText.append(xmlValue.c_str());

    else if (xmlStr == "Q3Y") // Status code
    {
      if (xmlValue != "0")
      {
        if (_ruleText.empty())
        {
          _ruleText.append("RTG SERVER ERROR");
        }
        else if (_ruleText.length() > _maxLineLength)
        {
          LOG4CXX_WARN(logger, "RTG returned: " << _ruleText);
          _ruleText = "RTG SERVER ERROR";
        }

        LOG4CXX_WARN(logger, "RTG returned error: " << xmlStr.c_str());
      }
    }
    else if (xmlStr == "S41") // Rule text
    {
      _ruleText = xmlValue.c_str();
    }
    /* TODO not yet supported
    else if (xmlStr == "S38")		// Category header
          ;
        else if (xmlStr == "S39")	// Record 2 restriction
          ;
        else if (xmlStr == "S40")	// Combinability scoreboard
          ;
        else if (xmlStr == "S42")	// Same point city pair text
          ;
      */
    else
      LOG4CXX_WARN(logger, "Unknown tag: " << xmlStr.c_str());
  }
}

void
RuleResponseContentHandler::processDGNAtts(const Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In processDGNAtts");

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    LOG4CXX_DEBUG(logger, xmlStr.c_str());

    if (xmlStr == "S46") // RTG diagnostic information
      _rtgDiagInfo.append(xmlValue.c_str());

    else
      LOG4CXX_WARN(logger, "Unknown tag: " << xmlStr.c_str());
  }
}

void
RuleResponseContentHandler::processRTRAtts(const Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In processRTRAtts");

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    LOG4CXX_DEBUG(logger, xmlStr.c_str());

    if (xmlStr == "Q3V") // Category number
      _catNum = atoi(xmlValue.c_str());

    else if (xmlStr == "Q4R") // Rule type
      _ruleType = atoi(xmlValue.c_str());

    else
      LOG4CXX_WARN(logger, "Unknown tag: " << xmlStr.c_str());
  }
}

void
RuleResponseContentHandler::processExceptionAtts(const Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In processExceptionAtts");

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    LOG4CXX_DEBUG(logger, xmlStr.c_str());

    if (xmlStr == "S31") // Error message
    {
      if (_bundledRequest)
      {
        LOG4CXX_ERROR(logger,
                      "Rule text request for category " << _catNum
                                                        << " failed: " << xmlValue.c_str());
      }
      else
      {
        LOG4CXX_ERROR(logger, "Rule text request failed: " << xmlValue.c_str());
      }

      _ruleText.append("** RULE TEXT ERROR DETECTED **");
    }

    else
      LOG4CXX_WARN(logger, "Unknown tag: " << xmlStr.c_str());
  }
}

const bool
RuleResponseContentHandler::parse(const char* content)
{
  if (!content)
    return false;
  bool retCode = true;

  MemBufInputSource* mbis = new MemBufInputSource(
      (const unsigned char*)content, static_cast<unsigned int>(strlen(content)), "");
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
  catch (const SAXParseException& spe)
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

void
RuleResponseContentHandler::updateRuleTextMap()
{
  // Insert rule text for this category/rule type into the map
  if (_catNum != -1 && _ruleType != -1)
  {
    RuleTextMapKey mapKey = std::make_pair(_catNum, _ruleType);

    RuleTextMap::iterator i = _ruleTextMap.find(mapKey);

    if (i != _ruleTextMap.end())
    {
      if (_ruleType == Combinability && i->second != "RTG REQUEST")
      {
        i->second += _ruleText;
      }
      else
      {
        i->second = _ruleText;
      }
    }
    else
    {
      LOG4CXX_DEBUG(logger,
                    "RTG response for category=" << _catNum << " and rule type=" << _ruleType
                                                 << " not found in rule text map");
    }
  }
}
}
