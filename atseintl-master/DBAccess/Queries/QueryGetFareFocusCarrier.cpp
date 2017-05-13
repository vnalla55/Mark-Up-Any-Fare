#include "DBAccess/Queries/QueryGetFareFocusCarrier.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusCarrierSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareFocusCarrier::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusCarrier"));
std::string QueryGetFareFocusCarrier::_baseSQL;
bool
QueryGetFareFocusCarrier::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusCarrier> _getFareFocusCarrier;

const char*
QueryGetFareFocusCarrier::getQueryName() const
{
  return "GETFAREFOCUSCARRIER";
}

void
QueryGetFareFocusCarrier::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusCarrierSQLStatement<QueryGetFareFocusCarrier> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSCARRIER");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusCarrier::findFareFocusCarrier(std::vector<FareFocusCarrierInfo*>& lst,
                                                   uint64_t carrierItemNo)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(carrierItemNo));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusCarrierInfo* info(nullptr);
  FareFocusCarrierInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusCarrierSQLStatement<QueryGetFareFocusCarrier>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSCARRIER: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareFocusCarrierHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.FareFocusCarrierHistorical"));
std::string QueryGetFareFocusCarrierHistorical::_baseSQL;
bool
QueryGetFareFocusCarrierHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareFocusCarrierHistorical> _getFareFocusCarrierHistorical;

const char*
QueryGetFareFocusCarrierHistorical::getQueryName() const
{
  return "GETFAREFOCUSCARRIERHISTORICAL";
}

void
QueryGetFareFocusCarrierHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareFocusCarrierHistoricalSQLStatement<QueryGetFareFocusCarrierHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREFOCUSCARRIERHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareFocusCarrierHistorical::findFareFocusCarrier(
    std::vector<FareFocusCarrierInfo*>& lst,
    uint64_t carrierItemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(carrierItemNo));
  substParm(2, startDate);
  substParm(3, endDate);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusCarrierInfo* info(nullptr);
  FareFocusCarrierInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareFocusCarrierHistoricalSQLStatement<
        QueryGetFareFocusCarrierHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFAREFOCUSCARRIERHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareFocusCarrier::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllFareFocusCarrier"));
std::string QueryGetAllFareFocusCarrier::_baseSQL;
bool
QueryGetAllFareFocusCarrier::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareFocusCarrier> _getAllFareFocusCarrier;

const char*
QueryGetAllFareFocusCarrier::getQueryName() const
{
  return "GETALLFAREFOCUSCARRIER";
}

void
QueryGetAllFareFocusCarrier::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareFocusCarrierSQLStatement<QueryGetAllFareFocusCarrier> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREFOCUSCARRIER");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareFocusCarrier::findAllFareFocusCarrier(
    std::vector<FareFocusCarrierInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareFocusCarrierInfo* info(nullptr);
  FareFocusCarrierInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareFocusCarrierSQLStatement<QueryGetAllFareFocusCarrier>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREFOCUSCARRIER: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse

