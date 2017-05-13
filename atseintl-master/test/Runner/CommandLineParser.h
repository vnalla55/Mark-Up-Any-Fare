//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <string>
#include <vector>

#include <boost/program_options.hpp>

class CommandLineParser
{
public:
  CommandLineParser()
    : _guard(false), _format(CPPUNIT), _list(false), _desc(getOptionsDescription())
  {
  }

  bool parse(int argc, char* argv[]);

  const std::string& xmlFilename() const { return _xmlFilename; }
  const std::string& testName() const { return _testName; }
  const std::vector<std::string>& libNames() const { return _libNames; }

  bool isGuard() const { return _guard; }
  bool isJUnitFormat() const { return _format == JUNIT; }
  bool isList() const { return _list; }

protected:
  enum XmlFormat
  {
    JUNIT,
    CPPUNIT
  };

  void printUsage(const std::string& name) const;
  boost::program_options::options_description getOptionsDescription();

  std::string _xmlFilename;
  std::string _testName;
  std::vector<std::string> _libNames;
  bool _guard;
  bool _format;
  bool _list;
  boost::program_options::options_description _desc;
};

#endif
