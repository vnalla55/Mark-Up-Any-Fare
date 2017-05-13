#include "DBAccess/Queries/QueryGetRBDByCabin.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetRBDByCabinSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr QueryGetRBDByCabin::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.RBDByCabin"));
std::string QueryGetRBDByCabin::_baseSQL;
bool QueryGetRBDByCabin::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetRBDByCabin> _getRBDByCabin;

const char* QueryGetRBDByCabin::getQueryName() const
{
  return "GETRBDBYCABIN";
}

void QueryGetRBDByCabin::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRBDByCabinSQLStatement<QueryGetRBDByCabin> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETRBDBYCABIN");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetRBDByCabin::findRBDByCabin(std::vector<RBDByCabinInfo*>& lst,
                                        const VendorCode& vendor,
                                        const CarrierCode& carrier)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(vendor, 1);
  substParm(carrier, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  RBDByCabinInfo* info(nullptr);
  RBDByCabinInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetRBDByCabinSQLStatement<QueryGetRBDByCabin>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETRBDBYCABIN: NumRows: " << res.numRows() << " Time = " << stopTimer()
               << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr QueryGetRBDByCabinHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.RBDByCabinHistorical"));
std::string QueryGetRBDByCabinHistorical::_baseSQL;
bool QueryGetRBDByCabinHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetRBDByCabinHistorical> _getRBDByCabinHistorical;

const char* QueryGetRBDByCabinHistorical::getQueryName() const
{
  return "GETRBDBYCABINHISTORICAL";
}

void QueryGetRBDByCabinHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRBDByCabinHistoricalSQLStatement<QueryGetRBDByCabinHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETRBDBYCABINHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetRBDByCabinHistorical::findRBDByCabin(
    std::vector<RBDByCabinInfo*>& lst,
    const VendorCode& vendor,
    const CarrierCode& carrier,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  RBDByCabinInfo* info(nullptr);
  RBDByCabinInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetRBDByCabinHistoricalSQLStatement<
        QueryGetRBDByCabinHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger, "GETRBDBYCABINHISTORICAL: NumRows: "
               << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr QueryGetAllRBDByCabin::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllRBDByCabin"));

std::string QueryGetAllRBDByCabin::_baseSQL;

bool QueryGetAllRBDByCabin::_isInitialized(false);

SQLQueryInitializerHelper<QueryGetAllRBDByCabin> _getAllRBDByCabin;

const char* QueryGetAllRBDByCabin::getQueryName() const
{
  return "GETALLRBDBYCABIN";
}

void QueryGetAllRBDByCabin::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllRBDByCabinSQLStatement<QueryGetAllRBDByCabin> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLRBDBYCABIN");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetAllRBDByCabin::findAllRBDByCabin(std::vector<RBDByCabinInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  RBDByCabinInfo* info(nullptr);
  RBDByCabinInfo* infoPrev(nullptr);
  while ((row = res.nextRow()))
  {
    info = QueryGetAllRBDByCabinSQLStatement<QueryGetAllRBDByCabin>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger, "GETALLRBDBYCABIN: NumRows: " << res.numRows() << " Time = " << stopTimer()
               << " (" << stopCPU() << ") ms");
  res.freeResult();
}

} // tse
