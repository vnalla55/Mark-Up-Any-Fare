//----------------------------------------------------------------------------
//  File:           QueryGetTaxNation.cpp
//  Description:    QueryGetTaxNation
//  Created:        8/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetTaxNation.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxNationSQLStatement.h"
#include "DBAccess/TaxOrderTktIssue.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTaxNation::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxNation"));
std::string QueryGetTaxNation::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxNRound::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxNation.GetTaxNationRound"));
std::string QueryGetTaxNRound::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxNCollect::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxNation.GetTaxNationCollect"));
std::string QueryGetTaxNCollect::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxOrderTktIssue::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxNation.GetTaxOrderTktIssue"));
std::string QueryGetTaxOrderTktIssue::_baseSQL;

bool QueryGetTaxNation::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxNation> g_GetTaxNation;
const char*
QueryGetTaxNation::getQueryName() const
{
  return "GETTAXNATION";
};

void
QueryGetTaxNation::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxNationSQLStatement<QueryGetTaxNation> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXNATION");
    substTableDef(&_baseSQL);

    QueryGetTaxNRound::initialize();
    QueryGetTaxNCollect::initialize();
    QueryGetTaxOrderTktIssue::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxNation::findTaxNation(std::vector<tse::TaxNation*>& TaxN, const NationCode& nationCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(nationCode, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TaxNation* pTaxNation = nullptr;
  tse::TaxNation* prevTaxN = nullptr;
  while ((row = res.nextRow()))
  {
    pTaxNation = QueryGetTaxNationSQLStatement<QueryGetTaxNation>::mapRowToTaxNation(row, prevTaxN);
    if (pTaxNation != prevTaxN)
      TaxN.push_back(pTaxNation);

    prevTaxN = pTaxNation;
  }
  LOG4CXX_INFO(_logger,
               "GETTAXNATION: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                          << stopCPU() << ")");
  res.freeResult();

  // Get Children
  QueryGetTaxNRound SQLTaxNationRound(_dbAdapt);
  QueryGetTaxNCollect SQLTaxNCollect(_dbAdapt);
  QueryGetTaxOrderTktIssue SQLTaxOrderTktIssue(_dbAdapt);
  std::vector<TaxNation*>::iterator TaxIt;
  for (TaxIt = TaxN.begin(); TaxIt != TaxN.end(); TaxIt++)
  {
    SQLTaxNationRound.getExceptNation(*TaxIt, nationCode);
    SQLTaxNCollect.getCollectNation(*TaxIt, nationCode);
    SQLTaxOrderTktIssue.getOrderTktIssue(*TaxIt, nationCode);
  }
} // QueryGetTaxNation::findTaxNation()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxNRound
//
///////////////////////////////////////////////////////////
bool QueryGetTaxNRound::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxNRound> g_GetTaxNRound;
const char*
QueryGetTaxNRound::getQueryName() const
{
  return "GETTAXNROUND";
}

void
QueryGetTaxNRound::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxNRoundSQLStatement<QueryGetTaxNRound> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXNROUND");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxNRound::getExceptNation(tse::TaxNation* a_pTaxNation, const NationCode& nationCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(nationCode, 1);
  substParm(2, a_pTaxNation->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pTaxNation->taxNationRound().push_back(
        QueryGetTaxNRoundSQLStatement<QueryGetTaxNRound>::mapRowToExceptNation(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTAXNROUND: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                          << stopCPU() << ")");
  res.freeResult();
} // getExceptNation()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxNCollect
//
///////////////////////////////////////////////////////////
bool QueryGetTaxNCollect::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxNCollect> g_GetTaxNCollect;
const char*
QueryGetTaxNCollect::getQueryName() const
{
  return "GETTAXNCOLLECT";
}

void
QueryGetTaxNCollect::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxNCollectSQLStatement<QueryGetTaxNCollect> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXNCOLLECT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxNCollect::getCollectNation(tse::TaxNation* a_pTaxNation, const NationCode& nationCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(nationCode, 1);
  substParm(2, a_pTaxNation->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pTaxNation->taxNationCollect().push_back(
        QueryGetTaxNCollectSQLStatement<QueryGetTaxNCollect>::mapRowToCollectNation(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTAXNCOLLECT: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
} // getCollectNation()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxOrderTktIssue
//
///////////////////////////////////////////////////////////
bool QueryGetTaxOrderTktIssue::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxOrderTktIssue> g_GetTaxOrderTktIssue;
const char*
QueryGetTaxOrderTktIssue::getQueryName() const
{
  return "GETTAXORDERTKTISSUE";
}

void
QueryGetTaxOrderTktIssue::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxOrderTktIssueSQLStatement<QueryGetTaxOrderTktIssue> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXORDERTKTISSUE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxOrderTktIssue::getOrderTktIssue(tse::TaxNation* a_pTaxNation,
                                           const NationCode& nationCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(nationCode, 1);
  substParm(2, a_pTaxNation->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    TaxOrderTktIssue taxOrderTktIssue;
    QueryGetTaxOrderTktIssueSQLStatement<QueryGetTaxOrderTktIssue>::mapRow(*row, taxOrderTktIssue);
    a_pTaxNation->taxOrderTktIssue().push_back(taxOrderTktIssue);
  }
  LOG4CXX_INFO(_logger,
               "GETTAXORDERTKTISSUE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // getOrderTktIssue()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxNationHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTaxNationHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxNationHistorical"));
std::string QueryGetTaxNationHistorical::_baseSQL;
bool QueryGetTaxNationHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxNationHistorical> g_GetTaxNationHistorical;

const char*
QueryGetTaxNationHistorical::getQueryName() const
{
  return "GETTAXNATIONHISTORICAL";
};

void
QueryGetTaxNationHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxNationHistoricalSQLStatement<QueryGetTaxNationHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXNATIONHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetTaxNRound::initialize();
    QueryGetTaxNCollect::initialize();
    QueryGetTaxOrderTktIssue::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxNationHistorical::findTaxNation(std::vector<tse::TaxNation*>& TaxN,
                                           const NationCode& nationCode,
                                           const DateTime& startDate,
                                           const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(nationCode, 1);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TaxNation* pTaxNation = nullptr;
  tse::TaxNation* prevTaxN = nullptr;
  while ((row = res.nextRow()))
  {
    pTaxNation =
        QueryGetTaxNationHistoricalSQLStatement<QueryGetTaxNationHistorical>::mapRowToTaxNation(
            row, prevTaxN);
    if (pTaxNation != prevTaxN)
      TaxN.push_back(pTaxNation);

    prevTaxN = pTaxNation;
  }
  LOG4CXX_INFO(_logger,
               "GETTAXNATIONHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();

  // Get Children
  QueryGetTaxNRound SQLTaxNationRound(_dbAdapt);
  QueryGetTaxNCollect SQLTaxNCollect(_dbAdapt);
  QueryGetTaxOrderTktIssue SQLTaxOrderTktIssue(_dbAdapt);
  std::vector<TaxNation*>::iterator TaxIt;
  for (TaxIt = TaxN.begin(); TaxIt != TaxN.end(); TaxIt++)
  {
    SQLTaxNationRound.getExceptNation(*TaxIt, nationCode);
    SQLTaxNCollect.getCollectNation(*TaxIt, nationCode);
    SQLTaxOrderTktIssue.getOrderTktIssue(*TaxIt, nationCode);
  }
} // QueryGetTaxNationHistorical::findTaxNation()
}
