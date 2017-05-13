#include "DBAccess/Queries/QueryGetCommissionRule.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCommissionRuleSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr QueryGetCommissionRule::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CommissionRule"));
std::string QueryGetCommissionRule::_baseSQL;
bool QueryGetCommissionRule::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCommissionRule> _getCommissionRule;

const char* QueryGetCommissionRule::getQueryName() const
{
  return "GETCOMMISSIONRULE";
}

void QueryGetCommissionRule::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCommissionRuleSQLStatement<QueryGetCommissionRule> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMMISSIONRULE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetCommissionRule::findCommissionRule(std::vector<CommissionRuleInfo*>& lst,
                                                const VendorCode& vendor,
                                                uint64_t programId)
{
  Row* row(0);
  DBResultSet res(_dbAdapt);
  substParm(vendor, 1);
  substParm(2, static_cast<int64_t>(programId));
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    CommissionRuleInfo* info(QueryGetCommissionRuleSQLStatement<QueryGetCommissionRule>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETCOMMISSIONRULE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                        << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr QueryGetCommissionRuleHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CommissionRuleHistorical"));
std::string QueryGetCommissionRuleHistorical::_baseSQL;
bool QueryGetCommissionRuleHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCommissionRuleHistorical> _getCommissionRuleHistorical;

const char* QueryGetCommissionRuleHistorical::getQueryName() const
{
  return "GETCOMMISSIONRULEHISTORICAL";
}

void QueryGetCommissionRuleHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCommissionRuleHistoricalSQLStatement<QueryGetCommissionRuleHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMMISSIONRULEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetCommissionRuleHistorical::findCommissionRule(std::vector<CommissionRuleInfo*>& lst,
                                                          const VendorCode& vendor,
                                                          uint64_t programId,
                                                          DateTime startDate,
                                                          DateTime endDate)
{
  Row* row(0);
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(2, static_cast<int64_t>(programId));
  substParm(3, startDate);
  substParm(4, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    CommissionRuleInfo* info(QueryGetCommissionRuleHistoricalSQLStatement<
      QueryGetCommissionRuleHistorical>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETCOMMISSIONRULEHISTORICAL: NumRows: " << res.numRows()
                        << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr QueryGetAllCommissionRule::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllCommissionRule"));
std::string QueryGetAllCommissionRule::_baseSQL;
bool QueryGetAllCommissionRule::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllCommissionRule> _getAllCommissionRule;

const char* QueryGetAllCommissionRule::getQueryName() const
{
  return "GETALLCOMMISSIONRULE";
}

void QueryGetAllCommissionRule::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCommissionRuleSQLStatement<QueryGetAllCommissionRule> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCOMMISSIONRULE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetAllCommissionRule::findAllCommissionRule(std::vector<CommissionRuleInfo*>& lst)
{
  Row* row(0);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    CommissionRuleInfo* info(QueryGetAllCommissionRuleSQLStatement<QueryGetAllCommissionRule>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETALLCOMMISSIONRULE: NumRows: " << res.numRows()
                        << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
