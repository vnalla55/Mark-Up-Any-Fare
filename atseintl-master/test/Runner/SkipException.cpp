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

#include <limits>

#include "test/Runner/SkipException.h"

namespace
{
const CppUnit::SourceLine
line("", std::numeric_limits<int>::min());
const CppUnit::Message
msg("test case skipped");
}

SkipException::SkipException() : CppUnit::Exception(msg, line) {}

bool
isSkipException(const CppUnit::Exception& e)
{
  return e.message() == msg && e.sourceLine() == line;
}
