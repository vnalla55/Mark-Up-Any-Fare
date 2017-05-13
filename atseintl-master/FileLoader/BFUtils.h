//----------------------------------------------------------------------------
//  File:        BFUtils.h
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

#pragma once

#include "Common/TsePrimitiveTypes.h"

#include <string>
#include <vector>

namespace tse
{
class Logger;

class BFUtils
{
public:
  static time_t getLatestFile(const std::string& dataDir,
                              const std::string& prefix,
                              const std::string& suffix,
                              std::string& latestFile,
                              const int32_t& generation = -1);

  static void getAllFileGenerations(const std::string& dataDir,
                                    const std::string& prefix,
                                    const std::string& suffix,
                                    std::vector<int32_t>& availableGenerations);

  static int getBadEntriesThreshold();

private:
  static Logger _logger;
};
}

