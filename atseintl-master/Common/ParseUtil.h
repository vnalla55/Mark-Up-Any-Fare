//----------------------------------------------------------------------------
//
//  File:        ParseUtil.h
//  Created:     3/14/2006
//  Authors:
//
//  Description: Common functions required for parsing command entry.
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
#pragma once

#include "Common/TseStringTypes.h"

#include <vector>

namespace tse
{

class ParseUtil
{
public:
  static bool parseIndexes(std::vector<uint16_t>& indexVec,
                           const std::string& entries,
                           const std::string& seperator);

}; // class ParseUtil
} // namespace tse
