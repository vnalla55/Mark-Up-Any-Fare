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

#include <cppunit/TextOutputter.h>

#ifndef FAILURESOUTPUTTER_H
#define FAILURESOUTPUTTER_H

class FailuresOutputter : public CppUnit::TextOutputter
{
public:
  FailuresOutputter(CppUnit::TestResultCollector* result, CppUnit::OStream& stream);
  unsigned countSkipTests() const;
  void printFailures();
  void printStatistics();
};

#endif // FAILURESOUTPUTTER_H
