//-------------------------------------------------------------------
//
//  File:        DecodeContentHandler.cpp
//  Created:     September 5, 2014
//  Authors:     Roland Kwolek
//
//  Description: Decode request XML parser
//
//
//  Copyright Sabre 2014
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

#include "Xform/DecodeContentHandler.h"

#include "Common/Logger.h"
#include "DataModel/DecodeTrx.h"

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

using namespace XERCES_CPP_NAMESPACE;

namespace tse
{
Logger
DecodeContentHandler::_logger("atseintl.Xform.DecodeContentHandler");

bool
DecodeContentHandler::parse(const char* content)
{
  bool retCode = true;

  MemBufInputSource* mbis;
  try
  {
    mbis = new MemBufInputSource((const unsigned char*)content, strlen(content), "");
  }
  catch (std::bad_alloc&)
  {
    LOG4CXX_FATAL(_logger, "Unable to allocate MemBufInputSource");
    return false;
  }

  _reader = XMLReaderFactory::createXMLReader();

  _reader->setFeature(XMLUni::fgSAX2CoreValidation, false);
  _reader->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);

  _reader->setContentHandler(this);
  _reader->setErrorHandler(this);

  try
  {
    _reader->parse(*mbis);
  }
  catch (const XMLException& xmle)
  {
    char* msgTxt = XMLString::transcode(xmle.getMessage());
    LOG4CXX_ERROR(_logger, "XMLException: " << msgTxt);
    delete[] msgTxt;
    retCode = false;
  }
  catch (SAXParseException& spe)
  {
    char* msgTxt = XMLString::transcode(spe.getMessage());
    LOG4CXX_ERROR(_logger, "SAXParseException: " << msgTxt);
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

bool
DecodeContentHandler::setTrx(DecodeTrx& trx)
{
  if (!isDataValid())
    return false;
  trx.setLocation(_location);

  _billing.updateTransactionIds(trx.transactionId());
  _billing.updateServiceNames(Billing::SVC_TAX);

  Billing* billing = trx.dataHandle().create<Billing>();
  (*billing) = _billing;
  trx.setBilling(billing);

  return true;
}

bool
DecodeContentHandler::setTrxType(DataHandle& dataHandle, Trx*& trx) const
{
  DecodeTrx* decodeTrx = nullptr;

  dataHandle.get(decodeTrx);

  if (!decodeTrx)
    return false;

  trx = decodeTrx;

  return true;
}

void
DecodeContentHandler::startElement(const XMLCh* const uri,
                                   const XMLCh* const localname,
                                   const XMLCh* const qname,
                                   const xercesc::Attributes& attrs)
{
  std::string tagName;
  transcodeToStrUpper(qname, tagName);

  if (tagName == "OPT")
    handleOpt(attrs);
  else if (tagName == "BIL")
    handleBil(attrs);
}

void
DecodeContentHandler::handleOpt(const xercesc::Attributes& attrs)
{
  std::string value;
  getAttrValue(attrs, "RTG", value);
  _location = value;
}

void
DecodeContentHandler::handleBil(const xercesc::Attributes& attrs)
{
  std::string value;
  bool result;

  result = getAttrValue(attrs, "A20", value);
  if (result)
    _billing.userPseudoCityCode() = value;

  result = getAttrValue(attrs, "Q03", value);
  if (result)
    _billing.userStation() = value;

  result = getAttrValue(attrs, "Q02", value);
  if (result)
    _billing.userBranch() = value;

  result = getAttrValue(attrs, "AE0", value);
  if (result)
    _billing.partitionID() = value;

  result = getAttrValue(attrs, "AD0", value);
  if (result)
    _billing.userSetAddress() = value;

  result = getAttrValue(attrs, "C20", value);
  if (result)
    _billing.parentServiceName() = value;

  result = getAttrValue(attrs, "A22", value);
  if (result)
    _billing.aaaCity() = value;

  result = getAttrValue(attrs, "AA0", value);
  if (result)
    _billing.aaaSine() = value;

  result = getAttrValue(attrs, "A70", value);
  if (result)
    _billing.actionCode() = value;

  result = getAttrValue(attrs, "C01", value);
  if (result)
    _billing.clientTransactionID() = Billing::string2transactionId(value.c_str());

  result = getAttrValue(attrs, "C00", value);
  if (result)
    _billing.parentTransactionID() = Billing::string2transactionId(value.c_str());

  result = getAttrValue(attrs, "S0R", value);
  if (result)
    _billing.requestPath() = Billing::string2transactionId(value.c_str());
}

void
DecodeContentHandler::transcodeToStrUpper(const XMLCh* const xch, std::string& str)
{
  const char* ch = XMLString::transcode(xch);
  str.assign(ch);
  delete[] ch;
  std::transform(str.begin(), str.end(), str.begin(), (int (*)(int))toupper);
}

bool
DecodeContentHandler::getAttrValue(const Attributes& attrs,
                                   const char* const attr,
                                   std::string& value)
{
  const XMLCh* tag = XMLString::transcode(attr);
  const XMLCh* valueLoc = attrs.getValue(tag);
  delete[] tag;
  if (valueLoc != nullptr)
  {
    transcodeToStrUpper(valueLoc, value);
    return true;
  }

  return false;
}

} // namespace tse
