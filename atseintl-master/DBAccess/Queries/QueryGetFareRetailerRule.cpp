#include "DBAccess/Queries/QueryGetFareRetailerRule.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareRetailerRuleSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareRetailerRule::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareRetailerRule"));
std::string QueryGetFareRetailerRule::_baseSQL;
bool
QueryGetFareRetailerRule::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareRetailerRule> _getFareRetailerRule;

const char*
QueryGetFareRetailerRule::getQueryName() const
{
  return "GETFARERETAILERRULE";
}

void
QueryGetFareRetailerRule::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareRetailerRuleSQLStatement<QueryGetFareRetailerRule> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARERETAILERRULE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareRetailerRule::findFareRetailerRule(std::vector<FareRetailerRuleInfo*>& lst,
                                               uint64_t fareRetailerRuleId)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(fareRetailerRuleId));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  substParm(3, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    FareRetailerRuleInfo* info(QueryGetFareRetailerRuleSQLStatement<QueryGetFareRetailerRule>::mapRow(row, nullptr));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger,
               "GETFARERETAILERRULE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareRetailerRuleHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareRetailerRuleHistorical"));
std::string QueryGetFareRetailerRuleHistorical::_baseSQL;
bool
QueryGetFareRetailerRuleHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareRetailerRuleHistorical> _getFareRetailerRuleHistorical;

const char*
QueryGetFareRetailerRuleHistorical::getQueryName() const
{
  return "GETFARERETAILERRULEHISTORICAL";
}

void
QueryGetFareRetailerRuleHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareRetailerRuleHistoricalSQLStatement<QueryGetFareRetailerRuleHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARERETAILERRULEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareRetailerRuleHistorical::findFareRetailerRule(std::vector<FareRetailerRuleInfo*>& lst,
                                                         uint64_t fareRetailerRuleId,
                                                         const DateTime& startDate,
                                                         const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(fareRetailerRuleId));
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, static_cast<int64_t>(fareRetailerRuleId));
  substParm(5, startDate);
  substParm(6, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareRetailerRuleInfo* info(nullptr);
  FareRetailerRuleInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareRetailerRuleHistoricalSQLStatement<
        QueryGetFareRetailerRuleHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFARERETAILERRULEHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareRetailerRule::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareRetailerRule"));
std::string QueryGetAllFareRetailerRule::_baseSQL;
bool
QueryGetAllFareRetailerRule::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareRetailerRule> _getAllFareRetailerRule;

const char*
QueryGetAllFareRetailerRule::getQueryName() const
{
  return "GETALLFARERETAILERRULE";
}

void
QueryGetAllFareRetailerRule::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareRetailerRuleSQLStatement<QueryGetAllFareRetailerRule> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFARERETAILERRULE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareRetailerRule::findAllFareRetailerRule(std::vector<FareRetailerRuleInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareRetailerRuleInfo* info(nullptr);
  FareRetailerRuleInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareRetailerRuleSQLStatement<QueryGetAllFareRetailerRule>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFARERETAILERRULE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}

} // tse
