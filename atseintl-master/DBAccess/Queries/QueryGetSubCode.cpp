//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetSubCode.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSubCodeSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSubCode::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSubCode"));
std::string QueryGetSubCode::_baseSQL;
bool QueryGetSubCode::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSubCode> g_GetSubCode;

const char*
QueryGetSubCode::getQueryName() const
{
  return "GETSUBCODE";
}

void
QueryGetSubCode::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSubCodeSQLStatement<QueryGetSubCode> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSUBCODE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetSubCode::findSubCodeInfo(std::vector<SubCodeInfo*>& subCodes,
                                 const VendorCode& vendor,
                                 const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    subCodes.push_back(QueryGetSubCodeSQLStatement<QueryGetSubCode>::mapRowToSubCodeInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSUBCODE: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                      << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetSubCodeHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSubCodeHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetSubCodeHistorical"));
std::string QueryGetSubCodeHistorical::_baseSQL;
bool QueryGetSubCodeHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSubCodeHistorical> g_GetSubCodeHistorical;

const char*
QueryGetSubCodeHistorical::getQueryName() const
{
  return "GETSUBCODEHISTORICAL";
}

void
QueryGetSubCodeHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSubCodeHistoricalSQLStatement<QueryGetSubCodeHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSUBCODEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetSubCodeHistorical::findSubCodeInfo(std::vector<SubCodeInfo*>& subCodes,
                                           const VendorCode& vendor,
                                           const CarrierCode& carrier,
                                           const DateTime& startDate,
                                           const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    subCodes.push_back(
        QueryGetSubCodeHistoricalSQLStatement<QueryGetSubCodeHistorical>::mapRowToSubCodeInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSUBCODEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
}
} // tse
