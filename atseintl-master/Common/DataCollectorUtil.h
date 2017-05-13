//-------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/DateTime.h"

namespace tse
{

class DataCollectorUtil
{
public:
  static std::string format(const DateTime& dateTime, bool includeMillis = true);

private:
  // Placed here so they wont be called
  //
  DataCollectorUtil();
  ~DataCollectorUtil();
  DataCollectorUtil(const DataCollectorUtil& rhs);
  DataCollectorUtil& operator=(const DataCollectorUtil& rhs);
};
}

