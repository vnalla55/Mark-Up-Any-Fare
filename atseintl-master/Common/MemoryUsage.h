//----------------------------------------------------------------------------
//
//  Description: get memory usage for this process
//    Author: Adrienne A. Stipe
//
//  Copyright Sabre 2004
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

#include <cstddef>
#include "Util/BranchPrediction.h"

namespace tse
{

class MemoryUsage
{
private:
  // get specified field number from this process' stat file.
  // Returns "" if error or not found.
  static const char* getStatField(const int& fieldNum, char* buffP, size_t buffSize);

public:
  // return virtual memory size (in bytes) for this process.
  // Returns 0 if error or not found.
  static size_t getVirtualMemorySize();
  static size_t getResidentMemorySize();
  static size_t getAvailableMemory();
  static size_t getTotalMemory();
};

} // end tse namespace

