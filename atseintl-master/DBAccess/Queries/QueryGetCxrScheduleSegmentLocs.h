//----------------------------------------------------------------------------
//          File:           QueryGetCxrScheduleSegmentLocs.h
//          Description:    QueryGetCxrScheduleSegmentLocs
//          Created:        11/01/2010
//          Authors:
//
//          Updates:
//
//     � 2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/CxrScheduleSegmentLocs.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetCxrScheduleSegmentLocs : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCxrScheduleSegmentLocs(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCxrScheduleSegmentLocs(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCxrScheduleSegmentLocs() {};

  virtual const char* getQueryName() const override;

  void findCxrScheduleSegmentLocs(CxrScheduleSegmentLocs& info, const CarrierCode& cxr);

  static void initialize();

  const QueryGetCxrScheduleSegmentLocs& operator=(const QueryGetCxrScheduleSegmentLocs& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCxrScheduleSegmentLocs& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCxrScheduleSegmentLocs

} // namespace tse

