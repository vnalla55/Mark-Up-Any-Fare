#ifndef _CONFIG_H
#define _CONFIG_H

#include "Common/Logger.h"

#include <string>

#include <boost/program_options.hpp>
#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

class Config
{
  static log4cxx::LoggerPtr _logger;
  boost::program_options::variables_map _vm;
  boost::program_options::options_description _desc;
  std::string _configFile;
  std::string _DBAccessPath;
  std::string _DataHandleClassName;
  std::string _XMLDataSource;
  std::string _OutputDir;
  std::string _MockClassName;
  std::string _Log4cxxCfgFile;
  std::string _TseTypesFile;
  bool _CreateAccessors;
  bool _CreateDHMockH;
  bool _CreateDHMockCpp;
  bool _CreateDHCpp;

  void outputUsage(const boost::program_options::options_description& desc);
  bool reposrtError(const char* str1, const char* str2 = 0);

public:
  const std::string& DBAccessPath() const { return _DBAccessPath; }
  const std::string& DataHandleClassName() const { return _DataHandleClassName; }
  const std::string& XMLDataSource() const { return _XMLDataSource; }
  const std::string& OutputDir() const { return _OutputDir; }
  const std::string& Log4cxxCfgFile() const { return _Log4cxxCfgFile; }
  const std::string& MockClassName() const { return _MockClassName; }
  const std::string& TseTypesFile() const { return _TseTypesFile; }
  const bool& CreateAccessors() const { return _CreateAccessors; }
  const bool& CreateDHMockH() const { return _CreateDHMockH; }
  const bool& CreateDHMockCpp() const { return _CreateDHMockCpp; }
  const bool& CreateDHCpp() const { return _CreateDHCpp; }

  Config();
  bool read(int argc, char* argv[]);
};
#endif
