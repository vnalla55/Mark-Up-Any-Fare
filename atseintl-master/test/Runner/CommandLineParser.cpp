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

#include <iostream>

#include "test/Runner/CommandLineParser.h"

namespace po = boost::program_options;

po::options_description
CommandLineParser::getOptionsDescription()
{
  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "produce help message")(
      "xml,x", po::value<std::string>(&_xmlFilename), "set output xml filename")(
      "jxml,j", po::value<std::string>(&_xmlFilename), "set output (junit like) xml filename")(
      "list,l", "print list of test names")(
      "test,t", po::value<std::string>(&_testName), "set test case/suite to run")(
      "lib,L", po::value<std::vector<std::string> >(&_libNames), "set test lib(s) to run")(
      "guard,g", "throw an exception for signal 11");

  return desc;
}

bool
CommandLineParser::parse(int argc, char* argv[]) try
{
  po::positional_options_description p;
  p.add("test", -1);

  po::parsed_options po = po::command_line_parser(argc, argv).options(_desc).positional(p).run();

  po::variables_map vm;
  po::store(po, vm);
  po::notify(vm);

  _guard = vm.count("guard");
  _list = vm.count("list");
  _format = vm.count("jxml") ? JUNIT : CPPUNIT;

  if (!vm.count("help"))
    return true;

  printUsage(argv[0]);
  return false;
}
catch (const std::exception& e)
{
  std::cout << "Caution: " << e.what() << "\n";
  printUsage(argv[0]);
  return false;
}

void
CommandLineParser::printUsage(const std::string& name) const
{
  std::cout << "Usage: " << name << " [options] [test case/suite name]\n" << _desc << "\n";
}
