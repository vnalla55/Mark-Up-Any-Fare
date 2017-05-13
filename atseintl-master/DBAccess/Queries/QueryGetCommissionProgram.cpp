#include "DBAccess/Queries/QueryGetCommissionProgram.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCommissionProgramSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr QueryGetCommissionProgram::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CommissionProgram"));
std::string QueryGetCommissionProgram::_baseSQL;
bool QueryGetCommissionProgram::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCommissionProgram> _getCommissionProgram;

const char* QueryGetCommissionProgram::getQueryName() const
{
  return "GETCOMMISSIONPROGRAM";
}

void QueryGetCommissionProgram::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCommissionProgramSQLStatement<QueryGetCommissionProgram> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMMISSIONPROGRAM");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetCommissionProgram::findCommissionProgram(std::vector<CommissionProgramInfo*>& lst,
                                                      const VendorCode& vendor,
                                                      uint64_t contractId)
{
  Row* row(0);
  DBResultSet res(_dbAdapt);
  substParm(vendor, 1);
  substParm(2, static_cast<int64_t>(contractId));
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    CommissionProgramInfo* info(QueryGetCommissionProgramSQLStatement<QueryGetCommissionProgram>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETCOMMISSIONPROGRAM: NumRows: " << res.numRows() << " Time = " << stopTimer()
                        << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr QueryGetCommissionProgramHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CommissionProgramHistorical"));
std::string QueryGetCommissionProgramHistorical::_baseSQL;
bool QueryGetCommissionProgramHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCommissionProgramHistorical> _getCommissionProgramHistorical;

const char* QueryGetCommissionProgramHistorical::getQueryName() const
{
  return "GETCOMMISSIONPROGRAMHISTORICAL";
}

void QueryGetCommissionProgramHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCommissionProgramHistoricalSQLStatement<QueryGetCommissionProgramHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMMISSIONPROGRAMHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetCommissionProgramHistorical::findCommissionProgram(std::vector<CommissionProgramInfo*>& lst,
                                                                const VendorCode& vendor,
                                                                uint64_t contractId,
                                                                DateTime startDate,
                                                                DateTime endDate)
{
  Row* row(0);
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(2, static_cast<int64_t>(contractId));
  substParm(3, startDate);
  substParm(4, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    CommissionProgramInfo* info(QueryGetCommissionProgramHistoricalSQLStatement<
      QueryGetCommissionProgramHistorical>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETCOMMISSIONPROGRAMHISTORICAL: NumRows: " << res.numRows()
                        << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr QueryGetAllCommissionProgram::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllCommissionProgram"));
std::string QueryGetAllCommissionProgram::_baseSQL;
bool QueryGetAllCommissionProgram::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllCommissionProgram> _getAllCommissionProgram;

const char* QueryGetAllCommissionProgram::getQueryName() const
{
  return "GETALLCOMMISSIONPROGRAM";
}

void QueryGetAllCommissionProgram::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCommissionProgramSQLStatement<QueryGetAllCommissionProgram> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCOMMISSIONPROGRAM");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetAllCommissionProgram::findAllCommissionProgram(std::vector<CommissionProgramInfo*>& lst)
{
  Row* row(0);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    CommissionProgramInfo* info(QueryGetAllCommissionProgramSQLStatement<QueryGetAllCommissionProgram>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETALLCOMMISSIONPROGRAM: NumRows: " << res.numRows()
                        << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
