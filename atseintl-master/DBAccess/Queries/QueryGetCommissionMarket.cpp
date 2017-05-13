#include "DBAccess/Queries/QueryGetCommissionMarket.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCommissionMarketSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr QueryGetCommissionMarket::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CommissionMarket"));
std::string QueryGetCommissionMarket::_baseSQL;
bool QueryGetCommissionMarket::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCommissionMarket> _getCommissionMarket;

const char* QueryGetCommissionMarket::getQueryName() const
{
  return "GETCOMMISSIONMARKET";
}

void QueryGetCommissionMarket::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCommissionMarketSQLStatement<QueryGetCommissionMarket> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMMISSIONMARKET");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetCommissionMarket::findCommissionMarket(std::vector<CommissionMarketSegInfo*>& lst,
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
    CommissionMarketSegInfo* info(QueryGetCommissionMarketSQLStatement<QueryGetCommissionMarket>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETCOMMISSIONMARKET: NumRows: " << res.numRows() << " Time = " << stopTimer()
                        << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr QueryGetCommissionMarketHistorical::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CommissionMarketHistorical"));
std::string QueryGetCommissionMarketHistorical::_baseSQL;
bool QueryGetCommissionMarketHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCommissionMarketHistorical> _getCommissionMarketHistorical;

const char* QueryGetCommissionMarketHistorical::getQueryName() const
{
  return "GETCOMMISSIONMARKETHISTORICAL";
}

void QueryGetCommissionMarketHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCommissionMarketHistoricalSQLStatement<QueryGetCommissionMarketHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMMISSIONMARKETHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetCommissionMarketHistorical::findCommissionMarket(std::vector<CommissionMarketSegInfo*>& lst,
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
    CommissionMarketSegInfo* info(QueryGetCommissionMarketHistoricalSQLStatement<
      QueryGetCommissionMarketHistorical>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETCOMMISSIONMARKETHISTORICAL: NumRows: " << res.numRows()
                        << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

} // tse
