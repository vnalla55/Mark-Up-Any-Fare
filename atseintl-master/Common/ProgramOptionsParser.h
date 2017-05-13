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

#pragma once

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <iosfwd>
#include <string>
#include <vector>

namespace tse
{

class ProgramOptionsParser
{
public:
  ProgramOptionsParser(const std::string& defaultConfigFile = "");
  ProgramOptionsParser(std::ostream& outputStream,
                       bool exitOnHelp,
                       const std::string& defaultConfigFile);

  void parseArguments(int argc, char* argv[]);

  void readVariablesFromArguments(bool& isDaemon,
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
                                  std::string& groupname);

  void displayUsage();

private:

  void setUpPossibleOptions();
  void checkPassedArguments(int argc, char* argv[]);

  template<typename T>
  void readOptionIfAvailable(const char* optionName, T& valueToSet);

  template<typename T>
  void printInformationsAboutSetValue(const std::string& key, const T& value);


  boost::program_options::options_description _programOptionsDescription;
  boost::program_options::variables_map _variableMap;

  std::ostream& _outputStream;
  const bool _exitOnHelp; /** for tests only **/
  const std::string defaultConfigFile;
};

} // namespace tse

