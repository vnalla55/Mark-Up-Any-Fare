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

#ifndef ISSKIPTEST_H
#define ISSKIPTEST_H

#include <cppunit/TestFailure.h>

#include "test/Runner/SkipException.h"

inline bool
isSkipTest(const CppUnit::TestFailure& failure)
{
  return isSkipException(*failure.thrownException());
}

struct IsSkipTest
{
public:
  bool operator()(const CppUnit::TestFailure& failure) const { return isSkipTest(failure); }

  bool operator()(const CppUnit::TestFailure* failure) const { return isSkipTest(*failure); }
};

#endif // ISSKIPTEST_H
