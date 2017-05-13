#include "test/DataHandleGenerator/DataHandleDataMap.h"
//Need to touch this file when adding new XML to test/DBAccessMock/DataHandleDataMap.xml

// FIXME: <cstdio> should be included in <log4cxx/logger.h>
#include "Common/Logger.h"

#include <cstdio>

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

const char* XMLDataTags::DATAHANDLEDATAMAP = "DATAHANDLEDATAMAP";
const char* XMLDataTags::FUNCTION = "FUNCTION";
const char* XMLDataTags::XMLDATA = "XMLDATA";
const char* XMLDataTags::PARAM = "PARAM";
const char* XMLDataTags::RETURN = "RETURN";
const char* XMLDataTags::FACTORYINCLUDES = "FACTORYINCLUDES";
const char* XMLDataTags::INCLUDE = "INCLUDE";
const char* XMLDataTags::IGNORE = "IGNORE";

using namespace XERCES_CPP_NAMESPACE;

log4cxx::LoggerPtr
DataHandleDataMapParser::_logger(log4cxx::Logger::getLogger("DataHandleParser.DataHandleDataMap"));

///////////////////////////////////////////////////////////////////////////////
DataHandleData::DataHandleData() {}
DataHandleData::DataHandleData(const DataHandleData& r) : _return(r._return)
{
  std::vector<std::pair<std::string, std::string> >::const_iterator it = r._params.begin();
  std::vector<std::pair<std::string, std::string> >::const_iterator ie = r._params.end();
  for (; it != ie; it++)
    _params.push_back(std::make_pair(it->first, it->second));
}
void
DataHandleData::clear()
{
  _return = "";
  _params.clear();
}
/////////////////////////////////////////////////////////////////////////////////
DataHandleDataMapParser::~DataHandleDataMapParser()
{
  try { XMLPlatformUtils::Terminate(); }
  catch (...) {}
}
DataHandleDataMapParser::DataHandleDataMapParser(
    std::multimap<std::string, DataHandleData>& map,
    std::set<std::string>& includes,
    std::map<std::string, std::set<std::string> >& ignores)
  : _DataHandleDataMap(map), _includes(includes), _ignores(ignores) {};
const bool
DataHandleDataMapParser::initialize()
{
  try { XMLPlatformUtils::Initialize(); }
  catch (const XMLException& xmle)
  {
    XMLStr msgTxt(xmle.getMessage());
    LOG4CXX_ERROR(_logger, "XMLException: " << msgTxt.c_str());
    return false;
  }
  catch (SAXParseException& spe)
  {
    XMLStr msgTxt(spe.getMessage());
    LOG4CXX_ERROR(_logger, "SAXParseException: " << msgTxt.c_str());
    return false;
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger, "Unknown exception");
    return false;
  }
  return true;
}
const bool
DataHandleDataMapParser::Parse(const char* file)
{
  LOG4CXX_DEBUG(_logger, "Parsing xml data file: " << file);
  SAX2XMLReader* reader = XMLReaderFactory::createXMLReader();

  // Flag settings
  reader->setFeature(XMLUni::fgSAX2CoreValidation, true);
  reader->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);

  // Set document handlers
  reader->setContentHandler(this);
  reader->setErrorHandler(this);
  _retCode = true;
  try
  {
    // reader->parse(*mbis);
    reader->parse(file);
  }
  catch (const XMLException& xmle)
  {
    XMLStr msgTxt(xmle.getMessage());
    LOG4CXX_ERROR(_logger, "XMLException: " << msgTxt.c_str());
    _retCode = false;
  }
  catch (SAXParseException& spe)
  {
    XMLStr msgTxt(spe.getMessage());
    LOG4CXX_ERROR(_logger, "SAXParseException: " << msgTxt.c_str());
    _retCode = false;
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger, "Unknown exception");
    _retCode = false;
  }
  if (reader)
    delete reader;
  return _retCode;
}

void
DataHandleDataMapParser::startElement(const XMLCh* const uri,
                                      const XMLCh* const localname,
                                      const XMLCh* const qname,
                                      const Attributes& attrs)
{
  XMLUStr tagName(qname);

  if (tagName == XMLDataTags::DATAHANDLEDATAMAP)
    processDataHandleDataMap(attrs);
  else if (tagName == XMLDataTags::FUNCTION)
    processFunction(attrs);
  else if (tagName == XMLDataTags::XMLDATA)
    processXMLData(attrs);
  else if (tagName == XMLDataTags::PARAM)
    processParam(attrs);
  else if (tagName == XMLDataTags::RETURN)
    processReturn(attrs);
  else if (tagName == XMLDataTags::FACTORYINCLUDES)
    processFactoryIncludes(attrs);
  else if (tagName == XMLDataTags::INCLUDE)
    processInclude(attrs);
  else if (tagName == XMLDataTags::IGNORE)
    processIgnore(attrs);
  else
    LOG4CXX_WARN(_logger, "Unknown tag: " << tagName.c_str());
}

