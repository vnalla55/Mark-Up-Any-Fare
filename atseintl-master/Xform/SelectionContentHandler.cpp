//-------------------------------------------------------------------
//
//  File:        SelectionContentHandler.cpp
//  Created:     January 29, 2005
//  Authors:     Mike Carroll
//
//  Description: Members and methods for Parsing international XML2 content
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

#include "Xform/SelectionContentHandler.h"

#include "Common/Logger.h"
#include "Util/Base64.h"
#include "Util/CompressUtil.h"

#include <algorithm>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

using namespace XERCES_CPP_NAMESPACE;

namespace tse
{
static Logger
logger("atseintl.Xform.SelectionContentHandler");

void
SelectionContentHandler::startElement(const XMLCh* const uri,
                                      const XMLCh* const localname,
                                      const XMLCh* const qname,
                                      const Attributes& attrs)
{
  std::string tagName;
  transcodeToStrUpper(qname, tagName);

  if (tagName == "REQ")
  {
    const XMLCh* tag = XMLString::transcode("A70");
    const XMLCh* value = attrs.getValue(tag);
    delete[] tag;
    if (value != nullptr)
    {
      transcodeToStrUpper(value, _request);
    }
    tag = XMLString::transcode("S81");
    value = attrs.getValue(tag);
    delete[] tag;
    if (value != nullptr)
    {
      std::string selectionStr;
      transcodeToStrUpper(value, selectionStr);
      parseSelection(selectionStr, _selection);
    }
    tag = XMLString::transcode("PBV");
    value = attrs.getValue(tag);
    delete[] tag;
    if (value != nullptr)
    {
      std::string nomatchStr;
      transcodeToStrUpper(value, nomatchStr);
      _noMatch = (nomatchStr.c_str())[0];
    }
    tag = XMLString::transcode("P77");
    value = attrs.getValue(tag);
    delete[] tag;
    if (value != nullptr)
    {
      std::string recordQuote;
      transcodeToStrUpper(value, recordQuote);
      _recordQuote = (recordQuote.c_str())[0];
    }
    tag = XMLString::transcode("P52");
    value = attrs.getValue(tag);
    delete[] tag;
    if (value != nullptr)
    {
      std::string rebook;
      transcodeToStrUpper(value, rebook);
      _rebook = (rebook.c_str())[0];
    }
  }
  else if (tagName == "DTS")
  {
    _inDts = true;
  }
}

void
SelectionContentHandler::endElement(const XMLCh* const uri,
                                    const XMLCh* const localname,
                                    const XMLCh* const qname)
{
  std::string tagName;
  transcodeToStrUpper(qname, tagName);
  if (tagName == "DTS")
  {
    _inDts = false;
    _content = Base64::decode(_encodedContent);
    CompressUtil::decompressBz2(_content);
  }
}

void
SelectionContentHandler::characters(const XMLCh* const xchars, const unsigned int length)
{
  if (_inDts)
  {
    const char* chars = XMLString::transcode(xchars);
    _encodedContent.append(chars);
  }
}

const bool
SelectionContentHandler::parse(const char*& content)
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

void
SelectionContentHandler::transcodeToStrUpper(const XMLCh* const xch, std::string& str)
{
  const char* ch = XMLString::transcode(xch);
  str.assign(ch);
  delete[] ch;
  std::transform(str.begin(), str.end(), str.begin(), (int (*)(int))toupper);
}

void
SelectionContentHandler::parseSelection(const std::string& selectionString,
                                        std::vector<uint16_t>& selectionList)
{
  uint16_t selection = 0;
  size_t numBegin = std::string::npos;
  size_t numSize = std::string::npos;
  size_t index = 0;
  std::string digits = "0123456789";
  for (;;)
  {
    numBegin = selectionString.find_first_of(digits, index);
    if (numBegin == std::string::npos)
      break;
    index = selectionString.find_first_not_of(digits, numBegin);
    numSize = (index == std::string::npos ? selectionString.size() - numBegin : index - numBegin);

    selection = atoi(selectionString.substr(numBegin, numSize).c_str());
    selectionList.push_back(selection);
  }
}
}
