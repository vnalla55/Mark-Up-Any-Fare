// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include "Common/ProgramOptionsParser.h"

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/regex.hpp>

#include <iomanip>
#include <iostream>
#include <locale>
#include <map>

namespace tse
{

namespace
{
  std::map<std::string, std::string> mapOfInformations;

  const unsigned CHARACTERS_FOR_ARGUMENT_NAME = 40u;

  inline bool isLongOption(const std::string& argument)
  {
    const static boost::regex expressionForLongOptionsAssignedInLine("(-)((daemon)|(([a-zA-Z]{2,})(=)(.*))|(help))");
    return boost::regex_match(argument, expressionForLongOptionsAssignedInLine);
  }

  std::string convertArgumentToCompatibleLongOption(const std::string& argument)
  {
    std::string outputArgument(1, '-');
    outputArgument += argument;
    return outputArgument;
  }

  std::vector<std::string>
  convertArgumentsToReadableFormForProgramOptions(int argc, char* argv[])
  {
    std::vector<std::string> outputArguments;
    outputArguments.reserve(argc);

    for(int i=0; i<argc; ++i)
    {
      if(isLongOption(argv[i]))
        outputArguments.push_back(convertArgumentToCompatibleLongOption(argv[i]));
      else
        outputArguments.push_back(argv[i]);
    }

    return outputArguments;
  }

  inline void fillMapOfInformations()
  {
    mapOfInformations.insert(std::make_pair("acport", "APPLICATION_CONSOLE.PORT"));
    mapOfInformations.insert(std::make_pair("bigip", "SERVER_SOCKET_ADP.BIGIP"));
    mapOfInformations.insert(std::make_pair("config", "Config file"));
    mapOfInformations.insert(std::make_pair("daemon", "Daemon mode"));
    mapOfInformations.insert(std::make_pair("define", "Define"));
    mapOfInformations.insert(std::make_pair("connlimit", "SERVER_SOCKET_ADP.CONNLIMIT"));
    mapOfInformations.insert(std::make_pair("connlimitBasis", "SERVER_SOCKET_ADP.CONNLIMIT_BASIS"));
    mapOfInformations.insert(std::make_pair("poolname", "SERVER_SOCKET_ADP.POOLNAME"));
    mapOfInformations.insert(std::make_pair("ipport", "SERVER_SOCKET_ADP.PORT"));
    mapOfInformations.insert(std::make_pair("readyfile", "Ready file"));
    mapOfInformations.insert(std::make_pair("systype", "SERVER_SOCKET_ADP.SYSTYPE"));
    mapOfInformations.insert(std::make_pair("appconsolegroup", "APPLICATION_CONSOLE.GROUPNAME"));
  }

  inline void trimAllElements(std::vector<std::string>& elements)
  {
    using namespace boost;
    range::for_each(elements, bind(trim<std::string>, _1, std::locale()));
  }

  inline std::string getLastTrimmedStringFromVector(const std::vector<std::string>& elements)
  {
    const std::string& laseTrimmedElement = boost::algorithm::trim_copy(elements.back());
    return laseTrimmedElement;
  }

  template<typename DestType, typename SourceType>
  inline DestType castIfDifferentType(const SourceType& valueToCast)
  {
    try
    {
      return boost::lexical_cast<DestType>(valueToCast);
    }
    catch(const boost::bad_lexical_cast& e)
    {
      std::string errorMessage("The argument can't be casted: ");
      errorMessage += e.what();
      throw std::invalid_argument(errorMessage);
    }
  }

