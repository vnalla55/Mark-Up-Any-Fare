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

#include <cppunit/TestFailure.h>
#include <cppunit/portability/Stream.h>

#include "test/Runner/ProgressListener.h"
#include "test/Runner/IsSkipTest.h"

void
ProgressListener::startTest(CppUnit::Test* test)
{
  _lastTestSkip = false;
  CppUnit::BriefTestProgressListener::startTest(test);
}

void
ProgressListener::addFailure(const CppUnit::TestFailure& failure)
{
  if (!(_lastTestSkip = isSkipTest(failure)))
    CppUnit::BriefTestProgressListener::addFailure(failure);
}

void
ProgressListener::endTest(CppUnit::Test* test)
{
  if (_lastTestSkip)
    CppUnit::stdCOut() << " : skip\n";
  else
    CppUnit::BriefTestProgressListener::endTest(test);
}
