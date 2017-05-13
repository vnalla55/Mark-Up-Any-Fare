//----------------------------------------------------------------------------
//
//  File:        StackUtil.cpp
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
#include "Util/StackUtil.h"

#include <cstdlib>
#include <sstream>

#include <cxxabi.h>
#include <execinfo.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

namespace tse
{

namespace StackUtil
{

std::string
getStackTrace(const int maxDepth, bool demangle)
{
  void* frames[maxDepth];
  const int depth = getRawStackTrace(frames, maxDepth);
  return getStackTrace(frames, depth, demangle);
}

void
outputStackTrace(const int maxDepth)
{
  void* frames[maxDepth];
  const int depth = getRawStackTrace(frames, maxDepth);
  backtrace_symbols_fd(frames, depth, STDERR_FILENO);
}

int
getRawStackTrace(void* frames[], const int maxDepth)
{
  return backtrace(frames, maxDepth);
}

std::string
getStackTrace(void* const frames[], const int depth, bool demangle)
{
  if (depth == 0)
    return "";

  std::ostringstream oss;

  size_t funcNameSize = 256;
  char* funcName = (char*)malloc(funcNameSize);

  char** symbols = backtrace_symbols(frames, depth);

  oss << "Stack trace:\n";
  for (int i = 0; i < depth; ++i)
  {
    if (demangle)
      oss << demangleStackEntry(symbols[i], funcName, funcNameSize);
    else
      oss << symbols[i] << '\n';
  }
  oss << "End of stack trace\n";

  std::free(symbols);
  std::free(funcName);

  return oss.str();
}

std::string
demangleStackEntry(char* frame, char*& functionName, size_t& functionNameLen)
{
  std::ostringstream output;

  char *begin_name = nullptr;
  char *begin_offset = nullptr;
  char *end_offset = nullptr;

  // find parentheses and +address offset surrounding the mangled name:
  // ./module(function+0x15c) [0x8048a6d]
  for (char *p = frame; *p; ++p)
  {
    if (*p == '(')
    {
      begin_name = p;
    }
    else
    {
      if (*p == '+')
      {
        begin_offset = p;
      }
      else
      {
        if (*p == ')' && begin_offset)
        {
          end_offset = p;
          break;
        }
      }
    }
  }

  if (begin_name && begin_offset && end_offset && begin_name < begin_offset)
  {
    *begin_name++ = '\0';
    *begin_offset++ = '\0';
    *end_offset = '\0';

    // mangled name is now in [begin_name, begin_offset) and caller
    // offset in [begin_offset, end_offset). now apply
    // __cxa_demangle():

    char *module_offset = nullptr;
    for (char *p = begin_name; p > frame; --p)
    {
      if (*p == '/')
      {
        module_offset = p + 1;
        break;
      }
    }

    int status;
    char* ret = abi::__cxa_demangle(begin_name, functionName, &functionNameLen, &status);
    if (status == 0)
    {
      functionName = ret; // use possibly realloc()-ed string

      // skip libUtil.so: tse::StackUtil::...
      // tseserver.static: tse::StackUtil::...
      // tseserver.shopping.1: tse::StackUtil::... etc entries
      if (module_offset && (
            (strcmp(module_offset, "libUtil.so") == 0) ||    //dynamic
            (strncmp(module_offset, "tseserver", 9) == 0)))  //static
      {
        if (strncmp(functionName, "tse::StackUtil::", 16) == 0)
        {
          return "";
        }
      }

      output << (module_offset ? module_offset : frame)
             << ": " << functionName << "+" << begin_offset << "\n";
    }
    else
    {
      // demangling failed. Output function name as a C function with
      // no arguments.
      output << (module_offset ? module_offset : frame)
             << ": " << begin_name << "()+" << begin_offset << "\n";
    }
  }
  else
  {
    // couldn't parse the line? print the whole line.
    output << frame << "\n";
  }

  return output.str();
}

} //namespace StackUtil

} //namespace tse
