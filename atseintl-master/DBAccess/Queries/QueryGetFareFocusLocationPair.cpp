#include "DBAccess/Queries/QueryGetFareFocusLocationPair.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusLocationPairSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareFocusLocationPair::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusLocationPair"));
std::string QueryGetFareFocusLocationPair::_baseSQL;
bool
QueryGetFareFocusLocationPair::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusLocationPair> _getFareFocusLocationPair;

const char*
QueryGetFareFocusLocationPair::getQueryName() const
{
  return "GETFAREFOCUSLOCATIONPAIR";
}

void
QueryGetFareFocusLocationPair::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusLocationPairSQLStatement<QueryGetFareFocusLocationPair> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSLOCATIONPAIR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusLocationPair::findFareFocusLocationPair(std::vector<FareFocusLocationPairInfo*>& lst,
                                                   uint64_t locationPairItemNo)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(locationPairItemNo));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusLocationPairInfo* info(nullptr);
  FareFocusLocationPairInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusLocationPairSQLStatement<QueryGetFareFocusLocationPair>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSLOCATIONPAIR: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareFocusLocationPairHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusLocationPairHistorical"));
std::string QueryGetFareFocusLocationPairHistorical::_baseSQL;
bool
QueryGetFareFocusLocationPairHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusLocationPairHistorical> _getFareFocusLocationPairHistorical;

const char*
QueryGetFareFocusLocationPairHistorical::getQueryName() const
{
  return "GETFAREFOCUSLOCATIONPAIRHISTORICAL";
}

void
QueryGetFareFocusLocationPairHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusLocationPairHistoricalSQLStatement<QueryGetFareFocusLocationPairHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSLOCATIONPAIRHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusLocationPairHistorical::findFareFocusLocationPair(
    std::vector<FareFocusLocationPairInfo*>& lst,
    uint64_t locationPairItemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(locationPairItemNo));
  substParm(2, startDate);
  substParm(3, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusLocationPairInfo* info(nullptr);
  FareFocusLocationPairInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusLocationPairHistoricalSQLStatement<
        QueryGetFareFocusLocationPairHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSLOCATIONPAIRHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareFocusLocationPair::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllFareFocusLocationPair"));
std::string QueryGetAllFareFocusLocationPair::_baseSQL;
bool
QueryGetAllFareFocusLocationPair::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareFocusLocationPair> _getAllFareFocusLocationPair;

const char*
QueryGetAllFareFocusLocationPair::getQueryName() const
{
  return "GETALLFAREFOCUSLOCATIONPAIR";
}

void
QueryGetAllFareFocusLocationPair::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareFocusLocationPairSQLStatement<QueryGetAllFareFocusLocationPair> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREFOCUSLOCATIONPAIR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareFocusLocationPair::findAllFareFocusLocationPair(
    std::vector<FareFocusLocationPairInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusLocationPairInfo* info(nullptr);
  FareFocusLocationPairInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareFocusLocationPairSQLStatement<QueryGetAllFareFocusLocationPair>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREFOCUSLOCATIONPAIR: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