  template<>
  inline std::string castIfDifferentType(const std::string& valueToCast)
  {
    return valueToCast;
  }
}

ProgramOptionsParser::ProgramOptionsParser(const std::string& defaultConfigFile)
  : _programOptionsDescription("Usage: tseserver [OPTIONS]..."),
    _outputStream(std::cout),
    _exitOnHelp(true),
    defaultConfigFile(defaultConfigFile)
{
}

ProgramOptionsParser::ProgramOptionsParser(std::ostream& outputStream,
                                           bool exitOnHelp,
                                           const std::string& defaultConfigFile)
  : _programOptionsDescription("Usage: tseserver [OPTIONS]..."),
    _outputStream(outputStream),
    _exitOnHelp(exitOnHelp),
    defaultConfigFile(defaultConfigFile)
{
}

void ProgramOptionsParser::parseArguments(int argc, char* argv[])
{
  setUpPossibleOptions();
  checkPassedArguments(argc, argv);
}

template<typename T>
void ProgramOptionsParser::printInformationsAboutSetValue(const std::string& key,
                                                          const T& value)
{
  if(mapOfInformations.count(key))
    _outputStream << "INFO: " << std::setw(CHARACTERS_FOR_ARGUMENT_NAME) << std::left
                  << mapOfInformations[key] << " [" << value << "] set from command line.\n";
}
template<>
void ProgramOptionsParser::printInformationsAboutSetValue(const std::string& key,
                                                          const std::vector<std::string>& values)
{
  if(mapOfInformations.count(key))
  {
    _outputStream << "INFO: " << std::setw(CHARACTERS_FOR_ARGUMENT_NAME) << std::left
                  << mapOfInformations[key] << " [";
    for (const auto& value : values)
      _outputStream << value << ", ";
    _outputStream << "] set from command line.\n";
  }
}
template<>
void ProgramOptionsParser::printInformationsAboutSetValue(const std::string& key,
                                                          const bool& /*value*/)
{
  if(mapOfInformations.count(key))
    _outputStream << "INFO: "  << std::setw(CHARACTERS_FOR_ARGUMENT_NAME) << std::left
                  << mapOfInformations[key] << " set from command line.\n";
}

template<typename T>
void ProgramOptionsParser::readOptionIfAvailable(const char* optionName,
                                                 T& valueToSet)
{
  if(_variableMap.count(optionName))
  {
    const std::vector<std::string>& vectorOfValues = _variableMap[optionName].as< std::vector<std::string> >();

    /** Old code was resilient for the same program argument provided multiple times,
     *  so the new code behaviour is compatible with previous and last provided value is chosen: **/
    const std::string& lastValueInVector = getLastTrimmedStringFromVector(vectorOfValues);

    valueToSet = castIfDifferentType<T>(lastValueInVector);
    printInformationsAboutSetValue(optionName, valueToSet);
  }
}

template<>
void ProgramOptionsParser::readOptionIfAvailable(const char* optionName,
                                                 std::vector<std::string>& valueToSet)
{
  if(_variableMap.count(optionName))
  {
    valueToSet = _variableMap[optionName].as< std::vector<std::string> >();
    trimAllElements(valueToSet);
    printInformationsAboutSetValue(optionName, valueToSet);
  }
}

template<>
void ProgramOptionsParser::readOptionIfAvailable(const char* optionName,
                                                 bool& valueToSet)
{
  if(_variableMap.count(optionName))
  {
    valueToSet = true;
    printInformationsAboutSetValue(optionName, valueToSet);
  }
}

void ProgramOptionsParser::readVariablesFromArguments(bool& isDaemon,
                                                      std::string& config,
                                                      std::vector<std::string>& defines,
                                                      std::string& readyFile,
                                                      uint16_t& ipport,
                                                      std::string& bigip,
                                                      std::string& poolname,
                                                      std::string& systype,
                                                      uint16_t& acport,
                                                      uint16_t& connlimit,
                                                      std::string& connlimitBasis,
                                                      std::string& groupname)
{
  displayUsage();

  readOptionIfAvailable("acport", acport);
  readOptionIfAvailable("bigip", bigip);
  readOptionIfAvailable("config", config);
  readOptionIfAvailable("daemon", isDaemon);
  readOptionIfAvailable("define", defines);
  readOptionIfAvailable("connlimit", connlimit);
  readOptionIfAvailable("connlimitBasis", connlimitBasis);
  readOptionIfAvailable("appconsolegroup", groupname);
  readOptionIfAvailable("poolname", poolname);
  readOptionIfAvailable("ipport", ipport);
  readOptionIfAvailable("readyfile", readyFile);
  readOptionIfAvailable("systype", systype);
}

void ProgramOptionsParser::displayUsage()
{
  if(_variableMap.count("help"))
  {
    _outputStream << _programOptionsDescription << std::endl;
    if(_exitOnHelp)
      exit(EXIT_SUCCESS);
  }
}

void ProgramOptionsParser::setUpPossibleOptions()
{
  const std::string& configMessage = "use configfile, default './" + defaultConfigFile + '\'';

  using namespace boost::program_options;
  _programOptionsDescription.add_options()
      ("help,h", "display help message")
      ("acport,a", value<std::vector<std::string> >(), "override AppConsole port")
      ("bigip,b",  value<std::vector<std::string> >(), "specify BigIP device")
      ("config,c", value<std::vector<std::string> >(), configMessage.c_str())
      ("daemon,d", "start as a daemon process")
      ("define,D", value<std::vector<std::string> >(), "override a config value")
      ("connlimit,m", value<std::vector<std::string> >(), "specify BigIP connection limit")
      ("connlimitBasis,l", value<std::vector<std::string> >(), "specify BigIP connection limit basis [member|pool]")
      ("appconsolegroup,g", value<std::vector<std::string> >(), "specify AppConsole reporting group name")
      ("poolname,n", value<std::vector<std::string> >(),  "specify BigIP pool name")
      ("ipport,p",  value<std::vector<std::string> >(),   "override server port")
      ("readyfile,r", value<std::vector<std::string> >(), "set \"ready\" sentinel file")
      ("systype,s", value<std::vector<std::string> >(), "specify system type (ProdHybrid, CertHybrid, etc.)")
  ;

  fillMapOfInformations();
}

void ProgramOptionsParser::checkPassedArguments(int argc, char* argv[])
{
  using namespace boost::program_options;

  std::vector<std::string> compatibleArguments
    = convertArgumentsToReadableFormForProgramOptions(argc, argv);

  try
  {
    parsed_options programOptionsParser = command_line_parser(compatibleArguments)
          .options(_programOptionsDescription)
          .allow_unregistered()
          .run();
    store(programOptionsParser, _variableMap);
    notify(_variableMap);
  }
  catch(error& e)
  {
    _outputStream << "Error during arguments parsing: " << e.what() << std::endl;
    std::cerr << "Error during arguments parsing: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}


} // namespace tse
