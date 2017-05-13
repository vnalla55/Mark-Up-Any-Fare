//----------------------------------------------------------------------------
//  File:           QueryGetCorpId.cpp
//  Description:    QueryGetCorpId
//  Created:        5/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetCorpId.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCorpIdSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCorpId::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCorpId"));
std::string QueryGetCorpId::_baseSQL;
bool QueryGetCorpId::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCorpId> g_GetCorpId;

const char*
QueryGetCorpId::getQueryName() const
{
  return "GETCORPID";
}

void
QueryGetCorpId::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCorpIdSQLStatement<QueryGetCorpId> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCORPID");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCorpId::findCorpId(std::vector<tse::CorpId*>& corpIds, std::string& corpId)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, corpId);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    corpIds.push_back(QueryGetCorpIdSQLStatement<QueryGetCorpId>::mapRowToCorpId(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCORPID: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                       << stopCPU() << ")");
  res.freeResult();
} // findCorpId()

///////////////////////////////////////////////////////////
//  QueryGetCorpIdHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCorpIdHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetCorpIdHistorical"));
std::string QueryGetCorpIdHistorical::_baseSQL;
bool QueryGetCorpIdHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCorpIdHistorical> g_GetCorpIdHistorical;

const char*
QueryGetCorpIdHistorical::getQueryName() const
{
  return "GETCORPIDHISTORICAL";
};

void
QueryGetCorpIdHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCorpIdHistoricalSQLStatement<QueryGetCorpIdHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCORPIDHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCorpIdHistorical::findCorpId(std::vector<tse::CorpId*>& corpIds,
                                     std::string& corpId,
                                     const DateTime& startDate,
                                     const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, corpId);
  substParm(2, startDate);
  substParm(3, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    corpIds.push_back(
        QueryGetCorpIdHistoricalSQLStatement<QueryGetCorpIdHistorical>::mapRowToCorpId(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCORPIDHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetCorpIdHistorical::findCorpId()
}
