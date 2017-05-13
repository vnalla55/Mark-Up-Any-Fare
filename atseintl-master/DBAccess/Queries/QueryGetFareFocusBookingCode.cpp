#include "DBAccess/Queries/QueryGetFareFocusBookingCode.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusBookingCodeSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareFocusBookingCode::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusBookingCode"));
std::string QueryGetFareFocusBookingCode::_baseSQL;
bool
QueryGetFareFocusBookingCode::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusBookingCode> _getFareFocusBookingCode;

const char*
QueryGetFareFocusBookingCode::getQueryName() const
{
  return "GETFAREFOCUSBOOKINGCODE";
}

void
QueryGetFareFocusBookingCode::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusBookingCodeSQLStatement<QueryGetFareFocusBookingCode> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSBOOKINGCODE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusBookingCode::findFareFocusBookingCode(std::vector<FareFocusBookingCodeInfo*>& lst,
                                                       uint64_t bookingCodeItemNo)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(bookingCodeItemNo));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusBookingCodeInfo* info(nullptr);
  FareFocusBookingCodeInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusBookingCodeSQLStatement<QueryGetFareFocusBookingCode>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSBOOKINGCODE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareFocusBookingCodeHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusBookingCodeHistorical"));
std::string QueryGetFareFocusBookingCodeHistorical::_baseSQL;
bool
QueryGetFareFocusBookingCodeHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusBookingCodeHistorical> _getFareFocusBookingCodeHistorical;

const char*
QueryGetFareFocusBookingCodeHistorical::getQueryName() const
{
  return "GETFAREFOCUSBOOKINGCODEHISTORICAL";
}

void
QueryGetFareFocusBookingCodeHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusBookingCodeHistoricalSQLStatement<QueryGetFareFocusBookingCodeHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSBOOKINGCODEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusBookingCodeHistorical::findFareFocusBookingCode(
    std::vector<FareFocusBookingCodeInfo*>& lst,
    uint64_t bookingCodeItemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(bookingCodeItemNo));
  substParm(2, startDate);
  substParm(3, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusBookingCodeInfo* info(nullptr);
  FareFocusBookingCodeInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusBookingCodeHistoricalSQLStatement<
        QueryGetFareFocusBookingCodeHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSBOOKINGCODEHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareFocusBookingCode::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllFareFocusBookingCode"));
std::string QueryGetAllFareFocusBookingCode::_baseSQL;
bool
QueryGetAllFareFocusBookingCode::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareFocusBookingCode> _getAllFareFocusBookingCode;

const char*
QueryGetAllFareFocusBookingCode::getQueryName() const
{
  return "GETALLFAREFOCUSBOOKINGCODE";
}

void
QueryGetAllFareFocusBookingCode::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareFocusBookingCodeSQLStatement<QueryGetAllFareFocusBookingCode> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREFOCUSBOOKINGCODE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareFocusBookingCode::findAllFareFocusBookingCode(
    std::vector<FareFocusBookingCodeInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusBookingCodeInfo* info(nullptr);
  FareFocusBookingCodeInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareFocusBookingCodeSQLStatement<QueryGetAllFareFocusBookingCode>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREFOCUSBOOKINGCODE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
