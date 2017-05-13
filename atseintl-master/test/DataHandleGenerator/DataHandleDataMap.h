#ifndef _DATAHANDLEDATAMAP_H
#define _DATAHANDLEDATAMAP_H

#include "Common/Logger.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>

#include <log4cxx/helpers/objectptr.h>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>

using xercesc::Attributes;
using xercesc::DefaultHandler;
using xercesc::SAXParseException;

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

struct XMLDataTags
{
  static const char* DATAHANDLEDATAMAP;
  static const char* FUNCTION;
  static const char* XMLDATA;
  static const char* PARAM;
  static const char* RETURN;
  static const char* FACTORYINCLUDES;
  static const char* INCLUDE;
  static const char* IGNORE;
};

struct DataHandleData
{
  std::vector<std::pair<std::string, std::string> > _params;
  std::string _return;
  DataHandleData();
  DataHandleData(const DataHandleData& r);
  void clear();
};

class DataHandleDataMapParser : public DefaultHandler
{
  static log4cxx::LoggerPtr _logger;
  std::multimap<std::string, DataHandleData>& _DataHandleDataMap;
  std::set<std::string>& _includes;
  std::map<std::string, std::set<std::string> >& _ignores;
  DataHandleData _data;
  std::string _function;
  bool _retCode;
  std::pair<std::string, std::string> _param;
  enum isInParam
  {
    UNKNOWN,
    PARAM,
    RETURN
  } _isInParam;

  class XMLStr
  {
    std::string _string;

  public:
    XMLStr(const XMLCh* str)
    {
      char* chTagName = XERCES_CPP_NAMESPACE::XMLString::transcode(str);
      _string = chTagName;
      XERCES_CPP_NAMESPACE::XMLString::release(&chTagName);
    }
    bool operator==(const std::string& v) { return _string == v; }
    bool operator==(const char* v) { return _string == v; }
    bool operator==(const XMLStr& v) { return _string == v._string; }
    operator std::string() { return _string; }
    const char* c_str() { return _string.c_str(); }
  };
  class XMLUStr
  {
    std::string _string;

  public:
    XMLUStr(const XMLCh* str)
    {
      char* chTagName = XERCES_CPP_NAMESPACE::XMLString::transcode(str);
      _string = chTagName;
      XERCES_CPP_NAMESPACE::XMLString::release(&chTagName);
      std::transform(_string.begin(), _string.end(), _string.begin(), (int (*)(int))toupper);
    }
    bool operator==(const std::string& v) { return _string == v; }
    bool operator==(const char* v) { return _string == v; }
    bool operator==(const XMLUStr& v) { return _string == v._string; }
    operator std::string() { return _string; }
    const char* c_str() { return _string.c_str(); }
  };

protected:
  void startElement(const XMLCh* const uri,
                    const XMLCh* const localname,
                    const XMLCh* const qname,
                    const Attributes& attrs);

#if XERCES_VERSION_MAJOR < 3
  virtual void characters(const XMLCh* const s, const unsigned len);
#else
  virtual void characters(const XMLCh* const s, const XMLSize_t len);
#endif
  virtual void endElement(const XMLCh* const, const XMLCh* const, const XMLCh* const);

  void processDataHandleDataMap(const Attributes& attrs);
  void processFunction(const Attributes& attrs);
  void processXMLData(const Attributes& attrs);
  void processParam(const Attributes& attrs);
  void processReturn(const Attributes& attrs);
  void processFactoryIncludes(const Attributes& attrs);
  void processInclude(const Attributes& attrs);
  void processIgnore(const Attributes& attrs);

  virtual void warning(const SAXParseException& exc);
  virtual void error(const SAXParseException& exc);
  virtual void fatalError(const SAXParseException& exc);

public:
  const bool initialize();
  const bool Parse(const char* file);

  DataHandleDataMapParser(std::multimap<std::string, DataHandleData>& map,
                          std::set<std::string>& includes,
                          std::map<std::string, std::set<std::string> >& ignore);
  ~DataHandleDataMapParser();
};

#endif
