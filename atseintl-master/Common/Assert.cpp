//
// Copyright Sabre 2011-12-05
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//

#include "Common/Assert.h"

#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Util/StackUtil.h"

#include <sstream>

namespace
{
tse::Logger
logger("atseintl.Assert");
}

namespace tse
{

void
assertionFailed(const char* assertion, const char* file, unsigned int line, const char* function)
{
  std::ostringstream msg;
  msg << "Assertion (" << assertion << ") failed";
  if (function)
    msg << " in function " << function;
  msg << " at " << file << ":" << line;

  LOG4CXX_FATAL(logger, msg.str().c_str());

#ifndef NDEBUG
  //NDEBUG used to avoid configuration/transaction dependency when something is
  //wrong. scons TYPE=release build defines NDEBUG so release type logs will not
  //be impacted by stack printout.
  const int MAX_DEPTH = 63;
  LOG4CXX_FATAL(logger, StackUtil::getStackTrace(MAX_DEPTH, true));
#endif

  throw ErrorResponseException(ErrorResponseException::SYSTEM_EXCEPTION, "SYSTEM EXCEPTION");
}
}
