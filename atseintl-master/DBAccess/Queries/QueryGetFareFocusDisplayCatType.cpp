#include "DBAccess/Queries/QueryGetFareFocusDisplayCatType.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusDisplayCatTypeSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareFocusDisplayCatType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusDisplayCatType"));
std::string QueryGetFareFocusDisplayCatType::_baseSQL;
bool
QueryGetFareFocusDisplayCatType::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusDisplayCatType> _getFareFocusDisplayCatType;

const char*
QueryGetFareFocusDisplayCatType::getQueryName() const
{
  return "GETFAREFOCUSDISPLAYCATTYPE";
}

void
QueryGetFareFocusDisplayCatType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusDisplayCatTypeSQLStatement<QueryGetFareFocusDisplayCatType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSDISPLAYCATTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusDisplayCatType::findFareFocusDisplayCatType(std::vector<FareFocusDisplayCatTypeInfo*>& lst,
                                                   uint64_t displayCatTypeItemNo)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(displayCatTypeItemNo));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusDisplayCatTypeInfo* info(nullptr);
  FareFocusDisplayCatTypeInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusDisplayCatTypeSQLStatement<QueryGetFareFocusDisplayCatType>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSDISPLAYCATTYPE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareFocusDisplayCatTypeHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusDisplayCatTypeHistorical"));
std::string QueryGetFareFocusDisplayCatTypeHistorical::_baseSQL;
bool
QueryGetFareFocusDisplayCatTypeHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusDisplayCatTypeHistorical> _getFareFocusDisplayCatTypeHistorical;

const char*
QueryGetFareFocusDisplayCatTypeHistorical::getQueryName() const
{
  return "GETFAREFOCUSDISPLAYCATTYPEHISTORICAL";
}

void
QueryGetFareFocusDisplayCatTypeHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusDisplayCatTypeHistoricalSQLStatement<QueryGetFareFocusDisplayCatTypeHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSDISPLAYCATTYPEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusDisplayCatTypeHistorical::findFareFocusDisplayCatType(
    std::vector<FareFocusDisplayCatTypeInfo*>& lst,
    uint64_t displayCatTypeItemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(displayCatTypeItemNo));
  substParm(2, startDate);
  substParm(3, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusDisplayCatTypeInfo* info(nullptr);
  FareFocusDisplayCatTypeInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusDisplayCatTypeHistoricalSQLStatement<
        QueryGetFareFocusDisplayCatTypeHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSDISPLAYCATTYPEHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareFocusDisplayCatType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllFareFocusDisplayCatType"));
std::string QueryGetAllFareFocusDisplayCatType::_baseSQL;
bool
QueryGetAllFareFocusDisplayCatType::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareFocusDisplayCatType> _getAllFareFocusDisplayCatType;

const char*
QueryGetAllFareFocusDisplayCatType::getQueryName() const
{
  return "GETALLFAREFOCUSDISPLAYCATTYPE";
}

void
QueryGetAllFareFocusDisplayCatType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareFocusDisplayCatTypeSQLStatement<QueryGetAllFareFocusDisplayCatType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREFOCUSDISPLAYCATTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareFocusDisplayCatType::findAllFareFocusDisplayCatType(
    std::vector<FareFocusDisplayCatTypeInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusDisplayCatTypeInfo* info(nullptr);
  FareFocusDisplayCatTypeInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareFocusDisplayCatTypeSQLStatement<QueryGetAllFareFocusDisplayCatType>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREFOCUSDISPLAYCATTYPE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
