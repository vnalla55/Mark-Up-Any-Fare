#ifndef _DHGENERATOR_H
#define _DHGENERATOR_H

#include "Common/Logger.h"
#include "test/DataHandleGenerator/DataHandleDataMap.h"
#include "test/DataHandleGenerator/Types.h"

#include <map>
#include <string>

#include <boost/regex.hpp>
#include <log4cxx/helpers/objectptr.h>

class Config;

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

class DHGenerator
{
  static log4cxx::LoggerPtr _logger;
  const Config& _config;
  static std::multimap<std::string, DataHandleData> _dataHandleDataMap;
  static std::set<std::string> _factoryIncludes;
  GlobalNamespace _namespaceHeader;
  GlobalNamespace _namespaceCpp;
  static std::map<Function*, std::string> _accessorMap;
  static bool _createAccessors;
  static bool _accessorCreated;
  std::map<std::string, std::set<std::string> > _ignores;

  class ClassFinder
  {
    boost::regex _expression;
    std::map<std::string, std::string>& _classToFile;

    void loadFile(const std::string& filename, std::string& s);

  public:
    void findClasses(const std::string& path, const std::string& filename);
    ClassFinder(std::map<std::string, std::string>& classToFile);
  };

protected:
  bool parseDataHandleDataCfg();
  bool parseDataHandleHeader();
  bool parseTseTypes();

  bool parseDataHandleCpp();

  bool createMockHeader();
  bool createMockCpp();
  bool createCpp();

  bool adoptHeaderForCpp();

public:
  DHGenerator(const Config& cfg);

  static void getMockFunImplFromXML(const Function* fun, std::ofstream& file, int align);
  static void createDHAccessorMap(const Class* cl);
  static std::map<Function*, std::string>& dataHandleAccessorMap() { return _accessorMap; }
  static bool createAccessors() { return _createAccessors; }

  bool generateMock();
};

#endif
