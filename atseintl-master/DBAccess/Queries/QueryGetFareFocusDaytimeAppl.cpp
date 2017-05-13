#include "DBAccess/Queries/QueryGetFareFocusDaytimeAppl.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusDaytimeApplSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareFocusDaytimeAppl::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusDaytimeAppl"));
std::string QueryGetFareFocusDaytimeAppl::_baseSQL;
bool
QueryGetFareFocusDaytimeAppl::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusDaytimeAppl> _getFareFocusDaytimeAppl;

const char*
QueryGetFareFocusDaytimeAppl::getQueryName() const
{
  return "GETFAREFOCUSDAYTIMEAPPL";
}

void
QueryGetFareFocusDaytimeAppl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusDaytimeApplSQLStatement<QueryGetFareFocusDaytimeAppl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSDAYTIMEAPPL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusDaytimeAppl::findFareFocusDaytimeAppl(std::vector<FareFocusDaytimeApplInfo*>& lst,
                                                   uint64_t dayTimeApplItemNo)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(dayTimeApplItemNo));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusDaytimeApplInfo* info(nullptr);
  FareFocusDaytimeApplInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusDaytimeApplSQLStatement<QueryGetFareFocusDaytimeAppl>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSDAYTIMEAPPL: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareFocusDaytimeApplHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusDaytimeApplHistorical"));
std::string QueryGetFareFocusDaytimeApplHistorical::_baseSQL;
bool
QueryGetFareFocusDaytimeApplHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusDaytimeApplHistorical> _getFareFocusDaytimeApplHistorical;

const char*
QueryGetFareFocusDaytimeApplHistorical::getQueryName() const
{
  return "GETFAREFOCUSDAYTIMEAPPLHISTORICAL";
}

void
QueryGetFareFocusDaytimeApplHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusDaytimeApplHistoricalSQLStatement<QueryGetFareFocusDaytimeApplHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSDAYTIMEAPPLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusDaytimeApplHistorical::findFareFocusDaytimeAppl(
    std::vector<FareFocusDaytimeApplInfo*>& lst,
    uint64_t dayTimeApplItemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(dayTimeApplItemNo));
  substParm(2, startDate);
  substParm(3, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusDaytimeApplInfo* info(nullptr);
  FareFocusDaytimeApplInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusDaytimeApplHistoricalSQLStatement<
        QueryGetFareFocusDaytimeApplHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSDAYTIMEAPPLHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareFocusDaytimeAppl::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllFareFocusDaytimeAppl"));
std::string QueryGetAllFareFocusDaytimeAppl::_baseSQL;
bool
QueryGetAllFareFocusDaytimeAppl::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareFocusDaytimeAppl> _getAllFareFocusDaytimeAppl;

const char*
QueryGetAllFareFocusDaytimeAppl::getQueryName() const
{
  return "GETALLFAREFOCUSDAYTIMEAPPL";
}

void
QueryGetAllFareFocusDaytimeAppl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareFocusDaytimeApplSQLStatement<QueryGetAllFareFocusDaytimeAppl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREFOCUSDAYTIMEAPPL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareFocusDaytimeAppl::findAllFareFocusDaytimeAppl(
    std::vector<FareFocusDaytimeApplInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusDaytimeApplInfo* info(nullptr);
  FareFocusDaytimeApplInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareFocusDaytimeApplSQLStatement<QueryGetAllFareFocusDaytimeAppl>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREFOCUSDAYTIMEAPPL: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
