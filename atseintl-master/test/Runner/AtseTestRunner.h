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

#ifndef ATSETESTRUNNER_H
#define ATSETESTRUNNER_H

#include <iosfwd>

#include <cppunit/TestRunner.h>
#include <cppunit/plugin/PlugInManager.h>

namespace CppUnit
{
class TestResult;
}

class TimekeepingTestResultCollector;

class AtseTestRunner : protected CppUnit::TestRunner
{
public:
  AtseTestRunner(bool isJUnitFormat);
  AtseTestRunner(CppUnit::PlugInManager& plugInManager, bool isJUnitFormat);
  ~AtseTestRunner();

  using CppUnit::TestRunner::addTest;
  bool run(const std::string& fileName, const std::string& testName);
  void printTestsList(std::ostream& os) const;

protected:
  void registerPackages();

  template <typename C>
  void collectTest(C& coll, CppUnit::Test* test);

  template <typename O, typename C>
  void writeXmlOutput(C& coll, const std::string& fileName);

  template <typename O, typename C>
  void writeNotRunXmlOutput(const std::string& fileName);

  bool xmlExecute(const std::string& fileName, const std::string& testName);
  bool jXmlExecute(const std::string& fileName, const std::string& testName);
  bool txtExecute(const std::string& testName);
  bool run(const std::string& testName, bool quiet);

  void printTestsList(const CppUnit::Test* test, const std::string& prefix, std::ostream& os) const;

  bool _isJUnitFormat;
  TimekeepingTestResultCollector* _collector;
  CppUnit::TestResult* _eventManager;
};

#endif
