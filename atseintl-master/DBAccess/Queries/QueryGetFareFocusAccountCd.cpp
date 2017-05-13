#include "DBAccess/Queries/QueryGetFareFocusAccountCd.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusAccountCdSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareFocusAccountCd::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusAccountCd"));
std::string QueryGetFareFocusAccountCd::_baseSQL;
bool
QueryGetFareFocusAccountCd::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusAccountCd> _getFareFocusAccountCd;

const char*
QueryGetFareFocusAccountCd::getQueryName() const
{
  return "GETFAREFOCUSACCOUNTCD";
}

void
QueryGetFareFocusAccountCd::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusAccountCdSQLStatement<QueryGetFareFocusAccountCd> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSACCOUNTCD");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusAccountCd::findFareFocusAccountCd(std::vector<FareFocusAccountCdInfo*>& lst,
                                                   uint64_t accountCdItemNo)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(accountCdItemNo));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusAccountCdInfo* info(nullptr);
  FareFocusAccountCdInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusAccountCdSQLStatement<QueryGetFareFocusAccountCd>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSACCOUNTCD: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareFocusAccountCdHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusAccountCdHistorical"));
std::string QueryGetFareFocusAccountCdHistorical::_baseSQL;
bool
QueryGetFareFocusAccountCdHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusAccountCdHistorical> _getFareFocusAccountCdHistorical;

const char*
QueryGetFareFocusAccountCdHistorical::getQueryName() const
{
  return "GETFAREFOCUSACCOUNTCDHISTORICAL";
}

void
QueryGetFareFocusAccountCdHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusAccountCdHistoricalSQLStatement<QueryGetFareFocusAccountCdHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSACCOUNTCDHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusAccountCdHistorical::findFareFocusAccountCd(
    std::vector<FareFocusAccountCdInfo*>& lst,
    uint64_t accountCdItemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(accountCdItemNo));
  substParm(2, startDate);
  substParm(3, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusAccountCdInfo* info(nullptr);
  FareFocusAccountCdInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusAccountCdHistoricalSQLStatement<
        QueryGetFareFocusAccountCdHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSACCOUNTCDHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareFocusAccountCd::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllFareFocusAccountCd"));
std::string QueryGetAllFareFocusAccountCd::_baseSQL;
bool
QueryGetAllFareFocusAccountCd::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareFocusAccountCd> _getAllFareFocusAccountCd;

const char*
QueryGetAllFareFocusAccountCd::getQueryName() const
{
  return "GETALLFAREFOCUSACCOUNTCD";
}

void
QueryGetAllFareFocusAccountCd::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareFocusAccountCdSQLStatement<QueryGetAllFareFocusAccountCd> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREFOCUSACCOUNTCD");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareFocusAccountCd::findAllFareFocusAccountCd(
    std::vector<FareFocusAccountCdInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusAccountCdInfo* info(nullptr);
  FareFocusAccountCdInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareFocusAccountCdSQLStatement<QueryGetAllFareFocusAccountCd>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREFOCUSACCOUNTCD: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
