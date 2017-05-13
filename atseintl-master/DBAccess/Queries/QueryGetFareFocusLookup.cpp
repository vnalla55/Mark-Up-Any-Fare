#include "DBAccess/Queries/QueryGetFareFocusLookup.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusLookupSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareFocusLookup::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetFareFocusLookup"));
std::string QueryGetFareFocusLookup::_baseSQL;
bool
QueryGetFareFocusLookup::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusLookup> _getFareFocusLookup;

const char*
QueryGetFareFocusLookup::getQueryName() const
{
  return "GETFAREFOCUSLOOKUP";
}

void
QueryGetFareFocusLookup::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusLookupSQLStatement<QueryGetFareFocusLookup> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSLOOKUP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusLookup::findFareFocusLookup(std::vector<FareFocusLookupInfo*>& lst,
                                             const PseudoCityCode& pcc)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(pcc, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusLookupInfo* info(new FareFocusLookupInfo);
  info->pcc() = pcc;
  lst.push_back(info);

  while ((row = res.nextRow()))
  {
    QueryGetFareFocusLookupSQLStatement<QueryGetFareFocusLookup>::mapRow(row, info);
  }
  LOG4CXX_INFO(_logger, "GETFAREFOCUSLOOKUP: NumRows: " << res.numRows() << " Time = " << stopTimer()
               << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareFocusLookupHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetFareFocusLookupHistorical"));
std::string QueryGetFareFocusLookupHistorical::_baseSQL;
bool
QueryGetFareFocusLookupHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusLookupHistorical> _getFareFocusLookupHistorical;

const char*
QueryGetFareFocusLookupHistorical::getQueryName() const
{
  return "GETFAREFOCUSLOOKUPHISTORICAL";
}

void
QueryGetFareFocusLookupHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusLookupHistoricalSQLStatement<QueryGetFareFocusLookupHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSLOOKUPHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusLookupHistorical::findFareFocusLookup(
  std::vector<FareFocusLookupInfo*>& lst,
  const PseudoCityCode& pcc,
  const DateTime& startDate,
  const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(pcc, 1);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, startDate);
  substParm(5, endDate);
  substParm(pcc, 6);
  substParm(7, startDate);
  substParm(8, endDate);
  substParm(9, startDate);
  substParm(10, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusLookupInfo* info(new FareFocusLookupInfo);
  info->pcc() = pcc;
  lst.push_back(info);

  while ((row = res.nextRow()))
  {
    QueryGetFareFocusLookupHistoricalSQLStatement<QueryGetFareFocusLookupHistorical>::mapRow(row, info);
  }
  LOG4CXX_INFO(_logger, "GETFAREFOCUSLOOKUPHISTORICAL: NumRows: "
               << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

} // tse
