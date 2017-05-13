#include "DBAccess/Queries/QueryGetCustomerSecurityHandshake.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCustomerSecurityHandshakeSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetCustomerSecurityHandshake::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetCustomerSecurityHandshake"));
std::string QueryGetCustomerSecurityHandshake::_baseSQL;
bool
QueryGetCustomerSecurityHandshake::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCustomerSecurityHandshake> _getCustomerSecurityHandshake;

const char*
QueryGetCustomerSecurityHandshake::getQueryName() const
{
  return "GETCUSTOMERSECURITYHANDSHAKE";
}

void
QueryGetCustomerSecurityHandshake::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCustomerSecurityHandshakeSQLStatement<QueryGetCustomerSecurityHandshake> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCUSTOMERSECURITYHANDSHAKE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetCustomerSecurityHandshake::findCustomerSecurityHandshake(std::vector<CustomerSecurityHandshakeInfo*>& lst,
                                                                 const PseudoCityCode& pcc,
                                                                 const Code<8>& productCode)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(pcc, 1);
  substParm(productCode, 2);
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(3, dtMinus);
  substParm(4, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    CustomerSecurityHandshakeInfo* info(
      QueryGetCustomerSecurityHandshakeSQLStatement<QueryGetCustomerSecurityHandshake>::mapRow(row, nullptr));
      lst.push_back(info);
  }
  LOG4CXX_INFO(_logger,
               "GETCUSTOMERSECURITYHANDSHAKE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetCustomerSecurityHandshakeHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetCustomerSecurityHandshakeHistorical"));
std::string QueryGetCustomerSecurityHandshakeHistorical::_baseSQL;
bool
QueryGetCustomerSecurityHandshakeHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCustomerSecurityHandshakeHistorical> _getCustomerSecurityHandshakeHistorical;

const char*
QueryGetCustomerSecurityHandshakeHistorical::getQueryName() const
{
  return "GETCUSTOMERSECURITYHANDSHAKEHISTORICAL";
}

void
QueryGetCustomerSecurityHandshakeHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCustomerSecurityHandshakeHistoricalSQLStatement<QueryGetCustomerSecurityHandshakeHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCUSTOMERSECURITYHANDSHAKEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetCustomerSecurityHandshakeHistorical::findCustomerSecurityHandshake(
  std::vector<CustomerSecurityHandshakeInfo*>& lst,
  const PseudoCityCode& pcc,
  const Code<8>& productCode,
  const DateTime& startDate,
  const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(pcc, 1);
  substParm(productCode, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    CustomerSecurityHandshakeInfo* info(QueryGetCustomerSecurityHandshakeHistoricalSQLStatement<
                                        QueryGetCustomerSecurityHandshakeHistorical>::mapRow(row, nullptr));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger,
               "GETCUSTOMERSECURITYHANDSHAKEHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllCustomerSecurityHandshake::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllCustomerSecurityHandshake"));
std::string QueryGetAllCustomerSecurityHandshake::_baseSQL;
bool
QueryGetAllCustomerSecurityHandshake::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllCustomerSecurityHandshake> _getAllCustomerSecurityHandshake;

const char*
QueryGetAllCustomerSecurityHandshake::getQueryName() const
{
  return "GETALLCUSTOMERSECURITYHANDSHAKE";
}

void
QueryGetAllCustomerSecurityHandshake::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCustomerSecurityHandshakeSQLStatement<QueryGetAllCustomerSecurityHandshake> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCUSTOMERSECURITYHANDSHAKE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllCustomerSecurityHandshake::findAllCustomerSecurityHandshake(std::vector<CustomerSecurityHandshakeInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    CustomerSecurityHandshakeInfo* info(QueryGetAllCustomerSecurityHandshakeSQLStatement<QueryGetAllCustomerSecurityHandshake>::mapRow(row, nullptr));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger,
               "GETALLCUSTOMERSECURITYHANDSHAKE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
