//----------------------------------------------------------------------------
//  File:           QueryGetTaxReissue.cpp
//  Description:    QueryGetTaxReissue
//  Created:        10/14/2007
// Authors:         Dean Van Decker
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetTaxReissue.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxReissueSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTaxReissue::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxReissue"));
std::string QueryGetTaxReissue::_baseSQL;
bool QueryGetTaxReissue::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxReissue> g_GetTaxReissue;

const char*
QueryGetTaxReissue::getQueryName() const
{
  return "GETTAXREISSUE";
}

void
QueryGetTaxReissue::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxReissueSQLStatement<QueryGetTaxReissue> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXREISSUE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxReissue::findTaxReissue(std::vector<TaxReissue*>& taxReissue, const TaxCode& code)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(code, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  TaxReissue* prev = nullptr;
  while ((row = res.nextRow()))
  {
    TaxReissue* rec =
        QueryGetTaxReissueSQLStatement<QueryGetTaxReissue>::mapRowToTaxReissue(row, prev);
    if (prev != rec)
    {
      taxReissue.push_back(rec);
      prev = rec;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETTAXREISSUE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                           << stopCPU() << ")");
  res.freeResult();
} // QueryGetTaxReissue::findTaxReissue()

///////////////////////////////////////////////////////////
//  QueryGetTaxReissueHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTaxReissueHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxReissueHistorical"));
std::string QueryGetTaxReissueHistorical::_baseSQL;
bool QueryGetTaxReissueHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxReissueHistorical> g_GetTaxReissueHistorical;

const char*
QueryGetTaxReissueHistorical::getQueryName() const
{
  return "GETTAXREISSUEHISTORICAL";
}

void
QueryGetTaxReissueHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxReissueHistoricalSQLStatement<QueryGetTaxReissueHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXREISSUEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxReissueHistorical::findTaxReissue(std::vector<TaxReissue*>& taxReissue,
                                             const TaxCode& code)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(code, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  TaxReissue* prev = nullptr;
  while ((row = res.nextRow()))
  {
    TaxReissue* rec =
        QueryGetTaxReissueHistoricalSQLStatement<QueryGetTaxReissueHistorical>::mapRowToTaxReissue(
            row, prev);
    if (prev != rec)
    {
      taxReissue.push_back(rec);
      prev = rec;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETTAXREISSUEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetTaxReissueHistorical::findTaxReissue()

///////////////////////////////////////////////////////////
//  QueryGetAllTaxReissueHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllTaxReissueHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllTaxReissueHistorical"));
std::string QueryGetAllTaxReissueHistorical::_baseSQL;
bool QueryGetAllTaxReissueHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTaxReissueHistorical> g_GetAllTaxReissueHistorical;

const char*
QueryGetAllTaxReissueHistorical::getQueryName() const
{
  return "GETALLTAXREISSUEHISTORICAL";
}

void
QueryGetAllTaxReissueHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTaxReissueHistoricalSQLStatement<QueryGetAllTaxReissueHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTAXREISSUEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllTaxReissueHistorical::findAllTaxReissues(std::vector<TaxReissue*>& taxReissue)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  TaxReissue* prev = nullptr;
  while ((row = res.nextRow()))
  {
    TaxReissue* rec = QueryGetAllTaxReissueHistoricalSQLStatement<
        QueryGetAllTaxReissueHistorical>::mapRowToTaxReissue(row, prev);
    if (prev != rec)
    {
      taxReissue.push_back(rec);
      prev = rec;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETALLTAXREISSUEHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetAllTaxReissueHistorical::findTaxReissue()
}
