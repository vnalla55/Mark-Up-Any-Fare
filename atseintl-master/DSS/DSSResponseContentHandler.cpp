//-------------------------------------------------------------------
// Copyright Sabre 2004
// The copyright to the computer program(s) herein
// is the property of Sabre.
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//-------------------------------------------------------------------

#include "DSS/DSSResponseContentHandler.h"

#include "Common/XMLChString.h"
#include "Common/Logger.h"
#include "DSS/FlightCount.h"

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
logger("atseintl.DSS.DSSResponseContentHandler");

const bool
DSSResponseContentHandler::initialize()
{
  try { XMLPlatformUtils::Initialize(); }
  catch (const XMLException& e) { return false; }
  return true;
}

void
DSSResponseContentHandler::startElement(const XMLCh* const uri,
                                        const XMLCh* const localname,
                                        const XMLCh* const qname,
                                        const Attributes& attrs)
{
  char* chTagName = XMLString::transcode(qname);
  std::string tagName(chTagName);
  delete[] chTagName;
  std::transform(tagName.begin(), tagName.end(), tagName.begin(), (int (*)(int))toupper);

  if (tagName == "DSS")
    processResponseAtts(attrs);
  else if (tagName == "FSD")
    processFSDAtts(attrs);
  else if (tagName == "PTM")
    processPTMAtts(attrs);
  else
    LOG4CXX_INFO(logger, "Ignoring Unknown tag: " + tagName);
}

void
DSSResponseContentHandler::processFSDAtts(const Attributes& attrs)
{
  // lint --e{413}
  LOG4CXX_DEBUG(logger, "In processResponseAtts");
  FlightCount* ft(nullptr);
  _trx.dataHandle().get(ft);
  _flightCounts.push_back(ft);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    if (xmlStr == "COD") // carrier code
    {
      ft->_carrier = xmlValue.c_str();
    }
    else if (xmlStr == "ICD") // direct carrier indicator
    {
      ft->_isDirectCarrier = xmlValue == "true";
    }
    else if (xmlStr == "NST") // Non stop count
    {
      ft->_nonStop = atoi(xmlValue.c_str());
    }
    else if (xmlStr == "DIR") // Direct count
    {
      ft->_direct = atoi(xmlValue.c_str());
    }
    else if (xmlStr == "ONL") // online connection
    {
      ft->_onlineConnection = atoi(xmlValue.c_str());
    }
    else if (xmlStr == "ISE") // interline service exists indicator
    {
      ft->_interLineServiceExist = true;
    }
    else if (xmlStr == "ERR") // Error message
    {
      LOG4CXX_ERROR(logger, "DSS Server Failed to Return  Response : ");
    }
    else
      LOG4CXX_WARN(logger, "Unknown tag: " << xmlStr.c_str());
  }
}

const bool
DSSResponseContentHandler::parse(const char* content)
{
  if (!content)
    return false;

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
}
