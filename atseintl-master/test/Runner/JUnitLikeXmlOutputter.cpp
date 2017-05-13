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

#include <map>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <stdexcept>


#include <cppunit/Exception.h>
#include <cppunit/Test.h>
#include <cppunit/TestPath.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/tools/XmlElement.h>
#include <cppunit/portability/Stream.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "test/Runner/IsSkipTest.h"
#include "test/Runner/JUnitLikeXmlOutputter.h"
#include "test/Runner/TimekeepingTestResultCollector.h"

JUnitLikeXmlOutputter::JUnitLikeXmlOutputter(const TimekeepingTestResultCollector& result,
                                             const CppUnit::Test& rootTest,
                                             CppUnit::OStream& stream,
                                             const std::string& encoding,
                                             const std::string& rootTreeNode)
  : _result(result),
    _rootTest(rootTest),
    _stream(stream),
    _xml(encoding),
    _rootTreeNode(rootTreeNode)
{
}

void
JUnitLikeXmlOutputter::write()
{
  create();
  _stream << _xml.toString();
}

void
JUnitLikeXmlOutputter::setStyleSheet(const std::string& styleSheet)
{
  _xml.setStyleSheet(styleSheet);
}

void
JUnitLikeXmlOutputter::setStandalone(bool standalone)
{
  _xml.setStandalone(standalone);
}

struct ByName
{
  bool operator()(const CppUnit::Test* left, const CppUnit::Test* right) const
  {
    return left->getName() < right->getName();
  }
};

typedef std::map<const CppUnit::Test*, const CppUnit::TestFailure*, ByName> BaseTestsSuite;

class JUnitLikeXmlOutputter::TestSuite : private BaseTestsSuite
{
public:
  typedef BaseTestsSuite::const_iterator const_iterator;

  TestSuite() : _failures(0), _errors(0), _skipped(0) {}

  void insert(const CppUnit::Test* test) { (*this)[test] = 0; }

  void insert(const CppUnit::TestFailure* test)
  {
    if (isSkipTest(*test))
      ++_skipped;
    else
    {
      if (test->isError())
        ++_errors;
      else
        ++_failures;
    }

    (*this)[test->failedTest()] = test;
  }

  std::size_t failures() const { return _failures; }
  std::size_t errors() const { return _errors; }
  std::size_t skipped() const { return _skipped; }

  using BaseTestsSuite::size;
  using BaseTestsSuite::begin;
  using BaseTestsSuite::end;

private:
  std::size_t _failures, _errors, _skipped;
};

std::string
JUnitLikeXmlOutputter::getSuiteName(const CppUnit::Test* test) const
{
  std::string::size_type pos = test->getName().rfind("::");
  return test->getName().substr(0, pos);
}

void
JUnitLikeXmlOutputter::splitTestName(const std::string& name,
                                     std::string& className,
                                     std::string& testName) const
{
  std::string::size_type pos = name.rfind("::");
  if (pos != std::string::npos)
  {
    className = name.substr(0, pos);
    testName = name.substr(pos + 2);
  }
  else
    className = testName = name;
}

std::string
JUnitLikeXmlOutputter::deleteNamespaces(std::string str)
{
  std::string::size_type lastColon = std::string::npos;
  while ( (lastColon = str.find_last_of(":")) != std::string::npos)
  {
    std::string::size_type lastSlash = str.substr(0,lastColon).find_last_of("/");
    str.erase(lastSlash+1, lastColon-lastSlash);
  }
  return str;
}

std::string
JUnitLikeXmlOutputter::getClassName(const CppUnit::Test* test) const
{
  const std::string allTests = "/" + _rootTreeNode + "/";

  CppUnit::TestPath path;
  _rootTest.findTestPath(_rootTest.findTest(test->getName()), path);

  std::string name = path.toString();
  std::string::size_type pos = name.find(allTests);
  if (pos != std::string::npos)
    name.erase(pos, allTests.size());

  name = deleteNamespaces(name.substr(0, name.find_last_of("/")));
  name.insert(name.find_last_of("/")+1, ".");
  return name.substr(0, name.rfind(test->getName()) - 1);
}

std::string
JUnitLikeXmlOutputter::getPackageName(const CppUnit::Test* test) const
{
  std::string className = getClassName(test);
  if (className[0] == '/')
    className.erase(className.begin());
  return className.substr(0, className.find('/'));
}

