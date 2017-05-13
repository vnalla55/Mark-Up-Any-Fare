//----------------------------------------------------------------------------
// � 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetPaxTypeCode.h"

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPaxTypeCodeSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetPaxTypeCode::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPaxTypeCode"));

std::string QueryGetPaxTypeCode::_baseSQL;
bool QueryGetPaxTypeCode::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPaxTypeCode> g_GetPaxTypeCode;

QueryGetPaxTypeCode::QueryGetPaxTypeCode(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};

QueryGetPaxTypeCode::QueryGetPaxTypeCode(DBAdapter* dbAdapt, const std::string& sqlStatement)
  : SQLQuery(dbAdapt, sqlStatement) {};

QueryGetPaxTypeCode::~QueryGetPaxTypeCode() {};

const QueryGetPaxTypeCode&
QueryGetPaxTypeCode::
operator=(const QueryGetPaxTypeCode& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
}

const QueryGetPaxTypeCode&
QueryGetPaxTypeCode::
operator=(const std::string& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
};

void
QueryGetPaxTypeCode::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPaxTypeCodeSQLStatement<QueryGetPaxTypeCode> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPAXTYPECODE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

const char*
QueryGetPaxTypeCode::getQueryName() const
{
  return "GETPAXTYPECODE";
}

void
QueryGetPaxTypeCode::findPaxTypeCodeInfo(std::vector<const PaxTypeCodeInfo*>& data,
                                         const VendorCode& vendor,
                                         int itemNumber)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    data.push_back(
        QueryGetPaxTypeCodeSQLStatement<QueryGetPaxTypeCode>::mapRowToPaxTypeCodeInfo(row));
  }
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetPaxTypeCodeHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetPaxTypeCodeHistorical"));
std::string QueryGetPaxTypeCodeHistorical::_baseSQL;
bool QueryGetPaxTypeCodeHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPaxTypeCodeHistorical> g_GetPaxTypeCodeHistorical;

QueryGetPaxTypeCodeHistorical::QueryGetPaxTypeCodeHistorical(DBAdapter* dbAdapt)
  : QueryGetPaxTypeCode(dbAdapt, _baseSQL)
{
}

QueryGetPaxTypeCodeHistorical::~QueryGetPaxTypeCodeHistorical() {}

const char*
QueryGetPaxTypeCodeHistorical::getQueryName() const
{
  return "GETPAXTYPECODEHISTORICAL";
}

void
QueryGetPaxTypeCodeHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPaxTypeCodeHistoricalSQLStatement<QueryGetPaxTypeCodeHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPAXTYPECODEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetPaxTypeCodeHistorical::findPaxTypeCodeInfo(std::vector<const PaxTypeCodeInfo*>& data,
                                                   const VendorCode& vendor,
                                                   int itemNumber,
                                                   const DateTime& startDate,
                                                   const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    data.push_back(QueryGetPaxTypeCodeHistoricalSQLStatement<
        QueryGetPaxTypeCodeHistorical>::mapRowToPaxTypeCodeInfo(row));
  }
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}
}
