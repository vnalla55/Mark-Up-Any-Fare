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

#ifndef PROGRESSLISTENER_H
#define PROGRESSLISTENER_H

#include <cppunit/BriefTestProgressListener.h>

class ProgressListener : public CppUnit::BriefTestProgressListener
{
public:
  ProgressListener() : _lastTestSkip(false) {}

  void startTest(CppUnit::Test* test);
  void addFailure(const CppUnit::TestFailure& failure);
  void endTest(CppUnit::Test* test);

private:
  bool _lastTestSkip;
};

#endif // PROGRESSLISTENER_H
