#include "DBAccess/Queries/QueryGetCommissionContract.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCommissionContractSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr QueryGetCommissionContract::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CommissionContract"));
std::string QueryGetCommissionContract::_baseSQL;
bool QueryGetCommissionContract::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCommissionContract> _getCommissionContract;

const char* QueryGetCommissionContract::getQueryName() const
{
  return "GETCOMMISSIONCONTRACT";
}

void QueryGetCommissionContract::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCommissionContractSQLStatement<QueryGetCommissionContract> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMMISSIONCONTRACT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetCommissionContract::findCommissionContract(std::vector<CommissionContractInfo*>& lst,
                                                        const VendorCode& vendor,
                                                        const CarrierCode& carrier,
                                                        const PseudoCityCode& pcc)
{
  Row* row(0);
  DBResultSet res(_dbAdapt);
  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(pcc, 3);
  DateTime dt(DateTime::localTime());
  substParm(4, dt);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    CommissionContractInfo* info(QueryGetCommissionContractSQLStatement<QueryGetCommissionContract>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETCOMMISSIONCONTRACT: NumRows: " << res.numRows() << " Time = " << stopTimer()
                        << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr QueryGetCommissionContractHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CommissionContractHistorical"));
std::string QueryGetCommissionContractHistorical::_baseSQL;
bool QueryGetCommissionContractHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCommissionContractHistorical> _getCommissionContractHistorical;

const char* QueryGetCommissionContractHistorical::getQueryName() const
{
  return "GETCOMMISSIONCONTRACTHISTORICAL";
}

void QueryGetCommissionContractHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCommissionContractHistoricalSQLStatement<QueryGetCommissionContractHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMMISSIONCONTRACTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetCommissionContractHistorical::findCommissionContract(std::vector<CommissionContractInfo*>& lst,
                                                                  const VendorCode& vendor,
                                                                  const CarrierCode& carrier,
                                                                  const PseudoCityCode& pcc,
                                                                  DateTime startDate,
                                                                  DateTime endDate)
{
  Row* row(0);
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(pcc, 3);
  substParm(4, startDate);
  substParm(5, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    CommissionContractInfo* info(QueryGetCommissionContractHistoricalSQLStatement<
      QueryGetCommissionContractHistorical>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETCOMMISSIONCONTRACTHISTORICAL: NumRows: " << res.numRows()
                        << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr QueryGetAllCommissionContract::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllCommissionContract"));
std::string QueryGetAllCommissionContract::_baseSQL;
bool QueryGetAllCommissionContract::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllCommissionContract> _getAllCommissionContract;

const char* QueryGetAllCommissionContract::getQueryName() const
{
  return "GETALLCOMMISSIONCONTRACT";
}

void QueryGetAllCommissionContract::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCommissionContractSQLStatement<QueryGetAllCommissionContract> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCOMMISSIONCONTRACT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void QueryGetAllCommissionContract::findAllCommissionContract(std::vector<CommissionContractInfo*>& lst)
{
  Row* row(0);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    CommissionContractInfo* info(QueryGetAllCommissionContractSQLStatement<QueryGetAllCommissionContract>::mapRow(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger, "GETALLCOMMISSIONCONTRACT: NumRows: " << res.numRows()
                        << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