void
JUnitLikeXmlOutputter::create()
{
  std::map<std::string, TestSuite> suites;

  typedef CppUnit::TestResultCollector::Tests::const_iterator TestsIt;
  const CppUnit::TestResultCollector::Tests& tests = _result.tests();

  for (TestsIt i = tests.begin(); i != tests.end(); ++i)
    suites[getSuiteName(*i)].insert(*i);

  typedef CppUnit::TestResultCollector::TestFailures::const_iterator TestFailuresIt;
  const CppUnit::TestResultCollector::TestFailures& failures = _result.failures();

  for (TestFailuresIt i = failures.begin(); i != failures.end(); ++i)
    suites[getSuiteName((*i)->failedTest())].insert(*i);

  CppUnit::XmlElement* root = new CppUnit::XmlElement("testsuites");
  _xml.setRootElement(root);

  typedef std::map<std::string, TestSuite>::const_iterator It;
  for (It i = suites.begin(); i != suites.end(); ++i)
    root->addElement(createTestSuiteElement(i->second, i->first));
}

namespace
{

std::string
toString(const double& time)
{
  std::ostringstream os;
  os << std::setprecision(10) << std::showpoint << time;
  return os.str();
}

} // namespace

CppUnit::XmlElement*
JUnitLikeXmlOutputter::createTestSuiteElement(const TestSuite& suite, const std::string& suiteName)
    const
{
  CppUnit::XmlElement* node = new CppUnit::XmlElement("testsuite");

  node->addAttribute("name", suiteName);
  node->addAttribute("tests", static_cast<int>(suite.size()));
  node->addAttribute("failures", static_cast<int>(suite.failures()));
  node->addAttribute("errors", static_cast<int>(suite.errors()));
  node->addAttribute("skipped", static_cast<int>(suite.skipped()));

  if (suite.size())
    node->addAttribute("package", getPackageName(suite.begin()->first));

  double suiteTime = 0.0;
  for (TestSuite::const_iterator i = suite.begin(); i != suite.end(); ++i)
  {
    node->addElement((i->second) ? createFailedTestCaseElement(i->second)
                                 : createTestCaseElement(i->first));
    suiteTime += _result.getTime(i->first);
  }

  node->addAttribute("time", toString(suiteTime));

  return node;
}

CppUnit::XmlElement*
JUnitLikeXmlOutputter::createTestCaseElement(const CppUnit::Test* test) const
{
  CppUnit::XmlElement* xmlElem = new CppUnit::XmlElement("testcase");

  std::string tName, cName;
  splitTestName(test->getName(), cName, tName);

  xmlElem->addAttribute("classname", getClassName(test));
  xmlElem->addAttribute("name", tName);
  xmlElem->addAttribute("time", toString(_result.getTime(test)));

  return xmlElem;
}

CppUnit::XmlElement*
JUnitLikeXmlOutputter::createFailedTestCaseElement(const CppUnit::TestFailure* fTest) const
{
  CppUnit::XmlElement* xmlElem = createTestCaseElement(fTest->failedTest());

  if (isSkipTest(*fTest))
    xmlElem->addElement(createTestCaseSubElement(fTest, "skipped"));
  else
    xmlElem->addElement((fTest->isError() ? createErrorTestCaseElement(fTest)
                                          : createFailureTestCaseElement(fTest)));

  return xmlElem;
}

std::string
JUnitLikeXmlOutputter::getContent(const CppUnit::TestFailure* fTest) const
{
  CppUnit::OStringStream stream;
  stream << fTest->sourceLine().fileName() << ":" << fTest->sourceLine().lineNumber();
  return stream.str();
}

CppUnit::XmlElement*
JUnitLikeXmlOutputter::createTestCaseSubElement(const CppUnit::TestFailure* fTest,
                                                const std::string name) const
{
  CppUnit::XmlElement* xmlElem = new CppUnit::XmlElement(name);
  xmlElem->addAttribute("message", fTest->thrownException()->what());
  return xmlElem;
}

CppUnit::XmlElement*
JUnitLikeXmlOutputter::createErrorTestCaseElement(const CppUnit::TestFailure* fTest) const
{
  CppUnit::XmlElement* xmlElem = createTestCaseSubElement(fTest, "error");
  xmlElem->addAttribute("type", "error");
  if (fTest->sourceLine().isValid())
    xmlElem->setContent(getContent(fTest));
  return xmlElem;
}

CppUnit::XmlElement*
JUnitLikeXmlOutputter::createFailureTestCaseElement(const CppUnit::TestFailure* fTest) const
{
  CppUnit::XmlElement* xmlElem = createTestCaseSubElement(fTest, "failure");
  xmlElem->addAttribute("type", "assertion");
  if (fTest->sourceLine().isValid())
    xmlElem->setContent(getContent(fTest));
  return xmlElem;
}
