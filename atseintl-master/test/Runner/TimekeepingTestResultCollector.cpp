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

#include <algorithm>
#include <iterator>

#include <cppunit/TestFailure.h>

#include "test/Runner/TimekeepingTestResultCollector.h"
#include "test/Runner/IsSkipTest.h"

double
TimekeepingTestResultCollector::getTime(const CppUnit::Test* test) const
{
  std::map<const CppUnit::Test*, Timer>::const_iterator i = _times.find(test);
  return i != _times.end() ? i->second.time() : 0.0;
}

void
TimekeepingTestResultCollector::startTest(CppUnit::Test* test)
{
  _times[test].start();
  TestResultCollector::startTest(test);
}

void
TimekeepingTestResultCollector::addFailure(const CppUnit::TestFailure& failure)
{
  bool success = wasSuccessful();
  TestResultCollector::addFailure(failure);

  if (success && isSkipTest(failure))
    TestSuccessListener::reset();
}

void
TimekeepingTestResultCollector::endTest(CppUnit::Test* test)
{
  TestResultCollector::endTest(test);
  _times[test].stop();
}

void
TimekeepingTestResultCollector::cleanSkippedTests()
{
  typedef CppUnit::TestResultCollector::TestFailures::iterator It;
  It end = std::stable_partition(m_failures.begin(), m_failures.end(), IsSkipTest());

  for (It f = m_failures.begin(); f != end; ++f)
  {
    CppUnit::TestResultCollector::Tests::iterator t =
        std::find(m_tests.begin(), m_tests.end(), (*f)->failedTest());
    if (t != m_tests.end())
      m_tests.erase(t);
  }

  m_failures.erase(m_failures.begin(), end);
}
