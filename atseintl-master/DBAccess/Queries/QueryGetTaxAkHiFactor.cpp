//----------------------------------------------------------------------------
//  File:           QueryGetTaxAkHiFactor.cpp
//  Description:    QueryGetTaxAkHiFactor
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

#include "DBAccess/Queries/QueryGetTaxAkHiFactor.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxAkHiFactorSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetOneTaxAkHiFactor::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOneTaxAkHiFactor"));
std::string QueryGetOneTaxAkHiFactor::_baseSQL;
bool QueryGetOneTaxAkHiFactor::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOneTaxAkHiFactor> g_GetOneTaxAkHiFactor;

const char*
QueryGetOneTaxAkHiFactor::getQueryName() const
{
  return "GETONETAXAKHIFACTOR";
}

void
QueryGetOneTaxAkHiFactor::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOneTaxAkHiFactorSQLStatement<QueryGetOneTaxAkHiFactor> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETONETAXAKHIFACTOR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetOneTaxAkHiFactor::findTaxAkHiFactor(std::vector<tse::TaxAkHiFactor*>& lstTAHF,
                                            LocCode& city)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(city, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstTAHF.push_back(
        QueryGetOneTaxAkHiFactorSQLStatement<QueryGetOneTaxAkHiFactor>::mapRowToTaxAkHiFactor(row));
  }
  LOG4CXX_INFO(_logger,
               "GETONETAXAKHIFACTOR: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findTaxAkHiFactor()

///////////////////////////////////////////////////////////
//
//  QueryGetAllTaxAkHiFactor
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllTaxAkHiFactor::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllTaxAkHiFactor"));
std::string QueryGetAllTaxAkHiFactor::_baseSQL;
bool QueryGetAllTaxAkHiFactor::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTaxAkHiFactor> g_GetAllTaxAkHiFactor;

const char*
QueryGetAllTaxAkHiFactor::getQueryName() const
{
  return "GETALLTAXAKHIFACTOR";
}

void
QueryGetAllTaxAkHiFactor::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTaxAkHiFactorSQLStatement<QueryGetAllTaxAkHiFactor> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTAXAKHIFACTOR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllTaxAkHiFactor::findAllTaxAkHiFactor(std::vector<tse::TaxAkHiFactor*>& lstTAHF)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstTAHF.push_back(
        QueryGetAllTaxAkHiFactorSQLStatement<QueryGetAllTaxAkHiFactor>::mapRowToTaxAkHiFactor(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLTAXAKHIFACTOR: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findAllTaxAkHiFactor()

log4cxx::LoggerPtr
QueryGetOneTaxAkHiFactorHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOneTaxAkHiFactorHistorical"));
std::string QueryGetOneTaxAkHiFactorHistorical::_baseSQL;
bool QueryGetOneTaxAkHiFactorHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOneTaxAkHiFactorHistorical> g_GetOneTaxAkHiFactorHistorical;

const char*
QueryGetOneTaxAkHiFactorHistorical::getQueryName() const
{
  return "GETONETAXAKHIFACTORHISTORICAL";
}

void
QueryGetOneTaxAkHiFactorHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOneTaxAkHiFactorHistoricalSQLStatement<QueryGetOneTaxAkHiFactorHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETONETAXAKHIFACTORHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetOneTaxAkHiFactorHistorical::findTaxAkHiFactorHistorical(
    std::vector<tse::TaxAkHiFactor*>& lstTAHF,
    LocCode& city,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(city, 1);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstTAHF.push_back(QueryGetOneTaxAkHiFactorHistoricalSQLStatement<
        QueryGetOneTaxAkHiFactorHistorical>::mapRowToTaxAkHiFactor(row));
  }
  LOG4CXX_INFO(_logger,
               "GETONETAXAKHIFACTORHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findTaxAkHiFactorHistorical()
}
