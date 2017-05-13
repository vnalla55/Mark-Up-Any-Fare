#include "FrequentFlyerContentHandler.h"

#include "Common/Logger.h"
#include "DataModel/FrequentFlyerTrx.h"
#include "Util/Algorithm/StrToUpper.h"

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

#include <memory>

using namespace XERCES_CPP_NAMESPACE;

namespace tse
{
static Logger
logger("atseintl.Xform.FrequentFlyerContentHandler");

static const std::string REQUEST_OPTIONS = "REQUESTOPTIONS";
static const std::string CARRIER_CODE = "CarrierCode";

bool
FrequentFlyerContentHandler::parse(const char* content)
{
  bool retCode = true;

  MemBufInputSource mbis((const unsigned char*)content, strlen(content), "");

  std::unique_ptr<xercesc::SAX2XMLReader> reader(XMLReaderFactory::createXMLReader());

  reader->setFeature(XMLUni::fgSAX2CoreValidation, false);
  reader->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);

  reader->setContentHandler(this);
  reader->setErrorHandler(this);

  try
  {
    reader->parse(mbis);
  }
  catch (const XMLException& xmle)
  {
    char* msgTxt = XMLString::transcode(xmle.getMessage());
    LOG4CXX_ERROR(logger, "XMLException: " << msgTxt);
    XMLString::release(&msgTxt);
    retCode = false;
  }
  catch (SAXParseException& spe)
  {
    char* msgTxt = XMLString::transcode(spe.getMessage());
    LOG4CXX_ERROR(logger, "SAXParseException: " << msgTxt);
    XMLString::release(&msgTxt);
    retCode = false;
  }

  return retCode;
}

void
FrequentFlyerContentHandler::setTrxType(DataHandle& dataHandle, Trx*& trx) const
{
  trx = &dataHandle.safe_create<FrequentFlyerTrx>();
}

void
FrequentFlyerContentHandler::setTrx(FrequentFlyerTrx* trx) const
{
  trx->setCxrs(_cxrs);
}

std::string
FrequentFlyerContentHandler::transcodeToStrUpper(const XMLCh* const xch)
{
  std::unique_ptr<const char> ch(XMLString::transcode(xch));
  std::string str;
  str.assign(ch.get());
  alg::toupper(str);
  return str;
}

bool
FrequentFlyerContentHandler::getAttrValue(const xercesc::Attributes& attrs,
                                          const char* const attr,
                                          std::string& value)
{
  std::unique_ptr<const XMLCh> tag(XMLString::transcode(attr));
  const XMLCh* valueLoc = attrs.getValue(tag.get());
  if (valueLoc != nullptr)
  {
    value = transcodeToStrUpper(valueLoc);
    return true;
  }

  return false;
}

void
FrequentFlyerContentHandler::startElement(const XMLCh* const uri,
                                          const XMLCh* const localname,
                                          const XMLCh* const qname,
                                          const xercesc::Attributes& attrs)
{
  std::string value;
  std::string tagName = transcodeToStrUpper(qname);
  if (tagName == REQUEST_OPTIONS)
  {
    if (getAttrValue(attrs, CARRIER_CODE.c_str(), value))
      _cxrs.insert(value);
  }
}

} // namespace tse
