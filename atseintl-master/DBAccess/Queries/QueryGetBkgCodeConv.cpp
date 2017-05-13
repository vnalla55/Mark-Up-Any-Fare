//----------------------------------------------------------------------------
//  File:           QueryGetBkgCodeConv.cpp
//  Description:    QueryGetBkgCodeConv
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

#include "DBAccess/Queries/QueryGetBkgCodeConv.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBkgCodeConvSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetBkgCodeConv::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetBkgCodeConv"));
std::string QueryGetBkgCodeConv::_baseSQL;
bool QueryGetBkgCodeConv::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBkgCodeConv> g_GetBkgCodeConv;

const char*
QueryGetBkgCodeConv::getQueryName() const
{
  return "GETBKGCODECONV";
}

void
QueryGetBkgCodeConv::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBkgCodeConvSQLStatement<QueryGetBkgCodeConv> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBKGCODECONV");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBkgCodeConv::findBookingCodeConv(std::vector<tse::BookingCodeConv*>& bkgCodeConvs,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const TariffNumber& ruleTariff,
                                         const RuleNumber& rule)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, vendor);
  substParm(2, carrier);
  substParm(3, ruleTariff);
  substParm(4, rule);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    bkgCodeConvs.push_back(
        QueryGetBkgCodeConvSQLStatement<QueryGetBkgCodeConv>::mapRowToBookingCodeConv(row));
  }
  LOG4CXX_INFO(_logger,
               "GETBKGCODECONV: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
} // findBookingCodeConv()

///////////////////////////////////////////////////////////
//
//  QueryGetAllBkgCodeConv
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllBkgCodeConv::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllBookingCodeConv"));
std::string QueryGetAllBkgCodeConv::_baseSQL;
bool QueryGetAllBkgCodeConv::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllBkgCodeConv> g_GetAllBkgCodeConv;

const char*
QueryGetAllBkgCodeConv::getQueryName() const
{
  return "GETALLBKGCODECONV";
}

void
QueryGetAllBkgCodeConv::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllBkgCodeConvSQLStatement<QueryGetAllBkgCodeConv> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLBKGCODECONV");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllBkgCodeConv::findAllBookingCodeConv(std::vector<tse::BookingCodeConv*>& bkgCodeConvs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    bkgCodeConvs.push_back(
        QueryGetAllBkgCodeConvSQLStatement<QueryGetAllBkgCodeConv>::mapRowToBookingCodeConv(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLBKGCODECONV: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // findAllBookingCodeConv()

///////////////////////////////////////////////////////////
//
//  QueryGetBkgCodeConvHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetBkgCodeConvHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetBkgCodeConvHistorical"));
std::string QueryGetBkgCodeConvHistorical::_baseSQL;
bool QueryGetBkgCodeConvHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBkgCodeConvHistorical> g_GetBkgCodeConvHistorical;

const char*
QueryGetBkgCodeConvHistorical::getQueryName() const
{
  return "GETBKGCODECONVHISTORICAL";
}

void
QueryGetBkgCodeConvHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBkgCodeConvHistoricalSQLStatement<QueryGetBkgCodeConvHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBKGCODECONVHISTORICAL");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetBkgCodeConvHistorical::findBookingCodeConv(std::vector<tse::BookingCodeConv*>& bkgCodeConvs,
                                                   const VendorCode& vendor,
                                                   const CarrierCode& carrier,
                                                   const TariffNumber& ruleTariff,
                                                   const RuleNumber& rule,
                                                   const DateTime& startDate,
                                                   const DateTime& endDate)

{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, vendor);
  substParm(2, carrier);
  substParm(3, ruleTariff);
  substParm(4, rule);
  substParm(5, startDate);
  substParm(6, endDate);
  substParm(7, vendor);
  substParm(8, carrier);
  substParm(9, ruleTariff);
  substParm(10, rule);
  substParm(11, startDate);
  substParm(12, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    bkgCodeConvs.push_back(QueryGetBkgCodeConvHistoricalSQLStatement<
        QueryGetBkgCodeConvHistorical>::mapRowToBookingCodeConv(row));
  }
  LOG4CXX_INFO(_logger,
               "GETBKGCODECONVHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findBookingCodeConv()
}
