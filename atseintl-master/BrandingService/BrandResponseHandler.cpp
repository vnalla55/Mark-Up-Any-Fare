//-------------------------------------------------------------------
//
//  File:        BrandResponseContentHandler.cpp
//  Created:     Feb 03, 2008
//  Authors:     Mauricio Dantas
//
//  Description: Members and methods for Parsing Brand response content
//
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "BrandingService/BrandResponseHandler.h"

#include "BrandingService/BrandResponseItem.h"
#include "Common/Logger.h"
#include "Common/XMLChString.h"
#include "DataModel/PricingTrx.h"

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

#include <iostream>
#include <stack>
#include <string>
#include <vector>

using namespace XERCES_CPP_NAMESPACE;

namespace tse
{
static Logger
logger("atseintl.Xform.BrandResponseHandler");

BrandResponseHandler::BrandResponseHandler(const PricingTrx& trx,
                                           std::vector<BrandResponseItem*>& brandResponseItemVec)
  : _trx(trx),
    _brandResponseItemVec(brandResponseItemVec),
    _content(EMPTY_STRING()),
    _carrierCodeTMP(EMPTY_STRING()),
    _campaignCodeTMP(EMPTY_STRING())
{
}

BrandResponseHandler::~BrandResponseHandler()
{
  try { XMLPlatformUtils::Terminate(); }
  catch (... /*const XMLException& e*/)
  {
    // Ignore
  }
}

const bool
BrandResponseHandler::initialize()
{
  try { XMLPlatformUtils::Initialize(); }
  catch (const XMLException& e) { return false; }
  return true;
}

void
BrandResponseHandler::startElement(const XMLCh* const uri,
                                   const XMLCh* const localname,
                                   const XMLCh* const qname,
                                   const Attributes& attrs)

{
  char* chTagName = XMLString::transcode(qname);
  std::string tagName(chTagName);
  delete[] chTagName;
  std::transform(tagName.begin(), tagName.end(), tagName.begin(), (int (*)(int))toupper);

  if (tagName == "MSG") // ERROR from service
  {
    processMSGAtts(attrs);
  }
  else if (tagName == "CBR")
  {
    //  LOG4CXX_DEBUG(logger, "Processing CBR tag" );

    processCBRAtts(attrs);
  }
  else if (tagName == "SB1") // SB1 Campaign Code
  {
    _content.clear();
    _inDts = true;
  }
  else if (tagName == "BRN")
  {
    _trx.dataHandle().get(_brp);
    _brandResponseItemVec.push_back(_brp);

    if (_campaignCodeTMP.empty())
    {
      _campaignCodeTMP.append("   ");
    }
    _brp->_campaignCode.assign(_campaignCodeTMP.c_str());
  }
  //        Brand ID             Brand Name         Brand Text         Booking Codes        Campaign
  //        Code
  else if (tagName == "SB2" || tagName == "SB3" || tagName == "SB4" || tagName == "SB5" ||
           tagName == "SB7") //  Brand ID
  {
    _content.clear();
    _inDts = true;
  }
  else if (tagName == "FBC") // FareBasis Codes
  {
    processFBCAtts(attrs);
  }
  else
  {
    LOG4CXX_DEBUG(logger, "Unknown tag: " + tagName);
  }
}

void
BrandResponseHandler::processCBRAtts(const Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In processCBRAtts");

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    LOG4CXX_DEBUG(logger, xmlStr.c_str());

    if (xmlStr == "B10") // Carrier Code
    {
      _carrierCodeTMP.append(xmlValue.c_str());
    }
    else
      LOG4CXX_DEBUG(logger, "Unknown tag: " << xmlStr.c_str());
  }
}

void
BrandResponseHandler::processFBCAtts(const Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In processFBCAtts");
  std::string incExcIndicator;

  incExcIndicator.clear();
  int numAtts = attrs.getLength();

  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    // LOG4CXX_DEBUG(logger, xmlStr.c_str() );

    std::string xmlString = xmlValue.c_str();
    if (xmlStr == "N21") // FareBasis include/exclude indicator
    {
      incExcIndicator = xmlValue.c_str();
    }
    else if (xmlStr == "SB6") // FareBasis list
    {
      std::string::size_type pos = 0, prev_pos = 0;

      while ((pos = xmlString.find_first_of('|', pos)) != std::string::npos)
      {
        if (incExcIndicator == "Y")
        {
          _brp->_includedFClassVec.push_back(xmlString.substr(prev_pos, pos - prev_pos));
        }
        else
        {
          _brp->_excludedFClassVec.push_back(xmlString.substr(prev_pos, pos - prev_pos));
        }
        prev_pos = ++pos;
      }
      // push last fare basis code
      if (incExcIndicator == "Y")
      {
        _brp->_includedFClassVec.push_back(xmlString.substr(prev_pos, pos - prev_pos));
      }
      else
      {
        _brp->_excludedFClassVec.push_back(xmlString.substr(prev_pos, pos - prev_pos));
      }
    }
    else
      LOG4CXX_DEBUG(logger, "Unknown tag: " << xmlStr.c_str());
  }
}

void
BrandResponseHandler::processMSGAtts(const Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In processMSGAtts");

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    if (xmlStr == "S18") // Message test
    {
      LOG4CXX_INFO(logger, "Brand Fare request failed Code: " << xmlValue.c_str());
    }
    else
      LOG4CXX_DEBUG(logger, "Unknown tag: " << xmlStr.c_str());
  }
}

void
BrandResponseHandler::endElement(const XMLCh* const uri,
                                 const XMLCh* const localname,
                                 const XMLCh* const qname)
{
  char* chTagName = XMLString::transcode(localname);
  std::string tagName(chTagName);
  delete[] chTagName;
  std::transform(_content.begin(), _content.end(), _content.begin(), (int (*)(int))toupper);

  if (tagName == "SB1") // SB1 campaignCode
  {
    _campaignCodeTMP = _content;
  }
  else if (tagName == "SB2") // SB2 Brand Code(Brand ID)
  {
    _brp->_brandCode.assign(_content.c_str());
  }
  else if (tagName == "SB3") // SB3 Brand Name
  {
    _brp->_brandName.assign(_content.c_str());
  }
  else if (tagName == "SB4") // SB4  Brand Text
  {
    _brp->_brandText.assign(_content.c_str());
  }
  else if (tagName == "SB5") // SB5 Booking Codes
  {
    std::string::size_type pos = 0, prev_pos = 0;

    while ((pos = _content.find_first_of('|', pos)) != std::string::npos)
    {
      _brp->_bookingCodeVec.push_back(_content.substr(prev_pos, pos - prev_pos));
      prev_pos = ++pos;
    }

    // push last booking code
    _brp->_bookingCodeVec.push_back(_content.substr(prev_pos, 1));
  }
  if (tagName == "SB7") // SB7 campaignCode
  {
    _campaignCodeTMP = _content;
  }

  _inDts = false;
}

void
BrandResponseHandler::characters(const XMLCh* const xchars, const unsigned int length)
{
  // Append characters to _content for processing by method endElement( )

  if (_inDts)
  {
    const char* chars = XMLString::transcode(xchars);
    _content.append(chars);
  }
}

const bool
BrandResponseHandler::parse(const char* content)
{
  if (!content)
  {
    return false;
  }
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
