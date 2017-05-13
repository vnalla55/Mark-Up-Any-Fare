//----------------------------------------------------------------------------
// � 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetServiceBaggage.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetServiceBaggageSQLStatement.h"
#include "DBAccess/ServiceBaggageInfo.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetServiceBaggage::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetServiceBaggage"));

std::string QueryGetServiceBaggage::_baseSQL;
bool QueryGetServiceBaggage::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetServiceBaggage> g_GetServiceBaggage;

QueryGetServiceBaggage::QueryGetServiceBaggage(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};

QueryGetServiceBaggage::QueryGetServiceBaggage(DBAdapter* dbAdapt, const std::string& sqlStatement)
  : SQLQuery(dbAdapt, sqlStatement) {};

QueryGetServiceBaggage::~QueryGetServiceBaggage() {};

const QueryGetServiceBaggage&
QueryGetServiceBaggage::
operator=(const QueryGetServiceBaggage& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
}

const QueryGetServiceBaggage&
QueryGetServiceBaggage::
operator=(const std::string& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
};

void
QueryGetServiceBaggage::initialize()
{
  if (!_isInitialized)
  {
    QueryGetServiceBaggageSQLStatement<QueryGetServiceBaggage> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSERVICEBAGGAGE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

const char*
QueryGetServiceBaggage::getQueryName() const
{
  return "GETSERVICEBAGGAGE";
}

void
QueryGetServiceBaggage::findServiceBaggageInfo(std::vector<const ServiceBaggageInfo*>& data,
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
        QueryGetServiceBaggageSQLStatement<QueryGetServiceBaggage>::mapRowToServiceBaggageInfo(
            row));
  }
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetServiceBaggageHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetServiceBaggageHistorical"));
std::string QueryGetServiceBaggageHistorical::_baseSQL;
bool QueryGetServiceBaggageHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetServiceBaggageHistorical> g_GetServiceBaggageHistorical;

QueryGetServiceBaggageHistorical::QueryGetServiceBaggageHistorical(DBAdapter* dbAdapt)
  : QueryGetServiceBaggage(dbAdapt, _baseSQL)
{
}

QueryGetServiceBaggageHistorical::~QueryGetServiceBaggageHistorical() {}

const char*
QueryGetServiceBaggageHistorical::getQueryName() const
{
  return "GETSERVICEBAGGAGEHISTORICAL";
}

void
QueryGetServiceBaggageHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetServiceBaggageHistoricalSQLStatement<QueryGetServiceBaggageHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSERVICEBAGGAGEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetServiceBaggageHistorical::findServiceBaggageInfo(
    std::vector<const ServiceBaggageInfo*>& data,
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
    data.push_back(QueryGetServiceBaggageHistoricalSQLStatement<
        QueryGetServiceBaggageHistorical>::mapRowToServiceBaggageInfo(row));
  }
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}
}
