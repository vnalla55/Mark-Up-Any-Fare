//----------------------------------------------------------------------------
//  File:           QueryGetTicketStock.cpp
//  Description:    QueryGetTicketStock
//  Created:        5/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetTicketStock.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTicketStockSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTicketStock::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTicketStock"));
std::string QueryGetTicketStock::_baseSQL;
bool QueryGetTicketStock::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTicketStock> g_GetTicketStock;

const char*
QueryGetTicketStock::getQueryName() const
{
  return "GETTICKETSTOCK";
}

void
QueryGetTicketStock::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTicketStockSQLStatement<QueryGetTicketStock> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTICKETSTOCK");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTicketStock::findTicketStock(std::vector<tse::TicketStock*>& lstTS, int ticketStockCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, ticketStockCode);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TicketStock* tsPrev = nullptr;
  tse::TicketStock* ts = nullptr;
  while ((row = res.nextRow()) != nullptr)
  {
    ts = QueryGetTicketStockSQLStatement<QueryGetTicketStock>::mapRowToTicketStock(row, tsPrev);
    if (ts != tsPrev)
      lstTS.push_back(ts);

    tsPrev = ts;
  }
  LOG4CXX_INFO(_logger,
               "GETTICKETSTOCK: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
} // findTicketStock()

///////////////////////////////////////////////////////////
//
//  QueryGetTicketStockHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTicketStockHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTicketStockHistorical"));
std::string QueryGetTicketStockHistorical::_baseSQL;
bool QueryGetTicketStockHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTicketStockHistorical> g_GetTicketStockHistorical;

const char*
QueryGetTicketStockHistorical::getQueryName() const
{
  return "GETTICKETSTOCKHISTORICAL";
}

void
QueryGetTicketStockHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTicketStockHistoricalSQLStatement<QueryGetTicketStockHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTICKETSTOCKHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTicketStockHistorical::findTicketStock(std::vector<tse::TicketStock*>& lstTS,
                                               int ticketStockCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, ticketStockCode);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TicketStock* tsPrev = nullptr;
  tse::TicketStock* ts = nullptr;
  while ((row = res.nextRow()) != nullptr)
  {
    ts = QueryGetTicketStockHistoricalSQLStatement<
        QueryGetTicketStockHistorical>::mapRowToTicketStock(row, tsPrev);
    if (ts != tsPrev)
      lstTS.push_back(ts);

    tsPrev = ts;
  }
  LOG4CXX_INFO(_logger,
               "GETTICKETSTOCKHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findTicketStockHistorical()

///////////////////////////////////////////////////////////
//
//  QueryGetAllTicketStock
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllTicketStock::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllTicketStock"));
std::string QueryGetAllTicketStock::_baseSQL;
bool QueryGetAllTicketStock::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTicketStock> g_GetAllTicketStock;

const char*
QueryGetAllTicketStock::getQueryName() const
{
  return "GETALLTICKETSTOCK";
}

void
QueryGetAllTicketStock::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTicketStockSQLStatement<QueryGetAllTicketStock> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTICKETSTOCK");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllTicketStock::findAllTicketStock(std::vector<tse::TicketStock*>& lstTS)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TicketStock* tsPrev = nullptr;
  tse::TicketStock* ts = nullptr;
  while ((row = res.nextRow()) != nullptr)
  {
    ts = QueryGetAllTicketStockSQLStatement<QueryGetAllTicketStock>::mapRowToTicketStock(row,
                                                                                         tsPrev);
    if (ts != tsPrev)
      lstTS.push_back(ts);

    tsPrev = ts;
  }
  LOG4CXX_INFO(_logger,
               "GETALLTICKETSTOCK: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // findAllTicketStock()

///////////////////////////////////////////////////////////
//
//  QueryGetAllTicketStockHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllTicketStockHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllTicketStockHistorical"));
std::string QueryGetAllTicketStockHistorical::_baseSQL;
bool QueryGetAllTicketStockHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTicketStockHistorical> g_GetAllTicketStockHistorical;

const char*
QueryGetAllTicketStockHistorical::getQueryName() const
{
  return "GETALLTICKETSTOCKHISTORICAL";
}

void
QueryGetAllTicketStockHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTicketStockHistoricalSQLStatement<QueryGetAllTicketStockHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTICKETSTOCKHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllTicketStockHistorical::findAllTicketStock(std::vector<tse::TicketStock*>& lstTS)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TicketStock* tsPrev = nullptr;
  tse::TicketStock* ts = nullptr;
  while ((row = res.nextRow()) != nullptr)
  {
    ts = QueryGetAllTicketStockHistoricalSQLStatement<
        QueryGetAllTicketStockHistorical>::mapRowToTicketStock(row, tsPrev);
    if (ts != tsPrev)
      lstTS.push_back(ts);

    tsPrev = ts;
  }
  LOG4CXX_INFO(_logger,
               "GETALLTICKETSTOCKHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllTicketStock()
}
