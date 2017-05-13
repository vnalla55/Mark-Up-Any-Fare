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

#pragma once

#include "Util/BranchPrediction.h"

#include <log4cxx/logger.h>

#define IS_DEBUG_ENABLED(x) ((x)->isDebugEnabled())

#ifdef DEBUG_LOG_DISABLED

#undef LOG4CXX_DEBUG
#define LOG4CXX_DEBUG(x, y)

#undef IS_DEBUG_ENABLED
#define IS_DEBUG_ENABLED(x) (false)

#endif

#define SUPPRESS_UNUSED_WARNING(var) (void)(var)


namespace tse
{
class Logger : public log4cxx::LoggerPtr
{
public:
  explicit Logger(const char* const name);
};
} // tse
