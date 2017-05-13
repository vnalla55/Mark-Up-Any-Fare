#include "test/DataHandleGenerator/Config.h"

#include "Common/Logger.h"

#include <fstream>
#include <iostream>

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>

log4cxx::LoggerPtr
Config::_logger(log4cxx::Logger::getLogger("DataHandleParser.Config"));

void
Config::outputUsage(const boost::program_options::options_description& desc)
{
  std::cout << "DataHandleMock generator\n"
            << "\nAuthor: sg893962\n\n" << desc;
}
Config::Config() : _desc("DataHandleMock generator options") {}
bool
Config::read(int argc, char* argv[])
{
  if (argc >= 2 &&
      (!strcmp(argv[1], "-?") || !strcmp(argv[1], "--?") || !strcmp(argv[1], "-h") ||
       !strcmp(argv[1], "--h") || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-help")))
  {
    outputUsage(_desc);
    return false;
  }
  try
  {
    _desc.add_options()("help", "Display this message")(
        "configFile",
        boost::program_options::value<std::string>(&_configFile)->default_value("config.ini"),
        "Optionally provide a path to a file that contains configuration settings")(
        "DBAccessPath",
        boost::program_options::value<std::string>(&_DBAccessPath)
            ->default_value("/vobs/atseintl/DBAccess"),
        "Path to DBAccess directory")("DataHandleClassName",
                                      boost::program_options::value<std::string>(
                                          &_DataHandleClassName)->default_value("DataHandle"),
                                      "DataHandle class name override")(
        "XMLDataSource",
        boost::program_options::value<std::string>(&_XMLDataSource)
            ->default_value("DataHandleDataMap.xml"),
        "Path to the xml containing information used to generate DataHandleMock return values")(
        "OutputDir",
        boost::program_options::value<std::string>(&_OutputDir),
        "Path to the xml containing information used to generate DataHandleMock return values")(
        "MockClassName",
        boost::program_options::value<std::string>(&_MockClassName)
            ->default_value("DataHandleMock"),
        "Output mock class name")("Log4cxxCfgFile",
                                  boost::program_options::value<std::string>(&_Log4cxxCfgFile)
                                      ->default_value("log4cxx.properties"),
                                  "LOG4CXX logger configuration file")(
        "TseTypesFile",
        boost::program_options::value<std::string>(&_TseTypesFile)
            ->default_value("/vobs/atseintl/Common/TsePrimitiveTypes.h"),
        "File with Tse types definitions")(
        "CreateAccessors",
        boost::program_options::value<bool>(&_CreateAccessors)->default_value(false),
        "Create accessors for DataHandleMock Function")(
        "CreateDHMockH",
        boost::program_options::value<bool>(&_CreateDHMockH)->implicit_value("false"),
        "Create DataHandleMock.h")(
        "CreateDHMockCpp",
        boost::program_options::value<bool>(&_CreateDHMockCpp)->implicit_value("false"),
        "Create DataHandleMock.cpp")(
        "CreateDHCpp",
        boost::program_options::value<bool>(&_CreateDHCpp)->implicit_value("false"),
        "Create DataHandle.cpp");

    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, _desc),
                                  _vm);
    boost::program_options::notify(_vm);
  }
  catch (std::exception const& e)
  {
    return reposrtError("Error during comand line parsing", e.what());
  }
  catch (...) { return reposrtError("Unknown exception"); }

  if (_vm.count("configFile"))
  {
    try
    {
      std::ifstream is(_configFile.c_str());
      if (!is.is_open())
      {
        return reposrtError("Unable to open configuration file", _configFile.c_str());
      }
      boost::program_options::store(boost::program_options::parse_config_file(is, _desc), _vm);
      boost::program_options::notify(_vm);
    }
    catch (std::exception const& e)
    {
      return reposrtError("Error during comand line parsing", e.what());
    }
    catch (...) { return reposrtError("Unknown exception"); }
  }
  if (_Log4cxxCfgFile.size())
    log4cxx::PropertyConfigurator::configure(_Log4cxxCfgFile.c_str());
  else
    log4cxx::BasicConfigurator::configure();

  return true;
}
bool
Config::reposrtError(const char* str1, const char* str2)
{
  log4cxx::BasicConfigurator::configure();
  LOG4CXX_ERROR(_logger, str1 << (str2 ? str2 : ""));
  return false;
}
