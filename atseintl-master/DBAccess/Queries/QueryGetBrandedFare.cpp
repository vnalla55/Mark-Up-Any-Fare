//----------------------------------------------------------------------------
//  File:           QueryGetBrandedFare.cpp
//  Description:    QueryGetBrandedFare
//  Created:        03/22/2013
//
// Copyright 2013, Sabre Inc. All rights reserved. This software/documentation is the
// confidential and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetBrandedFare.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBrandedFareSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetBrandedFare::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetBrandedFare"));
std::string QueryGetBrandedFare::_baseSQL;
bool
QueryGetBrandedFare::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetBrandedFare> _getBrandedFare;

const char*
QueryGetBrandedFare::getQueryName() const
{
  return "GETBRANDEDFARE";
}

void
QueryGetBrandedFare::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBrandedFareSQLStatement<QueryGetBrandedFare> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBRANDEDFARE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetBrandedFare::findBrandedFare(std::vector<BrandedFare*>& lst,
                                     const VendorCode& vendor,
                                     const CarrierCode& carrier)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(vendor, 1);
  substParm(carrier, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  BrandedFare* bfPrev(nullptr);
  BrandedFare* bf(nullptr);
  while ((row = res.nextRow()))
  {
    bf = QueryGetBrandedFareSQLStatement<QueryGetBrandedFare>::mapRowToBrandedFare(row, bfPrev);
    if (bf != bfPrev)
      lst.push_back(bf);

    bfPrev = bf;
  }
  LOG4CXX_INFO(_logger,
               "GETBRANDEDFARE: NumRows: " << res.numRows() << " Time = " << stopTimer() << " ("
                                           << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetBrandedFareHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetBrandedFareHistorical"));
std::string QueryGetBrandedFareHistorical::_baseSQL;
bool
QueryGetBrandedFareHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetBrandedFareHistorical> _getBrandedFareHistorical;

const char*
QueryGetBrandedFareHistorical::getQueryName() const
{
  return "GETBRANDEDFAREHISTORICAL";
}

void
QueryGetBrandedFareHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBrandedFareHistoricalSQLStatement<QueryGetBrandedFareHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBRANDEDFAREHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetBrandedFareHistorical::findBrandedFare(std::vector<BrandedFare*>& lst,
                                               const VendorCode& vendor,
                                               const CarrierCode& carrier,
                                               const DateTime& startDate,
                                               const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  BrandedFare* bfPrev(nullptr);
  BrandedFare* bf(nullptr);
  while ((row = res.nextRow()))
  {
    bf = QueryGetBrandedFareHistoricalSQLStatement<
        QueryGetBrandedFareHistorical>::mapRowToBrandedFare(row, bfPrev);
    if (bf != bfPrev)
      lst.push_back(bf);

    bfPrev = bf;
  }
  LOG4CXX_INFO(_logger,
               "GETBRANDEDFAREHISTORICAL: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllBrandedFare::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllBrandedFare"));
std::string QueryGetAllBrandedFare::_baseSQL;
bool
QueryGetAllBrandedFare::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllBrandedFare> _getAllBrandedFare;

const char*
QueryGetAllBrandedFare::getQueryName() const
{
  return "GETALLBRANDEDFARE";
}

void
QueryGetAllBrandedFare::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllBrandedFareSQLStatement<QueryGetAllBrandedFare> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLBRANDEDFARE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllBrandedFare::findAllBrandedFare(std::vector<BrandedFare*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  BrandedFare* bfPrev(nullptr);
  BrandedFare* bf(nullptr);
  while ((row = res.nextRow()))
  {
    bf = QueryGetAllBrandedFareSQLStatement<QueryGetAllBrandedFare>::mapRowToBrandedFare(row,
                                                                                         bfPrev);
    if (bf != bfPrev)
      lst.push_back(bf);

    bfPrev = bf;
  }
  LOG4CXX_INFO(_logger,
               "GETALLBRANDEDFARE: NumRows: " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ") mSecs");
  res.freeResult();
}
} // tse
