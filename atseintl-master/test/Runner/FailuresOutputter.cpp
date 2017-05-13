//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include <cppunit/TestResultCollector.h>

#include "test/Runner/FailuresOutputter.h"
#include "test/Runner/IsSkipTest.h"

FailuresOutputter::FailuresOutputter(CppUnit::TestResultCollector* result, CppUnit::OStream& stream)
  : CppUnit::TextOutputter(result, stream)
{
}

unsigned
FailuresOutputter::countSkipTests() const
{
  CppUnit::TestResultCollector::TestFailures const& failures = m_result->failures();
  std::ptrdiff_t ans = std::count_if(failures.begin(), failures.end(), IsSkipTest());
  return static_cast<unsigned>(ans);
}

void
FailuresOutputter::printFailures()
{
  int number = 0;
  typedef CppUnit::TestResultCollector::TestFailures::const_iterator It;
  for (It f = m_result->failures().begin(); f != m_result->failures().end(); ++f)
  {
    if (isSkipTest(**f))
      continue;
    m_stream << "\n";
    printFailure(*f, ++number);
  }
}

void
FailuresOutputter::printStatistics()
{
  unsigned skipped = countSkipTests();
  m_stream << "Test Results:\n"
           << "Run:  " << m_result->runTests() << "   Skipped: " << skipped
           << "   Failures: " << m_result->testFailures() - skipped
           << "   Errors: " << m_result->testErrors() << "\n";
}
