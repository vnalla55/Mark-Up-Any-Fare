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

#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestFailure.h>

#include <OutputSuite.h>
#include <XmlUniformiser.h>

#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/Runner/TimekeepingTestResultCollector.h"
#include "test/Runner/JUnitLikeXmlOutputter.h"
#include "test/Runner/SkipException.h"

class JUnitLikeXmlOutputterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(JUnitLikeXmlOutputterTest);

  CPPUNIT_TEST(testWriteXmlResultWithNoTest);
  CPPUNIT_TEST(testWriteXmlResultWithOneFailure);
  CPPUNIT_TEST(testWriteXmlResultWithOneError);
  CPPUNIT_TEST(testWriteXmlResultWithOneSkipped);
  CPPUNIT_TEST(testWriteXmlResultWithOneSuccess);
  CPPUNIT_TEST(testWriteXmlResultWithThreeFailureTwoErrorsAndTwoSuccess);
  CPPUNIT_TEST(testWriteXmlResultWithThreeFailureTwoErrorsTwoSkippedAndTwoSuccess);
  CPPUNIT_TEST(testWriteXmlResultWithThreeFailureTwoErrorsAndTwoSuccessDifferentSuites);
  CPPUNIT_TEST(testDeleteNamespacesFromPath);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _result = _memH.create<NoTimeResultCollector>();
    _suites = _memH.create<Suites>();
  }

  void tearDown() { _memH.clear(); }

  void testWriteXmlResultWithNoTest()
  {
    std::string expectedXml = "<testsuites></testsuites>";
    CPPUNITTEST_ASSERT_XML_EQUAL(expectedXml, getXml());
  }

  void testWriteXmlResultWithOneFailure()
  {
    addTestFailure("spn::suitename::testname1", "failure1", CppUnit::SourceLine("test.cpp", 3));

    std::string expectedXml =
        "<testsuites>"
        "<testsuite name=\"spn::suitename\" tests=\"1\" failures=\"1\" "
        "errors=\"0\" skipped=\"0\" package=\"Runner\" time=\"0.000000000\">"
        "<testcase classname=\"/Runner/.suitename\" name=\"testname1\" time=\"0.000000000\">"
        "<failure message=\"failure1\n\" type=\"assertion\">"
        "test.cpp:3"
        "</failure>"
        "</testcase>"
        "</testsuite>"
        "</testsuites>";

    CPPUNITTEST_ASSERT_XML_EQUAL(expectedXml, getXml());
  }

  void testWriteXmlResultWithOneError()
  {
    addTestError("suitename::testname1", "message error1");

    std::string expectedXml =
        "<testsuites>"
        "<testsuite name=\"suitename\" tests=\"1\" failures=\"0\" "
        "errors=\"1\" skipped=\"0\" package=\"Runner\" time=\"0.000000000\">"
        "<testcase classname=\"/Runner/.suitename\" name=\"testname1\" time=\"0.000000000\">"
        "<error message=\"message error1\n\" type=\"error\">"
        "</error>"
        "</testcase>"
        "</testsuite>"
        "</testsuites>";

    CPPUNITTEST_ASSERT_XML_EQUAL(expectedXml, getXml());
  }

  void testWriteXmlResultWithOneSkipped()
  {
    addTestSkipped("suitename::testname1");

    std::string expectedXml =
        "<testsuites>"
        "<testsuite name=\"suitename\" tests=\"1\" failures=\"0\" "
        "errors=\"0\" skipped=\"1\" package=\"Runner\" time=\"0.000000000\">"
        "<testcase classname=\"/Runner/.suitename\" name=\"testname1\" time=\"0.000000000\">"
        "<skipped message=\"test case skipped\n\">"
        "</skipped>"
        "</testcase>"
        "</testsuite>"
        "</testsuites>";

    CPPUNITTEST_ASSERT_XML_EQUAL(expectedXml, getXml());
  }

  void testWriteXmlResultWithOneSuccess()
  {
    addTest("spn::suite::testname1");

    std::string expectedXml =
        "<testsuites>"
        "<testsuite name=\"spn::suite\" tests=\"1\" failures=\"0\" "
        "errors=\"0\" skipped=\"0\" package=\"Runner\" time=\"0.000000000\">"
        "<testcase classname=\"/Runner/.suite\" name=\"testname1\" time=\"0.000000000\" >"
        "</testcase>"
        "</testsuite>"
        "</testsuites>";

    CPPUNITTEST_ASSERT_XML_EQUAL(expectedXml, getXml());
  }

  void testWriteXmlResultWithThreeFailureTwoErrorsAndTwoSuccess()
  {
    addTestFailure("suite::test1", "failure1");
    addTestError("suite::test2", "error1");
    addTestFailure("suite::test3", "failure2");
    addTestFailure("suite::test4", "failure3");
    addTest("suite::test5");
    addTestError("suite::test6", "error2");
    addTest("suite::test7");

    std::string expectedXml =
        "<testsuites>"
        "<testsuite name=\"suite\" tests=\"7\" failures=\"3\" "
        "errors=\"2\" skipped=\"0\" package=\"Runner\" time=\"0.000000000\">"
        "<testcase classname=\"/Runner/.suite\" name=\"test1\" time=\"0.000000000\">"
        "<failure message=\"failure1\n\" type=\"assertion\"></failure></testcase>"
        "<testcase classname=\"/Runner/.suite\" name=\"test2\" time=\"0.000000000\">"
        "<error message=\"error1\n\" type=\"error\"></error></testcase>"
        "<testcase classname=\"/Runner/.suite\" name=\"test3\" time=\"0.000000000\">"
        "<failure message=\"failure2\n\" type=\"assertion\"></failure></testcase>"
        "<testcase classname=\"/Runner/.suite\" name=\"test4\" time=\"0.000000000\">"
        "<failure message=\"failure3\n\" type=\"assertion\"></failure></testcase>"
        "<testcase classname=\"/Runner/.suite\" name=\"test5\" time=\"0.000000000\"></testcase>"
        "<testcase classname=\"/Runner/.suite\" name=\"test6\" time=\"0.000000000\">"
        "<error message=\"error2\n\" type=\"error\"></error></testcase>"
        "<testcase classname=\"/Runner/.suite\" name=\"test7\" time=\"0.000000000\"></testcase>"
        "</testsuite>"
        "</testsuites>";

    CPPUNITTEST_ASSERT_XML_EQUAL(expectedXml, getXml());
  }

  void testWriteXmlResultWithThreeFailureTwoErrorsTwoSkippedAndTwoSuccess()
  {
    addTestFailure("suite::test1", "failure1");
    addTestError("suite::test2", "error1");
    addTestFailure("suite::test3", "failure2");
    addTestSkipped("suite::test4");
    addTestFailure("suite::test5", "failure3");
    addTest("suite::test6");
    addTestError("suite::test7", "error2");
    addTestSkipped("suite::test8");
    addTest("suite::test9");

    std::string expectedXml =
        "<testsuites>"
        "<testsuite name=\"suite\" tests=\"9\" failures=\"3\" "
        "errors=\"2\" skipped=\"2\" package=\"Runner\" time=\"0.000000000\">"
        "<testcase classname=\"/Runner/.suite\" name=\"test1\" time=\"0.000000000\">"
        "<failure message=\"failure1\n\" type=\"assertion\"></failure></testcase>"
        "<testcase classname=\"/Runner/.suite\" name=\"test2\" time=\"0.000000000\">"
        "<error message=\"error1\n\" type=\"error\"></error></testcase>"
        "<testcase classname=\"/Runner/.suite\" name=\"test3\" time=\"0.000000000\">"
        "<failure message=\"failure2\n\" type=\"assertion\"></failure></testcase>"
        "<testcase classname=\"/Runner/.suite\" name=\"test4\" time=\"0.000000000\">"
        "<skipped message=\"test case skipped\n\"></skipped></testcase>"
        "<testcase classname=\"/Runner/.suite\" name=\"test5\" time=\"0.000000000\">"
        "<failure message=\"failure3\n\" type=\"assertion\"></failure></testcase>"
        "<testcase classname=\"/Runner/.suite\" name=\"test6\" time=\"0.000000000\"></testcase>"
        "<testcase classname=\"/Runner/.suite\" name=\"test7\" time=\"0.000000000\">"
        "<error message=\"error2\n\" type=\"error\"></error></testcase>"
        "<testcase classname=\"/Runner/.suite\" name=\"test8\" time=\"0.000000000\">"
        "<skipped message=\"test case skipped\n\"></skipped></testcase>"
        "<testcase classname=\"/Runner/.suite\" name=\"test9\" time=\"0.000000000\"></testcase>"
        "</testsuite>"
        "</testsuites>";

    CPPUNITTEST_ASSERT_XML_EQUAL(expectedXml, getXml());
  }

  void testWriteXmlResultWithThreeFailureTwoErrorsAndTwoSuccessDifferentSuites()
  {
    addTestFailure("suiteA::test1", "failure1");
    addTestError("suiteA::test2", "error1");

    addTestFailure("suiteB::test1", "failure2");
    addTestFailure("suiteB::test2", "failure3");

    addTest("suiteC::test1");
    addTestError("suiteC::test2", "error2");
    addTest("suiteC::test3");

    std::string expectedXml =
        "<testsuites>"
        "<testsuite name=\"suiteA\" tests=\"2\" failures=\"1\" "
        "errors=\"1\" skipped=\"0\" package=\"Runner\" time=\"0.000000000\">"
        "<testcase classname=\"/Runner/.suiteA\" name=\"test1\" time=\"0.000000000\">"
        "<failure message=\"failure1\n\" type=\"assertion\"></failure></testcase>"
        "<testcase classname=\"/Runner/.suiteA\" name=\"test2\" time=\"0.000000000\">"
        "<error message=\"error1\n\" type=\"error\"></error></testcase>"
        "</testsuite>"
        "<testsuite name=\"suiteB\" tests=\"2\" failures=\"2\" "
        "errors=\"0\" skipped=\"0\" package=\"Runner\" time=\"0.000000000\">"
        "<testcase classname=\"/Runner/.suiteB\" name=\"test1\" time=\"0.000000000\">"
        "<failure message=\"failure2\n\" type=\"assertion\"></failure></testcase>"
        "<testcase classname=\"/Runner/.suiteB\" name=\"test2\" time=\"0.000000000\">"
        "<failure message=\"failure3\n\" type=\"assertion\"></failure></testcase>"
        "</testsuite>"
        "<testsuite name=\"suiteC\" tests=\"3\" failures=\"0\" "
        "errors=\"1\" skipped=\"0\" package=\"Runner\" time=\"0.000000000\">"
        "<testcase classname=\"/Runner/.suiteC\" name=\"test1\" time=\"0.000000000\"></testcase>"
        "<testcase classname=\"/Runner/.suiteC\" name=\"test2\" time=\"0.000000000\">"
        "<error message=\"error2\n\" type=\"error\"></error></testcase>"
        "<testcase classname=\"/Runner/.suiteC\" name=\"test3\" time=\"0.000000000\"></testcase>"
        "</testsuite>"
        "</testsuites>";

    CPPUNITTEST_ASSERT_XML_EQUAL(expectedXml, getXml());
  }

  void testDeleteNamespacesFromPath()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("AAA/BBB/CCC"), JUnitLikeXmlOutputter::deleteNamespaces("zzz::AAA/aaa::BBB/aaa::bbb::CCC"));
  }

