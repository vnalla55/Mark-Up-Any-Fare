//----------------------------------------------------------------------------
//
//  Description: get memory usage for this process
//    Author: Adrienne A. Stipe
//
//  Updates:
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

#include "Common/MemoryUsage.h"
#include "Util/BranchPrediction.h"

#include <boost/algorithm/string.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include <fstream>
#include <iostream>
#include <string>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

namespace tse
{
namespace
{
int _fd = 0;
boost::mutex _fdMutex;

const int RESIDENT_MEM_SIZE = 24;
const int VIRTUAL_MEM_SIZE = 23;
const size_t FAKE_MEMORY_SIZE = 10000000;
const size_t PAGE_SIZE = 4096;

const std::string getStatFilePath()
{
  char fileName[100];
  snprintf(fileName, sizeof(fileName), "/proc/%d/stat", getpid());
  return std::string(fileName);
}

}

const char*
MemoryUsage::getStatField(const int& fieldNum, char* buffP, size_t buffSize)
{
  {
    boost::lock_guard<boost::mutex> g(_fdMutex);

    // default
    *buffP = '\0';

    static const std::string fileName = getStatFilePath();

    // open the stat file or reposition it
    if (_fd == 0)
    {
      _fd = open(fileName.c_str(), O_RDONLY);
    }
    else
    {
      if (UNLIKELY(lseek(_fd, SEEK_SET, 0) == -1))
      {
        return buffP;
      }
    }

    // read the whole file
    if (UNLIKELY(read(_fd, buffP, buffSize) <= 0))
    {
      return buffP;
    }
  }

  // if exists, there is only one line in the file
  int i = 1;
  char* blankP;
  char* posP = buffP;

  // parse off up to the next blank or end of line
  blankP = strchr(posP, ' '); // lint !e668
  while ((i < fieldNum) && (blankP != nullptr))
  {
    posP = blankP + 1;
    blankP = strchr(posP, ' ');
    i++;
  }
  if (LIKELY(i == fieldNum))
  {
    if (LIKELY(blankP != nullptr))
    {
      *blankP = '\0';
    }
    return posP;
  }
  else
  {
    *buffP = '\0';
    return buffP;
  }
}

size_t
MemoryUsage::getVirtualMemorySize()
{
  // go get field string
  char buffer[500];
  const char* fieldP = nullptr;
  fieldP = getStatField(VIRTUAL_MEM_SIZE, buffer, sizeof(buffer));

  // convert to int and return
  if (LIKELY((fieldP != nullptr) && (*fieldP != '\0')))
  {
    return (strtoul(fieldP, nullptr, 10));
  }
  return 0;
}

size_t
MemoryUsage::getResidentMemorySize()
{
  char buffer[500];
  const char* fieldP = getStatField(RESIDENT_MEM_SIZE, buffer, sizeof(buffer));
  if (LIKELY(fieldP != nullptr && *fieldP != '\0'))
  {
    return strtoul(fieldP, nullptr, 10) * PAGE_SIZE;
  }
  return 0;
}

size_t
MemoryUsage::getAvailableMemory()
{
  std::ifstream stream("/proc/meminfo");
  if (UNLIKELY(!stream))
    return FAKE_MEMORY_SIZE;

  const size_t BUFFER_SIZE(2048);
  char buffer[BUFFER_SIZE] = {};
  stream.read(buffer, BUFFER_SIZE - 1);

  boost::iterator_range<char*> fieldNm(boost::find_first(buffer, "MemFree:"));
  if (UNLIKELY(!fieldNm))
    return FAKE_MEMORY_SIZE;

  const size_t freeMem = strtoul(fieldNm.end(), nullptr, 10);

  fieldNm = boost::find_first(buffer, "Buffers:");
  if (UNLIKELY(!fieldNm))
    return FAKE_MEMORY_SIZE;

  const size_t buffers = strtoul(fieldNm.end(), nullptr, 10);

  fieldNm = boost::find_first(buffer, "Cached:");
  if (UNLIKELY(!fieldNm))
    return FAKE_MEMORY_SIZE;

  const size_t cached = strtoul(fieldNm.end(), nullptr, 10);

  return freeMem + buffers + cached;
}

size_t
MemoryUsage::getTotalMemory()
{
  size_t total(FAKE_MEMORY_SIZE);
  std::ifstream stream("/proc/meminfo");
  if (stream)
  {
    const size_t BUFFER_SIZE(2048);
    char buffer[BUFFER_SIZE] = {};
    stream.read(buffer, BUFFER_SIZE);
    boost::iterator_range<char*> fieldNm(boost::find_first(buffer, "MemTotal:"));
    if (fieldNm)
    {
      total = strtoul(fieldNm.end(), nullptr, 10);
    }
  }
  return total;
}
} // tse

