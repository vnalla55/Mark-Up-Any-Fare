#include "DBAccess/Queries/QueryGetFareRetailerRuleLookup.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareRetailerRuleLookupSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareRetailerRuleLookup::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetFareRetailerRuleLookup"));
std::string QueryGetFareRetailerRuleLookup::_baseSQL;
bool
QueryGetFareRetailerRuleLookup::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareRetailerRuleLookup> _getFareRetailerRuleLookup;

const char*
QueryGetFareRetailerRuleLookup::getQueryName() const
{
  return "GETFARERETAILERRULELOOKUP";
}

void
QueryGetFareRetailerRuleLookup::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareRetailerRuleLookupSQLStatement<QueryGetFareRetailerRuleLookup> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARERETAILERRULELOOKUP");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
}

void
QueryGetFareRetailerRuleLookup::findFareRetailerRuleLookup(std::vector<FareRetailerRuleLookupInfo*>& lst,
                                                           Indicator applicationType,
                                                           const PseudoCityCode& sourcePcc,
                                                           const PseudoCityCode& pcc)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, applicationType);
  substParm(sourcePcc, 2);
  substParm(pcc, 3);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareRetailerRuleLookupInfo* info(new FareRetailerRuleLookupInfo);
  info->applicationType() = applicationType;
  info->sourcePcc() = sourcePcc;
  info->pcc() = pcc;
  lst.push_back(info);

  while ((row = res.nextRow()))
  {
    QueryGetFareRetailerRuleLookupSQLStatement<QueryGetFareRetailerRuleLookup>::mapRow(row, info);
  }
  LOG4CXX_INFO(_logger, "GETFARERETAILERRULELOOKUP: NumRows: " << res.numRows() << " Time = " << stopTimer()
               << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareRetailerRuleLookupHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetFareRetailerRuleLookupHistorical"));
std::string QueryGetFareRetailerRuleLookupHistorical::_baseSQL;
bool
QueryGetFareRetailerRuleLookupHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareRetailerRuleLookupHistorical> _getFareRetailerRuleLookupHistorical;

const char*
QueryGetFareRetailerRuleLookupHistorical::getQueryName() const
{
  return "GETFARERETAILERRULELOOKUPHISTORICAL";
}

void
QueryGetFareRetailerRuleLookupHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareRetailerRuleLookupHistoricalSQLStatement<QueryGetFareRetailerRuleLookupHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARERETAILERRULELOOKUPHISTORICAL");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
}

void
QueryGetFareRetailerRuleLookupHistorical::findFareRetailerRuleLookup(
  std::vector<FareRetailerRuleLookupInfo*>& lst,
  Indicator applicationType,
  const PseudoCityCode& sourcePcc,
  const PseudoCityCode& pcc,
  const DateTime& startDate,
  const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, applicationType);
  substParm(sourcePcc, 2);
  substParm(pcc, 3);
  substParm(4, startDate);
  substParm(5, endDate);
  substParm(6, startDate);
  substParm(7, endDate);

  substParm(8, applicationType);
  substParm(sourcePcc, 9);
  substParm(pcc, 10);
  substParm(11, startDate);
  substParm(12, endDate);
  substParm(13, startDate);
  substParm(14, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareRetailerRuleLookupInfo* info(new FareRetailerRuleLookupInfo);
  info->applicationType() = applicationType;
  info->sourcePcc() = sourcePcc;
  info->pcc() = pcc;
  lst.push_back(info);


  while ((row = res.nextRow()))
  {
    QueryGetFareRetailerRuleLookupHistoricalSQLStatement<QueryGetFareRetailerRuleLookupHistorical>::mapRow(row, info);
  }
  LOG4CXX_INFO(_logger, "GETFARERETAILERRULELOOKUPHISTORICAL: NumRows: "
               << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

} // tse
