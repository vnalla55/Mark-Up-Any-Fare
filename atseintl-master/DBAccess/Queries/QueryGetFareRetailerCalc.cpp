#include "DBAccess/Queries/QueryGetFareRetailerCalc.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareRetailerCalcSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareRetailerCalc::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareRetailerCalc"));
std::string QueryGetFareRetailerCalc::_baseSQL;
bool
QueryGetFareRetailerCalc::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareRetailerCalc> _getFareRetailerCalc;

const char*
QueryGetFareRetailerCalc::getQueryName() const
{
  return "GETFARERETAILERCALC";
}

void
QueryGetFareRetailerCalc::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareRetailerCalcSQLStatement<QueryGetFareRetailerCalc> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARERETAILERCALC");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareRetailerCalc::findFareRetailerCalc(std::vector<FareRetailerCalcInfo*>& lst,
                                                 uint64_t fareRetailerCalcItemNo)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(fareRetailerCalcItemNo));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareRetailerCalcInfo* info(nullptr);
  FareRetailerCalcInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareRetailerCalcSQLStatement<QueryGetFareRetailerCalc>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFARERETAILERCALC: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareRetailerCalcHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareRetailerCalcHistorical"));
std::string QueryGetFareRetailerCalcHistorical::_baseSQL;
bool
QueryGetFareRetailerCalcHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareRetailerCalcHistorical> _getFareRetailerCalcHistorical;

const char*
QueryGetFareRetailerCalcHistorical::getQueryName() const
{
  return "GETFARERETAILERCALCHISTORICAL";
}

void
QueryGetFareRetailerCalcHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareRetailerCalcHistoricalSQLStatement<QueryGetFareRetailerCalcHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARERETAILERCALCHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareRetailerCalcHistorical::findFareRetailerCalc(
    std::vector<FareRetailerCalcInfo*>& lst,
    uint64_t fareRetailerCalcItemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(fareRetailerCalcItemNo));
  substParm(2, startDate);
  substParm(3, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareRetailerCalcInfo* info(nullptr);
  FareRetailerCalcInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareRetailerCalcHistoricalSQLStatement<
        QueryGetFareRetailerCalcHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFARERETAILERCALCHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareRetailerCalc::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllFareRetailerCalc"));
std::string QueryGetAllFareRetailerCalc::_baseSQL;
bool
QueryGetAllFareRetailerCalc::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareRetailerCalc> _getAllFareRetailerCalc;

const char*
QueryGetAllFareRetailerCalc::getQueryName() const
{
  return "GETALLFARERETAILERCALC";
}

void
QueryGetAllFareRetailerCalc::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareRetailerCalcSQLStatement<QueryGetAllFareRetailerCalc> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFARERETAILERCALC");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareRetailerCalc::findAllFareRetailerCalc(
    std::vector<FareRetailerCalcInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareRetailerCalcInfo* info(nullptr);
  FareRetailerCalcInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareRetailerCalcSQLStatement<QueryGetAllFareRetailerCalc>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFARERETAILERCALC: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse



