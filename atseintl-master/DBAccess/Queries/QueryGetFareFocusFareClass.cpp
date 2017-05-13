#include "DBAccess/Queries/QueryGetFareFocusFareClass.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusFareClassSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareFocusFareClass::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusFareClass"));
std::string QueryGetFareFocusFareClass::_baseSQL;
bool
QueryGetFareFocusFareClass::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusFareClass> _getFareFocusFareClass;

const char*
QueryGetFareFocusFareClass::getQueryName() const
{
  return "GETFAREFOCUSFARECLASS";
}

void
QueryGetFareFocusFareClass::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusFareClassSQLStatement<QueryGetFareFocusFareClass> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSFARECLASS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusFareClass::findFareFocusFareClass(std::vector<FareFocusFareClassInfo*>& lst,
                                                   uint64_t fareClassItemNo)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(fareClassItemNo));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusFareClassInfo* info(nullptr);
  FareFocusFareClassInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusFareClassSQLStatement<QueryGetFareFocusFareClass>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSFARECLASS: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareFocusFareClassHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusFareClassHistorical"));
std::string QueryGetFareFocusFareClassHistorical::_baseSQL;
bool
QueryGetFareFocusFareClassHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusFareClassHistorical> _getFareFocusFareClassHistorical;

const char*
QueryGetFareFocusFareClassHistorical::getQueryName() const
{
  return "GETFAREFOCUSFARECLASSHISTORICAL";
}

void
QueryGetFareFocusFareClassHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusFareClassHistoricalSQLStatement<QueryGetFareFocusFareClassHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSFARECLASSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusFareClassHistorical::findFareFocusFareClass(
    std::vector<FareFocusFareClassInfo*>& lst,
    uint64_t fareClassItemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(fareClassItemNo));
  substParm(2, startDate);
  substParm(3, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusFareClassInfo* info(nullptr);
  FareFocusFareClassInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusFareClassHistoricalSQLStatement<
        QueryGetFareFocusFareClassHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSFARECLASSHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareFocusFareClass::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllFareFocusFareClass"));
std::string QueryGetAllFareFocusFareClass::_baseSQL;
bool
QueryGetAllFareFocusFareClass::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareFocusFareClass> _getAllFareFocusFareClass;

const char*
QueryGetAllFareFocusFareClass::getQueryName() const
{
  return "GETALLFAREFOCUSFARECLASS";
}

void
QueryGetAllFareFocusFareClass::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareFocusFareClassSQLStatement<QueryGetAllFareFocusFareClass> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREFOCUSFARECLASS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareFocusFareClass::findAllFareFocusFareClass(
    std::vector<FareFocusFareClassInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusFareClassInfo* info(nullptr);
  FareFocusFareClassInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareFocusFareClassSQLStatement<QueryGetAllFareFocusFareClass>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREFOCUSFARECLASS: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
