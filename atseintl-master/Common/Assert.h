//
// Copyright Sabre 2011
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

#pragma once

#include "Util/BranchPrediction.h"

/// Assertion macro.
/// This macro can be used to document assertions in your algorithms. It is
/// implementing the same concept as standard assert but in case of a failure,
/// it throws an exception instead of calling abort.
///
/// Don't overuse it. There is no release build configured for this macro so
/// every conditional will be executed on production, depending on the
/// situation, this may impact performance.
///
/// @note Since this assertion could go without noticice, every failure is
/// also logged at a FATAL level, monitor your log file and make sure your
/// assertions are not failing.
#define TSE_ASSERT(cond)                                                                           \
  (LIKELY(cond) ? static_cast<void>(0)                                                             \
          : (tse::assertionFailed(#cond, __FILE__, __LINE__, __PRETTY_FUNCTION__),                 \
             static_cast<void>(0)))

namespace tse
{
/// Log failed assertion and throw exception.
/// @note Always throws.
[[noreturn]] void
assertionFailed(const char* assertion, const char* file, unsigned int line, const char* function);
}

