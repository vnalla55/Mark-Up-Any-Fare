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

#ifndef TIMEKEEPINGTESTRESULTCOLLECTOR_H
#define TIMEKEEPINGTESTRESULTCOLLECTOR_H

#include <map>

#include <cppunit/TestResultCollector.h>

#include "test/Runner/Timer.h"

namespace CppUnit
{
class Test;
class TestFailure;
}

class TimekeepingTestResultCollector : public CppUnit::TestResultCollector
{
public:
  virtual void startTest(CppUnit::Test* test);
  virtual void addFailure(const CppUnit::TestFailure& failure);
  virtual void endTest(CppUnit::Test* test);
  double getTime(const CppUnit::Test* test) const;
  void cleanSkippedTests();

protected:
  std::map<const CppUnit::Test*, Timer> _times;
};

#endif
