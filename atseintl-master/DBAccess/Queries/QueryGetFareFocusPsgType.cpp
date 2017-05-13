#include "DBAccess/Queries/QueryGetFareFocusPsgType.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusPsgTypeSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareFocusPsgType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusPsgType"));
std::string QueryGetFareFocusPsgType::_baseSQL;
bool
QueryGetFareFocusPsgType::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusPsgType> _getFareFocusPsgType;

const char*
QueryGetFareFocusPsgType::getQueryName() const
{
  return "GETFAREFOCUSPSGTYPE";
}

void
QueryGetFareFocusPsgType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusPsgTypeSQLStatement<QueryGetFareFocusPsgType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSPSGTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusPsgType::findFareFocusPsgType(std::vector<FareFocusPsgTypeInfo*>& lst,
                                                   uint64_t psgTypeItemNo)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(psgTypeItemNo));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusPsgTypeInfo* info(nullptr);
  FareFocusPsgTypeInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusPsgTypeSQLStatement<QueryGetFareFocusPsgType>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSPSGTYPE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareFocusPsgTypeHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusPsgTypeHistorical"));
std::string QueryGetFareFocusPsgTypeHistorical::_baseSQL;
bool
QueryGetFareFocusPsgTypeHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusPsgTypeHistorical> _getFareFocusPsgTypeHistorical;

const char*
QueryGetFareFocusPsgTypeHistorical::getQueryName() const
{
  return "GETFAREFOCUSPSGTYPEHISTORICAL";
}

void
QueryGetFareFocusPsgTypeHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusPsgTypeHistoricalSQLStatement<QueryGetFareFocusPsgTypeHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSPSGTYPEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusPsgTypeHistorical::findFareFocusPsgType(
    std::vector<FareFocusPsgTypeInfo*>& lst,
    uint64_t psgTypeItemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(psgTypeItemNo));
  substParm(2, startDate);
  substParm(3, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusPsgTypeInfo* info(nullptr);
  FareFocusPsgTypeInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusPsgTypeHistoricalSQLStatement<
        QueryGetFareFocusPsgTypeHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSPSGTYPEHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareFocusPsgType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllFareFocusPsgType"));
std::string QueryGetAllFareFocusPsgType::_baseSQL;
bool
QueryGetAllFareFocusPsgType::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareFocusPsgType> _getAllFareFocusPsgType;

const char*
QueryGetAllFareFocusPsgType::getQueryName() const
{
  return "GETALLFAREFOCUSPSGTYPE";
}

void
QueryGetAllFareFocusPsgType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareFocusPsgTypeSQLStatement<QueryGetAllFareFocusPsgType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREFOCUSPSGTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareFocusPsgType::findAllFareFocusPsgType(
    std::vector<FareFocusPsgTypeInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusPsgTypeInfo* info(nullptr);
  FareFocusPsgTypeInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareFocusPsgTypeSQLStatement<QueryGetAllFareFocusPsgType>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREFOCUSPSGTYPE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
