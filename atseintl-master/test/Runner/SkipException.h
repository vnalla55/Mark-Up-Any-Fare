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

#ifndef SKIPEXCEPTION_H
#define SKIPEXCEPTION_H

#include <cppunit/Exception.h>

class SkipException : public CppUnit::Exception
{
public:
  SkipException();
};

bool
isSkipException(const CppUnit::Exception& e);

#endif // SKIPEXCEPTION_H
