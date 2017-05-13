//----------------------------------------------------------------------------
//     � 2010, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetBaggageSectorException.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBaggageSectorExceptionSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetBaggageSectorException::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetBaggageSectorException"));
std::string QueryGetBaggageSectorException::_baseSQL;
bool QueryGetBaggageSectorException::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBaggageSectorException> g_GetBaggageSectorException;

const char*
QueryGetBaggageSectorException::getQueryName() const
{
  return "QUERYGETBAGGAGESECTOREXCEPTION";
}

void
QueryGetBaggageSectorException::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBaggageSectorExceptionSQLStatement<QueryGetBaggageSectorException> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETBAGGAGESECTOREXCEPTION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBaggageSectorException::findBaggageSectorException(
    std::vector<tse::BaggageSectorException*>& exceptions, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  substParm(1, carrier);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    exceptions.push_back(QueryGetBaggageSectorExceptionSQLStatement<
        QueryGetBaggageSectorException>::mapRowToBaggageSectorException(row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETBAGGAGESECTOREXCEPTION: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//
//  QueryGetBaggageSectorExceptionHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetBaggageSectorExceptionHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetBaggageSectorExceptionHistorical"));
std::string QueryGetBaggageSectorExceptionHistorical::_baseSQL;
bool QueryGetBaggageSectorExceptionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBaggageSectorExceptionHistorical>
g_GetBaggageSectorExceptionHistorical;

const char*
QueryGetBaggageSectorExceptionHistorical::getQueryName() const
{
  return "QUERYGETBAGGAGESECTOREXCEPTIONHISTORICAL";
}

void
QueryGetBaggageSectorExceptionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBaggageSectorExceptionHistoricalSQLStatement<QueryGetBaggageSectorExceptionHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETBAGGAGESECTOREXCEPTIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBaggageSectorExceptionHistorical::findBaggageSectorException(
    std::vector<tse::BaggageSectorException*>& exceptions, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    exceptions.push_back(QueryGetBaggageSectorExceptionHistoricalSQLStatement<
        QueryGetBaggageSectorExceptionHistorical>::mapRowToBaggageSectorException(row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETBAGGAGESECTOREXCEPTIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//
//  QueryGetAllBaggageSectorException()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllBaggageSectorException::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllBaggageSectorException"));
std::string QueryGetAllBaggageSectorException::_baseSQL;
bool QueryGetAllBaggageSectorException::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllBaggageSectorException> g_GetAllBaggageSectorException;

const char*
QueryGetAllBaggageSectorException::getQueryName() const
{
  return "QUERYGETALLBAGGAGESECTOREXCEPTION";
}
void
QueryGetAllBaggageSectorException::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllBaggageSectorExceptionSQLStatement<QueryGetAllBaggageSectorException> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETALLBAGGAGESECTOREXCEPTION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllBaggageSectorException::findAllBaggageSectorException(
    std::vector<tse::BaggageSectorException*>& exceptions)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    exceptions.push_back(QueryGetAllBaggageSectorExceptionSQLStatement<
        QueryGetAllBaggageSectorException>::mapRowToBaggageSectorException(row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETALLBAGGAGESECTOREXCEPTION: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//
//  QueryGetAllBaggageSectorExceptionHistorical()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllBaggageSectorExceptionHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetAllBaggageSectorExceptionHistorical"));
std::string QueryGetAllBaggageSectorExceptionHistorical::_baseSQL;
bool QueryGetAllBaggageSectorExceptionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllBaggageSectorExceptionHistorical>
g_GetAllBaggageSectorExceptionHistorical;

const char*
QueryGetAllBaggageSectorExceptionHistorical::getQueryName() const
{
  return "QUERYGETALLBAGGAGESECTOREXCEPTIONHISTORICAL";
}

void
QueryGetAllBaggageSectorExceptionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllBaggageSectorExceptionHistoricalSQLStatement<
        QueryGetAllBaggageSectorExceptionHistorical> sqlStatement;
    _baseSQL =
        sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETALLBAGGAGESECTOREXCEPTIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllBaggageSectorExceptionHistorical::findAllBaggageSectorException(
    std::vector<tse::BaggageSectorException*>& exceptions)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    exceptions.push_back(QueryGetAllBaggageSectorExceptionHistoricalSQLStatement<
        QueryGetAllBaggageSectorExceptionHistorical>::mapRowToBaggageSectorException(row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETALLBAGGAGESECTOREXCEPTIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
}
