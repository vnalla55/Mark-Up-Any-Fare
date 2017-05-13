#include "DBAccess/Queries/QueryGetFareFocusRuleCode.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusRuleCodeSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareFocusRuleCode::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusRuleCode"));
std::string QueryGetFareFocusRuleCode::_baseSQL;
bool
QueryGetFareFocusRuleCode::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusRuleCode> _getFareFocusRuleCode;

const char*
QueryGetFareFocusRuleCode::getQueryName() const
{
  return "GETFAREFOCUSRULECODE";
}

void
QueryGetFareFocusRuleCode::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusRuleCodeSQLStatement<QueryGetFareFocusRuleCode> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSRULECODE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusRuleCode::findFareFocusRuleCode(std::vector<FareFocusRuleCodeInfo*>& lst,
                                                   uint64_t ruleCdItemNo)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(ruleCdItemNo));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusRuleCodeInfo* info(nullptr);
  FareFocusRuleCodeInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusRuleCodeSQLStatement<QueryGetFareFocusRuleCode>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSRULECODE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareFocusRuleCodeHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusRuleCodeHistorical"));
std::string QueryGetFareFocusRuleCodeHistorical::_baseSQL;
bool
QueryGetFareFocusRuleCodeHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusRuleCodeHistorical> _getFareFocusRuleCodeHistorical;

const char*
QueryGetFareFocusRuleCodeHistorical::getQueryName() const
{
  return "GETFAREFOCUSRULECODEHISTORICAL";
}

void
QueryGetFareFocusRuleCodeHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusRuleCodeHistoricalSQLStatement<QueryGetFareFocusRuleCodeHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSRULECODEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusRuleCodeHistorical::findFareFocusRuleCode(
    std::vector<FareFocusRuleCodeInfo*>& lst,
    uint64_t ruleCdItemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(ruleCdItemNo));
  substParm(2, startDate);
  substParm(3, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusRuleCodeInfo* info(nullptr);
  FareFocusRuleCodeInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusRuleCodeHistoricalSQLStatement<
        QueryGetFareFocusRuleCodeHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSRULECODEHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareFocusRuleCode::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllFareFocusRuleCode"));
std::string QueryGetAllFareFocusRuleCode::_baseSQL;
bool
QueryGetAllFareFocusRuleCode::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareFocusRuleCode> _getAllFareFocusRuleCode;

const char*
QueryGetAllFareFocusRuleCode::getQueryName() const
{
  return "GETALLFAREFOCUSRULECODE";
}

void
QueryGetAllFareFocusRuleCode::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareFocusRuleCodeSQLStatement<QueryGetAllFareFocusRuleCode> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREFOCUSRULECODE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareFocusRuleCode::findAllFareFocusRuleCode(
    std::vector<FareFocusRuleCodeInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusRuleCodeInfo* info(nullptr);
  FareFocusRuleCodeInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareFocusRuleCodeSQLStatement<QueryGetAllFareFocusRuleCode>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREFOCUSRULECODE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse


