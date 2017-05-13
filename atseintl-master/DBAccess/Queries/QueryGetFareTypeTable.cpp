//----------------------------------------------------------------------------
//  (C) 2009, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetFareTypeTable.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareTypeTableSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetFareTypeTable::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetFareTypeTable"));
std::string QueryGetFareTypeTable::_baseSQL;
bool QueryGetFareTypeTable::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareTypeTable> g_QueryGetFareTypeTable;

const char*
QueryGetFareTypeTable::getQueryName() const
{
  return "QUERYGETFARETYPETABLE";
}

void
QueryGetFareTypeTable::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareTypeTableSQLStatement<QueryGetFareTypeTable> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETFARETYPETABLE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareTypeTable::findFareTypeTable(std::vector<FareTypeTable*>& FareTypeTableVec,
                                         const VendorCode& vendor,
                                         int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    FareTypeTableVec.push_back(
        QueryGetFareTypeTableSQLStatement<QueryGetFareTypeTable>::mapRowToFareTypeTable(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFARETYPETABLE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllFareTypeTable::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllFareTypeTable"));
std::string QueryGetAllFareTypeTable::_baseSQL;
bool QueryGetAllFareTypeTable::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareTypeTable> g_QueryGetAllFareTypeTable;

const char*
QueryGetAllFareTypeTable::getQueryName() const
{
  return "QUERYGETALLFARETYPETABLE";
}

void
QueryGetAllFareTypeTable::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareTypeTableSQLStatement<QueryGetAllFareTypeTable> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETALLFARETYPETABLE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllFareTypeTable::findAllFareTypeTable(std::vector<FareTypeTable*>& fareTypeTableVec)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fareTypeTableVec.push_back(
        QueryGetAllFareTypeTableSQLStatement<QueryGetAllFareTypeTable>::mapRowToFareTypeTable(row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETALLFARETYPETABLE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetFareTypeTableHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetFareTypeTableHistorical"));
std::string QueryGetFareTypeTableHistorical::_baseSQL;
bool QueryGetFareTypeTableHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareTypeTableHistorical> g_QueryGetFareTypeTableHistorical;

const char*
QueryGetFareTypeTableHistorical::getQueryName() const
{
  return "QUERYGETFARETYPETABLEHISTORICAL";
}

void
QueryGetFareTypeTableHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareTypeTableHistoricalSQLStatement<QueryGetFareTypeTableHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETFARETYPETABLEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareTypeTableHistorical::findFareTypeTable(std::vector<FareTypeTable*>& FareTypeTableVec,
                                                   const VendorCode& vendor,
                                                   int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    FareTypeTableVec.push_back(QueryGetFareTypeTableHistoricalSQLStatement<
        QueryGetFareTypeTableHistorical>::mapRowToFareTypeTable(row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETFARETYPETABLEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
}
