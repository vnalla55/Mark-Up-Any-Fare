#include "DBAccess/Queries/QueryGetFareFocusSecurity.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusSecuritySQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareFocusSecurity::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusSecurity"));
std::string QueryGetFareFocusSecurity::_baseSQL;
bool
QueryGetFareFocusSecurity::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusSecurity> _getFareFocusSecurity;

const char*
QueryGetFareFocusSecurity::getQueryName() const
{
  return "GETFAREFOCUSSECURITY";
}

void
QueryGetFareFocusSecurity::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusSecuritySQLStatement<QueryGetFareFocusSecurity> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSSECURITY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusSecurity::findFareFocusSecurity(std::vector<FareFocusSecurityInfo*>& lst,
                                                 uint64_t securityItemNo)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(securityItemNo));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusSecurityInfo* info(nullptr);
  FareFocusSecurityInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusSecuritySQLStatement<QueryGetFareFocusSecurity>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSSECURITY: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareFocusSecurityHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusSecurityHistorical"));
std::string QueryGetFareFocusSecurityHistorical::_baseSQL;
bool
QueryGetFareFocusSecurityHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusSecurityHistorical> _getFareFocusSecurityHistorical;

const char*
QueryGetFareFocusSecurityHistorical::getQueryName() const
{
  return "GETFAREFOCUSSECURITYHISTORICAL";
}

void
QueryGetFareFocusSecurityHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusSecurityHistoricalSQLStatement<QueryGetFareFocusSecurityHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSSECURITYHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusSecurityHistorical::findFareFocusSecurity(
    std::vector<FareFocusSecurityInfo*>& lst,
    uint64_t securityItemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(securityItemNo));
  substParm(2, startDate);
  substParm(3, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusSecurityInfo* info(nullptr);
  FareFocusSecurityInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusSecurityHistoricalSQLStatement<
        QueryGetFareFocusSecurityHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSSECURITYHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareFocusSecurity::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllFareFocusSecurity"));
std::string QueryGetAllFareFocusSecurity::_baseSQL;
bool
QueryGetAllFareFocusSecurity::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareFocusSecurity> _getAllFareFocusSecurity;

const char*
QueryGetAllFareFocusSecurity::getQueryName() const
{
  return "GETALLFAREFOCUSSECURITY";
}

void
QueryGetAllFareFocusSecurity::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareFocusSecuritySQLStatement<QueryGetAllFareFocusSecurity> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREFOCUSSECURITY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareFocusSecurity::findAllFareFocusSecurity(
    std::vector<FareFocusSecurityInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusSecurityInfo* info(nullptr);
  FareFocusSecurityInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareFocusSecuritySQLStatement<QueryGetAllFareFocusSecurity>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREFOCUSSECURITY: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
