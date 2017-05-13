//----------------------------------------------------------------------------
// � 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetSectorDetail.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSectorDetailSQLStatement.h"
#include "DBAccess/SectorDetailInfo.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetSectorDetail::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSectorDetail"));

std::string QueryGetSectorDetail::_baseSQL;
bool QueryGetSectorDetail::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSectorDetail> g_GetSectorDetail;

QueryGetSectorDetail::QueryGetSectorDetail(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};

QueryGetSectorDetail::QueryGetSectorDetail(DBAdapter* dbAdapt, const std::string& sqlStatement)
  : SQLQuery(dbAdapt, sqlStatement) {};

QueryGetSectorDetail::~QueryGetSectorDetail() {};

const QueryGetSectorDetail&
QueryGetSectorDetail::
operator=(const QueryGetSectorDetail& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
}

const QueryGetSectorDetail&
QueryGetSectorDetail::
operator=(const std::string& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
};

void
QueryGetSectorDetail::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSectorDetailSQLStatement<QueryGetSectorDetail> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSECTORDETAIL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

const char*
QueryGetSectorDetail::getQueryName() const
{
  return "GETSECTORDETAIL";
}

void
QueryGetSectorDetail::findSectorDetailInfo(std::vector<const SectorDetailInfo*>& data,
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
        QueryGetSectorDetailSQLStatement<QueryGetSectorDetail>::mapRowToSectorDetailInfo(row));
  }
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetSectorDetailHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetSectorDetailHistorical"));
std::string QueryGetSectorDetailHistorical::_baseSQL;
bool QueryGetSectorDetailHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSectorDetailHistorical> g_GetSectorDetailHistorical;

QueryGetSectorDetailHistorical::QueryGetSectorDetailHistorical(DBAdapter* dbAdapt)
  : QueryGetSectorDetail(dbAdapt, _baseSQL)
{
}

QueryGetSectorDetailHistorical::~QueryGetSectorDetailHistorical() {}

const char*
QueryGetSectorDetailHistorical::getQueryName() const
{
  return "GETSECTORDETAILHISTORICAL";
}

void
QueryGetSectorDetailHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSectorDetailHistoricalSQLStatement<QueryGetSectorDetailHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSECTORDETAILHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetSectorDetailHistorical::findSectorDetailInfo(std::vector<const SectorDetailInfo*>& data,
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
    data.push_back(QueryGetSectorDetailHistoricalSQLStatement<
        QueryGetSectorDetailHistorical>::mapRowToSectorDetailInfo(row));
  }
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}
}
