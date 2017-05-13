//----------------------------------------------------------------------------
//  File:           QueryGetPfcTktDesigExcept.cpp
//  Description:    QueryGetPfcTktDesigExcept
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

#include "DBAccess/Queries/QueryGetPfcTktDesigExcept.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPfcTktDesigExceptSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetPfcTktDesigExcept::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPfcTktDesigExcept"));
std::string QueryGetPfcTktDesigExcept::_baseSQL;
bool QueryGetPfcTktDesigExcept::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPfcTktDesigExcept> g_GetPfcTktDesigExcept;

const char*
QueryGetPfcTktDesigExcept::getQueryName() const
{
  return "GETPFCTKTDESIGEXCEPT";
}

void
QueryGetPfcTktDesigExcept::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPfcTktDesigExceptSQLStatement<QueryGetPfcTktDesigExcept> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPFCTKTDESIGEXCEPT");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetPfcTktDesigExcept::findPfcTktDesigExcept(std::vector<const tse::PfcTktDesigExcept*>& tde,
                                                 const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tde.push_back(
        QueryGetPfcTktDesigExceptSQLStatement<QueryGetPfcTktDesigExcept>::mapRowToPfcTktDesigExcept(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETPFCTKTDESIGEXCEPT: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findPfcTktDesigExcept()

///////////////////////////////////////////////////////////
//
//  QueryGetPfcTktDesigExceptHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetPfcTktDesigExceptHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPfcTktDesigExceptHistorical"));
std::string QueryGetPfcTktDesigExceptHistorical::_baseSQL;
bool QueryGetPfcTktDesigExceptHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPfcTktDesigExceptHistorical> g_GetPfcTktDesigExceptHistorical;

const char*
QueryGetPfcTktDesigExceptHistorical::getQueryName() const
{
  return "GETPFCTKTDESIGEXCEPTHISTORICAL";
}

void
QueryGetPfcTktDesigExceptHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPfcTktDesigExceptHistoricalSQLStatement<QueryGetPfcTktDesigExceptHistorical>
    sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPFCTKTDESIGEXCEPTHISTORICAL");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetPfcTktDesigExceptHistorical::findPfcTktDesigExcept(
    std::vector<const tse::PfcTktDesigExcept*>& tde,
    const CarrierCode& carrier,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tde.push_back(QueryGetPfcTktDesigExceptHistoricalSQLStatement<
        QueryGetPfcTktDesigExceptHistorical>::mapRowToPfcTktDesigExcept(row));
  }
  LOG4CXX_INFO(_logger,
               "GETPFCTKTDESIGEXCEPTHISTORICAL: NumRows "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findPfcTktDesigExcept()
}
