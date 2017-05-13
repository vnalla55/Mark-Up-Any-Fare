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

#ifndef JUNITLIKEXMLOUTPUTTER_H
#define JUNITLIKEXMLOUTPUTTER_H

#include <cppunit/Outputter.h>
#include <cppunit/tools/XmlDocument.h>

namespace CppUnit
{
class Test;
class TestFailure;
class TestResultCollector;
class XmlElement;
} // CppUnit

class TimekeepingTestResultCollector;

class JUnitLikeXmlOutputter : public CppUnit::Outputter
{
public:
  JUnitLikeXmlOutputter(const TimekeepingTestResultCollector& result,
                        const CppUnit::Test& rootTest,
                        CppUnit::OStream& stream,
                        const std::string& encoding = std::string("ISO-8859-1"),
                        const std::string& rootTreeNode = "Atsev2");

  virtual void write();
  void setStyleSheet(const std::string& styleSheet);
  void setStandalone(bool standalone);

  static
  std::string deleteNamespaces(std::string str);

protected:
  class TestSuite;

  void create();
  CppUnit::XmlElement* createTestCaseElement(const CppUnit::Test* test) const;
  CppUnit::XmlElement* createFailedTestCaseElement(const CppUnit::TestFailure* test) const;
  CppUnit::XmlElement*
  createTestSuiteElement(const TestSuite& suite, const std::string& suiteName) const;
  CppUnit::XmlElement*
  createTestCaseSubElement(const CppUnit::TestFailure* fTest, const std::string name) const;
  CppUnit::XmlElement* createFailureTestCaseElement(const CppUnit::TestFailure* fTest) const;
  CppUnit::XmlElement* createErrorTestCaseElement(const CppUnit::TestFailure* fTest) const;

  std::string getSuiteName(const CppUnit::Test* test) const;
  std::string getClassName(const CppUnit::Test* test) const;
  void splitTestName(const std::string& name, std::string& className, std::string& testName) const;
  std::string getContent(const CppUnit::TestFailure* fTest) const;
  std::string getPackageName(const CppUnit::Test* test) const;

  const TimekeepingTestResultCollector& _result;
  const CppUnit::Test& _rootTest;
  CppUnit::OStream& _stream;
  std::string _encoding;
  std::string _styleSheet;
  CppUnit::XmlDocument _xml;
  std::string _rootTreeNode;

private:
  JUnitLikeXmlOutputter(const JUnitLikeXmlOutputter&);
  JUnitLikeXmlOutputter& operator=(const JUnitLikeXmlOutputter&);
};

#endif
