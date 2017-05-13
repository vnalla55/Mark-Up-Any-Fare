#include "DBAccess/Queries/QueryGetFareRetailerResultingFareAttr.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareRetailerResultingFareAttrSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareRetailerResultingFareAttr::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareRetailerResultingFareAttr"));
std::string QueryGetFareRetailerResultingFareAttr::_baseSQL;
bool
QueryGetFareRetailerResultingFareAttr::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareRetailerResultingFareAttr> _getFareRetailerResultingFareAttr;

const char*
QueryGetFareRetailerResultingFareAttr::getQueryName() const
{
  return "GETFARERETAILERRESULTINGFAREATTR";
}

void
QueryGetFareRetailerResultingFareAttr::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareRetailerResultingFareAttrSQLStatement<QueryGetFareRetailerResultingFareAttr> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARERETAILERRESULTINGFAREATTR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareRetailerResultingFareAttr::findFareRetailerResultingFareAttr(std::vector<FareRetailerResultingFareAttrInfo*>& lst,
                                                                         uint64_t resultingFareAttrItemNo)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(1, static_cast<int64_t>(resultingFareAttrItemNo));
  //substCurrentDate();
  DateTime dtMinus(DateTime::localTime());
  dtMinus = dtMinus.subtractDays(1);
  substParm(2, dtMinus);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    FareRetailerResultingFareAttrInfo*
    info(QueryGetFareRetailerResultingFareAttrSQLStatement<QueryGetFareRetailerResultingFareAttr>::mapRow(row, nullptr));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger,
               "GETFARERETAILERRESULTINGFAREATTR: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareRetailerResultingFareAttrHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareRetailerResultingFareAttrHistorical"));
std::string QueryGetFareRetailerResultingFareAttrHistorical::_baseSQL;
bool
QueryGetFareRetailerResultingFareAttrHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetFareRetailerResultingFareAttrHistorical> _getFareRetailerresultingFareAttrHistorical;

const char*
QueryGetFareRetailerResultingFareAttrHistorical::getQueryName() const
{
  return "GETFARERETAILERRESULTINGFAREATTRHISTORICAL";
}

void
QueryGetFareRetailerResultingFareAttrHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareRetailerResultingFareAttrHistoricalSQLStatement<QueryGetFareRetailerResultingFareAttrHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARERETAILERRESULTINGFAREATTRHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareRetailerResultingFareAttrHistorical::findFareRetailerResultingFareAttr(std::vector<FareRetailerResultingFareAttrInfo*>& lst,
                                                                                    uint64_t resultingFareAttrItemNo,
                                                                                    const DateTime& startDate,
                                                                                    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(1, static_cast<int64_t>(resultingFareAttrItemNo));
  substParm(2, startDate);
  substParm(3, endDate);
  substCurrentDate();

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareRetailerResultingFareAttrInfo* info(nullptr);
  FareRetailerResultingFareAttrInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetFareRetailerResultingFareAttrHistoricalSQLStatement<
        QueryGetFareRetailerResultingFareAttrHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETFARERETAILERRESULTINGFAREATTRHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareRetailerResultingFareAttr::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareRetailerResultingFareAttr"));
std::string QueryGetAllFareRetailerResultingFareAttr::_baseSQL;
bool
QueryGetAllFareRetailerResultingFareAttr::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllFareRetailerResultingFareAttr> _getAllFareRetailerResultingFareAttr;

const char*
QueryGetAllFareRetailerResultingFareAttr::getQueryName() const
{
  return "GETALLFARERETAILERRESULTINGFAREATTR";
}

void
QueryGetAllFareRetailerResultingFareAttr::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareRetailerResultingFareAttrSQLStatement<QueryGetAllFareRetailerResultingFareAttr> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFARERETAILERRESULTINGFAREATTR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareRetailerResultingFareAttr::findAllFareRetailerResultingFareAttr(std::vector<FareRetailerResultingFareAttrInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareRetailerResultingFareAttrInfo* info(nullptr);
  FareRetailerResultingFareAttrInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllFareRetailerResultingFareAttrSQLStatement<QueryGetAllFareRetailerResultingFareAttr>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFARERETAILERRESULTINGFAREATTR: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}

} // tse
