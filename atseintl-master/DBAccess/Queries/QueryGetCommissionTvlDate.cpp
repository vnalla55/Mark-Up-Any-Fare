#include "DBAccess/Queries/QueryGetCommissionTvlDate.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCommissionTvlDateSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr QueryGetCommissionTvlDate::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CommissionTvlDate"));
std::string QueryGetCommissionTvlDate::_baseSQL;
bool QueryGetCommissionTvlDate::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCommissionTvlDate> _getCommissionTvlDate;

const char* QueryGetCommissionTvlDate::getQueryName() const
{
  return "GETCOMMISSIONvLDATE";
}

void QueryGetCommissionTvlDate::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCommissionTvlDateSQLStatement<QueryGetCommissionTvlDate> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMMISSIONTVLDATE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetCommissionTvlDate::findCommissionTvlDate(std::vector<CommissionTravelDatesSegInfo*>& lst,
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
    CommissionTravelDatesSegInfo* info(QueryGetCommissionTvlDateSQLStatement<QueryGetCommissionTvlDate>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETCOMMISSIONTVLDATE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                        << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr QueryGetCommissionTvlDateHistorical::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CommissionTvlDateHistorical"));
std::string QueryGetCommissionTvlDateHistorical::_baseSQL;
bool QueryGetCommissionTvlDateHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCommissionTvlDateHistorical> _getCommissionTvlDateHistorical;

const char* QueryGetCommissionTvlDateHistorical::getQueryName() const
{
  return "GETCOMMISSIONTVLDATEHISTORICAL";
}

void QueryGetCommissionTvlDateHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCommissionTvlDateHistoricalSQLStatement<QueryGetCommissionTvlDateHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMMISSIONTVLDATEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetCommissionTvlDateHistorical::findCommissionTvlDate(std::vector<CommissionTravelDatesSegInfo*>& lst,
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
    CommissionTravelDatesSegInfo* info(QueryGetCommissionTvlDateHistoricalSQLStatement<
      QueryGetCommissionTvlDateHistorical>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETCOMMISSIONvLDATEHISTORICAL: NumRows: " << res.numRows()
                        << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

} // tse
