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

#ifndef SKIPTESTCALLER_H
#define SKIPTESTCALLER_H

#include <cppunit/TestCaller.h>

#include "test/Runner/SkipException.h"

template <typename Fixture>
class SkipTestCaller : public CppUnit::TestCaller<Fixture>
{
private:
  typedef void (Fixture::*TestMethod)();

public:
  SkipTestCaller(const std::string& name, TestMethod test, Fixture* fixture)
    : CppUnit::TestCaller<Fixture>(name, test, fixture)
  {
  }

  void runTest() { throw SkipException(); }
};

#endif // SKIPTESTCALLER_H
