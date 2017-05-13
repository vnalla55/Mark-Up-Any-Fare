//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/DateTime.h"
#include "DBAccess/raw_ptr.h"

namespace tse
{

class DAOUtils
{
public:
  enum CacheBy
  {
    NODATES = 0,
    DAILY,
    WEEKLY,
    HALFMONTHLY,
    MONTHLY,
    TWOMONTHS,
    THREEMONTHS,
    FOURMONTHS,
    SIXMONTHS,
    YEARLY,
    ERROR
  };

  static void getDateRange(DateTime ticketDate,
                           DateTime& startDate,
                           DateTime& endDate,
                           CacheBy cacheBy);

  static DateTime nextCacheDate(DateTime startDate, CacheBy cacheBy);

  static DateTime firstCacheDate(DateTime& firstCacheDate, time_t& nextCacheDate);

private:

}; // End of class DAOUtils
}// tse