void
DataHandleDataMapParser::endElement(const XMLCh* const uri,
                                    const XMLCh* const localname,
                                    const XMLCh* const qname)
{
  XMLUStr tagName(qname);
  if (tagName == XMLDataTags::PARAM)
  {
    _data._params.push_back(_param);
    _param.second = "";
  }
  else if (tagName == XMLDataTags::XMLDATA)
  {
    if (_data._return.size())
      _DataHandleDataMap.insert(std::make_pair(_function, _data));
    _data.clear();
  }
  _isInParam = UNKNOWN;
}

#if XERCES_VERSION_MAJOR < 3
void
DataHandleDataMapParser::characters(const XMLCh* const chars, const unsigned len)
#else
void
DataHandleDataMapParser::characters(const XMLCh* const chars, const XMLSize_t)
#endif
{
  if (_isInParam == PARAM)
    _param.second += XMLStr(chars);
  else if (_isInParam == RETURN)
    _data._return += XMLStr(chars);
}

void
DataHandleDataMapParser::processDataHandleDataMap(const Attributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Parsing DataHandleDataMap tag");
}
void
DataHandleDataMapParser::processFunction(const Attributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Parsing Function tag");
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLUStr xmlStr(attrs.getLocalName(i));
    XMLStr xmlValue(attrs.getValue(i));

    if (xmlStr == "NAME")
      _function = xmlValue;
    else
      LOG4CXX_WARN(_logger, "Unknown Function atribute: " << xmlStr.c_str());
  }
}
void
DataHandleDataMapParser::processXMLData(const Attributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Parsing XMLData tag");
}
void
DataHandleDataMapParser::processParam(const Attributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Parsing Param tag");
  int numAtts = attrs.getLength();
  _isInParam = PARAM;
  for (int i = 0; i < numAtts; i++)
  {
    XMLUStr xmlStr(attrs.getLocalName(i));
    XMLStr xmlValue(attrs.getValue(i));

    if (xmlStr == "NAME")
      _param.first = xmlValue;
    else
      LOG4CXX_WARN(_logger, "Unknown Param atribute: " << xmlStr.c_str());
  }
}
void
DataHandleDataMapParser::processReturn(const Attributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Parsing Return tag");
  int numAtts = attrs.getLength();
  _isInParam = RETURN;
  for (int i = 0; i < numAtts; i++)
  {
    XMLUStr xmlStr(attrs.getLocalName(i));
    LOG4CXX_WARN(_logger, "Unknown Return atribute: " << xmlStr.c_str());
  }
}
void
DataHandleDataMapParser::processFactoryIncludes(const Attributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Parsing FactoryIncludes tag");
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLUStr xmlStr(attrs.getLocalName(i));
    LOG4CXX_WARN(_logger, "Unknown FactoryInclueds atribute: " << xmlStr.c_str());
  }
}
void
DataHandleDataMapParser::processInclude(const Attributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Parsing Include tag");
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLUStr xmlStr(attrs.getLocalName(i));
    XMLStr xmlValue(attrs.getValue(i));

    if (xmlStr == "NAME")
      _includes.insert(xmlValue);
    else
      LOG4CXX_WARN(_logger, "Unknown Include atribute: " << xmlStr.c_str());
  }
}
void
DataHandleDataMapParser::processIgnore(const Attributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Parsing Ignore function tag");
  int numAtts = attrs.getLength();
  std::string name;
  std::string type;
  for (int i = 0; i < numAtts; i++)
  {
    XMLUStr xmlStr(attrs.getLocalName(i));
    XMLStr xmlValue(attrs.getValue(i));

    if (xmlStr == "NAME")
      name = xmlValue;
    else if (xmlStr == "TYPE")
      type = xmlValue;
    else
      LOG4CXX_WARN(_logger, "Unknown Ignore function attribute:  " << xmlStr.c_str());
  }
  if (type.size() && name.size())
    _ignores[type].insert(name);
}
void
DataHandleDataMapParser::warning(const SAXParseException& exc)
{
  char* message = XMLString::transcode(exc.getMessage());
  LOG4CXX_WARN(_logger,
               "Warning - Line :" << exc.getLineNumber() << " Column: " << exc.getColumnNumber()
                                  << " - " << message);
  XMLString::release(&message);
}
void
DataHandleDataMapParser::error(const SAXParseException& exc)
{
  char* message = XMLString::transcode(exc.getMessage());
  LOG4CXX_ERROR(_logger,
                "Error - Line :" << exc.getLineNumber() << " Column: " << exc.getColumnNumber()
                                 << " - " << message);
  XMLString::release(&message);
  _retCode = false;
}
void
DataHandleDataMapParser::fatalError(const SAXParseException& exc)
{
  char* message = XMLString::transcode(exc.getMessage());
  LOG4CXX_ERROR(_logger,
                "Fatal Error - Line :" << exc.getLineNumber()
                                       << " Column: " << exc.getColumnNumber() << " - " << message);
  XMLString::release(&message);
  _retCode = false;
}
