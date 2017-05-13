//----------------------------------------------------------------------------
//          File:           QueryGetInterlineTicketCarrierStatus.cpp
//          Description:    QueryGetInterlineTicketCarrierStatus
//          Created:        02/27/2012
//          Authors:        M Dantas
//
//          Updates:
//
//     (c)2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetInterlineTicketCarrierStatus.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetInterlineTicketCarrierStatusSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetInterlineTicketCarrierStatus::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetInterlineTicketCarrierStatus"));
std::string QueryGetInterlineTicketCarrierStatus::_baseSQL;
bool QueryGetInterlineTicketCarrierStatus::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetInterlineTicketCarrierStatus> g_GetInterlineTicketCarrierStatus;

const char*
QueryGetInterlineTicketCarrierStatus::getQueryName() const
{
  return "GETINTERLINETICKETCARRIERSTATUS";
}

void
QueryGetInterlineTicketCarrierStatus::initialize()
{
  if (!_isInitialized)
  {
    QueryGetInterlineTicketCarrierStatusSQLStatement<QueryGetInterlineTicketCarrierStatus>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETINTERLINETICKETCARRIERSTATUS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetInterlineTicketCarrierStatus::findInterlineTicketCarrierStatus(
    std::vector<tse::InterlineTicketCarrierStatus*>& interlineTicketCarriersStatus,
    const CarrierCode& carrier,
    const CrsCode& crsCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, crsCode);
  substCurrentDate();

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    interlineTicketCarriersStatus.push_back(QueryGetInterlineTicketCarrierStatusSQLStatement<
        QueryGetInterlineTicketCarrierStatus>::mapRowToInterlineTicketCarrierStatus(row));
  }

  LOG4CXX_INFO(_logger,
               "GETINTERLINETICKETCARRIERSTATUS: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findInterlineTicketCarrierStatus()

///////////////////////////////////////////////////////////
//
//  QueryGetInterlineTicketCarriertatusHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetInterlineTicketCarrierStatusHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetInterlineTicketCarrierStatusHistorical"));
std::string QueryGetInterlineTicketCarrierStatusHistorical::_baseSQL;
bool QueryGetInterlineTicketCarrierStatusHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetInterlineTicketCarrierStatusHistorical>
g_GetInterlineTicketCarrierStatusHistorical;

const char*
QueryGetInterlineTicketCarrierStatusHistorical::getQueryName() const
{
  return "GETINTERLINETICKETCARRIERSTATUSHISTORICAL";
}

void
QueryGetInterlineTicketCarrierStatusHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetInterlineTicketCarrierStatusHistoricalSQLStatement<
        QueryGetInterlineTicketCarrierStatusHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETINTERLINETICKETCARRIERSTATUSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetInterlineTicketCarrierStatusHistorical::findInterlineTicketCarrierStatus(
    std::vector<tse::InterlineTicketCarrierStatus*>& interlineTicketCarriersStatus,
    const CarrierCode& carrier,
    const CrsCode& crsCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  substParm(1, carrier);
  substParm(2, crsCode);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    interlineTicketCarriersStatus.push_back(
        QueryGetInterlineTicketCarrierStatusHistoricalSQLStatement<
            QueryGetInterlineTicketCarrierStatusHistorical>::
            mapRowToInterlineTicketCarrierStatus(row));
  }

  LOG4CXX_INFO(_logger,
               "GETINTERLINETICKETCARRIERSTATUSHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findInterlineTicketCarrierStatus()

///////////////////////////////////////////////////////////
//
//  QueryGetAllInterlineTicketCarrierStatus
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllInterlineTicketCarrierStatus::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllInterlineTicketCarrierStatus"));
std::string QueryGetAllInterlineTicketCarrierStatus::_baseSQL;
bool QueryGetAllInterlineTicketCarrierStatus::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllInterlineTicketCarrierStatus>
g_GetAllInterlineTicketCarrierStatus;

const char*
QueryGetAllInterlineTicketCarrierStatus::getQueryName() const
{
  return "GETALLINTERLINETICKETCARRIERSTATUS";
}

void
QueryGetAllInterlineTicketCarrierStatus::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllInterlineTicketCarrierStatusSQLStatement<QueryGetAllInterlineTicketCarrierStatus>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINTERLINETICKETCARRIERSTATUS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()
void
QueryGetAllInterlineTicketCarrierStatus::findAllInterlineTicketCarrierStatus(
    std::vector<tse::InterlineTicketCarrierStatus*>& interlineTicketCarriersStatus)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    interlineTicketCarriersStatus.push_back(QueryGetAllInterlineTicketCarrierStatusSQLStatement<
        QueryGetAllInterlineTicketCarrierStatus>::mapRowToInterlineTicketCarrierStatus(row));
  }

  LOG4CXX_INFO(_logger,
               "GETALLINTERLINETICKETCARRIERSTATUS: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllInterlineTicketCarrierStatus()

///////////////////////////////////////////////////////////
//
//  QueryGetAllInterlineTicketCarrierStatusHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllInterlineTicketCarrierStatusHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetAllInterlineTicketCarrierStatusHistorical"));
std::string QueryGetAllInterlineTicketCarrierStatusHistorical::_baseSQL;
bool QueryGetAllInterlineTicketCarrierStatusHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllInterlineTicketCarrierStatusHistorical>
g_GetAllInterlineTicketCarrierStatusHistorical;

const char*
QueryGetAllInterlineTicketCarrierStatusHistorical::getQueryName() const
{
  return "GETALLINTERLINETICKETCARRIERSTATUSHISTORICAL";
}

void
QueryGetAllInterlineTicketCarrierStatusHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllInterlineTicketCarrierStatusHistoricalSQLStatement<
        QueryGetAllInterlineTicketCarrierStatusHistorical> sqlStatement;
    _baseSQL =
        sqlStatement.RegisterColumnsAndBaseSQL("GETALLINTERLINETICKETCARRIERSTATUSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllInterlineTicketCarrierStatusHistorical::findAllInterlineTicketCarrierStatus(
    std::vector<tse::InterlineTicketCarrierStatus*>& interlineTicketCarriersStatus)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    interlineTicketCarriersStatus.push_back(
        QueryGetAllInterlineTicketCarrierStatusHistoricalSQLStatement<
            QueryGetAllInterlineTicketCarrierStatusHistorical>::
            mapRowToInterlineTicketCarrierStatus(row));
  }

  LOG4CXX_INFO(_logger,
               "GETALLINTERLINETICKETCARRIERSTATUSHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllInterlineTicketCarrierStatus()
}
