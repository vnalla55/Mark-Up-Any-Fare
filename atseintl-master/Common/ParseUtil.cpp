//----------------------------------------------------------------------------
//
//  File:        ParseUtil.cpp
//  Created:     2/20/2006
//  Authors:     Andrew Ahmad
//
//  Description: Common functions required for ATSE WPA processing.
//
//  Updates:
//
//  Copyright Sabre 2006
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

#include "Common/ParseUtil.h"

namespace tse
{
bool
ParseUtil::parseIndexes(std::vector<uint16_t>& indexVec,
                        const std::string& entries,
                        const std::string& seperator)
{
  size_t indexBeg = 0;
  size_t indexEnd = entries.length();
  const size_t npos = std::string::npos;
  const size_t sepLen = seperator.length();

  try
  {
    for (indexEnd = entries.find(seperator, 0); indexBeg != npos;
         indexBeg = indexEnd + sepLen, indexEnd = entries.find(seperator, indexBeg))
    {
      size_t indexLen = (indexEnd == npos) ? entries.length() - indexBeg : indexEnd - indexBeg;

      if (indexLen == 0)
        return false;

      // format should be <index>[-<index>]{/<index>[-<index>]}...
      // TODO parsing range
      const std::string& indexStr = entries.substr(indexBeg, indexLen);
      size_t rangeSeperatorPos = indexStr.find("-", 0);
      if (rangeSeperatorPos == npos)
      {
        const uint16_t index = uint16_t(atoi(indexStr.c_str()));
        indexVec.push_back(index);
        if (indexEnd == npos)
          break;
        else
          continue;
      }

      const uint16_t indexFrom = uint16_t(atoi(indexStr.substr(0, rangeSeperatorPos).c_str()));
      const uint16_t indexTo =
          uint16_t(atoi(indexStr.substr(rangeSeperatorPos + 1,
                                        indexStr.length() - rangeSeperatorPos - 1).c_str()));
      for (uint16_t index = indexFrom; index <= indexTo; index++)
      {
        indexVec.push_back(index);
      }
      if (indexEnd == npos)
        break;
    }
  }
  catch (...)
  {
    return false;
  };

  return true;
}
}
