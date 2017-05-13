//----------------------------------------------------------------------------
//  File:           QueryGetNuc.cpp
//  Description:    QueryGetNuc
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

#include "DBAccess/Queries/QueryGetNuc.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNucSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetOneNuc::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOneNuc"));
std::string QueryGetOneNuc::_baseSQL;
bool QueryGetOneNuc::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOneNuc> g_GetOneNuc;

const char*
QueryGetOneNuc::getQueryName() const
{
  return "GETONENUC";
}

void
QueryGetOneNuc::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOneNucSQLStatement<QueryGetOneNuc> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETONENUC");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetOneNuc::findNUC(std::vector<tse::NUCInfo*>& nucs, CurrencyCode cur, CarrierCode carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(cur, 1);
  substParm(carrier, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    nucs.push_back(QueryGetOneNucSQLStatement<QueryGetOneNuc>::mapRowToNUCInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETONENUC: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                       << stopCPU() << ")");
  res.freeResult();
} // findNUC()

///////////////////////////////////////////////////////////
//
//  QueryGetAllNucs
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllNucs::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllNucs"));
std::string QueryGetAllNucs::_baseSQL;
bool QueryGetAllNucs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllNucs> g_GetAllNucs;

const char*
QueryGetAllNucs::getQueryName() const
{
  return "GETALLNUCS";
}

void
QueryGetAllNucs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllNucsSQLStatement<QueryGetAllNucs> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLNUCS");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
tse::QueryGetAllNucs::findAllNUC(std::vector<tse::NUCInfo*>& nucs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    nucs.push_back(QueryGetAllNucsSQLStatement<QueryGetAllNucs>::mapRowToNUCInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLNUCS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                        << stopCPU() << ")");

  res.freeResult();
} // findAllNUC()

///////////////////////////////////////////////////////////
//
//  QueryGetNucHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetNucHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNucHistorical"));
std::string QueryGetNucHistorical::_baseSQL;
bool QueryGetNucHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNucHistorical> g_GetNucHistorical;

const char*
QueryGetNucHistorical::getQueryName() const
{
  return "GETNUCHISTORICAL";
}

void
QueryGetNucHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNucHistoricalSQLStatement<QueryGetNucHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNUCHISTORICAL");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNucHistorical::findNUC(std::vector<tse::NUCInfo*>& nucs,
                               CurrencyCode cur,
                               CarrierCode carrier,
                               const DateTime& startDate,
                               const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  //    std::string startdate;
  //    convertDate(startdate, startDate);
  //    std::string enddate;
  //    convertDate(enddate, endDate);

  substParm(cur, 1);
  substParm(carrier, 2);
  substParm(3, endDate);
  substParm(4, endDate);
  substParm(5, startDate);
  substParm(cur, 6);
  substParm(carrier, 7);
  substParm(8, endDate);
  substParm(9, endDate);
  substParm(10, startDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    nucs.push_back(QueryGetNucHistoricalSQLStatement<QueryGetNucHistorical>::mapRowToNUCInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETNUCHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // QueryGetNucHistorical::findNUC()
}
