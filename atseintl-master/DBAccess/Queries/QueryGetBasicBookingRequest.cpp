//----------------------------------------------------------------------------
//  File:           QueryGetBasicBookingRequest.cpp
//  Description:    QueryGetBasicBookingRequest
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

#include "DBAccess/Queries/QueryGetBasicBookingRequest.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBasicBookingRequestSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetBasicBookingRequest::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetBasicBookingRequest"));

std::string QueryGetBasicBookingRequest::_baseSQL;
bool QueryGetBasicBookingRequest::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBasicBookingRequest> g_GetBasicBookingRequest;

const char*
QueryGetBasicBookingRequest::getQueryName() const
{
  return "GETBASICBOOKINGREQUEST";
}

void
QueryGetBasicBookingRequest::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBasicBookingRequestSQLStatement<QueryGetBasicBookingRequest> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBASICBOOKINGREQUEST");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBasicBookingRequest::findBasicBookingRequest(std::vector<tse::BasicBookingRequest*>& bbr,
                                                     const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    bbr.push_back(QueryGetBasicBookingRequestSQLStatement<
        QueryGetBasicBookingRequest>::mapRowToBasicBookingRequest(row));
  }
  LOG4CXX_INFO(_logger,
               "GETBASICBOOKINGREQUEST: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findBasicBookingRequest()

///////////////////////////////////////////////////////////
//
//  QueryGetBasicBookingRequests
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetBasicBookingRequests::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetBasicBookingRequests"));
std::string QueryGetBasicBookingRequests::_baseSQL;
bool QueryGetBasicBookingRequests::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBasicBookingRequests> g_GetBasicBookingRequests;

const char*
QueryGetBasicBookingRequests::getQueryName() const
{
  return "GETBASICBOOKINGREQUESTS";
}

void
QueryGetBasicBookingRequests::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBasicBookingRequestsSQLStatement<QueryGetBasicBookingRequests> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBASICBOOKINGREQUESTS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBasicBookingRequests::findAllBBR(std::vector<tse::BasicBookingRequest*>& bbr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    bbr.push_back(QueryGetBasicBookingRequestsSQLStatement<
        QueryGetBasicBookingRequests>::mapRowToBasicBookingRequest(row));
  }
  LOG4CXX_INFO(_logger,
               "GETBASICBOOKINGREQUESTS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllBBR()
}
