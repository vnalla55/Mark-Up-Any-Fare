#include "DBAccess/Queries/QueryGetFareFocusRule.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusRuleSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareFocusRule::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareFocusRule"));
std::string QueryGetFareFocusRule::_baseSQL;
bool
QueryGetFareFocusRule::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusRule> _getFareFocusRule;

const char*
QueryGetFareFocusRule::getQueryName() const
{
  return "GETFAREFOCUSRULE";
}

void
QueryGetFareFocusRule::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusRuleSQLStatement<QueryGetFareFocusRule> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSRULE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusRule::findFareFocusRule(std::vector<FareFocusRuleInfo*>& lst,
                                         uint64_t fareFocusRuleId)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(fareFocusRuleId));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  substParm(3, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    FareFocusRuleInfo* info(QueryGetFareFocusRuleSQLStatement<QueryGetFareFocusRule>::mapRow(row, nullptr));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSRULE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareFocusRuleHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareFocusRuleHistorical"));
std::string QueryGetFareFocusRuleHistorical::_baseSQL;
bool
QueryGetFareFocusRuleHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusRuleHistorical> _getFareFocusRuleHistorical;

const char*
QueryGetFareFocusRuleHistorical::getQueryName() const
{
  return "GETFAREFOCUSRULEHISTORICAL";
}

void
QueryGetFareFocusRuleHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusRuleHistoricalSQLStatement<QueryGetFareFocusRuleHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSRULEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusRuleHistorical::findFareFocusRule(
    std::vector<FareFocusRuleInfo*>& lst,
    uint64_t fareFocusRuleId,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(fareFocusRuleId));
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, static_cast<int64_t>(fareFocusRuleId));
  substParm(5, startDate);
  substParm(6, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusRuleInfo* info(nullptr);
  FareFocusRuleInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusRuleHistoricalSQLStatement<
        QueryGetFareFocusRuleHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSRULEHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareFocusRule::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareFocusRule"));
std::string QueryGetAllFareFocusRule::_baseSQL;
bool
QueryGetAllFareFocusRule::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareFocusRule> _getAllFareFocusRule;

const char*
QueryGetAllFareFocusRule::getQueryName() const
{
  return "GETALLFAREFOCUSRULE";
}

void
QueryGetAllFareFocusRule::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareFocusRuleSQLStatement<QueryGetAllFareFocusRule> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREFOCUSRULE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareFocusRule::findAllFareFocusRule(std::vector<FareFocusRuleInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusRuleInfo* info(nullptr);
  FareFocusRuleInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareFocusRuleSQLStatement<QueryGetAllFareFocusRule>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREFOCUSRULE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
