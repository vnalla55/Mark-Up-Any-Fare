//----------------------------------------------------------------------------
//
//  File:        StackUtil.h
//  Created:     02/25/2005
//  Authors:     KS
//
//  Description: StackTrace Utility Function
//
//  Updates:
//
//  Copyright Sabre 2005
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

#include <string>

namespace tse
{

namespace StackUtil
{
  const int DEFAULT_MAX_DEPTH = 25;

  // getStackTrace allocates memory, while outputStackTrace doesn't.
  // getStackTrace should only be used in cases where we know
  // that dynamic memory allocation is safe.
  std::string getStackTrace(const int maxDepth = DEFAULT_MAX_DEPTH,
                            bool demangle = false);
  void outputStackTrace(const int maxDepth = DEFAULT_MAX_DEPTH);

  int getRawStackTrace(void* frames[], const int maxDepth = DEFAULT_MAX_DEPTH);
  std::string getStackTrace(void* const frames[], const int depth = DEFAULT_MAX_DEPTH,
                            bool demangle = false);

  // https://panthema.net/2008/0901-stacktrace-demangled/
  // returns new line ended string with demangled (if possible) module and
  // function name. Skips (returns empty string) entries referring to StackUtil
  // methods.
  std::string demangleStackEntry(char* frame, char*& functionName,
                                 size_t& functionNameLen);
};

}

