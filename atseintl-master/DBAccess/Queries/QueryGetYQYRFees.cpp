//----------------------------------------------------------------------------
//  File:           QueryGetYQYRFees.cpp
//  Description:    QueryGetYQYRFees
//  Created:        8/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetYQYRFees.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/HashKey.h"
#include "DBAccess/Queries/QueryGetYQYRFeesSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetYQYRFees::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetYQYRFees"));
std::string QueryGetYQYRFees::_baseSQL;

bool QueryGetYQYRFees::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetYQYRFees> g_GetYQYRFees;

const char*
QueryGetYQYRFees::getQueryName() const
{
  return "GETYQYRFEES";
};

void
QueryGetYQYRFees::initialize()
{
  if (!_isInitialized)
  {
    QueryGetYQYRFeesSQLStatement<QueryGetYQYRFees> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETYQYRFEES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetYQYRFees::findYQYRFees(std::vector<tse::YQYRFees*>& yqyrFees, CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  YQYRFees* fee = nullptr;
  YQYRFees* feePrev = nullptr;
  while ((row = res.nextRow()))
  {
    fee = QueryGetYQYRFeesSQLStatement<QueryGetYQYRFees>::mapRowToYQYRFees(row, feePrev);
    if (LIKELY(fee != feePrev))
      yqyrFees.push_back(fee);

    feePrev = fee;
  }
  LOG4CXX_INFO(_logger,
               "GETYQYRFEES: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ")");
  res.freeResult();

} // QueryGetYQYRFees::findYQYRFees()

typedef HashKey<VendorCode, int> YQYRFeesKey;
typedef std::map<YQYRFeesKey, tse::YQYRFees*> YQYRFeesCacheMap;

///////////////////////////////////////////////////////////
//
//  QueryGetYQYRFeesHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetYQYRFeesHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetYQYRFeesHistorical"));
std::string QueryGetYQYRFeesHistorical::_baseSQL;
bool QueryGetYQYRFeesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetYQYRFeesHistorical> g_GetYQYRFeesHistorical;

const char*
QueryGetYQYRFeesHistorical::getQueryName() const
{
  return "GETYQYRFEESHISTORICAL";
};

void
QueryGetYQYRFeesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetYQYRFeesHistoricalSQLStatement<QueryGetYQYRFeesHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETYQYRFEESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetYQYRFeesHistorical::findYQYRFees(std::vector<tse::YQYRFees*>& yqyrFees,
                                         CarrierCode& carrier,
                                         const DateTime& startDate,
                                         const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  YQYRFees* fee = nullptr;
  YQYRFees* feePrev = nullptr;
  while ((row = res.nextRow()))
  {

    fee = QueryGetYQYRFeesHistoricalSQLStatement<QueryGetYQYRFeesHistorical>::mapRowToYQYRFees(
        row, feePrev);

    if (fee != feePrev)
    {
      // detect if this is new version of previous entry...
      // its needed for historical data processing
      fee->modDate() = boost::date_time::pos_infin;

      if (feePrev)
      {
        if (fee->seqNo() == feePrev->seqNo() && fee->subCode() == feePrev->subCode() &&
            fee->carrier() == feePrev->carrier() && fee->vendor() == feePrev->vendor() &&
            fee->taxCode() == feePrev->taxCode())
        {
          feePrev->modDate() = fee->createDate();
        }
      }
      // end moddate block

      yqyrFees.push_back(fee);
    }
    feePrev = fee;
  }
  LOG4CXX_INFO(_logger,
               "GETYQYRFEESHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();

} // QueryGetYQYRFeesHistorical::findYQYRFees()

///////////////////////////////////////////////////////////
//
//  QueryGetAllYQYRFees
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllYQYRFees::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllYQYRFees"));
std::string QueryGetAllYQYRFees::_baseSQL;
bool QueryGetAllYQYRFees::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllYQYRFees> g_GetAllYQYRFees;

const char*
QueryGetAllYQYRFees::getQueryName() const
{
  return "GETALLYQYRFEES";
};

void
QueryGetAllYQYRFees::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllYQYRFeesSQLStatement<QueryGetAllYQYRFees> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLYQYRFEES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllYQYRFees::findAllYQYRFees(std::vector<tse::YQYRFees*>& yqyrFees)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::YQYRFees* fee = nullptr;
  tse::YQYRFees* feePrev = nullptr;
  while ((row = res.nextRow()))
  {
    fee = QueryGetAllYQYRFeesSQLStatement<QueryGetAllYQYRFees>::mapRowToYQYRFees(row, feePrev);
    if (fee != feePrev)
      yqyrFees.push_back(fee);

    feePrev = fee;
  }
  LOG4CXX_INFO(_logger,
               "GETALLYQYRFEES: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();

} // findAllYQYRFees()
}
