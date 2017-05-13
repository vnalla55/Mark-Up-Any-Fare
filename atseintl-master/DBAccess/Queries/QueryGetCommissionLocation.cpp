#include "DBAccess/Queries/QueryGetCommissionLocation.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCommissionLocationSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr QueryGetCommissionLocation::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CommissionLocation"));
std::string QueryGetCommissionLocation::_baseSQL;
bool QueryGetCommissionLocation::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCommissionLocation> _getCommissionLocation;

const char* QueryGetCommissionLocation::getQueryName() const
{
  return "GETCOMMISSIONLOCATION";
}

void QueryGetCommissionLocation::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCommissionLocationSQLStatement<QueryGetCommissionLocation> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMMISSIONLOCATION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetCommissionLocation::findCommissionLocation(std::vector<CommissionLocSegInfo*>& lst,
                                                        const VendorCode& vendor,
                                                        uint64_t itemNo)
{
  Row* row(0);
  DBResultSet res(_dbAdapt);
  substParm(vendor, 1);
  substParm(2, static_cast<int64_t>(itemNo));
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    CommissionLocSegInfo* info(QueryGetCommissionLocationSQLStatement<QueryGetCommissionLocation>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETCOMMISSIONLOCATION: NumRows: " << res.numRows() << " Time = " << stopTimer()
                        << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr QueryGetCommissionLocationHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CommissionLocationHistorical"));
std::string QueryGetCommissionLocationHistorical::_baseSQL;
bool QueryGetCommissionLocationHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCommissionLocationHistorical> _getCommissionLocationHistorical;

const char* QueryGetCommissionLocationHistorical::getQueryName() const
{
  return "GETCOMMISSIONLOCATIONHISTORICAL";
}

void QueryGetCommissionLocationHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCommissionLocationHistoricalSQLStatement<QueryGetCommissionLocationHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMMISSIONLOCATIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetCommissionLocationHistorical::findCommissionLocation(std::vector<CommissionLocSegInfo*>& lst,
                                                                  const VendorCode& vendor,
                                                                  uint64_t itemNo,
                                                                  DateTime startDate,
                                                                  DateTime endDate)
{
  Row* row(0);
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(2, static_cast<int64_t>(itemNo));
  //substParm(3, startDate);
  substParm(3, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    CommissionLocSegInfo* info(QueryGetCommissionLocationHistoricalSQLStatement<
      QueryGetCommissionLocationHistorical>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETCOMMISSIONLOCATIONHISTORICAL: NumRows: " << res.numRows()
                        << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

} // tse
