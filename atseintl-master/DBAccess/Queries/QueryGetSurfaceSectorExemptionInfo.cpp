//----------------------------------------------------------------------------
//  File:           QueryGetSurfaceSectorExemptionInfo.cpp
//  Description:    QueryGetSurfaceSectorExemptionInfo
//  Created:        1/13/2008
//  Author:         Marcin Augustyniak
//
//
// � 2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetSurfaceSectorExemptionInfo.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSurfaceSectorExemptionInfoSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSurfaceSectorExemptionInfo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetSurfaceSectorExemptionInfo"));
std::string QueryGetSurfaceSectorExemptionInfo::_baseSQL;
bool QueryGetSurfaceSectorExemptionInfo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSurfaceSectorExemptionInfo> g_GetSurfaceSectorExemptionInfo;

const char*
QueryGetSurfaceSectorExemptionInfo::getQueryName() const
{
  return "GETSURFACESECTOREXEMPTIONINFO";
}

void
QueryGetSurfaceSectorExemptionInfo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSurfaceSectorExemptionInfoSQLStatement<QueryGetSurfaceSectorExemptionInfo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSURFACESECTOREXEMPTIONINFO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSurfaceSectorExemptionInfo::findSurfaceSectorExemptionInfo(
    std::vector<SurfaceSectorExemptionInfo*>* sseInfoVec, const CarrierCode& validatingCarrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, validatingCarrier);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  SurfaceSectorExemptionInfo* sseInfo = nullptr;
  SurfaceSectorExemptionInfo* sseInfoPrev = nullptr;
  while ((row = res.nextRow()))
  {
    sseInfo = QueryGetSurfaceSectorExemptionInfoSQLStatement<
        QueryGetSurfaceSectorExemptionInfo>::mapRowToSurfaceSectorExemptionInfo(row, sseInfoPrev);
    if (sseInfo != sseInfoPrev)
    {
      sseInfoVec->push_back(sseInfo);
    }
    sseInfoPrev = sseInfo;
  }
  LOG4CXX_INFO(_logger,
               "SURFACESECTOREXEMPTIONINFO: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findSurfaceSectorExemptionInfo()

///////////////////////////////////////////////////////////
//
//  QueryGetSurfaceSectorExemptionInfoHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSurfaceSectorExemptionInfoHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetSurfaceSectorExemptionInfoHistorical"));
std::string QueryGetSurfaceSectorExemptionInfoHistorical::_baseSQL;
bool QueryGetSurfaceSectorExemptionInfoHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSurfaceSectorExemptionInfoHistorical>
g_GetSurfaceSectorExemptionInfoHistorical;

const char*
QueryGetSurfaceSectorExemptionInfoHistorical::getQueryName() const
{
  return "GETSURFACESECTOREXEMPTIONINFOHISTORICAL";
}

void
QueryGetSurfaceSectorExemptionInfoHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSurfaceSectorExemptionInfoHistoricalSQLStatement<
        QueryGetSurfaceSectorExemptionInfoHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSURFACESECTOREXEMPTIONINFOHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetSurfaceSectorExemptionInfo::initialize();
    _isInitialized = true;
  }
} // initialize()
}