protected:
  std::string getXml()
  {
    CppUnit::OStringStream stream;
    JUnitLikeXmlOutputter outputter(
        *_result, *getRootTestSuite(), stream, "ISO-8859-1", "JUnitLike");
    outputter.write();
    return stream.str();
  }

  CppUnit::TestSuite* getRootTestSuite()
  {
    CppUnit::TestSuite* root = _memH(createTestSuite("Runner"));
    for (Suites::const_iterator i = _suites->begin(); i != _suites->end(); ++i)
      root->addTest(i->second);
    return root;
  }

  void addTest(const std::string& testName)
  {
    CppUnit::Test* test = createTest(testName);

    _result->startTest(test);
    _result->endTest(test);
  }

  void addTestFailure(const std::string& testName,
                      const std::string& message,
                      CppUnit::SourceLine sourceLine = CppUnit::SourceLine())
  {
    addGenericTestFailure(testName, CppUnit::Message(message), sourceLine, false);
  }

  void addTestError(const std::string& testName,
                    const std::string& message,
                    CppUnit::SourceLine sourceLine = CppUnit::SourceLine())
  {
    addGenericTestFailure(testName, CppUnit::Message(message), sourceLine, true);
  }

  void addTestSkipped(const std::string& testName)
  {
    CppUnit::Test* test = createTest(testName);

    _result->startTest(test);
    CppUnit::TestFailure failure(test, new SkipException(), false);
    _result->addFailure(failure);
    _result->endTest(test);
  }

  void addGenericTestFailure(const std::string& testName,
                             CppUnit::Message message,
                             CppUnit::SourceLine sourceLine,
                             bool isError)
  {

    CppUnit::Test* test = createTest(testName);

    _result->startTest(test);
    CppUnit::TestFailure failure(test, new CppUnit::Exception(message, sourceLine), isError);
    _result->addFailure(failure);
    _result->endTest(test);
  }

  CppUnit::Test* createTest(const std::string& testName)
  {
    CppUnit::TestCase* test = new CppUnit::TestCase(testName);
    getTestSuite(testName)->addTest(test);
    return test;
  }

  CppUnit::TestSuite* createTestSuite(const std::string& suiteName)
  {
    return new CppUnit::TestSuite(suiteName);
  }

  std::string getSuiteName(const std::string& testName)
  {
    return testName.substr(0, testName.rfind("::"));
  }

  CppUnit::TestSuite* getTestSuite(const std::string& testName)
  {
    const std::string suiteName = getSuiteName(testName);
    Suites::const_iterator p = _suites->find(suiteName);
    if (p != _suites->end())
      return p->second;
    return (*_suites)[suiteName] = createTestSuite(suiteName);
  }

  class NoTimeResultCollector : public TimekeepingTestResultCollector
  {
  public:
    void startTest(CppUnit::Test* test) { TestResultCollector::startTest(test); }

    void endTest(CppUnit::Test* test) { TestResultCollector::endTest(test); }
  };

  typedef std::map<std::string, CppUnit::TestSuite*> Suites;

  TimekeepingTestResultCollector* _result;
  tse::TestMemHandle _memH;
  Suites* _suites;
};

CPPUNIT_TEST_SUITE_REGISTRATION(JUnitLikeXmlOutputterTest);
