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

#ifndef CPPUNITSKIPTEST_H
#define CPPUNITSKIPTEST_H

#include <cppunit/extensions/HelperMacros.h>

#include "test/Runner/SkipTestCaller.h"

#define CPPUNIT_SKIP_TEST(_testMethod_)                                                            \
  CPPUNIT_TEST_SUITE_ADD_TEST(                                                                     \
      (new SkipTestCaller<TestFixtureType>(context.getTestNameFor(#_testMethod_),                  \
                                           &TestFixtureType::_testMethod_,                         \
                                           context.makeFixture())))

#endif // CPPUNITSKIPTEST_H
