//----------------------------------------------------------------------------
//  File:           QueryGetCxrScheduleSegmentLocs.cpp
//  Description:    QueryGetCxrScheduleSegmentLocs
//  Created:        11/01/2010
//  Authors:
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetCxrScheduleSegmentLocs.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/CxrScheduleSegmentLocs.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCxrScheduleSegmentLocsSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCxrScheduleSegmentLocs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCxrScheduleSegmentLocs"));

std::string QueryGetCxrScheduleSegmentLocs::_baseSQL;
bool QueryGetCxrScheduleSegmentLocs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCxrScheduleSegmentLocs> g_GetCxrScheduleSegmentLocs;

const char*
QueryGetCxrScheduleSegmentLocs::getQueryName() const
{
  return "GETSCHEDULESEGMENTLOCINFO";
}

void
QueryGetCxrScheduleSegmentLocs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCxrScheduleSegmentLocsSQLStatement<QueryGetCxrScheduleSegmentLocs> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCXRSCHEDULESEGMENTLOCS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCxrScheduleSegmentLocs::findCxrScheduleSegmentLocs(CxrScheduleSegmentLocs& info,
                                                           const CarrierCode& cxr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(cxr, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  res.executeQuery(this);

  info.carrier() = cxr;

  while ((row = res.nextRow()))
  {
    QueryGetCxrScheduleSegmentLocsSQLStatement<QueryGetCxrScheduleSegmentLocs>::mapRowToLocInfo(
        row, info.locCodes());
  }

  LOG4CXX_INFO(_logger,
               "GETCXRSCHEDULESEGMENTLOCS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findCxrScheduleSegmentLocs()
}
