//----------------------------------------------------------------------------
//  File:           QueryGetAllGenericTaxCode.cpp
//  Description:    QueryGetAllGenericTaxCode
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

#include "DBAccess/Queries/QueryGetAllGenericTaxCode.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAllGenericTaxCodeSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetAllGenericTaxCode::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllGenericTaxCode"));
std::string QueryGetAllGenericTaxCode::_baseSQL;
bool QueryGetAllGenericTaxCode::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllGenericTaxCode> g_GetAllGenericTaxCode;

const char*
QueryGetAllGenericTaxCode::getQueryName() const
{
  return "GETALLGENERICTAXCODE";
}

void
QueryGetAllGenericTaxCode::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllGenericTaxCodeSQLStatement<QueryGetAllGenericTaxCode> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLGENERICTAXCODE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllGenericTaxCode::findAllGenericTaxCode(std::vector<tse::GenericTaxCode*>& taxCodes)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    taxCodes.push_back(
        QueryGetAllGenericTaxCodeSQLStatement<QueryGetAllGenericTaxCode>::mapRowToGenericTaxCode(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLGENERICTAXCODE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllGenericTaxCode()

///////////////////////////////////////////////////////////
//
//  QueryGetAllGenericTaxCodeHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllGenericTaxCodeHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllGenericTaxCodeHistorical"));
std::string QueryGetAllGenericTaxCodeHistorical::_baseSQL;
bool QueryGetAllGenericTaxCodeHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllGenericTaxCodeHistorical> g_GetAllGenericTaxCodeHistorical;

const char*
QueryGetAllGenericTaxCodeHistorical::getQueryName() const
{
  return "GETALLGENERICTAXCODEHISTORICAL";
}

void
QueryGetAllGenericTaxCodeHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllGenericTaxCodeHistoricalSQLStatement<QueryGetAllGenericTaxCodeHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLGENERICTAXCODEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllGenericTaxCodeHistorical::findAllGenericTaxCode(
    std::vector<tse::GenericTaxCode*>& taxCodes, const DateTime& startDate, const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, startDate);
  substParm(2, endDate);
  substParm(3, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    taxCodes.push_back(QueryGetAllGenericTaxCodeHistoricalSQLStatement<
        QueryGetAllGenericTaxCodeHistorical>::mapRowToGenericTaxCode(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLGENERICTAXCODEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllGenericTaxCode()
}
