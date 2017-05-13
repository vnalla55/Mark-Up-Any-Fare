//----------------------------------------------------------------------------
//  File:           QueryGetTravelRestriction.cpp
//  Description:    QueryGetTravelRestriction
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
#include "DBAccess/Queries/QueryGetTravelRestriction.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTravelRestrictionSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTravelRestriction::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTravelRestriction"));
std::string QueryGetTravelRestriction::_baseSQL;
bool QueryGetTravelRestriction::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTravelRestriction> g_GetTravelRestriction;

const char*
QueryGetTravelRestriction::getQueryName() const
{
  return "GETTRAVELRESTRICTION";
}

void
QueryGetTravelRestriction::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTravelRestrictionSQLStatement<QueryGetTravelRestriction> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTRAVELRESTRICTION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTravelRestriction::findTravelRestriction(std::vector<tse::TravelRestriction*>& lstTR,
                                                 VendorCode& vendor,
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
    lstTR.push_back(
        QueryGetTravelRestrictionSQLStatement<QueryGetTravelRestriction>::mapRowToTravelRestriction(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETTRAVELRESTRICTION: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findTravelRestriction()

///////////////////////////////////////////////////////////
//  QueryGetTravelRestrictionHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTravelRestrictionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTravelRestrictionHistorical"));
std::string QueryGetTravelRestrictionHistorical::_baseSQL;
bool QueryGetTravelRestrictionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTravelRestrictionHistorical> g_GetTravelRestrictionHistorical;

const char*
QueryGetTravelRestrictionHistorical::getQueryName() const
{
  return "GETTRAVELRESTRICTIONHISTORICAL";
}

void
QueryGetTravelRestrictionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTravelRestrictionHistoricalSQLStatement<QueryGetTravelRestrictionHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTRAVELRESTRICTIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTravelRestrictionHistorical::findTravelRestriction(
    std::vector<tse::TravelRestriction*>& lstTR,
    VendorCode& vendor,
    int itemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstTR.push_back(QueryGetTravelRestrictionHistoricalSQLStatement<
        QueryGetTravelRestrictionHistorical>::mapRowToTravelRestriction(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTRAVELRESTRICTIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findTravelRestriction()
}
