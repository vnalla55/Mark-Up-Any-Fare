//----------------------------------------------------------------------------
//          File:           QueryGetInterlineTicketCarrier.cpp
//          Description:    QueryGetInterlineTicketCarrier
//          Created:        10/1/2010
//          Authors:        Anna Kulig
//
//          Updates:
//
//     (c)2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetInterlineTicketCarrier.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetInterlineTicketCarrierSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetInterlineTicketCarrier::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetInterlineTicketCarrier"));
std::string QueryGetInterlineTicketCarrier::_baseSQL;
bool QueryGetInterlineTicketCarrier::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetInterlineTicketCarrier> g_GetInterlineTicketCarrier;

const char*
QueryGetInterlineTicketCarrier::getQueryName() const
{
  return "GETINTERLINETICKETCARRIER";
}

void
QueryGetInterlineTicketCarrier::initialize()
{
  if (!_isInitialized)
  {
    QueryGetInterlineTicketCarrierSQLStatement<QueryGetInterlineTicketCarrier> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETINTERLINETICKETCARRIER");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetInterlineTicketCarrier::findInterlineTicketCarrier(
    std::vector<tse::InterlineTicketCarrierInfo*>& interlineCarriers, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    interlineCarriers.push_back(QueryGetInterlineTicketCarrierSQLStatement<
        QueryGetInterlineTicketCarrier>::mapRowToInterlineTicketCarrier(row));
  }

  LOG4CXX_INFO(_logger,
               "GETINTERLINETICKETCARRIER: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findInterlineTicketCarrier()

///////////////////////////////////////////////////////////
//
//  QueryGetInterlineTicketCarrierHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetInterlineTicketCarrierHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetInterlineTicketCarrierHistorical"));
std::string QueryGetInterlineTicketCarrierHistorical::_baseSQL;
bool QueryGetInterlineTicketCarrierHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetInterlineTicketCarrierHistorical>
g_GetInterlineTicketCarrierHistorical;

const char*
QueryGetInterlineTicketCarrierHistorical::getQueryName() const
{
  return "GETINTERLINETICKETCARRIERHISTORICAL";
}

void
QueryGetInterlineTicketCarrierHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetInterlineTicketCarrierHistoricalSQLStatement<QueryGetInterlineTicketCarrierHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETINTERLINETICKETCARRIERHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetInterlineTicketCarrierHistorical::findInterlineTicketCarrier(
    std::vector<tse::InterlineTicketCarrierInfo*>& interlineCarriers, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    interlineCarriers.push_back(QueryGetInterlineTicketCarrierHistoricalSQLStatement<
        QueryGetInterlineTicketCarrierHistorical>::mapRowToInterlineTicketCarrier(row));
  }

  LOG4CXX_INFO(_logger,
               "GETINTERLINETICKETCARRIERHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findInterlineTicketCarrier()

///////////////////////////////////////////////////////////
//
//  QueryGetAllInterlineTicketCarrier
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllInterlineTicketCarrier::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllInterlineTicketCarrier"));
std::string QueryGetAllInterlineTicketCarrier::_baseSQL;
bool QueryGetAllInterlineTicketCarrier::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllInterlineTicketCarrier> g_GetAllInterlineTicketCarrier;

const char*
QueryGetAllInterlineTicketCarrier::getQueryName() const
{
  return "GETALLINTERLINETICKETCARRIER";
}

void
QueryGetAllInterlineTicketCarrier::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllInterlineTicketCarrierSQLStatement<QueryGetAllInterlineTicketCarrier> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINTERLINETICKETCARRIER");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllInterlineTicketCarrier::findAllInterlineTicketCarrier(
    std::vector<tse::InterlineTicketCarrierInfo*>& interlineCarriers)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    interlineCarriers.push_back(QueryGetAllInterlineTicketCarrierSQLStatement<
        QueryGetAllInterlineTicketCarrier>::mapRowToInterlineTicketCarrier(row));
  }

  LOG4CXX_INFO(_logger,
               "GETALLINTERLINETICKETCARRIER: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllInterlineTicketCarrier()

///////////////////////////////////////////////////////////
//
//  QueryGetAllInterlineTicketCarrierHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllInterlineTicketCarrierHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetAllInterlineTicketCarrierHistorical"));
std::string QueryGetAllInterlineTicketCarrierHistorical::_baseSQL;
bool QueryGetAllInterlineTicketCarrierHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllInterlineTicketCarrierHistorical>
g_GetAllInterlineTicketCarrierHistorical;

const char*
QueryGetAllInterlineTicketCarrierHistorical::getQueryName() const
{
  return "GETALLINTERLINETICKETCARRIERHISTORICAL";
}

void
QueryGetAllInterlineTicketCarrierHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllInterlineTicketCarrierHistoricalSQLStatement<
        QueryGetAllInterlineTicketCarrierHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINTERLINETICKETCARRIERHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllInterlineTicketCarrierHistorical::findAllInterlineTicketCarrier(
    std::vector<tse::InterlineTicketCarrierInfo*>& interlineCarriers)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    interlineCarriers.push_back(QueryGetAllInterlineTicketCarrierHistoricalSQLStatement<
        QueryGetAllInterlineTicketCarrierHistorical>::mapRowToInterlineTicketCarrier(row));
  }

  LOG4CXX_INFO(_logger,
               "GETALLINTERLINETICKETCARRIERHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllInterlineTicketCarrier()
}
