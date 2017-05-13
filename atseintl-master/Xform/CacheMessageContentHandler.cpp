#include "Xform/CacheMessageContentHandler.h"

#include "Common/Global.h"
#include "Common/Logger.h"

#include <algorithm>

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

namespace tse
{

static Logger
logger("atseintl.Xforms.CacheMessageContentHandler");

using namespace XERCES_CPP_NAMESPACE;

CacheMessageContentHandler::CacheMessageContentHandler(CacheMessageXMLParser& parser)
  : _parser(parser)
{
}

bool
CacheMessageContentHandler::parse(const char* str)
{
  try { XMLPlatformUtils::Initialize(); }
  catch (const XMLException& e)
  {
    LOG4CXX_ERROR(logger, "Could not initialize XML parser");
    return false;
  }

  const unsigned char* const data = reinterpret_cast<const unsigned char*>(str);
  MemBufInputSource membuf(data, strlen(str), "");

  SAX2XMLReader* reader = XMLReaderFactory::createXMLReader();
  if (reader == nullptr)
  {
    LOG4CXX_ERROR(logger, "Could not create XML reader");
    return false;
  }

  reader->setFeature(XMLUni::fgSAX2CoreValidation, false);
  reader->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);

  reader->setContentHandler(this);
  reader->setErrorHandler(this);

  try { reader->parse(membuf); }
  catch (const XMLException& e)
  {
    LOG4CXX_ERROR(logger, "XML Exception while parsing request");
    delete reader;
    return false;
  }
  catch (const SAXParseException& e)
  {
    LOG4CXX_ERROR(logger, "Sax Parsing exception while parsing request");
    delete reader;
    return false;
  }

  delete reader;
  return true;
}

void
CacheMessageContentHandler::startElement(const XMLCh* uri,
                                         const XMLCh* localname,
                                         const XMLCh* name,
                                         const Attributes& attributes)
{
  _parser.startElement(name, attributes);
  _text.clear();
}

void
CacheMessageContentHandler::endElement(const XMLCh* uri, const XMLCh* localname, const XMLCh* name)
{
  _text.push_back(0);
  _parser.endElement(name, &_text[0]);
}

void
CacheMessageContentHandler::characters(const XMLCh* chars, unsigned int len)
{
  _text.resize(_text.size() + len);
  std::copy(chars, chars + len, _text.end() - len);
}
}
