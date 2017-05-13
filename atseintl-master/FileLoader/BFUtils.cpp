//----------------------------------------------------------------------------
//  File:        BFUtils.cpp
//  Created:     2009-01-01
//
//  Description: Bound Fare Util class
//
//  Updates:
//
//  Copyright Sabre 2009
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

#include "FileLoader/BFUtils.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"

#include <iostream>

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

namespace tse
{

Logger
BFUtils::_logger("atseintl.BFCacheUpdate");

namespace
{
ConfigurableValue<int>
badEntriesThreshold("BOUND_FARE", "BAD_ENTRIES_THRESHOLD", 5);
}

time_t
BFUtils::getLatestFile(const std::string& dataDir,
                       const std::string& prefix,
                       const std::string& suffix,
                       std::string& latestFile,
                       const int32_t& generation)
{
  time_t timestampOfFoundFile = 0;
  DIR* dir = nullptr;

  std::string newPrefix = prefix;

  if (generation != -1)
  {
    char buffer[8];
    sprintf(buffer, "%07d", generation);

    std::string sGeneration = buffer;
    newPrefix = newPrefix + sGeneration + ".";
  }

  dir = opendir(dataDir.c_str());

  if (nullptr == dir)
  {
    LOG4CXX_ERROR(_logger, "BFUtils::getLatestFile:opendir error:" << strerror(errno));
  }
  else
  {
    struct dirent entry;
    struct dirent* result = nullptr;
    int return_code = 0;

    std::string fileName = "";
    std::string path = "";
    struct stat buf;

    return_code = readdir_r(dir, &entry, &result);

    while ((result != nullptr) && (0 == return_code))
    {
      fileName = entry.d_name;
      path = dataDir + fileName;

      if (0 != stat(path.c_str(), &buf))
      {
        LOG4CXX_ERROR(_logger, "BFUtils::getLatestFile:stat error:" << strerror(errno));
      }
      else
      {
        if ((newPrefix.empty() || (0 == fileName.find(newPrefix))) &&
            (fileName.size() - suffix.size() == fileName.find(suffix)))
        {
          timestampOfFoundFile = buf.st_mtime;
          latestFile = fileName;
          break;
        }
      }

      return_code = readdir_r(dir, &entry, &result);
    }

    if (return_code != 0)
    {
      LOG4CXX_ERROR(_logger, "BFUtils::getLatestFile:readdir_r() error:" << strerror(errno));
    }

    closedir(dir);
  }

  return timestampOfFoundFile;
}

void
BFUtils::getAllFileGenerations(const std::string& dataDir,
                               const std::string& prefix,
                               const std::string& suffix,
                               std::vector<int32_t>& availableGenerations)
{
  DIR* dir = nullptr;

  dir = opendir(dataDir.c_str());

  if (nullptr == dir)
  {
    LOG4CXX_ERROR(_logger, "BFUtils::getLatestFileGeneration:opendir error:" << strerror(errno));
  }
  else
  {
    struct dirent entry;
    struct dirent* result = nullptr;
    int return_code = 0;

    std::string fileName = "";
    std::string path = "";
    struct stat buf;

    return_code = readdir_r(dir, &entry, &result);

    while ((result != nullptr) && (0 == return_code))
    {
      fileName = entry.d_name;
      path = dataDir + fileName;

      if (0 != stat(path.c_str(), &buf))
      {
        LOG4CXX_ERROR(_logger, "BFUtils::getLatestFileGeneration:stat error:" << strerror(errno));
      }
      else
      {
        if ((prefix.empty() || (0 == fileName.find(prefix))) &&
            (fileName.size() - suffix.size() == fileName.find(suffix)) &&
            (fileName.size() != (prefix.size() + suffix.size() - 1)))
        {
          fileName.erase(fileName.begin(), fileName.begin() + prefix.size());
          fileName.erase(fileName.find(suffix), fileName.find(suffix) + suffix.size());

          try
          {
            int32_t generation = atoi(fileName.c_str());

            availableGenerations.push_back(generation);
          }
          catch (...)
          {
            LOG4CXX_ERROR(
                _logger,
                "BFUtils::getLatestFileGeneration() - Conversion of generation number failed");
          }
        }
      }

      return_code = readdir_r(dir, &entry, &result);
    }

    if (return_code != 0)
    {
      LOG4CXX_ERROR(_logger,
                    "BFUtils::getLatestFileGeneration:readdir_r() error:" << strerror(errno));
    }

    closedir(dir);
  }

  std::sort(availableGenerations.begin(), availableGenerations.end());
}

int
BFUtils::getBadEntriesThreshold()
{
  return badEntriesThreshold.getValue();
}
}
