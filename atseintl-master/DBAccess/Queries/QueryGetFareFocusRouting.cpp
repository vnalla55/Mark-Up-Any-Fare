#include "DBAccess/Queries/QueryGetFareFocusRouting.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusRoutingSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareFocusRouting::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusRouting"));
std::string QueryGetFareFocusRouting::_baseSQL;
bool
QueryGetFareFocusRouting::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusRouting> _getFareFocusRouting;

const char*
QueryGetFareFocusRouting::getQueryName() const
{
  return "GETFAREFOCUSROUTING";
}

void
QueryGetFareFocusRouting::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusRoutingSQLStatement<QueryGetFareFocusRouting> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSROUTING");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusRouting::findFareFocusRouting(std::vector<FareFocusRoutingInfo*>& lst,
                                                   uint64_t routingItemNo)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(routingItemNo));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusRoutingInfo* info(nullptr);
  FareFocusRoutingInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusRoutingSQLStatement<QueryGetFareFocusRouting>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSROUTING: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareFocusRoutingHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusRoutingHistorical"));
std::string QueryGetFareFocusRoutingHistorical::_baseSQL;
bool
QueryGetFareFocusRoutingHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusRoutingHistorical> _getFareFocusRoutingHistorical;

const char*
QueryGetFareFocusRoutingHistorical::getQueryName() const
{
  return "GETFAREFOCUSROUTINGHISTORICAL";
}

void
QueryGetFareFocusRoutingHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusRoutingHistoricalSQLStatement<QueryGetFareFocusRoutingHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSROUTINGHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusRoutingHistorical::findFareFocusRouting(
    std::vector<FareFocusRoutingInfo*>& lst,
    uint64_t routingItemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(routingItemNo));
  substParm(2, startDate);
  substParm(3, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusRoutingInfo* info(nullptr);
  FareFocusRoutingInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusRoutingHistoricalSQLStatement<
        QueryGetFareFocusRoutingHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSROUTINGHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareFocusRouting::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllFareFocusRouting"));
std::string QueryGetAllFareFocusRouting::_baseSQL;
bool
QueryGetAllFareFocusRouting::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareFocusRouting> _getAllFareFocusRouting;

const char*
QueryGetAllFareFocusRouting::getQueryName() const
{
  return "GETALLFAREFOCUSROUTING";
}

void
QueryGetAllFareFocusRouting::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareFocusRoutingSQLStatement<QueryGetAllFareFocusRouting> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREFOCUSROUTING");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareFocusRouting::findAllFareFocusRouting(
    std::vector<FareFocusRoutingInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusRoutingInfo* info(nullptr);
  FareFocusRoutingInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareFocusRoutingSQLStatement<QueryGetAllFareFocusRouting>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREFOCUSROUTING: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
