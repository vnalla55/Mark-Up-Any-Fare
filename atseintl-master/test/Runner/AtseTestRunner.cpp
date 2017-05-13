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

#include <stdexcept>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Exception.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestResult.h>
#include <cppunit/XmlOutputter.h>

#include "test/Runner/AtseTestRunner.h"
#include "test/Runner/ProgressListener.h"
#include "test/Runner/JUnitLikeXmlOutputter.h"
#include "test/Runner/FailuresOutputter.h"
#include "test/Runner/TimekeepingTestResultCollector.h"

class CppUnitXmlOutputter : public CppUnit::XmlOutputter
{
public:
  CppUnitXmlOutputter(CppUnit::TestResultCollector& result,
                      const CppUnit::Test& rootTest,
                      CppUnit::OStream& stream,
                      const std::string& encoding = std::string("ISO-8859-1"))
    : CppUnit::XmlOutputter(&result, stream, encoding), _rootTest(rootTest)
  {
  }

protected:
  const CppUnit::Test& _rootTest;
};

AtseTestRunner::AtseTestRunner(bool isJUnitFormat)
  : _isJUnitFormat(isJUnitFormat),
    _collector(new TimekeepingTestResultCollector),
    _eventManager(new CppUnit::TestResult())
{
  CPPUNIT_REGISTRY_ADD("All Tests", "Atsev2");
  _eventManager->addListener(_collector);
  addTest(CppUnit::TestFactoryRegistry::getRegistry("Atsev2").makeTest());
}

AtseTestRunner::AtseTestRunner(CppUnit::PlugInManager& plugInManager, bool isJUnitFormat)
  : _isJUnitFormat(isJUnitFormat),
    _collector(new TimekeepingTestResultCollector),
    _eventManager(new CppUnit::TestResult())
{
  CPPUNIT_REGISTRY_ADD("All Tests", "Atsev2");
  plugInManager.addListener(_eventManager);
  _eventManager->addListener(_collector);
  addTest(CppUnit::TestFactoryRegistry::getRegistry("Atsev2").makeTest());
}

AtseTestRunner::~AtseTestRunner()
{
  delete _eventManager;
  delete _collector;
}

bool
AtseTestRunner::run(const std::string& fileName, const std::string& testName)
{
  return fileName.empty()
             ? txtExecute(testName)
             : (_isJUnitFormat ? jXmlExecute(fileName, testName) : xmlExecute(fileName, testName));
}

template <typename C>
void
AtseTestRunner::collectTest(C& coll, CppUnit::Test* test)
{
  if (test->getChildTestCount() != 0)
  {
    for (int i = 0; i < test->getChildTestCount(); ++i)
      collectTest(coll, test->getChildTestAt(i));
    return;
  }

  static const CppUnit::Message msg("test case not run");
  CppUnit::TestFailure failTest(test, new CppUnit::Exception(msg), true);
  coll.addFailure(failTest);
  coll.startTest(test);
}

template <typename O, typename C>
void
AtseTestRunner::writeXmlOutput(C& coll, const std::string& fileName)
{
  std::ofstream file(fileName.c_str());
  if (!file.good())
    throw std::runtime_error("Cannot write to file " + fileName);
  O out(coll, *m_suite, file);
  out.write();
}

template <typename O, typename C>
void
AtseTestRunner::writeNotRunXmlOutput(const std::string& fileName)
{
  C coll;
  collectTest(coll, m_suite);
  writeXmlOutput<O>(coll, fileName);
}

bool
AtseTestRunner::xmlExecute(const std::string& fileName, const std::string& testName)
{
  writeNotRunXmlOutput<CppUnitXmlOutputter, TimekeepingTestResultCollector>(fileName);

  bool pass = run(testName, true);
  _collector->cleanSkippedTests(); // CppUnit::XmlOutputter not support skiped tests
  writeXmlOutput<CppUnitXmlOutputter>(*_collector, fileName);
  return pass;
}

bool
AtseTestRunner::jXmlExecute(const std::string& fileName, const std::string& testName)
{
  writeNotRunXmlOutput<JUnitLikeXmlOutputter, TimekeepingTestResultCollector>(fileName);

  bool pass = run(testName, true);
  writeXmlOutput<JUnitLikeXmlOutputter>(*_collector, fileName);
  return pass;
}

bool
AtseTestRunner::txtExecute(const std::string& testName)
{
  ProgressListener coll;
  _eventManager->addListener(&coll);
  return run(testName, false);
}

bool
AtseTestRunner::run(const std::string& testName, bool quiet)
{
  if (quiet)
    std::cout.flush().setstate(std::ios::badbit);

  FailuresOutputter outputter(_collector, std::cout);
  TestRunner::run(*_eventManager, testName);
  std::cout.flush().clear();
  outputter.write();

  return _collector->wasSuccessful();
}

void
AtseTestRunner::printTestsList(const CppUnit::Test* test,
                               const std::string& prefix,
                               std::ostream& os) const
{
  os << prefix << "\\_" << test->getName() << '\n';

  if (test->getChildTestCount() != 0)
    for (int i = 0; i < test->getChildTestCount(); ++i)
      printTestsList(test->getChildTestAt(i), "  " + prefix, os);
}

void
AtseTestRunner::printTestsList(std::ostream& os) const
{
  printTestsList(CppUnit::TestFactoryRegistry::getRegistry("Atsev2").makeTest(), "", os);
}
