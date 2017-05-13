//----------------------------------------------------------------------------
//  File:           QueryGetFareTypeMatrixs.cpp
//  Description:    QueryGetFareTypeMatrixs
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

#include "DBAccess/Queries/QueryGetFareTypeMatrixs.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareTypeMatrixsSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareTypeMatrixs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareTypeMatrixs"));
std::string QueryGetFareTypeMatrixs::_baseSQL;
bool QueryGetFareTypeMatrixs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareTypeMatrixs> g_GetFareTypeMatrixs;

const char*
QueryGetFareTypeMatrixs::getQueryName() const
{
  return "GETFARETYPEMATRIXS";
}

void
QueryGetFareTypeMatrixs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareTypeMatrixsSQLStatement<QueryGetFareTypeMatrixs> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARETYPEMATRIXS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareTypeMatrixs::findAllFareTypeMatrix(std::vector<tse::FareTypeMatrix*>& FareTypeMatrixs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    FareTypeMatrixs.push_back(
        QueryGetFareTypeMatrixsSQLStatement<QueryGetFareTypeMatrixs>::mapRowToFareTypeMatrix(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFARETYPEMATRIXS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareTypeMatrix()

///////////////////////////////////////////////////////////
//
//  QueryGetFareTypeMatrixsHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFareTypeMatrixsHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareTypeMatrixsHistorical"));
std::string QueryGetFareTypeMatrixsHistorical::_baseSQL;
bool QueryGetFareTypeMatrixsHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareTypeMatrixsHistorical> g_GetFareTypeMatrixsHistorical;

const char*
QueryGetFareTypeMatrixsHistorical::getQueryName() const
{
  return "GETFARETYPEMATRIXSHISTORICAL";
}

void
QueryGetFareTypeMatrixsHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareTypeMatrixsHistoricalSQLStatement<QueryGetFareTypeMatrixsHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARETYPEMATRIXSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareTypeMatrixsHistorical::findAllFareTypeMatrix(
    std::vector<tse::FareTypeMatrix*>& FareTypeMatrixs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    FareTypeMatrixs.push_back(QueryGetFareTypeMatrixsHistoricalSQLStatement<
        QueryGetFareTypeMatrixsHistorical>::mapRowToFareTypeMatrix(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFARETYPEMATRIXSHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareTypeMatrix()
}
