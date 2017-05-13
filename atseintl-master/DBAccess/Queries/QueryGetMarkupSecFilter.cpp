//----------------------------------------------------------------------------
//  File:           QueryGetMarkupSecFilter.cpp
//  Description:    QueryGetMarkupSecFilter
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
#include "DBAccess/Queries/QueryGetMarkupSecFilter.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMarkupSecFilterSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMarkupSecFilter::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMarkupSecFilter"));
std::string QueryGetMarkupSecFilter::_baseSQL;
bool QueryGetMarkupSecFilter::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMarkupSecFilter> g_GetMarkupSecFilter;

const char*
QueryGetMarkupSecFilter::getQueryName() const
{
  return "GETMARKUPSECFILTER";
}

void
QueryGetMarkupSecFilter::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMarkupSecFilterSQLStatement<QueryGetMarkupSecFilter> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMARKUPSECFILTER");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMarkupSecFilter::findMarkupSecFilter(std::vector<tse::MarkupSecFilter*>& lstMSF,
                                             VendorCode& vendor,
                                             CarrierCode& carrier,
                                             TariffNumber ruleTariff,
                                             RuleNumber& rule)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char rtStr[15];
  sprintf(rtStr, "%d", ruleTariff);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(rtStr, 3);
  substParm(rule, 4);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()) != nullptr)
  {
    lstMSF.push_back(
        QueryGetMarkupSecFilterSQLStatement<QueryGetMarkupSecFilter>::mapRowToMarkupSecFilter(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMARKUPSECFILTER: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findMarkupSecFilter()

///////////////////////////////////////////////////////////
//  QueryGetMarkupSecFilterHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMarkupSecFilterHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetMarkupSecFilterHistorical"));
std::string QueryGetMarkupSecFilterHistorical::_baseSQL;
bool QueryGetMarkupSecFilterHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMarkupSecFilterHistorical> g_GetMarkupSecFilterHistorical;

const char*
QueryGetMarkupSecFilterHistorical::getQueryName() const
{
  return "GETMARKUPSECFILTERHISTORICAL";
}

void
QueryGetMarkupSecFilterHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMarkupSecFilterHistoricalSQLStatement<QueryGetMarkupSecFilterHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMARKUPSECFILTERHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMarkupSecFilterHistorical::findMarkupSecFilter(std::vector<tse::MarkupSecFilter*>& lstMSF,
                                                       VendorCode& vendor,
                                                       CarrierCode& carrier,
                                                       TariffNumber ruleTariff,
                                                       RuleNumber& rule,
                                                       const DateTime& startDate,
                                                       const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char rtStr[15];
  sprintf(rtStr, "%d", ruleTariff);

  substParm(1, vendor);
  substParm(2, carrier);
  substParm(3, rtStr);
  substParm(4, rule);
  substParm(5, endDate);
  substParm(6, startDate);
  substParm(7, vendor);
  substParm(8, carrier);
  substParm(9, rtStr);
  substParm(10, rule);
  substParm(11, endDate);
  substParm(12, startDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstMSF.push_back(QueryGetMarkupSecFilterHistoricalSQLStatement<
        QueryGetMarkupSecFilterHistorical>::mapRowToMarkupSecFilter(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMARKUPSECFILTERHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findMarkupSecFilter()
}
